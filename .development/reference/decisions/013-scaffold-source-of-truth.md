# ADR-013: Scaffold source of truth — rsrc versionato, symlink in dev

**Data**: 2026-06-12
**Status**: Accepted
**Impact**: critical
**Sommario**: Stabilisce `rsrc/project-scaffold/` come unica fonte di verità versionata in git, con `~/.devdash/scaffolds/dev-dash-standard` come symlink ad essa in modalità dev (il promote dell'app scrive nel working tree) e come copia per gli utenti finali, eliminando ogni meccanismo di sync.

## Contesto

Lo scaffold `dev-dash-standard` esisteva in due copie fisiche:

- `rsrc/project-scaffold/` — tracciata in git, curata a mano, ma mai
  aggiornata dai flussi dell'app (al 2026-06-12 mancavano
  `.development/automation/`, `post-merge` e gli hook orchestratori).
- `~/.devdash/scaffolds/dev-dash-standard/` — la copia runtime letta da
  `ScaffoldRepository`, aggiornata dal promote dell'app durante il
  dogfooding (2026-06-11), ma fuori da ogni versionamento.

Il flusso reale del dogfooding va progetto → promote → scaffold runtime:
qualunque modello "rsrc sorgente + copia installata + sync" richiederebbe un
terzo meccanismo di riallineamento (sync-back) e lascerebbe comunque
finestre di drift tra una sincronizzazione e l'altra.

## Decisione

1. **`rsrc/project-scaffold/` è l'unica fonte di verità**, versionata in git.
2. **Sulla macchina di sviluppo**, `~/.devdash/scaffolds/dev-dash-standard`
   è un **symlink** a `rsrc/project-scaffold/`. Non esiste una seconda copia:
   il promote dell'app scrive attraverso il symlink e atterra direttamente
   nel working tree di git, dove la modifica viene revisionata e committata.
   Il dogfooding È il flusso di versionamento.
3. **Per gli utenti finali** (percorso install, ADR-011), lo scaffold viene
   **copiato** in `~/.devdash/scaffolds/`: il symlink è esclusivamente la
   modalità dev/dogfooding.

Stesso rationale di ADR-002 (symlink vs copy per il vault): zero sync da
mantenere, single source of truth, drift strutturalmente impossibile.

## Contenuto dello scaffold

- Artefatti **generati** non vivono nello scaffold: si distribuisce il
  generatore (`generate-claude-config.sh`), non il suo output
  (`key-decisions.md`).
- File **locali per definizione** (`settings.local.json`) sono esclusi.

## Conseguenze

- **Pro**: nessun meccanismo di sync; ogni modifica fatta dall'app allo
  scaffold compare in `git status` e passa dalla review; storia completa
  dello scaffold in git.
- **Contro**: un'operazione distruttiva dell'app sugli scaffold toccherebbe
  il working tree del repo — accettato perché versionato (recuperabile con
  git), ed è anzi il posto più osservabile dove far atterrare le scritture.
- `ScaffoldRepository::Refresh()` usa `entry.is_directory()`, che segue i
  symlink: nessuna modifica al codice necessaria.
- Su Windows i symlink richiedono Developer Mode — irrilevante: riguarda
  solo la modalità dev, gli utenti ricevono una copia (cfr. ADR-002).

## Setup (modalità dev)

```bash
rm -rf ~/.devdash/scaffolds/dev-dash-standard
ln -s /data/repos/dev-dash/rsrc/project-scaffold ~/.devdash/scaffolds/dev-dash-standard
```
