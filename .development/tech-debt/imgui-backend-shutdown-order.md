---
type: bug
priority: high
status: closed
discovered: 2026-05-10
closed: 2026-05-10
related: []
related_decision: null
report: null
---

# ImGui assertion on exit: backend non spento prima di DestroyContext

## Problem

All'uscita dell'applicazione, ImGui lancia un'assertion:

```
Assertion `(g.IO.BackendPlatformUserData == __null) &&
"Forgot to shutdown Platform backend?"'
```

Causa: `App::~App()` chiamava `ImGui::DestroyContext()` nel corpo del
distruttore, prima che `_imguiBackend` venisse distrutto. In C++, il corpo
del distruttore esegue PRIMA dei member destructor (ordine LIFO). Quindi
`ImGui_ImplVulkan_Shutdown()` e `ImGui_ImplSDL3_Shutdown()` (chiamati da
`ImGuiBackend::~ImGuiBackend()`) arrivavano troppo tardi.

## Fix applicato

`app/src/app/app.cpp` — `App::~App()`: aggiunto `_imguiBackend.reset()`
prima di `ImGui::DestroyContext()`. Calling `reset()` su un `unique_ptr`
distrugge l'oggetto immediatamente; quando il member destructor automatico
gira dopo, il puntatore è già null (no-op).

## Related Documentation

- **Code Location**: `app/src/app/app.cpp` — `App::~App()`
- **Code Location**: `app/src/platform/imgui_backend.cpp` — `~ImGuiBackend()`
