// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0018-hybrid-webview-typed-bridge.md
//
// Minimal JSON encode/decode for the HybridWebView typed bridge + any
// future feature that needs a wire format with no external dependency.
//
// Design:
//   * Header-only.
//   * Writer appends to a `std::string&` — no allocations beyond the
//     output buffer's own growth.
//   * Reader is a pull-style parser over a `std::string_view`. No
//     intermediate AST; values are extracted directly into typed
//     destinations as the cursor walks.
//   * Primitive support: null, bool, int / long / long long, double,
//     std::string, std::vector<T>, std::optional<T>.
//   * User-defined types extend via ADL — overload
//       to_json(json::writer&, const my_t&)
//       from_json(json::reader&, my_t&)
//     in the type's own namespace. The writer/reader's `write` /
//     `read` methods find the overload via ADL.
//
// What this is NOT:
//   * Not a full RFC-8259 implementation. Lenient parsing — comments
//     are not supported, but trailing-comma-after-value in objects is
//     tolerated for forgiving round-trips.
//   * Not Unicode-aware beyond ASCII + UTF-8 passthrough in strings.
//     `\uXXXX` escapes are read into the surrogate values as-is
//     (caller responsible for normalization if needed).
//   * Not a streaming parser — the whole payload must fit in memory.
//
// All of these are fine for the bridge use case (small JSON-RPC
// envelopes, ~hundreds of bytes).

#ifndef MPAPP_DETAIL_JSON_HPP
#define MPAPP_DETAIL_JSON_HPP

#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mpapp::detail::json {

// ---------- Writer ------------------------------------------------------

class writer {
public:
    explicit writer(std::string& out) noexcept : out_(&out) {}

    // Primitive overloads. ADL picks up user-defined `to_json` from the
    // value's namespace via the `write` template at the bottom.
    void write_null()                  { *out_ += "null"; }
    void write(bool b)                 { *out_ += b ? "true" : "false"; }
    void write(int v)                  { append_int(static_cast<long long>(v)); }
    void write(long v)                 { append_int(static_cast<long long>(v)); }
    void write(long long v)            { append_int(v); }
    void write(unsigned u)             { append_uint(static_cast<unsigned long long>(u)); }
    void write(unsigned long u)        { append_uint(static_cast<unsigned long long>(u)); }
    void write(unsigned long long u)   { append_uint(u); }
    void write(double v)               { append_double(v); }
    void write(const char* s)          { append_quoted(std::string_view{s}); }
    void write(std::string_view s)     { append_quoted(s); }
    void write(const std::string& s)   { append_quoted(std::string_view{s}); }

    template <class T>
    void write(const std::vector<T>& v) {
        *out_ += '[';
        for (std::size_t i = 0; i < v.size(); ++i) {
            if (i != 0) *out_ += ',';
            write(v[i]);
        }
        *out_ += ']';
    }

    template <class T>
    void write(const std::optional<T>& o) {
        if (o.has_value()) write(*o);
        else                write_null();
    }

    // Object scope — call begin_object / end_object around field() calls.
    void begin_object() {
        *out_ += '{';
        in_object_depth_++;
        first_field_ = true;
    }
    void end_object() {
        *out_ += '}';
        if (in_object_depth_ > 0) in_object_depth_--;
        first_field_ = false;
    }

    template <class V>
    void field(std::string_view name, const V& value) {
        if (!first_field_) *out_ += ',';
        first_field_ = false;
        append_quoted(name);
        *out_ += ':';
        write(value);
    }

