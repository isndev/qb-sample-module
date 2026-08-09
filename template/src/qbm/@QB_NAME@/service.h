/**
 * @file src/qbm/@QB_NAME@/service.h
 * @brief The service actor qbm-@QB_NAME@ provides.
 *
 * A `qb::ServiceActor<Tag>` is a singleton per VirtualCore: at most one instance of a given Tag
 * exists on a core, and any actor on that core can reach it with `getService<Service>()` without
 * being told an ActorId. That is what makes it the natural shape for a module — a module ships
 * capability, and consumers should not have to plumb identifiers to find it.
 *
 * The Tag must be a COMPLETE type. `ServiceActor<struct MyTag>` only declares one, and the
 * resulting service is a different type from everybody else's idea of it; define the struct.
 */

#ifndef QBM_@QB_NAME_UPPER@_SERVICE_H
#define QBM_@QB_NAME_UPPER@_SERVICE_H

#include <cstdint>
#include <qb/actor.h>
#include <qb/io.h>

#include "./events.h"

namespace qbm::@QB_NAME@ {

/// Tag identifying this service. Complete, deliberately — see the header comment.
struct Tag {};

class Service : public qb::ServiceActor<Tag> {
public:
    Service() = default;

    /**
     * `onInit()` is a coroutine returning `qb::io::async::task<bool>`, not a plain `bool` — that
     * changed in 2.6 and the old signature does not compile. `co_return false` fails this
     * actor's construction cleanly and the engine reports it through `Main::hasError()`;
     * THROWING from here is a different, louder outcome. Register every event you handle before
     * returning: an event with no registration is silently not delivered.
     */
    qb::io::async::task<bool>
    onInit() override {
        registerEvent<Request>(*this);
        co_return true;
    }

    void
    on(const Request &event) {
        // `push()` is ordered and takes any event; `send()` is unordered and only accepts
        // trivially-destructible ones. Prefer push until you have measured a reason not to.
        push<Reply>(event.getSource(), event.payload, ++_sequence);
    }

    /// How many requests this instance has answered. Reading it from ANOTHER actor would be a
    /// data race; ask by event instead. It is exposed for the service's own tests.
    [[nodiscard]] std::uint32_t
    served() const noexcept {
        return _sequence;
    }

private:
    std::uint32_t _sequence{0};
};

} // namespace qbm::@QB_NAME@

#endif // QBM_@QB_NAME_UPPER@_SERVICE_H
