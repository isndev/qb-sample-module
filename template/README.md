# qbm-@QB_NAME@

A module for the [qb](https://github.com/isndev/qb) Actor Framework, scaffolded from
[`qb-sample-module`](https://github.com/isndev/qb-sample-module) against qb **@QB_VERSION@**.

It is a working module with a tiered test suite and a CI workflow — not a stub. It provides a
`qbm::@QB_NAME@::Service` that answers `Request` events with `Reply` events; replace that with
whatever your module actually does. The structure is the point.

## Build and test

```bash
cmake -S .github/ci/superbuild -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --target qbm-@QB_NAME@-tests
ctest --test-dir build --output-on-failure -L module:qbm-@QB_NAME@
```

Nothing else is needed: qb is fetched at the ref the scaffolder recorded. To build against a qb
checkout you already have:

```bash
cmake -S .github/ci/superbuild -B build -DQBM_CI_QB_DIR=/path/to/qb
```

**Name the target and the label.** `QB_BUILD_TESTS` is a global switch, so the same build also
registers qb's own ~174 tests; an unqualified `cmake --build build` compiles all of them and an
unqualified `ctest` tries to run them. Both commands above are scoped to this module.

## Why `-S .github/ci/superbuild` and not `-S .`

A qbm module **cannot be configured on its own**. Its `CMakeLists.txt` calls
`qb_register_module()` and `qb_add_test()`, which live in `qb/cmake/qbFunctions.cmake` and are
development-time helpers that an *installed* qb does not ship — so `CMAKE_PREFIX_PATH` pointed at
an installed qb does not help either. What works is a root that `add_subdirectory()`s a qb
**source** tree first and the module second, and that is what `.github/ci/superbuild/CMakeLists.txt`
is. `qbm-http`, `qbm-pgsql` and `qbm-redis` each carry the same file at the same path; keeping it
here means nothing has to move if this module ever becomes its own published repository.

Configuring the module directly is not a mysterious failure — it is a `FATAL_ERROR` that says all
of the above.

## Layout

```
CMakeLists.txt                      qb_register_module() — the module's identity
src/qbm/@QB_NAME@/
  @QB_NAME@.h                       umbrella header: the one include a consumer needs
  @QB_NAME@.cpp                     non-template definitions
  events.h                          the events this module exchanges
  service.h                         the ServiceActor it provides
tests/
  unit/                             pure logic, no engine
  system/                           a real qb::Main, real actors, nothing outside the process
.github/ci/superbuild/CMakeLists.txt
.github/workflows/ci.yml
```

`src/` **is** the include root, which is why the public headers sit at `src/qbm/@QB_NAME@/`:
`<qbm/@QB_NAME@/@QB_NAME@.h>` is then the same string in this source tree and in an installed
prefix. `qb_register_module()` enforces it with a hard error — a differently shaped module would
install into the wrong place and fail only at a downstream consumer's first `#include`.

## Using it from a project

Drop this directory into a project's `qbm/` folder. A project scaffolded with
`qb-new-project.sh` calls `qb_load_modules()` on that folder, so it is picked up automatically:

```cmake
target_link_libraries(myapp PRIVATE qbm::@QB_NAME@)
```

```cpp
#include <qbm/@QB_NAME@/@QB_NAME@.h>
```

## The rules this module already follows

These are enforced by scripts in the qb tree, and a module that breaks them fails review rather
than failing mysteriously later.

- **`.h` is the only header extension.** No `.tpp`, no `.inl`; 3.0 retired all nine that existed.
  A template body goes at the tail of the header that declares it.
- **Every `#include` sits at the top of the file, outside every namespace.** An include written
  inside `namespace qbm::@QB_NAME@ { }` declares `qbm::@QB_NAME@::std`, and stays harmless only
  while some earlier header happens to pull the same file in first.
- **Nothing non-template goes in a consumer-visible header.** One non-`inline` definition reaching
  two translation units is a duplicate symbol at the *consumer's* link, while this module's own
  build stays green. Definitions belong in the `.cpp`.
- **`onInit()` returns `qb::io::async::task<bool>`** and ends in `co_return`. The pre-2.6 `bool`
  signature does not compile.
- **A test fixture handed to a coroutine-spawning framework template lives in a NAMED namespace.**
  An anonymous one gives the coroutine frame a field of no-linkage type, which gcc reports as
  `-Werror=subobject-linkage` — and only sometimes, so the convention is applied everywhere.
- **Time is `qb::duration` / `qb::mono_time` / `qb::wall_time`.** `qb::Timestamp` and
  `<qb/system/timestamp.h>` were removed in 3.0.

## Test tiers

`tests/CMakeLists.txt` registers through `qb_register_module_test()`, the same helper the three
published modules use, producing `qbm-@QB_NAME@-test-<tier>-<name>` with `module:` and `tier:`
labels. The tiers are a contract about what a test may touch — unit (pure logic), system (a real
engine, in-process only), integration (needs something external), benchmark.

If you add an integration test, give it `REQUIRES live` so it **skips** rather than fails where
the service is absent — and then never quote a bare `100% tests passed`, because ctest counts a
skip as a pass. Report registered, executed and skipped separately.

## Pinning for production

The superbuild root's `QB_GIT_REF` is what the scaffolder resolved on the day you ran it. If it
names a **branch**, your build follows a moving line. Set it to a released tag once you want
reproducible builds.

## License

The template carries Apache-2.0, matching qb. This module is yours — replace this section and the
`LICENSE` file with whatever you actually ship under.
