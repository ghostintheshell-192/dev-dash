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
| **Scope** | `G` = utente/globale (`~/.claude/`), `P` = progetto (`./` o `.claude/`), `W` = workspace (convenzione utente, non Anthropic) |
| **Path** | Pattern relativo allo scope root |
| **Formato** | markdown / markdown+frontmatter / json / dir / shell |
| **Postura** | `RO` read-only · `RM` read-mostly + quick-edit · `RW` read + full-edit |
| **Composizione** | `concat` · `override` · `union` · `n/a` (vedi sezione "Modalità") |
| **Origine** | `A` = Anthropic-defined · `U` = User scaffolding · `M` = Mixed |
| **Doc** | Link alla doc ufficiale (per origine A) |

### Modalità di composizione fra scope

- **`concat`** — entrambi i livelli contribuiscono, niente vince. Il rendering deve mostrare entrambi affiancati.
- **`override`** — gerarchia formale, livello più specifico vince sulla stessa chiave. Il rendering è un "merge resolver".
- **`union`** — coesistenza, override per nome solo in caso di collisione (semantica esatta da verificare per ogni tipo). Il rendering è una lista con badge dello scope.
- **`n/a`** — risorsa esiste a un solo livello.

---

## A. Memoria di istruzioni — `[Anthropic]`

Cuore del sistema CLAUDE.md. È qui che Claude legge "come comportarsi".

