# DevDash — Resource Model (DRAFT)

*Created: 2026-04-26 · Updated: 2026-04-26 (J.1-J.5 resolved). In flux finché il viewer base non è implementato e validato. Working artifact per la discussione di redesign dev-dash.*

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
- **`override`** — gerarchia formale, livello più specifico vince sulla stessa **chiave di identità**. La chiave varia per tipo: chiave JSON (`settings.json` e dintorni), nome del server (MCP), campo `name:` di frontmatter (subagent, skill, output style), filename (slash command legacy). Il rendering è un "merge resolver": per ogni chiave mostra il vincitore e segnala i livelli **shadowati** dietro (badge `[shadowed: <scope>]`). ⚠️ **La direzione della gerarchia varia per famiglia di risorsa** — non c'è una regola unica "più vicino vince": agents `Managed > CLI > Project > User > Plugin`, skills `Enterprise > User > Project` (con plugin namespaced, no conflict), MCP `Local > Project > User > Plugin > Connectors`, settings `Managed > Project local > Project shared > User`. Vedi sezioni B/C/D per le mappe complete.
- **`union`** — coesistenza pura, **nessuno shadowing**: tutti i livelli si attivano insieme. Si applica dove non esiste concetto di "stesso nome che shadowa": **hooks** (ogni hook config è additiva su matcher/event) e **path-scoped rules** (ogni rule si carica indipendentemente se i suoi `paths:` matchano). Rendering: lista con badge `[G]`/`[P]` per provenienza, nessuna etichetta `shadowed`.
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

**`claudeMdExcludes`** (in `.claude/settings.local.json` o ai vari livelli di settings): array di glob pattern che **escludono** specifici CLAUDE.md dall'essere caricati. Per mostrare correttamente "what Claude actually sees", dev-dash deve **applicare** questi exclude alla catena A.1 e marcare i file mutati come `excluded` (fase 1, vedi J.4). La **gestione** dei pattern (UI add/remove) è fase 2 nice-to-have.

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

Estensioni utente ai comportamenti di Claude. Modalità `override` gerarchico **con gerarchie diverse per famiglia** — vedi note in C.2.

