// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0021-tableview-cell-types.md
//
// `mpapp::image_cell` — text_cell + an image. Most commonly used for
// list rows with leading icons (settings menus, contact lists). Inherits
// the text + detail surface from text_cell and adds image_uri.

#ifndef MPAPP_INTERNAL_BASIC_IMAGE_CELL_HPP
#define MPAPP_INTERNAL_BASIC_IMAGE_CELL_HPP

#include <string>

#include "../observable.hpp"
#include "../platform.hpp"
#include "basic_text_cell.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class image_cell_handler;

class basic_image_cell : public internal::basic_text_cell {
public:
    basic_image_cell() = default;
    ~basic_image_cell() override = default;

    basic_image_cell(const basic_image_cell&)            = delete;
    basic_image_cell& operator=(const basic_image_cell&) = delete;
    basic_image_cell(basic_image_cell&&)                 = delete;
    basic_image_cell& operator=(basic_image_cell&&)      = delete;

    Observable<std::string> image_uri{""};

    image_cell_handler<platform::current>&       ic_handler() noexcept       { return *ic_handler_; }
    const image_cell_handler<platform::current>& ic_handler() const noexcept { return *ic_handler_; }
    bool                                         has_ic_handler() const noexcept { return ic_handler_ != nullptr; }
    void                                         set_ic_handler(image_cell_handler<platform::current>& h) noexcept { ic_handler_ = &h; }

private:
    image_cell_handler<platform::current>* ic_handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_IMAGE_CELL_HPP
