---
captured: 2026-04-26
status: open
context: "Emersa durante la chiusura di J.4 nel resource-model, branch docs/resource-model-oq-resolution. Doppio seed: prima 'background task agents con Haiku/Sonnet decoupled dalla sessione', poi raffinata a 'skills come azioni eseguibili nell'UI di dev-dash con CRUD + run'."
tags: [skills, ux, automation, dashboard, dev-dash-feature]
---

## Cos'è

Trasformare l'elenco delle skill in **vista principale actionable** di dev-dash, dove ogni skill è una riga con: preview del contenuto rendered, metadata (scope, modello, allowed-tools), CRUD (create/edit/duplicate/delete), e un bottone **Esegui** che la lancia direttamente.

Il flusso ergonomico target: *crea skill → vedi preview → eseguila one-click*. Nessuno step di "copia-incolla nella CLI", nessuna ricerca del filename giusto, nessun overhead.

Lo **scheduling** è un add-on opzionale (skill `schedule` di Claude Code esiste già, e/o skill `loop` per intervalli). Sulla stessa skill può esistere come "esegui ora" + "schedula" come due azioni distinte. Lo scheduling NON è il focus primario.

## Perché merita

- **Allinea dev-dash con come si lavora davvero in CLI**: la skill è già una primitiva di Claude Code (non si reinventa nulla), ma l'invocazione `/skill-name` richiede ricordare il nome ed essere in sessione attiva. Un launcher visivo abbassa il costo cognitivo di "cosa esiste, cosa fa, lanciamola".
- **Inverte il rapporto tra dev-dash e Claude**: oggi dev-dash è un viewer passivo (specchia config/docs); con skills runnable diventa anche un launchpad. Resta coerente con la filosofia "DevDash manages documentation and context" — eseguire una skill *è* applicare contesto.
- **Idee concrete di task per cui vorrei un bottone** (seed, non spec):
  - Health check periodico della config (drift, link morti, dead path-scoped rules)
  - Auto-grooming idee parcheggiate (suggerire promote/drop dopo N giorni)
  - Pruning `.memory-bank/` vecchi (handoff > N mesi)
  - Audit settimanale "cosa è stato fatto in questo progetto" → digest
  - Rigenerazione di `INDEX.md` / `ARCHITECTURE.md` on-demand invece che via hook
- **Modello override**: la skill stessa può specificare `model:` nel frontmatter (Haiku/Sonnet/Opus). Una volta che l'esecuzione è one-click, la scelta del modello diventa parte del design della skill — non spreco di Opus per task triviali.

## Primitive Anthropic già esistenti

Importante non duplicare:

- **Skill** (`~/.claude/skills/<name>/SKILL.md`) con `model:`, `allowed-tools`, `disable-model-invocation`, `paths` — già la primitiva giusta per "azione standardizzata configurabile".
- **Skill `schedule`** — gestisce scheduled remote agents (cron + one-shot). Per il caso "skill che gira da sola periodicamente".
- **Skill `loop`** — esegue un prompt o slash-command su intervallo, locale alla CLI.
- **Subagent con `model` override** — per task da delegare *dentro* una sessione. Ortogonale al caso "azione standalone".

Il valore aggiunto di dev-dash è la **superficie visiva** sopra queste primitive (CRUD UI, preview, launcher), non la logica di esecuzione.

## Next-step minimo se si riprende

1. Spec dedicata in `.development/specs/planned/feature-skills-runner.md` (o nome più snello).
2. Decisioni cardinali da sciogliere:
   - Esecuzione: spawn `claude --skill <name>` come processo figlio? Oppure altra strada (API diretta)?
   - Surface dell'output: pannello dedicato? Stream nel main view? Toast?
   - CRUD su skill di scope user (`~/.claude/skills/`) vs project (`.claude/skills/`) — UX per scegliere lo scope al momento di create/duplicate.
   - Se/come integrare `schedule` come azione opzionale sulla stessa skill (vs feature separata "routines").
   - Quando promuovere il file: aggiornare frontmatter `status: promoted-to-spec` con link.

## Relazione con il resource-model

L'idea è un **caso d'uso** del render `union/override` per la riga "Skill" (sezione C del resource-model), arricchito con quick-actions (Run, Edit, Duplicate, Schedule). Architettonicamente: niente di nuovo nel modello risorse, è la prima dimostrazione concreta di "adapter con quick-action sostantive".

Probabile fase 2 di dev-dash: dopo che il viewer base (resource-model + render override) è solido, è la skill family il candidato naturale per essere il primo adapter "actionable" (non solo readable).
