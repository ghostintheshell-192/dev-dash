# Coding Standards

## C++ Conventions

Applies to all C++ code under `app/`, including code ported from
external sources (Germen Pulchrum). Borrowed code is translated to these
conventions on the way in — the source project's style is not preserved.

| Element | Convention | Example |
| ------- | ---------- | ------- |
| File names | `snake_case.cpp` / `snake_case.h` | `document_loader.cpp` |
| Types (class, struct, enum) | `PascalCase` | `class DocumentLoader` |
| Member functions & free functions | `PascalCase` | `LoadResult Load(...)` |
| Local variables & parameters | `camelCase` | `std::filesystem::path filePath` |
| Private member fields | `_camelCase` | `VkDevice _device;` |
| Public struct fields | `camelCase` | `std::string relativePath;` |
| Constants (`constexpr`) | `kPascalCase` | `constexpr int kMaxFramesInFlight = 3;` |
| Namespaces | `snake_case` (sparingly; nest only when needed) | `namespace dev_dash::platform` |
| Indentation | 4 spaces | — |
| Braces | Allman (open brace on its own line) | — |
| `std::` qualification | Always explicit; never `using namespace std` | `std::printf(...)` |

**Why these choices**:

- `PascalCase` for types and member functions: consistent across the codebase;
  also keeps symmetry if C bindings are ever needed.
- The `k`-prefix for constants comes from the Google C++ style — it makes a
  constant visually distinct from a type at the call site, which matters in
  ImGui/Vulkan code where types and constants are densely interleaved.
- `camelCase` for public struct fields (extends the local/param rule) — aligns
  with `MarkdownFonts` established style in the PoC.
- Allman braces and 4-space indent match the style established in
  `app/src/main.cpp`.

When porting Italian-named identifiers from Germen, translate to English
*and* re-style in one pass — never carry over the Italian/source style.

## Layer Boundary Rules

- `core/` ← nothing: only `<filesystem>`, `<vector>`, `<string>`, `<optional>`.
- `services/` ← `core/`.
- `ui/` ← `core/`, `services/`, `<imgui.h>`, `<imgui_md.h>`.
- `platform/` ← SDL3, Vulkan, vk-bootstrap, `<imgui.h>`. **NOT** `ui/` or `services/`.
- `app/` ← all layers (composition root).

No `#include "ui/..."` in `platform/*.h`. No `#include "platform/..."` in
`services/*.h` or `core/*.h`.

## Dependency Injection

Services are injected by reference in constructors. No null checks needed for
references. Raw pointer (`T*`) signals optional non-owning dependency.

```cpp
class DocumentPanelHost
{
public:
    DocumentPanelHost(services::DocumentLoader& loader,
                      MarkdownRenderer& renderer);
};
```
