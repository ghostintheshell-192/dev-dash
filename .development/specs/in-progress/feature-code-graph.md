---
type: feature
priority: must-have
status: in-progress
category: ui
related: [feature-ui-overhaul]
depends_on: []
decided_by: ../../reference/decisions/014-germen-coevolution-strategy.md
created: 2026-04-26
updated: 2026-10-05
---

# Code Graph — class diagram on demand

> **Riscritta il 2026-09-26** per lo stack attuale (C++/ImGui) e per la
> libreria diagrammi di Dario, [ImGuiDot](https://github.com/DPD85/ImGuiDot).
> La versione di aprile (pre-pivot, C#/Roslyn/Mermaid) è nella storia git.
> Da quella sopravvive il principio: **un solo modello dei dati, più viste**.

## Summary

dev-dash mostra il **class diagram UML** del progetto aperto, generato dal
codice e quindi sempre aggiornato. Il diagramma però **non è mai quello
dell'intero progetto**: con centinaia di classi diventa un groviglio
illeggibile. L'utente sceglie prima le classi che gli interessano da un elenco,
poi clicca "Crea diagramma". Nel diagramma vede le classi scelte e, attenuati,
i loro vicini diretti, così produttori e consumatori restano visibili e il grafo
si può esplorare un passo alla volta.

Il primo linguaggio è **C++**, che è anche il linguaggio di dev-dash, Germen e
ImGuiDot: tre progetti reali su cui provarlo fin da subito.

## Motivation

- `ARCHITECTURE.md` è un albero di file: orienta, ma non mostra le
  **relazioni** (chi eredita da chi, chi contiene chi, chi usa chi).
- I diagrammi disegnati a mano invecchiano appena il codice cambia. Quelli
  generati dal codice no, e questo è coerente con la filosofia
  documentation-first di dev-dash.
- Il class diagram è il livello "code" del modello C4. Resta la notazione di
  riferimento a quel livello; il suo limite reale è la scala, non l'età, e la
  selezione esplicita risolve proprio quello.

## User stories

- **US-0 — Leggere il codice**: clicco "New code analysis" e si apre una
  scheda con le cartelle del progetto che contengono C++. Scelgo quali
  leggere e clicco "Analyze". La scheda mi dice cosa è stato letto: i file,
  i file letti solo in parte, dove e perché.
- **US-1 — Scegliere cosa vedere**: accanto al diagramma, dentro la sua
  scheda, vedo le classi del progetto in un albero per namespace o per
  cartella. Ogni voce ha una casella di spunta, a qualunque livello (un clic
  seleziona un modulo intero), e in cima c'è un campo di ricerca. Quello che
  spunto lì cambia quel diagramma, e solo quello.
- **US-2 — Creare il diagramma**: clicco "New diagram" e si apre una scheda
  ancorabile, con i filtri aperti; quello che spunto entra nel diagramma,
  che si aggiorna subito.
- **US-3 — Vedere i vicini**: nel diagramma compaiono anche i vicini diretti
  (a un passo) delle classi selezionate, come **nodi fantasma**: attenuati,
  solo col nome, senza membri. Posso nasconderli con un'opzione.
- **US-4 — Esplorare**: cliccando un nodo fantasma lo aggiungo alla
  selezione e il diagramma si aggiorna.
- **US-5 — Sempre aggiornato**: se modifico un file del progetto, l'elenco e
  il diagramma aperto si aggiornano senza che io debba chiederlo.

## Design

### Separazione delle responsabilità

Il confine fra dev-dash e ImGuiDot è il **testo DOT**:

- **dev-dash** estrae la struttura dal codice, la tiene come modello, genera il
  DOT delle classi selezionate e dei loro vicini.
- **ImGuiDot** riceve il DOT, calcola il layout con Graphviz e lo disegna con
  la draw list di ImGui. Resta una libreria generica, senza nessuna conoscenza
  di UML o di dev-dash.

Questo soddisfa il vincolo di ADR-014 (modulo grafi portabile, layout e disegno
separati) e chiude la domanda che l'ADR lasciava aperta fra Graphviz e un
algoritmo di layout proprio: si usa Graphviz, tramite ImGuiDot.

### Layer (ADR-010)

| Layer | Cosa aggiunge |
| ----- | ------------- |
| `core/` | Tipi dei dati: classe (nome, namespace, file, membri), membro (nome, tipo, visibilità, metodo/attributo), relazione (da, a, tipo). |
| `services/` | **Estrattore** (codice → modello, vedi Fase 1) e **generatore DOT** (modello + selezione → testo). Il generatore è una funzione pura, testabile con Catch2 senza stack grafico. |
| `ui/` | Sezione Code graph della sidebar (comandi), scheda dell'analisi, schede di diagramma con i loro filtri, che usano ImGuiDot con l'interfaccia a stato in cache (`Update` quando cambia il DOT, `Draw` a ogni frame). Menu View nella top bar. Vedi *Interazione*. |

Segue la regola del `feature-ui-overhaul`: sidebar = struttura, area di lavoro
= contenuto. La selezione a caselle riprende il modello d'interazione della
vista Compare. I filtri, che sono dati di un diagramma, stanno nella sua
scheda, non nella sidebar.

### Interazione (decisa il 2026-10-05)

Il prototipo della sidebar a voci annidate (`experiment/sidebar-toolbar`,
2026-10-02) ha mostrato due difetti. Le voci che *eseguono* qualcosa
("Analyze code", "Project folders") non si riconoscono come comandi: in una
sidebar l'occhio legge navigazione. E la scelta delle cartelle da leggere e la
vista per cartella dei filtri sembravano la stessa cosa due volte. Da qui la
divisione in tre luoghi, ciascuno con un compito solo, più un menu.

**1. La sezione Code graph della sidebar: poche voci, tutte comandi chiari.**

- **New code analysis**, prima voce, sempre attiva: apre la scheda
  dell'analisi. Non legge niente da sola: la lettura parte dal pulsante
  "Analyze" della scheda, dopo aver visto le cartelle.
- **New diagram**: apre una scheda di diagramma vuota, con i filtri aperti.
  Oscurata finché il codice non è stato letto.

**2. La scheda dell'analisi: cosa è stato letto e da dove.**

Una scheda del workspace, come i diagrammi. Dall'alto (rivista il
2026-10-05 dopo la prima prova):

- il pulsante **"Analyze"**, sempre con questo nome (anche per rileggere),
  e accanto quanti file e quante classi sono stati letti e quando, oppure
  "not analyzed yet";
- l'eventuale errore della lettura, con il messaggio catturato;
- **l'albero delle cartelle da leggere**, visibile da subito e tutto aperto,
  perché si capisca che è un selettore. Le cartelle si trovano all'apertura
  della scheda con una sola visita dell'albero, senza leggere il codice
  (`CppClassExtractor::Scan`); la regola di default lascia fuori `test` e
  `tests`;
- dopo la lettura, e solo allora: i **file letti** (chiusi) e i **file letti
  in parte**, in una tabella con colonne etichettate: *File*, *Line*, *Not
  understood* (il motivo: un token mancante, "expected a type identifier",
  o il testo saltato, con "(a macro?)" se comincia con una parola in
  maiuscolo), *Code* (la riga).

La scelta delle cartelle sta qui, accanto al suo effetto, e non fra i filtri,
perché non è un filtro: decide fra quali file si cercano i nomi delle classi.
Leggere anche una cartella che contiene copie delle stesse classi (`poc/`)
renderebbe ambigue le relazioni di tutto il progetto. La vista per cartella
dei filtri invece sfoglia soltanto le classi già lette.

**3. Le schede di diagramma: ogni diagramma ha i suoi filtri.**

**Ogni diagramma ha la sua selezione** (scelto il 2026-10-05), e i filtri che
la mostrano e la modificano vivono **dentro la scheda**: un riquadro a destra del
diagramma, che si apre e si chiude da un pulsante della toolbar della scheda.
Contiene la ricerca, la scelta della vista (namespace | cartelle), l'albero
delle classi con le caselle e "Clear selection". Ogni modifica rigenera il
diagramma della scheda.

Il legame fra filtri e diagramma è così spaziale, e non si può sbagliare: con
due diagrammi affiancati, ognuno ha i suoi filtri sotto gli occhi. Il docking
non cambia: si aggancia e si sposta la scheda intera, filtri compresi. Il costo
è la larghezza che l'albero toglie al diagramma, per questo il riquadro si
chiude e lascia tutta la tela al diagramma.

La toolbar della scheda contiene anche "Neighbours" e la scelta dei membri da
mostrare, sempre visibile perché chi guarda il diagramma si accorga di cosa
esclude (risolve OQ-2):

`Members: [x] public  [ ] protected  [ ] private`

Tre caselle indipendenti: pubblici spuntati, protetti e privati no. Si può
chiedere anche "solo privati". Sostituiscono "All members"; il generatore
passa da una soglia d'accesso (`memberAccess`) a un insieme di livelli.

Alternative scartate:

- **Un pannello dei filtri unico e mobile**, alla Photoshop, legato alla
  scheda attiva. Il legame sarebbe invisibile: con due diagrammi affiancati,
  "attivo" è l'ultimo cliccato, e si modifica facilmente quello sbagliato. I
  pannelli di Photoshop reggono perché sono strumenti generici; i filtri sono
  dati del diagramma, e stanno con lui. I pannelli mobili restano adatti agli
  strumenti davvero generici.
- **I filtri preparano, "New diagram" fotografa** (il modello del prototipo):
  semplice, ma il diagramma non si può più correggere.
- **Snapshot dei filtri** caricati esplicitamente: chiaro, ma ogni modifica
  diventa salva-carica. È però l'idea delle **viste salvate**, che arriva dopo,
  sopra questo modello: un diagramma si salva come vista e, riaperto, la
  scheda dice da quale vista viene (vedi *Out of Scope*).

**4. Il menu View nella top bar** (risolve OQ-5): apre le viste del workspace,
cioè Config, History, Compare, Code analysis e l'anteprima DOT (oggi il
pulsante "Diagram" della top bar, che si confonde con "New diagram").

