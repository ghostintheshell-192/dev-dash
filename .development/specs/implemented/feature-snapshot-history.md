---
type: feature
priority: must-have
status: implemented
category: core
part_of: wedge
related: [feature-effective-config-view, feature-scaffold-management]
depends_on: [feature-scaffold-management][../../reference/technical/resource-model.md]
created: 2026-05-10
---

# Snapshot & History

> **Parte 3/3 della wedge feature di DevDash.** Permette di salvare stati
> intermedi della config del progetto corrente (espliciti dall'utente +
> automatici prima di operazioni distruttive di DevDash) e di tornarci.
> Le altre due parti sono `feature-effective-config-view.md` (parte 1) e
> `feature-scaffold-management.md` (parte 2).

## Summary

Uno snapshot è una copia della configurazione corrente di un progetto
salvata sotto `~/.devdash/snapshots/<project-slug>/<timestamp>-<slug>/`,
ripristinabile con un'azione esplicita.

Strutturalmente uno snapshot è identico a uno scaffold (cartella di file
copy). Differisce solo per:

| | Scaffold | Snapshot |
|---|---|---|
| **Scope** | Cross-project | Project-specific |
| **Intent** | Riuso (applico ovunque) | Backup (storia di *questo* progetto) |
| **Storage** | `~/.devdash/scaffolds/<name>/` | `~/.devdash/snapshots/<project-slug>/<timestamp>-<slug>/` |
| **Restore =** | Apply | Apply (forzato, perché *vuoi* sovrascrivere) |
| **UI** | "Scaffolds" library | "History of this project" panel |

Gli snapshot sono di due tipi:
- **Espliciti**: l'utente li crea manualmente con un nome, per avere un
  punto di ritorno nominato.
- **Automatici**: DevDash li crea prima di ogni operazione che sovrascrive
  file (apply, promote-overwriting, restore), per garantire reversibilità
  delle proprie azioni.

Importante: gli autosnapshot **non vengono fatti a ogni edit dell'utente**
sui file del progetto. L'editing fuori da DevDash (editor di testo, IDE)
è coperto da git. Gli autosnapshot proteggono dalle azioni di DevDash, non
sostituiscono il versioning del progetto.

## Background

Gap emerso durante la review della wedge feature: tra "tweak in-place" e
"promote-to-scaffold" esiste un terreno intermedio in cui l'utente vuole
un punto di salvataggio del proprio progetto senza ancora promuoverlo
a scaffold riutilizzabile. Senza snapshot l'utente è costretto a:

- Promuovere troppo presto (inquinando lo scaffold con stati intermedi),
- Tenere la modifica solo in memoria/git (uscendo dalla dashboard),
- Rischiare di perderla se DevDash applica un altro scaffold sopra.

## User Stories

### US-1 — Salvare snapshot esplicito della config corrente

**Come** utente che ha tweakato la config del progetto e vuole un punto
di ritorno
**Voglio** salvare uno snapshot nominato dello stato attuale
**Per** poter riprovare modifiche diverse senza paura di perdere il lavoro.

**Acceptance:**

- L'utente clicca "Save snapshot" → inserisce nome + descrizione opzionale.
- Lo snapshot è una copia della cartella `.claude/` (e degli altri pezzi
  scaffold-relevant del progetto) salvata sotto
  `~/.devdash/snapshots/<project-slug>/<timestamp>-<slug>/`.
- Lo snapshot è visibile nella "History" del progetto corrente.
- Lo snapshot **non** appare nella lista degli scaffold riutilizzabili
  (parte 2/3): è specifico al progetto.

### US-2 — Ripristinare da snapshot

**Come** utente
**Voglio** ripristinare uno snapshot precedente del progetto corrente
**Per** tornare a uno stato salvato.

**Acceptance:**

- L'utente seleziona uno snapshot dalla History → "Restore".
- Il restore è un apply forzato (sovrascrittura): default è ripristinare
  *tutto* lo snapshot, con la possibilità di selezione granulare come
  per l'apply scaffold.
- Prima del restore, DevDash crea automaticamente un autosnapshot
  (`auto-pre-restore-...`) per garantire che anche il restore sia reversibile.

### US-3 — Autosnapshot prima di operazioni distruttive

**Come** utente
**Voglio** che DevDash crei automaticamente uno snapshot prima di
operazioni che sovrascrivono file (apply scaffold, promote-overwriting,
restore)
**Per** avere sempre un punto di ritorno anche quando non ho ricordato
di farlo manualmente.

**Acceptance:**

