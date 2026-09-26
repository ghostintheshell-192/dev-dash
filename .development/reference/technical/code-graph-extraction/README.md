# Code-graph extraction experiment (2026-09-26)

Fase 1 di [`feature-code-graph`](../../../specs/planned/feature-code-graph.md):
estrarre le classi C++ e le loro relazioni con **libclang** e con
**tree-sitter**, e confrontare i risultati. La decisione presa sulla base di
queste misure è [ADR-017](../../decisions/017-code-graph-extraction.md).

Gli script sono in Python perché entrambe le librerie hanno binding pronti.
L'esperimento misura la **qualità dell'estrazione**, non la velocità
dell'implementazione finale, che sarà in C++.

## File

| File | Cosa fa |
| ---- | ------- |
| `model.py` | Modello condiviso: classe, membro, relazione (`inherits`, `composes`, `aggregates`, `depends`). |
| `extract_clang.py` | Estrattore semantico con libclang. Legge `compile_commands.json`. |
| `extract_ts.py` | Estrattore tree-sitter "ingenuo": associa i tipi alle classi solo per nome corto. |
| `extract_ts2.py` | Estrattore tree-sitter con il nostro **risolutore di nomi**: ricerca per scope, alias, `using`, basi template. |
| `compare.py` | Confronta due estrazioni: classi, membri, relazioni. |
| `tricky/src/` | Sei casi C++ difficili con risposta nota. |

## Come rifarlo

```bash
pip install libclang tree-sitter tree-sitter-cpp

# dev-dash: serve compile_commands.json (il preset linux-debug lo esporta)
cmake --preset linux-debug                         # da app/
python3 extract_clang.py app/src app/build/linux-debug/compile_commands.json > clang.json
python3 extract_ts2.py app/src > ts2.json
python3 compare.py clang.json ts2.json

# casi difficili: compile_commands.json minimo, con percorsi assoluti
D=$PWD/tricky
echo "[{\"directory\":\"$D\",\"file\":\"$D/src/tricky.cpp\",\"command\":\"c++ -I$D/src -std=c++20 -c $D/src/tricky.cpp\"}]" > $D/compile_commands.json
python3 extract_clang.py $D/src $D/compile_commands.json
python3 extract_ts2.py $D/src
```

Versioni usate: libclang 18.1.1 (wheel pip), tree-sitter 0.26.0,
tree-sitter-cpp 0.23.4, GCC 13.3 per gli header interni.

## Risultati

### Codice reale: `app/src/` di dev-dash

| | Classi | Relazioni | Tempo |
|---|---|---|---|
| libclang | 52 | 121 (54 composes, 26 aggregates, 40 depends, 1 inherits) | 52–54 s |
| tree-sitter ingenuo | 52 | 121, identiche | 0,04 s |
| tree-sitter + risolutore | 52 | 121, identiche | 0,04 s |

Il primo confronto mostrava 18 differenze, tutte bug degli script e non degli
strumenti:

- libclang: `unique_ptr<T>` e `vector<T>` hanno argomenti template nascosti
  (`default_delete<T>`, `allocator<T>`), che lo script leggeva come riferimenti
  a `T`.
- tree-sitter: i metodi che restituiscono un riferimento venivano scambiati per
  attributi.

Una volta corretti, i risultati sono identici. Il tempo di libclang è in
Python, su un solo thread, e rilegge ImGui, Vulkan e SDL per ogni file
sorgente.

### Casi difficili (`tricky/`)

| Caso | libclang | tree-sitter ingenuo | tree-sitter + risolutore |
|---|---|---|---|
| 1. Stesso nome in due namespace | ✅ | ❌ collega entrambi | ✅ |
| 2. Alias `using Nodes = vector<a::Node>` | ✅ | ❌ relazione persa | ✅ |
| 3. Classe generata da una macro | ✅ | ❌ classe persa | ❌ classe persa |
| 4. Base template (CRTP) | ✅ | ⚠️ nome non risolto | ✅ |
| 5. Alias template di smart pointer | ✅ | ❌ collega anche il `Node` sbagliato | ✅ |
| 6. `using a::Node;` | ✅ | ❌ collega entrambi | ✅ |

Il limite di tree-sitter è il preprocessore: le macro non vengono espanse.

### Frequenza nel codice reale

In dev-dash, Germen Pulchrum e ImGuiDot non c'è nessun nome di classe ripetuto
in namespace diversi, e nessuna classe generata da macro. Ci sono 7 alias di
tipo (dev-dash + Germen), che il risolutore espande.

### Trappole pratiche

- La wheel pip di libclang non include gli header interni del compilatore:
  senza `-isystem $(gcc -print-file-name=include)` ogni file si ferma al primo
  include di libc (`stddef.h` not found). `extract_clang.py` li prende da GCC.
