// SPDX-License-Identifier: Apache-2.0
// Tests for mpapp/detail/json.hpp — the JSON layer the future
// HybridWebView typed bridge (per ADR-0018) will sit on.

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mpapp/detail/json.hpp>

using Catch::Approx;

namespace mpapp_json = mpapp::detail::json;

TEST_CASE("writer encodes JSON primitives",
          "[detail][json][writer]") {
    std::string out;
    mpapp_json::writer w{out};
    w.write_null();
    CHECK(out == "null");

    out.clear();
    w.write(true);
    CHECK(out == "true");

    out.clear();
    w.write(42);
    CHECK(out == "42");

    out.clear();
    w.write(-7);
    CHECK(out == "-7");

    out.clear();
    w.write(std::string{"hello"});
    CHECK(out == "\"hello\"");

    out.clear();
    w.write(std::string{"with \"quotes\" and \\ and\nnewline"});
    CHECK(out == "\"with \\\"quotes\\\" and \\\\ and\\nnewline\"");

    out.clear();
    // Control characters get \u escapes.
    w.write(std::string{"\x01\x1f"});
    CHECK(out == "\"\\u0001\\u001f\"");
}

TEST_CASE("writer encodes objects via field()",
          "[detail][json][writer]") {
    std::string out;
    mpapp_json::writer w{out};
    w.begin_object();
    w.field("id", 7);
    w.field("method", std::string{"add"});
    w.field("ok", true);
    w.end_object();
    CHECK(out == R"({"id":7,"method":"add","ok":true})");
}

TEST_CASE("writer encodes vectors and optionals",
          "[detail][json][writer]") {
    std::string out;
    mpapp_json::writer w{out};
    w.write(std::vector<int>{1, 2, 3});
    CHECK(out == "[1,2,3]");

    out.clear();
    w.write(std::vector<std::string>{"a", "b"});
    CHECK(out == R"(["a","b"])");

    out.clear();
    w.write(std::optional<int>{42});
    CHECK(out == "42");

    out.clear();
    w.write(std::optional<int>{});
    CHECK(out == "null");
}

TEST_CASE("reader decodes JSON primitives",
          "[detail][json][reader]") {
    {
        mpapp_json::reader r{"true"};
        bool b = false;
        REQUIRE(r.read(b));
        CHECK(b == true);
    }
    {
        mpapp_json::reader r{"false"};
        bool b = true;
        REQUIRE(r.read(b));
        CHECK(b == false);
    }
    {
        mpapp_json::reader r{"42"};
        int v = 0;
        REQUIRE(r.read(v));
        CHECK(v == 42);
    }
    {
        mpapp_json::reader r{"-13"};
        int v = 0;
        REQUIRE(r.read(v));
        CHECK(v == -13);
    }
    {
        mpapp_json::reader r{"3.14"};
        double d = 0;
        REQUIRE(r.read(d));
        CHECK(d == 3.14);
    }
    {
        mpapp_json::reader r{R"("hello world")"};
        std::string s;
        REQUIRE(r.read(s));
        CHECK(s == "hello world");
    }
    {
        mpapp_json::reader r{R"("a\nb\t\"c\"")"};
        std::string s;
        REQUIRE(r.read(s));
        CHECK(s == "a\nb\t\"c\"");
    }
}

TEST_CASE("reader decodes nested object via next_field",
          "[detail][json][reader]") {
    const std::string payload = R"({"id": 7, "method": "add", "args": [1, 2]})";
    mpapp_json::reader r{payload};

    REQUIRE(r.expect_object_begin());
    int            id     = 0;
    std::string    method;
    std::vector<int> args;

    std::string field_name;
    while (r.next_field(field_name)) {
        if      (field_name == "id")     { REQUIRE(r.read(id));     }
        else if (field_name == "method") { REQUIRE(r.read(method)); }
        else if (field_name == "args")   { REQUIRE(r.read(args));   }
        else                              { REQUIRE(r.skip_value()); }
    }
    REQUIRE(r.ok());
    CHECK(id     == 7);
    CHECK(method == "add");
    REQUIRE(args.size() == 2);
    CHECK(args[0] == 1);
    CHECK(args[1] == 2);
}

TEST_CASE("reader handles vectors of strings",
          "[detail][json][reader]") {
    mpapp_json::reader r{R"(["alpha","beta","gamma"])"};
    std::vector<std::string> v;
    REQUIRE(r.read(v));
    REQUIRE(v.size() == 3);
    CHECK(v[0] == "alpha");
    CHECK(v[1] == "beta");
    CHECK(v[2] == "gamma");
}

TEST_CASE("reader handles empty array and empty object",
          "[detail][json][reader]") {
    {
        mpapp_json::reader r{"[]"};
        std::vector<int> v;
        REQUIRE(r.read(v));
        CHECK(v.empty());
    }
    {
        mpapp_json::reader r{"{}"};
        REQUIRE(r.expect_object_begin());
        std::string name;
        CHECK(!r.next_field(name));
        CHECK(r.ok());
    }
}

TEST_CASE("reader treats null as nullopt for optional<T>",
          "[detail][json][reader]") {
    {
        mpapp_json::reader r{"null"};
        std::optional<int> o;
        REQUIRE(r.read(o));
        CHECK(!o.has_value());
    }
    {
        mpapp_json::reader r{"42"};
        std::optional<int> o;
        REQUIRE(r.read(o));
        REQUIRE(o.has_value());
        CHECK(*o == 42);
    }
}