### A.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Doc |
|---|---|---|---|---|---|---|
| CLAUDE.md (user) | G | `~/.claude/CLAUDE.md` | md | RW | concat | [memory](https://code.claude.com/docs/en/memory) |
| CLAUDE.md (project) | P | `./CLAUDE.md` *o* `.claude/CLAUDE.md` | md | RW | concat | [memory](https://code.claude.com/docs/en/memory) |
| Auto-memory (per project) | G | `~/.claude/projects/<encoded-path>/memory/MEMORY.md` + `<topic>.md` | md+frontmatter | RM | n/a | [memory#auto-memory](https://code.claude.com/docs/en/memory) |
| Path-scoped rules (user) | G | `~/.claude/rules/*.md` | md+frontmatter | RM | union | [memory#rules](https://code.claude.com/docs/en/memory) |
| Path-scoped rules (project) | P | `.claude/rules/*.md` | md+frontmatter | RM | union | [memory#rules](https://code.claude.com/docs/en/memory) |
| Agent memory (user) | G | `~/.claude/agent-memory/<agent>/MEMORY.md` | md | RM | n/a | [memory](https://code.claude.com/docs/en/memory) |
| Agent memory (project) | P | `.claude/agent-memory/<agent>/MEMORY.md` | md | RM | n/a | [memory](https://code.claude.com/docs/en/memory) |

### A.2 — Note

- **CLAUDE.md componibile**: user + project si **sommano** nel context — non c'è un "vincitore". Vista dev-dash: due pannelli affiancati con ordine di concatenazione esplicito.
- **Auto-memory**: gestita da Claude stesso (write tramite agente). Postura `RM` perché editare a mano è raro ma legittimo.
- **Path-scoped rules**: nuovo meccanismo Anthropic con frontmatter (`description`, `globs`, `alwaysApply`). Caricamento condizionale, NON sempre attivo come CLAUDE.md.
- **`.rules/` di workspace ≠ `.claude/rules/` di Anthropic**. Il `.rules/` nelle convenzioni di sheet-atlas/dev-dash è una directory custom (W scope), non una feature Claude Code. Vedi sezione G.
- **Campi chiave per "Path-scoped rule"**: `description`, `globs`, `alwaysApply` (frontmatter) + body.

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
| Subagent | G + P | `~/.claude/agents/*.md`, `.claude/agents/*.md` | md+frontmatter | RW | union, collision local-wins ⚠ | [sub-agents](https://code.claude.com/docs/en/sub-agents) |
| Skill | G + P | `~/.claude/skills/<name>/SKILL.md` (+ files), `.claude/skills/<name>/...` | dir + md+frontmatter | RW | union, collision local-wins ⚠ | [skills](https://code.claude.com/docs/en/skills) |
| Slash command (legacy) | G + P | `~/.claude/commands/*.md`, `.claude/commands/*.md` | md | RW | union ⚠ deprecato | [commands](https://code.claude.com/docs/en/commands) |
| Output style | G + P | `~/.claude/output-styles/*.md`, `.claude/output-styles/*.md` | md+frontmatter | RW | union | [output-styles](https://code.claude.com/docs/en/output-styles) |

### C.2 — Note

- **Skills sostituiscono Commands**: la doc Anthropic dice "for new workflows, use skills/ instead — same `/name` invocation, plus you can bundle supporting files". Trattare commands come legacy in dev-dash, ma comunque visualizzabili.
- **Campi chiave per Subagent** (frontmatter): `name`, `description`, `tools`, `model`, `color` (probabilmente).
- **Campi chiave per Skill** (frontmatter): `name`, `description`, eventuali `allowed-tools`, `model`.
- **Collisione di nomi** ⚠: assumo che il livello locale vinca su quello globale per stesso nome. **Da verificare prima di codificare** — potrebbe essere errore, potrebbero coesistere con disambiguatore, potrebbe essere semplice override.
- Vista dev-dash: lista unica con badge `[G]`/`[P]`, collisioni evidenziate.

---

## D. Hooks & integrazioni esterne — `[Anthropic]`

### D.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Doc |
|---|---|---|---|---|---|---|
| Hook | G + P | dentro `settings.json` campo `hooks` | json + script paths | RM | union (tutti i livelli si attivano) | [hooks](https://code.claude.com/docs/en/hooks) · [hooks-guide](https://code.claude.com/docs/en/hooks-guide) |
| MCP server (project) | P | `./.mcp.json` (project root, NON dentro `.claude/`) | json | RM | union con user-scope | [mcp](https://code.claude.com/docs/en/mcp) |
| MCP server (user) | G | `~/.claude.json` | json | RM | union con project-scope | [mcp](https://code.claude.com/docs/en/mcp) |
| Channel | P | configurato come MCP server | json | RM | n/a | [channels](https://code.claude.com/docs/en/channels) · [channels-reference](https://code.claude.com/docs/en/channels-reference) |
| Plugin | G + P | bundle (skills + agents + hooks + MCP combinati) | dir | RM | union | [plugins](https://code.claude.com/docs/en/plugins) · [plugins-reference](https://code.claude.com/docs/en/plugins-reference) |

### D.2 — Note

- **Hooks**: gli eventi documentati includono `SessionStart`, `SessionEnd`, `PreToolUse`, `PostToolUse`, `UserPromptSubmit`, `PreCompact`, `Stop`, `SubagentStop`, `Notification`, `WorktreeCreate`. Già usati nel progetto (`SessionStart` rigenera INDEX.md, ecc.).
- **MCP** ha due file separati per scope user/project — non è un singolo file con override. È union pura.
- **Plugins** è il meccanismo più recente e interessante: combina skills/agents/hooks/MCP in un unico bundle distribuibile via marketplace. Da capire se per uso personale conviene o se è over-engineering.
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
| Themes | G | `~/.claude/themes/*` | varies | RM | n/a (solo user) | [interactive-mode](https://code.claude.com/docs/en/interactive-mode) |
| Interactive mode | G | preferenze tastiera/UI in settings | json | RM | n/a | [interactive-mode](https://code.claude.com/docs/en/interactive-mode) |

### E.2 — Note

- Gruppo a basso valore aggiunto per dev-dash — la maggior parte si configura una volta e si dimentica. Adapter minimi.

---

## F. Sessioni & cronologie — `[Anthropic]` + `[User]`

### F.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Origine | Doc |
|---|---|---|---|---|---|---|---|
| Conversation history (built-in) | G | `~/.claude/projects/<encoded-path>/*.jsonl` | jsonl | RO | n/a | A | [agent-sdk/sessions](https://code.claude.com/docs/en/agent-sdk/sessions) |
| Checkpointing state | P | gestito da Claude internamente | varies | RO | n/a | A | [checkpointing](https://code.claude.com/docs/en/checkpointing) |
| Session handoff (manuale) | P | `.memory-bank/YYYY-MM-DD-HHmm-<slug>.md` | md | RW | n/a | U | — |

### F.2 — Note

- **Conversation history** è il pilastro che hai citato: la search robusta sui transcripts esportati. Postura `RO` — non ha senso editare, ma indicizzare full-text e cercare sì.
- È architettonicamente un'**altra bestia** rispetto al pattern resource+scope (richiede indicizzazione, ranking, navigazione cronologica). Probabile che meriti un suo pilastro separato in dev-dash, non un semplice adapter.
- I file `.jsonl` di Anthropic sono cronologie complete delle sessioni — già esistenti, non serve un hook per esportarli. L'hook `session-archive.py` che hai nel progetto duplica/sintetizza in formato leggibile.

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
| CURRENT-STATUS | `.development/CURRENT-STATUS.md` | md | RW | manuale |
| INDEX | `.development/INDEX.md` | md | RO | auto-generato (hook) |
| ARCHITECTURE | `.development/ARCHITECTURE.md` | md | RO | auto-generato (hook) |
| README / docs | `docs/*.md`, `README.md` | md | RW | |
| `.rules/` workspace | `<workspace>/.rules/*.md` | md | RW | convenzione sheet-atlas, NON `.claude/rules/` |
| `.personal/` | `.personal/**/*` | varies | RW | gitignored |

### G.2 — Note

- **Frontmatter standardizzato** per ADR/spec/tech-debt rende possibile un adapter unico ben fatto: campi tipo `type`, `priority`, `status`, `discovered`, `related`. Vista kanban/tabella praticamente gratuita.
- **`.rules/` workspace vs `.claude/rules/` Anthropic**: punto da chiarire nel progetto. Anthropic ha introdotto path-scoped rules con frontmatter dopo che la convenzione `.rules/` di sheet-atlas era già consolidata. Decidere se convergere o tenere distinti.
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
| 1 | CLAUDE.md user | A | G | concat |
| 2 | CLAUDE.md project | A | P | concat |
| 3 | Auto-memory | A | G (per project) | n/a |
| 4 | Path-scoped rules | A | G+P | union |
| 5 | Agent memory | A | G+P | n/a |
| 6 | settings.json | A | G+P (+local) | override |
| 7 | Permissions | A | G+P | override+merge |
| 8 | Permission mode | A | G+P | override |
| 9 | Env vars | A | G+P | override |
| 10 | Model config | A | G+P | override |
| 11 | Subagent | A | G+P | union |
| 12 | Skill | A | G+P | union |
| 13 | Slash command (legacy) | A | G+P | union |
| 14 | Output style | A | G+P | union |
| 15 | Hook | A | G+P | union |
| 16 | MCP server | A | G+P (file diversi) | union |
| 17 | Channel | A | P | n/a |
| 18 | Plugin | A | G+P | union |
| 19 | Status line | A | G+P | override |
| 20 | Keybindings | A | G | n/a |
| 21 | Themes | A | G | n/a |
| 22 | Conversation history | A | G | n/a |
| 23 | Checkpointing | A | P | n/a |
| 24 | Session handoff | U | P | n/a |
| 25 | ADR | U | P | n/a |
| 26 | Spec (5 buckets) | U | P | n/a |
| 27 | Tech-debt issue | U | P | n/a |
| 28 | Generated docs (INDEX/ARCHITECTURE) | U | P | n/a |
| 29 | Manual docs (README/CURRENT-STATUS/docs/) | U | P | n/a |
| 30 | `.rules/` workspace | U | W | n/a |

### Distribuzione

- **Origine**: 23 Anthropic, 7 User scaffolding
- **Scope**: 7 solo G, 4 solo P, 18 G+P (o varianti), 1 solo W
- **Composizione**: 11 union, 6 override, 2 concat, 11 n/a
- **Postura**: 20 RM (read-mostly + quick-edit), 7 RW (full edit utile), 3 RO

→ La modalità dominante è `union` (estensioni: agents/skills/commands/hooks/output-styles/MCP). Conferma che la vista più frequente è "lista unica con badge dello scope".
→ Il pattern `concat` è raro ma centrale (CLAUDE.md). Solo 2 risorse, ma sono *quelle* che usi tutti i giorni.
→ `override` è quasi tutto in `settings.json` e dintorni.

---

## J. Open questions

1. **Semantica esatta di collisione di nomi** per agents/skills/commands/hooks. Locale vince? Errore? Coesistenza con disambiguatore? Vale per ogni tipo o cambia? — *da verificare prima di codificare il render `union`*.
2. **`.rules/` workspace vs `.claude/rules/` Anthropic**: convergere o tenere distinti?
3. **Conversation history search**: pilastro a sé o adapter del modello generale?
4. **Plugins**: meccanismo importante per uso personale o roba enterprise? Quanto investire?
5. **Auto-memory per progetto** è sotto `~/.claude/projects/<encoded-path>/` — qual è l'algoritmo di encoding del path? (probabilmente sostituire `/` con `-`, da confermare).
6. **Themes** — esiste una cartella `~/.claude/themes/`? Quale formato? (la doc è scarna).
7. **Checkpointing**: quanto del suo state è ispezionabile dall'utente? Vale la pena esporlo?
8. **`.worktreeinclude`** dentro o fuori scope?

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
