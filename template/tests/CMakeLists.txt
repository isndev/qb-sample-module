#
# qbm-@QB_NAME@ tiered test suite.
#
# qb_register_module_test() is the same helper qbm-http, qbm-pgsql and qbm-redis use. It produces
# a target and ctest entry called `qbm-<module>-test-<tier>-<name>` and attaches the labels
# `module:qbm-<module>` and `tier:<tier>`, so this module's tests select the same way theirs do:
#
#   ctest --test-dir build -L module:qbm-@QB_NAME@
#   ctest --test-dir build -L tier:unit
#
# THE TIERS are a contract about what a test may touch:
#   unit/        pure logic. No engine, no sockets, no sleeping. Milliseconds.
#   system/      a real qb::Main with real actors, nothing outside this process.
#   integration/ needs something external. Add `REQUIRES live` so it SKIPS rather than fails
#                where the service is absent — and then never quote a bare `100% tests passed`,
#                because ctest counts a skip as a pass. Report executed and skipped separately.
#   benchmark/   qb_register_module_benchmark(); google-benchmark, never run by ctest.
#
# INCLUDES adds this directory so a test can reach a shared fixture header by "../shared/x.h".
# The module's own public headers are reached by their shipped prefix — <qbm/@QB_NAME@/...> —
# which is what a consumer writes, so the suite exercises the real spelling.

function(@QB_NAME@_test TIER NAME RELPATH)
    qb_register_module_test(
            MODULE_NAME @QB_NAME@ TIER ${TIER} TEST_NAME ${NAME}
            SOURCES ${RELPATH}
            INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

# UNIT — pure logic, no engine
@QB_NAME@_test(unit version unit/version.cpp)

# SYSTEM — a real engine, in-process only
@QB_NAME@_test(system service-roundtrip system/service-roundtrip.cpp)