**La disposizione dei pannelli si ricorda** fra un avvio e l'altro, in
`~/.devdash/layout.ini` (prima ImGui non la salvava: `io.IniFilename =
nullptr`).

I pannelli restano dentro la finestra dell'app: portarli fuori, in finestre
del sistema operativo (il *multi-viewport* di ImGui, per esempio per usare un
secondo monitor), è fuori scope.

### Notazione (UML pragmatico, non puristico)

- **Riquadro classe** a scomparti: nome / attributi / metodi. Di default solo
  i membri pubblici; molteplicità e nomi di ruolo omessi.
- **Ereditarietà**: freccia con triangolo vuoto (`arrowhead=onormal`).
- **Composizione / aggregazione**: rombo pieno / vuoto (`diamond` /
  `odiamond`), dedotti dal tipo del membro (valore o `unique_ptr` →
  composizione; riferimento o puntatore semplice → aggregazione).
- **Dipendenza** (usa il tipo in una firma, senza esserne membro): freccia
  tratteggiata (`style=dashed`).
- **Nodi fantasma**: solo nome, colori con trasparenza.
- I colori seguono il tema di dev-dash ("Grafite & Ambra") e vengono scritti
  esplicitamente nel DOT, così il risultato non dipende dai default di
  ImGuiDot.

