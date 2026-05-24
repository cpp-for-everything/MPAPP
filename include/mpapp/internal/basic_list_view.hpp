// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ListView.md
//
// `mpapp::list_view` — legacy MAUI virtualized item host. Modern MAUI
// code uses [[CollectionView]]; ListView ships for source-compat parity.
// Mock surface keeps items as a flat `std::vector<std::string>` (real
// item_template handling is deferred to the virtualized-item-host ADR).

#ifndef MPAPP_INTERNAL_BASIC_LIST_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_LIST_VIEW_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class list_view_handler;

class basic_list_view : public view {
public:
    basic_list_view() = default;
    ~basic_list_view() override = default;

    basic_list_view(const basic_list_view&)            = delete;
    basic_list_view& operator=(const basic_list_view&) = delete;
    basic_list_view(basic_list_view&&)                 = delete;
    basic_list_view& operator=(basic_list_view&&)      = delete;

    // ----- Surface ------------------------------------------------------

    Observable<std::vector<std::string>> items_source{};
    Observable<int>                      selected_index{-1};   // -1 = none
    Observable<int>                      row_height{-1};       // -1 = native default
    Observable<bool>                     has_unevenly_sized_rows{false};

    // ----- Events -------------------------------------------------------

    signal<int>                          item_tapped{};        // emits tapped row index

    // ----- Handler ------------------------------------------------------

    list_view_handler<platform::current>&       lv_handler() noexcept       { return *lv_handler_; }
    const list_view_handler<platform::current>& lv_handler() const noexcept { return *lv_handler_; }
    bool                                        has_lv_handler() const noexcept { return lv_handler_ != nullptr; }
    void                                        set_lv_handler(list_view_handler<platform::current>& h) noexcept { lv_handler_ = &h; }

private:
    list_view_handler<platform::current>* lv_handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_LIST_VIEW_HPP
