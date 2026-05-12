# Self-Hosted Runner Setup

This note documents how to register the user's Windows machine as a self-hosted GitHub Actions runner for the MPAPP repository, the security boundaries we enforce, and the software the runner needs. The future macOS runner setup is summarized at the bottom but deferred to [[M-07-macOS-Real]].

See [[CI Strategy]] for the budget rationale (CLAUDE rule 8) and which jobs target which runners.

## 1. Why self-hosted

GitHub-hosted runners are great but expensive on minutes when a job is long-running (Android emulator boots, repeated cross-builds) or needs hardware we don't get from a cloud SKU (specific GPU, persistent caches, attached devices). Per [[CI Strategy]] we route:

- `android-emulator` (every PR, async) onto `mpapp-windows-self`.
- `wslg-gtk4-smoke` (daily) onto `mpapp-windows-self`.
- All Apple builds onto `mpapp-macos-self` once it exists.

The cloud runners stay on `windows-latest`, `ubuntu-latest`, and `macos-latest` (release-only) for the budget-light jobs.

## 2. Registering the Windows runner

The runner is registered against the MPAPP repository (not the org, not as a public runner). Steps performed on the user's Windows machine, in an elevated PowerShell:

1. In GitHub, navigate to `Settings -> Actions -> Runners -> New self-hosted runner` and pick `Windows / x64`. Copy the configuration token GitHub generates (one-shot, ~1 hour TTL).
2. Create a dedicated working directory: `C:\actions-runner`. Do not nest it inside the MPAPP checkout — the runner manages its own workspace per job.
3. Download and extract the latest runner package (currently `actions-runner-win-x64-*.zip`) into `C:\actions-runner`.
4. From inside `C:\actions-runner`, run the configuration command. The label `mpapp-windows-self` is the contract every workflow keys against:

   ```powershell
   .\config.cmd `
     --url https://github.com/<owner>/MPAPP `
     --token <token-from-step-1> `
     --name mpapp-win-primary `
     --labels mpapp-windows-self `
     --work _work `
     --unattended
   ```

5. Install the runner as a Windows service so it survives reboots:

   ```powershell
   .\svc.cmd install
   .\svc.cmd start
   ```

6. Confirm the runner shows up as `Idle` in `Settings -> Actions -> Runners`. The label list must include `self-hosted` (added automatically) and `mpapp-windows-self`.

To rotate the runner (token expiry, machine swap), run `.\config.cmd remove --token <removal-token>` first; never delete the directory while the service is running.

## 3. Security boundaries

Self-hosted runners are dangerous by default — a malicious PR can run arbitrary code on the host. We harden this in three layers:

1. **Scope to a single repo.** The runner is registered at the repository level, not the organization. Other repos under the same account cannot pick it up.
2. **No fork PRs.** Every job that targets the self-hosted runner is guarded with `if: github.event.pull_request.head.repo.full_name == github.repository`. This means fork PRs (which can carry hostile workflow changes) cannot trigger jobs on the user's machine. Only PRs whose source branch lives in the MPAPP repo itself — i.e., branches the maintainer pushed — can run on `mpapp-windows-self`.
3. **Least privilege.** The runner service account is a standard user, not an administrator. The user account has access only to the runner's `_work` directory and the toolchains listed below. Secrets are scoped per-environment in GitHub and are never echoed in logs.

If GitHub ever adds repository-level required reviewers for self-hosted-runner workflows, enable that as a fourth layer. Until then, the fork-PR guard is the load-bearing check — do not remove it.

The `continue-on-error: true` flag on the `android-emulator` job in `pr.yml` exists so cloud-runner jobs aren't blocked while the self-hosted runner is offline or being reconfigured. Flip it off once the runner has been stable for a week.

## 4. Software the runner needs

The Windows self-hosted runner has the following installed before the runner service starts. Versions are pinned where a specific one is required by the build:

- **Git for Windows** (`git --version` must succeed in a fresh PowerShell). Submodule checkout uses `submodules: recursive` in workflows; this is wholly dependent on a working `git` on PATH.
- **Visual Studio 2022 Build Tools or full IDE**, with workloads:
  - "Desktop development with C++" (MSVC v143 toolchain, Windows 10/11 SDK, CMake tools).
  - "Game development with C++" is *not* required.
- **Android SDK + emulator**. Install via Android Studio's SDK Manager or `sdkmanager` CLI:
  - Platform-tools (adb, fastboot).
  - System images: `system-images;android-34;google_apis;arm64-v8a` (preferred) or `x86_64` if arm64 host emulation is unavailable.
  - Android Emulator. Confirm `emulator -list-avds` works.
  - The runner service account needs to be in the `Hyper-V Administrators` group for hardware-accelerated emulation (HAXM is deprecated; Windows Hypervisor Platform is the supported path).
- **Zig**. We do not pre-install Zig on the runner. The `mpapp` CLI downloads and pins the toolchain on first use (see [[zig-install]]). This keeps the version in lock-step with what cloud runners use.
- **Python 3.11+**. Required for the vault scripts (`py -c "..."` snippets in PRs, the conformance harness, and the markdown linters). Ensure `py` is on PATH via the official Python installer's "Add Python to PATH" option.
- **CMake 3.27+** (already pulled in by VS Build Tools — verify with `cmake --version`).

Do *not* install global C++ libraries or compilers other than MSVC on the runner — the goal is to reproduce a clean cloud environment plus the things only a local box can do (devices, emulators, persistent caches).

The runner's `_work/_tool/` is whitelisted in any antivirus exclusion list. Without that exclusion, on-access scans add minutes to every CMake configure.

## 5. Maintaining the runner

- **Health check.** Once a week, log in to the machine, run `Get-Service actions.runner.* | fl Status,StartType`. Status should be `Running`. If it isn't, restart with `Restart-Service` and check `_diag/Runner_*.log` for the cause.
- **Disk usage.** The runner does not garbage-collect `_work` aggressively. If free space drops below 20 GB, stop the service, delete `_work/<repo>/_*` directories that aren't tied to an active job, and restart.
- **Updates.** Runner self-updates on every job pickup as long as `disableupdate` is not set. We leave self-update on; no action required from the maintainer.

## 6. Future macOS runner

Setup for `mpapp-macos-self` is deferred to [[M-07-macOS-Real]]. At a high level, the same shape applies:

1. Register the user's MacBook Pro against the MPAPP repository with label `mpapp-macos-self`.
2. Install Xcode (full IDE, command-line tools alone are insufficient because we need iOS Simulator), Homebrew, CMake, and Python 3.11+.
3. Apple signing requirements mean macOS-hosted CI is the only path for signed Apple builds; cross-builds from non-Apple hosts can produce unsigned artifacts but not shippable ones.
4. The same fork-PR guard applies. macOS jobs in `release.yml` keep targeting `macos-latest` until the self-hosted runner has soaked for two weeks; then we flip the matrix to prefer `mpapp-macos-self` and use `macos-latest` only as a fallback when the self-hosted runner is offline.

Open items for the macOS runner are tracked under [[M-07-macOS-Real]] and [[T-0008-mac-ios-test-harness-design]].

## Links

- [[CI Strategy]]
- [[CLAUDE]] rule 8
- [[M-07-macOS-Real]]
- [[T-0008-mac-ios-test-harness-design]]
- [[zig-install]]
