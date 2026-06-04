// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Templates.md
//
// `mpapp::data_template_selector` — abstract base that selects a
// `data_template` based on the item and the container view.  Mirrors
// MAUI's `DataTemplateSelector`: the protected `on_select_template`
// virtual is overridden by concrete selectors; the public
// `select_template` non-virtual wrapper calls it, following the
// non-virtual interface (NVI) idiom used throughout MPAPP.
//
// Usage:
//   struct my_selector : mpapp::data_template_selector {
//     const mpapp::data_template*
//     on_select_template(const std::string& item,
//                        const mpapp::view* container) const override { ... }
//   };
//   auto* tpl = selector.select_template(item, &host);

#ifndef MPAPP_TEMPLATES_DATA_TEMPLATE_SELECTOR_HPP
#define MPAPP_TEMPLATES_DATA_TEMPLATE_SELECTOR_HPP

#include <string>

#include "../bindable_layout.hpp"   // mpapp::data_template
#include "../view.hpp"              // mpapp::view

namespace mpapp {

// Abstract selector.  Derive and override `on_select_template`.
// The returned pointer is non-owning; lifetime must exceed the call-site.
class data_template_selector {
public:
    data_template_selector()                                          = default;
    virtual ~data_template_selector()                                 = default;

    data_template_selector(const data_template_selector&)            = delete;
    data_template_selector& operator=(const data_template_selector&) = delete;
    data_template_selector(data_template_selector&&)                 = delete;
    data_template_selector& operator=(data_template_selector&&)      = delete;

    // NVI wrapper — calls on_select_template.
    [[nodiscard]] const data_template*
    select_template(const std::string& item, const view* container) const {
        return on_select_template(item, container);
    }

protected:
    // Override to provide selection logic.  Return nullptr when no template
    // applies (equivalent to MAUI returning null from SelectTemplate).
    [[nodiscard]] virtual const data_template*
    on_select_template(const std::string& item,
                       const view*        container) const = 0;
};

} // namespace mpapp

#endif // MPAPP_TEMPLATES_DATA_TEMPLATE_SELECTOR_HPP
