// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Editor.md
//
// `mpapp::editor` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_editor` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::editor x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_editor x;
//     mpapp::editor_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_EDITOR_HPP
#define MPAPP_EDITOR_HPP

#include "internal/basic_editor.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_editor` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/editor_handler.hpp"

namespace mpapp {

class editor : public internal::basic_editor {
public:
    editor() {
        set_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_is_read_only(*this);
    }

    editor(const editor&)            = delete;
    editor& operator=(const editor&) = delete;
    editor(editor&&)                 = delete;
    editor& operator=(editor&&)      = delete;

private:
    internal::editor_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::editor_handler<>` (host-current) and
// `mpapp::editor_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using editor_handler = internal::editor_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_EDITOR_HPP
