---
type: research
subject: ".NET MAUI Architecture Deep-Dive"
framework: maui
created: 2026-05-12
applicableTo:
  - "[[Type System]]"
  - "[[Handlers]]"
  - "[[Markup]]"
  - "[[Controls Inventory]]"
  - "[[Platform Interop]]"
  - "[[Async Executor and Event Loops]]"
recommendation: adapt
tags:
  - type/research
  - framework/maui
---

# .NET MAUI Architecture & Platform Fundamentals: Comprehensive Research Guide

> [!important] Why this document lives here
> This is the **authoritative reference** for MAUI's architecture. MPAPP mirrors MAUI's structure 1:1 (per [[ADR-0004-maui-xaml-superset-compat]]), so understanding MAUI is foundational. Per [[CLAUDE]] rule 7, behavior questions resolve against `D:\GitHub\MPAPP\references\maui\src\` first, then this document.

**Document Date:** May 2026
**Current MAUI Version:** .NET 10
**Focus:** Cross-platform framework design, embedded server capabilities, and .NET ecosystem integration

---

## 1. Overview: What is MAUI?

### Definition and Problem Statement

**.NET MAUI** stands for **.NET Multi-platform App UI**. It is a Microsoft-backed, open-source framework for building native mobile and desktop applications using C# and XAML, enabling developers to create a single codebase that compiles to native applications on multiple platforms.

**Problem MAUI Solves:**
- **Code Duplication:** Eliminates the need to rewrite UI and business logic for each platform (iOS, Android, macOS, Windows)
- **Maintenance Burden:** Single codebase means one set of bugs to fix, one feature pipeline to manage
- **Developer Friction:** Allows .NET developers to stay in the C# ecosystem without learning Swift, Kotlin, or JavaScript
- **Platform Fragmentation:** Unifies the fragmented Xamarin ecosystem into a cohesive, opinionated framework

### MAUI vs. Xamarin: Key Differences

| Aspect | Xamarin.Forms | .NET MAUI |
|--------|---------------|-----------|
| **Project Structure** | Separate platform-specific projects (iOS, Android) + shared project | Single project (multi-targeting) |
| **Platform Support** | Mobile-focused (iOS, Android only) | Mobile (iOS, Android) + Desktop (macOS, Windows) |
| **Control Architecture** | Renderers (heavy wrappers) | Handlers (lightweight, direct mappings) |
| **Performance** | Moderate, with rendering overhead | Improved startup and memory footprint |
| **Hot Reload** | Limited support | First-class XAML + C# hot reload |
| **Support Status** | **End-of-Life (May 2024)** | **Active, strategic investment** |
| **Build System** | Legacy SDK approach | Modern .NET 6+ workloads |

**Strategic Reason for Unification:** Microsoft consolidated Xamarin into MAUI to compete with Flutter and React Native while leveraging the .NET ecosystem's strengths (type system, LINQ, async/await, mature tooling).

### Platform Support & Target Environments

**MAUI Supports:**
- **iOS** (11.0+) → Compiles to native ARM64 via **AOT (Ahead-of-Time)**
- **Android** (8.0+) → **JIT (Just-In-Time)** compilation at app launch or **AOT** via R8/D8
- **macOS** (10.15+) → Uses **Mac Catalyst** (iOS app runtime on macOS)
- **Windows** (Windows 10 22H2+) → **WinUI 3** native windows
- **Linux** → Limited/unofficial support; not a primary target
- **Web** → Via **Blazor Hybrid** (WebView + C# backend)

**Note:** Desktop (macOS, Windows) support is newer and less mature than mobile. Mac Catalyst, while enabling code sharing, imposes iOS-like constraints on macOS.

### Current Maturity & Production Readiness

**Status as of May 2026:**
- **Officially Production-Ready:** Yes, for most mobile and desktop scenarios
- **Enterprise Backing:** Full Microsoft support with .NET engineering team
- **Market Adoption:** Growing, with major brands (e.g., Snyk, Syncfusion) shipping MAUI apps
- **Known Gaps:**
  - macOS/Catalyst experience is "desktop-like but not native-like"
  - Linux support is effectively non-existent
  - Some third-party Xamarin packages may not be compatible
  - Platform inconsistencies due to abstraction trade-offs

**Recommendation:** MAUI is appropriate for new projects targeting iOS, Android, and Windows. For macOS-only or Linux-heavy scenarios, consider native or Avalonia (C#/.NET alternative).

---

## 2. .NET Platform Fundamentals

### Common Language Runtime (CLR)

The **CLR** is the runtime engine that executes all .NET applications. It provides:

1. **Just-In-Time (JIT) Compilation:** Translates IL (Intermediate Language) bytecode to native machine code at runtime, allowing platform-specific optimization
2. **Garbage Collection:** Automatic memory management with configurable collection strategies
3. **Type Safety:** Enforces type checking and bounds checking at runtime
4. **Exception Handling:** Managed exception propagation with try/catch semantics
5. **Reflection & Metadata:** Full runtime introspection of types, methods, and properties

**Architecture Flow:**
```
C# Source Code
    ↓ (Roslyn compiler)
IL (Intermediate Language) bytecode
    ↓ (Packaged as .dll/.exe)
Compiled Assembly
    ↓ (At runtime, via CLR)
JIT Compiler (or AOT)
    ↓
Native Machine Code (x86_64, ARM64, etc.)
    ↓
CPU Execution
```

### JIT (Just-In-Time) Compilation

**How JIT Works:**
- The CLR interprets IL instructions at runtime
- Methods are compiled on-demand (when first called)
- Compilation is cached in memory for subsequent calls
- The JIT compiler has access to runtime statistics (profiling), allowing it to optimize hot paths

**Advantages of JIT:**
- **Adaptive Optimization:** Can recompile hot methods with better strategies based on runtime behavior
- **Platform-Specific Code:** Generates machine code optimized for the exact CPU at runtime
- **Lazy Compilation:** Only compiles code that is actually used
- **Dynamic Patching:** Can inline calls, remove null checks, and inline cache data structures dynamically

**Disadvantages:**
- **Startup Latency:** First call to a method triggers compilation delay (microseconds to milliseconds)
- **Memory Overhead:** Compiled code is kept in memory alongside IL
- **Unpredictable Pauses:** Compilation can cause brief GC or compilation stalls

**MAUI Usage:** Android apps use JIT (or Tiered JIT with QuickJIT in .NET 6+), which causes a few hundred milliseconds of startup lag on initial launch.

### AOT (Ahead-of-Time) Compilation

**How AOT Works:**
- IL is compiled to native machine code **before** the application runs
- The compiled binary is distributed as native code
- No runtime compilation step is needed

**Advantages of AOT:**
- **Fast Startup:** No compilation delay; code runs immediately
- **Predictable Performance:** No GC or compilation pauses during execution
- **Smaller Runtime:** Smaller IL footprint (only IL metadata remains)
- **Disk Efficiency:** Native code can be stored more densely

**Disadvantages:**
- **Larger Binary Size:** Compiled code is less efficient than JIT (fewer optimization opportunities)
- **Longer Build Time:** Compilation happens at build time
- **AOT Limitations:** Some reflection and dynamic dispatch patterns are harder to support
- **Ahead-of-Time Binding:** Must know which code paths will be used; generics and virtual calls have limited optimization

**MAUI Usage:** 
- **iOS apps are always AOT-compiled** (Apple policy)
- **Android apps use `NativeAOT`** (optional, via `/p:PublishAot=true` in .NET 9+) to reduce startup time and app size
- **macOS/Windows apps typically use JIT** (though NativeAOT is available)

### Garbage Collection (GC)

**.NET uses generational garbage collection:**

```
Generation 0: Newest allocations (collected frequently)
Generation 1: Medium-lived objects (collected occasionally)
Generation 2: Long-lived objects (collected rarely)
Large Object Heap (LOH): Objects >85KB (collected occasionally)
```

**Collection Process:**
1. **Mark Phase:** Walk the object graph from GC roots (stack, static fields) and mark reachable objects
2. **Compact Phase:** Shift live objects together to defragment the heap
3. **Sweep Phase:** Release unmarked memory back to the allocator

**GC Pause Characteristics:**
- **Workstation GC** (default): Non-concurrent, short pauses (~millisecond or less)
- **Server GC** (opt-in): Concurrent collection, longer walls-clock time but more parallelism
- **Generational Hypothesis:** Most allocations are short-lived; GC0 collections are fast

**MAUI Implications:**
- **Mobile Devices:** GC pauses can cause jank (dropped frames). Large object allocations should be avoided in render loops
- **Background Tasks:** Allocating large objects on background threads can trigger collection that freezes the UI thread briefly

### Type System & Generics

**.NET has a strong, static type system:**

**Key Concepts:**
- **Value Types:** `struct`, `int`, `bool`, `decimal` (stack-allocated, zero heap overhead)
- **Reference Types:** `class`, arrays, delegates (heap-allocated with reference indirection)
- **Nullable Types:** `int?`, `string?` (with null-coalescing operators)
- **Type Erasure:** Generic type information is preserved at runtime (unlike Java)

**Generics Example:**
```csharp
// Generic class
public class Container<T> {
    private T value;
    public T Get() => value;
    public void Set(T v) => value = v;
}

