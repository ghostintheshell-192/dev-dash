# ADR-010: Architettura del progetto vero — split layered

**Data**: 2026-05-10
**Status**: Accepted
**Impact**: critical
**Sommario**: Struttura il progetto vero come split layered a cinque cartelle (`core/services/ui/platform/app/`, promozione di `poc/` → `app/`), con pattern "panel as viewmodel" invece di MVVM, classi concrete invece di interfacce virtuali, e callback con ordine di dichiarazione per il `MarkdownRenderer`.
**Sub-decisione di**: [ADR-008](008-pivot-to-cpp-imgui.md) (pivot a C++/Dear ImGui)

## Contesto

ADR-008 ha sancito il pivot da .NET/Avalonia a C++20 + Dear ImGui (SDL3 + Vulkan). Il PoC sotto `poc/` ha validato lo stack ed è funzionalmente completo. La domanda aperta lasciata da ADR-008 era: come strutturare il progetto vero?

Tre vincoli convergenti hanno guidato la decisione:

1. **Wedge feature definita** in tre spec planned (`feature-effective-config-view.md`,
   `feature-scaffold-management.md`, `feature-snapshot-history.md`). La struttura del
   codice deve permettere di aggiungere ognuno di quei tre pannelli + i motori di
   servizio sottostanti **senza rivoluzionare l'esistente**.

2. **`Renderer` PoC è una god class** (~825 righe). Mescola plumbing SDL3/Vulkan,
   init ImGui, main loop, gestione panel markdown, file I/O. Va spaccata.

3. **Dear ImGui è immediate-mode**. MVVM — il pattern del codebase legacy
   `.NET/Avalonia` — non si trasferisce 1:1: non esistono "Views" come oggetti
   retained con binding lifecycle.

## Decisione

### 1. Layout layered con cinque cartelle

Sotto `app/src/` (rinomina di `poc/src/`):

```
core/      — modelli puri (data types), no dipendenze esterne
services/  — logica dominio (file I/O, resolver, diff/apply, snapshot)
ui/        — panel-classes ImGui, font library, markdown renderer
platform/  — SDL3/Vulkan/ImGui plumbing
app/       — composition root + main loop
```

Promozione di `poc/` → `app/` come cambio di nome esplicito.

### 2. "Panel as viewmodel" invece di MVVM

Niente layer "ViewModel" separato. Ogni panel-class in `ui/`:

- è disegnata come una classe ImGui normale (`Render()` chiamato per frame)
- tiene il proprio stato locale
- riceve servizi via dependency injection costruttoriale, per riferimento

Il "ViewModel" e la "View" coincidono per costruzione. Tentare di forzare una
separazione retained-style introduce scaffolding artificiale in immediate-mode.

### 3. Concrete classes invece di virtual interfaces

I servizi (`DocumentLoader`, `ConfigResolver`, `DiffEngine`, `ApplyEngine`,
`SnapshotService`, `ScaffoldRepository`) sono **classi concrete**, non interfacce
astratte. Test usano fixture su filesystem reale (tmpdir), non mock.

Promozione a virtual interface solo se emerge un seam concreto richiesto (es. mock
di un servizio remoto, plugin pattern). Nessuna interfaccia speculativa.

### 4. Pattern callback per `MarkdownRenderer`

`MarkdownRenderer` espone `SetLinkHandler(std::function<void(std::string_view)>)`.
Default: apri URL esterno via `SDL_OpenURL`. `DocumentPanelHost` si registra come
handler nel proprio costruttore per intercettare i `claudeimport://` link.

Il problema lifetime (callback con `[this]` su `DocumentPanelHost` capturato dal
renderer) si risolve via **declaration order in `App`**: `DocumentPanelHost`
dichiarato prima, `MarkdownRenderer` dichiarato dopo. La distruzione automatica
LIFO per `unique_ptr` distrugge il renderer prima → callback non più invocabile →
host distrutto safe dopo.

### 5. `FontLibrary` come risorsa app-wide in `ui/`

I font (IBM Plex Sans + DejaVu fallback) sono una risorsa di **applicazione**, non
specifica al markdown. La wedge avrà multiple panel ImGui che useranno gli stessi
font.

`ui::FontLibrary` carica i font nell'atlas ImGui una volta sola, posseduto da
`App`. Sia `MarkdownRenderer` che future panel della wedge ricevono
`const FontLibrary&` per riferimento.

### 6. CMake: target singolo, sources organizzate per cartella

Un solo executable target `dev-dash`. Ogni sotto-cartella (`core/`, `services/`,
ecc.) ha il proprio `CMakeLists.txt` che aggiunge i file via
`target_sources(dev-dash PRIVATE ...)`. Promozione a static lib differita a
quando arriveranno i test (in cui `core` e `services` sono testabili in
isolamento e la lib-form è il modo idiomatico di linkare un test runner).

### 7. PCH

