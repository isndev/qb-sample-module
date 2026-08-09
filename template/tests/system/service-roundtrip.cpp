/**
 * @file tests/system/service-roundtrip.cpp
 * @brief System tier — a real qb::Main, real actors, nothing outside this process.
 *
 * This is the shape almost every qb test ends up having, so it is worth reading once:
 *
 *  - The fixtures live in a NAMED namespace, never an anonymous one. A fixture handed to a
 *    coroutine-spawning framework template ends up as a field of a frame class defined in a qb
 *    header, and a type with no linkage there is a `-Werror=subobject-linkage` failure on gcc.
 *    The compiler is not a reliable oracle for it: measured over a full build, five files
 *    carried the shape and gcc-14 diagnosed one. So the convention is applied everywhere rather
 *    than where a compiler happened to complain.
 *  - Every in-actor expectation is ALSO mirrored to a process-global atom that the body asserts
 *    after join(). An EXPECT_* that runs on a worker thread and never executes cannot fail;
 *    without the mirror, an actor that silently never ran would look like a pass.
 *  - Nothing sleeps and nothing polls a clock. The actors self-terminate and the engine drains;
 *    ctest's TIMEOUT is the only backstop. A test that waits a fixed number of milliseconds is a
 *    test that is flaky on a loaded machine.
 */

#include <atomic>
#include <cstdint>
#include <gtest/gtest.h>
#include <qb/actor.h>
#include <qb/main.h>
#include <qbm/@QB_NAME@/@QB_NAME@.h>

namespace service_roundtrip_test {

std::atomic<int>  g_replies{0};
std::atomic<bool> g_service_was_reachable{false};
std::atomic<bool> g_sequence_was_monotonic{true};

constexpr int kRounds = 5;

/// Talks to the module's service without ever being told its ActorId — `getService<T>()` finds
/// the singleton on this core, which is the whole point of a ServiceActor.
class Client : public qb::Actor {
public:
    qb::io::async::task<bool>
    onInit() override {
        registerEvent<qbm::@QB_NAME@::Reply>(*this);

        auto *service = getService<qbm::@QB_NAME@::Service>();
        if (service == nullptr) {
            // co_return false fails this actor cleanly; the engine reports it through
            // Main::hasError(), which the body checks.
            co_return false;
        }
        g_service_was_reachable.store(true);

        for (int i = 0; i < kRounds; ++i) {
            push<qbm::@QB_NAME@::Request>(service->id(), "ping");
        }
        co_return true;
    }

    void
    on(const qbm::@QB_NAME@::Reply &event) {
        // The service numbers its replies; delivery to one actor is ordered, so the sequence
        // must arrive strictly increasing. This is the assertion that would catch a reordering
        // regression in the engine, which is why it is here and not just a count.
        if (event.sequence != static_cast<std::uint32_t>(_seen + 1)) {
            g_sequence_was_monotonic.store(false);
        }
        ++_seen;

        if (g_replies.fetch_add(1) + 1 == kRounds) {
            kill();
            qb::Main::stop();
        }
    }

private:
    int _seen{0};
};

} // namespace service_roundtrip_test

TEST(ServiceRoundtrip, AnswersEveryRequestInOrder) {
    using namespace service_roundtrip_test;

    g_replies.store(0);
    g_service_was_reachable.store(false);
    g_sequence_was_monotonic.store(true);

    qb::Main engine;

    // The service and its client on the same core. A ServiceActor is a singleton PER CORE, so a
    // client on core 1 would need its own instance there — getService<>() never crosses cores.
    engine.addActor<qbm::@QB_NAME@::Service>(0);
    engine.addActor<Client>(0);

    engine.start();
    engine.join();

    EXPECT_FALSE(engine.hasError());
    EXPECT_TRUE(g_service_was_reachable.load());
    EXPECT_EQ(kRounds, g_replies.load());
    EXPECT_TRUE(g_sequence_was_monotonic.load());
}
