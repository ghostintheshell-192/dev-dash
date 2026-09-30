---
type: feature
priority: must-have
status: planned
category: core
related: [feature-scaffold-management, feature-snapshot-history]
depends_on: [feature-scaffold-management]
decided_by: ../../reference/decisions/015-scaffold-templating.md
created: 2026-06-28
---

# Scaffold Templating

> **Estende `feature-scaffold-management`** con la sostituzione dei parametri
> decisa in [ADR-015](../../reference/decisions/015-scaffold-templating.md).
> Rende lo scaffold *riapplicabile*: l'utente applica uno scaffold a un progetto
> nuovo e DevDash lo guida a riempire i parametri specifici del progetto, invece
> di lasciargli una ricognizione manuale su tutti i file.

## Summary

Oggi `apply` e `promote` sono copie pure: lo scaffold contiene placeholder
(`{PROJECT_NAME}`, …) che nessun meccanismo risolve, e applicarlo a un progetto
nuovo lascia all'utente la correzione manuale — non segnalata — dei valori
specifici.

Questa spec introduce il **templating guidato da manifest**:

- lo scaffold vive in forma *template* (parametri come placeholder), i progetti
  in forma *concreta*; `apply` concretizza, `promote` templatizza;
- i parametri sono **dichiarati in un manifest per-scaffold**, non indovinati dal
  testo;
- il manifest è scritto **solo dalla GUI** (non corrompibile per costruzione);
- la sostituzione è **esplicita**: inventory → valori → preview → apply →
  segnalazione dei residui (`required` bloccanti).

Vale per ciascuno degli scaffold multipli che DevDash già gestisce
(`feature-scaffold-management` US-4): ogni scaffold ha il proprio manifest, non
esiste uno scaffold "master".

## Background

`feature-scaffold-management` (implemented) ha realizzato discovery, diff, apply
e promote come operazioni di copia, e ha lasciato in *Open Questions* la sorte
dei placeholder, con inclinazione a eliminarli ("no magic"). ADR-015 rivede
quell'inclinazione: il dilemma "sostituzione = magic" è falso — la sostituzione
è opaca solo se nascosta, mentre è l'edit manuale (residui invisibili) a esserlo
davvero. La sostituzione *esplicita e guidata* realizza la trasparenza meglio
dell'astensione.

## User Stories

### US-1 — Promuovere una config a scaffold, templatizzandola

**Come** utente che ha messo a punto la configurazione di un progetto e vuole
farne uno standard
**Voglio** promuovere la config a scaffold sostituendo i valori specifici del
progetto con parametri
**Per** riusarla su altri progetti senza portarmi dietro ciò che è specifico di
questo.

**Acceptance:**

- L'utente marca quali porzioni diventano parametri — un parametro **nuovo** o
  uno **esistente** già nel manifest.
- La marcatura aggiorna il manifest dello scaffold (scritto dalla GUI).
- Nello scaffold risultante i valori marcati sono sostituiti dai placeholder.
- L'utente vede il diff prima di confermare (come l'attuale promote).
- Promuovere allo scaffold esistente o salvare come scaffold nuovo restano
  entrambe disponibili (US-3 di `feature-scaffold-management`).

### US-2 — Applicare uno scaffold a un progetto nuovo, guidati

**Come** utente su un progetto vergine, non inizializzato
**Voglio** applicare uno scaffold ed essere guidato a riempire i parametri
**Per** ottenere un progetto già pronto senza dover correggere i file a mano.

**Acceptance:**

- DevDash mostra l'**overview di tutti i parametri** del manifest dello scaffold
  scelto: nome, descrizione, `required`/`optional`, eventuale default.
- L'utente inserisce i valori.
- DevDash mostra la **preview** del risultato prima di scrivere.
- All'apply: scrittura transazionale + autosnapshot (come l'attuale apply).
- **Residui**: un parametro `required` non risolto **blocca** l'apply ed è
  segnalato; un `optional` non risolto non blocca (warning).

### US-3 — Gestire i parametri di uno scaffold da pannello

**Come** utente
**Voglio** vedere e modificare i parametri di uno scaffold in un pannello
**Per** rinominarli, marcarli `required`/`optional`, descriverli o rimuoverli —
senza editare un file a mano.

**Acceptance:**

- Un pannello mostra in forma tabellare i parametri dichiarati nel manifest.
- Ogni modifica è scritta dalla GUI nel manifest.
- Non è mai necessario (né previsto) editare il manifest come testo.

## Requirements

### Funzionali

- [ ] **Manifest per-scaffold**: dichiara i parametri dello scaffold (nome,
  descrizione, `required`/`optional`). Uno per scaffold. Scritto **solo dalla
  GUI**.
- [ ] **Inventory**: dato uno scaffold, elencare i suoi parametri (dal manifest)
  per l'overview di apply e per la gestione.
