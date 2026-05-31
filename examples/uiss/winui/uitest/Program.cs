// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Windows interactive smoke test for the УИСС WinUI 3 app.
//
// Drives the real built uiss.exe through UI Automation (FlaUI / UIA3): launches it,
// fills the login fields, INVOKES the login control, and asserts the UI navigates —
// a genuine on-Windows interactivity check (exercises the same handler dispatch as
// Linux/Android).
//
//   dotnet run --project examples/uiss/winui/uitest -c Release -- <path-to-uiss.exe>
//
// NOTE: a full FindAllDescendants() throws E_UNEXPECTED against this app's WinUI 3
// UIA provider (an accessibility-tree defect; see vault T-0067), so this walks the
// tree level-by-level with per-node try/catch to skip the bad subtree.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using FlaUI.Core;
using FlaUI.Core.AutomationElements;
using FlaUI.Core.Capturing;
using FlaUI.Core.Definitions;
using FlaUI.UIA3;

internal static class Program
{
    private static int Main(string[] args)
    {
        var exe = args.Length > 0
            ? args[0]
            : @"D:\GitHub\MPAPP\build-vs143\examples\uiss\Debug\uiss.exe";
        var outDir = args.Length > 1 ? args[1] : Path.GetDirectoryName(Path.GetFullPath(exe));

        if (!File.Exists(exe)) { Console.WriteLine($"[FAIL] exe not found: {exe}"); return 10; }
        Console.WriteLine($"[uitest] launching {exe}");

        var app = Application.Launch(new ProcessStartInfo
        {
            FileName = exe,
            WorkingDirectory = Path.GetDirectoryName(Path.GetFullPath(exe)),
        });

        try
        {
            using var automation = new UIA3Automation();
            var window = app.GetMainWindow(automation, TimeSpan.FromSeconds(25));
            if (window == null) { Console.WriteLine("[FAIL] no main window appeared"); return 2; }
            Console.WriteLine($"[uitest] window: '{window.Name}'  class={ClassOf(window)}");
            TryFocus(window);
            Thread.Sleep(1500);

            // Diagnostic: dump the window's direct UIA children + whether each subtree
            // enumerates. Reveals where the WinUI XAML-island UIA bridge breaks.
            Console.WriteLine("--- top-level UIA children ---");
            try
            {
                foreach (var c in window.FindAllChildren())
                {
                    string sub;
                    try { sub = c.FindAllChildren().Length + " grandchildren"; }
                    catch (Exception ex) { sub = "ENUM THROWS: " + ex.GetType().Name + " 0x" + (ex.HResult.ToString("X8")); }
                    Console.WriteLine($"   {Ct(c)} class='{ClassOf(c)}' name='{Name(c)}' -> {sub}");
                }
            }
            catch (Exception ex) { Console.WriteLine("   window.FindAllChildren threw: " + ex.Message); }

            int errsBefore;
            var beforeEls = Collect(window, out errsBefore);
            var before = Tags(beforeEls);
            Console.WriteLine($"[uitest] collected {beforeEls.Count} elements (subtree errors skipped: {errsBefore})");
            Print(before, "BEFORE login");
            TrySnap(window, Path.Combine(outDir, "uitest-1-login.png"));

            // Fill the login fields (Факултетен номер / ЕГН) if present.
            var edits = beforeEls.Where(e => Ct(e) == ControlType.Edit).Select(e => e.AsTextBox()).ToList();
            Console.WriteLine($"[uitest] edit fields: {edits.Count}");
            if (edits.Count >= 1) TryEnter(edits[0], "201221001");
            if (edits.Count >= 2) TryEnter(edits[1], "0000000000");
            Thread.Sleep(400);

            // Find the login control: a Button or MenuItem named "Вход".
            var clickable = beforeEls.Where(e => Ct(e) == ControlType.Button || Ct(e) == ControlType.MenuItem).ToList();
            Console.WriteLine("[uitest] clickable: " +
                string.Join(", ", clickable.Select(b => $"{Ct(b)}:'{Name(b)}'")));
            var login = clickable.FirstOrDefault(b => (Name(b) ?? "").Contains("Вход"))
                        ?? clickable.FirstOrDefault(b => { try { return b.IsEnabled; } catch { return false; } });
            if (login == null) { Console.WriteLine("[FAIL] no login/clickable control found"); return 3; }

            Console.WriteLine($"[uitest] invoking login control {Ct(login)}:'{Name(login)}'");
            Invoke(login);
            Thread.Sleep(2200);

            var afterEls = Collect(window, out _);
            var after = Tags(afterEls);
            Print(after, "AFTER login");
            TryFocus(window);
            TrySnap(window, Path.Combine(outDir, "uitest-2-after-login.png"));

            var added = after.Except(before).ToList();
            var removed = before.Except(after).ToList();
            Console.WriteLine($"[uitest] elements: before={before.Count} after={after.Count} " +
                              $"added={added.Count} removed={removed.Count}");
            if (added.Count > 0) Console.WriteLine("[uitest] NEW after login: " + string.Join(" | ", added.Take(30)));
            if (removed.Count > 0) Console.WriteLine("[uitest] GONE after login: " + string.Join(" | ", removed.Take(15)));

            if (added.Count > 0 || removed.Count > 0)
            {
                Console.WriteLine("[PASS] invoking the login control drove a UI change — the Windows app is interactive.");
                return 0;
            }
            Console.WriteLine("[FAIL] UI did not change after invoking the login control.");
            return 4;
        }
        finally
        {
            try { if (!app.HasExited) app.Close(); } catch { }
            try { Thread.Sleep(500); if (!app.HasExited) app.Kill(); } catch { }
        }
    }

