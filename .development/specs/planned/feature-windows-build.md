---
type: feature
priority: nice-to-have
status: planned
category: infrastructure
related: [feature-code-graph]
depends_on: []
decided_by: ../../reference/decisions/011-release-and-distribution.md
created: 2026-10-02
---

# Windows build

## Summary

dev-dash si compila e funziona su Windows con MSVC, senza toccare il percorso
Linux. Il codice C++ è già quasi portabile. Le barriere sono tre: la
configurazione CMake, che rifiuta apposta ogni piattaforma diversa da Linux;
la cartella home, letta solo da `$HOME`; e le conversioni tra percorsi e testo,
che su Windows non danno UTF-8.

## Motivation

- **Il trigger c'è.** Il 2026-10-02 Dario ha provato dev-dash su Windows: la
  build si ferma, e con un paio di modifiche minime l'app parte. Però mostra
  il messaggio "$HOME is not set — scaffolds and snapshots are disabled this
  session.", e Valentina l'ha visto a schermo. È il primo uso reale fuori da
  Linux, e viene da un collaboratore. ADR-006 (addendum 2026-06-10) e ADR-011
  rimandavano la scelta della piattaforma proprio a un caso concreto.
- **Il costo è basso e conosciuto.** Ogni `.cpp` di `app/src` e di `app/tests`
  passa il controllo sintattico con mingw-w64 (`-fsyntax-only`, dipendenze alle
  versioni fissate), sia su `develop` sia sul ramo del code graph. ImGuiDot,
  compreso Graphviz con flex e bison, supporta già MSVC.
- Le modifiche di Dario sono servite a farla partire. Qui vogliamo farle
  bene: configurazione dichiarata, home risolta in un punto solo, percorsi in
  UTF-8.

## Stato rilevato (2026-10-02)

Analisi statica più controllo con mingw-w64. MSVC non era disponibile.

### Bloccanti per la build

1. `app/CMakeLists.txt`: il dispatch piattaforma/compilatore finisce in
   `FATAL_ERROR "Platform ... not supported yet (Linux only)"`.
2. `app/CMakePresets.json`: tutti i preset hanno la condizione
   `hostSystemName == Linux`.
3. `app/cmake/compilers/`: c'è solo `gcc.cmake` (`-fPIC`, `-Wall -Wextra`,
   `DEVDASH_WERROR`). Manca il corrispettivo MSVC.

### Corretti solo in apparenza (la build passa, l'app no)

4. **Home.** `std::getenv("HOME")` in tre punti:
   - `services/config_resolver.cpp` (`ResolveWithDefaultGlobal`): senza home
     sparisce lo strato globale `~/.claude`;
   - `app/app.cpp` (`EnsureRuntimeDirs`): scaffold e snapshot disattivati,
     ed è il messaggio visto a schermo;
   - `ui/project_selector_panel.cpp`: cartella iniziale del dialogo "Browse".

   Su Windows `HOME` di solito non è impostata. La home è `%USERPROFILE%`, ed è
   lì che Claude Code tiene `.claude`.
5. **Codifica dei sorgenti.** Nove stringhe dell'interfaccia contengono `—` e
   `·`. Senza `/utf-8`, MSVC legge i sorgenti con la codepage di sistema e
   l'interfaccia mostra caratteri sbagliati.
6. **Percorsi come testo.** Ci sono 54 chiamate a `.string()` /
   `.generic_string()` e nessuna a `.u8string()`. Su Windows `.string()`
   converte nella codepage ANSI, mentre ImGui vuole UTF-8: un percorso come
   `C:\Users\Agnès\…` si vede storpiato o si perde. Vale anche al contrario:
   `std::filesystem::path(const char*)` su Windows interpreta il testo come
   ANSI, quindi un percorso UTF-8 che arriva da ImGui (`InputText`) o da SDL
   (dialogo cartelle, `SDL_GetBasePath`) va convertito esplicitamente.

### Da verificare su Windows

7. **Include transitivi.** La libreria standard di MSVC si porta dietro
   header diversi da libstdc++. Possono mancare `#include` che su GCC arrivano
   per caso. mingw non lo rileva.
8. **Symlink degli scaffold** (ADR-013, modalità dev): su Windows servono la
   modalità sviluppatore o i diritti di amministratore. Il percorso di
   produzione (copia) non ne ha bisogno.
9. **Install e pacchetto:** `GNUInstallDirs`, il file `.desktop` e il
   generatore CPack `TGZ` sono pensati per Linux.

## Design

### Home: un solo punto di risoluzione

La home si risolve **una volta**, nella composition root (`app/`), con un helper
di `platform/` basato su `SDL_GetUserFolder(SDL_FOLDER_HOME)`. Su Windows SDL usa
`SHGetKnownFolderPath(FOLDERID_Profile)`, cioè la stessa cartella di
`%USERPROFILE%`. Su Linux legge solo `$HOME` (verificato nel sorgente di SDL
3.2.20, `filesystem/unix/SDL_sysfilesystem.c`). Il risultato è testo UTF-8 con
il separatore finale, e va trasformato in `std::filesystem::path` passando da
`char8_t` (punto 6).

Il percorso risolto viene iniettato:

