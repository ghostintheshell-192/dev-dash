# Tangential Ideas

Note di idee emerse durante conversazioni di lavoro che meritano attenzione futura ma non *adesso*. Capture-and-keep-going pattern: salviamo qui invece di interrompere il workflow corrente o di tenerle in testa col rischio di perderle.

## Quando si cattura un'idea

- L'utente lo richiede esplicitamente con frasi tipo *"facciamo una nota e andiamo avanti"*, *"salviamo per dopo"*, *"non adesso, parcheggiamo"*, *"questa è interessante ma fuori scope"*.
- Claude può proporre proattivamente la cattura quando percepisce un thread tangenziale con dignità propria che rischia di derailare il lavoro principale: *"vuoi che la noti?"*

## Format

**Filename**: `YYYY-MM-DD-<short-slug>.md` (data di cattura + slug descrittivo)

**Frontmatter**:

```yaml
---
captured: 2026-04-26
status: open  # open | parked | promoted-to-{spec|tech-debt|adr|skill} | dropped
context: "breve descrizione di dove è emersa l'idea"
tags: [tag1, tag2]
---
```

**Body**: corto, focalizzato:
- *Cos'è* l'idea
- *Perché* merita
- *Quale* è il next-step minimo se mai si riprende

Non scrivere una spec qui — la cattura è leggera per design. Se l'idea matura, viene **promossa** a uno spec / ADR / tech-debt / skill.

## Lifecycle states

| Status | Significato |
|---|---|
| `open` | Cattura fresca, nessuna decisione ancora |
| `parked` | Esplicitamente differita, da rivisitare in futuro |
| `promoted-to-{spec\|tech-debt\|adr\|skill}` | Promossa a un artefatto più durevole — il file resta come traccia storica con link al nuovo |
| `dropped` | Decisione di non perseguire, file resta come record |

**Quando promuovi**: NON cancellare il file dell'idea. Aggiorna lo `status`, aggiungi un link al nuovo artefatto, mantieni il body originale. Tracciabilità = sapere da dove le cose sono nate.

## Promote workflow

Pseudo-procedura quando un'idea viene promossa:

1. Identificare il target (spec / tech-debt / ADR / skill / convenzione)
2. Creare il nuovo artefatto nella sua location naturale
3. Tornare al file dell'idea, aggiornare frontmatter:
   ```yaml
   status: promoted-to-spec
   promoted_to: ../../.development/specs/in-progress/feature-X.md
   promoted_at: YYYY-MM-DD
   ```
4. Ground truth è il nuovo artefatto da quel momento

## Bootstrap & evoluzione

Questa cartella + convenzione sono nate dalla discussione del 2026-04-26 (vedi `2026-04-26-capture-tangential-ideas.md`), durante il lavoro su `feature/code-graph`. Iniziata come pattern manuale — se si rivela utile, sarà promossa a una skill formale (`/note <text>`) per uso più automatico.

La regola operativa per Claude è in `.claude/rules/idea-capture.md` (auto-loaded ad ogni sessione).
