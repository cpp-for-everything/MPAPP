// SPDX-License-Identifier: Apache-2.0
// `mpapp::content_view` — single-child container; equivalent to a
// Border with no stroke. The point is to host an interchangeable child
// via Observable<shared_ptr<view>>.

#ifndef MPAPP_CONTENT_VIEW_HPP
#define MPAPP_CONTENT_VIEW_HPP

#include <memory>

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class content_view_handler;

class content_view : public view {
public:
    content_view() = default;

    Observable<std::shared_ptr<view>>   content{};

    content_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const content_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                           has_handler() const noexcept { return handler_ != nullptr; }
    void                                           set_handler(content_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    content_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_CONTENT_VIEW_HPP