// Generic constraints
public class Repository<T> where T : IEntity {
    public void Save(T entity) { /* ... */ }
}

// Covariance/Contravariance
IEnumerable<Dog> dogs = GetDogs();
IEnumerable<Animal> animals = dogs; // Covariance: Dog is Animal
```

**MAUI Relevance:**
- MVVM bindings rely on generic `ObservableCollection<T>` and `INotifyPropertyChanged` on generic view models
- Property bindings use reflection on generic types, so type information is critical

### Async/Await & Task-Based Concurrency

**.NET introduced async/await in .NET 4.5 for non-blocking I/O:**

**The Task Model:**
```csharp
// Synchronous (blocking)
string data = LoadDataSync(); // Blocks caller

// Asynchronous (non-blocking)
Task<string> task = LoadDataAsync(); // Returns immediately
string data = await task; // Resumes when complete

// Task is a monad-like abstraction
Task<int> GetNumber() => Task.FromResult(42);

// Composable via continuation
Task<int> result = GetNumber()
    .ContinueWith(t => t.Result * 2);
```

**How Async/Await Works (Under the Hood):**
1. Compiler converts `async`/`await` into a state machine
2. Each `await` point becomes a state transition
3. `await` operator checks if the `Task` is complete; if not, returns control to caller
4. When the `Task` completes, continuation resumes execution

**Synchronization Context:**
- Each thread has a `SynchronizationContext` (e.g., UI thread context)
- `await` by default resumes on the original context
- On the UI thread, this ensures UI updates happen on the correct thread

**MAUI Async Model:**
```csharp
// Button click handler (async Task)
private async void OnClickCommand(object sender, EventArgs e)
{
    IsLoading = true;
    try
    {
        var data = await ApiClient.FetchDataAsync(); // Non-blocking
        Data = data;
    }
    finally
    {
        IsLoading = false;
    }
}

// Async command via RelayCommand (MVVM pattern)
[RelayCommand]
private async Task FetchData()
{
    var data = await ApiClient.FetchDataAsync();
    Items = new ObservableCollection<Item>(data);
}
```

---

## 3. XAML & UI Markup

### What is XAML?

**XAML** (Extensible Application Markup Language) is an XML-based markup language for defining UIs declaratively. XAML elements map directly to C# classes; XAML attributes map to properties or events.

**XAML in MAUI:**
- Provides a declarative way to describe page layouts and UI composition
- Automatically compiled to C# code (XAML code-behind)
- Supports data binding, style inheritance, and resource dictionaries
- Enables tooling (hot reload, WYSIWYG designers)

### XAML to C# Mapping

**Simple Example:**
```xaml
<!-- XAML -->
<VerticalStackLayout Padding="20" Spacing="10">
    <Label Text="Hello, MAUI!" FontSize="24" TextColor="Blue" />
    <Button Text="Click Me" Clicked="OnButtonClicked" />
</VerticalStackLayout>
```

**Equivalent C# Code:**
```csharp
// C# equivalent
var layout = new VerticalStackLayout
{
    Padding = 20,
    Spacing = 10,
    Children =
    {
        new Label
        {
            Text = "Hello, MAUI!",
            FontSize = 24,
            TextColor = Colors.Blue
        },
        new Button
        {
            Text = "Click Me"
        }
    }
};
layout.Children[1].Clicked += OnButtonClicked;
```

**XAML Compilation Process:**
1. XAML parser reads markup
2. Parser instantiates C# types and sets properties
3. Event handlers are wired to methods in code-behind
4. Compiled to an intermediate C# file, then to IL

### Code-Behind Pattern

```xaml
<!-- MainPage.xaml -->
<?xml version="1.0" encoding="utf-8" ?>
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             x:Class="MyApp.MainPage"
             Title="Home">
    <VerticalStackLayout>
        <Entry x:Name="NameEntry" Placeholder="Enter name" />
        <Button Text="Submit" Clicked="OnSubmitClicked" />
    </VerticalStackLayout>
</ContentPage>
```

```csharp
// MainPage.xaml.cs (code-behind)
public partial class MainPage : ContentPage
{
    public MainPage()
    {
        InitializeComponent(); // Auto-generated from XAML
    }

    private void OnSubmitClicked(object sender, EventArgs e)
    {
        var name = NameEntry.Text; // Access XAML elements by x:Name
        DisplayAlert("Hello", $"Welcome, {name}!");
    }
}
```

**Note:** Direct control references (e.g., `NameEntry`) are generated by the XAML compiler from `x:Name` attributes. This couples the view to the code-behind, which is why MVVM patterns use data binding instead.

### Data Binding

Data binding connects a view property (target) to a view model property (source) so changes propagate automatically.

**Basic Binding Syntax:**
```xaml
<!-- OneWay binding (default): source → target -->
<Label Text="{Binding UserName}" />

<!-- TwoWay binding: source ↔ target -->
<Entry Text="{Binding UserName, Mode=TwoWay}" />

<!-- OneWayToSource: target → source (less common) -->
<Slider Value="{Binding Brightness, Mode=OneWayToSource}" />

<!-- Binding with converter -->
<Label Text="{Binding CreatedDate, StringFormat='Created: {0:G}'}" />

<!-- Binding with converter function -->
<Label Text="{Binding IsLoading, Converter={StaticResource BoolToTextConverter}}" />
```

**Binding Modes:**
- **OneWay (Default):** Source changes → target auto-updates. Target changes are ignored.
- **TwoWay:** Source ↔ target. Both synchronize. Useful for editable fields.
- **OneWayToSource:** Target changes → source auto-updates. Source changes are ignored.
- **OneTime:** Target is set once from source at binding initialization; no further sync.

**BindingContext:**
```csharp
// View Model
public class MainViewModel : INotifyPropertyChanged
{
    private string userName = "Alice";
    public string UserName
    {
        get => userName;
        set
        {
            if (userName != value)
            {
                userName = value;
                OnPropertyChanged(nameof(UserName)); // Notify binding
            }
        }
    }

    public event PropertyChangedEventHandler PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string name = "")
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}

// View code-behind
public MainPage()
{
    InitializeComponent();
    BindingContext = new MainViewModel(); // All bindings reference this object
}
```

### Style System & Themes

**XAML Styles:**
```xaml
<!-- Define a style -->
<Style x:Key="LabelStyle" TargetType="Label">
    <Setter Property="FontSize" Value="16" />
    <Setter Property="TextColor" Value="Black" />
    <Setter Property="Margin" Value="10" />
</Style>

<!-- Apply a style -->
<Label Text="Styled Text" Style="{StaticResource LabelStyle}" />

<!-- Implicit style (applied to all Label controls) -->
<Style TargetType="Label">
    <Setter Property="FontSize" Value="14" />
</Style>
```

**Implicit Styles & Inheritance:**
```xaml
<!-- Base style -->
<Style x:Key="BaseButtonStyle" TargetType="Button">
    <Setter Property="CornerRadius" Value="8" />
    <Setter Property="Padding" Value="10,5" />
</Style>

<!-- Derived style -->
<Style x:Key="PrimaryButtonStyle" TargetType="Button" BasedOn="{StaticResource BaseButtonStyle}">
    <Setter Property="BackgroundColor" Value="Blue" />
    <Setter Property="TextColor" Value="White" />
</Style>
```

**App-Level Theme:**
```xml
<!-- App.xaml -->
<Application.Resources>
    <ResourceDictionary>
        <!-- Light theme (default) -->
        <Color x:Key="PrimaryColor">Blue</Color>
        <Color x:Key="BackgroundColor">White</Color>
    </ResourceDictionary>
</Application.Resources>

<!-- Referenced in App.xaml.cs for AppTheme changes -->
public App()
{
    InitializeComponent();
    UserAppTheme = AppTheme.Dark; // Switch to dark theme
}
```

### Custom Controls & Composition

**Creating a Custom Control:**
```csharp
// Custom control inheriting from ContentView (composite)
public class CardView : ContentView
{
    public static readonly BindableProperty TitleProperty =
        BindableProperty.Create(nameof(Title), typeof(string), typeof(CardView), "");

    public string Title
    {
        get => (string)GetValue(TitleProperty);
        set => SetValue(TitleProperty, value);
    }

    public CardView()
    {
        Content = new Frame
        {
            BorderColor = Colors.LightGray,
            CornerRadius = 8,
            Padding = 15,
            Content = new VerticalStackLayout
            {
                Children =
                {
                    new Label { Text = Title, FontAttributes = FontAttributes.Bold }
                }
            }
        };
    }
}

// Using the custom control
<ContentPage xmlns:local="clr-namespace:MyApp">
    <local:CardView Title="My Card" />