- in `services::ConfigResolver`, nel costruttore. `ResolveWithDefaultGlobal`
  sparisce o diventa un sottile involucro attorno a `Resolve(project, homeDir)`,
  che esiste già e i test usano già;
- in `EnsureRuntimeDirs` e nel pannello di selezione del progetto.

In questo modo `services/` non dipende da SDL (ADR-010) e nessun livello legge
più l'ambiente da solo. Su Linux il comportamento resta quello di oggi: con
`HOME` non impostata la home manca e il messaggio di avvio resta. Un ripiego
su `getpwuid` sarebbe un'aggiunta separata, da fare solo se serve.

### Percorsi e testo UTF-8

Due helper, per esempio in `core/` o in `services/adapter_utils.h`:

- `std::string ToUtf8(const std::filesystem::path&)`, basato su `u8string()`;
- `std::filesystem::path FromUtf8(std::string_view)`.

Tutte le conversioni percorso ↔ testo che toccano ImGui, SDL o i file letti
passano da lì. Su Linux sono identiche a `.string()`, quindi niente cambia nel
comportamento attuale.

### Toolchain

- `app/cmake/compilers/msvc.cmake`: `/W4` (e `/WX` con `DEVDASH_WERROR`) sul
  target `dev-dash-warnings`, `/utf-8` su **tutti** i nostri target (anche
  `imgui_md`, che compila le nostre stringhe dentro `MarkdownRenderer`),
  `_CRT_SECURE_NO_WARNINGS` per `std::getenv` se ne resta qualcuno.
- Il dispatch in `app/CMakeLists.txt` diventa `if(LINUX) … elseif(WIN32 AND
  MSVC) … else() FATAL_ERROR`. Il rifiuto delle piattaforme non supportate
  resta esplicito.
- Preset `windows-base` / `windows-debug` / `windows-release` con condizione
  `hostSystemName == Windows`, generatore Ninja (da Developer PowerShell) e
  `cl` come compilatore.
- Prerequisiti documentati: Vulkan SDK (per `find_package(Vulkan)`),
  winflexbison (per Graphviz, come indica ImGuiDot), CMake ≥ 3.28, Ninja.

## Work Breakdown

### Fase 1 — Correttezza su tutte le piattaforme (si può fare da Linux)

- [ ] Helper `platform::` per la home via SDL; iniezione in `ConfigResolver`,
      `App::EnsureRuntimeDirs`, `ProjectSelectorPanel`. Nessun `getenv("HOME")`
      rimasto nel codice.
- [ ] Helper `ToUtf8` / `FromUtf8`; sostituzione delle conversioni
      percorso ↔ testo nei punti che toccano ImGui e SDL.
- [ ] Test: `ConfigResolver` con home iniettata (in gran parte già coperto da
      `Resolve(project, homeDir)`), round-trip UTF-8 con un percorso non ASCII.
- [ ] Controllo con mingw-w64 (`-fsyntax-only`) ancora pulito.

### Fase 2 — Toolchain Windows

- [ ] `msvc.cmake`, dispatch, preset Windows.
- [ ] README: sezione di build per Windows con i prerequisiti, e "Linux only"
      riformulato.
- [ ] Prova reale su Windows. Dario, o una macchina virtuale: build, avvio,
      progetto aperto, strato globale visibile, scaffold e snapshot attivi.

### Fase 3 — Facoltativa

- [ ] Job CI `windows-latest` (build e test, senza release).
- [ ] Pacchetto (CPack `ZIP`) e risoluzione asset accanto all'eseguibile.
      Si decide solo se ci sarà chi lo usa davvero.

## Acceptance Criteria

- Su Windows 10/11 con MSVC e Vulkan SDK, il preset `windows-debug`
  configura, compila e passa i test.
- L'app parte senza messaggi di avvio su `$HOME`; lo strato globale
  `%USERPROFILE%\.claude` compare nella configurazione effettiva;
  `%USERPROFILE%\.devdash\{scaffolds,snapshots}` vengono creati.
- Un progetto in una cartella con caratteri non ASCII si apre, e il suo percorso
  si vede correttamente nella sidebar e nella status bar.
- Su Linux build, test e comportamento restano identici.

## Out of Scope

- macOS: la stessa struttura lo renderebbe un passo piccolo, ma non c'è un
  trigger.
- Installer Windows (MSI, NSIS, winget) e firma del codice.
- Compilatori diversi da MSVC su Windows (mingw, clang-cl), salvo quello che
  viene gratis.

## Open questions

- **Dove sta `.devdash` su Windows?** Accanto a `.claude` nella home, per
  simmetria con Claude Code e per non avere due casi nel codice, oppure in
  `%APPDATA%`, come vorrebbero le convenzioni di Windows? Proposta: home, finché
  nessuno se ne lamenta.
- **Le modifiche di Dario:** chiedergli il diff. Dice cosa ha davvero
  incontrato (soprattutto il punto 7) e fa da caso di prova.

## Related

- [ADR-011](../../reference/decisions/011-release-and-distribution.md):
  Linux-first, crescita a scalini
- [ADR-006](../../reference/decisions/006-desktop-vs-vscode-extension.md),
  addendum 2026-06-10: la forma di distribuzione si decide al primo trigger
  concreto
- [ADR-010](../../reference/decisions/010-architecture-design.md): confini dei
  layer (la home si risolve in `app/` con `platform/`, non in `services/`)
