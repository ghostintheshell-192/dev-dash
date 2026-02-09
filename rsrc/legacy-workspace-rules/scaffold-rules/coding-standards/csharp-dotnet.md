# C#/.NET Coding Standards

## 1. Naming Conventions

### Files and Folders
- **Files**: `PascalCase.cs` (`UserController.cs`, `DatabaseService.cs`)
- **Folders**: `PascalCase` (`Controllers/`, `Services/`, `Models/`)
- **Test files**: `*Tests.cs` (`UserServiceTests.cs`, `ControllerTests.cs`)

### Code Elements
- **Classes/Interfaces**: `PascalCase` (`UserService`, `IRepository`, `DatabaseContext`)
- **Methods/Properties**: `PascalCase` (`GetUsers()`, `UserName`, `IsActive`)
- **Fields**: `_camelCase` (`_userId`, `_connectionString`)
- **Constants**: `PascalCase` (`DefaultTimeout`, `MaxRetries`)
- **Local variables**: `camelCase` (`userName`, `totalCount`, `isValid`)
- **Parameters**: `camelCase` (`userId`, `connectionString`)

### Interfaces and Generics
- Interfaces start with 'I' (`IUserRepository`, `IApiClient`)
- Generic type parameters use single letters starting with 'T' (`T`, `TKey`, `TValue`)

## 2. Using Statements and Organization

### Using Order
1. System namespaces (System, System.Collections.Generic)
2. Microsoft namespaces (Microsoft.AspNetCore, Microsoft.Extensions)
3. Third-party namespaces (AutoMapper, FluentValidation, Newtonsoft.Json)
4. Local project namespaces (relative to project root)

### File Organization
- Group constants at class level
- Order fields before constructor
- Place constructor before public methods
- Implement private methods after public methods

## 3. Dependency Injection and Architecture

### Controller Pattern
- Use constructor injection for dependencies
- Validate parameters at controller entry points
- Apply proper HTTP status codes for different scenarios
- Implement consistent error response format
- Use action filters for cross-cutting concerns

### Service Pattern
- Implement service interfaces for dependency inversion
- Use constructor validation with ArgumentNullException
- Apply business logic validation with custom exceptions
- Implement proper logging at service boundaries
- Use mapping between DTOs and domain entities

### Repository Pattern
- Create repository interfaces for data access abstraction
- Implement async methods for all database operations
- Use Entity Framework query optimization techniques
- Apply proper transaction management
- Implement proper disposal patterns for database contexts

## 4. Data Models and DTOs

### Entity Model Design
- Use data annotations for validation
- Implement navigation properties for relationships
- Apply proper indexing strategies
- Use computed properties for derived values
- Implement audit fields (CreatedAt, UpdatedAt)

### DTO Pattern Usage
- Create separate DTOs for create, read, update operations
- Use validation attributes on DTO properties
- Implement proper mapping strategies (AutoMapper)
- Apply pagination for list responses
- Use wrapper objects for API responses

## 5. Error Handling and Exceptions

### Custom Exception Hierarchy
- Create abstract base exception classes
- Implement specific exception types for business rules
- Use proper HTTP status code mapping
- Maintain exception cause chains
- Apply operational vs programming error distinction

### Global Exception Handling
- Implement middleware for centralized exception handling
- Log exceptions with structured logging
- Return consistent error response format
- Handle different exception types appropriately
- Avoid exposing sensitive information in error messages

## 6. Async/Await and Threading

### Async Best Practices
- Use async/await consistently throughout call chain
- Apply ConfigureAwait(false) in library code
- Avoid blocking async calls with .Result or .Wait()
- Use CancellationToken for long-running operations
- Implement proper exception handling in async methods

### Threading Considerations
- Use thread-safe collections when needed
- Apply proper locking mechanisms for shared resources
- Avoid deadlocks with proper async patterns
- Use Task.Run for CPU-bound operations
- Implement proper disposal in async scenarios

