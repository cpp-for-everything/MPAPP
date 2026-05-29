// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Picker.md
//
// `mpapp::picker` — dropdown / single-selection from a list of strings.
// The cross-platform surface keeps the items as `std::vector<std::string>`
// for now; richer item-template binding lands when the binding-layer work
// in M-04 wraps the data-template surface.

#ifndef MPAPP_INTERNAL_BASIC_PICKER_HPP
#define MPAPP_INTERNAL_BASIC_PICKER_HPP

#include <memory>
#include <string>
#include <vector>

#include "../binding/relay_command.hpp"   // RFC-0014 command_base (ICommand)
#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class picker_handler;

class basic_picker : public view {
public:
    basic_picker() {
        // RFC-0014: control-side command. When the selection changes the
        // bound ICommand is executed (self-gated). The command body reads
        // `selected_index` for the chosen row. Mirrors basic_button.command.
        selected_index.changed.subscribe(command_slot_, command_cb_);
    }

    basic_picker(const basic_picker&)            = delete;
    basic_picker& operator=(const basic_picker&) = delete;
    basic_picker(basic_picker&&)                 = delete;
    basic_picker& operator=(basic_picker&&)      = delete;

    Observable<std::vector<std::string>> items{};
    Observable<int>                      selected_index{-1};   // -1 = nothing selected
    Observable<std::string>              title{};              // shown in the popup header on some platforms

    // ICommand executed on selection change (RFC-0014).
    Observable<std::shared_ptr<command_base>> command{};

    picker_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const picker_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                     has_handler() const noexcept { return handler_ != nullptr; }
    void                                     set_handler(picker_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    struct command_invoke_cb_t {
        basic_picker* self;
        void operator()(int) const {
            if (auto c = self->command.get()) c->execute();
        }
    };
    command_invoke_cb_t       command_cb_{this};
    signal_slot<const int&>   command_slot_{};

    picker_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_PICKER_HPP
