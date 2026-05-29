// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/View.md
//
// `mpapp::view` — cross-platform base of every visible widget. Mirrors
// MAUI's `IView` / `ViewHandler.ViewMapper` cross-cutting property
// surface (identity, layout, visual state, transforms, hit-testing).
//
// This is the **mock surface** (P2 / ADR-0008): every property is wired
// up as an `Observable<T>`, every command is a `Command<>` tag, and a
// handler reference is stored non-owning. Sets are forwarded to the
// platform handler via `notify_<prop>()` helpers that exist only so the
// mock handler can record them — real handlers route the same way.
//
// For the mock surface the rich types (`brush_ref`, `shadow`,
// `semantics`, `geometry_ref`) are reduced to lightweight stand-ins
// (`std::string` for resource names, plain enums for state). The full
// types land alongside their real handlers in P3+.

#ifndef MPAPP_VIEW_HPP
#define MPAPP_VIEW_HPP

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_VIEW_HAS_STD_FORMAT 1
#endif

#include "behaviors/behavior.hpp"                  // for view::behaviors (RFC-0009)
#include "binding/binding_context.hpp"            // for view::binding_context (RFC-0007)
#include "command.hpp"
#include "effects/effect.hpp"                      // for view::effects (RFC-0009)
#include "internal/basic_gesture_recognizer.hpp"  // for view::gesture_recognizers
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

// Forward-declared so view can hold a `shared_ptr<resource_dictionary>`
// without dragging in the dictionary's `<any>` / `<unordered_map>` cost
// in headers that only touch view's primitive members. Translation
// units that want to mutate `resources` include
// `<mpapp/resources/resource_dictionary.hpp>` themselves (or the
// `static_resource.hpp` walker, which transitively brings it in).
class resource_dictionary;

// Lightweight stand-ins for the rich types the full View surface will
// carry. Defined here so every layout-group mock can use them without a
// dedicated header per primitive. The real geometry / brush / semantics
// types replace these in P3 — the property names and units stay the same.

enum class visibility : std::uint8_t {
    visible   = 0,
    hidden    = 1,
    collapsed = 2,
};

enum class flow_direction : std::uint8_t {
    match_parent = 0,
    left_to_right = 1,
    right_to_left = 2,
};

// A bitmask of edges the framework treats as safe-area boundaries. The
// mock surface stores it as a plain `std::uint8_t` — bit 0 = top, bit 1
// = right, bit 2 = bottom, bit 3 = left. The full `safe_area_edges`
// enum / struct lands with the real handlers.
using safe_area_edges_mask = std::uint8_t;

// Symbolic brush reference. Real implementations replace this with a
// `brush_ref` variant over solid / linear / radial brushes; the mock
// records the string repr.
struct brush_ref {
    std::string name;

    bool operator==(const brush_ref&) const = default;
};

// Symbolic shadow descriptor — enough for tests to observe "shadow set".
struct shadow_desc {
    double offset_x = 0.0;
    double offset_y = 0.0;
    double radius   = 0.0;
    double opacity  = 0.0;

    bool operator==(const shadow_desc&) const = default;
};

// Forward-declared so the cross-platform header doesn't pull in the mock
// (or any real) backend. Each platform specialises this template under
// `mpapp/handlers/<platform>/view_handler.hpp`.
template <class Platform = platform::current>
class view_handler;

class view {
public:
    view() = default;
    virtual ~view() = default;

    view(const view&)            = delete;
    view& operator=(const view&) = delete;
    view(view&&)                 = delete;
    view& operator=(view&&)      = delete;

    // ----- Identity / accessibility -------------------------------------
    Observable<std::string>                 automation_id{""};
    Observable<std::optional<std::string>>  tool_tip{std::nullopt};
    // Screen-reader accessible name (MAUI SemanticProperties.Description).
    // "" = unset: real handlers leave the platform default (e.g. a button's
    // own text) in place. Applied by the per-control handler's
    // `map_semantics` (GTK accessible label / WinUI AutomationProperties.Name
    // / Android contentDescription).
    Observable<std::string>                 semantic_description{""};

    // ----- Layout -------------------------------------------------------
    Observable<double>                      width{-1.0};          // -1 = unset / auto
    Observable<double>                      height{-1.0};
    Observable<double>                      minimum_width{0.0};
    Observable<double>                      minimum_height{0.0};
    Observable<double>                      maximum_width{0.0};
    Observable<double>                      maximum_height{0.0};
    Observable<flow_direction>              flow{flow_direction::match_parent};
    Observable<safe_area_edges_mask>        safe_area_edges{0};

