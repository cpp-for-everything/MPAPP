// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// Diagnostic collection for the mpapp-xc XAML compiler.
//
// A `diagnostic` records a single error or warning at a specific location in
// the input XAML. The `diagnostic_collector` aggregates them so the CLI can
// emit them all at the end of a compilation run rather than aborting on the
// first failure -- this matches the diagnostic-quality commitment from
// vault/10_Architecture/Markup.md.

#pragma once

#include <cstddef>
#include <filesystem>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace mpapp::xc {

enum class severity {
    error,
    warning,
    note,
};

struct diagnostic {
    severity level{severity::error};
    std::filesystem::path source_path{};
    std::ptrdiff_t line{0};   // 1-based; 0 if unknown
    std::ptrdiff_t column{0}; // 1-based; 0 if unknown
    std::string message{};
};

class diagnostic_collector {
public:
    void add(diagnostic d) { entries_.push_back(std::move(d)); }

    void error(std::filesystem::path path,
               std::ptrdiff_t line,
               std::ptrdiff_t column,
               std::string message) {
        entries_.push_back(
            diagnostic{severity::error, std::move(path), line, column, std::move(message)});
    }

    void warning(std::filesystem::path path,
                 std::ptrdiff_t line,
                 std::ptrdiff_t column,
                 std::string message) {
        entries_.push_back(
            diagnostic{severity::warning, std::move(path), line, column, std::move(message)});
    }

    [[nodiscard]] const std::vector<diagnostic>& entries() const noexcept { return entries_; }

    [[nodiscard]] bool has_errors() const noexcept {
        for (const auto& d : entries_) {
            if (d.level == severity::error) return true;
        }
        return false;
    }

    [[nodiscard]] bool empty() const noexcept { return entries_.empty(); }

    void clear() noexcept { entries_.clear(); }

private:
    std::vector<diagnostic> entries_{};
};

inline std::string_view to_string(severity s) noexcept {
    switch (s) {
        case severity::error: return "error";
        case severity::warning: return "warning";
        case severity::note: return "note";
    }
    return "error";
}

inline void write(std::ostream& os, const diagnostic& d) {
    os << d.source_path.string();
    if (d.line > 0) {
        os << ':' << d.line;
        if (d.column > 0) os << ':' << d.column;
    }
    os << ": " << to_string(d.level) << ": " << d.message;
}

} // namespace mpapp::xc
