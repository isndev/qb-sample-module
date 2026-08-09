/**
 * @file src/qbm/@QB_NAME@/@QB_NAME@.h
 * @brief Umbrella header for qbm-@QB_NAME@ — the one include a consumer needs.
 *
 *     #include <qbm/@QB_NAME@/@QB_NAME@.h>
 *
 * That spelling is the same in this source tree and in an installed prefix, because src/ is the
 * module's include root and the public headers live at src/qbm/@QB_NAME@/. qbm-http, qbm-pgsql
 * and qbm-redis all publish themselves this way.
 *
 * THE HEADER RULES THIS MODULE FOLLOWS, and that the qb tree enforces with scripts:
 *  - `.h` is the only header extension. No `.tpp`, no `.inl`; 3.0 retired all nine that existed.
 *    A template body goes at the tail of the header that declares it.
 *  - Every #include sits at the top of the file, OUTSIDE every namespace. An include written
 *    inside `namespace qbm::@QB_NAME@ { }` declares `qbm::@QB_NAME@::std`, and stays harmless
 *    only for as long as some earlier header happens to pull the same file in first.
 *  - Nothing NON-template goes in a header that consumers include. One inline-less definition
 *    reaching two translation units is a duplicate symbol at the CONSUMER's link, while this
 *    module's own build stays green. Definitions live in the .cpp.
 */

#ifndef QBM_@QB_NAME_UPPER@_H
#define QBM_@QB_NAME_UPPER@_H

#include <string_view>

#include "./events.h"
#include "./service.h"

namespace qbm::@QB_NAME@ {

/// The module version, from project(VERSION) in this module's CMakeLists.txt via
/// QBM_@QB_NAME_UPPER@_VERSION. Declared here, DEFINED in @QB_NAME@.cpp — see the header rules.
[[nodiscard]] std::string_view version() noexcept;

} // namespace qbm::@QB_NAME@

#endif // QBM_@QB_NAME_UPPER@_H