- [ ] **Concretize (apply)**: risolvere i placeholder con i valori forniti e
  scrivere il progetto, in modo transazionale (riuso dell'apply esistente).
- [ ] **Templatize (promote)**: sostituire le porzioni marcate con i placeholder
  e aggiornare il manifest.
- [ ] **Apply guidato**: flusso overview → valori → preview → apply →
  segnalazione residui.
- [ ] **Validazione residui**: `required` non risolto è bloccante; `optional`
  non lo è.
- [ ] **Per-scaffold**: il modello vale indipendentemente per ogni scaffold in
  `~/.devdash/scaffolds/`.

### Non funzionali

- **Trasparenza**: l'utente vede l'inventory e la preview prima che l'app
  scriva. Nessuna sostituzione nascosta.
- **Robustezza del manifest**: scritto solo dalla GUI → non corrompibile a mano.
  Versionato in git in chiaro (GUI = writer, git = storia/diff).
- **Transazionalità**: l'apply resta atomico (o tutto o niente) con autosnapshot
  pre-scrittura.
- **Resilienza**: manifest assente o parziale → l'app non crasha; degrada a
  comportamento prevedibile (es. nessun parametro dichiarato ⇒ apply = copia).

## Acceptance Criteria

- [ ] Applicare uno scaffold con parametri a un progetto vergine, riempiendo i
  valori → l'output non contiene placeholder residui dei parametri risolti.
- [ ] Lasciare vuoto un parametro `required` → l'apply è bloccato e il parametro
  mancante è segnalato; lasciare vuoto un `optional` → l'apply procede.
- [ ] Promuovere una config marcando un valore come parametro → ri-applicare lo
  scaffold a un altro progetto chiede quel parametro.
- [ ] Il manifest non viene mai editato a mano nel flusso: ogni sua modifica
  passa dalla GUI.
- [ ] Uno scaffold senza manifest (o senza parametri) si applica come copia,
  senza errori.

## Technical Notes

- **Innesto sugli engine esistenti**: `ApplyEngine`/`PromoteEngine` oggi copiano
  verbatim. Il templating è uno stadio a monte (inventory + sostituzione) che
  trasforma il contenuto prima/durante la copia; valutare se come nuovo
  `TemplateEngine` o come estensione degli engine esistenti.
- **Manifest per-scaffold**: vive dentro la cartella dello scaffold (location e
  formato esatti → Open Questions).
- **Pannello manifest**: segue il pattern "panel as viewmodel" (ADR-010), come
  gli altri pannelli `ui/`.
- **Autosnapshot**: l'apply guidato riusa l'autosnapshot pre-apply di
  `feature-snapshot-history`.

## Open Questions

Ereditate da ADR-015, da chiudere in fase di design/implementazione:

- **Formato e schema del manifest** (es. TOML o JSON indentato) e dove vive
  fisicamente nello scaffold.
- **Forme multiple di uno stesso parametro** (display name / slug /
  identificatore di codice): come dichiararle e risolverle in un colpo.
- **Meccanica della marcatura** durante il promote (interazione UI).
- **Default e inferenze** (es. nome progetto dal nome cartella): quali, e dove
  vengono calcolate.
- **Registrazione della mappa di risoluzione** a un apply (relazione con
  `feature-snapshot-history`).

## Decisioni ereditate da scaffold-management

Open Questions di `feature-scaffold-management` risolte il 2026-06-28 e **migrate
qui** (sono lavoro attivo, non record d'archivio):

- **Default scaffold** → file marker **`.default-scaffold`** (namespace sul *tipo
  di risorsa*; convenzione generale `.default-<risorsa>`), **gitignored**
  (preferenza locale, non contenuto). *Impatto codice*:
  `ScaffoldRepository::SetDefault`/`Refresh` usano oggi `.devdash-default` (+ file
  committato in `rsrc/project-scaffold/`) — rename da fare.
- **Permessi per stack** → **Opzione A**: scaffold per-stack (`coding-cpp`,
  `coding-rust`, …), ognuno col proprio manifest e `.default-scaffold`. Costo
  noto: duplicazione delle parti language-agnostic. Un assist stile-B (proponi
  `coding-cpp` vedendo `CMakeLists.txt`) resta comodità *sopra* A, non alternativa.
- **Scaffold seed** → **Opzione B**: seed del primo scaffold da
  `rsrc/project-scaffold/` (risorsa versionata, non blob — obiezione "embedded"
  superata da ADR-013), copiato in `~/.devdash/scaffolds/` al primo avvio.
- **Granularità del diff** → MVP **file intero**; la granularità per-sezione
  markdown è evoluzione **ortogonale** (layer semantico parse-heading, non scelta
  di algoritmo né libreria).
- **Conflict resolution su apply** → MVP **skip-with-confirmation**; il three-way
  merge richiede di registrare la base a ogni apply (vedi
  [ADR-015](../../reference/decisions/015-scaffold-templating.md) OQ "mappa di
  risoluzione") ed è eseguibile con `git merge-file`.

(La OQ "Variabili / placeholder" è la **genesi** di questa spec e di ADR-015, non
ripetuta qui.)

## Evoluzione post-MVP

Direzioni decise ma fuori dall'MVP:

- **Compute del diff via git.** Per l'MVP il diff di contenuto usa l'LCS interno
  (`core/line_diff.h`, file interi, config piccole). Post-MVP si passa a un
  compute più performante basato sui comandi git (`git diff`), già nello stack
  grazie al versioning gestito degli scaffold — togliendo il cap a 2000 righe e
  la tabella DP O(m·n). È un tassello dell'integrazione *progressiva* di git
  nella codebase (vedi la decisione sul versioning degli scaffold).
- **Visualizzazione del diff.** Il rendering resta ImGui-native
  (`ui/file_diff_panel`), migliorato in modo incrementale. Nessun diff-tool
  esterno: non è embeddabile in ImGui e romperebbe la coerenza visiva.
- **Affordance di allineamento esplicita.** Lo stato "inizializzato/allineato"
  è letto dal drift, non da un flag persistente (un boolean sarebbe pure mal
  posto con scaffold multipli). Un indicatore sintetico esplicito (es. % di
  allineamento a *scaffold X*) è **deferred**: da valutare pilotando DevDash su
  se stesso, non da costruire a priori.

## Related

- [ADR-015](../../reference/decisions/015-scaffold-templating.md) — decisione di
  principio.
- `feature-scaffold-management` — feature estesa (Open Question "Variabili /
  placeholder" risolta qui).
- `feature-snapshot-history` — autosnapshot pre-apply.
- ADR-013 — scaffold come fonte di verità versionata.