</ContentPage>
```

**BindableProperty Pattern:**
- Allows XAML data binding and style application
- Provides change notifications and validation
- Essential for custom controls

---

## 4. Platform Handler Architecture

### Overview: Handlers vs. Renderers

**Renderers (Xamarin.Forms Legacy):**
```
MAUI Control (e.g., Button)
    ↓
Renderer (heavy wrapper, holds native control)
    ↓
Native ViewGroup (Android) or UIView wrapper (iOS)
    ↓
Native Button (Android: android.widget.Button, iOS: UIButton)
```

**Handlers (MAUI Modern):**
```
MAUI Control (e.g., Button)
    ↓
Handler (lightweight mapper, delegates to native control)
    ↓
Native Button (Android: MaterialButton, iOS: UIButton)
```

**Key Difference:** Handlers **remove the wrapper layer**, reducing view hierarchy depth and simplifying layout calculations.

### Handler Architecture

```csharp
// A handler bridges a MAUI control to a native control
public interface IButtonHandler : IViewHandler
{
    Button VirtualView { get; } // The MAUI Button
    PlatformView NativeView { get; } // The native control (android.widget.Button, UIButton, etc.)
}

// Handler implementation (pseudo-code)
public partial class ButtonHandler : ViewHandler<Button, MauiButton>
{
    // MauiButton is a platform-specific wrapper (e.g., AppCompat Button on Android)

    protected override MauiButton CreatePlatformView()
    {
        return new MauiButton(Context); // Android
    }

    private void MapText(IButtonHandler handler, Button button)
    {
        handler.PlatformView.Text = button.Text; // Sync MAUI property to native
    }

    private void MapClicked(IButtonHandler handler, Button button)
    {
        handler.PlatformView.Click += (s, e) => button.Clicked?.Invoke(button, EventArgs.Empty);
    }
}
```

### Virtual Views & Native Views

**Virtual Views** are the cross-platform MAUI controls (Button, Label, Entry).

**Native Views** are platform-specific controls:
- **Android:** `android.widget.Button`, `android.widget.EditText`
- **iOS:** `UIButton`, `UITextField`
- **Windows:** `Microsoft.UI.Xaml.Controls.Button`, `TextBox`
- **macOS:** `AppKit.NSButton`, `NSTextField` (via Catalyst)

Handlers provide **property mappers** that translate MAUI properties to native properties:

```csharp
// Property mapper configuration
public static void MapText(IButtonHandler handler, Button button)
    => handler.PlatformView.Text = button.Text;

public static void MapTextColor(IButtonHandler handler, Button button)
    => handler.PlatformView.SetTextColor(button.TextColor.ToPlatformColor());

// Global mapper registration
ButtonHandler.Mapper.Add(nameof(Button.Text), MapText);
ButtonHandler.Mapper.Add(nameof(Button.TextColor), MapTextColor);
```

### Property Mapping & Native Property Setting

**How a Property Change Propagates:**

```
MAUI C# Code:
button.Text = "Click Me"; // Property setter

↓ (PropertyChanged event fires)

Handler Property Mapper:
MapText handler is invoked
handler.PlatformView.Text = "Click Me"; // Set native property

↓ (Platform rendering)

Native View Renders:
Platform-specific button renders with new text
```

**Bidirectional Binding (Two-Way):**
```csharp
// Native control notifies handler of change
handler.PlatformView.TextChanged += (s, e) =>
{
    handler.VirtualView.Text = handler.PlatformView.Text;
    // This triggers MAUI property changed, which updates data bindings
};
```

### Customizing Handlers

Developers can override default handlers to customize platform-specific behavior:

```csharp
// Custom handler for iOS-specific Button customization
public class CustomButtonHandler : ButtonHandler
{
    protected override MauiButton CreatePlatformView()
    {
        var button = base.CreatePlatformView();
        
        if (OperatingSystem.IsIOS())
        {
            // iOS-specific customization
            button.ShowsTouchWhenHighlighted = true;
        }

        return button;
    }
}

// Register custom handler in MauiProgram
builder.ConfigureMauiHandlers(handlers =>
{
    handlers.AddHandler<Button, CustomButtonHandler>();
});
```

---

## 5. Control & Rendering System

### Standard MAUI Controls

**Layout Controls:**
- **VerticalStackLayout / HorizontalStackLayout:** Stack children linearly
- **Grid:** Two-dimensional layout with rows and columns
- **FlexLayout:** CSS Flexbox-inspired layout
- **AbsoluteLayout:** Absolute positioning
- **ScrollView:** Scrollable container

**Content Controls:**
- **Button:** Clickable button
- **Label:** Read-only text
- **Entry:** Single-line text input
- **Editor:** Multi-line text input
- **Picker:** Dropdown selection
- **DatePicker / TimePicker:** Date/time selection
- **Switch:** Boolean toggle
- **Slider:** Numeric range selection
- **ProgressBar:** Progress indication

**List Controls:**
- **CollectionView:** Modern, virtualized collection view (recommended)
- **ListView:** Legacy virtualized list (still supported)
- **TableView:** Settings-style table

**Advanced Controls:**
- **WebView:** Embedded web content
- **HybridWebView:** Web + native interop
- **MapsView:** Maps integration (via package)
- **GraphicsView:** Custom canvas drawing

### Layout Engine & Constraint-Based Sizing

The MAUI layout system uses a **two-phase measurement & arrangement algorithm:**

```
Phase 1: Measure
─────────────────────────────────────
Available Space
    ↓
Each child's Measure() called with available constraints
    ↓
Child returns its desired size
    ↓
Parent calculates total desired size

Phase 2: Arrange
─────────────────────────────────────
Actual available space (may differ from desired)
    ↓
Parent arranges each child with final bounds
    ↓
Child's Arrange() is called with final bounds
    ↓
Child returns actual size used
```

**Measure Contract:**
```csharp
public Size Measure(double widthConstraint, double heightConstraint)
{
    // Returns: the size the control wants given the constraints
    // Example: if widthConstraint is 300, label might want Size(280, 40)
}

public void Arrange(Rect bounds)
{
    // Called with final bounds; control positions itself within
    // If bounds.Width is 300 but label wanted 280, it uses 300
}
```

**Example: VerticalStackLayout Measurement:**
```
VerticalStackLayout with MaximumWidth=400, Spacing=10
  - Child 1: Desired=350×30
  - Child 2: Desired=350×50
  - Child 3: Desired=350×30

Measure: VerticalStackLayout.Measure(400, ∞)
  → Calls Child 1.Measure(400, ∞) → 350×30
  → Calls Child 2.Measure(400, ∞) → 350×50
  → Calls Child 3.Measure(400, ∞) → 350×30
  → Total: 350×130 (max width of children + heights + spacing)
  → Returns 350×130

Arrange: VerticalStackLayout.Arrange(Rect(0, 0, 400, 200))
  → Child 1 at (0, 0, 400, 30)
  → Child 2 at (0, 40, 400, 50)
  → Child 3 at (0, 100, 400, 30)
```

### MAUI Control to Platform Control Mapping

| MAUI Control | Android | iOS | Windows | macOS |
|--------------|---------|-----|---------|-------|
| Button | MaterialButton | UIButton | Button | NSButton |
| Label | TextView | UILabel | TextBlock | NSTextField |
| Entry | TextInputEditText | UITextField | TextBox | NSTextField |
| Editor | EditText | UITextView | TextBox (multiline) | NSTextView |
| Image | ImageView | UIImageView | Image | NSImageView |
| Switch | SwitchMaterial | UISwitch | ToggleSwitch | NSButton |
| Slider | SeekBar | UISlider | Slider | NSSlider |
| ProgressBar | ProgressBar | UIProgressView | ProgressBar | NSProgressIndicator |
| Picker | Spinner | UIPickerView | ComboBox | NSPopUpButton |
| CollectionView | RecyclerView | UICollectionView | ItemsRepeater | NSCollectionView |

### Custom Drawing with GraphicsView

GraphicsView enables custom 2D graphics via the IDrawable interface:

```csharp
// Custom drawable
public class CircleDrawable : IDrawable
{
    public float Radius { get; set; } = 50;
    public Color FillColor { get; set; } = Colors.Blue;

    public void Draw(ICanvas canvas, RectF dirtyRect)
    {
        canvas.FillColor = FillColor;
        canvas.FillCircle(dirtyRect.Center.X, dirtyRect.Center.Y, Radius);
    }
}

// Use in XAML
<GraphicsView Drawable="{Binding CircleDrawable}" />

// Or in C#
var graphicsView = new GraphicsView
{
    Drawable = new CircleDrawable { Radius = 75 }
};
```

### Animation System

**Built-in Animation:**
```csharp
// Simple property animation
await button.ScaleTo(1.2, 500); // Scale to 1.2 over 500ms
await label.FadeTo(0.5, 1000); // Fade to 50% opacity
await entry.TranslateTo(100, 0, 250); // Slide right

// Composite animation
await Task.WhenAll(
    button.ScaleTo(1.2, 500),
    button.RotateTo(360, 500)
); // Scale and rotate simultaneously