    // Level-by-level walk with per-node try/catch — a single malformed UIA node makes
    // a recursive FindAllDescendants() throw E_UNEXPECTED, so we isolate failures.
    private static List<AutomationElement> Collect(AutomationElement root, out int errors)
    {
        var acc = new List<AutomationElement>();
        int errs = 0;
        void Walk(AutomationElement el, int depth)
        {
            if (depth > 14) return;
            AutomationElement[] kids;
            try { kids = el.FindAllChildren(); }
            catch { errs++; return; }
            foreach (var k in kids) { acc.Add(k); Walk(k, depth + 1); }
        }
        Walk(root, 0);
        errors = errs;
        return acc;
    }

    private static ControlType Ct(AutomationElement e) { try { return e.ControlType; } catch { return ControlType.Custom; } }
    private static string Name(AutomationElement e) { try { return e.Name ?? ""; } catch { return ""; } }
    private static string ClassOf(AutomationElement e) { try { return e.ClassName ?? ""; } catch { return ""; } }

    private static List<string> Tags(List<AutomationElement> els)
    {
        var list = new List<string>();
        foreach (var e in els)
        {
            var ct = Ct(e); var name = Name(e);
            if ((ct == ControlType.Button || ct == ControlType.Edit || ct == ControlType.Text ||
                 ct == ControlType.MenuItem || ct == ControlType.ListItem || ct == ControlType.TabItem)
                && !string.IsNullOrWhiteSpace(name))
                list.Add($"{ct}:{name}");
        }
        return list;
    }

    private static void Print(List<string> tags, string label)
    {
        Console.WriteLine($"--- {label} ({tags.Count} named controls) ---");
        foreach (var t in tags.Take(45)) Console.WriteLine("   " + t);
    }

    private static void Invoke(AutomationElement el)
    {
        try { el.AsButton().Invoke(); return; } catch { }
        try { el.Patterns.Invoke.Pattern.Invoke(); return; } catch { }
        try { el.Click(); } catch (Exception ex) { Console.WriteLine("[warn] invoke fallback failed: " + ex.Message); }
    }

    private static void TryEnter(TextBox tb, string value)
    {
        try { tb.Focus(); tb.Text = value; }
        catch { try { tb.Enter(value); } catch { } }
    }

    private static void TryFocus(Window w)
    {
        try { w.FocusNative(); } catch { try { w.Focus(); } catch { } }
    }

    private static void TrySnap(Window w, string path)
    {
        try { Capture.Element(w).ToFile(path); Console.WriteLine($"[uitest] captured {Path.GetFileName(path)}"); }
        catch (Exception ex) { Console.WriteLine("[warn] capture failed: " + ex.Message); }
    }
}
