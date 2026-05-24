// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Picker.md
//
// `mpapp::picker` — dropdown / single-selection from a list of strings.
// The cross-platform surface keeps the items as `std::vector<std::string>`
// for now; richer item-template binding lands when the binding-layer work
// in M-04 wraps the data-template surface.

#ifndef MPAPP_INTERNAL_BASIC_PICKER_HPP
#define MPAPP_INTERNAL_BASIC_PICKER_HPP

#include <string>
#include <vector>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class picker_handler;

class basic_picker : public view {
public:
    basic_picker() = default;

    Observable<std::vector<std::string>> items{};
    Observable<int>                      selected_index{-1};   // -1 = nothing selected
    Observable<std::string>              title{};              // shown in the popup header on some platforms

    picker_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const picker_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                     has_handler() const noexcept { return handler_ != nullptr; }
    void                                     set_handler(picker_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    picker_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_PICKER_HPP
