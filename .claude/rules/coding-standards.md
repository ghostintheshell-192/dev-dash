# Coding Standards

## Naming Conventions

- **Classes/Methods/Properties**: `PascalCase`
- **Local variables/parameters**: `camelCase`
- **Private fields**: `_camelCase`
- **Constants**: `PascalCase`
- **Interfaces**: Prefix with `I` (e.g., `IUserService`)

## Code Style

- **Enforced via**: `dotnet format` + `.editorconfig`
- **Pre-commit hook**: Verifies formatting before commit
- **Manual check**: `dotnet format --verify-no-changes`

## File Organization

- **One class per file**: Class name matches file name
- **Using statements order**:
  1. System namespaces
  2. Microsoft namespaces
  3. Third-party namespaces
  4. Local project namespaces
- **File structure**:
  1. Constants
  2. Fields
  3. Constructor
  4. Public methods
  5. Private methods

## Documentation

- **XML comments**: Required for public APIs and complex methods
- **Language**: English only
- **Focus**: Explain "why", not "what"
- **Example**:

  ```csharp
  /// <summary>
  /// Resolves the effective Claude configuration for a project by merging
  /// global, workspace, and project-level rules in precedence order.
  /// Required because Claude Code itself does not expose this resolution.
  /// </summary>
  ```

## Dependency Injection

- **Constructor injection**: Inject dependencies through constructor
- **Use interfaces**: Depend on abstractions, not implementations
- **Validate early**: Check for null in constructor, fail fast
- **Example**:

  ```csharp
  public class WorkspaceService
  {
      private readonly IFileSystemService _fileSystem;

      public WorkspaceService(IFileSystemService fileSystem)
      {
          _fileSystem = fileSystem ?? throw new ArgumentNullException(nameof(fileSystem));
      }
  }
  ```

---

## C++ Conventions (PoC under `poc/`)

Applies to all C++ code in the C++/ImGui PoC, including code ported from
external sources (Germen Pulchrum). Borrowed code is translated to these
conventions on the way in — the source project's style is not preserved.

| Element | Convention | Example |
| ------- | ---------- | ------- |
| File names | `snake_case.cpp` / `snake_case.h` | `renderer.cpp` |
| Types (class, struct, enum) | `PascalCase` | `class DeletionQueue` |
| Member functions & free functions | `PascalCase` | `void Init();` |
| Local variables & parameters | `camelCase` | `uint32_t apiVersion` |
| Private member fields | `_camelCase` | `VkDevice _device;` |
| Constants (`constexpr`) | `kPascalCase` | `constexpr int kMaxFramesInFlight = 3;` |
| Namespaces | `snake_case` (sparingly; nest only when needed) | `namespace dev_dash::renderer` |
| Indentation | 4 spaces | — |
| Braces | Allman (open brace on its own line) | — |
| `std::` qualification | Always explicit; never `using namespace std` | `std::printf(...)` |

**Why these choices**:

- `PascalCase` for types and member functions stays coherent with the .NET side
  of the codebase, so future C# bindings (if needed) read symmetrically.
- The `k`-prefix for constants comes from the Google C++ style — it makes a
  constant visually distinct from a type at the call site, which matters in
  ImGui/Vulkan code where types and constants are densely interleaved.
- Allman braces and 4-space indent match the style established in
  `poc/src/main.cpp` at Step 1.

When porting Italian-named identifiers from Germen, translate to English
*and* re-style in one pass — never carry over the Italian/source style.
