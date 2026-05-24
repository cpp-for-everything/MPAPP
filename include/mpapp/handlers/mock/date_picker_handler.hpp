// SPDX-License-Identifier: Apache-2.0
// Mock basic_date_picker handler.

#ifndef MPAPP_HANDLERS_MOCK_DATE_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_DATE_PICKER_HANDLER_HPP

#include <string>

#include "../../internal/basic_date_picker.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class date_picker_handler<platform::mock>
    : public mock_handler_base {
public:
    date_picker_handler() = default;

    void map_date(basic_date_picker& p)   { bind("date",   p.date,   binding_date_); }
    void map_format(basic_date_picker& p) { bind("format", p.format, binding_format_); }

private:
    detail::property_binding<date_value>   binding_date_{};
    detail::property_binding<std::string>  binding_format_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_DATE_PICKER_HANDLER_HPP
