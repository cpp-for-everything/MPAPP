// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// mpapp-xc CLI entry point.
//
// Usage:
//   mpapp-xc <input.xaml> [--out <output.gen.hpp>]
//
// Exit codes:
//   0 — success
//   1 — XAML parse / compile error (diagnostics emitted to stderr)
//   2 — invalid usage / missing input file

#include "mpapp_xc/diagnostics.hpp"
#include "mpapp_xc/emitter.hpp"
#include "mpapp_xc/parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr int exit_success = 0;
constexpr int exit_compile_error = 1;
constexpr int exit_usage_error = 2;

void print_usage(std::ostream& os) {
    os << "Usage: mpapp-xc <input.xaml> [--out <output.gen.hpp>]\n";
}

struct cli_args {
    std::filesystem::path input{};
    std::filesystem::path output{}; // empty -> stdout
};

bool parse_args(int argc, char** argv, cli_args& out, std::ostream& err) {
    std::vector<std::string_view> positional{};
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if (arg == "--out" || arg == "-o") {
            if (i + 1 >= argc) {
                err << "mpapp-xc: error: " << arg << " requires a value\n";
                return false;
            }
            out.output = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            print_usage(std::cout);
            std::exit(exit_success);
        } else if (!arg.empty() && arg.front() == '-') {
            err << "mpapp-xc: error: unknown option '" << arg << "'\n";
            return false;
        } else {
            positional.push_back(arg);
        }
    }

    if (positional.size() != 1) {
        err << "mpapp-xc: error: expected exactly one input file\n";
        return false;
    }
    out.input = std::filesystem::path{positional.front()};
    return true;
}

bool read_file(const std::filesystem::path& path,
               std::string& out,
               std::ostream& err) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        err << "mpapp-xc: error: cannot open input file '" << path.string() << "'\n";
        return false;
    }
    std::ostringstream buffer{};
    buffer << stream.rdbuf();
    out = buffer.str();
    return true;
}

bool write_file(const std::filesystem::path& path,
                std::string_view contents,
                std::ostream& err) {
    if (path.has_parent_path()) {
        std::error_code ec{};
        std::filesystem::create_directories(path.parent_path(), ec);
        // Non-fatal: writing will fail with its own message if the dir is
        // missing.
    }
    std::ofstream stream{path, std::ios::binary};
    if (!stream) {
        err << "mpapp-xc: error: cannot open output file '" << path.string() << "'\n";
        return false;
    }
    stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    return static_cast<bool>(stream);
}

} // namespace

int main(int argc, char** argv) {
    cli_args args{};
    if (!parse_args(argc, argv, args, std::cerr)) {
        print_usage(std::cerr);
        return exit_usage_error;
    }

    std::string source{};
    if (!read_file(args.input, source, std::cerr)) {
        return exit_usage_error;
    }

    mpapp::xc::diagnostic_collector diagnostics{};
    const auto doc = mpapp::xc::parse(source, args.input, diagnostics);

    for (const auto& d : diagnostics.entries()) {
        mpapp::xc::write(std::cerr, d);
        std::cerr << '\n';
    }
    if (diagnostics.has_errors()) {
        return exit_compile_error;
    }

    const auto generated = mpapp::xc::emit(doc);

    if (args.output.empty()) {
        std::cout << generated;
    } else if (!write_file(args.output, generated, std::cerr)) {
        return exit_compile_error;
    }

    return exit_success;
}
