// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Editor.md
//
// `mpapp::editor` — multi-line text input. Mock surface mirrors Entry
// minus `is_password` and adds no other primitive slots.

#ifndef MPAPP_EDITOR_HPP
#define MPAPP_EDITOR_HPP

#include <string>

#include "command.hpp"
#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform = platform::current>
class editor_handler;

class editor : public control<editor> {
public:
    editor() = default;

    editor(const editor&)            = delete;
    editor& operator=(const editor&) = delete;
    editor(editor&&)                 = delete;
    editor& operator=(editor&&)      = delete;

    Observable<std::string> text{""};
    Observable<std::string> placeholder{""};
    Observable<bool>        is_read_only{false};
    Observable<int>         max_length{-1};
    Observable<int>         cursor_position{0};

    void completed(Command<> = {}) {}
    void text_changed(std::string, Command<std::string> = {}) {}

    editor_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const editor_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(editor_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    editor_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_EDITOR_HPP