Un singolo `pch.h` a `app/src/pch.h` include:

- Std headers stabili (vector, string, filesystem, ecc.).
- Heavy third-party stabili: SDL3, Vulkan, vk-bootstrap, ImGui core.

Esclude: ImGui backend headers (`imgui_impl_*`), `imgui_md.h`, application
headers. Iniettato force-include via `target_precompile_headers`.

### 8. Namespaces

Root `dev_dash::` con sotto-namespace `dev_dash::core`, `dev_dash::services`,
`dev_dash::ui`, `dev_dash::platform`, `dev_dash::app`. Snake_case per namespace
(allinea con coding-standards).

### 9. File naming e convenzioni

`snake_case.h` / `snake_case.cpp`. Class `PascalCase`. Methods `PascalCase`.
Locals e params `camelCase`. **Public struct fields `camelCase`** (estende la
regola dei locals/params, allinea allo stile esistente di `MarkdownFonts`).
Private fields `_camelCase`. Constants ed enum values `kPascalCase`.

## Rationale

### Layered split (vs minimal refactor)

Tre opzioni pesate:

- **B**: rinomina solo, lascia `Renderer` monolitico → al primo panel wedge il
  refactor diventa parte della feature. Anti-pattern.
- **C**: split mid-ground (estrai solo MarkdownPanel/PreprocessImports, lascia
  Vulkan dentro `Renderer`) → buon compromesso ma `Renderer` resta loop-owner,
  ogni nuovo panel lo tocca.
- **A**: split completo da subito → ~ora di refactor mechanical upfront, dopo
  aggiungere feature è strettamente additivo.

Scelto **A**. La wedge richiede 3 nuovi panel + 6 nuovi servizi: la "tassa" di
B/C verrebbe pagata 3 volte. A paga una volta sola. Inoltre l'utente ha
esplicitato preferenza per pianificare la struttura completa upfront, coerente
con il proprio modo di lavorare.

### Panel as viewmodel (vs MVVM transcribed)

Avalonia/MVVM separa View (XAML) da ViewModel (proprietà bindabili). In ImGui non
esiste "View" come oggetto: per ogni frame il codice emette comandi di disegno.
Lo stato del panel vive direttamente nella classe-panel. Forzare un secondo
"ViewModel" parallelo non aggiunge nulla, raddoppia il numero di tipi.

### Concrete services (vs virtual interfaces)

Per servizi che fanno I/O su filesystem, mockare il filesystem è
over-engineering. Test su `tmpdir` (con `std::filesystem::temp_directory_path()`)
sono più realistici, meno fragili e non richiedono vtable. Promote a virtual
interface quando emerge un seam reale.

### Callback per `MarkdownRenderer` (vs ownership)

Tre alternative esaminate:

- **A**: vector di pending paths owned globalmente, MarkdownRenderer pusha
  direttamente — circolare nell'ownership.
- **B**: `DocumentPanelHost` possiede `MarkdownRenderer` — semplice ma il renderer
  è privato, non condivisibile.
- **C** (scelto): callback `std::function` impostato dall'host nel suo costruttore.

C separa concettualmente il "puro renderer" da "chi gestisce gli eventi link",
permette a future panel della wedge (es. `EffectiveConfigPanel` che mostrerà
markdown con badge di provenienza) di condividere lo stesso `MarkdownRenderer`.
Il rischio lifetime è gestito dal declaration-order trick — pattern semplice e
documentabile in un commento sopra i due membri di `App`.

### `FontLibrary` come `ui::`

Il font system è UI tipografica, non specifico al markdown. ImGui carica i font
una volta nell'atlas; tenerli encapsulati in `MarkdownRenderer` significherebbe
ricaricarli o duplicarli per ogni panel-class. Estrazione a `ui::FontLibrary`
prepara la wedge senza sforzo.

Posizionato in `ui/` (non `platform/`) perché i font sono UI tipografica;
`platform/` resta puro plumbing rendering.

### CMake target singolo (vs multi-lib)

Per un progetto pre-test-suite, splittare in 5 static lib (una per layer) è
ceremonia inutile. Un solo executable con `target_sources()` per cartella mantiene
la struttura concettuale senza overhead CMake. Promozione a static lib quando
arrivano i test.

## Conseguenze

### Pro

- ✅ **Aggiungere feature wedge è strettamente additivo**: nuova spec → nuovi file
  in `core/`, `services/`, `ui/` + 2-3 righe di wiring in `App`. Nessun layer
  esistente cambia.
- ✅ **`Renderer` god class scompare**: 5 classi platform focused, ognuna con un
  solo lifecycle concern.
- ✅ **`MarkdownRenderer` riusabile**: la wedge userà rendering markdown in più
  panel; con il pattern callback, tutti possono iscrivere il proprio handler.
