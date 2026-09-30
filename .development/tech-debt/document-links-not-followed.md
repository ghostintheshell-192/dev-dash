---
type: bug
priority: medium
status: open
discovered: 2026-09-29
resolved: null
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
