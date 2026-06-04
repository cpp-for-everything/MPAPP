// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// XAML -> C++ emitter. Recursively lowers the parsed `xaml_document` tree into
// the body of an `auto build_<Class>()` function in `namespace
// mpapp::generated`. Each supported element becomes a heap-allocated
// `std::shared_ptr<mpapp::internal::basic_*>` local; attributes become member
// assignments; children are wired to their parent by the parent's
// `child_kind` (layout -> `parent->add(*child)`, single-content / page ->
// `parent->content = child`).
//
// shared_ptr is used (rather than stack locals) because every mpapp widget is
// non-copyable AND non-movable, and the single-content / page content slots
// are `Observable<std::shared_ptr<view>>`; a shared_ptr<derived> binds to all
// of `view&` (via `*ptr`) and `shared_ptr<view>` uniformly, so one strategy
// type-checks for layouts, decorators, and pages alike.
//
// The generated header includes the precise cross-platform `internal/basic_*`
// surface headers the tree uses. Those are the real mpapp public API (per
// ADR-0008 — the same bases the `mpapp::label` wrappers derive from) and
// compile standalone on any host; the native-handler-embedding wrappers would
// otherwise drag in the per-OS SDK (e.g. winrt on Windows). The umbrella
// <mpapp/mpapp.hpp> is referenced in a comment for traceability.
//
// Each emitted statement carries a `#line` directive pointing back at the
// originating XAML line so downstream C++ compile errors surface at the XAML
// source location (see vault/10_Architecture/Markup.md). Unknown tags /
// attributes degrade to a `// TODO` comment rather than failing the file.

#include "mpapp_xc/emitter.hpp"
#include "mpapp_xc/type_registry.hpp"

#include <cstddef>
#include <set>
#include <sstream>
#include <string>

namespace mpapp::xc {

namespace {

// Fallback used when the parser couldn't determine where an element opened.
// Matches the typical "XML decl on line 1, root on line 2" XAML shape.
constexpr std::ptrdiff_t root_line_fallback = 2;

std::string source_filename_for(const xaml_document& doc, const emit_options& options) {
    if (!options.source_filename.empty()) return options.source_filename;
    if (doc.source_path.has_filename()) return doc.source_path.filename().string();
    return doc.source_path.string();
}

// Mutable state threaded through the recursive lowering.
struct emit_context {
    std::ostringstream& os;
    std::string source_filename;
    std::set<std::string>& headers; // distinct includes the tree needs
    std::size_t next_id{0};

    std::ptrdiff_t line_for(std::ptrdiff_t element_line) const noexcept {
        return element_line > 0 ? element_line : root_line_fallback;
    }

