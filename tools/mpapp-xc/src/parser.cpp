// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.

#include "mpapp_xc/parser.hpp"

#include <pugixml.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace mpapp::xc {

namespace {

// Compute the (1-based) line number of a byte offset in the source buffer.
std::ptrdiff_t line_at(std::string_view source, std::ptrdiff_t offset) noexcept {
    if (offset < 0) return 0;
    const auto end = static_cast<std::size_t>(
        std::min<std::ptrdiff_t>(offset, static_cast<std::ptrdiff_t>(source.size())));
    return static_cast<std::ptrdiff_t>(
        1 + std::count(source.begin(), source.begin() + static_cast<std::ptrdiff_t>(end), '\n'));
}

xaml_element copy_element(const pugi::xml_node& node) {
    xaml_element out{};
    out.tag = node.name();
    for (const auto& attr : node.attributes()) {
        out.attributes.push_back({attr.name(), attr.value()});
    }
    for (const auto& child : node.children()) {
        if (child.type() == pugi::node_element) {
            out.children.push_back(copy_element(child));
        }
    }
    return out;
}

} // namespace

xaml_document parse(std::string_view source,
                    std::filesystem::path source_path,
                    diagnostic_collector& diagnostics) {
    xaml_document doc{};
    doc.source_path = source_path;

    pugi::xml_document xml{};
    const auto result =
        xml.load_buffer(source.data(), source.size(), pugi::parse_default, pugi::encoding_utf8);

    if (!result) {
        diagnostics.error(source_path,
                          line_at(source, static_cast<std::ptrdiff_t>(result.offset)),
                          0,
                          std::string{"XML parse error: "} + result.description());
        return doc;
    }

    const auto root = xml.document_element();
    if (!root) {
        diagnostics.error(source_path, 0, 0, "XAML file has no root element");
        return doc;
    }

    doc.has_root = true;
    doc.root = copy_element(root);

    const auto root_offset = root.offset_debug();
    if (root_offset >= 0) {
        doc.root_line = line_at(source, static_cast<std::ptrdiff_t>(root_offset));
    }

    for (const auto& attr : root.attributes()) {
        const std::string_view name{attr.name()};
        const std::string_view value{attr.value()};
        if (name == "xmlns") {
            doc.default_xmlns = std::string{value};
        } else if (name.starts_with("xmlns:")) {
            const auto prefix = name.substr(6);
            if (prefix == "x") doc.x_namespace = std::string{value};
        } else if (name == "x:Class") {
            doc.x_class = std::string{value};
        }
    }

    return doc;
}

std::string class_basename(std::string_view x_class) noexcept {
    if (x_class.empty()) return {};
    const auto last_dot = x_class.find_last_of('.');
    const auto base =
        last_dot == std::string_view::npos ? x_class : x_class.substr(last_dot + 1);
    return std::string{base};
}

} // namespace mpapp::xc