TEST_CASE("reader fails cleanly on malformed input",
          "[detail][json][reader]") {
    {
        // unterminated string
        mpapp_json::reader r{R"("no closing quote)"};
        std::string s;
        CHECK(!r.read(s));
        CHECK(!r.ok());
    }
    {
        // not a bool
        mpapp_json::reader r{"truu"};
        bool b = false;
        CHECK(!r.read(b));
        CHECK(!r.ok());
    }
    {
        // not a number
        mpapp_json::reader r{"abc"};
        int v = 0;
        CHECK(!r.read(v));
        CHECK(!r.ok());
    }
    {
        // array missing closer
        mpapp_json::reader r{"[1, 2, 3"};
        std::vector<int> v;
        CHECK(!r.read(v));
        CHECK(!r.ok());
    }
}

TEST_CASE("round-trip: writer output is reader input",
          "[detail][json][roundtrip]") {
    // Build a JSON-RPC-shaped envelope, parse it back.
    std::string buf;
    mpapp_json::writer w{buf};
    w.begin_object();
    w.field("id",     17);
    w.field("method", std::string{"echo"});
    w.field("args",   std::vector<std::string>{"hi", "world"});
    w.end_object();

    mpapp_json::reader r{buf};
    REQUIRE(r.expect_object_begin());
    int             id     = 0;
    std::string     method;
    std::vector<std::string> args;
    std::string name;
    while (r.next_field(name)) {
        if      (name == "id")     { REQUIRE(r.read(id));     }
        else if (name == "method") { REQUIRE(r.read(method)); }
        else if (name == "args")   { REQUIRE(r.read(args));   }
        else                        { REQUIRE(r.skip_value()); }
    }
    REQUIRE(r.ok());
    CHECK(id     == 17);
    CHECK(method == "echo");
    REQUIRE(args.size() == 2);
    CHECK(args[0] == "hi");
    CHECK(args[1] == "world");
}

// User-type extension via ADL — overload to_json/from_json in the
// type's own namespace. The writer/reader's `write`/`read` methods
// pick the overload up.
namespace point_ns {
struct point { int x; int y; };

void to_json(mpapp_json::writer& w, const point& p) {
    w.begin_object();
    w.field("x", p.x);
    w.field("y", p.y);
    w.end_object();
}

bool from_json(mpapp_json::reader& r, point& p) {
    if (!r.expect_object_begin()) return false;
    std::string name;
    while (r.next_field(name)) {
        if      (name == "x") { if (!r.read(p.x)) return false; }
        else if (name == "y") { if (!r.read(p.y)) return false; }
        else                   { if (!r.skip_value()) return false; }
    }
    return r.ok();
}
} // namespace point_ns

TEST_CASE("writer escapes the full control-character set",
          "[detail][json][writer]") {
    std::string out;
    mpapp_json::writer w{out};
    w.write(std::string{"\r\t\b\f"});
    CHECK(out == "\"\\r\\t\\b\\f\"");
}

TEST_CASE("reader parses doubles with exponents",
          "[detail][json][reader]") {
    {
        mpapp_json::reader r{"1.5e-3"};
        double d = 0;
        REQUIRE(r.read(d));
        CHECK(d == Approx(0.0015));
    }
    {
        mpapp_json::reader r{"2E2"};
        double d = 0;
        REQUIRE(r.read(d));
        CHECK(d == Approx(200.0));
    }
}

TEST_CASE("reader decodes every string escape",
          "[detail][json][reader]") {
    mpapp_json::reader r{R"("\\\/\n\r\t\b\f")"};
    std::string s;
    REQUIRE(r.read(s));
    CHECK(s == std::string("\\/\n\r\t\b\f"));
}

TEST_CASE("reader decodes \\uXXXX across the UTF-8 byte-length ranges",
          "[detail][json][reader]") {
    // U+0041 'A' (1 byte), U+00E9 'é' (2 bytes), U+20AC '€' (3 bytes).
    // Mixes lower- and upper-case hex digits to exercise every digit branch.
    std::string in;
    in += '"';
    in += static_cast<char>(0x5C); in += "u0041";   // 0x5C = '\'
    in += static_cast<char>(0x5C); in += "u00e9";
    in += static_cast<char>(0x5C); in += "u20AC";
    in += '"';
    mpapp_json::reader r{in};
    std::string s;
    REQUIRE(r.read(s));
    CHECK(s == std::string("A\xC3\xA9\xE2\x82\xAC"));
}

TEST_CASE("reader rejects malformed escapes",
          "[detail][json][reader]") {
    {   // non-hex digit in \u
        mpapp_json::reader r{R"("\u00zz")"};
        std::string s;
        CHECK(!r.read(s));
    }
    {   // truncated \u (fewer than 4 hex digits before end)
        mpapp_json::reader r{R"("\u12")"};
        std::string s;
        CHECK(!r.read(s));
    }
    {   // unknown escape letter
        mpapp_json::reader r{R"("\x")"};
        std::string s;
        CHECK(!r.read(s));
    }
}

TEST_CASE("reader skip_value walks over nested object values",
          "[detail][json][reader]") {
    mpapp_json::reader r{R"({"meta":{"a":1,"nested":{"x":[1,2]}},"id":9})"};
    REQUIRE(r.expect_object_begin());
    int id = 0;
    std::string name;
    while (r.next_field(name)) {
        if (name == "id") { REQUIRE(r.read(id)); }
        else               { REQUIRE(r.skip_value()); }  // skips the meta object
    }
    REQUIRE(r.ok());
    CHECK(id == 9);
}

TEST_CASE("user type round-trips via ADL to_json/from_json",
          "[detail][json][adl]") {
    std::string buf;
    {
        mpapp_json::writer w{buf};
        to_json(w, point_ns::point{3, 4});
    }
    CHECK(buf == R"({"x":3,"y":4})");

    mpapp_json::reader r{buf};
    point_ns::point p{};
    REQUIRE(from_json(r, p));
    CHECK(p.x == 3);
    CHECK(p.y == 4);
}
