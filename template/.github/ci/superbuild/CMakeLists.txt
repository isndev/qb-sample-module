#
# qbm-@QB_NAME@ — minimal superproject root.
#
# -----------------------------------------------------------------------------
# WHY IT EXISTS
# -------------
# A qbm module cannot be configured on its own, and its CMakeLists.txt says so with a FATAL_ERROR
# rather than dying on whichever qb_* command it reaches first: it calls qb_status_message(),
# qb_register_module() and qb_add_test(), which live in qb/cmake/qbFunctions.cmake and are
# DEVELOPMENT-TIME helpers an installed qb does not ship (lib/cmake/qb/ carries only qbConfig,
# qbConfigVersion, qbTargets and the Find modules). Pointing CMAKE_PREFIX_PATH at an installed qb
# therefore does not help.
#
# What the module needs is a root that add_subdirectory()es a qb SOURCE tree first and the module
# second. This is that root. It is the same shape qbm-http, qbm-pgsql and qbm-redis each carry at
# this exact path — keep it here, so that when this module grows up into its own repository with
# its own CI, nothing has to move.
#
# HOW TO USE IT, from the module root:
#     cmake -S .github/ci/superbuild -B build -DCMAKE_BUILD_TYPE=Release
#     cmake --build build --parallel
#     ctest --test-dir build --output-on-failure -L module:qbm-@QB_NAME@
#
# THE ONE DIFFERENCE FROM THE THREE PUBLISHED MODULES' COPIES, and it is deliberate: theirs
# REQUIRE both trees to be passed in (-DQBM_CI_QB_DIR / -DQBM_CI_MODULE_DIR), because their CI
# always passes them and a wrong default would be worse than an error. A freshly scaffolded module
# has no CI passing anything yet, so this copy defaults the module path to its own location and
# FETCHES qb when it is not given one — which is what makes the command above work on a machine
# that has never seen qb. Pass -DQBM_CI_QB_DIR=/path/to/qb to use a checkout you already have; the
# fetch is then skipped entirely.
#
# WHAT IT DELIBERATELY IS NOT
# ---------------------------
# Not a packaging entry point, and it cannot become one by accident: QB_INSTALL and QBM_INSTALL
# are forced OFF below. The only way to produce an INSTALLABLE qbm-<mod> is the qb-dev
# superproject's `package` preset.
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.24)

# NONE first, then enable_language() after the argument checks below: a bad path should report
# itself immediately, not after a compiler-identification probe.
project(qbm-@QB_NAME@-superbuild NONE)