// Custom animation
var animation = new Animation(
    callback: v => button.Scale = v,
    start: 1.0,
    end: 1.5,
    easing: Easing.SinOut
);
animation.Commit(this, "ScaleAnimation", 16, 500, finished: (v, c) => { });
```

**GraphicsView Animation Example:**
```csharp
public class AnimatedDrawable : IDrawable
{
    private double rotation = 0;

    public void Draw(ICanvas canvas, RectF dirtyRect)
    {
        canvas.SaveState();
        canvas.Translate(dirtyRect.Center.X, dirtyRect.Center.Y);
        canvas.Rotate((float)rotation);
        canvas.FillColor = Colors.Blue;
        canvas.FillCircle(0, -50, 10); // Draw circle rotating around center
        canvas.RestoreState();
    }

    public void UpdateRotation(double newRotation)
    {
        rotation = newRotation;
    }
}

// Animate rotation
var drawable = new AnimatedDrawable();
var graphicsView = new GraphicsView { Drawable = drawable };

var animation = new Animation(
    callback: v => drawable.UpdateRotation(v),
    start: 0,
    end: 360
);
animation.Commit(graphicsView, "Rotation", 16, 3000, repeat: () => true);
```

---

## 6. Platform Integration

### Calling Native Platform APIs

MAUI provides mechanisms for invoking native code on each platform.

### Objective-C/Swift Bridging (iOS)

**Method 1: Native Library Interop (Modern, Recommended)**

```csharp
// Step 1: Create binding project with Objective-C interface
// File: ZendeskBinding.cs in iOS binding project
[Protocol]
public interface IZendeskSupport
{
    [Export("initWithKey:")]
    IntPtr Constructor(string key);

    [Export("startChat")]
    void StartChat();
}

// Step 2: Use in MAUI
public class PlatformIntegration
{
    public void StartZendeskChat()
    {
        #if IOS
        var zendesk = new ZendeskSupport("my-key");
        zendesk.StartChat();
        #endif
    }
}
```

**Method 2: Direct P/Invoke (Lower-level)**

```csharp
using System.Runtime.InteropServices;

public static class iOSNative
{
    [DllImport("__Internal")]
    public static extern void SystemSleep(int seconds);

    public static void SleepDevice(int seconds)
    {
        if (OperatingSystem.IsIOS())
            SystemSleep(seconds);
    }
}
```

**Objective Sharpie Tool:**
Microsoft provides Objective Sharpie to automatically generate C# binding definitions from Objective-C headers:

```bash
sharpie bind -sdk iphoneos ZendeskSDK.framework/Headers/Zendesk.h
```

This generates C# class definitions corresponding to Objective-C classes and methods.

### Java/Kotlin Bridging (Android)

**Method 1: Managed Callable Wrappers (MCW) via Binding Projects**

```csharp
// Binding project C# wrapper for Android SDK
// File: AndroidManifest.xml + binding XML

[Register("com/zendesk/ZendeskSupport")]
public class ZendeskSupport : Java.Lang.Object
{
    static IntPtr class_ref = JNIEnv.FindClass("com/zendesk/ZendeskSupport");

    [Export]
    public void StartChat()
    {
        // JNI call to Java method
    }
}

// Use in MAUI
public class PlatformIntegration
{
    public void StartZendeskChat()
    {
        #if ANDROID
        var zendesk = new ZendeskSupport();
        zendesk.StartChat();
        #endif
    }
}
```

**Method 2: Direct Java Interop (JNIEnv)**

```csharp
using Android.Runtime;

public class JavaInterop
{
    // Find Java class
    static IntPtr buttonClass = JNIEnv.FindClass("android/widget/Button");
    static IntPtr setText = JNIEnv.GetMethodID(buttonClass, "setText", "(Ljava/lang/CharSequence;)V");

    public static void SetButtonText(IntPtr buttonPtr, string text)
    {
        // Call Java method via JNI
        JNIEnv.CallVoidMethod(buttonPtr, setText, new JValue(text));
    }
}
```

### Safe Interop at the Boundary

**Key Safety Rules:**

1. **Exception Handling:** Never let exceptions cross the boundary unhandled
   ```csharp
   public class SafeNativeWrapper
   {
       public bool TryCallNative(Action nativeCall)
       {
           try
           {
               nativeCall();
               return true;
           }
           catch (Exception ex)
           {
               Debug.WriteLine($"Native call failed: {ex}");
               return false;
           }
       }
   }
   ```

2. **Memory Ownership:** Always document which side owns allocated memory
   ```csharp
   [DllImport("MyLib")]
   public static extern IntPtr AllocateBuffer(int size); // C++ owns

   [DllImport("MyLib")]
   public static extern void FreeBuffer(IntPtr ptr); // Caller must free
   ```

3. **Thread Safety:** Native calls may block the UI thread
   ```csharp
   // Wrap long-running native calls
   var result = await Task.Run(() => NativeExpensiveOperation());
   ```

4. **Platform-Specific Code:**
   ```csharp
   // Use conditional compilation
   #if IOS
       iOSNativeCall();
   #elif ANDROID
       AndroidNativeCall();
   #endif

   // Or platform detection at runtime
   if (OperatingSystem.IsIOS()) { /* ... */ }
   ```

---

## 7. Threading & Async Model

### Threading Architecture

MAUI/Android/iOS are fundamentally **single-threaded for UI** operations. All UI mutations must occur on the main thread.

**Thread Types:**
- **Main/UI Thread:** Executes UI rendering and event handlers
- **Background Threads:** Task.Run, ThreadPool for I/O and compute
- **Platform-Specific Threads:**
  - iOS: Grand Central Dispatch (GCD)
  - Android: Handler/Looper, ThreadPool

### Task-Based Async/Await

```csharp
// Async method (non-blocking)
public async Task<Data> FetchDataAsync()
{
    using var client = new HttpClient();
    var response = await client.GetAsync("https://api.example.com/data"); // Non-blocking I/O
    var content = await response.Content.ReadAsStringAsync();
    return JsonSerializer.Deserialize<Data>(content);
}

// Call from UI
private async void OnLoadClicked()
{
    IsLoading = true;
    try
    {
        var data = await FetchDataAsync(); // Awaits without blocking UI thread
        DisplayData(data);
    }
    catch (Exception ex)
    {
        await DisplayAlert("Error", ex.Message);
    }
    finally
    {
        IsLoading = false;
    }
}

// Background work
private async Task ProcessLargeDatasetAsync()
{
    var result = await Task.Run(async () =>
    {
        // Compute-heavy work on thread pool
        return await ExpensiveCalculation();
    });
}
```

### MainThread Dispatcher & UI Thread Access

**Two APIs for accessing the main thread:**

**1. MainThread (Static, Recommended for new code)**
```csharp
// Static method
MainThread.BeginInvokeOnMainThread(() =>
{
    Label.Text = "Updated from background thread";
});

// Async variant (awaitable)
await MainThread.InvokeOnMainThreadAsync(() =>
{
    Label.Text = "Updated";
});

// With return value
string result = await MainThread.InvokeOnMainThreadAsync(() =>
{
    return Label.Text;
});
```

**2. Application.Current.Dispatcher (Instance, Cross-platform)**
```csharp
// Dispatch action
Application.Current?.Dispatcher.Dispatch(() =>
{
    Label.Text = "Updated";
});

// Async dispatch
await Application.Current?.Dispatcher.DispatchAsync(() =>
{
    Label.Text = "Updated";
});
```

**Caveat:** On Windows, `MainThread.InvokeOnMainThreadAsync()` may have issues; `Application.Current.Dispatcher` is more reliable.

### Background Work Patterns

**Pattern 1: Fire-and-Forget with Error Handling**
```csharp
private async void OnButtonClicked()
{
    _ = SyncDataInBackgroundAsync(); // Fire and forget (discard result)
}

private async Task SyncDataInBackgroundAsync()
{
    try
    {
        await SyncWithServer();
    }
    catch (Exception ex)
    {
        MainThread.BeginInvokeOnMainThread(() =>
        {
            DisplayAlert("Sync Failed", ex.Message);
        });
    }
}
```

**Pattern 2: Parallel Background Tasks**
```csharp
private async Task LoadMultipleSources()
{
    // Fetch all in parallel
    var images = Task.Run(() => FetchImagesAsync());
    var data = Task.Run(() => FetchDataAsync());
    var settings = Task.Run(() => FetchSettingsAsync());

    await Task.WhenAll(images, data, settings);

    MainThread.BeginInvokeOnMainThread(() =>
    {
        // Update UI with all results
    });
}
```

**Pattern 3: Cancellation**
```csharp
private CancellationTokenSource cts = new();

private async void OnLoadClicked()
{
    cts = new CancellationTokenSource();
    try
    {
        var data = await FetchDataAsync(cts.Token);
        DisplayData(data);
    }
    catch (OperationCanceledException)
    {
        await DisplayAlert("Cancelled", "Data loading was cancelled");
    }
}

