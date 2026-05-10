---
type: bug
priority: high
status: closed
discovered: 2025-12-11
closed: 2026-05-10
source: code-reviewer
report: archive/analysis/2025-12-11_report_code-reviewer.md
related: []
---

## Resolution

**Closed by obsoletion** (2026-05-10): the affected file
`src/DevDash/Views/Controls/MultiSidebar.axaml` was part of the .NET/Avalonia
implementation, removed from `develop` after the C++/Dear ImGui pivot
([ADR-008](../reference/decisions/008-pivot-to-cpp-imgui.md)). The bug no
longer exists because the code no longer exists. Recover via
`git checkout legacy/avalonia-final` if the historical fix becomes interesting.

---

# ContentControl Binding Error in MultiSidebar - Empty Sidebar Content

## Problem

Il controllo MultiSidebar non visualizza alcun contenuto nelle sidebar. Le icone funzionano, le animazioni funzionano, ma il contenuto (es. "PROJECTS", lista progetti) non appare.

## Analysis

**Location**: `/data/repos/dev-dash/src/DevDash/Views/Controls/MultiSidebar.axaml:85-86`

Il ContentControl ha due problemi critici:

1. **ContentTemplate binding malformato**: Il path `((models:SidebarItem)DataContext).ContentTemplate` tenta un cast non necessario
2. **Content mancante**: Il ContentControl ha solo `ContentTemplate` impostato, ma senza `Content` non c'e' nulla da renderizzare

Codice attuale:
```xml
<ContentControl ContentTemplate="{Binding Path=((models:SidebarItem)DataContext).ContentTemplate, RelativeSource={RelativeSource Mode=FindAncestor, AncestorType=Border}}"
                DataContext="{Binding DataContext, RelativeSource={RelativeSource AncestorType=controls:MultiSidebar}}"/>
```

Il ContentControl usa `Content` (non `DataContext`) come oggetto su cui applicare il `ContentTemplate`.

## Possible Solutions

### Soluzione 1: Fix diretto del binding (Raccomandato)
```xml
<ContentControl Content="{Binding DataContext, RelativeSource={RelativeSource AncestorType=controls:MultiSidebar}}"
                ContentTemplate="{Binding ContentTemplate}"/>
```

### Soluzione 2: Usare ParentDataContext property
La proprieta' `ParentDataContext` esiste gia' nel code-behind ma non e' utilizzata. Potrebbe essere usata per passare esplicitamente il ViewModel.

### Soluzione 3: Refactoring completo
Eliminare l'approccio ContentTemplate e usare controlli UserControl dedicati per ogni sidebar.

## Recommended Approach

**Soluzione 1** - E' il fix minimo che risolve il problema senza refactoring.

Logica:
- `ContentTemplate="{Binding ContentTemplate}"` prende il template dal SidebarItem (DataContext corrente nell'ItemsControl)
- `Content="{Binding DataContext, ...}"` prende MainWindowViewModel dal DataContext del MultiSidebar
- Il template viene applicato a Content, e i binding nel template risolvono correttamente

## Notes

- Source: Code review by code-reviewer agent
- Full details in: `.personal/archive/analysis/2025-12-11_report_code-reviewer.md`
- Issue blocca completamente la funzionalita' delle sidebar - priorita' alta
- Il fix richiede modificare una sola riga di codice