# The module is two directories up from this file. Resolved from CMAKE_CURRENT_LIST_DIR rather
# than assumed relative to the invocation directory, so `cmake -S .github/ci/superbuild` works
# from anywhere.
if(NOT QBM_CI_MODULE_DIR)
    get_filename_component(QBM_CI_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
endif()
get_filename_component(QBM_CI_MODULE_DIR "${QBM_CI_MODULE_DIR}" ABSOLUTE)
if(NOT EXISTS "${QBM_CI_MODULE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "QBM_CI_MODULE_DIR=${QBM_CI_MODULE_DIR} contains no CMakeLists.txt. This root expects to "
        "sit at <module>/.github/ci/superbuild/; pass -DQBM_CI_MODULE_DIR=<path> if it does not.")
endif()

enable_language(C CXX)

# Position-independent code for the whole tree. qbConfig also sets this for the qb subtree, but a
# module's bundled static dependencies are loaded from THIS root scope, so it has to be set here
# too — same reason, same line, as in the qb-dev superproject root.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Tests ON is the entire point of this root, so it is FORCEd: a stale cache or a generator that
# ignores a plain default must not be able to configure a "test" build with no tests in it.
set(QB_BUILD_TESTS ON CACHE BOOL "Build tests" FORCE)
set(QB_BUILD_BENCHMARKS OFF CACHE BOOL "Build benchmarks" FORCE)
set(QB_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)

# See the header: this root is a test super-build, never an install target. Forcing both OFF also
# avoids qb's install(EXPORT qbTargets) hard generate error when a bundled dependency (zlib as
# `zlibstatic`) is built from source and is in no export set.
set(QB_INSTALL OFF CACHE BOOL "Install qb framework" FORCE)
set(QBM_INSTALL OFF CACHE BOOL "Install qbm modules" FORCE)

enable_testing()

# qb: a checkout if one was given, otherwise fetched at the ref the scaffolder resolved.
if(QBM_CI_QB_DIR)
    get_filename_component(QBM_CI_QB_DIR "${QBM_CI_QB_DIR}" ABSOLUTE)
    if(NOT EXISTS "${QBM_CI_QB_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "QBM_CI_QB_DIR=${QBM_CI_QB_DIR} contains no CMakeLists.txt.")
    endif()
    message(STATUS "[qbm-@QB_NAME@] qb from ${QBM_CI_QB_DIR}")
    add_subdirectory("${QBM_CI_QB_DIR}" "${CMAKE_BINARY_DIR}/qb")
else()
    # PINNING: QB_GIT_REF is what the scaffolder resolved on the day this module was created. If
    # it names a branch you are following a moving line — set it to a released tag once you want
    # reproducible builds.
    set(QB_GIT_REF "@QB_REF@" CACHE STRING "isndev/qb git ref to build this module against")
    message(STATUS "[qbm-@QB_NAME@] fetching qb @ ${QB_GIT_REF} (pass -DQBM_CI_QB_DIR to use a local checkout)")
    include(FetchContent)
    FetchContent_Declare(qb
            GIT_REPOSITORY https://github.com/isndev/qb.git
            GIT_TAG        ${QB_GIT_REF}
            GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(qb)
endif()

# qb's helpers are the whole reason this root exists. If a checkout ever stops providing them, say
# that HERE — the alternative is the module's own FATAL_ERROR firing one line later and reading as
# "you configured the module directly", which would be wrong and would send the next person
# looking in the wrong file.
foreach(_qbm_ci_cmd IN ITEMS qb_status_message qb_register_module qb_register_module_test)
    if(NOT COMMAND ${_qbm_ci_cmd})
        message(FATAL_ERROR
            "the qb tree in use defines no ${_qbm_ci_cmd}(). This root add_subdirectory()s a qb "
            "SOURCE tree precisely to get qb/cmake/qbFunctions.cmake; an installed qb, or a qb "
            "too old to carry that helper, cannot be used here.")
    endif()
endforeach()

# ANTI-VACUITY, at configure time. The module's CMakeLists.txt returns EARLY and silently when
# QB_FOUND is false, and its tests/ subtree is behind if(QB_BUILD_TESTS) — either would leave this
# root configuring cleanly, building nothing, and letting `ctest -L module:qbm-@QB_NAME@` report
# `No tests were found!!!` with exit code 0. So count the registered test targets across the
# module's add_subdirectory() and require growth.
get_property(_qbm_ci_tests_before GLOBAL PROPERTY QB_TEST_TARGETS)
list(LENGTH _qbm_ci_tests_before _qbm_ci_before)

add_subdirectory("${QBM_CI_MODULE_DIR}" "${CMAKE_BINARY_DIR}/qbm/@QB_NAME@")

get_property(_qbm_ci_tests_after GLOBAL PROPERTY QB_TEST_TARGETS)
list(LENGTH _qbm_ci_tests_after _qbm_ci_after)
math(EXPR _qbm_ci_added "${_qbm_ci_after} - ${_qbm_ci_before}")

get_property(_qbm_ci_modules GLOBAL PROPERTY QB_MODULE_LIBRARIES)
if(NOT _qbm_ci_modules)
    message(FATAL_ERROR
        "${QBM_CI_MODULE_DIR} registered no qbm module. Its CMakeLists.txt returns early when "
        "QB_FOUND is false; this root would then build nothing and every count taken afterwards "
        "would be vacuous.")
endif()
if(_qbm_ci_added LESS 1)
    message(FATAL_ERROR
        "${_qbm_ci_modules} registered ${_qbm_ci_added} test targets. A test super-build with no "
        "tests in it is the one thing this root must not produce.")
endif()

# AN AGGREGATE TARGET FOR THIS MODULE'S TESTS, and the reason it is needed.
#
# QB_BUILD_TESTS is global: switching it on to get THIS module's two tests also registers qb's
# own ~174 into the same build. So a plain `cmake --build build` compiles the entire framework
# test suite, which this root does not run and qb's own CI already builds — minutes of work for
# nothing. The three published modules deal with it in their workflows, by intersecting the ctest
# names with the real target list; a scaffolded module has no workflow doing that yet, and
# "build my module" should not require a shell pipeline.
#
# The list is the DIFFERENCE across the module's add_subdirectory(), so it is whatever the module
# actually registered — nothing here names a test, and adding one to tests/CMakeLists.txt needs
# no edit in this file.
set(_qbm_ci_new_tests ${_qbm_ci_tests_after})
if(_qbm_ci_tests_before)
    list(REMOVE_ITEM _qbm_ci_new_tests ${_qbm_ci_tests_before})
endif()
add_custom_target(qbm-@QB_NAME@-tests DEPENDS ${_qbm_ci_new_tests})

qb_status_message("superbuild: module ${_qbm_ci_modules}, ${_qbm_ci_added} test targets registered")
qb_status_message("superbuild: build them with --target qbm-@QB_NAME@-tests, run them with -L module:qbm-@QB_NAME@")