### Aggiornamento e reattività

- L'estrazione gira **in un thread di background**: l'interfaccia non si
  blocca mai (principio "responsiveness as requirement").
- Dopo la prima estrazione completa, si ri-estrae **solo il file modificato**.
- Graphviz non è thread-safe, quindi `ImGuiDot::Update` gira sul thread
  principale. Con la selezione i grafi restano piccoli e il layout è
  istantaneo.
- dev-dash oggi non osserva i file del progetto: serve un meccanismo di
  notifica (inotify su Linux, oppure un controllo periodico delle date di
  modifica). La scelta si fa in Fase 3.

## Work Breakdown

### Fase 0 — Prerequisiti

- [ ] PR `fix/gcc13-sqrt` mergiata in ImGuiDot. Senza, la CI di dev-dash
      (GCC 13 su `ubuntu-latest`) non compila la libreria.
- [ ] PR `fix/diagram-layout-size` mergiata. Senza, il diagramma non occupa
      spazio nel layout e non scorre.
- [ ] ImGuiDot aggiunto ad `app/external/CMakeLists.txt` via CPM (Graphviz
      15.1.0 compilato dai sorgenti; richiede `bison` e `flex` sulla macchina
      di build e in CI).

### Fase 1 — Esperimento: libclang vs tree-sitter ✅

Chiusa il 2026-09-26 con
[ADR-017](../../reference/decisions/017-code-graph-extraction.md).
**Lettore di base: tree-sitter + risolutore di nomi nostro**, sempre
disponibile, con i limiti dichiarati. **libclang: componente opzionale
caricato a runtime**, per l'analisi avanzata di C, C++ e Objective-C.

