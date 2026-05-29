// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0006-visual-state-manager.md
//
// `mpapp::visual_state_manager` — grouped pseudo-state setters that
// every interactive control switches between (Normal / Pressed /
// PointerOver / Disabled / Focused / Selected). Counterpart to
// MAUI's `VisualStateManager`. Sits beside `mpapp::style` (RFC-0005)
// and reuses the same `function<void(view&)>` setter shape.
//
// Mock surface — apps drive transitions explicitly via
// `vsm.go_to_state(view, "Pressed")`. The per-platform real layer
// (per [[ADR-0008]] follow-ups) will auto-route system input events
// to the canonical state names declared in `mpapp::visual_states`.
//
// Like resource_dictionary and style, VSM is NOT a wrapper-component
// (ADR-0024) — it owns no native widget and has no embedded handler.

#ifndef MPAPP_RESOURCES_VISUAL_STATE_MANAGER_HPP
#define MPAPP_RESOURCES_VISUAL_STATE_MANAGER_HPP

#include <exception>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mpapp {

// Forward-declared — `visual_state` only references `view&` via the
// type-erased setter signature (same approach as `style.hpp`).
class view;

// Canonical names for the system-driven states the framework
// recognises. Apps may define custom names freely; only states in
// this namespace get auto-routed by per-platform real handlers when
// that follow-up work lands.
namespace visual_states {

inline constexpr std::string_view normal       = "Normal";
inline constexpr std::string_view disabled     = "Disabled";
inline constexpr std::string_view focused      = "Focused";
inline constexpr std::string_view selected     = "Selected";
inline constexpr std::string_view pointer_over = "PointerOver";
inline constexpr std::string_view pressed      = "Pressed";

} // namespace visual_states

class visual_state {
public:
    visual_state() = default;
    explicit visual_state(std::string state_name) : name{ std::move(state_name) } {}

    // Public string so designated initializers
    // (`visual_state{ .name = ..., .setters = ... }`) work.
    std::string name{};

    // Per-property setters, keyed by property name so XAML codegen
    // (M-09) can lower `<Setter Property="Opacity" Value="0.5"/>`
    // to an insert at `setters["Opacity"] = …`.
    std::unordered_map<std::string, std::function<void(view&)>> setters{};
};

class visual_state_group {
public:
    visual_state_group() = default;
    explicit visual_state_group(std::string group_name) : name{ std::move(group_name) } {}

    std::string                name{};
    std::vector<visual_state>  states{};

    // Name of the currently-applied state. Empty before the first
    // transition. Only one state per group is active at a time
    // (matches MAUI's `CurrentState`).
    std::string                current_state{};

    // Linear scan — group state lists are tiny (Normal / Pressed /
    // PointerOver / Disabled / Focused / Selected is the typical
    // shape), so a hash map's per-key overhead would lose to the
    // scan on every realistic input.
    [[nodiscard]] const visual_state* find_state(std::string_view state_name) const noexcept {
        for (const auto& s : states) {
            if (s.name == state_name) {
                return &s;
            }
        }
        return nullptr;
    }
};

class visual_state_manager {
public:
    visual_state_manager() = default;

    visual_state_manager(const visual_state_manager&)            = delete;
    visual_state_manager& operator=(const visual_state_manager&) = delete;
    visual_state_manager(visual_state_manager&&)                 = delete;
    visual_state_manager& operator=(visual_state_manager&&)      = delete;

    ~visual_state_manager() = default;

    std::vector<visual_state_group> groups{};

    // Walk every group, find the first state that matches
    // `state_name`, run its setters against `v`, mark the group's
    // current_state. Returns the count of groups that actually
    // transitioned (i.e., the matching state was different from
    // the group's `current_state`). Same-state calls are a no-op
    // and do NOT count.
    //
    // Multiple groups may contain a state with the same name; all
    // of them transition (matches MAUI's behaviour). The typical
    // single-group case returns 0 (miss) or 1 (hit).
    int go_to_state(view& v, std::string_view state_name) {
        int transitions = 0;
        for (auto& g : groups) {
            const auto* target = g.find_state(state_name);
            if (target == nullptr) {
                continue;
            }
            if (g.current_state == state_name) {
                // Same state — no-op, do not re-run setters.
                continue;
            }
            g.current_state = std::string{ state_name };
            for (const auto& [_, setter] : target->setters) {
                if (!setter) {
                    continue;
                }
                try {
                    setter(v);
                } catch (const std::exception&) {
                    // Swallowed per RFC-0006 §Detailed Design ⟶
                    // mirrors RFC-0005 style exception handling.
                } catch (...) {
                    // Same as above for non-std exceptions.
                }
            }
            ++transitions;
        }
        return transitions;
    }

    // Convenience: snapshot of every group's current state. Useful
    // for tests + for the per-platform real layer to know which
    // system state each group reports.
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
    snapshot_current_states() const {
        std::vector<std::pair<std::string, std::string>> out;
        out.reserve(groups.size());
        for (const auto& g : groups) {
            out.emplace_back(g.name, g.current_state);
        }
        return out;
    }
};

} // namespace mpapp

#endif // MPAPP_RESOURCES_VISUAL_STATE_MANAGER_HPP
