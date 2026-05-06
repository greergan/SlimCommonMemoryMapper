# slim::common::memory_mapper

A thread-safe, named shared-memory map library for storing and retrieving typed values (`bool`, `std::string`, `std::shared_ptr<std::string>`) across a process.

Version macros are stamped at configure time via CMake:

```cpp
SLIMCOMMONMEMORYMAPPER_VERSION   // e.g. "1.0.0"
SLIMCOMMONMEMORYMAPPER_GIT_HASH  // e.g. "a1b2c3d"
```

## Dependencies

| Package | Notes |
|---------|-------|
| `SlimCommonLog` | Used internally for trace, debug, and error logging |

## Concepts

The mapper maintains a global registry of named maps. Each map is a `shared_ptr` to an `unordered_map<string, content_variant>` and has its own dedicated mutex for fine-grained locking.

```
maps
 └── "my_map"  →  map_container (shared_ptr)
                    ├── "key_a"  →  bool
                    ├── "key_b"  →  std::string
                    └── "key_c"  →  shared_ptr<std::string>
```

Values are stored as a `content_variant`:

```cpp
using content_variant = std::variant<bool, std::string, std::shared_ptr<std::string>>;
```

## Usage

### Create or Attach a Map

```cpp
// Create a new empty map (no-op if it already exists)
slim::common::memory_mapper::create("my_map");

// Attach an existing shared map (useful for sharing across translation units)
auto shared = std::make_shared<slim::common::memory_mapper::map_container>();
slim::common::memory_mapper::attach("my_map", shared);
```

### Write

```cpp
memory_mapper::write("my_map", "flag",    true);
memory_mapper::write("my_map", "name",    std::string("slim"));
memory_mapper::write("my_map", "payload", std::make_shared<std::string>("large data"));
```

`write` creates the map automatically if it does not exist. Returns `true` if the entry was newly inserted, `false` if it was updated or an error occurred.

### Read

```cpp
bool        flag    = memory_mapper::read_bool("my_map", "flag");
std::string name    = memory_mapper::read_string("my_map", "name");
std::string_view sv = memory_mapper::read_string_view("my_map", "name");

// Returns shared_ptr<string>; handles both string and shared_ptr<string> variants
auto ptr = memory_mapper::read("my_map", "payload");
if (ptr) {
    std::cout << *ptr << "\n";
}
```

> **Note:** `read_string_view` returns a view into memory owned by the map. Do not hold the view across writes or map erasure.

### Inspect

```cpp
memory_mapper::map_exists("my_map");                    // true/false
memory_mapper::variable_exists("my_map", "flag");       // true/false

auto keys = memory_mapper::list_keys("my_map");         // std::vector<std::string>
```

### Erase

```cpp
memory_mapper::erase("my_map");                // remove the entire map
memory_mapper::erase("my_map", "flag");        // remove a single variable
```

> **Note:** Full map erasure (`erase(map_name)`) is currently a no-op pending a thread-safe implementation. Single-variable erasure is declared but not yet shown in the header — check the installed version for availability.

## API Reference

```cpp
namespace slim::common::memory_mapper {

using content_variant = std::variant<bool, std::string, std::shared_ptr<std::string>>;
using map_container   = std::unordered_map<std::string, content_variant>;
using map_pointer     = std::shared_ptr<map_container>;

// Lifecycle
bool attach(const std::string& map_name, map_pointer map);
bool create(const std::string& map_name);
void erase(const std::string& map_name);
void erase(const std::string& map_name, const std::string& variable_name);

// Inspection
bool map_exists(const std::string& map_name);
bool variable_exists(const std::string& map_name, const std::string& variable_name);
const std::vector<std::string> list_keys(const std::string& map_name);

// Read
std::shared_ptr<std::string> read(const std::string& map_name, const std::string& variable_name);
bool             read_bool(const std::string& map_name, const std::string& variable_name);
std::string      read_string(const std::string& map_name, const std::string& variable_name);
std::string_view read_string_view(const std::string& map_name, const std::string& variable_name);

// Write
bool write(const std::string& map_name, const std::string& variable_name, content_variant content);

} // namespace slim::common::memory_mapper
```

### Error Handling

All functions validate their inputs and throw a `const char*` exception if a map or variable name is empty. Allocation failures (`std::bad_alloc`) are caught internally, logged via `SlimCommonLog`, and return `false` rather than propagating.

## Building

```cmake
configure_file(include/slim/common/memory/mapper.h.in include/slim/common/memory/mapper.h)
target_include_directories(your_target PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
target_sources(your_target PRIVATE src/main.cpp)
```

Requires a C++20-capable compiler (`std::format`, `std::unordered_map::contains`).

## Releases

See the [GitHub Releases](https://github.com/your-org/slim/releases) page for downloads and full release notes.

### Changelog

#### v0.1.0
- Initial release
- Named map registry with per-map mutex locking
- `attach` and `create` for map lifecycle management
- `write` with `bool`, `std::string`, and `shared_ptr<std::string>` support
- `read`, `read_bool`, `read_string`, `read_string_view` accessors
- `map_exists`, `variable_exists`, `list_keys` inspection utilities
- Integrated `SlimCommonLog` tracing and error reporting
