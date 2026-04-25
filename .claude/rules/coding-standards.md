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
