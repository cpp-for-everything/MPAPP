// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Templates.md
//
// `mpapp::content_presenter` — lightweight view subclass that acts as a
// placeholder for template-expanded or directly-assigned content.
// Mirrors MAUI's `ContentPresenter`:
//
//   * `set_content(view*)` / `content()` — non-owning pointer to the
//     presented child (lifetime managed by the caller, matching the
//     pattern in `basic_content_view` where `content` is a
//     `shared_ptr<view>` owned by the parent).
//
//   * `apply_template(const control_template&)` — calls
//     `control_template::instantiate()`, stores the resulting
//     `unique_ptr<view>` (taking ownership), and exposes the raw pointer
//     via `templated_child()`.  If the template has no factory,
//     `templated_child()` becomes nullptr.
//
// This header is intentionally surface-only (no handler wiring) and
// header-only, keeping build-system impact to zero.

#ifndef MPAPP_TEMPLATES_CONTENT_PRESENTER_HPP
#define MPAPP_TEMPLATES_CONTENT_PRESENTER_HPP

#include <memory>

#include "../view.hpp"              // mpapp::view
#include "control_template.hpp"     // mpapp::control_template

namespace mpapp {

class content_presenter : public view {
public:
    content_presenter()  = default;
    ~content_presenter() = default;

    content_presenter(const content_presenter&)            = delete;
    content_presenter& operator=(const content_presenter&) = delete;
    content_presenter(content_presenter&&)                 = delete;
    content_presenter& operator=(content_presenter&&)      = delete;

    // ----- Presented content (non-owning) ----------------------------------

    // Set the presented child.  The presenter does NOT own the child; the
    // caller is responsible for the child's lifetime.  Passing nullptr
    // clears the presented content.
    void set_content(view* child) noexcept { content_ = child; }

    // Returns the currently presented child, or nullptr if none is set.
    [[nodiscard]] view* content() const noexcept { return content_; }

    // ----- Template expansion ----------------------------------------------

    // Instantiate `ct` and store the result.  Ownership of the new view is
    // taken by this presenter.  Any previously-instantiated child is dropped.
    // If `ct` has no factory, `templated_child()` is set to nullptr.
    void apply_template(const control_template& ct) {
        templated_child_ = ct.instantiate();
    }

    // Returns the view that was produced by the most recent `apply_template`
    // call, or nullptr if `apply_template` was never called (or the last
    // factory was empty).
    [[nodiscard]] view* templated_child() const noexcept {
        return templated_child_.get();
    }

private:
    view*                  content_         = nullptr;
    std::unique_ptr<view>  templated_child_{};
};

} // namespace mpapp

#endif // MPAPP_TEMPLATES_CONTENT_PRESENTER_HPP
