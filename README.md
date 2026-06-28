# DrON 1.0

A small, read-only configuration and data loader for C++20. It parses an
INI-style `.dron` file into a tree and lets you navigate that tree and pull
typed values with defaults. It never throws on a bad path - a missing key,
wrong type, or out-of-bounds index returns your fallback and logs once.

DrON 1.0 is **frozen**. It loads files. It does not write, save, serialize, or
round-trip. That is by design, not omission.

---

## The format

Sections, each a map of `key = value`. Values can nest.

```ini
# comments start with #
[Transform]
x = 160.0
y = 90.0
rotation = 0.0

[Meta]
type = PLAYER            # bare UPPER_CASE identifier - parses to a string

[Ship]
spawn = [160.0, 90.0]    # list
split = { into = MEDIUM, count = 2 }   # map
```

### Value types

| Literal          | Stored as     | Notes |
|------------------|---------------|-------|
| `"hello"`        | `std::string` | quoted string |
| `42`, `-7`       | `int64_t`     | the only integer type |
| `3.14`, `.5`     | `double`      | `.5` normalizes to `0.5` |
| `true` `false`   | `bool`        | |
| `UPPER_CASE`     | `std::string` | identifier - no distinct type, it's a string |
| `[ ... ]`        | list          | nestable |
| `{ k = v, ... }` | map           | nestable |

Identifiers, quoted strings, and multiline strings all land as `std::string`.
There is no identifier or enum type - if you want an enum, compare the string.

Multiline strings use triple quotes:

```ini
desc = """
line one
line two
"""
```

---

## Reading a file

```cpp
#include "DronReader.hpp"

DronReader reader("config.dron");   // loads on construction

double x   = reader["Transform"]["x"].as<double>(0.0);
int64_t w  = reader["Sprite"]["width"].as<int64_t>(0);
bool alive = reader["Player"]["alive"].as<bool>(false);
std::string name = reader["Meta"]["name"].as<std::string>("unnamed");
```

`reader["Section"]["key"]` descends into the tree. `.as<T>(fallback)` pulls the
value out as type `T`, returning `fallback` if the path is missing, the type is
wrong, or anything else goes sideways. It never throws.

You can also construct empty and load later:

```cpp
DronReader reader;
reader.load("config.dron");
```

---

## The four capabilities

**1. Descend with `operator[]`.** On the reader by section name, on a value by
key or by list index. Composes to any depth and never throws.

```cpp
reader["Nested"]["outer"]["inner"]["deep"].as<std::string>("");
reader["Tags"]["list"][0].as<std::string>("");
```

**2. Safe descent on bad paths.** Indexing something missing or wrong-typed
returns an *invalid* value that is still safely descendable. The chain never
blows up; the failure surfaces at the leaf when you call `as<T>`.

```cpp
// no crash even though none of this exists - returns 99, logs once
reader["Nope"]["missing"]["deeper"].as<int64_t>(99);
```

**3. `as<T>(fallback)` - the value accessor.** Valid `T`: `int64_t`, `double`,
`bool`, `std::string`. Ask for the type the value is actually stored as - there
is **no narrowing or conversion**. `as<int>(0)` or a bare string literal like
`as("x")` will not match and will always return the fallback. Read numbers as
`int64_t` / `double` and cast at the call site if you need a narrower type.

**4. `items()` / `elements()` - iteration.** When you don't know the keys ahead
of time. `items()` walks a map's key/value pairs, `elements()` walks a list.
Both yield `DronValue` so you keep the safe API. Iterating a non-map or non-list
yields nothing - no throw.

```cpp
for (auto& [key, value] : reader["Maps"]["flat"].items()) {
    // key is the entry name, value is a DronValue
}

for (auto element : reader["Lists"]["ints"].elements()) {
    int64_t n = element.as<int64_t>(0);
}

// walk every section without knowing their names (the spawn-walk pattern)
for (auto& [sectionName, sectionNode] : reader) {
    // sectionName is a std::string, sectionNode a DronNode
}
```

### Supporting helpers

- `valid()` - did this path resolve?
- `isMap()` / `isList()` - branch on container kind
- `is<T>()` - leaf type check
- `contains(key)` - does this map have this key?

```cpp
if (reader.contains("Audio")) { ... }
if (reader["Maps"]["flat"].contains("volume")) { ... }
if (reader["Lists"]["ids"].isList()) { ... }
```

---

## Logging

Descent is silent. The log fires **once, at the leaf**, when `as<T>(fallback)`
falls back - never at intermediate steps. The message names the full accumulated
path so it's actionable:

```
[DrON] couldn't resolve "Transform.rotation" - using fallback
```

A typo'd path produces one line, not step-by-step spam.

---

## Lifetime - read this

A `DronValue` is a **non-owning view** into its `DronReader`'s tree - it holds a
raw pointer into the reader's data. The rules:

- Do not let a `DronValue` outlive the `DronReader` it came from.
- Do not keep a `DronValue` across a `reader.load(...)` call - reloading
  replaces the tree and any outstanding views dangle.
- Do not copy or move the `DronReader` while holding `DronValue`s into it.

In practice: read the primitives you need out into your own variables, or keep
the reader alive for as long as you're reading. For the engine's use - load a
file, build components from it, done - this is a non-issue.

---

## Malformed input

DrON loads what it can and keeps going - one bad line never aborts the load.

- A number that overflows `int64_t` (e.g. `99999999999999999999`) is kept as the
  raw **string** rather than crashing, so the bad value stays visible.
- Garbage numerics (`--5`, a lone `-` or `.`) are stored as strings and logged.
- A malformed key (no `=`, starts with a digit) is logged and skipped.
- Comments inside containers are preserved as tokens but skipped by the parser.

None of these throw. The tokenizer prints a note to the console and the load
continues.

---

## Files

| File | Job |
|------|-----|
| `DronToken.hpp`     | token types and the `Token` struct |
| `DronTokenizer.*`   | text to tokens |
| `DronNode.hpp`      | the tree - `DronNode` (variant), `DronMap`, `DronList`, `DronDocument` |
| `DronParser.*`      | tokens to tree |
| `DronValue.*`       | safe navigation and typed reads (the facade) |
| `DronReader.*`      | top-level entry: load, deliver, iterate, print |

`DronNode` is a `std::variant<int64_t, double, bool, std::string, DronList,
DronMap>`. `DronDocument` is a `DronMap` (`std::map<std::string, DronNode>`), so
sections come back in alphabetical order, not file order - access is by name, so
this doesn't matter for reads.

---

## Inspecting loaded data

`DronReader::print()` dumps the whole document in readable form.
`DronReader::toString()` returns that same dump as a string for logging or
piping. Both are permanent - visual inspection of loaded data is useful for the
life of the tool, not just during development.

---

## Tests

`DronTest.cpp` runs an assertion suite against `torture_test.dron`, a fixture
built to stress every type at its edges, malformed input, structural characters
inside strings, deep nesting, empty containers, and every failure mode. The
checks double as usage examples - each one is a real call against the API with a
known expected result. Call `DronTests::runAll()` to run them.

---

## Scope

DrON 1.0 is frozen at exactly the four capabilities above plus their supporting
helpers. It is a **tool, not a product**. Deliberately absent, and not coming
back without a real consumer need driving a separate DrON 2.0 decision:

- no writer, save, or serialization
- no round-trip or layout-preserving edits
- no enum type or enum lookup
- no second value accessor or flat `(section, key, default)` helpers

If a future consumer genuinely can't get what it needs from this surface, the fix
is to code around it in the consumer and record the gap as a DrON 2.0 candidate -
not to edit 1.0.
