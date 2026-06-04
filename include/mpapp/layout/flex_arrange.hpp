// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/FlexLayout.md
//
// `mpapp::flex_arrange` — a pure, platform-independent CSS-flexbox
// arrange solver. Given a container description and a list of flex
// item descriptions, it computes the final rectangle of every child.
//
// This is the layout-math core that the platform `flex_layout_handler`
// implementations can lean on (or validate against). It performs no
// platform calls, allocates only the result vector + per-line scratch,
// and is fully deterministic. The algorithm follows the CSS Flexible
// Box Layout main/cross-axis model:
//
//   1. Sort items by `order` (stable).
//   2. Break items into flex lines according to wrap.
//   3. Resolve flexible lengths on the main axis (grow / shrink).
//   4. Distribute remaining free space with `justify_content`.
//   5. Size + position on the cross axis with `align_items` /
//      `align_self` (stretch / center / start / end).
//   6. Distribute lines across the cross axis with `align_content`.
//   7. Apply direction (row / column) and the *_reverse variants.

#ifndef MPAPP_LAYOUT_FLEX_ARRANGE_HPP
#define MPAPP_LAYOUT_FLEX_ARRANGE_HPP

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

#include "../internal/basic_flex_layout.hpp"

namespace mpapp {

// Computed rectangle for a single child, in container-local coordinates.
struct flex_rect {
    double x      = 0.0;
    double y      = 0.0;
    double width  = 0.0;
    double height = 0.0;

