# ADR-017: Estrazione del code graph — tree-sitter di base, libclang opzionale

**Data**: 2026-09-26
**Status**: Accepted
**Impact**: medium
**Sommario**: Il code graph estrae la struttura del codice con tree-sitter più un risolutore di nomi nostro, sempre disponibile e dichiarato con i suoi limiti; libclang (C, C++, Objective-C) è un componente opzionale caricato a runtime, che l'utente scarica solo se vuole l'analisi avanzata; per il C# l'equivalente sarebbe Roslyn.

## Contesto

[`feature-code-graph`](../../specs/planned/feature-code-graph.md) mostra il
class diagram UML delle classi scelte dall'utente, generato dal codice e
aggiornato in tempo reale. Per farlo serve un estrattore che, dal codice,
ricavi classi, membri e relazioni. Il primo linguaggio è il C++.

Le alternative serie erano due:

- **libclang**, il compilatore Clang usato come libreria: analisi semantica
  esatta, ma richiede `compile_commands.json` (quindi un progetto già
  configurato), è una dipendenza pesante e copre solo C, C++ e Objective-C.
- **tree-sitter**, un parser di sintassi: leggero, senza requisiti, con
  grammatiche per decine di linguaggi, ma non risolve i nomi. Dice "qui c'è un
  campo di tipo `b::Node`", non a quale classe si riferisce.

La Fase 1 della spec è stata un esperimento su `app/src/` di dev-dash e su sei
casi C++ difficili con risposta nota. Script, istruzioni e risultati completi
sono in
[`reference/technical/code-graph-extraction/`](../technical/code-graph-extraction/README.md).

Risultati in sintesi:

| | dev-dash reale | Casi difficili | Tempo su dev-dash |
|---|---|---|---|
| libclang | 52 classi, 121 relazioni | 6/6 | ~53 s (Python, un thread) |
| tree-sitter ingenuo | identico | 1/6 | 0,04 s |
| tree-sitter + risolutore nostro | identico | 5/6 (manca solo la macro) | 0,04 s |

Il risolutore è circa un centinaio di righe Python. Implementa una versione
semplificata della ricerca dei nomi del C++: dallo scope più interno verso
l'esterno, con alias, `using` e basi template. Il caso che resta fuori è la
classe generata da una macro, perché tree-sitter non esegue il preprocessore.

## Decisione

1. **tree-sitter con il nostro risolutore di nomi è il lettore di base**,
   sempre disponibile, per ogni progetto aperto in dev-dash. I suoi limiti
   (macro, casi che richiedono il compilatore) sono **dichiarati
   all'utente** nell'interfaccia, non nascosti.
2. **Le relazioni che il risolutore non sa attribuire con certezza si
   mostrano come incerte**, invece di tirare a indovinare.
3. **libclang è un componente opzionale, caricato a runtime.** dev-dash non
   lo collega in compilazione: lo cerca all'avvio e, se c'è, offre
   l'analisi avanzata per C, C++ e Objective-C sui progetti che hanno
   `compile_commands.json`. L'utente lo scarica solo se gli serve. Il
   meccanismo di installazione (pacchetto di sistema o download gestito da
   dev-dash) si decide quando si implementa questa parte.
4. **L'estrattore sta dietro un'interfaccia comune** in `services/`, che
   produce lo stesso modello (`core/`) qualunque sia la sorgente. È ciò che
   rende possibile il punto 3 e, più avanti, altri linguaggi.
5. **Per il C# l'analisi avanzata richiederebbe Roslyn**, non libclang,
   perché il C# non fa parte della famiglia C di Clang. Fuori scope finché il
   C# non entra nei linguaggi supportati.

## Rationale

- **Tempo reale.** La spec chiede un diagramma sempre aggiornato: 0,04 s
  contro decine di secondi. Anche un libclang in C++ e multi-thread resterebbe
  sull'ordine dei secondi (stima, non misurata).
- **Nessun requisito.** dev-dash apre progetti qualsiasi, e molti non hanno
  `compile_commands.json` a disposizione.
- **Stesso risultato sul codice reale.** Su dev-dash i tre estrattori danno
  lo stesso modello. Nei tre progetti che l'utente userà per primi (dev-dash,
  Germen, ImGuiDot) non ci sono i casi che il risolutore non copre.
- **La strada verso altri linguaggi** passa da tree-sitter: una grammatica
  in più più le regole di ricerca dei nomi del linguaggio, non un estrattore
  nuovo.
- **Nessuno perde precisione.** Chi ne ha bisogno scarica libclang; chi non ne
  ha bisogno non si porta dietro una dipendenza pesante. Principio dell'utente:
  "usare clang non è reato, il punto è consentire di scaricarlo solo se serve".

## Conseguenze

### Pro

- Diagramma immediato su qualsiasi progetto, senza configurazione.
- Dipendenza di base leggera (una libreria C, più le grammatiche).
- Precisione completa disponibile, su richiesta, per la famiglia C.
- Il modello comune rende sostituibile l'estrattore senza toccare UI e
  generatore DOT.

### Contro

- Il risolutore è una **reimplementazione approssimata** delle regole del C++,
  da mantenere nostra. Ogni caso nuovo che emerge dal codice reale è codice in
  più.
- Le macro restano fuori dal lettore di base.
- Due estrattori da tenere allineati sullo stesso modello; l'esperimento
  mostra che è verificabile con `compare.py`.
- Il caricamento dinamico di libclang (percorsi, versioni, gli header interni
  mancanti già incontrati nell'esperimento) è un problema di distribuzione da
  risolvere quando si arriva lì.
