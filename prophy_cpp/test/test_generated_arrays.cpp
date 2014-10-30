#include <vector>
#include <gtest/gtest.h>
#include "generated/Arrays.ppf.hpp"
#include "util.hpp"

using namespace testing;
using namespace prophy::generated;

TEST(generated_arrays, Builtin)
{
    std::vector<uint8_t> data(1024);

    Builtin x;
    x.x = 1;
    x.y = 2;
    size_t size = x.encode(data.data());

    EXPECT_EQ(8, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes("\x01\x00\x00\x00\x02\x00\x00\x00"), bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes("\x03\x00\x00\x00\x04\x00\x00\x00")));
    EXPECT_EQ(3, x.x);
    EXPECT_EQ(4, x.y);

    EXPECT_EQ(std::string(
            "x: 3\n"
            "y: 4\n"), x.print());
}

TEST(generated_arrays, BuiltinFixed)
{
    std::vector<uint8_t> data(1024);

    BuiltinFixed x;
    x.x[0] = 1;
    x.x[1] = 2;
    size_t size = x.encode(data.data());

    EXPECT_EQ(8, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes("\x01\x00\x00\x00\x02\x00\x00\x00"), bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes("\x03\x00\x00\x00\x04\x00\x00\x00")));
    EXPECT_EQ(3, x.x[0]);
    EXPECT_EQ(4, x.x[1]);

    EXPECT_EQ(std::string(
            "x: 3\n"
            "x: 4\n"), x.print());
}

