# ADR-008: Pivot dello stack — da C#/Avalonia a C++/Dear ImGui

**Data**: 2026-05-10
**Status**: Accettata
**Supersedes**: [ADR-001](001-stack-tecnologico.md)

## Contesto

[ADR-001](001-stack-tecnologico.md) aveva scelto **C# + Avalonia** come stack per
DevDash, con motivazioni allora valide: linguaggio già conosciuto dall'utente,
filesystem nativo via `System.IO`, MVVM via CommunityToolkit, distribuzione
cross-platform.

Tra dicembre 2025 e aprile 2026 il progetto è progredito su questo stack fino
alla v0.2.x: workspace portabile, sidebar multi-pannello, detection della
struttura `.rules/` / `.memory-bank/` / `.personal/`, rimozione del terminale
embedded ([ADR-007](007-rimozione-terminale-embedded.md)). L'app era funzionante
ma ancora largamente skeleton — gran parte delle feature di valore (rendering
markdown, kanban issue, effective-config view, code graph) erano da costruire.

Nel periodo aprile 2026 sono cambiate due cose contemporaneamente:

1. **Avalonia ha rilasciato il proprio modello commerciale** (effettivo da
   13 aprile 2026). Il framework core resta MIT, ma i DevTools (debugger UI
   interattivo) e l'estensione Visual Studio sono ora dietro un account portal
   — gratuito per uso non-commerciale ("Community tier") ma con registrazione
   obbligatoria sul portale Avalonia, soggetto ai loro Master Terms. Per
   debuggare un binding rotto in modo non-banale, di fatto, serve un account.

2. **Era disponibile un kickstart concreto in C++**: il progetto **Germen
   Pulchrum** del collaboratore Dario (DPD85) — un'app desktop C++/Dear ImGui
   già strutturata con SDL3 + Vulkan + vk-bootstrap, font system, theming,
   markdown rendering. Dario ha confermato disponibilità a continuare a
   evolvere la libreria e a coordinare scelte (PR, fix, scoperte) tra i due
   progetti.

Da queste due forze è nata l'opzione di un cambio di stack — non come
ripiego, ma come scelta che allineava meglio le motivazioni reali dell'utente
con uno strumento concreto disponibile.

## Decisione

**Riscrivere DevDash in C++20 con Dear ImGui (docking branch) su SDL3 + Vulkan**,
sostituendo lo stack .NET 8 + Avalonia. La transizione è stata validata da un
PoC in 4 step (aprile–maggio 2026) e dichiarata funzionalmente conclusa il
2026-05-04 (vedi memory-bank `2026-05-04-2400-cpp-poc-step4-complete.md`).

Stack target:

| Componente | Scelta | Note |
| ---------- | ------ | ---- |
| Linguaggio | C++20 | `CMAKE_CXX_STANDARD 20`, no extensions |
| GUI | Dear ImGui 1.92.x (docking branch) | Pinned via CPM, vendored config in `external/imgui-config.h` |
| Window/input | SDL3 3.2.20 | Built from source via CPM (Debian 12 non ha `libsdl3-dev`) |
| Renderer | Vulkan 1.3 + vk-bootstrap 1.3.302 | Pinned a 1.3.x finché non si passa a host con header Vulkan 1.4 |
| Build | CMake ≥3.28 + Ninja, GCC | Linux-only nel PoC, dispatch in `cmake/compilers/` |
| Package manager | CPM (CMake Package Manager) | No Conan, no system packages per dipendenze GUI |
| Markdown | imgui_md + MD4C | Vedi [ADR-009](009-markdown-library-imgui-md.md) |

Il codebase `.NET/Avalonia` legacy in `src/DevDash/` viene preservato sotto il
tag git `legacy/avalonia-final` e poi rimosso da `develop` in una sessione
dedicata. Il PoC vive in `poc/` finché il progetto vero non parte; a quel
punto verrà promosso a `app/` (o equivalente) con la struttura definitiva.

## Rationale

Le motivazioni del pivot sono — in ordine di peso reale — **personali**,
**pragmatiche**, e **strategiche**.

### 1. Personale: preferenza linguistica + rifiuto degli user agreement

L'utente preferisce C++ a C# come linguaggio per lavoro quotidiano, dopo anni
di C++ in ambito legacy (aviazione, C++98/11/14). DevDash è un personal tool,
e per un personal tool il linguaggio in cui si è felici di scrivere è una
variabile di prima classe — non un dettaglio.

Parallelamente, il modello commerciale di Avalonia introdotto ad aprile 2026
richiede registrazione su un portale (anche per il Community tier gratuito) per
accedere ai DevTools, soggetta ai Master Terms del vendor. È un legame
contrattuale che l'utente non vuole accettare per uno strumento personale.
Le alternative — usare solo l'estensione FOSS legacy o sviluppare senza
DevTools — peggiorano la developer experience per evitare il legame.