private void OnCancelClicked()
{
    cts?.Cancel(); // Cancel all pending operations
}

private async Task<Data> FetchDataAsync(CancellationToken ct)
{
    using var client = new HttpClient();
    var response = await client.GetAsync("https://...", ct); // Respects cancellation
    return await response.Content.ReadAsAsync<Data>();
}
```

---

## 8. Embedded Server Capabilities

### Can You Embed ASP.NET Core in MAUI?

**Short Answer:** Yes, it is possible but with limitations and significant trade-offs.

### Architecture: Hosting ASP.NET Core in MAUI

**High-Level Design:**
```
MAUI Application (iOS, Android, Windows)
    ↓
.NET Runtime (CLR)
    ↓
ASP.NET Core Kestrel Server (in-process)
    ↓
HTTP Endpoints (localhost:5000, etc.)
    ↓
HybridWebView or WebView (Local HTTP client)
    ↓
Web UI (Blazor, HTML/JS, or custom)
```

### Proof-of-Concept Implementation

```csharp
// Custom HostingService to run Kestrel in MAUI
public class EmbeddedServerService
{
    private WebApplication app;

    public async Task StartServerAsync(int port = 5000)
    {
        var builder = WebApplication.CreateBuilder();

        // Register services
        builder.Services.AddControllers();
        builder.Services.AddScoped<IDataRepository, LocalDataRepository>();

        app = builder.Build();

        // Map endpoints
        app.MapControllers();
        app.MapGet("/api/health", () => Results.Ok("Server is running"));

        // Run on localhost
        await app.StartAsync(new Uri($"http://127.0.0.1:{port}"));
    }

    public async Task StopServerAsync()
    {
        if (app != null)
            await app.StopAsync();
    }
}

// In MAUI App.xaml.cs
public partial class App : Application
{
    private EmbeddedServerService serverService;

    public App()
    {
        InitializeComponent();

        MainPage = new AppShell();

        // Start server on app launch
        serverService = new EmbeddedServerService();
        _ = serverService.StartServerAsync();
    }
}

// In a page with HybridWebView
<HybridWebView x:Name="HybridView" />

// Code-behind
public partial class MainPage : ContentPage
{
    public MainPage()
    {
        InitializeComponent();

        // Load embedded server content
        HybridView.Source = new HtmlWebViewSource
        {
            Html = @"
                <html>
                    <body>
                        <h1>Embedded Server</h1>
                        <button onclick='fetch(\"/api/health\").then(r => r.text()).then(t => alert(t))'>
                            Check Health
                        </button>
                    </body>
                </html>"
        };
    }
}
```

### Platform-Specific Limitations

**iOS Limitations:**
- **Sandboxing:** Apple's app sandbox restricts network access; localhost services are accessible within the app only
- **No Background Networking:** Server cannot continue running when app is backgrounded
- **Memory Constraints:** Tight memory budget; ASP.NET Core adds ~50-100 MB overhead

**Android Limitations:**
- **Memory Pressure:** Android terminates background services aggressively
- **Battery Drain:** Server polling and I/O drain battery quickly
- **No Persistent Services (without Framework):** Background services require WorkManager API

**Windows/macOS Considerations:**
- **Firewall:** Windows Defender Firewall may block Kestrel; explicit port-forwarding needed
- **Port Conflicts:** System services may occupy port 5000; requires dynamic port selection
- **Resource Usage:** Desktop has more memory; ASP.NET Core overhead is less critical

### Code Sharing Between UI and Server Layers

**Monolithic Approach (Recommended):**
```csharp
// Shared project: MyApp.Shared.csproj
public class TodoItem
{
    public int Id { get; set; }
    public string Title { get; set; }
    public bool IsCompleted { get; set; }
}

public interface ITodoRepository
{
    Task<IList<TodoItem>> GetAllAsync();
    Task SaveAsync(TodoItem item);
}

// MAUI App
public class TodoViewModel
{
    private ITodoRepository repository;

    public TodoViewModel(ITodoRepository repo) => repository = repo;

    [RelayCommand]
    public async Task LoadTodos()
    {
        Todos = await repository.GetAllAsync(); // Same interface on client and server
    }
}

// ASP.NET Core Controller
[ApiController]
[Route("api/todos")]
public class TodosController : ControllerBase
{
    private ITodoRepository repository;

    [HttpGet]
    public async Task<IActionResult> GetAll()
    {
        var items = await repository.GetAllAsync();
        return Ok(items); // Return shared model
    }
}
```

**API Layer Separation:**
```csharp
// MAUI → API Client
public class RemoteTodoRepository : ITodoRepository
{
    private HttpClient httpClient;

    public async Task<IList<TodoItem>> GetAllAsync()
    {
        var response = await httpClient.GetAsync("http://localhost:5000/api/todos");
        var json = await response.Content.ReadAsStringAsync();
        return JsonSerializer.Deserialize<IList<TodoItem>>(json);
    }
}

// Embedded Server → Local Repository
public class LocalTodoRepository : ITodoRepository
{
    private List<TodoItem> todos = new();

    public Task<IList<TodoItem>> GetAllAsync() => Task.FromResult((IList<TodoItem>)todos);
}

// DI: On desktop, use LocalTodoRepository; on mobile, use RemoteTodoRepository
// (if separating UI and server processes)
```

### Why It's Not Ideal

1. **No Deployment Separation:** Server and UI are in the same process; server lifecycle is tied to app
2. **Resource Overhead:** ASP.NET Core adds 50-100 MB to app size and startup time
3. **Complexity:** Adds network serialization/deserialization layer where direct in-process calls suffice
4. **Testing:** Harder to test server logic independently

**Better Pattern:** Keep server logic in MAUI ViewModels/Services; use ASP.NET Core only if you truly need a separate server (multi-user scenarios, web client, external integrations).

---

## 9. Dependency Injection & Service Locator

### Built-In DI Container

MAUI has a built-in DI container via `Microsoft.Extensions.DependencyInjection`.

**Setup in MauiProgram:**
```csharp
public static class MauiProgram
{
    public static MauiApp CreateMauiApp()
    {
        var builder = MauiApp.CreateBuilder();

        builder
            .UseMauiApp<App>()
            .ConfigureFonts(fonts =>
            {
                fonts.AddFont("OpenSans-Regular.ttf", "OpenSansRegular");
            })
            // Configure DI
            .ConfigureServices(services =>
            {
                // Transient: New instance every time
                services.AddTransient<MainPage>();
                services.AddTransient<MainViewModel>();

                // Scoped: One instance per scope (useful for web; less relevant in MAUI)
                services.AddScoped<IDataService, DataService>();

                // Singleton: One instance for app lifetime
                services.AddSingleton<INavigationService, NavigationService>();
                services.AddSingleton<IAppSettings, AppSettings>();

                // Factory registration
                services.AddSingleton<IApiClient>(provider =>
                {
                    var baseUrl = provider.GetRequiredService<IAppSettings>().ApiBaseUrl;
                    return new ApiClient(baseUrl);
                });
            });

        return builder.Build();
    }
}
```

### Service Registration & Resolution

**Constructor Injection (Preferred):**
```csharp
public class MainViewModel
{
    private IDataService dataService;
    private INavigationService navigationService;

    // DI container automatically resolves dependencies
    public MainViewModel(IDataService dataService, INavigationService navigationService)
    {
        this.dataService = dataService;
        this.navigationService = navigationService;
    }
}

// In page code-behind
public partial class MainPage : ContentPage
{
    public MainPage(MainViewModel viewModel)
    {
        InitializeComponent();
        BindingContext = viewModel; // Injected from container
    }
}

// Shell routing automatically resolves from container
Routing.RegisterRoute(nameof(MainPage), typeof(MainPage));
// When navigated to, MainPage is resolved from DI container
// All its dependencies are automatically injected
```

**Service Locator Pattern (Anti-Pattern, but available):**
```csharp
// Get IServiceProvider from App
var serviceProvider = Application.Current.Handler.MauiContext.Services;

// Manually resolve
var dataService = serviceProvider.GetRequiredService<IDataService>();

// Try to resolve (returns null if not found)
var optional = serviceProvider.GetService<IOptionalService>();
```

**Avoid Service Locator:** Constructor injection is cleaner and more testable.

### MVVM Patterns Support

MAUI supports standard MVVM through the DI container and data binding:

```csharp
// Base ViewModel with INotifyPropertyChanged
public abstract class BaseViewModel : INotifyPropertyChanged
{
    public event PropertyChangedEventHandler PropertyChanged;

    protected void SetProperty<T>(ref T backingField, T value, [CallerMemberName] string name = "")
    {
        if (!Equals(backingField, value))
        {
            backingField = value;
            OnPropertyChanged(name);
        }
    }

    protected void OnPropertyChanged([CallerMemberName] string name = "")
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}

// Concrete ViewModel
public class TodoViewModel : BaseViewModel
{
    private bool isLoading;
    public bool IsLoading
    {
        get => isLoading;
        set => SetProperty(ref isLoading, value);
    }

