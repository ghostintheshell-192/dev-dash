# C/C++ Coding Standards

## 1. Naming Conventions

### Files and Folders
- **Header files**: `snake_case.h` (`network_handler.h`, `data_parser.h`)
- **Source files**: `snake_case.c/.cpp` (`network_handler.c`, `data_parser.cpp`)
- **Folders**: `snake_case` (`src/`, `include/`, `test_utils/`)

### Code Elements
- **Functions/Variables**: `snake_case` (`calculate_average()`, `user_count`, `is_valid`)
- **Constants/Macros**: `UPPER_SNAKE_CASE` (`MAX_BUFFER_SIZE`, `PI_VALUE`, `DEFAULT_TIMEOUT`)
- **Types/Structs**: `snake_case_t` (`user_data_t`, `config_t`, `connection_info_t`)
- **Header guards**: `UPPER_SNAKE_CASE_H` (`NETWORK_HANDLER_H`, `DATA_PARSER_H`)
- **Enums**: `UPPER_SNAKE_CASE` values (`STATUS_SUCCESS`, `STATUS_ERROR`)

### C++ Specific
- **Classes**: `PascalCase` (`NetworkHandler`, `DataParser`)
- **Namespaces**: `snake_case` (`network_utils`, `data_processing`)
- **Template parameters**: `PascalCase` (`template<typename DataType>`)

## 2. Header File Organization

### Include Order and Structure
1. System headers (stdio.h, stdlib.h, string.h)
2. Platform-specific headers with conditional compilation
3. Third-party library headers
4. Project-specific headers

### Header Guard Pattern
- Use traditional `#ifndef/#define/#endif` guards
- Include C++ extern "C" guards for C headers used in C++
- Forward declare dependencies when possible
- Use PIMPL idiom for C++ classes to reduce compilation dependencies

## 3. Source File Organization

### File Structure
- Include corresponding header first
- Group internal constants and static variables
- Declare internal functions before implementation
- Implement public functions before internal ones
- Use static keyword for internal functions

## 4. Error Handling Patterns

### C Error Handling
- Use enumerated error codes with meaningful names
- Return error codes from functions, use output parameters for data
- Check and handle all error conditions explicitly
- Provide meaningful error messages with context

### C++ Exception Handling
- Create custom exception hierarchy with base classes
- Use RAII for automatic resource management
- Apply exception safety guarantees (basic, strong, no-throw)
- Handle specific exception types rather than catching all

## 5. Memory Management

### C Memory Management
- Always check malloc/calloc return values for NULL
- Match every malloc with corresponding free
- Set pointers to NULL after freeing
- Use safe memory allocation wrappers
- Implement cleanup functions for complex structures

### C++ RAII and Smart Pointers
- Use RAII for automatic resource cleanup
- Prefer smart pointers over raw pointers for ownership
- Use unique_ptr for exclusive ownership
- Apply shared_ptr only when shared ownership is necessary
- Implement proper move semantics and Rule of 5

## 6. Function and Class Design

### Function Design Principles
- Keep functions focused on single responsibility
- Validate input parameters at function entry
- Use const correctness for read-only parameters
- Prefer stack allocation over heap when possible
- Document complex algorithms and business logic

### C++ Class Design
- Follow SOLID principles for class design
- Use constructor initialization lists
- Implement proper copy/move constructors when needed
- Apply const member functions for non-modifying operations
- Use private members with public accessor methods

## 7. Testing Patterns

### C Testing Framework
- Create simple assertion macros for test validation
- Use function pointers for test case organization
- Implement test runners with pass/fail reporting
- Test both success and failure scenarios
- Use descriptive test function names

### C++ Testing with Google Test
- Use TEST and TEST_F macros for organized testing
- Apply proper setup and teardown in test fixtures
- Mock external dependencies consistently
- Test exception handling scenarios
- Use parameterized tests for data-driven testing

## 8. Performance and Optimization

### C Performance
- Use appropriate data types for size requirements
- Minimize dynamic memory allocation in performance-critical code
- Use compiler optimization flags appropriately
- Profile code before optimizing
- Consider cache-friendly data structures

### C++ Performance
- Use move semantics for expensive objects
- Apply template metaprogramming for compile-time optimization
- Prefer algorithms over hand-written loops
- Use reserve() for containers when size is known
- Implement proper const-correctness for optimization opportunities

## 9. Platform and Portability

### Cross-Platform Considerations
- Use standard library functions when available
- Isolate platform-specific code with conditional compilation
- Use appropriate integer types for platform independence
- Handle endianness differences for binary data
- Test on target platforms regularly

### Compiler Compatibility
- Use standard-compliant C/C++ features
- Avoid compiler-specific extensions unless necessary
- Test with multiple compiler versions
- Use appropriate compiler warning levels
- Handle deprecated feature warnings

## 10. Best Practices Summary

### C Best Practices
- Always initialize variables at declaration
- Use const for read-only data and parameters
- Check return values of system calls
- Use static for internal functions
- Document complex pointer operations

### C++ Best Practices
- Follow RAII for resource management
- Use smart pointers instead of raw pointers
- Prefer stack allocation over heap allocation
- Use const correctness throughout codebase
- Apply Rule of 3/5/0 consistently

### Code Quality
- Use static analysis tools (Clang Static Analyzer, Cppcheck)
- Implement proper error checking
- Write self-documenting code with clear names
- Use version control for all code changes
- Maintain consistent coding style

## 11. Common Anti-Patterns to Avoid

### Memory Management Anti-Patterns
- Not checking malloc return values
- Memory leaks from missing free calls
- Double-free errors
- Using freed memory (dangling pointers)
- Buffer overflows from unchecked bounds

### C++ Anti-Patterns
- Raw pointer ownership ambiguity
- Missing Rule of 5 implementation
- Catching all exceptions without specificity
- Manual resource management instead of RAII
- Premature optimization without profiling

### General Anti-Patterns
- Magic numbers without named constants
- Deep function nesting without extraction
- Global variables instead of proper parameter passing
- Ignoring compiler warnings
- Inconsistent error handling patterns

---

**Note**: Follow the general principles defined in `general-principles.md` along with these C/C++-specific guidelines. Use static analysis tools like Clang Static Analyzer, Cppcheck, or PVS-Studio to enforce these standards.