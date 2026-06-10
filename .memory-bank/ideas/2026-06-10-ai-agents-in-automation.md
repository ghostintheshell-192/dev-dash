---
captured: 2026-06-10
status: parked
context: "emersa durante la review di ADR-012 (automazione agnostica), branch docs/release-and-automation-plan"
tags: [automation, ai-agents, adr-012, self-improving]
---

# Agenti AI trigger-based nell'automazione di progetto

## Cos'è

Estendere il modello di automazione a due livelli (ADR-012) con un terzo tipo
di attore oltre a hook e script: **agenti AI che si attivano su trigger
determinati**. Criterio di soglia esplicito di Valentina: devono servire per
qualcosa di *importante* — per spostare un file bastano gli script. L'agente
entra dove serve giudizio, sintesi o comprensione del contesto.

DevDash come "progetto che migliora se stesso": primo laboratorio per questo
pattern, sia come testbed dell'infrastruttura sia — prospetticamente — come
feature del prodotto (DevDash gestisce contesto e documentazione: agenti che
la *curano* sono coerenti con la visione).

Esempi di calibro giusto (da vagliare nel brainstorming, non decisi):
sintesi semantica della sessione, drift detection tra documentazione e
codice, review della coerenza spec ↔ implementazione al merge, triage dei
tech-debt. Esempi sotto soglia: spostare file, rigenerare indici, formattare.

## Perché merita attenzione futura

- Si innesta naturalmente sul contratto degli entry point ADR-012 (un agente
  può essere il "corpo" di un entry point o di un hook, con la stessa
  interfaccia exit-code).
- È il ponte tra l'infrastruttura di automazione e la visione di DevDash
  (documentation-first, contesto governato).
- Richiede decisioni non banali: trigger (git event? sessione? cron?),
  perimetro di autonomia, costo, dove vive la configurazione nel modello a
  due livelli.

## Next-step minimo

Sessione di brainstorming dedicata con Valentina: definire 2-3 casi d'uso
sopra-soglia, scegliere il primo esperimento, decidere se diventa parte di
ADR-012 (esteso) o ADR nuovo.
