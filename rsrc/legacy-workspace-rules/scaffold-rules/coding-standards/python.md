# Python Coding Standards

## 1. Naming Conventions

### Files and Modules
- **Files/Modules**: `snake_case.py` (`data_processor.py`, `user_manager.py`)
- **Packages**: `snake_case` (`data_processing/`, `user_management/`)
- **Test files**: `test_*.py` or `*_test.py` (`test_user_manager.py`)

### Code Elements
- **Classes**: `PascalCase` (`DataProcessor`, `UserManager`, `DatabaseConnection`)
- **Functions/Variables**: `snake_case` (`process_data()`, `user_count`, `is_valid`)
- **Constants**: `UPPER_SNAKE_CASE` (`MAX_RETRIES`, `DEFAULT_TIMEOUT`, `API_BASE_URL`)
- **Private members**: `_leading_underscore` (`_internal_method`, `_private_var`)
- **Very private**: `__double_underscore` (name mangling, rarely used)

## 2. Import Organization

### Import Order (PEP 8)
1. Standard library imports (os, sys, json, pathlib, logging)
2. Related third-party imports (requests, pandas, numpy, flask)
3. Local application/library imports (relative to project)

### Import Guidelines
- Group imports with blank lines between groups
- Order alphabetically within each group
- Use absolute imports for packages, relative for local modules
- Avoid `import *` except in `__init__.py` when appropriate

### Correct Example:
```python
# Standard library
import logging
import os
import sys

# Third-party
import requests
from flask import Flask, jsonify
from pydantic import BaseModel

# Local imports
from .config import settings
from .utils import helper_function
```

### Incorrect Example:
```python
from flask import Flask, jsonify
from pydantic import BaseModel
import requests
import logging  # ❌ Mixed standard library with third-party
```

## 3. Function and Class Design

### Function Design Principles
- Use type hints consistently for parameters and return values
- Implement proper parameter validation at function entry
- Apply default parameter values appropriately
- Use descriptive docstrings following Google or NumPy style
- Keep functions focused on single responsibility

### Class Structure Organization
- Define `__init__` method with proper parameter validation
- Use `@property` decorators for computed attributes
- Implement `__str__` and `__repr__` methods for debugging
- Apply proper method organization (public before private)
- Use dataclasses for simple data containers

### Type Hinting Best Practices
- Use Union for multiple possible types
- Apply Optional for nullable values
- Use generics for reusable type definitions
- Import type hints from typing module consistently
- Use TypeVar for generic function definitions

## 4. Error Handling and Exceptions

### Exception Hierarchy Design
- Create custom exception classes inheriting from appropriate base classes
- Use specific exception types rather than generic Exception
- Implement proper exception chaining with `raise from`
- Apply descriptive error messages with context
- Use exceptions for exceptional conditions, not control flow

### Error Handling Patterns
- Use try-catch blocks with specific exception types
- Implement proper resource cleanup with context managers
- Apply proper logging at appropriate levels
- Handle exceptions at the right abstraction level
- Use finally blocks for guaranteed cleanup operations

## 5. Data Structures and Collections

### Collection Usage Guidelines
- Use list comprehensions for simple transformations
- Apply generator expressions for memory-efficient processing
- Use dict comprehensions for dictionary creation
- Prefer collections.defaultdict for default value scenarios
- Use sets for membership testing and deduplication

### Data Class and Named Tuple Usage
- Use dataclasses for mutable data containers
- Apply NamedTuple for immutable data structures
- Implement proper validation in `__post_init__` methods
- Use frozen dataclasses for immutable objects
- Apply proper field defaults and factory functions

## 6. Async Programming and Concurrency

### Async/Await Patterns
- Use async/await consistently for I/O-bound operations
- Apply proper exception handling in async functions
- Use asyncio.gather() for concurrent operations
- Implement proper cancellation handling
- Apply proper timeout handling for network operations

### Threading and Multiprocessing
- Use threading for I/O-bound tasks
- Apply multiprocessing for CPU-bound tasks
- Use proper synchronization primitives (locks, queues)
- Implement proper resource sharing strategies
- Apply proper cleanup for background threads