C++/Dear ImGui non ha equivalenti: Dear ImGui è MIT, SDL3 è zlib, Vulkan
SDK è open. Nessun account, nessun portale, nessun EULA da accettare per
toolchain di base.

### 2. Pragmatica: kickstart Germen + co-evoluzione con Dario

Il progetto Germen Pulchrum di Dario era un'app C++/Dear ImGui già
funzionante (Vulkan setup via vk-bootstrap, font system, theming, init
markdown rendering) — non una libreria astratta ma un sorgente concreto da
cui copiare strutture verificate. Strategia di consumo decisa: **(A) copia
con attribuzione** per ora, **(C) git subtree** in tasca per dopo se Germen
evolverà attivamente. Niente fork, niente dipendenza submodule.

Questo abbatte sostanzialmente l'effort di ripartenza: invece di re-imparare
Vulkan + SDL3 + ImGui da capo, c'è un'implementazione di riferimento da cui
mappare cosa serve a DevDash, cosa scartare (i18n, ImPlot, sistema temi
elaborato, status bar custom, DPI scaling, `nlohmann_json`), cosa rivedere.

Dario è disposto a continuare a evolvere Germen e a coordinare scoperte e
fix bidirezionalmente. La PR #43 di mgerhardy su `imgui_markdown` è un primo
esempio concreto di questa coordinazione (segnalata da DevDash a Germen).

### 3. Strategica: differenziazione e code graph rendering

Una nota d'analisi competitiva interna (vedi
`.personal/business/analysis/analysis.md`) osserva che il pivot da .NET/Avalonia
è difendibile solo se il nuovo stack *spedisce materialmente più veloce* o
*sblocca feature* che i competitor non possono pareggiare — in particolare
**code graph rendering at scale**, dove un renderer immediate-mode su Vulkan
ha una superficie di ottimizzazione che un'app retained-mode su Avalonia
non offre.

Questa motivazione è valida ma **non è il motore del pivot** — è un
allineamento favorevole. Se le prime due fossero state assenti, la sola
strategica non avrebbe giustificato la riscrittura.

### Motivazioni considerate e non riscontrate

L'utente non ha incontrato in pratica i limiti tecnici di Avalonia che
spesso motivano pivot di stack: niente problemi di performance, di binding,
di rendering, di distribuzione cross-platform. Avalonia avrebbe potuto
portare DevDash a v1 senza ostacoli tecnici. Il pivot non è una *fuga* da
Avalonia, è una *attrazione* verso C++/ImGui.

## Conseguenze

### Pro

- ✅ **Allineamento tra strumento e preferenze**: l'utente lavora nel
  linguaggio che preferisce su un toolchain interamente FOSS senza account.