TEST(generated_arrays, BuiltinDynamic)
{
    std::vector<uint8_t> data(1024);

    BuiltinDynamic x;
    x.x.push_back(1);
    x.x.push_back(2);
    size_t size = x.encode(data.data());

    EXPECT_EQ(12, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes("\x02\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00"), bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x03\x00\x00\x00\x05\x00\x00\x00\x03\x00\x00\x00\x01\x00\x00\x00")));
    EXPECT_EQ(3, x.x.size());
    EXPECT_EQ(5, x.x[0]);
    EXPECT_EQ(3, x.x[1]);
    EXPECT_EQ(1, x.x[2]);

    EXPECT_EQ(std::string(
            "x: 5\n"
            "x: 3\n"
            "x: 1\n"), x.print());
}

TEST(generated_arrays, BuiltinLimited)
{
    std::vector<uint8_t> data(1024);

    BuiltinLimited x;
    x.x.push_back(1);
    size_t size = x.encode(data.data());

    EXPECT_EQ(12, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes("\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"), bytes(data.data(), size));

    x.x.push_back(2);
    x.x.push_back(3);
    size = x.encode(data.data());

    EXPECT_EQ(12, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes("\x02\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00"), bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x01\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00")));
    EXPECT_EQ(1, x.x.size());
    EXPECT_EQ(3, x.x[0]);

    EXPECT_TRUE(x.decode(bytes(
            "\x02\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00")));
    EXPECT_EQ(2, x.x.size());
    EXPECT_EQ(1, x.x[0]);
    EXPECT_EQ(2, x.x[1]);

    EXPECT_EQ(std::string(
            "x: 1\n"
            "x: 2\n"), x.print());
}

TEST(generated_arrays, BuiltinGreedy)
{
    std::vector<uint8_t> data(1024);

    BuiltinGreedy x;
    x.x.push_back(1);
    x.x.push_back(2);
    size_t size = x.encode(data.data());

    EXPECT_EQ(8, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes("\x01\x00\x00\x00\x02\x00\x00\x00"), bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x03\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00")));
    EXPECT_EQ(3, x.x.size());
    EXPECT_EQ(3, x.x[0]);
    EXPECT_EQ(4, x.x[1]);
    EXPECT_EQ(5, x.x[2]);

    EXPECT_EQ(std::string(
            "x: 3\n"
            "x: 4\n"
            "x: 5\n"), x.print());
}

TEST(generated_arrays, Fixcomp)
{
    std::vector<uint8_t> data(1024);

    Fixcomp x;
    x.x.x = 1;
    x.x.y = 2;
    x.y.x = 3;
    x.y.y = 4;
    size_t size = x.encode(data.data());

    EXPECT_EQ(16, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes(
            "\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00"),
            bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x03\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00\x06\x00\x00\x00")));
    EXPECT_EQ(3, x.x.x);
    EXPECT_EQ(4, x.x.y);
    EXPECT_EQ(5, x.y.x);
    EXPECT_EQ(6, x.y.y);

    EXPECT_EQ(std::string(
            "x {\n"
            "  x: 3\n"
            "  y: 4\n"
            "}\n"
            "y {\n"
            "  x: 5\n"
            "  y: 6\n"
            "}\n"), x.print());
}

TEST(generated_arrays, FixcompFixed)
{
    std::vector<uint8_t> data(1024);

    FixcompFixed x;
    x.x[0].x = 1;
    x.x[0].y = 2;
    x.x[1].x = 3;
    x.x[1].y = 4;
    size_t size = x.encode(data.data());

    EXPECT_EQ(16, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes(
            "\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00"),
            bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x03\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00\x06\x00\x00\x00")));
    EXPECT_EQ(3, x.x[0].x);
    EXPECT_EQ(4, x.x[0].y);
    EXPECT_EQ(5, x.x[1].x);
    EXPECT_EQ(6, x.x[1].y);

    EXPECT_EQ(std::string(
            "x {\n"
            "  x: 3\n"
            "  y: 4\n"
            "}\n"
            "x {\n"
            "  x: 5\n"
            "  y: 6\n"
            "}\n"), x.print());
}

TEST(generated_arrays, FixcompDynamic)
{
    std::vector<uint8_t> data(1024);

    FixcompDynamic x;
    x.x.resize(2);
    x.x[0].x = 1;
    x.x[0].y = 2;
    x.x[1].x = 3;
    x.x[1].y = 4;
    size_t size = x.encode(data.data());

    EXPECT_EQ(20, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes(
            "\x02\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00"),
            bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x01\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00")));
    EXPECT_EQ(1, x.x.size());
    EXPECT_EQ(4, x.x[0].x);
    EXPECT_EQ(5, x.x[0].y);

    EXPECT_EQ(std::string(
            "x {\n"
            "  x: 4\n"
            "  y: 5\n"
            "}\n"), x.print());
}

TEST(generated_arrays, FixcompLimited)
{
    std::vector<uint8_t> data(1024);

    FixcompLimited x;
    x.x.resize(1);
    x.x[0].x = 1;
    x.x[0].y = 2;
    size_t size = x.encode(data.data());

    EXPECT_EQ(20, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes(
            "\x01\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
            bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x01\x00\x00\x00\x05\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")));
    EXPECT_EQ(1, x.x.size());
    EXPECT_EQ(5, x.x[0].x);
    EXPECT_EQ(6, x.x[0].y);

    EXPECT_EQ(std::string(
            "x {\n"
            "  x: 5\n"
            "  y: 6\n"
            "}\n"), x.print());
}

TEST(generated_arrays, FixcompGreedy)
{
    std::vector<uint8_t> data(1024);

    FixcompGreedy x;
    x.x.resize(2);
    x.x[0].x = 1;
    x.x[0].y = 2;
    x.x[1].x = 3;
    x.x[1].y = 4;
    size_t size = x.encode(data.data());

    EXPECT_EQ(16, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes(
            "\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00"),
            bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x03\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00\x06\x00\x00\x00")));
    EXPECT_EQ(2, x.x.size());
    EXPECT_EQ(3, x.x[0].x);
    EXPECT_EQ(4, x.x[0].y);
    EXPECT_EQ(5, x.x[1].x);
    EXPECT_EQ(6, x.x[1].y);

    EXPECT_EQ(std::string(
            "x {\n"
            "  x: 3\n"
            "  y: 4\n"
            "}\n"
            "x {\n"
            "  x: 5\n"
            "  y: 6\n"
            "}\n"), x.print());
}

TEST(generated_arrays, Dyncomp)
{
    std::vector<uint8_t> data(1024);

    Dyncomp x;
    x.x.x.push_back(1);
    x.x.x.push_back(2);
    x.x.x.push_back(3);
    size_t size = x.encode(data.data());

    EXPECT_EQ(16, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes(
            "\x03\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00"),
            bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x03\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00\x06\x00\x00\x00")));
    EXPECT_EQ(3, x.x.x.size());
    EXPECT_EQ(4, x.x.x[0]);
    EXPECT_EQ(5, x.x.x[1]);
    EXPECT_EQ(6, x.x.x[2]);

    EXPECT_EQ(std::string(
            "x {\n"
            "  x: 4\n"
            "  x: 5\n"
            "  x: 6\n"
            "}\n"), x.print());
}

TEST(generated_arrays, DyncompDynamic)
{
    std::vector<uint8_t> data(1024);

    DyncompDynamic x;
    x.x.resize(2);
    x.x[0].x.push_back(1);
    x.x[0].x.push_back(2);
    x.x[0].x.push_back(3);
    x.x[1].x.push_back(4);
    size_t size = x.encode(data.data());

    EXPECT_EQ(28, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes(
            "\x02\x00\x00\x00"
            "\x03\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00"
            "\x01\x00\x00\x00\x04\x00\x00\x00"),
            bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x02\x00\x00\x00"
            "\x02\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00"
            "\x01\x00\x00\x00\x03\x00\x00\x00")));
    EXPECT_EQ(2, x.x.size());
    EXPECT_EQ(2, x.x[0].x.size());
    EXPECT_EQ(1, x.x[0].x[0]);
    EXPECT_EQ(2, x.x[0].x[1]);
    EXPECT_EQ(1, x.x[1].x.size());
    EXPECT_EQ(3, x.x[1].x[0]);

    EXPECT_EQ(std::string(
            "x {\n"
            "  x: 1\n"
            "  x: 2\n"
            "}\n"
            "x {\n"
            "  x: 3\n"
            "}\n"), x.print());
}

TEST(generated_arrays, DyncompGreedy)
{
    std::vector<uint8_t> data(1024);

    DyncompGreedy x;
    x.x.resize(2);
    x.x[0].x.push_back(1);
    x.x[0].x.push_back(2);
    x.x[0].x.push_back(3);
    x.x[1].x.push_back(4);
    size_t size = x.encode(data.data());

    EXPECT_EQ(24, size);
    EXPECT_EQ(size, x.get_byte_size());
    EXPECT_EQ(bytes(
            "\x03\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00"
            "\x01\x00\x00\x00\x04\x00\x00\x00"),
            bytes(data.data(), size));

    EXPECT_TRUE(x.decode(bytes(
            "\x03\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00\x06\x00\x00\x00"
            "\x01\x00\x00\x00\x07\x00\x00\x00")));
    EXPECT_EQ(2, x.x.size());
    EXPECT_EQ(3, x.x[0].x.size());
    EXPECT_EQ(4, x.x[0].x[0]);
    EXPECT_EQ(5, x.x[0].x[1]);
    EXPECT_EQ(6, x.x[0].x[2]);
    EXPECT_EQ(1, x.x[1].x.size());
    EXPECT_EQ(7, x.x[1].x[0]);

    EXPECT_EQ(std::string(
            "x {\n"
            "  x: 4\n"
            "  x: 5\n"
            "  x: 6\n"
            "}\n"
            "x {\n"
            "  x: 7\n"
            "}\n"), x.print());
}
