// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0021-tableview-cell-types.md
//
// `mpapp::image_cell` — text_cell + an image. Most commonly used for
// list rows with leading icons (settings menus, contact lists). Inherits
// the text + detail surface from text_cell and adds image_uri.

#ifndef MPAPP_IMAGE_CELL_HPP
#define MPAPP_IMAGE_CELL_HPP

#include <string>

#include "observable.hpp"
#include "platform.hpp"
#include "text_cell.hpp"

namespace mpapp {

template <class Platform = platform::current>
class image_cell_handler;

class image_cell : public text_cell {
public:
    image_cell() = default;
    ~image_cell() override = default;

    image_cell(const image_cell&)            = delete;
    image_cell& operator=(const image_cell&) = delete;
    image_cell(image_cell&&)                 = delete;
    image_cell& operator=(image_cell&&)      = delete;

    Observable<std::string> image_uri{""};

    image_cell_handler<platform::current>&       ic_handler() noexcept       { return *ic_handler_; }
    const image_cell_handler<platform::current>& ic_handler() const noexcept { return *ic_handler_; }
    bool                                         has_ic_handler() const noexcept { return ic_handler_ != nullptr; }
    void                                         set_ic_handler(image_cell_handler<platform::current>& h) noexcept { ic_handler_ = &h; }

private:
    image_cell_handler<platform::current>* ic_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_IMAGE_CELL_HPP