- ✅ **Co-evoluzione con Germen**: due codebase che si influenzano a vicenda,
  scoperte tecniche e fix bidirezionali (esempio: feedback su upstream
  `imgui_markdown` → PR #43, scelta di pivottare a `imgui_md`).
- ✅ **Performance e controllo nativi**: rendering immediate-mode, integrazione
  Vulkan diretta, surface di ottimizzazione futura per feature pesanti
  (code graph al primo posto).
- ✅ **Bundle size e footprint**: nessun .NET runtime, nessuna shared library
  pesante; binario nativo statico-friendly.

### Contro

- ❌ **Costo della riscrittura**: la v0.2.x in Avalonia (workspace, sidebar,
  models, services, ViewModels, scaffold service) viene scartata. Mitigazione:
  **logica di business minima** — la maggior parte dei file `.NET` era ancora
  skeleton, non c'è grande IP perso. Le decisioni di prodotto (struttura
  workspace, modello `.personal/`/`.rules/`, philosophy "documentation +
  context, not execution") restano valide e si trasferiscono al nuovo stack.
- ❌ **C++ ha tooling più fragile** rispetto a .NET per refactoring,
  IDE-completion, testing. Mitigato da `compile_commands.json` (CMake export
  abilitato), VS Code + clangd, GDB launch config in repo.
- ❌ **Contraddice ADR-001**: la storia degli ADR mostra ora un cambio di
  stack a metà progetto. Documentato qui con il **Supersedes** esplicito
  per evitare narrazioni ambigue future.
- ❌ **Più piattaforme da gestire manualmente**: Avalonia astrae
  Linux/Windows/macOS; in C++/SDL3/Vulkan ogni piattaforma è un porting
  separato. Mitigazione: oggi Linux-only (`if(LINUX)` esplicito in CMake),
  Windows e macOS sono tracce separate per il futuro.

### Impatti

**Codice:**
- Nuovo albero `poc/` con `cmake/`, `external/` (CPM-managed), `src/`, `assets/`.
- Codebase legacy `.NET 8 + Avalonia` (`src/DevDash/`) preservato sotto tag
  `legacy/avalonia-final` e da rimuovere da `develop` con branch dedicato.
- CI .NET disabilitata: `.github/workflows/ci.yml` → `ci.yml.disabled` (commit
  `4a69bfb`, branch `chore/disable-dotnet-ci-pre-pivot`). Da rimpiazzare con
  workflow CMake/GCC quando il progetto vero parte.

**Documentazione:**
- [ADR-001](001-stack-tecnologico.md) marcato come **Superseded by ADR-008**.
- `CURRENT-STATUS.md` riallineato (ferma a 2026-01-01 fino a questa sessione).
- `coding-standards.md` esteso con sezione **C++ Conventions** (PoC under `poc/`),
  inclusa la regola di traduzione delle borrowed identifiers da Germen.
- Hook `02-dotnet-format` da rimpiazzare con `clang-format` quando il progetto
  vero parte.
- Permissions `Bash(dotnet build:*)` / `Bash(dotnet test:*)` in `.claude/settings.json`
  da sostituire con `cmake`/`ninja`/`make`.

**Dipendenze:**
- Rimosse: tutto il pacchetto NuGet (Avalonia.Desktop, CommunityToolkit.Mvvm,
  LiveMarkdown.Avalonia, Pty.Net — quest'ultimo già rimosso in ADR-007).
- Aggiunte (via CPM): SDL3 3.2.20, vk-bootstrap 1.3.302, Dear ImGui 1.92.6-docking,
  MD4C 0.5.3, imgui_md (vendored, MIT, Dmitry Mekhontsev).
- Sistema (system packages): Vulkan SDK headers, X11/Wayland devel headers
  (per build SDL3 da sorgenti).

**Roadmap:**
- Le feature pianificate pre-pivot (Markdown Rendering, Issue Management,
  Claude Context Panel) restano valide come *cosa* va costruito; cambia il
  *come* (controlli ImGui custom invece di Views Avalonia + ViewModels).
- L'**Effective Configuration View** — identificato in
  `.personal/business/analysis/analysis.md` come moat principale di DevDash —
  resta la wedge feature da implementare per prima nel progetto vero.

## Note per il futuro

### Cosa NON portare da Germen

Decisione confermata nei memory-bank handoff (Step 4): non importare da
Germen il sistema temi elaborato, il font 4-stack, ImPlot, i18n, status
bar custom, DPI scaling, `nlohmann_json`. Il font system di Dario (embedded
header C++ via `BinaryToCompressedC`) è un'alternativa al nostro approccio
file TTF — valutare per l'app vera se si vuole zero dipendenza da filesystem.

### Conventions per borrowed code

Le convenzioni C++ di DevDash (vedi `.claude/rules/coding-standards.md`,
sezione *C++ Conventions*) **non preservano lo stile del sorgente**: gli
identifiers italiani di Germen vanno tradotti in inglese e ri-stilati alle
convenzioni DevDash sulla via dell'import. L'attribuzione vive negli header
e in `THIRD_PARTY_NOTICES.md`, mai nei nomi dei simboli.

### Quando il progetto vero parte

Il PoC è `poc/`. Quando il progetto vero parte, va deciso:

- Rinomina `poc/` → `app/` (o equivalente).
- Cancellazione `src/DevDash/` su `develop` (con preservazione del tag
  `legacy/avalonia-final`).
- Struttura cartelle definitiva, equivalente MVVM in C++ (separazione
  domain / view / service o pattern alternativo da decidere).
- CMake definitivo: targets, install rules, packaging.

Questa è una decisione architettonica grossa che merita la sua propria
sessione e potenzialmente un **ADR-010**.

## Related ADRs

- [ADR-001](001-stack-tecnologico.md) — Superseded by this ADR.
- [ADR-006](006-desktop-vs-vscode-extension.md) — Resta valido: il pivot a
  C++/ImGui riguarda l'implementazione *desktop*; l'eventuale futura
  estensione VS Code resta in TypeScript come da ADR-006.
- [ADR-007](007-rimozione-terminale-embedded.md) — Resta valido: la decisione
  di non emulare un terminale è ortogonale allo stack.
- [ADR-009](009-markdown-library-imgui-md.md) — Sub-decisione del pivot:
  scelta della libreria markdown nel nuovo stack.

## Riferimenti

- PoC commits: `3825a6b` (Step 1), `2149ca4` (Step 4b), `c643eb2` (migrazione
  imgui_md), merge su `develop` `8641173` / `bbcac65` / `959a7c2`.
- Memory-bank: `2026-04-27-2030-cpp-poc-step1-and-cleanup.md`,
  `2026-05-03-1732-cpp-poc-step2-step3.md`,
  `2026-05-04-2400-cpp-poc-step4-complete.md`,
  `2026-05-09-2301-imgui-md-migration.md`.
- Tag legacy: `legacy/avalonia-final`.
- Avalonia commercial model: `https://avaloniaui.net/pricing`,
  `https://avaloniaui.net/legal/12` (effettivo dal 13 aprile 2026).
- Germen Pulchrum: `https://github.com/DPD85/Germen`.
