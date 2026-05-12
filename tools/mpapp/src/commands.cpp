// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// mpapp CLI subcommand stubs + the registry + the top-level dispatcher.
//
// Each subcommand's run() parses its own arguments with a tiny ad-hoc
// parser, prints a "not yet implemented" notice echoing what it recognized,
// and returns 0. The dispatcher itself handles routing, --version, --help,
// and the unknown-subcommand case.

#include "mpapp_cli/commands.hpp"
#include "mpapp_cli/dispatch.hpp"

#include <array>
#include <ostream>
#include <string>
#include <string_view>

namespace mpapp::cli {

namespace {

constexpr std::string_view kVersion = "mpapp 0.0.1";

// Look up a subcommand by name. Returns nullptr if not found.
const command* find_command(std::string_view name) {
    for (const auto& cmd : registry) {
        if (cmd.name == name) {
            return &cmd;
        }
    }
    return nullptr;
}

} // namespace

int print_version(std::ostream& out) {
    out << kVersion << '\n';
    return 0;
}

int print_help(std::string_view sub, std::ostream& out) {
    if (!sub.empty()) {
        if (const auto* cmd = find_command(sub)) {
            out << "usage: " << cmd->usage << '\n';
            out << "  " << cmd->summary << '\n';
            return 0;
        }
        out << "unknown command: " << sub << '\n';
        // Fall through and print the global help so the user can see what's
        // available.
    }

    out << "usage: mpapp <command> [<args>]\n";
    out << "\nCommands:\n";
    constexpr std::size_t column = 16;
    for (const auto& cmd : registry) {
        const std::size_t pad =
            cmd.name.size() < column ? column - cmd.name.size() : 1;
        out << "  " << cmd.name << std::string(pad, ' ') << cmd.summary << '\n';
    }
    out << "\nFlags:\n";
    out << "  --version       Print the mpapp version.\n";
    out << "  --help          Print this help.\n";
    return 0;
}

int dispatch(int argc, char** argv, std::ostream& out, std::ostream& err) {
    if (argc < 2) {
        return print_help({}, out);
    }

    const std::string_view first = argv[1];

    if (first == "--version" || first == "-V") {
        return print_version(out);
    }

    if (first == "--help" || first == "-h" || first == "help") {
        // `help <sub>` — per-subcommand help.
        std::string_view sub;
        if (argc >= 3) {
            sub = argv[2];
        }
        return print_help(sub, out);
    }

    if (const auto* cmd = find_command(first)) {
        // Hand the rest of argv to the subcommand. argv[0] becomes the
        // subcommand name; argv[1..argc-1] are its arguments.
        args_span sub_args{};
        sub_args.argc = argc - 1;
        sub_args.argv = argv + 1;
        return cmd->run(sub_args, out, err);
    }

    err << "unknown command: " << first << '\n';
    print_help({}, err);
    return 2;
}

namespace commands {

namespace {

// Returns true and advances `i` if argv[i] equals `flag`. Reads the value
// from argv[i+1] into `value`; reports the missing-value error to `err` and
// returns false if no value follows.
bool consume_value_flag(args_span args,
                        int& i,
                        std::string_view flag,
                        std::string& value,
                        std::ostream& err) {
    if (std::string_view{args.argv[i]} != flag) {
        return false;
    }
    if (i + 1 >= args.argc) {
        err << "missing value for " << flag << '\n';
        return false;
    }
    value = args.argv[i + 1];
    i += 1; // The outer loop bumps once more.
    return true;
}

} // namespace

int run_new(args_span args, std::ostream& out, std::ostream& err) {
    // Required positional: <name>.
    std::string name;
    for (int i = 1; i < args.argc; ++i) {
        std::string_view a = args.argv[i];
        if (a.starts_with('-')) {
            err << "unknown flag: " << a << '\n';
            return 2;
        }
        if (!name.empty()) {
            err << "unexpected argument: " << a << '\n';
            return 2;
        }
        name = a;
    }
    if (name.empty()) {
        err << "mpapp new: missing required <name>\n";
        err << "usage: mpapp new <name>\n";
        return 2;
    }
    out << "[mpapp new] not yet implemented (got: name=" << name << ")\n";
    return 0;
}

namespace {

// build/run/package all accept the same optional `--target <triple>` flag
// and stub out to a "[mpapp <name>] not yet implemented" line. Share the
// parser; the only thing that varies is the display name.
int run_target_only_stub(std::string_view name,
                         args_span args,
                         std::ostream& out,
                         std::ostream& err) {
    std::string target;
    for (int i = 1; i < args.argc; ++i) {
        std::string_view a = args.argv[i];
        if (a == "--target") {
            if (!consume_value_flag(args, i, "--target", target, err)) {
                return 2;
            }
            continue;
        }
        err << "unknown argument: " << a << '\n';
        return 2;
    }
    out << "[mpapp " << name << "] not yet implemented (got: target="
        << (target.empty() ? "<host>" : target) << ")\n";
    return 0;
}

} // namespace

int run_build(args_span args, std::ostream& out, std::ostream& err) {
    return run_target_only_stub("build", args, out, err);
}

int run_run(args_span args, std::ostream& out, std::ostream& err) {
    return run_target_only_stub("run", args, out, err);
}

int run_package(args_span args, std::ostream& out, std::ostream& err) {
    return run_target_only_stub("package", args, out, err);
}

int run_xaml_compile(args_span args, std::ostream& out, std::ostream& err) {
    std::string file;
    std::string out_path;
    for (int i = 1; i < args.argc; ++i) {
        std::string_view a = args.argv[i];
        if (a == "--out") {
            if (!consume_value_flag(args, i, "--out", out_path, err)) {
                return 2;
            }
            continue;
        }
        if (a.starts_with('-')) {
            err << "unknown flag: " << a << '\n';
            return 2;
        }
        if (!file.empty()) {
            err << "unexpected argument: " << a << '\n';
            return 2;
        }
        file = a;
    }
    if (file.empty()) {
        err << "mpapp xaml-compile: missing required <file>\n";
        err << "usage: mpapp xaml-compile <file> [--out <path>]\n";
        return 2;
    }
    out << "[mpapp xaml-compile] not yet implemented (got: file=" << file
        << ", out=" << (out_path.empty() ? "<auto>" : out_path) << ")\n";
    return 0;
}

int run_help(args_span args, std::ostream& out, std::ostream& /*err*/) {
    std::string_view sub;
    if (args.argc >= 2) {
        sub = args.argv[1];
    }
    return print_help(sub, out);
}

} // namespace commands

// Definition of the registry. The size in the array template argument must
// match the count of entries below; the declaration in dispatch.hpp pins
// the same size at compile time.
const std::array<command, 6> registry = {{
    {"new",
     "Create a new MPAPP app from a template.",
     "mpapp new <name>",
     &commands::run_new},
    {"build",
     "Build for the current host (or --target to cross-compile).",
     "mpapp build [--target <triple>]",
     &commands::run_build},
    {"run",
     "Build + launch on host (or emulator).",
     "mpapp run [--target <triple>]",
     &commands::run_run},
    {"package",
     "Produce a distributable artifact.",
     "mpapp package [--target <triple>]",
     &commands::run_package},
    {"xaml-compile",
     "Run mpapp-xc on a single file.",
     "mpapp xaml-compile <file> [--out <path>]",
     &commands::run_xaml_compile},
    {"help",
     "Print help (or per-subcommand help).",
     "mpapp help [<subcommand>]",
     &commands::run_help},
}};

} // namespace mpapp::cli
