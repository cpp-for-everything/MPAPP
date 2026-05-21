// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Grid.md
//                  vault/20_ADRs/ADR-0017-grid-track-definitions.md
//
// `mpapp::grid_layout` — 2D layout container with row + column tracks
// and per-child placement. Native rendering: Windows `mux::Controls::Grid`,
// Linux `GtkGrid`, Android `android.widget.GridLayout`. Each platform's
// native container does the actual measure + arrange — MPAPP just maps
// track_def -> the native track API.

#ifndef MPAPP_GRID_LAYOUT_HPP
#define MPAPP_GRID_LAYOUT_HPP

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

#include "layout.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

// A row or column track in a Grid. `Auto` sizes to its largest child,
// `Star(w)` takes a proportional weight of leftover space, `Fixed(px)`
// is a literal pixel width/height.
struct track_def {
    enum class kind : std::uint8_t { fixed, auto_, star };
    kind   k     = kind::auto_;
    double value = 0.0;   // pixels for fixed, weight for star (default 1.0)

    static constexpr track_def Auto() noexcept { return {kind::auto_, 0.0}; }
    static constexpr track_def Star(double weight = 1.0) noexcept { return {kind::star, weight}; }
    static constexpr track_def Fixed(double px) noexcept { return {kind::fixed, px}; }

    bool operator==(const track_def&) const = default;

    // Tiny parser: "Auto, *, 200, 2*" -> vector<track_def>. Whitespace
    // tolerant. Unknown tokens silently become Auto so the surface
    // doesn't throw on malformed XAML strings (an upstream validator
    // catches typos earlier).
    static std::vector<track_def> parse(std::string_view spec) {
        std::vector<track_def> out;
        auto trim = [](std::string_view s) {
            while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
            while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))  s.remove_suffix(1);
            return s;
        };
        std::size_t i = 0;
        while (i <= spec.size()) {
            std::size_t comma = spec.find(',', i);
            std::string_view tok = trim(spec.substr(i, comma == std::string_view::npos ? std::string_view::npos : comma - i));
            if (!tok.empty()) {
                if (tok == "Auto" || tok == "auto") {
                    out.push_back(track_def::Auto());
                } else if (tok == "*") {
                    out.push_back(track_def::Star(1.0));
                } else if (tok.back() == '*') {
                    // "N*" — weighted star.
                    double w = std::strtod(std::string(tok.substr(0, tok.size() - 1)).c_str(), nullptr);
                    if (!(w > 0.0)) w = 1.0;
                    out.push_back(track_def::Star(w));
                } else {
                    // Pixel literal.
                    double px = std::strtod(std::string(tok).c_str(), nullptr);
                    out.push_back(track_def::Fixed(px));
                }
            }
            if (comma == std::string_view::npos) break;
            i = comma + 1;
        }
        return out;
    }
};

template <class Platform>
class grid_layout_handler;

class grid_layout : public layout {
public:
    grid_layout() = default;
    ~grid_layout() override = default;

    grid_layout(const grid_layout&)            = delete;
    grid_layout& operator=(const grid_layout&) = delete;
    grid_layout(grid_layout&&)                 = delete;
    grid_layout& operator=(grid_layout&&)      = delete;

    // Legacy fixed-count surface. Real handlers prefer the
    // row_definitions / column_definitions value-type vectors when
    // populated, falling back to row_count / column_count of Auto
    // tracks otherwise.
    Observable<int>    row_count{1};
    Observable<int>    column_count{1};
    Observable<double> row_spacing{0.0};
    Observable<double> column_spacing{0.0};

    // ADR-0017 surface — typed track definitions. Empty means "fall back
    // to row_count Auto tracks" so existing apps don't break.
    Observable<std::vector<track_def>> row_definitions{};
    Observable<std::vector<track_def>> column_definitions{};

    // Convenience helper for XAML round-trip: populate from a
    // MAUI-style spec string.
    void set_rows_from_spec(std::string_view spec) {
        row_definitions.set(track_def::parse(spec));
    }
    void set_columns_from_spec(std::string_view spec) {
        column_definitions.set(track_def::parse(spec));
    }

    grid_layout_handler<platform::current>&       handler() noexcept       { return *grid_handler_; }
    const grid_layout_handler<platform::current>& handler() const noexcept { return *grid_handler_; }
    bool                                          has_handler() const noexcept { return grid_handler_ != nullptr; }
    void                                          set_handler(grid_layout_handler<platform::current>& h) noexcept { grid_handler_ = &h; }

private:
    grid_layout_handler<platform::current>* grid_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_GRID_LAYOUT_HPP