    // ----- Visual state -------------------------------------------------
    Observable<visibility>                  visibility_state{visibility::visible};
    Observable<bool>                        is_enabled{true};
    Observable<double>                      opacity{1.0};
    Observable<brush_ref>                   background{};
    Observable<shadow_desc>                 shadow{};

    // ----- Transforms ---------------------------------------------------
    Observable<double>                      translation_x{0.0};
    Observable<double>                      translation_y{0.0};
    Observable<double>                      scale{1.0};
    Observable<double>                      scale_x{1.0};
    Observable<double>                      scale_y{1.0};
    Observable<double>                      rotation{0.0};
    Observable<double>                      rotation_x{0.0};
    Observable<double>                      rotation_y{0.0};
    Observable<double>                      anchor_x{0.5};
    Observable<double>                      anchor_y{0.5};
    Observable<int>                         z_index{0};

    // ----- Hit testing --------------------------------------------------
    Observable<bool>                        input_transparent{false};

    // ----- Gesture recognizers (per RFC-0003) ---------------------------
    // Polymorphic collection of recognizers attached to this view. Held
    // by `shared_ptr` so binding layers / view-models can hold a ref to
    // mutate config without invalidating the slot list. The platform
    // `view_handler<P>::map_gestures(*this)` walks this vector and
    // installs the matching native listener for each recognizer's
    // `kind()` (see `RFC-0003-gesture-recognizers` for the per-platform
    // wire-up table).
    std::vector<std::shared_ptr<internal::basic_gesture_recognizer>>
                                            gesture_recognizers{};

    // Convenience emplace + return. `T` must derive from
    // `internal::basic_gesture_recognizer`. Returns a reference so
    // app code reads as:
    //     auto& tap = btn.add_gesture<mpapp::tap_gesture_recognizer>();
    //     tap.number_of_taps_required = 2;
    //     tap.tapped.subscribe(slot, cb);
    template <class T, class... Args>
    T& add_gesture(Args&&... args) {
        static_assert(std::is_base_of_v<internal::basic_gesture_recognizer, T>,
                      "view::add_gesture<T>: T must derive from "
                      "mpapp::internal::basic_gesture_recognizer");
        auto p = std::make_shared<T>(std::forward<Args>(args)...);
        T& ref = *p;
        gesture_recognizers.push_back(std::move(p));
        return ref;
    }

    // ----- Behaviors (per RFC-0009) -------------------------------------
    // Attached behaviors, MAUI's element.Behaviors. Held by shared_ptr
    // so view-models can keep a ref. `add_behavior` constructs + calls
    // on_attached; `remove_behavior` calls on_detached + drops. View
    // destruction simply drops the behaviors (the defaulted dtor does
    // not call on_detached — call remove_behavior first if a behavior's
    // detach has side effects). Mirrors `gesture_recognizers`.
    std::vector<std::shared_ptr<behavior>> behaviors{};

    template <class B, class... Args>
    B& add_behavior(Args&&... args) {
        static_assert(std::is_base_of_v<behavior, B>,
                      "view::add_behavior<B>: B must derive from mpapp::behavior");
        auto p = std::make_shared<B>(std::forward<Args>(args)...);
        B& ref = *p;
        behaviors.push_back(std::move(p));
        ref.on_attached(*this);
        return ref;
    }

    void remove_behavior(behavior& b) {
        for (auto it = behaviors.begin(); it != behaviors.end(); ++it) {
            if (it->get() == &b) {
                b.on_detached(*this);
                behaviors.erase(it);
                return;
            }
        }
    }

    // ----- Effects (per RFC-0009) ---------------------------------------
    // Attached platform effects, MAUI's element.Effects. Prefer a handler
    // for new code (ADR-0024); this exists for surface parity + legacy
    // XAML lowering.
    std::vector<std::shared_ptr<effect>> effects{};

    template <class E, class... Args>
    E& add_effect(Args&&... args) {
        static_assert(std::is_base_of_v<effect, E>,
                      "view::add_effect<E>: E must derive from mpapp::effect");
        auto p = std::make_shared<E>(std::forward<Args>(args)...);
        E& ref = *p;
        effects.push_back(std::move(p));
        ref.on_attached(*this);
        return ref;
    }

