# ADR-012: Automazione a due livelli, agnostica rispetto al codebase

**Data**: 2026-06-10
**Status**: Proposed
**Impact**: high
**Sommario**: Riduce l'automazione a due livelli (globale Claude + progetto self-contained, decommissionando il livello workspace) e separa orchestrazione e implementazione: gli hook diventano orchestratori generici e agnostici rispetto allo stack, mentre la conoscenza stack-specifica vive in entry point standard sotto `.development/automation/`.

## Contesto

L'infrastruttura di automazione attuale è il residuo di un sistema a tre
livelli (globale → workspace → progetto) costruito nel tempo: funzionava ma
era rigido, fragile, e rendeva opaca la gestione della documentazione.
L'audit del 2026-06-10 ha fotografato lo stato:

- `core.hooksPath` in `~/.gitconfig` punta a `/data/repos/.git-hooks`
  (livello workspace): gli hook **di progetto** (`.githooks/`) non girano mai,
  inclusa la branch protection (tech-debt `project-githooks-not-active`).
- Il livello workspace contiene roba morta (`02-dotnet-format`, `pre-push`
  con `dotnet test`) e hook che invocano script inesistenti
  (`05-generate-readme-status` → `generate-readme-status.py`, mai esistito).
- Progetto e workspace duplicano quasi tutto, con piccoli delta mai
  riconciliati (dev-dash ha fix e hook in più, ma gli mancano due moduli).

Il problema di fondo: gli hook mescolano **orchestrazione** (quando girare,
cosa bloccare) e **implementazione specifica dello stack** (quale formatter,
quale test runner). Ogni nuovo progetto copia tutto e poi diverge.

## Decisione

### 1. Due livelli, niente workspace

Restano solo:

- **Globale Claude** (`~/.claude/`): profilo utente, agenti, convenzioni
  trasversali. Nessun git hook a questo livello.
- **Progetto**: tutto il resto. Ogni progetto è self-contained: hook, script,
  config Claude, documentazione. Aprire un progetto deve bastare per avere
  controllo totale su configurazione e documentazione *di quel progetto*.

Il livello workspace (`/data/repos/.git-hooks`, `core.hooksPath` globale)
viene decommissionato (vedi piano di migrazione).

### 2. Separazione interfaccia/implementazione: entry point standard

Gli hook diventano **orchestratori generici, identici in ogni progetto**, che
non conoscono lo stack. Ogni capability invocabile è un **entry point
standard**: uno script con nome e contratto fissi in
`.development/automation/`, implementato da ciascun progetto per il proprio
stack.

```text
.development/automation/
├── build.sh          # builda il progetto (exit != 0 = fallito)
├── test.sh           # esegue i test (exit != 0 = falliti; exit 0 + msg se nessun test)
├── format-check.sh   # verifica formato (no-op dichiarato se non configurato)
├── format-fix.sh     # applica il formato (opzionale)
└── docs-update.sh    # rigenera la doc derivata (ARCHITECTURE.md, INDEX.md, tech-debt index)
```