- Prima di ogni apply/promote-overwriting/restore, DevDash crea uno
  snapshot con marker `auto-` e nome che descrive l'azione
  (`auto-pre-apply-scaffold-coding`, `auto-pre-restore-2026-04-30-1500`).
- Gli autosnapshot sono visibili in History, marcati visualmente come
  automatici.
- Politica di pruning: gli ultimi N autosnapshot per project sono
  preservati (default N=10, configurabile); i più vecchi vengono
  cancellati. Gli snapshot espliciti (US-1) **non** vengono mai pruned
  automaticamente.

## Requirements

### Funzionali

- [x] **Snapshot engine**: salvataggio della config corrente del project
  in `~/.devdash/snapshots/<project-slug>/<timestamp>-<slug>/`. Trigger
  espliciti (US-1) e automatici prima di apply/promote/restore (US-3).
- [x] **Restore engine**: apply forzato di uno snapshot al project di
  origine (US-2). Tecnicamente è una variante dell'apply engine della
  parte 2/3 con default conflict-resolution = overwrite invece di
  skip-with-confirmation.
- [x] **Pruning**: cancellazione automatica degli autosnapshot oltre il
  threshold N (default 10, configurabile). Mai applicato a snapshot
  espliciti.
- [x] **Project-slug resolution**: basename del path sanitizzato (MVP;
  nota: collisioni possibili se due progetti hanno lo stesso nome).
- [x] **Path configuration**: `~/.devdash/snapshots/` è il default,
  l'utente può override-arlo via config.

### Non funzionali

- **Storage cost**: gli snapshot non devono crescere senza controllo.
  Pruning automatico per autosnapshot.
- **Reversibilità**: ogni operazione distruttiva di DevDash è reversibile
  (apply, promote-overwriting, restore tutti generano un autosnapshot
  pre-azione).
- **Scoping**: lo snapshot di un progetto non è visibile/applicabile da
  un altro progetto. Mai.

## Acceptance Criteria

- [x] Salvare snapshot manuale, fare modifiche, salvare un secondo snapshot,
  restore al primo → il progetto torna allo stato del primo snapshot e un
  autosnapshot `auto-pre-restore-...` appare nella history.
- [x] Applicare uno scaffold a un progetto già configurato → un autosnapshot
  `auto-pre-apply-...` appare nella history e contiene la config pre-apply
  intatta.
- [x] Lasciare accumulare > N autosnapshot (default N=10) → i più vecchi
  vengono pruned, gli snapshot espliciti restano.
- [x] Modificare un file nel progetto con un editor esterno → **nessun**
  autosnapshot viene creato. (Editing fuori da DevDash è coperto da git.)

## Technical Notes

### Modello di dominio

- `Snapshot` — path assoluto a
  `~/.devdash/snapshots/<project-slug>/<timestamp>-<slug>/` + metadata
  (tipo: explicit/auto, descrizione, timestamp, riferimento al
  project di origine).
- `History` — vista derivata: lista cronologica degli snapshot per il
  project corrente, con indicazione del tipo (explicit/auto).

### Decisioni architetturali implicate

- **Snapshot e Scaffold riusano la stessa primitiva di basso livello**
  (filesystem copy + diff). Cambia solo il "contenitore" (snapshots/ vs
  scaffolds/) e il default del conflict-resolution su restore (overwrite)
  vs apply (skip-with-confirmation).
- **Restore è un apply specializzato**, non un sistema separato.

## Open Questions

- **Pruning policy default**: N=10 è un guess. Da rivedere dopo l'uso
  reale.
- **Project-slug resolution**: hash del path è robusto ma opaco; basename
  è leggibile ma collidente; metadata in-project è pulito ma richiede
  scrittura dentro il progetto. Da decidere.
- **Snapshot migration**: se l'utente sposta il progetto su filesystem
  (path cambia), gli snapshot indicizzati per `<project-slug>` derivato
  dal path vecchio vanno persi? UI per "ricollegare"? Out of MVP, ma
  importante per la semantica di portabilità.
- **Visibilità degli autosnapshot in UI**: di default visibili o
  nascosti dietro un toggle "show auto"? Inclinazione: visibili ma
  visivamente attenuati, perché nasconderli rischia di sorprendere
  l'utente quando il pruning li rimuove.

## Related

- `feature-effective-config-view.md`: parte 1/3.
- `feature-scaffold-management.md`: parte 2/3 (l'apply engine è
  riusato qui per il restore; gli autosnapshot prima di apply/promote
  sono triggered da quella spec).
- ADR-008, ADR-009: stack di implementazione.