Misure su `app/src/`: i due approcci estraggono lo stesso modello (52 classi,
121 relazioni); tree-sitter in 0,04 s, libclang in ~53 s. Sui sei casi
difficili: libclang 6/6, tree-sitter col risolutore 5/6 (solo la macro resta
fuori). Script e istruzioni per rifarlo in
[`reference/technical/code-graph-extraction/`](../../reference/technical/code-graph-extraction/README.md).

### Fase 2 — Modello e generatore

- [x] Tipi in `core/` (`core/code_model.h`): classe, membro, relazione con
      il flag `certain`.
- [x] Interfaccia comune dell'estrattore (ADR-017 §4): per ora è il modello
      `core::CodeModel` che ogni estrattore restituisce. La classe base
      virtuale arriva con il secondo estrattore (libclang), come vuole
      ADR-010 (classi concrete finché non serve un punto di sostituzione).
- [x] Estrattore tree-sitter con il risolutore di nomi
      (`services/cpp_class_extractor.*`), portato in C++ da `extract_ts2.py`.
      tree-sitter 0.26.13 e tree-sitter-cpp 0.23.4 via CPM. Verificato con
      `compare.py` contro lo script: identico su `app/src` (72 classi,
      140 relazioni, 526 membri), su `ImGuiDot/src` e sui casi difficili.
      14 test Catch2, fra cui i sei casi difficili.
- [x] Relazioni incerte marcate nel modello (ADR-017 §2): un nome che due
      `using namespace` rendono ambiguo dà una relazione incerta (la prima
      trovata, marcata). È l'unico caso per ora.
- Limite noto: le classi in un namespace anonimo finiscono nel namespace che
  lo contiene (come nello script); i dettagli interni di un `.cpp` compaiono
  quindi fra le classi del progetto.
- [x] Generatore DOT (`services/class_diagram_generator.*`): selezione +
      vicini fantasma → testo. Scelte fatte:
  - riquadro classe come `shape=record` (`{nome|attributi|metodi}`, righe
    chiuse da `\l`), non label HTML: in ImGuiDot si appoggia sulle righe già
    divise da Graphviz, lo stesso meccanismo delle etichette su più righe
    (PR #18);
  - scomparti vuoti omessi; costruttori, distruttore e operatori omessi;
    overload in una riga sola (i parametri non sono nel modello);
  - membri mostrati fino a un livello d'accesso scelto (`memberAccess`,
    default solo pubblici): risponde in parte a OQ-2;
  - ereditarietà scritta base → derivata (`dir=back`), così il layout mette
    la base in alto;
  - relazione incerta: linea punteggiata ed etichetta `?`;
  - le etichette tolgono il namespace comune a tutte le classi disegnate;
  - colori da una `DiagramPalette` passata dalla UI (i servizi non vedono
    il tema); una voce vuota lascia il default del renderer.
- [x] Test Catch2 del generatore (`tests/test_class_diagram_generator.cpp`):
      un output DOT completo atteso, più una regola per test.

### Fase 3 — Interfaccia

**Prima fetta visibile (2026-10-01)**: pannello "Code graph" (`ui/code_graph_panel.*`,
pulsante nella top bar) con lettura in background e annullabile, elenco delle
classi per scope con caselle (anche per scope intero) e filtro, opzioni
"Neighbours" / "All members". "Create diagram" apre ogni diagramma in una
scheda sua (US-2), con "Fit" e "Copy DOT": più diagrammi restano aperti
insieme, e chiudere la scheda è il modo di toglierne uno. Le schede vivono a
parte dall'elenco: chiudere l'elenco non le chiude. L'elenco sta nel
pannello, non ancora nella sidebar: per ora la scheda nuova copre l'elenco
nello stesso gruppo di schede, finché non si trascina altrove. Il generatore usa scatole semplici
(`recordShapes = false`) finché ImGuiDot non disegna i record.

Emerso usandolo (provato su dev-dash, su un progetto senza C++ e sui
sorgenti di Graphviz, ~1800 file):

- la radice del progetto include test e `poc/`: le strutture dei test
  finiscono fra le classi. Si saltano solo le cartelle nascoste e di build.
  Risolto il 2026-10-02: nodo "Folders" con l'albero delle cartelle che
  contengono C++ e una casella a tre stati per cartella; la prima lettura
  lascia fuori le cartelle `test` e `tests`, poi decide l'utente (la scelta
  vale per la sessione, non è ancora salvata);
