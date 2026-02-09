# Flutter/Dart Coding Standards

## 1. Naming Conventions

### Files and Folders
- **Files**: `snake_case.dart` (`home_screen.dart`, `user_service.dart`)
- **Folders**: `snake_case` (`lib/screens/`, `lib/services/`, `lib/models/`)
- **Test files**: `*_test.dart` (`home_screen_test.dart`)

### Code Elements
- **Classes**: `PascalCase` (`HomeScreen`, `UserService`, `DatabaseHelper`)
- **Variables/Methods**: `camelCase` (`userName`, `calculateTotal()`, `isLoggedIn`)
- **Constants**: `lowerCamelCase` (`defaultTimeout`, `maxRetries`)
- **Private members**: `_underscore` (`_privateMethod`, `_internalState`)
- **Enums**: `PascalCase` with `lowerCamelCase` values (`UserStatus.active`, `UserStatus.inactive`)

## 2. Import Organization

### Import Order
1. Dart SDK imports (dart:async, dart:convert, dart:io)
2. Flutter framework imports (package:flutter/material.dart)
3. Third-party package imports (package:provider, package:http)
4. Local imports with relative paths (../models/user.dart)

### Import Guidelines
- Group imports with blank lines between groups
- Order alphabetically within each group
- Use relative imports for local files within the same package
- Prefer specific imports over wildcard imports

## 3. Widget Structure and Organization

### Widget File Organization
1. Constants (static const declarations)
2. Fields (final and non-final instance variables)
3. Constructor (with proper parameter organization)
4. Build method (main widget construction)
5. Private helper methods (for widget composition)

### Widget Design Principles
- Use const constructors whenever possible for performance
- Extract complex widgets into separate classes
- Keep build methods simple and readable
- Use meaningful widget names that describe their purpose
- Prefer composition over inheritance for widget reuse

## 4. State Management Patterns

### Local State Management
- Use StatefulWidget for component-specific state
- Apply proper widget lifecycle management
- Implement proper disposal of controllers and resources
- Use setState() judiciously to minimize rebuilds

### Global State Management
- Choose appropriate state management solution (Provider, Bloc, Riverpod)
- Separate business logic from UI components
- Implement proper state persistence when needed
- Use immutable state objects for predictable updates

### State Update Strategies
- Apply proper state normalization techniques
- Implement optimistic updates for better UX
- Use proper loading and error states
- Handle offline scenarios gracefully

## 5. Async Programming and Futures

### Future and Stream Usage
- Use async/await consistently for asynchronous operations
- Implement proper error handling with try-catch blocks
- Apply stream controllers for reactive programming
- Use proper subscription management and disposal

### Error Handling Patterns
- Create custom exception classes for different error types
- Implement proper error boundaries and recovery
- Use Result pattern for explicit error handling
- Provide meaningful error messages to users

## 6. Performance Optimization

### Widget Performance
- Use const widgets to prevent unnecessary rebuilds
- Implement proper shouldRebuild logic for custom widgets
- Apply RepaintBoundary for expensive widgets
- Use ListView.builder for large lists instead of ListView

### Memory Management
- Dispose controllers, streams, and subscriptions properly
- Use weak references for large cached objects
- Implement proper image caching strategies
- Monitor memory usage with development tools

### Build Optimization
- Minimize widget tree depth where possible
- Use keys appropriately for widget identity
- Implement lazy loading for expensive operations
- Profile application performance regularly

## 7. Navigation and Routing

### Navigation Patterns
- Use named routes for complex navigation structures
- Implement proper route guards for authentication
- Apply proper back stack management
- Use proper transition animations

### Deep Linking
- Implement proper URL handling for web applications
- Use route parameters for dynamic content
- Apply proper route validation and error handling
- Implement proper state restoration from URLs

## 8. Testing Strategies

### Widget Testing
- Test widget behavior rather than implementation details
- Use proper test doubles for external dependencies
- Verify proper accessibility attributes
- Test different screen sizes and orientations

### Unit Testing
- Test business logic separately from UI components
- Mock external dependencies consistently
- Use proper test data builders for complex objects
- Test error scenarios and edge cases

### Integration Testing
- Test complete user workflows
- Use proper test environment setup
- Verify proper navigation and state changes
- Test offline scenarios and error recovery

## 9. Platform Integration

### Platform Channels
- Implement proper platform channel communication
- Use proper error handling for platform operations
- Apply consistent data serialization strategies
- Handle platform-specific differences gracefully

### Native Dependencies
- Use well-maintained packages with active support
- Implement proper fallbacks for missing platform features
- Test on all target platforms regularly
- Use proper permission handling strategies

## 10. Accessibility and Internationalization

### Accessibility Implementation
- Use semantic widgets for screen readers
- Implement proper focus management
- Apply appropriate contrast ratios and font sizes
- Test with accessibility tools and real users

### Internationalization Support
- Use proper localization strategies (intl package)
- Implement right-to-left layout support when needed
- Apply proper date, number, and currency formatting
- Test with different locales and languages

## 11. Code Quality and Architecture

### Architecture Patterns
- Apply proper separation of concerns
- Use dependency injection for testability
- Implement proper abstraction layers
- Follow SOLID principles where applicable

### Code Organization
- Group related functionality into packages
- Use proper barrel exports for clean APIs
- Implement consistent file and folder naming
- Apply proper documentation standards

## 12. Common Anti-Patterns to Avoid

### Widget Anti-Patterns
- Creating huge build methods without extraction
- Not using const constructors when possible
- Calling setState in build methods
- Creating widgets inside other widgets' build methods
- Using too many nested widgets without extraction

### State Management Anti-Patterns
- Mixing business logic with UI components
- Not disposing resources properly
- Using global state for local component state
- Implementing complex state mutations without proper patterns
- Not handling loading and error states consistently

### Performance Anti-Patterns
- Not using ListView.builder for large lists
- Creating expensive operations in build methods
- Not implementing proper image caching
- Using too many unnecessary widget rebuilds
- Not profiling performance before optimization

### Architecture Anti-Patterns
- Tight coupling between UI and business logic
- Not using proper dependency injection
- Implementing God classes with too many responsibilities
- Not following consistent error handling patterns
- Mixing different architectural patterns inconsistently

---

**Note**: Follow the general principles defined in `general-principles.md` along with these Flutter/Dart-specific guidelines. Use Flutter's built-in analysis tools and proper linting rules to enforce these standards.