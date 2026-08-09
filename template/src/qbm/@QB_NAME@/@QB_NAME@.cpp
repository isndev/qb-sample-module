/**
 * @file src/qbm/@QB_NAME@/@QB_NAME@.cpp
 * @brief Non-template definitions for qbm-@QB_NAME@.
 *
 * Small on purpose, and the right place for anything that is not a template: a definition that
 * lives in a header reaches every consumer translation unit, and one that is not `inline` is a
 * duplicate symbol at the consumer's link while this module's own build stays green.
 */

#include <string_view>

#include "./@QB_NAME@.h"

namespace qbm::@QB_NAME@ {

std::string_view
version() noexcept {
    // QBM_@QB_NAME_UPPER@_VERSION is compiled in from CMake. Never write the version out a
    // second time in C++: two copies are two chances to disagree, and that is how every stale
    // version string in this ecosystem started.
    return QBM_@QB_NAME_UPPER@_VERSION;
}

} // namespace qbm::@QB_NAME@
