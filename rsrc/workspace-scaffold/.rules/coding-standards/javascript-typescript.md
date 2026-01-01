# JavaScript/TypeScript Coding Standards

## 1. Naming Conventions

### Files and Folders
- **Files**: `kebab-case.js/.ts` (`user-service.ts`, `data-processor.js`)
- **Folders**: `kebab-case` (`components/`, `utils/`, `api-handlers/`)
- **React Components**: `PascalCase.tsx` (`UserProfile.tsx`, `DataTable.tsx`)
- **Test files**: `*.test.js/.ts` or `*.spec.js/.ts`

### Code Elements
- **Classes**: `PascalCase` (`UserService`, `DataProcessor`, `ApiClient`)
- **Functions/Variables**: `camelCase` (`processData()`, `userName`, `isValid`)
- **Constants**: `UPPER_SNAKE_CASE` (`MAX_RETRIES`, `API_BASE_URL`)
- **React Components**: `PascalCase` (`UserProfile`, `DataTable`)
- **React Hooks**: `use` + `PascalCase` (`useUserData`, `useApiClient`)
- **Types/Interfaces**: `PascalCase` (`User`, `ApiResponse`, `ConfigOptions`)

## 2. Import Organization

### Import Order
1. Node.js built-in modules
2. External libraries (React, axios, lodash, etc.)
3. Internal modules with absolute paths (@/types, @/services)
4. Relative imports (./styles, ../components)
5. Type-only imports last

### Export Patterns
- Prefer named exports over default exports
- Use default exports only for main component/class per file
- Group re-exports for clean public APIs

## 3. TypeScript Type Definitions

### Interface vs Type Usage
- **Interfaces**: For object shapes and contracts
- **Type aliases**: For unions, primitives, computed types
- **Generics**: For reusable type definitions with constraints

### Advanced Type Patterns
- Use discriminated unions for state management
- Implement conditional types for API endpoint typing
- Apply mapped types for transformation utilities
- Leverage utility types (Partial, Pick, Omit, Required)

## 4. Function and Class Design

### Function Patterns
- Use proper parameter validation with meaningful error messages
- Implement higher-order functions for cross-cutting concerns (retry, caching)
- Apply arrow functions for simple operations and callbacks
- Use function declarations for main business logic

### Class Design
- Implement dependency injection through constructor parameters
- Use private methods for internal operations
- Apply caching strategies for expensive operations
- Validate inputs at public method boundaries

## 5. React Component Patterns

### Component Structure
- Use functional components with hooks as default
- Apply useMemo for expensive calculations
- Use useCallback for event handlers to prevent unnecessary re-renders
- Implement proper loading, error, and empty states

### Custom Hooks
- Create reusable hooks for common patterns (API calls, local storage, debouncing)
- Ensure proper cleanup in useEffect hooks
- Handle component unmounting scenarios
- Return consistent interface objects

### State Management
- Use useState for local component state
- Apply useReducer for complex state logic
- Implement context providers for shared state
- Avoid prop drilling with proper state architecture

## 6. Error Handling Patterns

### Error Class Hierarchy
- Create abstract base error class with common properties
- Implement specific error types for different scenarios
- Use proper error codes and operational flags
- Maintain error cause chains for debugging

### Async Error Handling
- Implement Result<T, E> pattern for explicit error handling
- Use try-catch blocks with specific error type handling
- Apply safe async wrappers for external API calls
- Handle network timeouts and connection errors explicitly

### React Error Boundaries
- Implement error boundaries for component tree protection
- Log errors to monitoring services
- Provide meaningful fallback UI
- Allow error recovery mechanisms

## 7. Testing Patterns

### Unit Testing with Jest
- Mock external dependencies consistently
- Use descriptive test names that explain behavior
- Follow Arrange-Act-Assert pattern
- Test both happy paths and error scenarios

### React Component Testing
- Use React Testing Library for user-focused testing
- Test component behavior, not implementation details
- Mock external API calls and services
- Verify accessibility attributes and keyboard navigation

### Integration Testing
- Test complete user workflows
- Use real API endpoints with test data
- Verify error handling and loading states
- Test responsive design and cross-browser compatibility

## 8. Performance Optimization

### Bundle Optimization
- Implement code splitting with React.lazy()
- Use dynamic imports for feature modules
- Apply tree shaking for unused code elimination
- Optimize third-party library imports

### Runtime Performance
- Use virtual scrolling for large data sets
- Implement debouncing for user input handling
- Apply memoization for expensive calculations
- Use Web Workers for heavy computational tasks

### React Performance
- Apply React.memo() for expensive component re-renders
- Use proper dependency arrays in hooks
- Avoid anonymous functions in JSX props
- Implement proper key attributes for list items

## 9. Best Practices Summary

### TypeScript Best Practices
- Enable strict mode and strict null checks
- Prefer type inference over explicit typing when clear
- Use type guards for runtime type validation
- Avoid `any` type - use `unknown` for dynamic content

### React Best Practices
- Keep components small and single-purpose
- Use composition over inheritance
- Implement proper prop validation with TypeScript
- Follow React hooks rules consistently

### Code Quality
- Use ESLint and Prettier for consistent formatting
- Implement pre-commit hooks for code quality
- Use meaningful variable and function names
- Write self-documenting code with clear intent

## 10. Common Anti-Patterns to Avoid

### TypeScript Anti-Patterns
- Using `any` type without justification
- Non-null assertions without proper validation
- Ignoring strict null checks
- Over-complex generic type definitions

### React Anti-Patterns
- Mutating state objects directly
- Using array indices as React keys
- Calling hooks conditionally
- Creating components inside other components

### General Anti-Patterns
- Silent error handling without logging
- Magic numbers and strings without constants
- Deep object nesting without proper interfaces
- Inconsistent error handling patterns

---

**Note**: Follow the general principles defined in `general-principles.md` along with these JavaScript/TypeScript-specific guidelines. Use ESLint and Prettier for automated code quality enforcement.