- "file con errori di sintassi" era fuorviante: quasi tutti sono macro
  (`TEST_CASE`, `SDLCALL`) o un limite della grammatica (`= {}` come argomento
  di default). Ora si chiamano "letti in parte", con l'elenco nel tooltip;
- un intero namespace con tutti i membri dà un diagramma largo e piatto,
  illeggibile anche adattato alla vista: la selezione piccola è il caso d'uso;
- ImGuiDot: scatole molto più grandi del testo (Graphviz misura con il suo
  font, ImGui disegna con un altro), linee tratteggiate disegnate piene,
  fantasma riempiti come le classi;
- cambiare progetto durante una lettura lunga bloccava l'app finché la
  lettura non finiva: ora la lettura si annulla (`std::stop_token`).

- [x] Sezione della sidebar: albero per namespace, caselle a più livelli
      (a tre stati: piena, vuota, mista), ricerca. Fatto il 2026-10-02: la
      finestra "Code graph" e il suo pulsante non ci sono più, i diagrammi
      restano schede del workspace. Una classe con classi annidate è un nodo
      solo, la cui casella copre anche le annidate. La vista per cartella è
      rimandata: utile per i progetti senza namespace.
- [x] Pannello diagramma con ImGuiDot, opzione "mostra vicini". Navigazione
      (2026-10-02): Ctrl+rotellina zooma attorno al punto sotto il mouse,
      trascinamento (tasto sinistro o centrale), rotellina e Shift+rotellina
      spostano la vista, "Fit" adatta e centra. Niente scrollbar: la tela
      ha uno spostamento suo, così il punto sotto il mouse resta fermo anche
      quando il diagramma è più piccolo della vista.
- [ ] Estrazione in background e aggiornamento incrementale (osservazione
      dei file).

Dall'interazione decisa il 2026-10-05 (vedi *Interazione*). Il prototipo
`experiment/sidebar-toolbar` è il punto di partenza; della sua struttura a
voci annidate restano la vista per cartella e "Neighbours"/"All members"
nella toolbar della scheda.

- [x] Sezione Code graph della sidebar con due comandi: "Analyze code"
      (prima voce, sempre attiva) e "New diagram".
- [x] Scheda dell'analisi: file letti, avvisi, file letti in parte, albero
      delle cartelle da leggere con "Analyze again".
- [x] Disposizione dei pannelli salvata fra un avvio e l'altro.
- [x] Selezione per diagramma: ogni scheda ha la sua, e cambiarla rigenera
      il diagramma.
- [x] Filtri nella scheda di diagramma, in un riquadro che si apre e si
      chiude dalla toolbar. Via la sezione Filters dalla sidebar.
- [x] Caselle dei membri (public, protected, private) nella toolbar; il
      generatore accetta un insieme di livelli d'accesso.
- [x] Menu View nella top bar, con le viste del workspace e l'anteprima DOT
      al posto del pulsante "Diagram".
- [x] Scheda dell'analisi rivista: "New code analysis", cartelle visibili
      prima della lettura, "Analyze", tabella dei file letti in parte con il
      motivo.
- [ ] La casella "Filters" della toolbar si nota poco e non si capisce che
      apre la colonna dei filtri: va spostata nel menu View, come voce che
      mostra o nasconde i filtri del diagramma attivo. Fino ad allora resta
      nella toolbar.

Fatto il 2026-10-05 (`feature/code-graph-panels`). Emerso usandolo: ogni
modifica della selezione rifà il layout da capo, quindi il diagramma si
riadatta alla vista dopo ogni cambio, altrimenti finisce in parte fuori.

### Fase 4 — Esplorazione

- [ ] Clic su un nodo fantasma → lo aggiunge alla selezione. Richiede
      l'hit-testing in ImGuiDot (vedi sotto).

## Contributi a ImGuiDot

> I difetti e le mancanze di ImGuiDot emersi usando il code graph sono
> registrati come tech-debt con `upstream: ImGuiDot`, elencati a parte
> nell'indice di [`tech-debt/`](../../tech-debt/README.md): lì si segue cosa è
> già stato segnalato a Dario e cosa no.

Il class diagram UML ha bisogno di funzioni che ImGuiDot oggi non ha. Vanno
proposte a Dario come PR sul suo repo (ADR-014: contribuzione attiva
upstream), dal fork `ghostintheshell-192/ImGuiDot`, una modifica per PR. Per
le scelte di design (nuove API) si chiede prima a Dario.

