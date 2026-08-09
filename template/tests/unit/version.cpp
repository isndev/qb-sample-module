/**
 * @file tests/unit/version.cpp
 * @brief Unit tier — no engine, no sockets, no sleeping.
 *
 * Reached through the module's PUBLIC include prefix, which is what a consumer writes. A suite
 * that includes its own headers by relative path is not testing the surface it ships.
 */

#include <cstdint>
#include <string>
#include <gtest/gtest.h>
#include <qbm/@QB_NAME@/@QB_NAME@.h>

TEST(@QB_NAME_UPPER@Version, IsCompiledInFromCMake) {
    // Guards the CMake plumbing rather than the string: QBM_@QB_NAME_UPPER@_VERSION comes from
    // project(VERSION), and a broken DEFINES line would surface here as an empty value instead
    // of as a puzzling report from a consumer much later.
    EXPECT_FALSE(qbm::@QB_NAME@::version().empty());
    EXPECT_NE(std::string::npos, std::string{qbm::@QB_NAME@::version()}.find('.'));
}

TEST(@QB_NAME_UPPER@Events, CarryTheirPayload) {
    const qbm::@QB_NAME@::Request request{"ping"};
    EXPECT_STREQ("ping", request.payload.c_str());

    const qbm::@QB_NAME@::Reply reply{request.payload, 7u};
    EXPECT_STREQ("ping", reply.payload.c_str());
    EXPECT_EQ(7u, reply.sequence);
}

TEST(@QB_NAME_UPPER@Events, TruncateRatherThanAllocate) {
    // qb::string<64> is fixed-capacity with no heap behind it, which is what makes an event
    // constructor safe inside a noexcept push(). Oversized input is truncated, not thrown on --
    // worth knowing before you size a field.
    const std::string oversized(200, 'x');
    const qbm::@QB_NAME@::Request request{oversized.c_str()};
    EXPECT_LE(request.payload.size(), 64u);
}