Contratto minimo: ogni entry point è eseguibile dalla root del repo, esce con
codice 0/!=0, stampa una riga di esito. Un entry point assente = capability
non offerta dal progetto (l'orchestratore salta senza errore). In dev-dash:
`build.sh` incapsula `cmake --build --preset ...`, `test.sh` incapsula
`ctest`, `format-check.sh` clang-format. In un progetto .NET sarebbero
`dotnet build` / `dotnet test` / `dotnet format` — **stessi hook, stessa CI,
implementazione diversa**.

**Linguaggi multipli**: l'orchestratore non fa mai dispatch per linguaggio —
non sa cosa c'è nel progetto. Tutta la conoscenza "quale comando per quale
stack" vive *dentro* l'entry point del progetto, che in un repo multi-stack
semplicemente concatena: un `test.sh` di un progetto C++ con tooling Python
fa `ctest ... && pytest tools/`. Se un domani serve granularità (es. testare
solo una parte), l'entry point accetta argomenti — il contratto resta "senza
argomenti = tutto".

Consumatori degli entry point:

- **Git hooks** (`.githooks/`): pre-commit chiama `format-check.sh` e
  `docs-update.sh`; gli hook *strutturali* (branch protection, security scan,
  spec-workflow) restano negli hook perché sono già agnostici.
- **CI** (ADR-011): i workflow YAML chiamano `build.sh` / `test.sh` — il
  workflow diventa copiabile tra progetti, cambia solo il setup
  dell'ambiente (apt deps, SDK).
- **Claude Code** (`.claude/settings.json` hooks): SessionStart/SessionEnd
  continuano a chiamare gli script doc (che `docs-update.sh` aggrega).

### 3. Attivazione per-clone esplicita

`git config core.hooksPath .githooks` resta il meccanismo di attivazione (git
non permette di attivare hook dal repo per ragioni di sicurezza, è una
feature). Per renderlo un gesto unico e documentato: uno script
`.development/automation/bootstrap.sh` che setta la config locale e verifica
i prerequisiti. La config **locale** del clone vince sulla globale — la
migrazione di un progetto non dipende dallo smontaggio del workspace.

### 4. Scaffolding come prodotto, non come copia

La struttura `.githooks/` + `.development/automation/` + `.claude/` diventa
parte dello scaffold DevDash di riferimento (gestito *da* DevDash con
scaffold-management): i progetti nuovi la ricevono via apply, e il diff
scaffold ↔ progetto rende visibili i delta che prima si accumulavano in
silenzio. Questo chiude il cerchio con la visione del progetto: DevDash è lo
strumento con cui questa configurazione si vede e si governa.

## Piano di migrazione

1. **dev-dash** (subito): creare `.development/automation/` con gli entry
   point; ridurre gli hook a orchestratori; completare i moduli mancanti
   (`docs-update` assorbe `generate-index.py` che oggi gira solo via
   SessionStart); `bootstrap.sh`; attivare `core.hooksPath .githooks` locale.
2. **sheet-atlas** (quando deciso): stessa struttura, entry point .NET.
   Nessuna urgenza: continua a funzionare con gli hook workspace finché non
   migra.
3. **Decommissionamento workspace** (ultimo): quando tutti i progetti attivi
   sono migrati, rimuovere `core.hooksPath` da `~/.gitconfig` ed eliminare
   `/data/repos/.git-hooks/`. Fino ad allora resta, ma congelato: niente
   nuovi moduli a livello workspace.

## Conseguenze

- Un progetto nuovo si attiva con: apply dello scaffold + `bootstrap.sh` +
  implementazione degli entry point per il suo stack. Niente più copia-e-divergi.
- Gli hook orchestratori sono identici ovunque → aggiornarli è un diff dello
  scaffold, visibile in DevDash.
- Il `pre-push` workspace (dotnet test) sparisce: il suo ruolo lo prende la
  CI (ADR-011); un pre-push locale opzionale può chiamare `test.sh` nei
  progetti dove i test sono veloci.
- Costo: ogni progetto deve mantenere i propri entry point (poche righe di
  shell ciascuno) e la branch protection torna attiva solo dopo il bootstrap
  per-clone.

## Alternative considerate

- **Manifest dichiarativo** (`automation.toml` letto dagli hook): più
  "elegante" ma aggiunge un parser e un formato da imparare; gli script con
  nomi standard sono il manifest — eseguibili, ispezionabili, zero parsing.
- **Justfile/Makefile come interfaccia**: stessa idea, ma introduce una
  dipendenza tool (just) o una sintassi (make) per fare da semplice
  dispatcher; bash è già ovunque negli hook esistenti.
- **Tenere il livello workspace come "libreria" di hook condivisi**: è lo
  status quo che ha generato la confusione attuale; contraddice il requisito
  di progetti self-contained.
