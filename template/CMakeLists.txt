#
# qbm-@QB_NAME@ — a module for the qb Actor Framework.
#
# Scaffolded by qb-new-module.sh from isndev/qb-sample-module (@QB_TEMPLATE_REF@),
# against qb @QB_VERSION@.
#

cmake_minimum_required(VERSION 3.24)

project(qbm-@QB_NAME@ VERSION 0.1.0)

# THIS MODULE IS NOT STANDALONE-CONFIGURABLE, and it says so here rather than dying on whichever
# qb_* command it happens to reach first.
#
# qbm modules are built from a root that include()s qb's cmake/ helpers before
# add_subdirectory()ing them: this file calls qb_status_message() and qb_register_module(), and an
# INSTALLED qb ships neither — they are development-time helpers, not package content
# (lib/cmake/qb/ carries only qbConfig, qbConfigVersion, qbTargets and the Find modules). Pointing
# CMAKE_PREFIX_PATH at an installed qb therefore does not help, which is exactly the mistake this
# guard exists to name: without it the first symptom is
#     CMake Error at CMakeLists.txt:NN (qb_status_message):
#       Unknown CMake command "qb_status_message".
# which reads like a missing include rather than "wrong entry point".
#
# To build and test this module on its own, use the superbuild root that ships with it — it
# fetches qb and add_subdirectory()s it before this file:
#     cmake -S .github/ci/superbuild -B build
#     cmake --build build --parallel
#     ctest --test-dir build --output-on-failure
#
# To use it from an application, drop it in that project's qbm/ directory; the project template's
# qb_load_modules() call picks it up and exposes it as qbm::@QB_NAME@.
if(NOT COMMAND qb_status_message)
    message(FATAL_ERROR
        "[qbm-@QB_NAME@] this module cannot be configured on its own.\n"
        "     It needs a root that loads qb's CMake helpers (qb_status_message / "
        "qb_register_module / qb_add_test) first; an installed qb does not ship them, so "
        "CMAKE_PREFIX_PATH will not fix this.\n"
        "     Use the superbuild root that ships with this module:\n"
        "         cmake -S .github/ci/superbuild -B build")
endif()

# The superbuild root and a full superproject both add qb first, so this is normally true. It is
# checked because a root that adds this module WITHOUT qb would otherwise fail deep inside
# qb_register_module() instead of here.
if(NOT QB_FOUND)
    qb_status_message("qb framework not found, skipping the @QB_NAME@ module")
    return()
endif()

# -----------------------------------------------------------------------------
# The module
# -----------------------------------------------------------------------------
# THE LAYOUT RULE: public headers live at src/qbm/<name>/, and src/ IS the include root. That is
# what makes <qbm/@QB_NAME@/@QB_NAME@.h> the same string in this source tree and in an installed
# prefix, and it is enforced — qb_register_module() fails with a hard error if the directory is
# not there. Before 3.0 the include root was the module's PARENT directory, which made the
# consume spelling <@QB_NAME@/...> and claimed that name in every consumer's include namespace.
#
# No INCLUDES argument on purpose. qb_register_module() already exposes src/ as the module's
# include root; passing ${CMAKE_CURRENT_SOURCE_DIR} in addition puts a raw absolute source path,
# with no $<BUILD_INTERFACE:> guard, into INTERFACE_INCLUDE_DIRECTORIES — which turns
# install(TARGETS ... EXPORT) into a hard generate error.
#
# Add a HEADER_ONLY module by passing that keyword and dropping SOURCES. This one compiles a .cpp
# so that it is a real library from the start: it is easier to delete a source file than to
# discover, three months in, that everything you wrote had to be a template.
qb_register_module(
        NAME @QB_NAME@
        VERSION ${PROJECT_VERSION}
        DESCRIPTION "@QB_NAME@ module for the qb Actor Framework"
        DEPENDS qb-core
        SOURCES
        src/qbm/@QB_NAME@/@QB_NAME@.cpp
        DEFINES
        QBM_@QB_NAME_UPPER@_VERSION="${PROJECT_VERSION}"
)

qb_status_message("qbm-@QB_NAME@ ${PROJECT_VERSION} registered (consume it as qbm::@QB_NAME@)")

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
# QB_BUILD_TESTS is the superproject's switch, not this module's: a module registers its tests
# when the enclosing build wants tests, and stays silent otherwise.
if(QB_BUILD_TESTS)
    add_subdirectory(tests)
endif()