    private ObservableCollection<TodoItem> todos;
    public ObservableCollection<TodoItem> Todos
    {
        get => todos;
        set => SetProperty(ref todos, value);
    }

    private ICommand loadCommand;
    public ICommand LoadCommand
        => loadCommand ??= new AsyncRelayCommand(LoadTodosAsync);

    private ITodoRepository repository;

    public TodoViewModel(ITodoRepository repository)
    {
        this.repository = repository;
    }

    private async Task LoadTodosAsync()
    {
        IsLoading = true;
        try
        {
            var items = await repository.GetAllAsync();
            Todos = new ObservableCollection<TodoItem>(items);
        }
        finally
        {
            IsLoading = false;
        }
    }
}
```

**Community Toolkit MVVM:** The official .NET MAUI Community Toolkit provides `RelayCommand` and source generators to reduce boilerplate:

```csharp
[ObservableObject]
public partial class TodoViewModel
{
    [ObservableProperty]
    bool isLoading;

    [ObservableProperty]
    ObservableCollection<TodoItem> todos;

    private ITodoRepository repository;

    public TodoViewModel(ITodoRepository repository)
    {
        this.repository = repository;
    }

    [RelayCommand]
    private async Task LoadTodos()
    {
        IsLoading = true;
        try
        {
            var items = await repository.GetAllAsync();
            Todos = new ObservableCollection<TodoItem>(items);
        }
        finally
        {
            IsLoading = false;
        }
    }
}
```

The source generator automatically generates `INotifyPropertyChanged` and command properties.

---

## 10. Data Binding & MVVM

### Command Binding for Button Clicks

```xaml
<!-- Bind button to ICommand in ViewModel -->
<Button Text="Load Data"
        Command="{Binding LoadDataCommand}"
        IsEnabled="{Binding IsNotLoading}" />

<ActivityIndicator IsRunning="{Binding IsLoading}"
                   IsVisible="{Binding IsLoading}" />
```

```csharp
// ViewModel
public class MainViewModel : INotifyPropertyChanged
{
    public ICommand LoadDataCommand { get; }

    public MainViewModel()
    {
        LoadDataCommand = new AsyncRelayCommand(LoadDataAsync);
    }

    private async Task LoadDataAsync()
    {
        IsLoading = true;
        try
        {
            var data = await ApiClient.FetchAsync();
            Items = new ObservableCollection<Item>(data);
        }
        finally
        {
            IsLoading = false;
        }
    }

    // INotifyPropertyChanged implementation...
}
```

### Two-Way Data Binding

```xaml
<!-- OneWay: Edit entry in XAML (from source) -->
<Entry Text="{Binding UserName, Mode=OneWay}" />

<!-- TwoWay: Entry updates ViewModel property -->
<Entry Text="{Binding UserName, Mode=TwoWay}" />

<!-- Binding with converter -->
<Label Text="{Binding IsLoading, StringFormat='Loading: {0}'}" />
```

**TwoWay Example:**
```csharp
public class SettingsViewModel : INotifyPropertyChanged
{
    private string userName = "Alice";
    public string UserName
    {
        get => userName;
        set
        {
            if (userName != value)
            {
                userName = value;
                OnPropertyChanged(nameof(UserName));
                // Save to storage
                SecureStorage.SetAsync("UserName", value);
            }
        }
    }

    // INotifyPropertyChanged...
}
```

### INotifyPropertyChanged & Collection Change Notifications

```csharp
// Property change notification
public class Person : INotifyPropertyChanged
{
    private string firstName = "";
    public string FirstName
    {
        get => firstName;
        set
        {
            if (firstName != value)
            {
                firstName = value;
                OnPropertyChanged(nameof(FirstName));
            }
        }
    }

    public event PropertyChangedEventHandler PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string name = "")
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}

// Collection change notification
public class People
{
    public ObservableCollection<Person> Items { get; } = new();

    public void AddPerson(Person person)
    {
        Items.Add(person); // Triggers CollectionChanged event
    }

    public void RemovePerson(Person person)
    {
        Items.Remove(person); // Notifies CollectionView to update
    }
}

// In ViewModel
public class PeopleViewModel : INotifyPropertyChanged
{
    public ObservableCollection<Person> People { get; } = new();

    public void LoadPeople()
    {
        foreach (var person in GetPeopleFromDb())
        {
            People.Add(person); // Each Add triggers UI update
        }
    }

    // INotifyPropertyChanged...
}
```

### Computed Properties & MVVM Frameworks

```csharp
// Computed property (read-only, derived from other properties)
public class PersonViewModel : INotifyPropertyChanged
{
    private string firstName = "";
    public string FirstName
    {
        get => firstName;
        set { SetProperty(ref firstName, value); }
    }

    private string lastName = "";
    public string LastName
    {
        get => lastName;
        set { SetProperty(ref lastName, value); }
    }

    // Computed property
    public string FullName => $"{FirstName} {lastName}";

    // Problem: FullName change is not notified when FirstName/LastName change
    // Solution: Notify when dependencies change
    private void SetProperty<T>(ref T field, T value, [CallerMemberName] string name = "")
    {
        if (!Equals(field, value))
        {
            field = value;
            OnPropertyChanged(name);

            // Notify computed property dependents
            if (name == nameof(FirstName) || name == nameof(LastName))
                OnPropertyChanged(nameof(FullName));
        }
    }

    // INotifyPropertyChanged...
}
```

**Community Toolkit with ObservableProperty:**
```csharp
[ObservableObject]
public partial class PersonViewModel
{
    [ObservableProperty]
    string firstName;

    [ObservableProperty]
    string lastName;

    // Computed property with [NotifyPropertyChangedFor]
    [ObservableProperty]
    [NotifyPropertyChangedFor(nameof(FullName))]
    string firstName;

    public string FullName => $"{FirstName} {LastName}";
}
```

---

## 11. Build System & Compilation

### Single-Project Multi-Targeting

MAUI consolidates all platform targets into a single .csproj file:

```xml
<!-- MyApp.csproj -->
<Project Sdk="Microsoft.Maui.Sdk">
    <PropertyGroup>
        <TargetFrameworks>net10.0-ios;net10.0-android;net10.0-windows;net10.0-maccatalyst</TargetFrameworks>
        <UseMaui>true</UseMaui>
        <SingleProject>true</SingleProject>

        <!-- Platform-specific output paths -->
        <OutputType>Exe</OutputType>

        <!-- iOS Configuration -->
        <SupportedOSPlatformVersion Condition="$([MSBuild]::GetTargetPlatformIdentifier('$(TargetFramework)')) == 'ios'">11.0</SupportedOSPlatformVersion>

        <!-- Android Configuration -->
        <SupportedOSPlatformVersion Condition="$([MSBuild]::GetTargetPlatformIdentifier('$(TargetFramework)')) == 'android'">8.0</SupportedOSPlatformVersion>

        <!-- Windows Configuration -->
        <SupportedOSPlatformVersion Condition="$([MSBuild]::GetTargetPlatformIdentifier('$(TargetFramework)')) == 'windows'">10.0.19041.0</SupportedOSPlatformVersion>

        <!-- macOS Configuration -->
        <SupportedOSPlatformVersion Condition="$([MSBuild]::GetTargetPlatformIdentifier('$(TargetFramework)')) == 'maccatalyst'">13.1</SupportedOSPlatformVersion>
    </PropertyGroup>

    <!-- Dependencies -->
    <ItemGroup>
        <PackageReference Include="CommunityToolkit.Mvvm" Version="8.2.2" />
        <PackageReference Include="Microsoft.Maui.Controls" Version="10.0.0" />
    </ItemGroup>

    <!-- Platform-specific files -->
    <ItemGroup Condition="$([MSBuild]::GetTargetPlatformIdentifier('$(TargetFramework)')) == 'ios'">
        <BundleResource Include="Platforms/iOS/**/*" />
    </ItemGroup>

    <ItemGroup Condition="$([MSBuild]::GetTargetPlatformIdentifier('$(TargetFramework)')) == 'android'">
        <AndroidResource Include="Platforms/Android/**/*" />
    </ItemGroup>
</Project>
```

### Project Structure

```
MyApp/
├── Platforms/
│   ├── Android/
│   │   ├── AndroidManifest.xml
│   │   ├── MainActivity.cs
│   │   ├── Resources/
│   │   │   ├── values/
│   │   │   │   ├── colors.xml
│   │   │   │   └── strings.xml
│   │   │   └── drawable/
│   │   │       └── icon.png
│   │   └── Services/
│   │       └── PlatformService.Android.cs
│   ├── iOS/
│   │   ├── Info.plist
│   │   ├── Main.cs
│   │   ├── SceneDelegate.cs
│   │   ├── Assets.xcassets/
│   │   └── Services/
│   │       └── PlatformService.iOS.cs
│   ├── MacCatalyst/
│   │   └── Info.plist
│   └── Windows/
│       ├── App.xaml.cs
│       └── Package.appxmanifest
├── Resources/
│   ├── Fonts/
│   │   └── OpenSans-Regular.ttf
│   ├── Images/
│   │   └── icon.png
│   ├── Raw/
│   │   └── config.json
│   └── Styles/
│       └── Colors.xaml
├── Views/
│   └── MainPage.xaml
├── ViewModels/
│   └── MainViewModel.cs
├── Services/
│   └── DataService.cs
├── App.xaml
├── App.xaml.cs
├── AppShell.xaml
├── MauiProgram.cs
└── MyApp.csproj
```

### SDK Structure & Workloads

MAUI uses .NET Workloads for platform SDKs:

```bash
# Install MAUI workload
dotnet workload install maui