    void line_directive(std::ptrdiff_t element_line) {
        os << "#line " << line_for(element_line) << " \"" << source_filename << "\"\n";
    }
};

// Lower one element. Returns the variable name bound to it (e.g. "e0"), or an
// empty string if the element's tag is unknown (in which case a `// TODO`
// comment has already been emitted and no variable exists).
std::string lower_element(emit_context& ctx, const xaml_element& el, int indent) {
    const std::string pad(static_cast<std::size_t>(indent) * 4, ' ');
    const auto info = lookup_element(el.tag);

    if (!info.has_value()) {
        ctx.os << pad << "// TODO(mpapp-xc): unsupported element <" << el.tag << ">\n";
        return {};
    }

    ctx.headers.insert(std::string{info->header});
    const std::string var = "e" + std::to_string(ctx.next_id++);

    ctx.line_directive(el.line);
    ctx.os << pad << "auto " << var << " = std::make_shared<" << info->cpp_type
           << ">();\n";

    // HorizontalStackLayout has no distinct C++ type; encode its orientation.
    if (el.tag == "HorizontalStackLayout") {
        ctx.line_directive(el.line);
        ctx.os << pad << var << "->stack_orientation = mpapp::orientation::horizontal;\n";
    }

    for (const auto& attr : el.attributes) {
        const auto setter = lookup_attribute(el.tag, attr.name);
        if (!setter.has_value()) {
            // Silently skip the structural directives; comment on real ones.
            if (attr.name == "x:Class" || attr.name == "xmlns" ||
                attr.name.starts_with("xmlns:")) {
                continue;
            }
            ctx.os << pad << "// TODO(mpapp-xc): unsupported attribute " << attr.name
                   << "=\"" << attr.value << "\" on <" << el.tag << ">\n";
            continue;
        }
        ctx.line_directive(el.line);
        ctx.os << pad << var << "->" << setter->member << " = "
               << lower_value(setter->kind, attr.value) << ";\n";
    }

    // Recurse into children and wire each to this element per its child_kind.
    for (const auto& child : el.children) {
        const std::string child_var = lower_element(ctx, child, indent);
        if (child_var.empty()) continue; // unknown child already TODO'd
        switch (info->kind) {
            case child_kind::layout:
                ctx.line_directive(child.line);
                ctx.os << pad << var << "->add(*" << child_var << ");\n";
                break;
            case child_kind::single_content:
            case child_kind::page:
                ctx.line_directive(child.line);
                ctx.os << pad << var << "->content = " << child_var << ";\n";
                break;
            case child_kind::leaf:
                ctx.os << pad << "// TODO(mpapp-xc): <" << el.tag
                       << "> does not accept child <" << child.tag << ">\n";
                break;
        }
    }

    return var;
}

} // namespace

std::string emit(const xaml_document& doc, const emit_options& options) {
    const std::string source_filename = source_filename_for(doc, options);
    const std::string input_display =
        doc.source_path.empty() ? source_filename : doc.source_path.generic_string();

    const std::string fn_name = [&] {
        const auto base = doc.x_class.has_value() ? class_basename(*doc.x_class) : std::string{};
        return base.empty() ? std::string{"anonymous"} : base;
    }();

    const std::string root_tag = doc.has_root && !doc.root.tag.empty() ? doc.root.tag : "unknown";

    // Lower the body into a buffer first so we know exactly which mpapp
    // headers the tree touches, then emit a deduplicated include block.
    std::ostringstream body;
    std::set<std::string> headers;
    emit_context ctx{body, source_filename, headers};

    std::string tail; // the `return ...;` line
    const bool root_supported = doc.has_root && lookup_element(doc.root.tag).has_value();
    if (!root_supported) {
        // Preserve the empty / unsupported-root behavior: a compilable stub.
        body << "#line " << ctx.line_for(doc.root_line) << " \"" << source_filename << "\"\n";
        body << "    // TODO(mpapp-xc): empty or unsupported root <" << root_tag << ">\n";
        tail = "    return std::shared_ptr<mpapp::view>{};\n";
        headers.insert("mpapp/view.hpp");
    } else {
        const std::string root_var = lower_element(ctx, doc.root, 1);
        tail = "    return " + root_var + ";\n";
    }

    std::ostringstream os;
    os << "// Auto-generated by mpapp-xc from " << input_display << '\n';
    os << "// DO NOT EDIT.\n";
    os << '\n';
    os << "#pragma once\n";
    os << '\n';
    os << "// Real mpapp API (umbrella: <mpapp/mpapp.hpp>). The precise\n";
    os << "// cross-platform surface headers below keep this generated unit\n";
    os << "// host-compilable without the per-platform native SDK.\n";
    for (const auto& header : headers) {
        os << "#include <" << header << ">\n";
    }
    os << '\n';
    os << "#include <memory>\n";
    os << '\n';
    os << "namespace mpapp::generated {\n";
    os << '\n';
    os << "// Builds the <" << root_tag << "> visual tree and returns its root.\n";
    os << "inline auto build_" << fn_name << "() {\n";
    os << body.str();
    os << tail;
    os << "}\n";
    os << '\n';
    os << "} // namespace mpapp::generated\n";

    return os.str();
}

} // namespace mpapp::xc