- ✅ **Layer boundaries esplicite**: `platform/` non sa di `ui/`, `ui/` non sa di
  `platform/`. ImGui context lifecycle vive in `App` (composition root).
- ✅ **Construction order esplicito**: dependencies costruite via `make_unique`
  in `App` body, ordering documentato.

### Contro

- ❌ **Tassa upfront ~1h di refactor mechanical**: spostare `Renderer` in 5
  classi, riscrivere include, aggiornare CMakeLists. Costo una-tantum, ma esiste.
- ❌ **Declaration-order trick richiede commento**: il fatto che
  `_documentPanelHost` sia dichiarato prima di `_markdownRenderer` non è ovvio dal
  nome. Va commentato in `app.h`.
- ❌ **Concrete services significa test fixture su filesystem reale**: tmpdir per
  tutti i test = test più lenti rispetto a mock in-memory. Tradeoff accettato.
- ❌ **Multi-cartella aumenta verbosità di `#include`**: ogni file ha 3-5 include
  con path lunghi. Mitigazione: PCH copre i pesanti.

### Impatti

**Codice:**

- `poc/` rinominato `app/`. Tag `legacy/avalonia-final` resta come è.
- `Renderer` (poc) ricomposto in: `Window`, `VulkanContext`, `Swapchain`,
  `FrameResources`, `ImGuiBackend` (tutti `platform/`).
- `Rendering::MarkdownRenderer` → `dev_dash::ui::MarkdownRenderer`, font
  extraction in `ui::FontLibrary`.
- `Renderer::PreprocessImports` → `services::DocumentLoader::Load()`.
- `Renderer::OpenPanel` / `RenderMarkdownWindow` / `_panels` / `_pendingPanels`
  → `ui::DocumentPanelHost`.
- Nuovi: `core/{project,config_layer,effective_config,scaffold,snapshot,diff_entry}.h`.
- Nuovi: `services/{config_resolver,diff_engine,apply_engine,snapshot_service,scaffold_repository}.{h,cpp}`.
- `app/{app,sdl_session}.{h,cpp}` + `main.cpp` + `pch.h`.

**Build:**

- CMake target singolo, organizzazione per cartella.
- `target_precompile_headers` per il PCH.
- Asset copy POST_BUILD invariato.
- CI Linux/GCC come prossimo step (cleanup-post-pivot).

**Documentazione:**

- `.development/api-design.md` — design definitivo per implementare il refactor
  (compagno operativo di questo ADR).
- `docs/architecture.md` — riscrittura pubblica dopo questo ADR (è ancora stale,
  parla di Avalonia).
- `.development/ARCHITECTURE.md` (auto-mappa) — già aggiornato 2026-05-10.

## Note per il futuro

### Quando promuovere a virtual interface

Quando uno o più di:

- Un servizio remoto/HTTP che vuoi fakeare per dev offline.
- Un servizio con plug-in pattern (es. `ConfigResolver` con strategie multiple).
- Un servizio dipendente da hardware/network non testabile.

Mai promuovere per "potrebbe servire un test futuro" senza il test concreto in
vista.

### Quando dividere `platform/` ulteriormente

Le 5 classi platform coprono il PoC. Trigger noti per future divisioni:

- Offscreen render targets (code graph rendering pesante) → split di `Swapchain`
  in `Swapchain` + `OffscreenRenderTarget`.
- Async compute o multi-thread submit → `FrameResources` evolve a `RenderQueue`
  con fence pooling.
- Multi-window → `Window` evolve a `WindowManager`.

Non preventivare.

### Quando il "panel registry" diventa utile

Per ora `App::Run()` chiama hard-coded `_documentPanelHost->Render()`. Quando
arrivano i 3 panel della wedge:

- Se rimangono 3-4 panel statici → continua a chiamarli hard-coded.
- Se diventano configurabili dall'utente (layout salvato, panel apri/chiudi
  multipli dello stesso tipo) → introduci `IPanel` interface + registry.

Non promuovere preventivamente.

## Related ADRs

- [ADR-008](008-pivot-to-cpp-imgui.md) — Pivot dello stack. Questo ADR è la
  sub-decisione architetturale di quel pivot.
- [ADR-009](009-markdown-library-imgui-md.md) — Scelta della libreria markdown.
  Indipendente da questo ADR ma referenziata da `ui::MarkdownRenderer`.

## Riferimenti

- Spec wedge: `.development/specs/planned/feature-effective-config-view.md`,
  `feature-scaffold-management.md`, `feature-snapshot-history.md`.
- API design definitivo: `.development/api-design.md`.
- Memory-bank: `2026-05-10-1200-wedge-spec-and-pivot-adrs.md` (handoff che ha
  lasciato ADR-010 come prossimo step).
- Codice PoC pre-refactor: `poc/src/` (rimane invariato fino a sessione di
  refactor implementativo).
- Coding standards: `.claude/rules/coding-standards.md`, sezione
  **C++ Conventions**.
