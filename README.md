# psr_database

[![CI](https://github.com/raphasampaio/psr_database/actions/workflows/ci.yml/badge.svg)](https://github.com/raphasampaio/psr_database/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/raphasampaio/psr_database/graph/badge.svg?token=TwhY6YNDNN)](https://codecov.io/gh/raphasampaio/psr_database)

A cross-platform C++17 SQLite wrapper library with migration support and a C API for FFI integration.

## Features

- Modern C++17 API with RAII resource management
- Schema migrations with version tracking
- Cross-platform support (Windows, Linux, macOS)
- SQLite embedded via CMake FetchContent
- C API for FFI integration with other languages
- Lua scripting support via sol2
- Logging via spdlog
- TOML configuration support via toml++

## Building

### Prerequisites

- CMake 3.21+
- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+, MinGW)

### Quick Start

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DPSR_BUILD_C_API=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

### Install

```bash
cmake --install build --prefix /usr/local
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `PSR_BUILD_SHARED` | ON | Build shared library |
| `PSR_BUILD_TESTS` | ON | Build test suite |
| `PSR_BUILD_C_API` | OFF | Build C API wrapper |

### Code Formatting

```bash
find include src tests -name '*.cpp' -o -name '*.h' | xargs clang-format -i
```

## Usage

### C++ API

```cpp
#include <psr/database.h>

// Open database with migrations
auto db = psr::Database::from_schema("app.db", "schema/");

// Or open directly
psr::Database db(":memory:");
db.execute("CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT)");
db.execute("INSERT INTO users (name) VALUES (?)", {psr::Value{"Alice"}});

auto result = db.execute("SELECT * FROM users");
for (const auto& row : result) {
    std::cout << row.get_string(1).value() << std::endl;
}
```

### Migrations

Create numbered folders with `up.sql` files:

```
schema/
├── 1/
│   └── up.sql    # CREATE TABLE users (...)
├── 2/
│   └── up.sql    # CREATE TABLE posts (...)
└── 3/
    └── up.sql    # ALTER TABLE users ADD COLUMN email TEXT
```

```cpp
// Opens database and applies all pending migrations
auto db = psr::Database::from_schema("app.db", "schema/");

// Check current version
int64_t version = db.current_version();

// Manually apply migrations
db.migrate_up();
```

### C API

```c
#include <psr/c/database.h>
#include <psr/c/result.h>

psr_error_t error;

// Open with migrations
psr_database_t* db = psr_database_from_schema("app.db", "schema/", &error);

// Or open directly
psr_database_t* db = psr_database_open(":memory:", &error);

// Get current version
int64_t version = psr_database_current_version(db);

psr_result_t* result = psr_database_execute(db, "SELECT * FROM users", &error);
// ... iterate over results
psr_result_free(result);
psr_database_close(db);
```

### Using with CMake

```cmake
find_package(psr_database REQUIRED)
target_link_libraries(myapp PRIVATE psr::database)
```

## Dependencies

All dependencies are fetched automatically via CMake FetchContent:

- **SQLite** (v3.47.2) - Database engine
- **toml++** (v3.4.0) - TOML parsing
- **spdlog** (v1.15.0) - Logging
- **Lua** (v5.4.8) - Scripting engine
- **sol2** (v3.3.1) - Lua C++ bindings
- **GoogleTest** (v1.15.2) - Testing (optional)

## Project Structure

```
psr_database/
├── include/psr/        # Public headers
│   ├── database.h      # C++ Database class
│   ├── result.h        # C++ Result/Row/Value
│   ├── export.h        # DLL export macros
│   └── c/              # C API headers
│       ├── database.h
│       └── result.h
├── src/                # Implementation
│   ├── database.cpp
│   ├── result.cpp
│   └── c_api.cpp       # C API wrapper
├── tests/
└── cmake/
```

## License

See LICENSE file for details.