# Verify installation
dotnet workload list

# Install individual platform workloads
dotnet workload install ios android windows macos

# Update workloads
dotnet workload update
```

**Workload Contents:**
```
MAUI Workload
├── maui (core MAUI framework)
├── maui-ios (iOS SDK bindings)
├── maui-android (Android SDK bindings)
├── maui-windows (Windows SDK bindings)
└── maui-maccatalyst (macOS SDK bindings)
```

Each workload installs:
- Compiled MAUI libraries
- Platform SDK headers and bindings
- Templates and tooling

### Debug vs. Release Builds

**Debug Build:**
```bash
dotnet build -c Debug
```
- IL is not compiled; JIT compilation at runtime
- Debug symbols included (.pdb)
- Larger binary, slower startup
- Hot reload enabled

**Release Build:**
```bash
# Standard release (JIT on Android, AOT on iOS)
dotnet publish -c Release -f net10.0-android

# Release with NativeAOT (smaller, faster startup)
dotnet publish -c Release -f net10.0-android /p:PublishAot=true
```

- Optimized IL
- Debug symbols stripped
- Smaller binary (especially with NativeAOT)
- Faster startup (especially with AOT)
- No hot reload

**Platform-Specific Build Variants:**
```bash
# iOS (always AOT)
dotnet publish -c Release -f net10.0-ios

# Android with trimming
dotnet publish -c Release -f net10.0-android /p:PublishTrimmed=true

# Windows with single-file deployment
dotnet publish -c Release -f net10.0-windows /p:PublishSingleFile=true
```

---

## 12. Development Tools

### Visual Studio Integration

**Visual Studio 2022 (Recommended):**
- Full MAUI project templates
- XAML IntelliSense and designer preview
- Device Manager for Android emulators
- iOS Simulator launch
- Integrated debugger with breakpoints, watch windows, locals inspection

**Key Features:**
```
Create → New Project → .NET MAUI App
    ↓
Visual Studio scaffolds multi-targeting project
    ↓
Debug → Debug Android (or iOS, Windows, macOS)
    ↓
Debugger attaches; set breakpoints, inspect locals
```

### VS Code Extension

The .NET MAUI VS Code extension (now Generally Available as of 2024):
- XAML IntelliSense
- XAML Hot Reload (full reload of XAML on save)
- C# IntelliSense (via Omnisharp)
- Command palette for run/build commands

**Limitations:**
- No visual designer (compare to Visual Studio's XAML preview)
- C# Hot Reload is experimental

### Hot Reload

**XAML Hot Reload (Full Support):**
- Save XAML file
- Running app detects change
- Reloads XAML without rebuilding
- UI state is preserved (bound data remains)
- Works on: Android, iOS (via Hot Restart), WinUI, macOS

```xaml
<!-- Edit and save; app instantly reflects change -->
<Label Text="Updated Text"
       FontSize="24"
       TextColor="Blue" />
```

**C# Hot Reload (Limited):**
- Supported in Visual Studio 2022
- Experimental in VS Code
- Works for method bodies, property logic
- Does not support: type/member signature changes, new types

```csharp
// Edit method body; app recompiles and resumes
private int Calculate()
{
    int x = 10;
    int y = 20;
    return x + y; // Change to x * y; hot reloads
}
```

### iOS Hot Restart

Special for iOS: Hot Restart deploys to physical device without Mac build host (from Visual Studio on Windows):

```bash
# Requires iOS 15.1+
# Deploy and debug iOS app from Windows without Xcode
dotnet build -f net10.0-ios -c Debug
# App runs on iPhone with breakpoints attached to Visual Studio
```

### Debugging Experience

**Breakpoints & Inspection:**
```csharp
public async Task OnLoadClicked()
{
    // Set breakpoint here
    var data = await FetchDataAsync();

    // Watch window: inspect 'data' properties
    Items = new ObservableCollection<Item>(data);
}
```

**Multi-Target Debugging:**
- Debug Android, iOS, Windows, macOS from the same codebase
- Step through shared code, see platform-specific branches

**Remote Debugging:**
- iOS: Via Xcode debugger bridge
- Android: Via Android SDK debugger
- Windows/macOS: Local debugging

---

## 13. Performance Characteristics

### App Size

**Typical Sizes (Release Build):**

| Platform | Minimal App | Typical App | With ASP.NET Core |
|----------|------------|------------|------------------|
| **Android APK** | 5-9 MB | 15-30 MB | 50-80 MB |
| **iOS IPA** | 15-20 MB | 25-40 MB | 60-100 MB |
| **Windows MSIX** | 20-40 MB | 40-80 MB | 80-150 MB |
| **macOS DMG** | 30-50 MB | 50-100 MB | 100-180 MB |

**Comparison to React Native:**
- MAUI: 9 MB (APK)
- React Native: 54.5 MB (APK)

**Size Optimization Techniques:**
1. **Trimming:** Remove unused code (`/p:PublishTrimmed=true`)
2. **NativeAOT:** Compile to native binary (`/p:PublishAot=true`)
3. **Single-File:** Package runtime and assemblies into single executable

### Startup Time

**Typical Cold Start Times:**

| Platform | JIT Build | NativeAOT Build |
|----------|-----------|-----------------|
| **Android** | 2-4 seconds | 0.5-1 second |
| **iOS** | 0.5-1 second | (Always AOT) |
| **Windows** | 1-2 seconds | 0.3-0.5 second |
| **macOS** | 1-2 seconds | 0.3-0.5 second |

**Android Startup Improvements (.NET 6+):**
- QuickJIT: Faster initial JIT compilation
- Tiered JIT: Optimize hot paths after initial execution
- Reduction: 5-10% improvement over .NET 5

**iOS Startup:**
- Always AOT; near-instant startup
- Limitation: Larger app binary

### Memory Overhead

**Typical Memory Footprint:**

| Component | RAM Usage |
|-----------|-----------|
| **.NET Runtime (CLR)** | 15-20 MB |
| **MAUI Framework** | 10-15 MB |
| **Android Emulator (base)** | 1-2 GB |
| **iOS Simulator (base)** | 2-3 GB |
| **Typical App (idle)** | 50-100 MB |

**Memory Optimization:**
- Use `ObservableCollection` for large lists (virtualizes off-screen items)
- Dispose heavy resources (HttpClient, DbContext)
- Avoid large image allocations in render loops
- Profile with Xamarin Profiler or Android Studio Profiler

### GC Pause Impact

**Garbage Collection Pauses:**
- Workstation GC (default): 1-10 ms pauses
- Large object collections: 50-200 ms (rare)
- Impact on UI: Frame drops if pause > 16 ms (60 FPS target)

**Mitigation:**
- Avoid large object allocations on hot path
- Pool frequently allocated objects
- Use object reuse patterns (StringBuilder instead of string concatenation)

### Runtime Performance vs. Native

**Performance Parity:**
- **CPU-Bound:** MAUI is 80-95% of native C++ speed (JIT optimizations)
- **I/O-Bound:** MAUI ≈ native (async/await is not slower)
- **Rendering:** MAUI ≈ native (direct platform control mapping)

**MAUI Overhead:**
- Binding overhead: ~1-2% per data binding evaluation
- Handler mapping: Negligible (one-time at control creation)
- GC pauses: Occasional, but manageable

---

## 14. Ecosystem & Packages

### NuGet Package Ecosystem

**.NET MAUI has access to the full NuGet ecosystem:**

**Official Microsoft Packages:**
- `Microsoft.Maui.Controls` (core framework)
- `Microsoft.Maui.Essentials` (sensors, storage, notifications)
- `CommunityToolkit.Mvvm` (MVVM helpers)
- `CommunityToolkit.Maui` (additional controls, animations)

**Third-Party Provider Packages:**
- **Syncfusion:** Full UI control suite (DataGrid, Charts, Calendar, etc.)
- **Telerik:** Advanced controls (ImageEditor, Scheduler, etc.)
- **DevExpress:** Enterprise controls
- **Sharpnado:** Community controls (Tabs, Carousel, etc.)

**Popular Community Packages:**
```xml
<ItemGroup>
    <!-- HTTP & API -->
    <PackageReference Include="Refit" Version="6.3.2" /> <!-- Declarative HTTP client -->
    
    <!-- Data & Storage -->
    <PackageReference Include="Realm" Version="11.4.0" /> <!-- Mobile database -->
    <PackageReference Include="LiteDB" Version="5.0.16" /> <!-- Embedded document DB -->
    
    <!-- Logging & Diagnostics -->
    <PackageReference Include="Serilog" Version="3.0.1" /> <!-- Structured logging -->
    
    <!-- JSON -->
    <PackageReference Include="System.Text.Json" Version="8.0.0" /> <!-- Native JSON -->
    <PackageReference Include="Newtonsoft.Json" Version="13.0.3" /> <!-- Alternative -->
    
    <!-- Compression -->
    <PackageReference Include="SharpZipLib" Version="1.4.2" />
    
    <!-- Imaging -->
    <PackageReference Include="SkiaSharp" Version="2.88.6" /> <!-- 2D graphics -->
    
    <!-- Maps & Location -->
    <PackageReference Include="Microsoft.Maui.Controls.Maps" Version="10.0.0" />
