---
type: moc
area: properties
tags:
  - area/properties
  - area/type-system
---

# Observable Properties

`Observable<T>` is the foundation of MPAPP's MVVM layer. Every property bindable from XAML or from C++ `bind(&...)` expressions is an `Observable<T>`.

Per [[ADR-0009-public-api-template-wrappers-only]] this is the **only** mechanism; no macros, no attributes.

## Anatomy

```cpp
template <class T>
class Observable {
public:
    Observable() = default;
    explicit Observable(T value) : value_(std::move(value)) {}

    // Reads
    const T& get() const noexcept { return value_; }
    operator const T&() const noexcept { return value_; }   // implicit conversion

    // Writes
    void set(T value) {
        if (!(value == value_)) {
            value_ = std::move(value);
            changed.emit(value_);                            // intrusive signal
        }
    }
    Observable& operator=(T value) { set(std::move(value)); return *this; }

    // Subscription (intrusive — caller owns the node)
    mpapp::signal<const T&> changed;

private:
    T value_{};
};
```

## Intrusive signal/slot

Subscriptions are **intrusive nodes embedded in the subscriber**, not heap-allocated `std::function` lists. This means:

- No allocations on subscribe/unsubscribe.
- No `shared_ptr` cycles.
- Subscription lifetime is owned by the subscriber (RAII; auto-unsubscribe on destruction).

```cpp
struct SomeView {
    mpapp::signal_slot<const std::string&> name_changed_slot;
    void connect(todo_view_model& vm) {
        vm.name.changed.connect(name_changed_slot, [this](auto& v) {
            label.text = v;
        });
    }
    // name_changed_slot destructor auto-unsubscribes
};
```

## Computed properties

A `Computed<&Member, ...>` parameter on a member function declares dependencies:

```cpp
auto display(mpapp::Computed<&todo_view_model::count,
                             &todo_view_model::name> = {}) const
    { return std::format("{}: {}", name.get(), count.get()); }
```

The framework:

1. Detects `Computed<...>` in the function's parameter list at template-instantiation time.
2. Subscribes to each listed `Observable`'s `changed` signal.
3. Re-invokes the function when any dependency fires.
4. Exposes the result as a derived observable.

## Commands

`Command<Args...>` is the same trick for methods that should be bindable from XAML `Command="..."`:

```cpp
void increment(mpapp::Command<> = {}) { count.set(count + 1); }
```

The framework recognizes `Command<>` and exposes the method as a `mpapp::command` view bindable from C++ `bind(&VM::increment)` or XAML `Command="{Binding increment}"`.

`Command<Args...>` with non-empty `Args` accepts arguments (XAML `CommandParameter` or C++ direct call).

## Collection notifications

`mpapp::observable_vector<T>` emits **delta** signals (inserted / erased / replaced / moved), not just a blunt `changed`. This lets virtualized list controls (`CollectionView`, `ListView`) update incrementally — equivalent to MAUI's `ObservableCollection<T>` but with better granularity.

```cpp
observable_vector<todo_item> items;
items.inserted.connect(slot, [this](size_t i, const todo_item& it) { /* ... */ });
items.erased.connect(slot, [this](size_t i) { /* ... */ });
```

## Validation

`MPAPP_VALIDATE` — wait, no macros. We use a member type:

```cpp
class todo_view_model {
    mpapp::Observable<std::string> email{""};
    mpapp::Validates<&todo_view_model::email,
                     mpapp::is_email_format> email_validator{};
};
```

The presence of `Validates<&member, predicate>` makes `email` reflect validation state via `email.validation` (an observable `error_state`).

## See also

- [[Type System]]
- [[No Macros In Public API]]
- [[ADR-0009-public-api-template-wrappers-only]]
- [[Handlers]] (consumes Observable for native property updates)
