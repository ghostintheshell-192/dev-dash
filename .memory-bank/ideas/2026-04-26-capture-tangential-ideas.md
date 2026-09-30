---
captured: 2026-04-26
status: promoted-to-mechanism
promoted_to: ./README.md
promoted_at: 2026-04-26
context: "Emersa durante la sessione su feature/code-graph, mentre l'utente notava che il bootstrap della spec del code-graph aveva interrotto il flusso di lavoro principale sul resource-model"
tags: [workflow, meta, claude-code]
---

# Capture mechanism per idee tangenziali

## L'idea

Durante una conversazione di lavoro emergono spesso idee adiacenti — non rumore, ma cose che meritano davvero attenzione, semplicemente non *adesso*. Esempio canonico generatore: durante il lavoro sul resource-model è emersa la spec del code-graph + UML view; ottima idea, abbiamo dovuto interromperci per fare lo skeleton, e questo ha rotto il flow.

Serve un meccanismo per **salvare l'idea senza fermarsi**: l'utente dice "facciamo una nota e andiamo avanti", Claude scrive una nota strutturata in una location dedicata, si torna al thread principale.

## Perché merita

- Le idee non scritte muoiono o si trasformano in carico cognitivo
- Interrompere il workflow per ogni tangente è costoso
- Una location dedicata (separata dagli handoff di sessione) le rende ritrovabili e promovibili
- Serve a Claude (che le scrive) ma anche all'utente (che decide se promuoverle a spec/ADR/skill)

## Promozione → Diventata `.memory-bank/ideas/`

Vedi `README.md` in questa cartella per la convenzione completa: filename `YYYY-MM-DD-<slug>.md`, frontmatter con `captured`/`status`/`context`/`tags`, lifecycle (open → parked → promoted-to-X → dropped), regola Claude in `.claude/rules/idea-capture.md`.

Nota meta deliziosa: questa idea è stata catturata **da sé stessa** — il primo idea-note creato è quello che documenta il meccanismo, e si auto-promuove al meccanismo stesso. Eleganza ricorsiva.

## Possibile evoluzione futura

- Se la convenzione manuale tiene, promuoverla a una **skill formale** (es. `/note <text>` slash command) per ridurre ulteriormente la friction.
- Dev-dash dovrebbe esporre un panel "open ideas" con azioni di promozione (move-to-spec, move-to-tech-debt, ecc.) — vedi resource-model.md sezione G dove "Tangential idea note" è ora una risorsa first-class.