</ItemGroup>
```

### Compatibility with Xamarin.Forms Packages

**Migration Path:**
- Most Xamarin.Forms packages have MAUI equivalents
- Some packages require recompilation for .NET 6+ target framework
- Breaking changes: API surface may differ; requires updating code

**Example Migration:**
```csharp
// Xamarin.Forms (old)
using Xamarin.Forms;
using Xamarin.Forms.PlatformConfiguration.iOSSpecific;

// MAUI (new)
using Microsoft.Maui.Controls;
using Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific;
```

### Community Activity

**Community Size:**
- **GitHub:** dotnet/maui has 20K+ stars, active issue tracking
- **Stack Overflow:** 5K+ questions tagged with `maui`
- **Discord/Forums:** Active community support
- **Blog Posts:** Regular content on cross-platform development
- **NuGet:** 100+ MAUI-specific packages

**Contributing:**
- MAUI is open-source; contributions are welcome
- Community Toolkit is the official avenue for community contributions

---

## 15. Design Philosophy & Microsoft Strategy

### Why Microsoft Unified Xamarin into MAUI

**Motivation:**
1. **Competitive Pressure:** Flutter and React Native captured market share; Xamarin was fragmented
2. **Ecosystem Consolidation:** Single .NET platform across web, mobile, desktop
3. **Modern C#:** .NET 5+ features (async/await, nullable types, pattern matching) are better leveraged in greenfield design
4. **Developer Experience:** Single project, hot reload, modern tooling

### Cross-Platform as Primary Goal

**MAUI's Philosophy:**
- One codebase for all platforms by default
- Platform-specific customization via conditional compilation or platform-specific handlers
- UI abstraction is paramount; exact platform parity is secondary

**Trade-off:** 
- Achieves high code reuse (~95% shared code is common)
- Sacrifices some platform-specific optimizations and UX polish
- Developers can drop to native code when needed

### .NET Strategy & Long-Term Direction

**Microsoft's .NET Vision:**
1. **Unified Ecosystem:** Single runtime, tooling, and package ecosystem
2. **Cloud to Edge:** .NET for cloud (Azure), web (ASP.NET Core), desktop (WinUI 3), and mobile (MAUI)
3. **Performance Leadership:** NativeAOT, trimming, and optimization tools
4. **Developer Velocity:** Hot reload, modern syntax, productive libraries

**.NET Roadmap (2026-2027):**
- **NativeAOT Expansion:** Better support on mobile platforms
- **GitHub Copilot Integration:** Copilot for MAUI development
- **Diagnostics Improvements:** Better debugging and profiling
- **Performance Optimization:** Continued improvements to startup time and memory

### Enterprise Focus

**Microsoft's Enterprise Positioning:**
- **Support:** Professional support for MAUI via Microsoft
- **Tooling:** Visual Studio integration for enterprises
- **Security:** Compliance with enterprise requirements (TLS, authentication, data privacy)
- **Longevity:** Long-term support (LTS) versions of .NET (every 3 years)

**Enterprise Use Cases:**
- Line-of-business (LOB) applications
- Internal tools and utilities
- Cross-platform mobile apps for employee-facing services

---

## 16. Limitations & Trade-Offs

### Mobile Support Maturity

**MAUI is Mobile-First but Desktop-Optimized:**
- **iOS:** Well-supported, performance is excellent
- **Android:** Well-supported, large device fragmentation requires testing
- **macOS:** Uses Mac Catalyst (iOS app runtime); lacks native AppKit support; performance issues reported
- **Windows:** Full support via WinUI 3; good performance
- **Linux:** Minimal support; not a primary target

**Desktop-First Limitations on Mobile:**
If you design for desktop first and then mobile:
- Controls may be too large/too small for mobile screens
- Touch interaction patterns differ from mouse/keyboard
- Battery life and memory constraints are stricter on mobile

### App Size & Startup Time Trade-Offs

**APK/IPA Size:**
- MAUI: 9-30 MB (reasonable)
- React Native: 50+ MB (heavier)
- Native Swift/Kotlin: 5-15 MB (smaller)

**Startup Time:**
- MAUI (JIT): 2-4 seconds (Android)
- MAUI (AOT/NativeAOT): 0.5-1 second
- Native: 0.5-2 seconds

**Trade-off:** MAUI adds some overhead vs. native but is competitive with other cross-platform frameworks.

### Platform Parity & Feature Gaps

**MAUI Abstractions Hide Differences:**
- Button behavior differs between platforms (hold vs. tap)
- Keyboard handling varies (Return key names)
- Fonts availability differs
- File system access varies

**Mitigation:**
1. Test on real devices (not just emulators)
2. Customize handlers for platform-specific behavior
3. Use conditional compilation for platform-specific logic
4. Accept minor UX inconsistencies

### Learning Curve

**Required Knowledge:**
1. **C# Language:** Solid understanding of async/await, generics, LINQ
2. **XAML Markup:** Declarative UI syntax, binding expressions
3. **.NET Fundamentals:** CLR, NuGet, project structure
4. **MVVM Pattern:** Data binding, view models, commands
5. **Platform APIs:** Each platform's native capabilities (when customizing)

**Steepest Curves:**
- Async/await (if coming from synchronous languages)
- XAML syntax and binding expressions
- Platform-specific customization (native bridging)

---

## Conclusion

**.NET MAUI** is a production-ready, Microsoft-backed cross-platform framework that enables developers to build native iOS, Android, macOS, and Windows applications from a single C# codebase. It represents Microsoft's strategic consolidation of the Xamarin ecosystem and their long-term investment in cross-platform development.

### Key Strengths

1. **Single Codebase:** One project for all platforms
2. **Modern C#:** Full access to C# 12/13 features, async/await, LINQ
3. **Strong Ecosystem:** Access to full NuGet ecosystem
4. **Enterprise Backing:** Microsoft support, long-term stability
5. **Developer Productivity:** Hot reload, excellent tooling, MVVM support

### Key Limitations

1. **macOS via Catalyst:** Not native, performance concerns
2. **Linux Support:** Minimal
3. **Platform Parity:** Some features require platform-specific customization
4. **Learning Curve:** Requires understanding of async/await, XAML, MVVM

### When to Choose MAUI

- **✓ Best For:** Cross-platform mobile/desktop apps, enterprise LOB applications, teams familiar with C#/.NET
- **✗ Avoid If:** Platform-native performance is critical, targeting Linux, or learning cross-platform development for the first time

### Embedded Server Capabilities

Embedding ASP.NET Core in MAUI is **technically possible but not recommended** for typical scenarios. The overhead (50-100 MB) and complexity (separate process management, network serialization) outweigh the benefits when business logic can live in MAUI ViewModels. Consider embedded servers only for multi-user or true server-client architectures.

---

## Resources & Further Reading

### Official Microsoft Documentation
- [What is .NET MAUI?](https://learn.microsoft.com/en-us/dotnet/maui/what-is-maui)
- [.NET MAUI Architecture](https://learn.microsoft.com/en-us/dotnet/maui/architecture)
- [XAML Fundamentals](https://learn.microsoft.com/en-us/dotnet/maui/xaml/fundamentals/)
- [Data Binding & MVVM](https://learn.microsoft.com/en-us/dotnet/maui/xaml/fundamentals/mvvm)

### Community Resources
- [.NET MAUI Community Toolkit](https://github.com/CommunityToolkit/Maui)
- [MAUI GitHub Repository](https://github.com/dotnet/maui)
- [Syncfusion MAUI Controls](https://www.syncfusion.com/maui-controls)

### Learning & Blogs
- [.NET Blog (MAUI)](https://devblogs.microsoft.com/dotnet)
- [MAUI 2025 Development Guide](https://www.itpathsolutions.com/dotnet-maui-development-guide)
- [Comparing MAUI vs React Native](https://uxcam.com/blog/net-maui-vs-react-native)

---

**Document Version:** 1.0  
**Last Updated:** May 2026  
**Accuracy:** Based on .NET 10, MAUI 10.0, and published March-May 2026 sources
