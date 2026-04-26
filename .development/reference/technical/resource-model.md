# DevDash — Resource Model (DRAFT)

*Created: 2026-04-26 — In flux. Working artifact for the dev-dash redesign discussion.*

## Scope di questo documento

Modello uniforme per descrivere tutte le **risorse** che dev-dash deve esporre — sia config Claude Code (definite da Anthropic, bersaglio mobile) sia documentazione di progetto (definite dall'utente, convenzioni di scaffolding).

L'ipotesi architettonica sotto: dev-dash non è N feature pages, è **1 astrazione "risorsa-con-metadata-e-scope" × N adapter**. Ogni voce qui sotto è un adapter da scrivere.

---

## Schema delle colonne

| Colonna | Significato |
|---|---|
| **Tipo** | Nome breve della risorsa |
| **Scope** | `G` = utente/globale (`~/.claude/`) · `P` = progetto (`./` o `.claude/`). La maggior parte delle risorse usa solo `G` + `P`. **CLAUDE.md è speciale**: oltre a G+P ha anche managed policy (enterprise), ancestor directories (walk-up del tree), project-local (`CLAUDE.local.md` gitignored), subdirectory (on-demand). Vedi sezione A per il dettaglio. |
| **Path** | Pattern relativo allo scope root |
| **Formato** | markdown / markdown+frontmatter / json / dir / shell |
| **Postura** | `RO` read-only · `RM` read-mostly + quick-edit · `RW` read + full-edit |
| **Composizione** | `concat` · `override` · `union` · `n/a` (vedi sezione "Modalità") |
| **Origine** | `A` = Anthropic-defined · `U` = User scaffolding · `M` = Mixed |
| **Doc** | Link alla doc ufficiale (per origine A) |

### Modalità di composizione fra scope

- **`concat`** — *tutti* i livelli contribuiscono in catena, niente vince (può essere multi-livello, non solo G+P; vedi CLAUDE.md). Il rendering mostra ogni livello in ordine di concatenazione.
- **`override`** — gerarchia formale, livello più specifico vince sulla stessa chiave. Il rendering è un "merge resolver" che per ogni chiave mostra origine e vincitore.
- **`union`** — coesistenza con disambiguazione visuale per scope (badge `[G]`/`[P]`). Le collisioni sono individuate per **identità logica** (campo `name:` di frontmatter, chiave JSON), non solo per filename. Quando il runtime di Claude shadowa effettivamente uno (es. agents/skills, da verificare per tipo), badge `shadowed`. Per hooks la `union` è pura — tutti i livelli si attivano, nessuno shadowing, il badge è solo etichettatura della provenienza.
- **`n/a`** — risorsa esiste a un solo livello.

---

## A. Memoria di istruzioni — `[Anthropic]`

Cuore del sistema CLAUDE.md. È qui che Claude legge "come comportarsi". È anche la famiglia con la **catena di scope più complessa** dell'intero modello.

### A.1 — Catena CLAUDE.md (modalità `concat`, fino a 6 livelli)

Tutti i livelli che esistono vengono **concatenati** nel context all'inizio di ogni sessione, in ordine di specificità crescente. Niente vince, tutti contribuiscono. Vista dev-dash: chain visualizzata in ordine, ogni livello come pannello affiancato espandibile.

| Livello | Path | Scope | Postura | Note |
|---|---|---|---|---|
| Managed policy | `/etc/claude-code/CLAUDE.md` (Linux) · `/Library/Application Support/ClaudeCode/CLAUDE.md` (macOS) · `C:\Program Files\ClaudeCode\CLAUDE.md` (Win) | enterprise | `RO` | Org-wide, immutabile da utente. **Probabilmente fuori scope per uso personale.** |
| User | `~/.claude/CLAUDE.md` | G | `RW` | Tue preferenze trasversali ai progetti |
| Ancestor directories | `<ancestors>/CLAUDE.md` (walk-up del tree fino a cwd) | dynamic | `RW` | Es. `/data/repos/CLAUDE.md` se presente. Ogni livello dell'albero contribuisce. |
| Project | `./CLAUDE.md` *oppure* `./.claude/CLAUDE.md` | P | `RW` | Standard di progetto, committato |
| Project-local | `./CLAUDE.local.md` | P | `RW` | Override personale, gitignored. **Caricato dopo** project, quindi le tue note vincono in caso di conflitto a quel livello. |
| Subdirectory | `<subdir>/CLAUDE.md` o `<subdir>/CLAUDE.local.md` | dynamic | `RW` | **NON caricato all'avvio**. Si attiva on-demand quando Claude legge file in quel subdir. |

### A.2 — Altre risorse di memoria

| Tipo | Scope | Path | Formato | Postura | Composizione | Doc |
|---|---|---|---|---|---|---|
| Auto-memory MEMORY.md | G | `~/.claude/projects/<encoded-path>/memory/MEMORY.md` | md | `RW` | n/a | [memory#auto-memory](https://code.claude.com/docs/en/memory) |
| Auto-memory topic files | G | `~/.claude/projects/<encoded-path>/memory/<topic>.md` | md | `RW` | n/a | [memory#auto-memory](https://code.claude.com/docs/en/memory) |
| Path-scoped rules (user) | G | `~/.claude/rules/*.md` | md+frontmatter | `RM` | union | [memory#rules](https://code.claude.com/docs/en/memory) |
| Path-scoped rules (project) | P | `.claude/rules/*.md` | md+frontmatter | `RM` | union | [memory#rules](https://code.claude.com/docs/en/memory) |
| Agent memory (user) | G | `~/.claude/agent-memory/<agent>/MEMORY.md` | md | `RW` | n/a | [memory](https://code.claude.com/docs/en/memory) |
| Agent memory (project) | P | `.claude/agent-memory/<agent>/MEMORY.md` | md | `RW` | n/a | [memory](https://code.claude.com/docs/en/memory) |

### A.3 — Meccanismi trasversali

**`@path` imports**: un file CLAUDE.md può tirarne dentro altri con sintassi `@docs/git-instructions.md` o `@~/.claude/my-prefs.md`. Ricorsivo fino a 5 hop. Il content "effettivo" che Claude vede è file + import espansi. Dev-dash deve esporre **vista raw** + **vista expanded** (pulsante toggle) per evitare che l'utente debba mentalmente fare l'espansione.

**`claudeMdExcludes`** (in `.claude/settings.local.json` o ai vari livelli di settings): array di glob pattern che **escludono** specifici CLAUDE.md dall'essere caricati. Per mostrare correttamente "what Claude actually sees", dev-dash deve applicare questi exclude alla catena A.1 e marcare i file mutati come `excluded`.

**Auto-memory encoding**: il `<encoded-path>` di `~/.claude/projects/<encoded-path>/` è il path della repo root con `/` sostituiti da `-`. Per dev-dash: `/data/repos/dev-dash/` → `-data-repos-dev-dash`.

### A.4 — Note

- **Preferenza utente**: il sistema `.memory-bank/` (handoff sessioni) è preferito al sistema auto-memory di Anthropic per la continuità tra sessioni — vedi sezione F. Auto-memory resta `RW` perché è comunque utile poter editare a mano (es. cancellare una memoria sbagliata, correggerne una stale).
- **Path-scoped rules**: caricamento **condizionale** via frontmatter `paths:` (glob patterns). Senza `paths:` si caricano sempre. Lo "stato attivo" non è statico — dipende da cosa Claude sta leggendo. Per la vista dev-dash, esporre i `paths:` nei metadata e segnalare che il caricamento è dinamico.
- **Campi chiave per Path-scoped rule** (frontmatter): `paths` (lista di glob, opzionale).
- **`claudeMdExcludes` non vale per managed policy**: il livello enterprise è sempre attivo, non escludibile.
- **Symlink** sono supportati in `.claude/rules/`: due righe della tabella possono puntare al *fisicamente stesso file*. Edit propagato. Da segnalare nell'UI.

---

## B. Settings & permessi — `[Anthropic]`

Configurazione runtime di Claude Code. Modalità `override` formale.

### B.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Doc |
|---|---|---|---|---|---|---|
| settings.json (user) | G | `~/.claude/settings.json` | json | RM | override per chiave | [settings](https://code.claude.com/docs/en/settings) |
| settings.json (project, shared) | P | `.claude/settings.json` | json | RM | override per chiave | [settings](https://code.claude.com/docs/en/settings) |
| settings.local.json (project, gitignored) | P | `.claude/settings.local.json` | json | RM | override per chiave (precedenza più alta) | [settings](https://code.claude.com/docs/en/settings) |
| Permissions block | G + P | dentro `settings.json` (campo `permissions`) | json | RM | override + merge per categoria | [permissions](https://code.claude.com/docs/en/permissions) |
| Permission mode | G + P | dentro `settings.json` (campo `permissionMode`) | json | RM | override | [permission-modes](https://code.claude.com/docs/en/permission-modes) |
| Environment variables | G + P | dentro `settings.json` (campo `env`) | json | RM | override per chiave | [env-vars](https://code.claude.com/docs/en/env-vars) |
| Model config | G + P | dentro `settings.json` (`model`) | json | RM | override | [model-config](https://code.claude.com/docs/en/model-config) |

### B.2 — Note

- **Gerarchia di precedenza** (alta → bassa): managed (enterprise) > project local > project shared > user. Per dev-dash personale, ignoriamo "managed".
- **Merge resolver**: questa è l'unica famiglia che merita una vista "effective settings" tipo merge view (per ogni chiave: settato qui a X, qui a Y, vince Y).
- **Quick-edit ad alto valore**: toggle di permessi (allow/ask/deny per pattern), permission mode, hook on/off. Casi tipici della "piccola correzione costante" che hai descritto.

---

## C. Subagents, skills, commands, output styles — `[Anthropic]`

Estensioni utente ai comportamenti di Claude. Modalità `union` con potenziale collisione di nomi.

### C.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Doc |
|---|---|---|---|---|---|---|
| Subagent | G + P | `~/.claude/agents/*.md`, `.claude/agents/*.md` | md+frontmatter | RW | union, runtime shadowing? → vedi J.1 | [sub-agents](https://code.claude.com/docs/en/sub-agents) |
| Skill | G + P | `~/.claude/skills/<name>/SKILL.md` (+ files), `.claude/skills/<name>/...` | dir + md+frontmatter | RW | union, runtime shadowing? → vedi J.1 | [skills](https://code.claude.com/docs/en/skills) |
| Slash command (legacy) | G + P | `~/.claude/commands/*.md`, `.claude/commands/*.md` | md | RW | union, deprecato → migrare a Skill | [commands](https://code.claude.com/docs/en/commands) |
| Output style | G + P | `~/.claude/output-styles/*.md`, `.claude/output-styles/*.md` | md+frontmatter | RW | union | [output-styles](https://code.claude.com/docs/en/output-styles) |

### C.2 — Note

- **Skills sostituiscono Commands**: la doc Anthropic dice "for new workflows, use skills/ instead — same `/name` invocation, plus you can bundle supporting files". Trattare commands come legacy in dev-dash, ma comunque visualizzabili.
- **Campi chiave per Subagent** (frontmatter): `name`, `description`, `tools`, `model`, `color` (probabilmente).
- **Campi chiave per Skill** (frontmatter): `name`, `description`, eventuali `allowed-tools`, `model`.
- **Collisione di nomi**: strategia dev-dash decisa = **coesistenza con disambiguazione + edit** (vedi schema sezione "Modalità di composizione fra scope" e issue J.1). Identità per `name:` di frontmatter, non per filename.
- Vista dev-dash: lista unica con badge `[G]`/`[P]`, collisioni evidenziate per identità logica, badge `shadowed` quando il runtime davvero sceglie uno solo.

---

## D. Hooks & integrazioni esterne — `[Anthropic]`

### D.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Doc |
|---|---|---|---|---|---|---|
| Hook | G + P | dentro `settings.json` campo `hooks` | json + script paths | RM | union (tutti i livelli si attivano) | [hooks](https://code.claude.com/docs/en/hooks) · [hooks-guide](https://code.claude.com/docs/en/hooks-guide) |
| MCP server (project) | P | `./.mcp.json` (project root, NON dentro `.claude/`) | json | RM | union con user-scope | [mcp](https://code.claude.com/docs/en/mcp) |
| MCP server (user) | G | `~/.claude.json` | json | RM | union con project-scope | [mcp](https://code.claude.com/docs/en/mcp) |
| Channel | P | configurato come MCP server | json | RM | n/a | [channels](https://code.claude.com/docs/en/channels) · [channels-reference](https://code.claude.com/docs/en/channels-reference) |
| Plugin ⏬ | G + P | bundle (skills + agents + hooks + MCP combinati) | dir | RO (list-only) | union | [plugins](https://code.claude.com/docs/en/plugins) · [plugins-reference](https://code.claude.com/docs/en/plugins-reference) |

### D.2 — Note

- **Hooks**: gli eventi documentati includono `SessionStart`, `SessionEnd`, `PreToolUse`, `PostToolUse`, `UserPromptSubmit`, `PreCompact`, `Stop`, `SubagentStop`, `Notification`, `WorktreeCreate`. Già usati nel progetto (`SessionStart` rigenera INDEX.md, ecc.).
- **MCP** ha due file separati per scope user/project — non è un singolo file con override. È union pura.
- **Plugins** ⏬ (deprioritized): bundle distribuibili che combinano skills/agents/hooks/MCP/commands, installabili via marketplace. Pensati per *condividere* setup tra team/community. Per uso solo come quello dell'utente, valore basso — i mattoni primitivi (skill, agent, hook, MCP) sono già abbastanza. Adapter minimo: list-only "ecco i plugin installati", niente UI di gestione. **Ultima priorità.**
- **Campi chiave per Hook**: `event`, `matcher` (pattern di tool names o evento-specifico), `hooks[]` con `type` (command/prompt) e `command`.
- **Campi chiave per MCP server**: `command`, `args`, `env`, `transport` (stdio/http/sse).

---

## E. UI / surface — `[Anthropic]`

Personalizzazione dell'esperienza CLI/IDE.

### E.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Doc |
|---|---|---|---|---|---|---|
| Status line | G + P | dentro `settings.json` campo `statusLine` | json | RM | override | [statusline](https://code.claude.com/docs/en/statusline) |
| Keybindings | G | `~/.claude/keybindings.json` | json | RM | n/a (solo user) | [keybindings](https://code.claude.com/docs/en/keybindings) |
| Themes ⏬ | G | `~/.claude/themes/*` | varies | RM | n/a (solo user) | [interactive-mode](https://code.claude.com/docs/en/interactive-mode) |
| Interactive mode | G | preferenze tastiera/UI in settings | json | RM | n/a | [interactive-mode](https://code.claude.com/docs/en/interactive-mode) |

### E.2 — Note

- Gruppo a basso valore aggiunto per dev-dash — la maggior parte si configura una volta e si dimentica. Adapter minimi.
- **Themes** ⏬ deprioritized: temi colore per il CLI Claude. Una riga nello schema, ultima fase. Se non esiste neppure `~/.claude/themes/` sul sistema dell'utente, il tipo è semplicemente assente (il discovery la salta).

---

## F. Sessioni & cronologie

Tre cose strutturalmente diverse.

### F.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Origine | Stato |
|---|---|---|---|---|---|---|---|
| Conversation history → **Pilastro 5 (sez. L)** | G | `~/.claude/projects/<encoded-path>/*.jsonl` | jsonl | RO + indicizzata | n/a | A | Pilastro a sé, non adapter |
| Checkpointing ⛔ fuori scope | P | gestito da Claude internamente | varies | — | n/a | A | Vedi nota F.2 |
| Session handoff (manuale) | P | `.memory-bank/YYYY-MM-DD-HHmm-<slug>.md` | md | RW | n/a | U | Adapter standard |

### F.2 — Note

- **Conversation history** è promossa a **pilastro indipendente** (sezione L) perché architettonicamente non rientra nel pattern resource+scope: richiede indicizzazione full-text, ranking, navigazione cronologica, cross-reference. Vedi sezione L per la spec embrionale.
- **Checkpointing** ⛔: è una funzione di rewind interattivo *dentro* la sessione Claude (`Esc Esc` o `/rewind`) — restore code/conversation/entrambi a un prompt precedente, persistente 30 giorni. Non traccia bash commands né edit esterni. Quando ti serve, sei già nella CLI Claude — una dashboard ha poco da offrire. **Fuori scope per la prima fase**, eventualmente "list/inspect checkpoints di sessioni passate" come archeologia in fase 2.
- **Session handoff** (`.memory-bank/`): è il sistema di continuità preferito dall'utente, alternativo all'auto-memory di Anthropic (vedi sezione A.4). Il filename pattern `YYYY-MM-DD-HHmm-<slug>.md` rende possibile un timeline render gratuito.

---

## G. Documentazione & scaffolding di progetto — `[User]`

Convenzioni tue (da sheet-atlas), non Anthropic. Tutte mono-scope (P), composizione `n/a`. Origine `U` ovunque.

### G.1 — Quick-reference

| Tipo | Path | Formato | Postura | Note |
|---|---|---|---|---|
| ADR | `.development/reference/decisions/*.md` | md+frontmatter | RW | Architecture Decision Records |
| Spec — planned | `.development/specs/planned/*.md` | md+frontmatter | RW | spostati a in-progress dal post-checkout hook |
| Spec — in-progress | `.development/specs/in-progress/*.md` | md+frontmatter | RW | |
| Spec — implemented | `.development/specs/implemented/*.md` | md+frontmatter | RW | spostati dal pre-commit hook su merge |
| Spec — backlog/archived | `.development/specs/{backlog,archived}/*.md` | md+frontmatter | RW | |
| Tech-debt issue | `.development/tech-debt/*.md` | md+frontmatter | RW | kanban candidate |
| Tangential idea note | `.memory-bank/ideas/YYYY-MM-DD-<slug>.md` | md+frontmatter | RW | Capture di idee emerse in conversazione. Lifecycle `open → parked → promoted-to-X → dropped`. Vedi `.memory-bank/ideas/README.md` e regola operativa in `.claude/rules/idea-capture.md` |
| CURRENT-STATUS | `.development/CURRENT-STATUS.md` | md | RW | manuale |
| INDEX | `.development/INDEX.md` | md | RO | auto-generato (hook) |
| ARCHITECTURE | `.development/ARCHITECTURE.md` | md | RO | auto-generato (hook) |
| README / docs | `docs/*.md`, `README.md` | md | RW | |
| `.personal/` | `.personal/**/*` | varies | RW | gitignored |

### G.2 — Note

- **Frontmatter standardizzato** per ADR/spec/tech-debt rende possibile un adapter unico ben fatto: campi tipo `type`, `priority`, `status`, `discovered`, `related`. Vista kanban/tabella praticamente gratuita.
- **`.rules/` workspace è obsoleta** (era `/data/repos/.rules/`): vestige del vecchio setup tre-livelli (globale/workspace/locale). Sheet-atlas l'ha completamente abbandonata, sostituita da `.claude/rules/` Anthropic a livello progetto. Cose ancora utili lì dentro (es. `goto.yaml` per navigazione progetti, template, script): se servono, trovano nuova casa altrove (uno scaffold-tool, `/data/repos/.scripts/`, ecc.). **Non è uno scope per dev-dash.**
- **Auto-generati vs editabili**: `INDEX.md` e `ARCHITECTURE.md` sono `RO` perché rigenerati da hook. Ha senso che dev-dash li mostri come "file derivato" con un link "rigenera".

---

## H. Risorse di confine

| Tipo | Scope | Path | Formato | Composizione | Origine | Doc |
|---|---|---|---|---|---|---|
| `.worktreeinclude` | P | `./.worktreeinclude` | gitignore-like | n/a | A | [common-workflows#copy-gitignored-files-to-worktrees](https://code.claude.com/docs/en/common-workflows) |
| `.gitignore` | P | `./.gitignore` | gitignore | n/a | tooling | — |
| `.editorconfig` | P | `./.editorconfig` | INI | n/a | tooling | — |

Probabilmente fuori scope per dev-dash, ma listato per esaustività.

---

## I. Quick-reference globale — solo Tipo + Scope + Composizione

Per scansione rapida. Le righe `A` (Anthropic) sono quelle da tenere sotto osservazione per derive di doc.

| # | Tipo | Origine | Scope | Composizione |
|---:|---|:-:|---|---|
| 1 | CLAUDE.md (catena multi-livello, vedi A.1) | A | managed + G + ancestor + P + project-local + subdir | concat |
| 2 | Auto-memory MEMORY.md + topic files | A | G (per project) | n/a |
| 3 | Path-scoped rules | A | G+P | union |
| 4 | Agent memory | A | G+P | n/a |
| 5 | settings.json (+local) | A | G+P | override |
| 6 | Permissions | A | G+P | override+merge |
| 7 | Permission mode | A | G+P | override |
| 8 | Env vars | A | G+P | override |
| 9 | Model config | A | G+P | override |
| 10 | Subagent | A | G+P | union |
| 11 | Skill | A | G+P | union |
| 12 | Slash command (legacy) | A | G+P | union |
| 13 | Output style | A | G+P | union |
| 14 | Hook | A | G+P | union |
| 15 | MCP server | A | G+P (file diversi) | union |
| 16 | Channel | A | P | n/a |
| 17 | Plugin ⏬ | A | G+P | union |
| 18 | Status line | A | G+P | override |
| 19 | Keybindings | A | G | n/a |
| 20 | Themes ⏬ | A | G | n/a |
| 21 | Conversation history → **Pilastro 5 (sez. L)** | A | G | n/a |
| 22 | Checkpointing ⛔ fuori scope | A | P | n/a |
| 23 | Session handoff | U | P | n/a |
| 24 | ADR | U | P | n/a |
| 25 | Spec (5 buckets) | U | P | n/a |
| 26 | Tech-debt issue | U | P | n/a |
| 27 | Generated docs (INDEX/ARCHITECTURE) | U | P | n/a |
| 28 | Manual docs (README/CURRENT-STATUS/docs/) | U | P | n/a |

Legenda: ⏬ deprioritized (adapter minimo, ultima fase) · ⛔ fuori scope (almeno per la prima fase)

### Distribuzione (28 risorse totali)

- **Origine**: 22 Anthropic, 6 User scaffolding
- **Composizione**: 8 union, 6 override, 1 concat (multi-livello), 13 n/a
- **Stato**: 25 da implementare normalmente, 2 deprioritized (Plugin, Themes), 1 fuori scope (Checkpointing); più 1 pilastro separato (Conversation history)
- **Postura prevalente**: ~12 RW (full edit utile), ~13 RM (read-mostly + quick-edit), il resto RO o specializzati

→ La modalità dominante è `union` (estensioni: agents/skills/commands/hooks/output-styles/MCP). Conferma che la vista più frequente è "lista unica con badge dello scope".
→ Il pattern `concat` è raro ma centrale (CLAUDE.md). Una sola riga, ma la più complessa: 6 livelli di scope, imports, exclude, condizionalità.
→ `override` è quasi tutto in `settings.json` e dintorni.

---

## J. Open questions

### Risolte in discussione 2026-04-26

1. ✅ **Collisione di nomi** per agents/skills/commands/hooks → strategia: **coesistenza con disambiguazione + edit**. Render `union` mostra entrambi con badge `[G]`/`[P]`, marca `shadowed` se runtime sceglie uno solo, identità per `name:` di frontmatter (non solo filename). Per hooks: pura union, no shadowing. Resta da **verificare la semantica runtime esatta** (locale vince? errore?) per agents/skills prima di scrivere l'adapter.
2. ✅ **`.rules/` workspace** → **obsoleta**, rimossa dal modello (vedi sezione G.2).
3. ✅ **Conversation history** → **pilastro separato (sezione L)**, non adapter.
4. ✅ **Plugins** → **deprioritized**, adapter minimo list-only, ultima priorità.
5. ✅ **Auto-memory encoding** → `<repo-path>` con `/` → `-`. Verificato.
6. ✅ **Themes** → deprioritized, una riga nello schema, ultima fase.
7. ✅ **Checkpointing** → **fuori scope** prima fase.
8. ✅ **`.worktreeinclude`** → fuori scope, listato in H per esaustività.

### Aperte

- **J.1 — Semantica runtime di shadowing per agents/skills**. Quando un agent di nome `X` esiste sia globalmente che in progetto, cosa fa Claude a runtime? Locale vince silenziosamente? Errore? Coesistono con disambiguatore tipo `X@global` / `X@project`? *Vale per ogni tipo (agent, skill, command, hook, MCP server) o cambia?* Verifica via doc + test prima di scrivere il render `union`. Nota: per hooks la doc è chiara (tutti si attivano, no shadowing).
- **J.2 — Promozione di scope**. Workflow utile in dev-dash? Es. "promuovi questo agent da locale a globale" (o viceversa). È un'operazione che fai abbastanza spesso da meritare un comando dedicato, o è raro come "git mv" e basta che sia possibile a mano?
- **J.3 — Catena CLAUDE.md su filesystem reale**. Il walk-up di ancestor directories può scoprire CLAUDE.md inattesi (es. `/data/repos/CLAUDE.md` se mai esistesse). Dev-dash deve **scoprirli proattivamente** e mostrarli, o solo elencare ciò che trova senza commentare? Il caso d'uso interessante: scoprire che un ancestor sta iniettando contenuto che non sapevi.
- **J.4 — `claudeMdExcludes`**: dev-dash deve solo **applicare** gli exclude (mostrare cosa è effettivamente caricato) o anche **gestirli** (UI per aggiungere/rimuovere pattern)? La gestione è quick-edit di un campo JSON, banale; il valore è esporre la presenza dell'exclude.
- **J.5 — `.mcp.json` semantica**: come si compongono `~/.claude.json` (user-scope) e `./.mcp.json` (project-scope)? Union? Override per nome del server? Verifica.

---

## K. Implicazioni architettoniche

Quattro pattern di rendering, derivati dalla colonna *Composizione*:

1. **`concat-render`**: due (o più) viste affiancate con ordine di concatenazione esplicito. Usato da: CLAUDE.md user/project.
2. **`override-render` (merge resolver)**: vista chiave-per-chiave che mostra origine e vincitore. Usato da: settings.json e tutto ciò che vive dentro.
3. **`union-render` (lista con badge)**: lista unica, badge `[G]`/`[P]`, collisioni di nome evidenziate. Usato da: agents, skills, commands, hooks, MCP, output-styles, plugins, path-scoped rules.
4. **`single-render`**: vista singola, niente da comporre. Usato da: tutto il resto.

Più un quinto pattern dedicato:

5. **`history-render`**: search full-text + navigazione cronologica. Solo per conversation history. Architettonicamente diverso, da progettare a parte.

→ **Il "core" di dev-dash è 5 render + N adapter.** Quando Anthropic introduce qualcosa di nuovo, scegli il render giusto e scrivi un adapter da 30 righe.

---

## L. Pilastro 5 — Conversation history (spec embrionale)

Promossa a pilastro indipendente perché architettonicamente fuori dal pattern resource+scope: richiede **indicizzazione full-text**, navigazione **temporale**, e **cross-reference** con altri artefatti del progetto.

### L.1 — Sorgente dati

I file `.jsonl` in `~/.claude/projects/<encoded-path>/` sono cronologie complete e autoritative di tutte le sessioni Claude su un progetto, prodotte automaticamente dal CLI. Ogni riga è un evento (turn utente, turn assistente, tool call, ecc.). Encoding del path: `/` → `-` (vedi A.3).

Non serve un export hook per averle: esistono già. Il `session-archive.py` di progetto produce un *secondo* artefatto (handoff in `.memory-bank/`) che è una sintesi semantica curata, complementare al transcript raw.

### L.2 — Capacità candidate

In ordine di valore percepito, da scegliere/scartare in fase di spec dedicata:

- **Search full-text** sui transcripts. Filtri per: data, progetto, tool usato, file menzionati, errori incontrati, modello usato.
- **Cross-reference filesystem** ↔ **conversazione**: dato un file (es. `MainWindowViewModel.cs`), trovare tutte le sessioni che l'hanno toccato. Inverso: dato un commit, trovare la sessione in cui è stato prodotto.
- **Bridge `.memory-bank/` ↔ transcript**: dato un handoff, linkare alla sessione raw corrispondente. Inverso: dato un transcript, suggerire "questo merita un handoff?".
- **Promote to ADR/spec/issue**: estrarre un blocco interessante da una conversazione e portarlo come decisione, spec, o tech-debt nel progetto.
- **Timeline visualization** delle sessioni per progetto (durata, modello, # tool calls, # errori).
- **Diff fra sessioni** sullo stesso topic: utile quando esplori qualcosa più volte e vuoi capire cosa è cambiato nell'approccio.

### L.3 — Pattern di render dedicato

`history-render`: combinazione di **search box + faceted filters + timeline + transcript viewer**. Non assimilabile ai render delle altre sezioni. Da progettare con i suoi propri principi UX.

### L.4 — Stato

**Spec dedicata da scrivere separatamente** dopo che il resource-model è consolidato. Da decidere: priorità (subito dopo MVP del core? o fase 2?), perimetro minimo (solo search? search + cross-ref?), storage indice (in-memory? su disco?).

---

## M. Pilastro 6 — Config versioning (placeholder)

Pilastro sorello del 5 (Conversation history): entrambe sono "evoluzione di stato di Claude nel tempo", ma con operazioni diverse. La 5 è search/navigazione su una cronologia immutabile; la 6 è snapshot/restore/diff su uno stato mutevole.

### M.1 — Cosa serve

Un sistema di **profili nominati** sopra l'intera config Claude Code (`~/.claude/`, `~/.claude.json`, eventualmente `.claude/` di progetto). Capabilities target: snapshot, switch, diff, list, eventualmente cross-machine sync.

### M.2 — Caso d'uso primario

Testare l'impatto di una modifica alla config senza perdere il setup precedente né confondersi su quale variante si sta usando. Esempio concreto: sessioni con `ARCHITECTURE.md` auto-loaded vs senza, per misurare se davvero cambia il comportamento di Claude.

### M.3 — Spazio di design

Decisione cardine: backing store. Candidato forte è **git su `~/.claude/`** (snapshot/swap/diff sono primitive native), con esclusioni per file derived (auto-memory, conversation history) e secrets. Granularità inizialmente globale-only, per-progetto come fase 2.

### M.4 — Stato

**Spec dedicata da scrivere separatamente.** Punto di partenza completo: `.memory-bank/ideas/2026-04-26-claude-config-versioning.md` (idea catturata con sketch del design space e decisioni che la spec dovrà sciogliere). Quando si attacca, quella nota viene promossa a `promoted-to-spec` con link al nuovo file.

Pattern di render dedicato: probabilmente non ne ha bisogno di uno nuovo — un mix di `single-render` (lista profili) + `override-render` (diff fra due profili) basta. Da confermare in spec.