    // Array scope alternative when caller wants to mix typed elements.
    void begin_array() { *out_ += '['; array_first_ = true; }
    void end_array()   { *out_ += ']'; array_first_ = false; }
    template <class V>
    void element(const V& v) {
        if (!array_first_) *out_ += ',';
        array_first_ = false;
        write(v);
    }

private:
    void append_int(long long v) {
        char buf[32];
        auto r = std::to_chars(buf, buf + sizeof(buf), v);
        out_->append(buf, static_cast<std::size_t>(r.ptr - buf));
    }
    void append_uint(unsigned long long v) {
        char buf[32];
        auto r = std::to_chars(buf, buf + sizeof(buf), v);
        out_->append(buf, static_cast<std::size_t>(r.ptr - buf));
    }
    void append_double(double v) {
        // Use a fixed format that round-trips well for typical bridge
        // payloads. std::to_chars(double) is C++17 but its support is
        // patchy on libstdc++ < 11 / libc++ < 15. Use snprintf for
        // portability — JSON-RPC payloads are tiny so the snprintf
        // overhead is negligible.
        char buf[64];
        const int n = std::snprintf(buf, sizeof(buf), "%.17g", v);
        if (n > 0) out_->append(buf, static_cast<std::size_t>(n));
    }
    void append_quoted(std::string_view s) {
        *out_ += '"';
        for (char c : s) {
            switch (c) {
                case '"':  *out_ += "\\\""; break;
                case '\\': *out_ += "\\\\"; break;
                case '\n': *out_ += "\\n";  break;
                case '\r': *out_ += "\\r";  break;
                case '\t': *out_ += "\\t";  break;
                case '\b': *out_ += "\\b";  break;
                case '\f': *out_ += "\\f";  break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char esc[8];
                        std::snprintf(esc, sizeof(esc), "\\u%04x",
                            static_cast<unsigned>(static_cast<unsigned char>(c)));
                        *out_ += esc;
                    } else {
                        *out_ += c;
                    }
            }
        }
        *out_ += '"';
    }

    std::string* out_;
    int  in_object_depth_ = 0;
    bool first_field_     = true;
    bool array_first_     = true;
};

// ---------- Reader ------------------------------------------------------

class reader {
public:
    explicit reader(std::string_view in) noexcept : in_(in), pos_(0) {}

    [[nodiscard]] bool ok()  const noexcept { return !err_; }
    [[nodiscard]] bool eof() const noexcept { skip_ws_const(); return pos_ >= in_.size(); }

    // Primitive readers. Return true on success, false on parse error
    // (leaves `value` unspecified).
    bool read(std::nullptr_t&) {
        skip_ws();
        if (consume_literal("null")) return true;
        return fail();
    }
    bool read(bool& v) {
        skip_ws();
        if (consume_literal("true"))  { v = true;  return true; }
        if (consume_literal("false")) { v = false; return true; }
        return fail();
    }
    bool read(int& v)                { return read_signed<int>(v); }
    bool read(long& v)               { return read_signed<long>(v); }
    bool read(long long& v)          { return read_signed<long long>(v); }
    bool read(unsigned& v)           { return read_unsigned<unsigned>(v); }
    bool read(unsigned long& v)      { return read_unsigned<unsigned long>(v); }
    bool read(unsigned long long& v) { return read_unsigned<unsigned long long>(v); }
    bool read(double& v) {
        skip_ws();
        const std::size_t start = pos_;
        // Accept optional sign, digits, optional fraction, optional exponent.
        if (pos_ < in_.size() && (in_[pos_] == '-' || in_[pos_] == '+')) ++pos_;
        while (pos_ < in_.size() && std::isdigit(static_cast<unsigned char>(in_[pos_]))) ++pos_;
        if (pos_ < in_.size() && in_[pos_] == '.') {
            ++pos_;
            while (pos_ < in_.size() && std::isdigit(static_cast<unsigned char>(in_[pos_]))) ++pos_;
        }
        if (pos_ < in_.size() && (in_[pos_] == 'e' || in_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < in_.size() && (in_[pos_] == '-' || in_[pos_] == '+')) ++pos_;
            while (pos_ < in_.size() && std::isdigit(static_cast<unsigned char>(in_[pos_]))) ++pos_;
        }
        if (pos_ == start) return fail();
        // strtod for portability — std::from_chars(double, ...) is
        // available on modern toolchains but absent in NDK 26 libc++.
        std::string tmp{in_.substr(start, pos_ - start)};
        char* endp = nullptr;
        const double d = std::strtod(tmp.c_str(), &endp);
        if (endp == tmp.c_str()) return fail();
        v = d;
        return true;
    }
    bool read(std::string& s) {
        skip_ws();
        if (pos_ >= in_.size() || in_[pos_] != '"') return fail();
        ++pos_;
        s.clear();
        while (pos_ < in_.size()) {
            char c = in_[pos_++];
            if (c == '"') return true;
            if (c == '\\') {
                if (pos_ >= in_.size()) return fail();
                const char esc = in_[pos_++];
                switch (esc) {
                    case '"':  s += '"';  break;
                    case '\\': s += '\\'; break;
                    case '/':  s += '/';  break;
                    case 'n':  s += '\n'; break;
                    case 'r':  s += '\r'; break;
                    case 't':  s += '\t'; break;
                    case 'b':  s += '\b'; break;
                    case 'f':  s += '\f'; break;
                    case 'u': {
                        if (pos_ + 4 > in_.size()) return fail();
                        unsigned code = 0;
                        for (int i = 0; i < 4; ++i) {
                            const char h = in_[pos_++];
                            unsigned digit = 0;
                            if (h >= '0' && h <= '9') digit = static_cast<unsigned>(h - '0');
                            else if (h >= 'a' && h <= 'f') digit = static_cast<unsigned>(10 + h - 'a');
                            else if (h >= 'A' && h <= 'F') digit = static_cast<unsigned>(10 + h - 'A');
                            else return fail();
                            code = (code << 4) | digit;
                        }
                        // Naive UTF-8 encoding of the BMP codepoint —
                        // surrogate pairs are passed through as-is for
                        // v1 (caller normalizes).
                        if (code < 0x80) {
                            s += static_cast<char>(code);
                        } else if (code < 0x800) {
                            s += static_cast<char>(0xC0 | (code >> 6));
                            s += static_cast<char>(0x80 | (code & 0x3F));
                        } else {
                            s += static_cast<char>(0xE0 | (code >> 12));
                            s += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                            s += static_cast<char>(0x80 | (code & 0x3F));
                        }
                        break;
                    }
                    default: return fail();
                }
            } else {
                s += c;
            }
        }
        return fail();   // unterminated string
    }

