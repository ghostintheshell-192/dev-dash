# ADR-002: Symlink vs Copy per aggregazione vault

**Data**: 2024-12-02
**Status**: Accepted
**Impact**: medium
**Sommario**: Aggrega la documentazione nel Vault@Claude tramite symlink ai file originali, anziché copia fisica o aggregazione virtuale, per avere single source of truth e zero sync.

## Contesto

DevDash aggrega documentazione da più sorgenti nel Vault@Claude:

- Configurazioni Claude Code (`~/.claude/`)
- Workspace rules (`/data/repos/CLAUDE.md`)
- `.personal/` di ogni progetto

Opzioni per aggregare:

1. **Symlink**: puntatori ai file originali
2. **Copy**: duplicazione fisica dei file
3. **Virtual aggregation**: solo a runtime, nessun file fisico

## Decisione

Usare symlink per aggregare file nel vault.

## Rationale

- Zero sync da mantenere
- Single source of truth
- Modifica in Obsidian = modifica nel progetto originale
- Nessuna duplicazione = nessun rischio di drift

## Conseguenze

- **Pro**: Manutenzione zero, sempre aggiornato
- **Contro**: Su Windows richiede privilegi admin o Developer Mode
- **Mitigazione**: Documentare setup Windows, accettabile per target audience

## Setup

```bash
# Esempio symlink
ln -s /data/repos/sheet-atlas/.personal /data/documenti/Vault@Claude/progetti/sheet-atlas
```