## 7. Testing Patterns

### Unit Testing Structure
- Use AAA pattern (Arrange, Act, Assert)
- Create descriptive test method names
- Use test fixtures for complex setup
- Mock external dependencies consistently
- Test both happy path and error scenarios

### Integration Testing
- Use TestHost for API testing
- Apply in-memory database for isolated tests
- Test complete request/response cycles
- Verify proper HTTP status codes and content
- Use test data builders for complex scenarios

### Mocking Strategies
- Use Moq for interface mocking
- Verify method calls and parameter values
- Set up mock return values and exceptions
- Use callback verification for complex scenarios
- Apply proper mock lifecycle management

## 8. Performance and Optimization

### Entity Framework Optimization
- Use Include() for eager loading when needed
- Apply proper indexing strategies
- Implement query result caching
- Use compiled queries for repeated operations
- Avoid N+1 query problems

### API Performance
- Implement response caching where appropriate
- Use compression for large responses
- Apply proper pagination strategies
- Implement API versioning for backward compatibility
- Use CDN for static content delivery

### Memory Management
- Implement proper disposal patterns
- Use object pooling for expensive objects
- Apply weak references for large cached objects
- Monitor memory usage in production
- Use memory profiling tools for optimization

## 9. Security Best Practices

### Authentication and Authorization
- Use JWT tokens with proper expiration
- Implement role-based authorization
- Apply proper password hashing (bcrypt)
- Use HTTPS for all sensitive communications
- Implement proper session management

### Input Validation
- Validate all input at API boundaries
- Use parameterized queries for database operations
- Apply proper sanitization for user content
- Implement rate limiting for API endpoints
- Use CORS policies appropriately

### Data Protection
- Encrypt sensitive data at rest
- Use secure communication protocols
- Implement proper logging without sensitive data
- Apply data anonymization for non-production environments
- Use environment variables for secrets management

## 10. Configuration and Deployment

### Configuration Management
- Use appsettings.json for environment-specific settings
- Implement proper configuration validation
- Use strongly-typed configuration objects
- Apply configuration inheritance patterns
- Implement feature flags for gradual rollouts

### Deployment Strategies
- Use containerization with Docker
- Implement proper health checks
- Apply database migration strategies
- Use blue-green deployment for zero downtime
- Implement proper monitoring and alerting

## 11. Best Practices Summary

### Code Quality
- Use nullable reference types consistently
- Apply proper const and readonly usage
- Implement proper string interpolation
- Use collection expressions where appropriate
- Apply proper using statement patterns

### Architecture Principles
- Follow SOLID design principles
- Implement proper separation of concerns
- Use dependency injection consistently
- Apply domain-driven design concepts
- Implement proper abstraction layers

### Development Workflow
- Use Git branching strategies effectively
- Implement proper code review processes
- Apply continuous integration practices
- Use static analysis tools (SonarQube, Roslyn)
- Implement proper documentation standards

## 12. Common Anti-Patterns to Avoid

### Repository Anti-Patterns
- Exposing IQueryable from repository methods
- Creating generic repositories without business context
- Implementing repository pattern over Entity Framework without need
- Using repository for simple CRUD operations
- Not applying proper unit of work patterns

### Service Anti-Patterns
- Creating anemic service classes without business logic
- Tight coupling between services and data access
- Not implementing proper transaction boundaries
- Using service locator pattern instead of dependency injection
- Creating God services with too many responsibilities

### General Anti-Patterns
- Using exceptions for control flow
- Not disposing IDisposable objects properly
- Blocking async calls in synchronous methods
- Using ConfigureAwait(true) in library code
- Implementing manual JSON serialization instead of using libraries

---

**Note**: Follow the general principles defined in `general-principles.md` along with these C#/.NET-specific guidelines. Use static analysis tools like SonarQube or Roslyn analyzers to enforce these standards.