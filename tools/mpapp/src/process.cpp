// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// Cross-platform `spawn` + `which` for the mpapp CLI.

#include "process.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#if defined(_WIN32)
#  include <process.h>
#  include <io.h>
#else
#  include <sys/wait.h>
#  include <unistd.h>
#endif

namespace mpapp::cli::process {

namespace {

// Read everything from `stream` into `out` until EOF. Closes the stream.
void drain(std::FILE* stream, std::string& out) {
    std::array<char, 4096> buf{};
    while (auto n = std::fread(buf.data(), 1, buf.size(), stream)) {
        out.append(buf.data(), n);
    }
}

#if defined(_WIN32)

// Windows quoting: wrap the argument in double-quotes and escape embedded
// quotes/backslashes per the CRT rules. Sufficient for cmake/mpapp-xc.
std::string quote_arg(std::string_view arg) {
    if (!arg.empty() &&
        arg.find_first_of(" \t\"") == std::string_view::npos) {
        return std::string{arg};
    }
    std::string out;
    out.reserve(arg.size() + 2);
    out.push_back('"');
    std::size_t backslashes = 0;
    for (char c : arg) {
        if (c == '\\') {
            ++backslashes;
            out.push_back(c);
        } else if (c == '"') {
            out.append(backslashes + 1, '\\');
            out.push_back('"');
            backslashes = 0;
        } else {
            backslashes = 0;
            out.push_back(c);
        }
    }
    out.append(backslashes, '\\');
    out.push_back('"');
    return out;
}

std::string join_command(const std::vector<std::string>& args) {
    std::string cmd;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i) cmd.push_back(' ');
        cmd += quote_arg(args[i]);
    }
    return cmd;
}

#endif // _WIN32

} // namespace

int spawn(const std::vector<std::string>& args, std::string* output) {
    if (args.empty()) {
        return -1;
    }

#if defined(_WIN32)
    if (output) {
        const std::string cmd = join_command(args);
        // `_popen` on Windows returns combined stdout; stderr is inherited.
        // To capture both we append `2>&1` via `cmd.exe /c`.
        const std::string wrapped = "cmd.exe /c \"" + cmd + " 2>&1\"";
        std::FILE* pipe = _popen(wrapped.c_str(), "r");
        if (!pipe) {
            return -1;
        }
        drain(pipe, *output);
        const int rc = _pclose(pipe);
        return rc;
    }

    std::vector<const char*> c_args;
    c_args.reserve(args.size() + 1);
    for (const auto& a : args) c_args.push_back(a.c_str());
    c_args.push_back(nullptr);
    const intptr_t rc = _spawnvp(_P_WAIT, c_args[0],
                                 const_cast<char* const*>(c_args.data()));
    if (rc < 0) return -1;
    return static_cast<int>(rc);
#else
    int pipefd[2] = {-1, -1};
    if (output && ::pipe(pipefd) != 0) {
        return -1;
    }

    const pid_t pid = ::fork();
    if (pid < 0) {
        if (output) {
            ::close(pipefd[0]);
            ::close(pipefd[1]);
        }
        return -1;
    }

    if (pid == 0) {
        // Child.
        if (output) {
            ::dup2(pipefd[1], 1);
            ::dup2(pipefd[1], 2);
            ::close(pipefd[0]);
            ::close(pipefd[1]);
        }
        std::vector<char*> c_args;
        c_args.reserve(args.size() + 1);
        for (const auto& a : args) {
            c_args.push_back(const_cast<char*>(a.c_str()));
        }
        c_args.push_back(nullptr);
        ::execvp(c_args[0], c_args.data());
        // exec failed.
        _exit(127);
    }

    // Parent.
    if (output) {
        ::close(pipefd[1]);
        std::FILE* f = ::fdopen(pipefd[0], "r");
        if (f) {
            drain(f, *output);
            std::fclose(f);
        } else {
            ::close(pipefd[0]);
        }
    }

    int status = 0;
    while (::waitpid(pid, &status, 0) < 0) {
        // EINTR; retry.
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
#endif
}

std::string which(std::string_view name) {
    namespace fs = std::filesystem;

#if defined(_WIN32)
    char* path_buf = nullptr;
    std::size_t path_len = 0;
    if (_dupenv_s(&path_buf, &path_len, "PATH") != 0 || !path_buf) {
        if (path_buf) std::free(path_buf);
        return {};
    }
    std::string path_env_storage{path_buf};
    std::free(path_buf);
    const char* path_env = path_env_storage.c_str();
#else
    const char* path_env = std::getenv("PATH");
    if (!path_env) {
        return {};
    }
#endif

#if defined(_WIN32)
    constexpr char sep = ';';
    const bool has_ext = name.size() >= 4 &&
        (name.substr(name.size() - 4) == ".exe" ||
         name.substr(name.size() - 4) == ".EXE");
    const std::string candidate_name =
        has_ext ? std::string{name} : std::string{name} + ".exe";
#else
    constexpr char sep = ':';
    const std::string candidate_name{name};
#endif

    std::string_view rest{path_env};
    while (!rest.empty()) {
        const auto pos = rest.find(sep);
        std::string_view entry =
            pos == std::string_view::npos ? rest : rest.substr(0, pos);
        if (!entry.empty()) {
            fs::path p = fs::path{entry} / candidate_name;
            std::error_code ec;
            if (fs::exists(p, ec) && !fs::is_directory(p, ec)) {
                return p.string();
            }
        }
        if (pos == std::string_view::npos) break;
        rest.remove_prefix(pos + 1);
    }
    return {};
}

} // namespace mpapp::cli::process
