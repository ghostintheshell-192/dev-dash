# ADR-011: Modello di release e distribuzione

**Data**: 2026-06-10
**Status**: Proposta (draft per review)

## Contesto

La wedge è completa (3/3) e lo stack C++20/ImGui/SDL3/Vulkan è validato, ma il
progetto è uno *skeleton funzionante, non un programma distribuibile*: l'audit
del 2026-06-10 ha rilevato che mancano interamente install target, packaging,
versioning, processo di release, CI e test target. L'obiettivo dichiarato del
progetto è "un programma autocontenuto che, una volta finito, consente una
semplice installazione per essere usato subito".

Vincoli ereditati dalle decisioni precedenti:

- Linux-first (ADR-008): Debian 12 è la piattaforma di riferimento; nessun
  requisito Windows/macOS per ora.
- Tool personale-first, ma con **ambizione di distribuzione**: lo stack
  ImGui/SDL3 è stato scelto anche per la portabilità multi-OS. Le decisioni
  di oggi devono essere minime ma *scalabili*: aggiungere OS o installer in
  futuro deve essere additivo, non una ristrutturazione.
- Funzionalità minimalism: la soluzione più semplice che rende il programma
  installabile, non una pipeline enterprise.

## Decisione

### 1. Versioning

**SemVer con tag `v*` su `main`.** `main` resta "releases only" (workflow
esistente); ogni release è un merge `develop` → `main` + tag annotato
`vX.Y.Z`. La versione è dichiarata in un unico punto: `project(... VERSION
X.Y.Z)` nel `CMakeLists.txt` root di `app/`, esposta al codice via
`configure_file` (header `version.h` generato). Niente file `VERSION`
separato: il build system è la fonte di verità.

Un `CHANGELOG.md` al root, formato Keep-a-Changelog, aggiornato a mano alla
release. Nice-to-have dichiarato: generazione automatica con **git-cliff**
(già usato con soddisfazione in sheet-atlas), da introdurre quando il volume
di release lo giustifica — i commit message convenzionali già in uso lo
rendono possibile in qualunque momento.

### 2. Install target

`install()` rules in CMake:

- binario → `${CMAKE_INSTALL_BINDIR}` (`bin/`)
- asset (font) → `${CMAKE_INSTALL_DATADIR}/dev-dash/assets/` (`share/dev-dash/assets/`)
- desktop entry → `${CMAKE_INSTALL_DATADIR}/applications/dev-dash.desktop`
- icona → `${CMAKE_INSTALL_DATADIR}/icons/hicolor/...` (quando esisterà un'icona)

Uso di `GNUInstallDirs`. Il default `CMAKE_INSTALL_PREFIX=/usr/local` va bene;
l'installazione utente tipica è `cmake --install build --prefix ~/.local`.

**Risoluzione asset a runtime a due tentativi**: prima il path adiacente
all'eseguibile (`SDL_GetBasePath()/assets/`, copre build tree e bundle
portable), poi il path installato (`../share/dev-dash/assets/` relativo al
binario). Nessun path assoluto hardcoded — questo elimina anche il fallback
DejaVu su path Debian (`font_library.cpp:33`): DejaVu Sans viene **bundled
negli asset** (licenza libera, lo permette) invece di cercato nel sistema.

### 3. Packaging

**CPack con generator `TGZ`** come primo formato: un tarball
`dev-dash-X.Y.Z-linux-x86_64.tar.gz` con layout `bin/ + share/`, prodotto da
`cpack` dopo il build Release.

CPack è scelto proprio perché è *generator-based*: il percorso di crescita è
additivo, senza toccare le install rules. In ordine plausibile: AppImage o
`.deb` (Linux), NSIS o InnoSetup via step dedicato (Windows), DragNDrop/
productbuild (macOS). Lato stack, ImGui/SDL3 girano su Windows e macOS; su
macOS Vulkan passa da MoltenVK — fattibile, da validare quando sarà il
momento. Le install rules e la risoluzione asset (§2) sono già scritte per
funzionare su prefix arbitrari, che è il prerequisito comune a tutti gli
installer.

Nota onesta sul "self-contained": SDL3 è già statico (build from source via
CPM), ma il binario resta dinamicamente linkato a Vulkan loader e libc di
sistema. Accettabile per la fase attuale; il packaging davvero portabile
(AppImage) è il primo passo del percorso di crescita quando si punterà a
distribuire ad altri utenti.

### 4. Primo avvio

`App::Init()` crea `~/.devdash/scaffolds/` e `~/.devdash/snapshots/` se
assenti (`std::filesystem::create_directories`, errori loggati e mostrati
nella UI, non silenziati). Nessun wizard: la UI esistente (project selector)
è già il primo avvio. Un seed di scaffold di default è fuori scope (gli
scaffold sono dati personali dell'utente).

### 5. Test

**Catch2 v3 via CPM**, target `dev-dash-tests` separato dall'eseguibile,
registrato con `ctest`. I servizi si testano con fixture su directory
temporanee reali (coerente con "concrete services, no virtual interfaces" di
ADR-010). Primo modulo coperto: `SnapshotService` (save/list/restore/prune),
poi `ApplyEngine` e `ConfigResolver`.

Alternativa considerata: doctest (più leggero) — scartato perché Catch2 v3 ha
fixture e generatori più maturi e altrettanto semplice da consumare via CPM.

### 6. CI

GitHub Actions, **un solo workflow `ci.yml`** su push/PR verso `develop` e
`main`: configure + build (preset Release) + `ctest`. Il job usa una matrice
OS con una sola entry (`ubuntu-latest`) — oggi è equivalente a un job
singolo, ma aggiungere Windows/macOS domani è una riga, non un rewrite
(stessa forma della CI di sheet-atlas).
Il check di formato entra nel workflow solo quando esisterà `.clang-format`
(oggi assente, hook dormiente).

**Un workflow `release.yml`** su tag `v*`: build Release + `cpack` + GitHub
Release con il tarball allegato. `ci.yml.disabled` (era .NET) viene eliminato.

I passi di build/test della CI invocano gli **entry point standard di
progetto** definiti in ADR-012, non comandi cmake inline — così il workflow
YAML resta agnostico rispetto allo stack.

## Conseguenze

- Il progetto acquisisce la fase "da skeleton a prodotto" che mancava nel
  piano; la spec operativa `feature-release-readiness` traccia il lavoro.
- Ogni release su `main` è installabile con due comandi (`cmake --install` o
  unpack del tarball) su una macchina Linux con driver Vulkan.
- Il bundling di DejaVu aumenta il peso del repo/asset (~700KB) in cambio di
  rendering deterministico su qualunque distro.
- La CI fa da rete di sicurezza che oggi non esiste (nessuna build automatica
  dal pivot).

## Alternative considerate

- **AppImage subito**: massima portabilità, ma tooling in più (linuxdeploy) e
  zero beneficio finché l'unica macchina target è quella di sviluppo.
- **Niente install target, solo build tree**: status quo; contraddice
  l'obiettivo dichiarato del progetto.
- **GoogleTest**: più diffuso ma più pesante di Catch2 per un progetto CMake
  + CPM; nessun bisogno di mock framework (niente interfacce virtuali).