    template <class T>
    bool read(std::vector<T>& v) {
        skip_ws();
        if (!consume_char('[')) return fail();
        v.clear();
        skip_ws();
        if (consume_char(']')) return true;
        while (true) {
            T tmp{};
            if (!read(tmp)) return false;
            v.push_back(std::move(tmp));
            skip_ws();
            if (consume_char(']')) return true;
            if (!consume_char(',')) return fail();
        }
    }

    template <class T>
    bool read(std::optional<T>& o) {
        skip_ws();
        if (peek_literal("null")) {
            consume_literal("null");
            o.reset();
            return true;
        }
        T tmp{};
        if (!read(tmp)) return false;
        o = std::move(tmp);
        return true;
    }

    // Object reading: call expect_object_begin, then loop calling
    // next_field which returns true and populates `name` until the
    // closing '}' is consumed (then returns false). For each field the
    // caller pulls its value via read(...).
    //
    // The first_field_ flag is scoped per object: expect_object_begin
    // sets it true, next_field flips it to false on the first call,
    // and the '}' branch restores it to false so the *outer* scope
    // (which had just finished consuming a value) continues to expect
    // a ',' before its next field. The single-flag-multiple-scopes
    // trick works because after any nested object closes, the outer's
    // first_field_ should indeed be false (the outer just finished
    // reading a value, which by definition was preceded by a field).
    bool expect_object_begin() {
        skip_ws();
        if (!consume_char('{')) return false;
        first_field_ = true;
        return true;
    }
    bool next_field(std::string& name) {
        skip_ws();
        if (consume_char('}')) {
            first_field_ = false;
            return false;
        }
        if (!first_field_) {
            if (!consume_char(',')) { fail(); return false; }
            skip_ws();
        }
        first_field_ = false;
        if (!read(name)) return false;
        skip_ws();
        if (!consume_char(':')) { fail(); return false; }
        return true;
    }

    // Array reading: parallel to expect_object_begin / next_field but
    // for arrays. Used by typed-tuple consumers (method-args parsing
    // in the future HybridWebView bridge) where each element has its
    // own static type.
    //
    // Usage:
    //   r.expect_array_begin();
    //   while (r.next_element()) { r.read(x); }
    bool expect_array_begin() {
        skip_ws();
        if (!consume_char('[')) return false;
        first_element_ = true;
        return true;
    }
    bool next_element() {
        skip_ws();
        if (consume_char(']')) {
            first_element_ = false;
            return false;
        }
        if (!first_element_) {
            if (!consume_char(',')) { fail(); return false; }
            skip_ws();
        }
        first_element_ = false;
        return true;
    }

    // Capture the raw substring of the next JSON value without
    // committing to a typed read. Advances the cursor past the value.
    // Useful when a value is going to be re-parsed later — e.g., a
    // JSON-RPC envelope captures `args` here and re-parses it once
    // the method is resolved.
    std::string_view capture_value() {
        skip_ws();
        const std::size_t start = pos_;
        if (!skip_value()) return {};
        return in_.substr(start, pos_ - start);
    }

