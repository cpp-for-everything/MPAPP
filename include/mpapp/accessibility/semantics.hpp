// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Accessibility.md
//
// `mpapp::semantics` — attached-property facility for MAUI-style
// SemanticProperties. Keyed on view* (mirrors basic_grid_layout's
// per-child placement store). Stores description, hint, heading level,
// and accessible-tree inclusion flag per view.
//
// `mpapp::semantic_screen_reader` — interface for platform screen-reader
// integration. `mock_semantic_screen_reader` records announcements for
// unit testing.
//
// ADR-0002: no macros in the public API surface.

#ifndef MPAPP_ACCESSIBILITY_SEMANTICS_HPP
#define MPAPP_ACCESSIBILITY_SEMANTICS_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../view.hpp"

namespace mpapp {

// -------------------------------------------------------------------------
// heading_level — mirrors MAUI SemanticProperties.HeadingLevel
// -------------------------------------------------------------------------

enum class heading_level : std::uint8_t {
    none   = 0,
    level1 = 1,
    level2 = 2,
    level3 = 3,
    level4 = 4,
    level5 = 5,
    level6 = 6,
};

constexpr std::string_view to_string(heading_level h) noexcept {
    switch (h) {
        case heading_level::none:   return "none";
        case heading_level::level1: return "level1";
        case heading_level::level2: return "level2";
        case heading_level::level3: return "level3";
        case heading_level::level4: return "level4";
        case heading_level::level5: return "level5";
        case heading_level::level6: return "level6";
    }
    return "?";
}

// -------------------------------------------------------------------------
// semantics — attached-property store keyed on view*
//
// Usage (mirrors Grid.SetRow / Grid.GetRow):
//   semantics::set_description(my_view, "Profile photo");
//   auto desc = semantics::get_description(my_view);
// -------------------------------------------------------------------------

class semantics {
public:
    semantics() = delete; // static-only API

    static void set_description(view& v, std::string desc) {
        store_for(v).description_ = std::move(desc);
    }
    [[nodiscard]] static std::string get_description(const view& v) {
        auto it = store().find(key(v));
        return (it == store().end()) ? std::string{} : it->second.description_;
    }

    static void set_hint(view& v, std::string hint) {
        store_for(v).hint_ = std::move(hint);
    }
    [[nodiscard]] static std::string get_hint(const view& v) {
        auto it = store().find(key(v));
        return (it == store().end()) ? std::string{} : it->second.hint_;
    }

    static void set_heading_level(view& v, heading_level hl) {
        store_for(v).heading_level_ = hl;
    }
    [[nodiscard]] static heading_level get_heading_level(const view& v) {
        auto it = store().find(key(v));
        return (it == store().end()) ? heading_level::none : it->second.heading_level_;
    }

    static void set_is_in_accessible_tree(view& v, bool value) {
        store_for(v).is_in_accessible_tree_ = value;
    }
    [[nodiscard]] static bool get_is_in_accessible_tree(const view& v) {
        auto it = store().find(key(v));
        return (it == store().end()) ? true : it->second.is_in_accessible_tree_;
    }

    // Remove all attached properties for a view. Call before the view's
    // lifetime ends if its address could be reused (mirrors grid_layout
    // ADR-0014 cleanup note).
    static void clear(const view& v) {
        store().erase(key(v));
    }

private:
    struct entry {
        std::string  description_{};
        std::string  hint_{};
        heading_level heading_level_{ heading_level::none };
        bool         is_in_accessible_tree_{ true };
    };

    using key_t   = const view*;
    using store_t = std::unordered_map<key_t, entry>;

    static store_t& store() noexcept {
        static store_t s;
        return s;
    }

    static key_t key(const view& v) noexcept { return &v; }

    static entry& store_for(view& v) {
        return store()[key(v)];
    }
};

// -------------------------------------------------------------------------
// semantic_screen_reader — platform abstraction
// -------------------------------------------------------------------------

class semantic_screen_reader {
public:
    virtual ~semantic_screen_reader() = default;

    // Announce a string to the platform screen reader immediately.
    virtual void announce(std::string_view text) = 0;

protected:
    semantic_screen_reader() = default;
    semantic_screen_reader(const semantic_screen_reader&) = default;
    semantic_screen_reader& operator=(const semantic_screen_reader&) = default;
    semantic_screen_reader(semantic_screen_reader&&) = default;
    semantic_screen_reader& operator=(semantic_screen_reader&&) = default;
};

// -------------------------------------------------------------------------
// mock_semantic_screen_reader — records announcements for tests
// -------------------------------------------------------------------------

class mock_semantic_screen_reader : public semantic_screen_reader {
public:
    mock_semantic_screen_reader() = default;

    void announce(std::string_view text) override {
        announcements_.emplace_back(text);
    }

    [[nodiscard]] const std::vector<std::string>& announcements() const noexcept {
        return announcements_;
    }

    void clear_announcements() noexcept {
        announcements_.clear();
    }

    [[nodiscard]] bool empty() const noexcept {
        return announcements_.empty();
    }

private:
    std::vector<std::string> announcements_{};
};

} // namespace mpapp

#endif // MPAPP_ACCESSIBILITY_SEMANTICS_HPP
