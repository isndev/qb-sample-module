# qb-sample-module

The module template behind `qb-new-module.sh`. It is **not** a module you clone and use — it is
the payload a scaffolder renders. Start a module like this:

```bash
curl -fsSL https://raw.githubusercontent.com/isndev/qb/main/script/qb-new-module.sh | bash /dev/stdin mymodule
cd mymodule
cmake -S .github/ci/superbuild -B build
cmake --build build --parallel --target qbm-mymodule-tests
ctest --test-dir build --output-on-failure -L module:qbm-mymodule
```

That produces a fresh git repository with one commit, named `mymodule` throughout — target
`qbm-mymodule`, alias `qbm::mymodule`, namespace `qbm::mymodule`, headers at
`src/qbm/mymodule/` — depending on a qb the scaffolder chose. No submodules, no history that is
not yours, and nothing to rename by hand.

## Layout

```
template/     the payload. Everything here becomes the user's module.
README.md     this file — about the template, never copied
LICENSE
.github/workflows/template.yml
```

`template/` is a directory rather than "the repository minus an exclusion list" on purpose: an
exclusion list is a thing you forget to update, and the failure is silent — the user's module
quietly inherits the template's own CI or README.

## Placeholders

The scaffolder owns the substitution vocabulary; this template may only use these tokens, in file
contents **and** in path names (`src/qbm/@QB_NAME@/@QB_NAME@.h` is a real path in this repository).
An unknown `@QB_...@` token is a hard error at scaffold time, so adding one here without teaching
the scaffolder about it fails loudly rather than shipping a half-rendered tree.

| Token               | Becomes                                          | Example (`mymodule`) |
|---------------------|--------------------------------------------------|----------------------|
| `@QB_NAME@`         | the name the user passed, verbatim                | `mymodule`           |
| `@QB_NAME_LOWER@`   | lowercased                                        | `mymodule`           |
| `@QB_NAME_UPPER@`   | uppercased — include guards, compile definitions  | `MYMODULE`           |
| `@QB_REF@`          | the isndev/qb git ref the module builds against   | `develop`            |
| `@QB_VERSION@`      | the qb version the scaffolder shipped with        | `3.2.1`              |
| `@QB_TEMPLATE_REF@` | the ref of this template that was used            | `develop`            |

A module name is validated as `[a-z][a-z0-9_]*` — stricter than a project's, because it becomes a
CMake target, a C++ namespace and a directory that `qb_register_module()` requires by exact name.

## What the rendered module contains

A working module, not a stub: a `qbm::<name>::Service` (a `qb::ServiceActor`) answering `Request`
events with `Reply` events, a compiled `.cpp` so it is a real library from the start, a tiered
`tests/` suite registered through `qb_register_module_test()`, a CI workflow, and the superbuild
root that makes it buildable.

The superbuild root is the interesting part. A qbm module **cannot be configured standalone** — it
calls `qb_register_module()` and `qb_add_test()`, development-time helpers an installed qb does not
ship — so it needs a root that adds a qb *source* tree first. `template/.github/ci/superbuild/CMakeLists.txt`
is that root, in the same shape and at the same path as the one `qbm-http`, `qbm-pgsql` and
`qbm-redis` each carry, with one deliberate difference: theirs require both trees to be passed in
(`-DQBM_CI_QB_DIR` / `-DQBM_CI_MODULE_DIR`) because their CI always passes them, while this one
defaults its own path and fetches qb, so `cmake -S .github/ci/superbuild -B build` works with no
arguments on a machine that has never seen qb.

## How this template stays current

It did not, for seven years — it was last touched in 2019 and had stopped configuring entirely —
and the reason was structural rather than neglect: nothing bound a template version to a qb
version, and nothing ever built the template. Both are fixed, and both fixes matter.

**Version binding.** `qb-new-module.sh` resolves the qb ref from `QB_FRAMEWORK_VERSION` in the qb
it ships with, and writes it into the generated tree. The one-liner's URL therefore selects the
pairing — `.../qb/main/script/...` is the released line, `.../qb/v3.2.1/script/...` is that
release. Nothing is stored in this repository that can go stale. The scaffolder reports which ref
it used and why, including when it had to fall back.

Branches here follow qb's: **`develop`** is the next version, **`main`**/**`master`** the released
line. The scaffolder prefers a `v<version>` tag, falls back to the development line while that
version is unreleased, and says so either way.

**A lane that can fail.** `.github/workflows/template.yml` runs qb's actual `qb-new-module.sh`
against this checkout, then configures, builds and tests the result on Linux and macOS, weekly and
on every push. It asserts a non-zero translation-unit count, that no placeholder survived in a file
or a path, that the generated repository has exactly one commit and no remote, that the module
registered under the user's name rather than the template's, and that the suite executed rather
than skipped. Drift is now a red build instead of a discovery.

## Changing the template

Render it locally with the real scaffolder — no push required:

```bash
git clone https://github.com/isndev/qb
cd /tmp && QB_TEMPLATE_DIR=/path/to/qb-sample-module \
  bash /path/to/qb/script/qb-new-module.sh scratch
cd scratch && cmake -S .github/ci/superbuild -B build \
  && cmake --build build --parallel --target qbm-scratch-tests \
  && ctest --test-dir build -L module:qbm-scratch
```

`QB_TEMPLATE_DIR` is what the CI lane uses too, which is the point: the thing you run locally is
the thing that gates the pull request.

Keep `template/` buildable *after rendering*, not before — it is not expected to configure with the
placeholders in place.

## License

Apache-2.0, matching qb. See `LICENSE`.
