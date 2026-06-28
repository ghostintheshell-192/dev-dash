# ADR-015: Scaffold templating — sostituzione esplicita guidata da manifest

**Data**: 2026-06-28
**Status**: Accepted
**Impact**: high
**Sommario**: Rifonda `apply`/`promote` dello scaffold come trasformazioni inverse fra forma *template* (placeholder) e forma *concreta*: la risoluzione dei parametri è esplicita e guidata da un manifest dichiarato (editato solo da GUI, residui `required` bloccanti), superando l'inclinazione "no-substitution" di `feature-scaffold-management`.

## Contesto

ADR-013 stabilisce lo scaffold come artefatto utente versionato in git. La spec
`feature-scaffold-management` (implemented) ha realizzato `apply` e `promote`
come **copia pura** (`ApplyEngine`/`PromoteEngine` copiano file verbatim,
`overwrite_existing`), e ha lasciato in *Open Questions* la sorte dei placeholder
`{PROJECT_NAME}`, `{TECH_STACK_DESCRIPTION}` ecc., con inclinazione a
**eliminare** ogni sostituzione — marcata fra gli anti-pattern come
"string-replacement opachi", in nome di "trasparenza, no magic".

Quell'inclinazione era esplicitamente non risolta, ed è stata rimessa in
discussione perché materialmente fragile:

- Lo scaffold **resta pieno di placeholder** (e i generatori ne introducono
  altri), ma nessun meccanismo li risolve all'apply.
- Applicare lo scaffold a un progetto nuovo lascia quindi all'utente una
  **ricognizione manuale** su tutti i file per correggere i valori specifici
  del progetto — valori che **non gli vengono segnalati**: annegano nel testo.
- Uno scaffold che non si può riapplicare con le dovute correzioni, in modo
  affidabile, vanifica lo scopo stesso dell'artefatto.

Il dilemma della spec ("sostituzione = magic") è **falso**: la sostituzione è
magica solo se *nascosta*. È invece l'edit manuale a essere opaco — i residui
sono invisibili e nessuno li segnala.

## Decisione

1. **Due forme, due trasformazioni inverse.** Lo scaffold vive sempre in forma
   *template* (parametri come placeholder); i progetti in forma *concreta*.
   `apply` concretizza (template → concreto), `promote` templatizza
   (concreto → template). Sono le due direzioni che attraversano il confine
   scaffold↔progetto.

2. **Sostituzione esplicita e guidata, non automatica-nascosta.** "No magic"
   significa *mostrare*, non *astenersi*. Il percorso è: inventory dei parametri
   → raccolta dei valori → preview del risultato → apply transazionale (con
   autosnapshot, come già previsto) → segnalazione dei residui.

3. **I parametri sono dichiarati in un manifest, non indovinati.** Un manifest
   accompagna lo scaffold e dichiara i suoi parametri. È la spina dorsale di
   tutte e tre le operazioni: `apply` (overview + validazione), `promote`
   (cosa templatizzare, senza indovinare dal testo), e segnalazione
   (`required` vs `optional`).

4. **Il manifest è scritto solo dalla GUI.** L'utente non lo edita come testo:
   lo popola attraverso i gesti dell'app (marcatura durante il promote, pannello
   di gestione parametri). Questo rende il manifest **non corrompibile per
   costruzione** — la sintassi non passa mai per le mani dell'utente. È
   versionato in git in chiaro: la GUI è il *writer* canonico, git è
   *storia e diff* (l'utente lo legge nel diff, non lo edita a mano).

5. **Residui `required` bloccanti.** All'apply, un parametro `required` non
   risolto blocca l'operazione e viene segnalato; un `optional` non blocca.

6. **Per-scaffold, non un master unico.** Il modello si applica a ciascuno
   degli scaffold multipli che DevDash già gestisce
   (`feature-scaffold-management` US-4: `~/.devdash/scaffolds/<name>/`, ognuno
   indipendente, default via marker `.devdash-default`). Ogni scaffold porta
   il **proprio** manifest; non esiste uno scaffold "master" globale. Il
   templating non cambia la molteplicità: come l'utente mantiene più
   configurazioni per progetto, mantiene più scaffold generici riutilizzabili.

## Rationale

- **"No magic" = trasparenza attiva.** Mostrare l'inventory, far inserire i
  valori, mostrare la preview e segnalare i residui realizza la filosofia
  DevDash *meglio* dell'edit manuale, che è la versione davvero opaca.
- **Determinismo del promote.** Senza parametri dichiarati, templatizzare
  richiederebbe di *indovinare* quali stringhe del testo concreto sono
  parametri — fonte di falsi positivi e danno invisibile (templatizzare la
  cosa sbagliata in silenzio). Il manifest sposta la decisione dall'euristica
  alla dichiarazione esplicita.
- **Coerenza di prodotto sul manifest GUI-only.** Un gestore visuale di
  configurazioni che costringe a editare un file a mano si auto-contraddice;
  la GUI come unico writer è insieme coerente con la premessa del prodotto e
  guard-rail strutturale contro la corruzione del file.
- **Riapplicabilità.** Era il punto materiale all'origine della ri-decisione:
  rendere lo scaffold uno standard che si può davvero ri-applicare a progetti
  nuovi, con le correzioni del caso, senza ricognizione a mano.

## Conseguenze

### Pro

- Lo scaffold diventa riapplicabile a progetti nuovi senza ricognizione manuale
  dei valori specifici.
- Il `promote` è affidabile e deterministico (parametri dichiarati).
- La filosofia "trasparenza, no magic" è rafforzata, non contraddetta.
- Il manifest è robusto per costruzione (mai scritto a mano).

### Contro

- Più superficie da costruire rispetto alla copia pura: un motore di
  templating (inventory + sostituzione) e un pannello per il manifest.
- Il manifest è un nuovo artefatto da mantenere nello scaffold e in git.
- La prima dichiarazione di un parametro resta un gesto esplicito dell'utente.

## Open Questions

Da chiudere nella spec dedicata, non fissate qui:

- **Formato del manifest** (es. TOML o JSON indentato) e suo schema.
- **Forme multiple di uno stesso parametro** (es. display name / slug /
  identificatore di codice): come dichiararle e risolverle.
- **Meccanica della marcatura** durante il `promote` (interazione UI).
- **Default e inferenze** (es. nome progetto dal nome cartella): quali, e dove
  vengono calcolate.
- **Relazione con `feature-snapshot-history`**: se e come registrare la mappa
  di risoluzione usata a un apply. *(Aggancio emerso 2026-06-28: questa mappa è
  anche la **base** per un three-way merge sui conflitti d'apply — vedi OQ
  "Conflict resolution" in `feature-scaffold-management`.)*

## Related

- ADR-013 (scaffold come fonte di verità versionata) — vincolo a monte.
- `feature-scaffold-management` (Open Question "Variabili / placeholder") — è la
  decisione che questo ADR rivede.
