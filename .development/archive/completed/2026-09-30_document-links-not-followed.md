---
type: bug
priority: medium
status: resolved
discovered: 2026-09-29
resolved: 2026-09-30
related: []
related_decision: null
report: null
---

# I link nei documenti non si aprono

## Problem

Nei pannelli dei documenti un clic su un link non fa niente, tranne che per le
righe `@include` di `CLAUDE.md`. Non si aprono i link relativi ad altri
documenti (`[ADR-010](../reference/decisions/010-architecture-design.md)`), le
ancore (`#sezione`) e neppure gli URL esterni (`https://…`). Nessun messaggio
spiega perché.

## Analysis

`MarkdownRenderer` ha un gestore di default che apre gli URL con
`SDL_OpenURL`. `DocumentPanelHost` lo sostituisce nel costruttore
(`document_panel_host.cpp:19`) con `HandleLinkClick`, che accetta solo il
prefisso `claudeimport://` (i link che `DocumentLoader` genera per gli
`@include`) e scarta tutto il resto in silenzio. Il gestore non sa nemmeno da
quale pannello arriva il clic, quindi non potrebbe risolvere un percorso
relativo.

## Possible Solutions

- **Option A**: in `HandleLinkClick` smistare per tipo: `claudeimport://` come
  oggi; `http(s)://` e `mailto:` a `SDL_OpenURL`; percorsi relativi risolti
  rispetto al documento del pannello e aperti in un nuovo pannello se esistono;
  `#ancora` ignorata per ora. Serve che il renderer passi il documento corrente
  (per esempio impostando il percorso del pannello prima di `print()`).
- **Option B**: far generare a `DocumentLoader` link `claudeimport://` assoluti
  anche per i link markdown relativi, riscrivendo il sorgente. Più invasivo, e
  duplica il lavoro del parser.

## Recommended Approach

Option A. Le ancore interne (scroll alla sezione) possono restare un passo
successivo.

## Notes

Un file inesistente oggi fa solo `std::cerr` in `OpenPanel`: con i link
relativi conviene mostrarlo nella status bar.

## Related Documentation

- **Code Locations**: `app/src/ui/document_panel_host.cpp` (`HandleLinkClick`),
  `app/src/ui/markdown_renderer.cpp` (gestore di default),
  `app/src/services/document_loader.cpp` (link `claudeimport://`)

## Resolution

Option A, 2026-09-30 (branch `fix/document-links-not-followed`).

- `DocumentLoader::ResolveLink(documento, url)` in `services/` classifica il
  link: `@include` (`claudeimport://`), documento markdown, altro file o
  cartella, URL con schema, ancora, destinazione inesistente. I percorsi
  relativi si risolvono rispetto al documento; `#frammento` e `?query` si
  scartano, i `%XX` si decodificano, `file://` vale come percorso assoluto.
  Coperto da `app/tests/test_document_loader.cpp`.
- `DocumentPanelHost` registra il documento che sta disegnando
  (`_currentDocument`) e agisce sul risultato: i `.md` si aprono in un
  pannello, URL e file non markdown vanno all'applicazione di sistema
  (`MarkdownRenderer::OpenExternalUrl`), ancore e errori finiscono nella
  status bar (`SetStatusSink`, collegata dalla shell).
- I file non markdown non si aprono nel viewer per scelta: lì verrebbero
  resi come markdown (tech-debt `non-markdown-files-rendered-as-markdown`).
- Resta aperto: lo scroll alla sezione per le ancore.