| Serve per | In DOT | Stato in ImGuiDot | Tipo di contributo |
|---|---|---|---|
| Riquadro a scomparti | `shape=record` (scelto dal generatore) | ❌ | Feature grossa: si segue la geometria dei campi calcolata da Graphviz |
| Dipendenza tratteggiata | `style=dashed` | ❌ stili di linea ignorati | Feature piccola, comportamento definito da Graphviz |
| Clic su un nodo | — | ❌ | **Nuova API**: da discutere con Dario prima di scrivere codice |
| Errori di parsing leggibili | — | ❌ silenziosi | **Nuova API**: da discutere |
| Cluster (raggruppamento per modulo) | `subgraph cluster_*` | ❌ | Feature media; utile per le viste future a livello modulo |
| Colori coerenti col tema | — | In discussione con Dario | Non bloccante: dev-dash scrive i colori esplicitamente nel DOT |

## Out of Scope (per ora)

- **Linguaggi diversi da C++**. Il C non ha classi (servirebbe un altro
  diagramma, per esempio le dipendenze fra moduli). Con tree-sitter
  (ADR-017) aggiungere un linguaggio costa una grammatica più le sue regole
  di ricerca dei nomi. Per il C# l'analisi avanzata richiederebbe Roslyn, non
  libclang.
- **Analisi avanzata con libclang** (ADR-017 §3): componente opzionale, dopo
  il lettore di base.
- **Viste salvate**: un diagramma salvato con un nome (la sua selezione e le
  sue opzioni, per esempio in `shell-sidebar.json`), riapribile e sempre
  rigenerato dal codice; la scheda dice da quale vista viene. È
  documentazione che non invecchia, e il passo naturale dopo questa spec.
- **Vista testuale per Claude** dallo stesso modello (idea della versione di
  aprile: una lista di adiacenza caricata a inizio sessione, per consultare le
  relazioni invece di cercare nei file). Il modello di questa spec la rende
  possibile; va pesata sul costo in token.
- **Diagrammi a livello modulo** (stile C4) e altri tipi di diagramma.
- **Pannelli fuori dalla finestra** dell'app (multi-viewport di ImGui):
  con SDL3 e Vulkan è delicato, e per ora non serve.
- **Layout stabile**: aggiungendo una classe Graphviz ricalcola tutto e i nodi
  si spostano. Per la prima versione si accetta.

## Open questions

- ~~**OQ-1**: libclang o tree-sitter?~~ Risolta da ADR-017.
- **OQ-1b**: come arriva libclang sulla macchina dell'utente (pacchetto di
  sistema o download gestito da dev-dash) e come si presenta nell'interfaccia
  l'offerta dell'analisi avanzata.
- ~~**OQ-2**: quali membri mostrare di default?~~ Risolta il 2026-10-05:
  pubblici di default, con tre caselle visibili nella toolbar del diagramma
  (vedi *Interazione*).
- **OQ-3**: osservazione dei file: inotify o controllo periodico? Va deciso
  anche pensando a Windows (ADR-011: Linux-first, non Linux-only).
- **OQ-4**: dove vive il modello estratto: solo in memoria o anche su disco
  (cache per non ri-estrarre tutto a ogni avvio)?
- ~~**OQ-5**: da quale menu si mostrano i pannelli?~~ Risolta il
  2026-10-05: un menu View nella top bar. I filtri non ne hanno bisogno,
  stanno nella scheda.
- **OQ-6**: cosa si salva per progetto (in
  `~/.devdash/projects/<slug>/code-graph.json`): le cartelle escluse
  certamente; i diagrammi aperti con le loro selezioni forse, e sarebbero un
  primo passo verso le viste salvate. E se rileggere da solo il codice
  all'apertura di un progetto già letto.

## Related

- [ADR-014](../../reference/decisions/014-germen-coevolution-strategy.md) —
  co-evoluzione con Germen, modulo grafi portabile, contribuzione upstream.
- [ADR-010](../../reference/decisions/010-architecture-design.md) — layer.
- [resource-model.md](../../reference/technical/resource-model.md) — la
  discussione di aprile da cui è nata la prima versione.
- [ImGuiDot](https://github.com/DPD85/ImGuiDot) — libreria di Dario (Graphviz
  → ImGui).
