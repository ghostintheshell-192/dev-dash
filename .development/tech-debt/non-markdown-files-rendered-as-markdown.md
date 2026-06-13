---
type: bug
priority: medium
status: open
discovered: 2026-06-13
related: [markdown-code-block-styling, preprocess-imports-indented-fences]
related_decision: reference/decisions/009-markdown-library-imgui-md.md
report: null
---

# File non-Markdown renderizzati come Markdown (script, config)

## Problem

Aprendo nel viewer un file che NON è Markdown — uno script `.sh`, un file di
config, ecc. — il contenuto viene passato comunque al `MarkdownRenderer` e
interpretato come CommonMark. Conseguenze visibili (screenshot di riferimento
`/data/pictures/dev-dash/script sh.png`, file `format-fix.sh`):

- I commenti bash che iniziano con `#` diventano **heading H1 giganti**
  (`# Standard automation entry point` → titolo enorme), perché `#` in
  Markdown è un heading di livello 1.
- Il corpo dello script viene **riflowato** come paragrafo: gli a-capo
  significativi spariscono e token adiacenti si fondono
  (`then echo` → `thenecho`, righe `if` concatenate), rendendo lo script
  illeggibile e fuorviante.

Il viewer mostra correttamente il `.md`, ma tratta ogni documento come
Markdown a prescindere dall'estensione/tipo.

## Analysis

Il `DocumentPanelHost`/`MarkdownRenderer` non discrimina il tipo di file:
qualsiasi cosa caricata dal `DocumentLoader` finisce nel parser MD4C. Per i
file di configurazione che DevDash mostra (e che includono `.sh` negli hook,
`settings.json`, ecc.) questo è il caso comune, non l'eccezione.

Bug preesistente, indipendente dal lavoro di release-readiness — emerso
durante la verifica della fase 2 ma senza alcun rapporto con quelle modifiche.

## Possible Solutions

- **Option A**: Rilevare il tipo dal file (estensione / shebang) e, per i
  non-Markdown, renderizzare come testo monospace senza parsing (un semplice
  `TextUnformatted` in font mono, niente MD4C). Pro: semplice, rispetta il
  contenuto. Contro: nessun syntax highlighting (fuori scope, vedi ADR-011
  out-of-scope).
- **Option B**: Wrappare l'intero contenuto non-Markdown in un code fence
  ```` ``` ```` prima di passarlo al renderer, riusando lo styling dei code
  block già esistente. Pro: riusa il path esistente. Contro: fragile se il
  contenuto contiene esso stesso dei fence; meno diretto di A.
- **Option C**: Estendere il renderer con un vero highlighter per shell/json.
  Pro: UX migliore. Contro: scope grande, esplicitamente out-of-scope ora.

## Recommended Approach

Option A — riconoscere i non-Markdown e mostrarli come testo monospace
plain. È la correzione minima che elimina il comportamento sbagliato senza
introdurre un highlighter. Il discrimine può partire dall'estensione
(`.md`/`.markdown` → Markdown; tutto il resto → plain) con eventuale
fallback sullo shebang.

## Notes

Catturato durante la fase 2 di release-readiness (branch
`feature/release-readiness`); affrontato come thread dedicato dopo, per non
derailare la feature in corso.

## Related Documentation

- **Related Issues**: `markdown-code-block-styling`,
  `preprocess-imports-indented-fences`
- **Architecture Decision**: ADR-009 (libreria Markdown imgui_md)
- **Code Locations**: `app/src/ui/markdown_renderer.{h,cpp}`,
  `app/src/ui/document_panel_host.{h,cpp}`, `app/src/services/document_loader.{h,cpp}`

---

📍 **Investigation Note**: Read [ARCHITECTURE.md](../ARCHITECTURE.md) to locate relevant files and understand the architectural context before starting your analysis.