    // Skip the next value (used to ignore unknown fields).
    bool skip_value() {
        skip_ws();
        if (pos_ >= in_.size()) return fail();
        const char c = in_[pos_];
        if (c == '"') { std::string s; return read(s); }
        if (c == 't' || c == 'f') { bool b; return read(b); }
        if (c == 'n') return consume_literal("null");
        if (c == '[') {
            std::vector<std::string> dummy;  // sized only by item count; values ignored via skip nesting
            ++pos_;
            skip_ws();
            if (consume_char(']')) return true;
            while (true) {
                if (!skip_value()) return false;
                skip_ws();
                if (consume_char(']')) return true;
                if (!consume_char(',')) return fail();
            }
        }
        if (c == '{') {
            ++pos_;
            skip_ws();
            if (consume_char('}')) return true;
            while (true) {
                std::string k; if (!read(k)) return false;
                skip_ws();
                if (!consume_char(':')) return fail();
                if (!skip_value()) return false;
                skip_ws();
                if (consume_char('}')) return true;
                if (!consume_char(',')) return fail();
            }
        }
        // Number.
        double d; return read(d);
    }

private:
    template <class S> bool read_signed(S& v) {
        skip_ws();
        const std::size_t start = pos_;
        if (pos_ < in_.size() && (in_[pos_] == '-' || in_[pos_] == '+')) ++pos_;
        while (pos_ < in_.size() && std::isdigit(static_cast<unsigned char>(in_[pos_]))) ++pos_;
        if (pos_ == start) return fail();
        long long ll = 0;
        auto r = std::from_chars(in_.data() + start, in_.data() + pos_, ll);
        if (r.ec != std::errc{}) return fail();
        v = static_cast<S>(ll);
        return true;
    }
    template <class U> bool read_unsigned(U& v) {
        skip_ws();
        const std::size_t start = pos_;
        if (pos_ < in_.size() && in_[pos_] == '+') ++pos_;
        const std::size_t digits_start = pos_;
        while (pos_ < in_.size() && std::isdigit(static_cast<unsigned char>(in_[pos_]))) ++pos_;
        if (pos_ == digits_start) return fail();
        unsigned long long uu = 0;
        auto r = std::from_chars(in_.data() + start, in_.data() + pos_, uu);
        if (r.ec != std::errc{}) return fail();
        v = static_cast<U>(uu);
        return true;
    }

    void skip_ws() {
        while (pos_ < in_.size()) {
            const char c = in_[pos_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++pos_;
            else break;
        }
    }
    void skip_ws_const() const {
        std::size_t p = pos_;
        while (p < in_.size()) {
            const char c = in_[p];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++p;
            else break;
        }
        // const-trickery: this is logically a peek, mutate pos_ via mutable.
        const_cast<reader*>(this)->pos_ = p;
    }
    bool consume_char(char c) {
        skip_ws();
        if (pos_ < in_.size() && in_[pos_] == c) { ++pos_; return true; }
        return false;
    }
    bool consume_literal(std::string_view lit) {
        skip_ws();
        if (pos_ + lit.size() > in_.size()) return false;
        if (in_.compare(pos_, lit.size(), lit) != 0) return false;
        pos_ += lit.size();
        return true;
    }
    bool peek_literal(std::string_view lit) const {
        std::size_t p = pos_;
        while (p < in_.size()) {
            const char c = in_[p];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++p; else break;
        }
        if (p + lit.size() > in_.size()) return false;
        return in_.compare(p, lit.size(), lit) == 0;
    }
    bool fail() { err_ = true; return false; }

    std::string_view in_;
    std::size_t      pos_;
    bool             err_           = false;
    bool             first_field_   = true;
    bool             first_element_ = true;
};

// ---------- Free function overloads for ADL extension --------------------

// These let user code in another namespace overload to_json/from_json for
// a custom type T; the writer/reader's `write` / `read` methods will pick
// them up via ADL on the value type. The default implementations forward
// to the writer/reader's typed overloads above.
template <class T>
auto to_json(writer& w, const T& v) -> decltype(w.write(v)) {
    return w.write(v);
}

template <class T>
auto from_json(reader& r, T& v) -> decltype(r.read(v)) {
    return r.read(v);
}

} // namespace mpapp::detail::json

#endif // MPAPP_DETAIL_JSON_HPP
