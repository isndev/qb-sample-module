/**
 * @file src/qbm/@QB_NAME@/events.h
 * @brief The events qbm-@QB_NAME@ exchanges.
 *
 * Events are how actors talk. Two rules govern what you may put in one, and both are about
 * `push()` being `noexcept`:
 *
 *  - KEEP THEM SMALL AND CHEAP TO CONSTRUCT. A throwing constructor — or an allocation that
 *    fails — inside `push()` calls std::terminate(). `qb::string<N>` exists for exactly this:
 *    a fixed-capacity string with no heap behind it.
 *  - AN EVENT CROSSING CORES IS RELOCATED BY memcpy, TWICE. Its payload must therefore hold no
 *    pointer into itself. A std::string member is fine (it points to the heap); a member
 *    pointing at another member of the same event is not.
 */

#ifndef QBM_@QB_NAME_UPPER@_EVENTS_H
#define QBM_@QB_NAME_UPPER@_EVENTS_H

#include <cstdint>
#include <qb/event.h>
#include <qb/string.h>

namespace qbm::@QB_NAME@ {

/// Ask the service to echo something back. The reply goes to `getSource()`.
struct Request : qb::Event {
    qb::string<64> payload;

    Request() = default;
    explicit Request(const char *text) noexcept
        : payload(text) {}
};

/// What the service sends back.
struct Reply : qb::Event {
    qb::string<64> payload;
    std::uint32_t  sequence{0};

    Reply() = default;
    Reply(const qb::string<64> &text, std::uint32_t seq) noexcept
        : payload(text)
        , sequence(seq) {}
};

} // namespace qbm::@QB_NAME@

#endif // QBM_@QB_NAME_UPPER@_EVENTS_H