    // ----- Commands -----------------------------------------------------
    // Declared with the Command<> tag per ADR-0009. The XAML compiler
    // recognises the tag and lowers `Command="…"` bindings to direct
    // calls. The mock surface keeps the bodies trivial — the framework
    // owns the routing-to-handler concern and lands with the M-03
    // command mapper plumbing.
    void invalidate_measure(Command<> = {}) noexcept {}
    void focus(Command<>            = {}) noexcept {}
    void unfocus(Command<>          = {}) noexcept {}

    // ----- Resources (per RFC-0005) -------------------------------------
    // Per-view resource dictionary. Optional — null means "no local
    // resources, walk parent". `find_in<T>(view, key)` from
    // `<mpapp/resources/static_resource.hpp>` performs the hierarchical
    // walk (this_view.resources → parent.resources → ... → app.resources)
    // and returns the first typed match.
    //
    // Held by `shared_ptr` so app code and the XAML binding layer can
    // share a dictionary across views (theme + composition use this).
    // Default-null keeps the per-view memory cost at a single pointer
    // for views that never touch resources.
    std::shared_ptr<resource_dictionary> resources{};

    // ----- Visual-tree parent (used by find_in + future layout walks) --
    // Set by `layout::add` / `layout::insert` to wire the parent chain.
    // Tests that exercise `find_in` directly may also call `set_parent`
    // manually to stand up a hierarchy without a layout.
    [[nodiscard]] view* parent() const noexcept { return parent_; }
    void               set_parent(view* p) noexcept { parent_ = p; }

    // ----- Data-binding context (per RFC-0007) --------------------------
    // The view's LOCAL binding context. Empty by default; the inherited
    // (effective) context — a child uses its own if set, else the nearest
    // ancestor's — is resolved by `effective_binding_context(view)` in
    // <mpapp/binding/relative_source.hpp>, which walks the `parent()`
    // chain. Mirrors MAUI's BindingContext + its inheritance down the
    // visual tree.
    [[nodiscard]] binding_context&       local_binding_context() noexcept       { return binding_ctx_; }
    [[nodiscard]] const binding_context& local_binding_context() const noexcept { return binding_ctx_; }

    template <class C>
    void set_binding_context(std::shared_ptr<C> ctx) { binding_ctx_.set(std::move(ctx)); }

    // ----- Handler ------------------------------------------------------
    view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                   has_handler() const noexcept { return handler_ != nullptr; }
    void                                   set_handler(view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    view*                            parent_  = nullptr;
    binding_context                  binding_ctx_{};
    view_handler<platform::current>* handler_ = nullptr;
};

// Stable string repr for the mock-recording layer. Defined inline so any
// translation unit that includes <mpapp/view.hpp> gets a consistent
// std::format spelling for the enums — tests assert on these strings.

constexpr std::string_view to_string(visibility v) noexcept {
    switch (v) {
        case visibility::visible:   return "visible";
        case visibility::hidden:    return "hidden";
        case visibility::collapsed: return "collapsed";
    }
    return "?";
}

constexpr std::string_view to_string(flow_direction f) noexcept {
    switch (f) {
        case flow_direction::match_parent:  return "match_parent";
        case flow_direction::left_to_right: return "ltr";
        case flow_direction::right_to_left: return "rtl";
    }
    return "?";
}

} // namespace mpapp

// std::formatter specialisations — keep the recording stable across
// platforms / locales / compilers. They live with the type so any
// `std::format("{}", value)` call site (mock handler or user code) gets
// the same string.

#ifdef MPAPP_VIEW_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::visibility> : std::formatter<std::string_view> {
    auto format(mpapp::visibility v, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(v), ctx);
    }
};

template <>
struct std::formatter<mpapp::flow_direction> : std::formatter<std::string_view> {
    auto format(mpapp::flow_direction f, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(f), ctx);
    }
};

template <>
struct std::formatter<mpapp::brush_ref> : std::formatter<std::string_view> {
    auto format(const mpapp::brush_ref& b, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(b.name, ctx);
    }
};

template <>
struct std::formatter<mpapp::shadow_desc> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::shadow_desc& s, std::format_context& ctx) const {
        return std::format_to(ctx.out(),
                              "shadow(dx={},dy={},r={},a={})",
                              s.offset_x, s.offset_y, s.radius, s.opacity);
    }
};

#endif // MPAPP_VIEW_HAS_STD_FORMAT

#endif // MPAPP_VIEW_HPP
