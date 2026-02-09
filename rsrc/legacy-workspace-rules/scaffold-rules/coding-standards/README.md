# Coding Standards

## Index

### Core Principles

- [**general-principles.md**](./general-principles.md) - Universal coding principles, error handling, and refactoring checklist

### Language-Specific Standards

- [**flutter-dart.md**](./flutter-dart.md) - Flutter/Dart development standards and widget patterns
- [**python.md**](./python.md) - Python development standards and best practices
- [**csharp-dotnet.md**](./csharp-dotnet.md) - C#/.NET development standards and architectural patterns
- [**c-cpp.md**](./c-cpp.md) - C and C++ development standards and memory management
- [**javascript-typescript.md**](./javascript-typescript.md) - JavaScript/TypeScript and React development standards

## Usage Guidelines

### When to Consult These Standards

1. **Start with general-principles.md** for universal guidelines applicable to all languages
2. **Reference specific language files** when working on projects in that technology
3. **Apply naming conventions** as defined in each language standard
4. **Follow repository structure** as outlined in `../repository-standards.md`

### File Organization Strategy

- Each file focuses on **concepts over examples** for faster comprehension
- **Principle-based approach** - descriptions point to established patterns and practices
- **Anti-pattern identification** - clear guidance on what to avoid
- **Concise format** - optimized for quick reference and decision-making

### Integration with Development Workflow

- **Before coding**: Review relevant standards for the language/framework
- **During code review**: Reference specific sections for consistency checks
- **When refactoring**: Use the refactoring checklist in general-principles.md
- **For new team members**: Start with general-principles.md, then language-specific files

## Related Files

- `../repository-standards.md` - Repository organization, structure, and naming
- `../naming-mappings.yaml` - Automatic name conversion between conventions
- `./CLAUDE.md` - Global repository rules and development methodology

## Standards Philosophy

These standards prioritize:

- **Readability** over cleverness
- **Consistency** over personal preference  
- **Maintainability** over short-term convenience
- **Explicit error handling** over silent failures
- **Principle comprehension** over memorizing specific implementations
