---
captured: 2026-04-26
status: parked
context: "Emersa durante la discussione sulle 8 open question del resource-model. Originariamente accantonata come 'history versioning delle config globali è facile con git, fai git init e basta'. L'utente è tornata sul punto chiarendo che il vero bisogno non è solo storico, ma operativo: poter testare config diverse swappando con un click."
tags: [claude-code, config, dev-dash, multi-feature]
---

# Versioning + swap di configurazioni Claude Code

## L'idea

Un sistema di **profili nominati** sopra l'intera config Claude Code (`~/.claude/`, `~/.claude.json`, e potenzialmente `.claude/` di progetto). Permette di:

- **Snapshot** della config attuale come "profilo X" nominato
- **Switch** istantaneo a un altro profilo (un click in dev-dash, o un comando CLI)
- **Diff** fra due profili
- **List** dei profili disponibili
- (Eventuale) **Cross-machine sync**

## Caso d'uso primario (e generatore)

Testare l'effetto di una modifica alla config sul comportamento di Claude. Esempio concreto: voglio confrontare sessioni con `ARCHITECTURE.md` auto-loaded nel context vs sessioni baseline senza. Oggi devo armeggiare a mano coi file, rischio di confondermi su quale config sto testando di volta in volta. Con i profili: `claude-config switch baseline` → test 1, `claude-config switch with-arch-map` → test 2, `claude-config diff baseline with-arch-map` → conferma di cosa cambia.

## Perché merita

- **Sblocca esperimenti puliti** sulla config — fondamentale per testare ipotesi tipo "questa modifica al prompt cambia davvero il comportamento?"
- **Riduce ansia di rompere il setup**: se snapshotto prima di un esperimento, posso sempre tornare indietro
- **Cattura il "tinkering" che l'utente fa** in modo strutturato invece che caotico
- È **multi-feature** (capture, restore, diff, UI di gestione, eventualmente sync)

## Spazio di design (sketch)

### Backing store

Quattro candidati, in ordine di plausibilità:

1. **Git su `~/.claude/`** — `git init ~/.claude/`, profile = branch git, attivare = `git checkout`. Eredita gratis tutte le primitive che servono (snapshot, swap, diff, log). Caveat: file enormi/binari da escludere (auto-memory cresce continuamente).
2. **Filesystem swap** — directory `~/.claude.profiles/{baseline,with-arch-map,...}/`, attivare = simlink/copy. Semplice ma fragile.
3. **Overlay diff-based** — baseline + diff salvati come patch nominate. Eleganti ma fragili quando la baseline evolve.
4. **Profile-as-data** — config descritta in JSON sintetico, applicato via tool. Ortogonale ai file Anthropic, richiede traduttore.

### Cosa snapshottare e cosa no

- **SÌ**: `CLAUDE.md`, `settings.json`, `agents/`, `skills/`, `hooks/`, MCP (`~/.claude.json`), `output-styles/`, `rules/`
- **NO**: `auto-memory/` (derived state, cresce troppo), conversation history (ditto), `settings.local.json` (gitignored, contiene segreti), credentials/env vars con secrets

### Granularità di scope

- Globale-only (`~/.claude/`) — caso d'uso citato
- Anche per-progetto (`<proj>/.claude/`) — estensione naturale
- Iniziare da globale-only.

### Workflow utente (CLI immaginato)

```
claude-config snapshot baseline
claude-config snapshot with-arch-map
claude-config switch baseline
claude-config diff baseline with-arch-map
claude-config list
claude-config delete experiment-failed
```

Mappa 1-a-1 a comandi git se backing è git.

## Connessione architettonica al resource-model

Questa idea condivide un asse con il **Pilastro 5 (Conversation history)** del resource-model: entrambe sono "evoluzione di stato di Claude nel tempo". Ma le operazioni sono diverse — search per cronologia, snapshot/restore per config. Sono **pilastri sorelli, non lo stesso pilastro**.

Nel resource-model è stata aggiunta una **sezione M placeholder** ("Pilastro 6 — Config versioning") con riferimento a questa nota.

## Decisioni che la spec dovrà sciogliere

1. Backing store: git è la scelta naturale, ma confermare dopo aver verificato che file di auto-memory non rendano il repo ingestibile.
2. Granularità: globale-only first, per-progetto in fase 2.
3. Distribuzione: questo è un sotto-progetto autonomo o un componente di dev-dash? Secondo me autonomo per il backing+CLI, dev-dash come UI sopra.
4. Naming/CLI: `claude-config`? `claude-profile`? Confermare.
5. Migrazione: se l'utente ha già una config setup, come si crea il primo snapshot baseline senza interruzioni?
6. Cross-machine sync: scope iniziale o follow-up?

## Aggiornamento 2026-06-28 — benchmark e convergenza su git

Riemersa discutendo ADR-015/ADR-016 (scaffold templating + versioning git degli
scaffold). Due aggiunte:

1. **Nuovo caso d'uso — benchmark config × versione-codebase.** Oltre a "config A
   vs config B" (caso primario sopra), incrociare un **punto nello storico git
   del *codice* del progetto** ("tornare indietro nella codebase") con una config
   applicata, per misurare differenze di **performance**. È una dimensione in più
   rispetto al config-versioning puro: non solo *quale config*, ma *quale config
   su quale stato del codice*.

2. **Convergenza su git come backing store.** ADR-016 ha deciso git-gestito per
   gli scaffold: git sta diventando il backing store ricorrente del design
   (scaffold ora, config-versioning candidato qui). Rafforza il candidato #1
   ("Git su `~/.claude/`") di questa nota.

**Relazione con `feature-snapshot-history`** (chiarita oggi): NON sostituire gli
snapshot con git. Due livelli coesistono, per bisogni diversi:
- *snapshot leggeri* (pruning, niente branch) = undo rapido delle azioni DevDash;
- *config-versioning git per-progetto* (questa nota, granularità per-progetto) =
  versioning robusto dell'intera config quando serve.
Il timore "casino con i branch a riapplicare config" è proprio il motivo per cui
gli snapshot leggeri restano, e il git-versioning è un layer **aggiuntivo**, non
sostitutivo.

## Promozione futura → spec dedicata

Quando si attacca, **questa nota non si cancella** — diventa il punto di partenza della spec. Status passa a `promoted-to-spec` con link.

Posizione probabile della spec: `.development/specs/planned/feature-config-versioning.md` (o nome più snello).