## 7. Testing Strategies

### Unit Testing Structure
- Use unittest or pytest frameworks consistently
- Apply proper test isolation and independence
- Use descriptive test method names
- Implement proper setup and teardown methods
- Mock external dependencies appropriately

### Test Organization Patterns
- Group related tests in test classes
- Use parametrized tests for data-driven testing
- Apply proper test fixtures for complex setup
- Use property-based testing for complex algorithms
- Implement proper integration test strategies

## 8. Performance and Optimization

### Performance Best Practices
- Profile code before optimizing
- Use appropriate data structures for the use case
- Apply caching strategies for expensive operations
- Use generators for memory-efficient processing
- Implement proper database query optimization

### Memory Management
- Use weak references for large cached objects
- Apply proper cleanup for file handles and network connections
- Use context managers for resource management
- Monitor memory usage in production environments
- Implement proper garbage collection strategies

## 9. Package and Dependency Management

### Virtual Environment Management
- **Always use virtual environments** for project isolation
- Create with: `python -m venv venv` (name the directory `venv`)
- Activate: `source venv/bin/activate` (Linux/Mac) or `venv\Scripts\activate` (Windows)
- Deactivate: `deactivate`
- **Never commit `venv/` directory** - add to `.gitignore`
- Include activation instructions in README.md for new developers
- Use `pip freeze > requirements.txt` to capture exact versions

### Package Structure
- Use proper `__init__.py` files for package organization
- Implement consistent package versioning
- Apply proper dependency specification
- Use virtual environments for project isolation
- Implement proper build and distribution strategies

### Dependency Management
- Use requirements.txt or pyproject.toml for dependencies
- Pin dependency versions for reproducible builds (`pandas>=1.5.0,<2.0.0`)
- Apply proper dependency updates and security scanning
- Use lock files for exact dependency specification
- Implement proper dependency injection patterns
- Separate dev dependencies: `requirements-dev.txt` or use pyproject.toml sections

## 10. Documentation and Code Quality

### Documentation Standards
- Use docstrings for all public modules, classes, and functions
- Apply consistent docstring format (Google, NumPy, or Sphinx)
- Include type information in docstrings when helpful
- Provide usage examples for complex functions
- Document return values and raised exceptions

### Code Quality Tools
- Use black for automatic code formatting
- Apply flake8 or pylint for linting
- Use mypy for static type checking
- Implement pre-commit hooks for code quality
- Apply proper configuration for all tools

## 11. Security Best Practices

### Input Validation and Sanitization
- Validate all user input at application boundaries
- Use parameterized queries for database operations
- Apply proper sanitization for user-generated content
- Implement rate limiting for API endpoints
- Use proper authentication and authorization mechanisms

### Secure Coding Practices
- Never hardcode secrets or credentials
- Use environment variables for configuration
- Apply proper encryption for sensitive data
- Implement proper session management
- Use secure communication protocols (HTTPS, TLS)

## 12. Common Anti-Patterns to Avoid

### Import Anti-Patterns
- Using wildcard imports (`from module import *`)
- Circular import dependencies
- Importing modules inside functions unnecessarily
- Using relative imports incorrectly
- Not following PEP 8 import order

### Exception Handling Anti-Patterns
- Using bare `except:` clauses that catch all exceptions
- Catching exceptions without handling them appropriately
- Using exceptions for normal control flow
- Not providing sufficient error context
- Swallowing exceptions silently

### Performance Anti-Patterns
- Using string concatenation in loops instead of join()
- Not using list comprehensions when appropriate
- Creating unnecessary intermediate lists
- Not using appropriate data structures for the use case
- Premature optimization without profiling

### Code Organization Anti-Patterns
- Creating monolithic modules with too many responsibilities
- Not using proper separation of concerns
- Mixing business logic with presentation logic
- Not following consistent naming conventions
- Creating deep inheritance hierarchies unnecessarily

---

**Note**: Follow PEP 8 style guide and the general principles defined in `general-principles.md` along with these Python-specific guidelines. Use automated tools like black, flake8, and mypy for code quality enforcement.