    bool operator==(const flex_rect&) const = default;
};

// One child's contribution to the solve.
//
//   basis         — main-axis base size; -1 means "auto" (use measured_main).
//   grow          — flex-grow factor (>= 0).
//   shrink        — flex-shrink factor (>= 0).
//   align_self    — per-child cross-axis override (auto_ defers to container).
//   measured_main — content size along the main axis (used when basis auto).
//   measured_cross— content size along the cross axis (used when not stretched).
//   order         — visual ordering key; lower comes first (stable on ties).
struct flex_item_input {
    double          basis          = -1.0;
    double          grow           = 0.0;
    double          shrink         = 1.0;
    flex_align_self align_self     = flex_align_self::auto_;
    double          measured_main  = 0.0;
    double          measured_cross = 0.0;
    int             order          = 0;
};

// The container's resolved geometry + flex properties.
struct flex_container_input {
    double             width           = 0.0;
    double             height          = 0.0;
    flex_direction     direction       = flex_direction::row;
    flex_wrap          wrap            = flex_wrap::no_wrap;
    flex_justify       justify_content = flex_justify::start;
    flex_align_items   align_items     = flex_align_items::stretch;
    flex_align_content align_content   = flex_align_content::stretch;
    double             main_gap        = 0.0;
    double             cross_gap       = 0.0;
};

namespace internal::flex {

// Is the main axis horizontal? (row / row_reverse)
[[nodiscard]] constexpr bool is_row(flex_direction d) noexcept {
    return d == flex_direction::row || d == flex_direction::row_reverse;
}

// Is the main axis reversed? (row_reverse / column_reverse)
[[nodiscard]] constexpr bool is_main_reverse(flex_direction d) noexcept {
    return d == flex_direction::row_reverse
        || d == flex_direction::column_reverse;
}

// Resolved base main-axis size of an item (honoring auto basis).
[[nodiscard]] constexpr double base_main(const flex_item_input& it) noexcept {
    return it.basis < 0.0 ? it.measured_main : it.basis;
}

// Effective cross-axis alignment for an item (resolving align_self::auto_).
[[nodiscard]] constexpr flex_align_items
resolve_cross_align(flex_align_self self, flex_align_items container) noexcept {
    switch (self) {
        case flex_align_self::stretch: return flex_align_items::stretch;
        case flex_align_self::center:  return flex_align_items::center;
        case flex_align_self::start:   return flex_align_items::start;
        case flex_align_self::end:     return flex_align_items::end;
        case flex_align_self::auto_:   break;
    }
    return container;
}

// Result of laying a single line's items along the main axis. Each entry
// carries the item's index (into the order-sorted list), its resolved
// main size and its natural cross size.
struct placed_item {
    std::size_t index    = 0;
    double      main     = 0.0;  // resolved main-axis length
    double      cross    = 0.0;  // natural cross-axis length
    double      main_pos = 0.0;  // main-axis offset (line-local)
    double      cross_pos = 0.0; // cross-axis offset (container-local)
};

struct flex_line {
    std::vector<placed_item> items;
    double main_used = 0.0;   // sum of item main sizes (no gaps)
    double cross_size = 0.0;  // tallest natural cross size in the line
};

} // namespace internal::flex

// Compute the arranged rectangle of every input item.
//
// The returned vector is parallel to `items` (same length, same indexing),
// regardless of `order` or direction reversal — element i always describes
// items[i].
[[nodiscard]] inline std::vector<flex_rect>
flex_arrange(const flex_container_input& container,
             const std::vector<flex_item_input>& items) {
    using namespace internal::flex;

    const std::size_t n = items.size();
    std::vector<flex_rect> out(n);
    if (n == 0) {
        return out;
    }

    const bool   row          = is_row(container.direction);
    const bool   main_reverse = is_main_reverse(container.direction);
    const bool   wrap_reverse = container.wrap == flex_wrap::wrap_reverse;
    const bool   multiline    = container.wrap != flex_wrap::no_wrap;
    const double main_size    = row ? container.width  : container.height;
    const double cross_size   = row ? container.height : container.width;
    const double main_gap     = container.main_gap;
    const double cross_gap    = container.cross_gap;

    // ---- 1. order sort (stable) -------------------------------------
    std::vector<std::size_t> seq(n);
    std::iota(seq.begin(), seq.end(), std::size_t{0});
    std::stable_sort(seq.begin(), seq.end(),
                     [&](std::size_t a, std::size_t b) {
                         return items[a].order < items[b].order;
                     });

    // ---- 2. break into flex lines -----------------------------------
    std::vector<flex_line> lines;
    {
        flex_line current;
        for (std::size_t k = 0; k < seq.size(); ++k) {
            const std::size_t idx = seq[k];
            const double item_main = base_main(items[idx]);

            // Adding this item to the current line introduces one extra
            // gap per already-present item.
            const double used_with_gaps =
                current.main_used
                + main_gap * static_cast<double>(current.items.size())
                + item_main;

            if (multiline && !current.items.empty()
                && used_with_gaps > main_size) {
                lines.push_back(std::move(current));
                current = flex_line{};
            }

            placed_item pi;
            pi.index = idx;
            pi.main  = item_main;
            pi.cross = items[idx].measured_cross;
            current.main_used += item_main;
            current.cross_size =
                std::max(current.cross_size, items[idx].measured_cross);
            current.items.push_back(pi);
        }
        if (!current.items.empty()) {
            lines.push_back(std::move(current));
        }
    }

    // ---- 3. resolve flexible lengths (grow / shrink) per line -------
    for (flex_line& line : lines) {
        const std::size_t count = line.items.size();
        const double total_gap =
            count > 1 ? main_gap * static_cast<double>(count - 1) : 0.0;
        const double free = main_size - line.main_used - total_gap;

        if (free > 0.0) {
            double total_grow = 0.0;
            for (const placed_item& pi : line.items) {
                total_grow += items[pi.index].grow;
            }
            if (total_grow > 0.0) {
                for (placed_item& pi : line.items) {
                    const double g = items[pi.index].grow;
                    pi.main += free * (g / total_grow);
                }
            }
        } else if (free < 0.0) {
            // Shrink weighted by shrink * base size.
            double total_scaled = 0.0;
            for (const placed_item& pi : line.items) {
                total_scaled += items[pi.index].shrink * pi.main;
            }
            if (total_scaled > 0.0) {
                const double deficit = -free;
                for (placed_item& pi : line.items) {
                    const double scaled = items[pi.index].shrink * pi.main;
                    double next = pi.main - deficit * (scaled / total_scaled);
                    if (next < 0.0) {
                        next = 0.0;
                    }
                    pi.main = next;
                }
            }
        }

        // Recompute the line's used main extent after flexing.
        double used = 0.0;
        for (const placed_item& pi : line.items) {
            used += pi.main;
        }
        line.main_used = used;
    }

    // ---- 4. justify_content (main-axis distribution) ----------------
    for (flex_line& line : lines) {
        const std::size_t count = line.items.size();
        const double total_gap =
            count > 1 ? main_gap * static_cast<double>(count - 1) : 0.0;
        const double free = main_size - line.main_used - total_gap;
        const double remaining = free > 0.0 ? free : 0.0;
        const double dcount = static_cast<double>(count);

        double offset = 0.0;
        double between = main_gap;
        switch (container.justify_content) {
            case flex_justify::start:
                offset = 0.0;
                break;
            case flex_justify::center:
                offset = remaining / 2.0;
                break;
            case flex_justify::end:
                offset = remaining;
                break;
            case flex_justify::space_between:
                offset = 0.0;
                between = main_gap
                          + (count > 1 ? remaining / (dcount - 1.0) : 0.0);
                break;
            case flex_justify::space_around: {
                const double unit = remaining / dcount;
                offset = unit / 2.0;
                between = main_gap + unit;
                break;
            }
            case flex_justify::space_evenly: {
                const double unit = remaining / (dcount + 1.0);
                offset = unit;
                between = main_gap + unit;
                break;
            }
        }

        double cursor = offset;
        for (std::size_t k = 0; k < line.items.size(); ++k) {
            placed_item& pi = line.items[k];
            pi.main_pos = cursor;
            cursor += pi.main + between;
        }
    }

    // ---- 5. align_content (cross-axis line distribution) ------------
    double lines_cross = 0.0;
    for (const flex_line& line : lines) {
        lines_cross += line.cross_size;
    }
    const std::size_t line_count = lines.size();
    const double cross_gap_total =
        line_count > 1 ? cross_gap * static_cast<double>(line_count - 1)
                       : 0.0;
    const double cross_free = cross_size - lines_cross - cross_gap_total;

    // Per-line stretch height (align_content::stretch) and offsets.
    std::vector<double> line_cross_size(line_count);
    for (std::size_t i = 0; i < line_count; ++i) {
        line_cross_size[i] = lines[i].cross_size;
    }

    double line_offset = 0.0;
    double line_between = cross_gap;
    const double dlines = static_cast<double>(line_count);
    const double cross_remaining = cross_free > 0.0 ? cross_free : 0.0;

    switch (container.align_content) {
        case flex_align_content::start:
            line_offset = 0.0;
            break;
        case flex_align_content::center:
            line_offset = cross_remaining / 2.0;
            break;
        case flex_align_content::end:
            line_offset = cross_remaining;
            break;
        case flex_align_content::stretch:
            line_offset = 0.0;
            if (cross_free > 0.0 && line_count > 0) {
                const double add = cross_free / dlines;
                for (double& s : line_cross_size) {
                    s += add;
                }
            }
            break;
        case flex_align_content::space_between:
            line_offset = 0.0;
            line_between = cross_gap
                           + (line_count > 1
                                  ? cross_remaining / (dlines - 1.0)
                                  : 0.0);
            break;
        case flex_align_content::space_around: {
            const double unit = line_count > 0 ? cross_remaining / dlines : 0.0;
            line_offset = unit / 2.0;
            line_between = cross_gap + unit;
            break;
        }
    }

    // Single-line containers ignore align_content for offset purposes
    // (matches flexbox: a single line fills / aligns via align_items).
    if (line_count == 1) {
        line_offset = 0.0;
        if (container.align_content == flex_align_content::stretch
            && cross_free > 0.0) {
            line_cross_size[0] = lines[0].cross_size + cross_free;
        }
    }

    std::vector<double> line_cross_pos(line_count);
    {
        double c = line_offset;
        for (std::size_t i = 0; i < line_count; ++i) {
            line_cross_pos[i] = c;
            c += line_cross_size[i] + line_between;
        }
    }

    // Honor wrap_reverse: mirror the line ordering on the cross axis.
    if (wrap_reverse && line_count > 1) {
        for (std::size_t i = 0; i < line_count; ++i) {
            line_cross_pos[i] =
                cross_size - line_cross_pos[i] - line_cross_size[i];
        }
    }

    // ---- 6. cross-axis sizing/positioning per item ------------------
    for (std::size_t i = 0; i < line_count; ++i) {
        flex_line& line = lines[i];
        const double line_pos  = line_cross_pos[i];
        const double line_extent = line_cross_size[i];

        for (placed_item& pi : line.items) {
            const flex_align_items align = resolve_cross_align(
                items[pi.index].align_self, container.align_items);

            double item_cross = pi.cross;
            double item_cross_pos = line_pos;

            switch (align) {
                case flex_align_items::stretch:
                    item_cross = line_extent;
                    item_cross_pos = line_pos;
                    break;
                case flex_align_items::center:
                    item_cross_pos = line_pos + (line_extent - item_cross) / 2.0;
                    break;
                case flex_align_items::start:
                    item_cross_pos = line_pos;
                    break;
                case flex_align_items::end:
                    item_cross_pos = line_pos + (line_extent - item_cross);
                    break;
            }

            pi.cross      = item_cross;
            pi.cross_pos  = item_cross_pos;
        }
    }

    // ---- 7. emit rectangles (apply direction + reversals) -----------
    for (const flex_line& line : lines) {
        for (const placed_item& pi : line.items) {
            double main_pos = pi.main_pos;
            if (main_reverse) {
                main_pos = main_size - pi.main_pos - pi.main;
            }

            flex_rect r;
            if (row) {
                r.x      = main_pos;
                r.y      = pi.cross_pos;
                r.width  = pi.main;
                r.height = pi.cross;
            } else {
                r.x      = pi.cross_pos;
                r.y      = main_pos;
                r.width  = pi.cross;
                r.height = pi.main;
            }
            out[pi.index] = r;
        }
    }

    return out;
}

} // namespace mpapp

#endif // MPAPP_LAYOUT_FLEX_ARRANGE_HPP
