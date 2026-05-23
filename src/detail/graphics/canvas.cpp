// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Backend-independent parsers + helpers for the graphics
// facade per [[ADR-0015-graphics-backend-dual]].
//
// `color::from_hex` and `path::from_svg` live here because they are
// pure value-type operations that any backend can use unchanged.

#include <mpapp/detail/graphics/canvas.hpp>

#include <cctype>
#include <cstdlib>
#include <string>

namespace mpapp::detail::graphics {

namespace {

constexpr int hex_value(char c) noexcept {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

} // namespace

color color::from_hex(std::string_view hex) noexcept {
    if (!hex.empty() && hex.front() == '#') hex.remove_prefix(1);
    // Accept 6-char "RRGGBB" or 8-char "RRGGBBAA".
    if (hex.size() != 6 && hex.size() != 8) return color{};
    int channels[4]{0, 0, 0, 255};
    const std::size_t count = hex.size() / 2;
    for (std::size_t i = 0; i < count; ++i) {
        const int hi = hex_value(hex[i * 2]);
        const int lo = hex_value(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) return color{};
        channels[i] = (hi << 4) | lo;
    }
    return color{
        static_cast<float>(channels[0]) / 255.0f,
        static_cast<float>(channels[1]) / 255.0f,
        static_cast<float>(channels[2]) / 255.0f,
        static_cast<float>(channels[3]) / 255.0f,
    };
}

// ---- SVG path parser ------------------------------------------------------
//
// Supports the subset documented in canvas.hpp: M / L / Q / C / Z
// (uppercase absolute coords). Whitespace and commas separate numbers.
// Returns an empty path if any token fails to parse — same fail-quiet
// semantics as color::from_hex.

namespace {

class svg_cursor {
public:
    explicit svg_cursor(std::string_view s) noexcept : s_{s} {}

    [[nodiscard]] bool ok() const noexcept { return ok_; }
    [[nodiscard]] bool at_end() const noexcept { return i_ >= s_.size(); }

    void skip_sep() noexcept {
        while (i_ < s_.size() && (std::isspace(static_cast<unsigned char>(s_[i_])) || s_[i_] == ',')) {
            ++i_;
        }
    }

    char peek_command() noexcept {
        skip_sep();
        if (i_ >= s_.size()) return 0;
        const char c = s_[i_];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            ++i_;
            return c;
        }
        return 0;
    }

    double next_number() noexcept {
        skip_sep();
        if (i_ >= s_.size()) { ok_ = false; return 0.0; }
        const char* start = s_.data() + i_;
        char*       end   = nullptr;
        const double v = std::strtod(start, &end);
        if (end == start) { ok_ = false; return 0.0; }
        i_ += static_cast<std::size_t>(end - start);
        return v;
    }

private:
    std::string_view s_;
    std::size_t      i_  = 0;
    bool             ok_ = true;
};

} // namespace

path path::from_svg(std::string_view svg) {
    path p;
    svg_cursor cur{svg};
    while (cur.ok()) {
        const char cmd = cur.peek_command();
        if (cmd == 0) break;
        switch (cmd) {
            case 'M': {
                const double x = cur.next_number();
                const double y = cur.next_number();
                if (!cur.ok()) return path{};
                p.move_to(x, y);
                break;
            }
            case 'L': {
                const double x = cur.next_number();
                const double y = cur.next_number();
                if (!cur.ok()) return path{};
                p.line_to(x, y);
                break;
            }
            case 'Q': {
                const double cx = cur.next_number();
                const double cy = cur.next_number();
                const double x  = cur.next_number();
                const double y  = cur.next_number();
                if (!cur.ok()) return path{};
                p.quad_to(cx, cy, x, y);
                break;
            }
            case 'C': {
                const double cx1 = cur.next_number();
                const double cy1 = cur.next_number();
                const double cx2 = cur.next_number();
                const double cy2 = cur.next_number();
                const double x   = cur.next_number();
                const double y   = cur.next_number();
                if (!cur.ok()) return path{};
                p.cubic_to(cx1, cy1, cx2, cy2, x, y);
                break;
            }
            case 'Z':
            case 'z':
                p.close();
                break;
            default:
                // Unknown command — bail. Caller gets an empty path.
                return path{};
        }
    }
    return p;
}

} // namespace mpapp::detail::graphics
