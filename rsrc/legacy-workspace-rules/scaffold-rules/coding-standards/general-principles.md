# General Coding Principles

## 1. Universal Rules

### Fail Fast vs. Silent Failure

- **Before implementing fallback/recovery**: Ask if the failure indicates a problem that should be solved at the root
- **Avoid masking errors**: A fallback that "makes everything work" often hides real bugs  
- **Prefer controlled crashes**: It's better to fail clearly than continue in an inconsistent state
- **Distinguish fallback from default case**:
  - ❌ **Fallback** = "If this doesn't work, do this instead" (hides problems)
  - ✅ **Default case** = "In this expected situation, behave this way" (handles legitimate scenario)
- **Practical rule**: If you're asking "what do I do if this fails?", you're probably creating a problematic fallback. If you're asking "what's the normal behavior in this case?", it's probably a legitimate default case.

### Code Quality Fundamentals

- **Self-documenting code**: Write clear, expressive code instead of relying on comments
- **English only**: All code, comments, and documentation must be in English
- **Consistency**: Follow established patterns within the codebase
- **Simplicity**: Prefer the simplest solution that works
- **Single responsibility**: Each function/class should have one clear purpose
- **Meaningful names**: Use descriptive names that explain intent

### Comments Policy

- Add comments only when complex logic needs explanation
- **All comments must be in English**
- Prefer descriptive names over explanatory comments
- Document WHY, not WHAT
- Remove commented-out code before committing

## 2. Error Handling Examples

### ❌ Bad: Silent Failure (Fallback)

```javascript
function processUserData(userData) {
  try {
    return validateAndTransform(userData);
  } catch (error) {
    return {}; // Hides the real problem!
  }
}
```

### ✅ Good: Fail Fast

```javascript
function processUserData(userData) {
  if (!userData || typeof userData !== 'object') {
    throw new Error('User data must be a valid object');
  }
  
  if (!userData.email) {
    throw new Error('User email is required');
  }
  
  return validateAndTransform(userData);
}
```

### ✅ Good: Legitimate Default Case

```javascript
function getUserDisplayName(user) {
  if (user.displayName) return user.displayName;
  if (user.firstName && user.lastName) return `${user.firstName} ${user.lastName}`;
  return user.email; // Legitimate default - all users have email
}
```

## 3. Common Anti-Patterns to Avoid

### Code Structure

- **Magic numbers**: Use named constants instead of hardcoded values
- **Deep nesting**: Extract methods to reduce complexity (max 3-4 levels)
- **Long parameter lists**: Use objects/structs for grouped parameters (max 3-4 parameters)
- **God classes**: Split responsibilities into focused classes
- **Copy-paste code**: Extract common functionality into reusable methods
- **Premature optimization**: Profile before optimizing

### Naming Issues

- **Abbreviations**: Use full, descriptive names (`userManager` not `usrMgr`)
- **Misleading names**: Names should accurately reflect what the code does
- **Inconsistent conventions**: Follow the established patterns in the codebase
- **Generic names**: Avoid `data`, `info`, `manager` without context

## 4. Function Design Principles

### Size and Complexity

- **Maximum 25 lines** per function (guideline, not hard rule)
- **Single responsibility**: One function should do one thing well
- **Clear inputs/outputs**: Parameters and return values should be obvious
- **No side effects**: Unless clearly documented and necessary

### Parameters

- **Maximum 3-4 parameters** - more suggests the function is doing too much
- **Use descriptive parameter names**: `calculateTax(income, rate, deductions)` not `calc(a, b, c)`
- **Validate inputs**: Check parameters at the beginning of the function
- **Use default values** when appropriate

## 5. Code Review Checklist

### Before Submitting

- [ ] Code follows language-specific naming conventions
- [ ] No magic numbers or hardcoded values
- [ ] Error handling is explicit and meaningful
- [ ] Functions have single responsibility
- [ ] Code is self-documenting with clear names
- [ ] No commented-out code
- [ ] Tests cover new functionality
- [ ] Documentation updated if needed
- [ ] No silent failures or inappropriate fallbacks

### Security Considerations

- [ ] No hardcoded secrets or credentials
- [ ] Input validation for user-provided data
- [ ] Proper error messages (don't expose internal details)
- [ ] Resource cleanup (files, connections, memory)

### Performance Considerations

- [ ] No obvious performance bottlenecks
- [ ] Efficient algorithms for the use case
- [ ] Proper resource usage
- [ ] Consider caching for expensive operations

## 6. Testing Guidelines

### Test Structure

- **Arrange, Act, Assert**: Clear test organization
- **Descriptive test names**: Should explain what is being tested
- **Test one thing**: Each test should verify one specific behavior
- **Independent tests**: Tests should not depend on each other

### Test Coverage

- **Happy path**: Normal, expected behavior
- **Edge cases**: Boundary conditions and special cases
- **Error cases**: How the code handles invalid input
- **Integration points**: How components work together

## 7. Documentation Standards

### Code Documentation

- **README files**: Clear setup and usage instructions
- **API documentation**: For public interfaces
- **Architecture docs**: For complex systems
- **Inline docs**: For complex algorithms or business logic

### Documentation Quality

- **Keep it current**: Update docs when code changes
- **Be concise**: Clear and to the point
- **Include examples**: Show how to use the code
- **Explain decisions**: Document architectural choices and trade-offs

## 8. Refactoring Checklist

After completing a first implementation, systematically evaluate:

### 1. Code Duplication

- ✅ Are there nearly identical code blocks? Can they be extracted into methods?
- ✅ Do you have loops with similar structure but different values? Can they be parameterized?
- ✅ Are there repeated conditional expressions? Can they be extracted into predicate functions?

### 2. Responsibility Evaluation

- ✅ Does each method do too many different things? (>25 lines is suspicious)
- ✅ Are there comments that separate "logical sections" within methods?
- ✅ Do methods have more than 3-4 parameters? (Suggests too many responsibilities)

### 3. Domain Modeling

- ✅ Does the code correctly reflect real relationships between entities?
- ✅ Do class/method names reflect the domain language?
- ✅ Are there properties or methods "out of place" relative to the main responsibility?

### 4. Pattern Opportunities

- ✅ Object creation: Are Factory, Builder patterns needed?
- ✅ Conditional behaviors: Are Strategy, Command patterns appropriate?
- ✅ State management: Is State pattern or event-based approaches needed?

### 5. Abstraction and Interfaces

- ✅ Are there dependencies on concrete implementations instead of interfaces?
- ✅ Do classes contain methods operating at different abstraction levels?
- ✅ Can the code be improved with composition instead of inheritance?

### 6. Progressive Generalization

- ✅ Can generics/templates make the code more reusable?
- ✅ Are there specialized data structures that could be parameterized?
- ✅ Do similar functionalities exist in different types? Candidates for base class extraction.

### 7. Final Checks

- ✅ Does the new generalized code maintain or improve readability?
- ✅ Do the introduced abstractions have clear value or are they premature?
- ✅ Is the complexity/flexibility balance appropriate for current requirements?
- ✅ Does the modification introduce circular dependencies or unwanted coupling?

---

**Remember**: These principles apply across all programming languages. See language-specific files for detailed conventions and examples.