### C.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Doc |
|---|---|---|---|---|---|---|
| Subagent | M + CLI + P + G + Plugin | `<managed>/.claude/agents/*.md` · `--agents` CLI · `.claude/agents/*.md` · `~/.claude/agents/*.md` · `<plugin>/agents/*.md` | md+frontmatter | RW | override `Managed > CLI > Project > User > Plugin` | [sub-agents](https://code.claude.com/docs/en/sub-agents) |
| Skill | E + G + P + Plugin | `~/.claude/skills/<name>/SKILL.md` (+ files), `.claude/skills/<name>/...`, `<plugin>/skills/<name>/...` | dir + md+frontmatter | RW | override `Enterprise > User > Project` (plugin namespaced `plugin:skill`, no conflict) | [skills](https://code.claude.com/docs/en/skills) |
| Slash command (legacy) | G + P | `~/.claude/commands/*.md`, `.claude/commands/*.md` | md | RW | mergiato in skills (skill > command se collidono per nome) | [skills#custom-commands](https://code.claude.com/docs/en/skills) |
| Output style | G + P | `~/.claude/output-styles/*.md`, `.claude/output-styles/*.md` | md+frontmatter | RW | override (presunto, da verificare empiricamente) | [output-styles](https://code.claude.com/docs/en/output-styles) |

### C.2 — Note

- **⚠️ Gerarchie opposte tra agents e skills** — caso non simmetrico, va esposto chiaramente nell'UI. Per **subagents** vince il livello *più vicino* (`Project > User`); per **skills** vince il livello *più lontano* (`Enterprise > User > Project`, cioè user shadowa project). La stessa coppia di file `[G]`/`[P]` si comporta in modi opposti a seconda del tipo. Dev-dash deve mostrare la direzione di gerarchia esplicitamente accanto a ogni famiglia, non assumerla.
- **Skills hanno mergiato Commands**: doc ufficiale: "Custom commands have been merged into skills. A file at `.claude/commands/deploy.md` and a skill at `.claude/skills/deploy/SKILL.md` both create `/deploy` and work the same way. Your existing `.claude/commands/` files keep working." Se collidono per nome, la skill vince. Trattare commands come legacy in dev-dash.
- **Identità di matching**:
  - Subagent / Skill / Output style: campo `name:` di frontmatter (se omesso, fallback su nome della directory/filename).
  - Slash command: filename (no frontmatter univoco).
  - L'UI deve mostrare la chiave di identità rilevante, non solo il filename.
- **Campi chiave per Subagent** (frontmatter): `name`, `description`, `tools`, `model`, `color`, `mcpServers`, `hooks`, `memory`, `permissionMode`, `effort`, `isolation`.
- **Campi chiave per Skill** (frontmatter): `name`, `description`, `allowed-tools`, `model`, `disable-model-invocation`, `user-invocable`, `paths`, `arguments`, `context: fork`, `agent`, `effort`.
- **Live change detection per skills**: Claude Code watcha `~/.claude/skills/`, `.claude/skills/`, e gli `--add-dir`. Modifiche in-session sono picked up senza restart (creazione di una *nuova* top-level skills directory richiede restart). Implicazione per dev-dash: edit di una skill riflette immediatamente, niente "applica modifiche" needed.
- Vista dev-dash: render `override` (merge resolver) per ciascuna famiglia, con shadowed-list visibile e direzione di gerarchia esplicita. Non `union-list`.

---

## D. Hooks & integrazioni esterne — `[Anthropic]`

### D.1 — Quick-reference

| Tipo | Scope | Path | Formato | Postura | Composizione | Doc |
|---|---|---|---|---|---|---|
| Hook | G + P | dentro `settings.json` campo `hooks` | json + script paths | RM | union (tutti i livelli si attivano, no shadowing) | [hooks](https://code.claude.com/docs/en/hooks) · [hooks-guide](https://code.claude.com/docs/en/hooks-guide) |
| MCP server (local) | P (per-project, gitignored de facto) | `~/.claude.json` (entry del progetto, scope `local`) | json | RM | override per `name`: `Local > Project > User > Plugin > Connectors` | [mcp](https://code.claude.com/docs/en/mcp) |
| MCP server (project, shared) | P | `./.mcp.json` (project root, NON dentro `.claude/`) | json | RM | override (vedi sopra) | [mcp](https://code.claude.com/docs/en/mcp) |
| MCP server (user) | G | `~/.claude.json` (top-level, scope `user`) | json | RM | override (vedi sopra) | [mcp](https://code.claude.com/docs/en/mcp) |
| Channel | P | configurato come MCP server | json | RM | n/a | [channels](https://code.claude.com/docs/en/channels) · [channels-reference](https://code.claude.com/docs/en/channels-reference) |
| Plugin ⏬ | G + P | bundle (skills + agents + hooks + MCP combinati) | dir | RO (list-only) | varies (ogni componente segue la gerarchia della propria famiglia) | [plugins](https://code.claude.com/docs/en/plugins) · [plugins-reference](https://code.claude.com/docs/en/plugins-reference) |

### D.2 — Note

- **Hooks**: gli eventi documentati includono `SessionStart`, `SessionEnd`, `PreToolUse`, `PostToolUse`, `UserPromptSubmit`, `PreCompact`, `Stop`, `SubagentStop`, `Notification`, `WorktreeCreate`. Già usati nel progetto (`SessionStart` rigenera INDEX.md, ecc.). Modalità `union` pura (tutti i livelli si attivano, no shadowing).
- **MCP** è **override per nome del server**, non union. Tre scope distinti su due file:
  - `local` (per-project, privato): dentro `~/.claude.json`, nell'entry corrispondente al path del progetto. È il default quando aggiungi un server senza `--scope`.
  - `project` (per-project, shared): in `./.mcp.json` alla root del progetto, designato per essere committato.
  - `user` (cross-project, privato): dentro `~/.claude.json` a livello top.

  Gerarchia di precedenza (alto → basso): `Local > Project > User > Plugin > claude.ai connectors`. Plugins e connectors matchano per *endpoint* (URL/command), non per nome — duplicati cross-source vengono unificati. Quote doc: "When the same server is defined in more than one place, Claude Code connects to it once, using the definition from the highest-precedence source."
- **Implicazione per dev-dash**: `~/.claude.json` non è "user-scope only" — contiene **due scope distinti**. L'adapter MCP deve leggere il file e separare le entry per-project (sotto la chiave del path del progetto) dalle entry user (top-level). Mostrare entrambi gli scope come righe separate nel render.
- **Plugins** ⏬ (deprioritized): bundle distribuibili che combinano skills/agents/hooks/MCP/commands, installabili via marketplace. Pensati per *condividere* setup tra team/community. Per uso solo come quello dell'utente, valore basso — i mattoni primitivi (skill, agent, hook, MCP) sono già abbastanza. Adapter minimo: list-only "ecco i plugin installati", niente UI di gestione. **Ultima priorità.**
- **Campi chiave per Hook**: `event`, `matcher` (pattern di tool names o evento-specifico), `hooks[]` con `type` (command/prompt) e `command`.
- **Campi chiave per MCP server**: `command`, `args`, `env`, `transport` (stdio/http/sse), `oauth.scopes`, `authServerMetadataUrl`.

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
| Session handoff (manuale) | P | `.memory-bank/journal/handoffs/YYYY-MM-DD-HHmm-<slug>.md` | md | RW | n/a | U | Adapter standard |

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
| 10 | Subagent | A | M+CLI+P+G+Plugin | override (`Project > User`, vedi C.2) |
| 11 | Skill | A | E+G+P+Plugin | override (`User > Project`, plugin namespaced) |
| 12 | Slash command (legacy) | A | G+P | mergiato in skill |
| 13 | Output style | A | G+P | override (presunto) |
| 14 | Hook | A | G+P | union |
| 15 | MCP server | A | local+project+user+plugin+conn | override per `name` |
| 16 | Channel | A | P | n/a |
| 17 | Plugin ⏬ | A | G+P | varies (per componente) |
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
- **Composizione**: 1 concat, ~10 override, 2 union pura (hook, path-scoped rules), 1 mergiato (slash command → skill), 1 varies (plugin), ~13 n/a
- **Stato**: 25 da implementare normalmente, 2 deprioritized (Plugin, Themes), 1 fuori scope (Checkpointing); più 1 pilastro separato (Conversation history)
- **Postura prevalente**: ~12 RW (full edit utile), ~13 RM (read-mostly + quick-edit), il resto RO o specializzati

→ La modalità dominante è `override` gerarchico (settings family + estensioni: agents/skills/output-styles/MCP). Un singolo render `override` di qualità copre la maggioranza delle risorse.
→ Il pattern `concat` è raro ma centrale (CLAUDE.md). Una sola riga, ma la più complessa: 6 livelli di scope, imports, exclude, condizionalità.
→ `union` pura sopravvive solo su hooks e path-scoped rules — non più la modalità dominante come ipotizzato inizialmente.
→ **Asimmetria gerarchica**: la direzione di `override` non è uniforme. Subagents privilegia il livello più vicino (project shadowa user), skills l'opposto (user shadowa project). L'UI deve esporre la direzione esplicitamente per ogni famiglia.

---

## J. Open questions

### Risolte in discussione 2026-04-26 (prima tornata)

1. ✅ **`.rules/` workspace** → **obsoleta**, rimossa dal modello (vedi sezione G.2).
2. ✅ **Conversation history** → **pilastro separato (sezione L)**, non adapter.
3. ✅ **Plugins** → **deprioritized**, adapter minimo list-only, ultima priorità.
4. ✅ **Auto-memory encoding** → `<repo-path>` con `/` → `-`. Verificato.
5. ✅ **Themes** → deprioritized, una riga nello schema, ultima fase.
6. ✅ **Checkpointing** → **fuori scope** prima fase.
7. ✅ **`.worktreeinclude`** → fuori scope, listato in H per esaustività.

### Risolte in discussione 2026-04-26 (seconda tornata, J.1-J.5)

8. ✅ **J.1 — Semantica runtime di shadowing**. Verificata via doc Anthropic ufficiale (`code.claude.com/docs/en/sub-agents`, `/skills`, `/mcp`). Risultato: **non è `union` come ipotizzato, è `override` gerarchico** ma con **gerarchie diverse per famiglia**:
   - Subagents: `Managed > CLI > Project > User > Plugin` (project shadowa user).
   - Skills: `Enterprise > User > Project` (user shadowa project — opposto agli agents). Plugin namespaced `plugin:skill`, no conflict.
   - Slash commands legacy: mergiati in skills, skill > command per stesso nome.
   - MCP server: `Local > Project > User > Plugin > Connectors` (vedi J.5).
   - Hooks: `union` pura, tutti i livelli si attivano, no shadowing.
   - Output styles: presunto `override`, da verificare empiricamente.

   **Implicazioni**: il render `union` ipotizzato in prima tornata diventa `override` (merge resolver con shadowed-list). I 5 pattern di rendering della sezione K si riducono a 4. La direzione di gerarchia va esposta esplicitamente per famiglia nell'UI — non è uniforme.

9. ✅ **J.2 — Promote-scope**. Risoluzione: **quick-action contestuale del render `override`**, non operazione top-level. Frequenza prevista bassa in steady-state, alta ora durante setup config (~1 ogni 2-3 sessioni) e quando Anthropic riorganizza i path. Implementazione: filesystem mv + git operations sul progetto sorgente/destinazione (`git mv` cross-tree NON funziona — la home è fuori dal working tree del repo, serve `mv` plain + `git add` per registrare la deletion). L'adapter incapsula la sequenza corretta.

10. ✅ **J.3 — Discovery ancestor CLAUDE.md**. Risoluzione: **viewer puro fase 1**, fedele a ciò che Claude effettivamente carica (walk-up del filesystem applicato esattamente come fa il runtime). **Diagnostic layer fase 2 emergente**, scope definito dall'uso — seed iniziale: drift cleanup quando si cambia configurazione. Eleva a principio architettonico trasversale: *dev-dash mostra runtime truth, non spec* — vedi sezione K.

11. ✅ **J.4 — `claudeMdExcludes`**. Risoluzione: stesso pattern di J.3 — **applicazione fase 1** (la chain mostrata = chain effettivamente caricata, file marcati `excluded` se matchati da un pattern), **gestione fase 2** (UI add/remove pattern). Caso particolare del principio "view-of-truth runtime + management come nice-to-have".

12. ✅ **J.5 — Composizione MCP**. Verificata via doc ufficiale. Risultato: **`override` per nome del server**, non union. Tre scope distinti su due file:
    - `local` in `~/.claude.json` (per-project, sotto la chiave del progetto, default)
    - `project` in `./.mcp.json` (shared, designato per version control)
    - `user` in `~/.claude.json` (top-level, cross-project)

    Gerarchia: `Local > Project > User > Plugin > Connectors`. Plugins/connectors matchano per endpoint (URL/command). **Implicazione**: `~/.claude.json` non è "user-scope only" — l'adapter MCP deve separare le due aree.

### Aperte

*(nessuna al 2026-04-26 — tutte le OQ del modello base sono risolte. Restano i pilastri 5 e 6 con spec dedicate da scrivere — vedi L e M.)*

---

## K. Implicazioni architettoniche

### K.1 — Pattern di rendering

Quattro pattern di rendering, derivati dalla colonna *Composizione*:

1. **`concat-render`**: due (o più) viste affiancate con ordine di concatenazione esplicito. Usato da: catena CLAUDE.md (multi-livello).
2. **`override-render` (merge resolver)**: vista chiave-per-chiave che mostra il **vincitore** e i livelli **shadowati** dietro, con direzione di gerarchia esplicita per famiglia. Usato da: settings.json e dintorni, subagents, skills, output styles, MCP server, slash commands legacy. **È il pattern dominante** del modello (~10 risorse).
3. **`union-render` (lista con badge)**: lista unica con badge `[G]`/`[P]` per provenienza, **nessuno shadowing** (tutti i livelli attivi). Usato solo da: hooks, path-scoped rules.
4. **`single-render`**: vista singola, niente da comporre. Usato da: tutto il resto (auto-memory, agent memory, keybindings, themes, channel, e tutti gli artefatti scaffolding utente).

Più un quinto pattern dedicato:

5. **`history-render`**: search full-text + navigazione cronologica. Solo per conversation history. Architettonicamente diverso, da progettare a parte.

→ **Il "core" di dev-dash è 4 render principali + 1 dedicato + N adapter.** Quando Anthropic introduce qualcosa di nuovo, scegli il render giusto e scrivi un adapter sottile.

### K.2 — Principio trasversale: view-of-truth runtime

Emerso da J.3 e J.4, vale per **tutti gli adapter** del modello: **dev-dash mostra ciò che Claude effettivamente carica a runtime, non ciò che è dichiarato sulla carta**. Implicazioni concrete:

- **CLAUDE.md catena**: applicare `claudeMdExcludes`, risolvere `@`-imports come fa Claude (limite 5 hop), rispettare `--add-dir` se rilevante.
- **Subagents/skills/commands/MCP**: applicare la gerarchia di shadowing di J.1 — la riga "vincente" è quella che Claude usa, le altre sono mostrate come `shadowed`.
- **Path-scoped rules**: indicare che il caricamento è **condizionale** sui `paths:` glob — esporre i pattern, segnalare se la rule è attualmente attiva nel contesto in cui dev-dash è aperto.
- **Hooks**: indicare quale levels sono attivi per quale evento.

Niente "view-of-spec" parallela. Una sola vista, fedele al runtime. Filosofia coerente con `overview.md`: "DevDash manages documentation and context. Claude Code manages execution and automation" — la trasparenza sul runtime *è* gestione del contesto.

### K.3 — Layer diagnostico (fase 2, emergente)

Sopra il viewer base, un layer opzionale che fa **interpretazione attiva** su quanto il viewer mostra. Scope definito dall'uso, non specificato in anticipo. Seed iniziali (J.3, J.4, dalla discussione):

- Drift cleanup quando si cambia configurazione
- Segnalare ancestor CLAUDE.md inattesi
- Diff dall'ultima sessione su file di config rilevanti
- Pattern import circolari o `@`-targets cancellati
- Catene CLAUDE.md inusualmente lunghe in token count

Costruirlo dopo aver familiarizzato col viewer base. Il rischio è generare rumore se i segnali sono troppi o mal calibrati.

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
