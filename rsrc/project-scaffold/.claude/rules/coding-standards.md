# Coding Standards

## Language-Specific Standards

{LANGUAGE_SPECIFIC_STANDARDS}

> *Populated by DevDash from a language-specific template (naming
> conventions, formatter, file structure, idiomatic patterns) based on
> the detected tech stack.*

---

## Cross-Language Principles

These apply regardless of the language and complement the language-specific
standards above.

### File Organization

- **One primary entity per file**: filename matches the entity name
- **Predictable internal structure**: constants → fields → constructors →
  public surface → private helpers
- **Imports/usings ordered**: standard library → framework → third-party →
  local project

### Documentation

- **Language**: English only
- **Public API**: documented with the language's idiomatic doc-comment
  (XML doc, JSDoc, docstring, etc.)
- **Focus**: Explain *why*, not *what*. Code already shows *what*; comments
  exist to capture intent and constraints that aren't visible in the code.
- **Keep up-to-date**: a stale comment is worse than none

### Dependency Injection / Composition

- **Inject dependencies through the constructor** (or equivalent)
- **Depend on abstractions** (interfaces / protocols / traits), not concrete
  types
- **Validate early**: fail fast at construction time on invalid arguments

### Error Handling

- **Handle errors explicitly**: no swallowed exceptions, no silent fallbacks
- **Use the language's idiomatic error type** (exceptions, Result, Option,
  panic — depending on the language)
- **Provide meaningful messages**: include enough context to diagnose
- **Log with sufficient context**: at the boundary that owns the failure

### Testing

- **Cover new behaviour with tests** before the feature is considered done
- **Descriptive test names**: a failing test name should explain what broke
- **AAA pattern** (Arrange / Act / Assert) or equivalent structural clarity
