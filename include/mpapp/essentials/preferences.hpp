// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::preferences` — a typed key/value settings store. Counterpart
// to MAUI Essentials `Preferences`. Abstract interface + an in-memory
// implementation (the mock / default). Real per-platform backends
// (Windows registry / ApplicationData, Linux GSettings, Android
// SharedPreferences) implement the same interface and are injected via
// the DI container (RFC-0011). Values are string-backed with typed
// convenience accessors. No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_PREFERENCES_HPP
#define MPAPP_ESSENTIALS_PREFERENCES_HPP

#include <optional>
#include <string>
#include <unordered_map>

namespace mpapp {

class preferences {
public:
    virtual ~preferences() = default;

    // ---- Raw string store (the primitive every typed accessor maps to) --
    virtual void set_string(const std::string& key, const std::string& value) = 0;
    [[nodiscard]] virtual std::optional<std::string>
        get_string(const std::string& key) const = 0;
    virtual void remove(const std::string& key)        = 0;
    virtual void clear()                               = 0;
    [[nodiscard]] virtual bool contains(const std::string& key) const = 0;

    // ---- Typed convenience (default-on-miss, MAUI's Get(key, default)) ---
    [[nodiscard]] std::string get(const std::string& key, const std::string& fallback) const {
        auto v = get_string(key);
        return v ? *v : fallback;
    }
    [[nodiscard]] long get(const std::string& key, long fallback) const {
        auto v = get_string(key);
        if (!v) return fallback;
        try { return std::stol(*v); } catch (...) { return fallback; }
    }
    [[nodiscard]] double get(const std::string& key, double fallback) const {
        auto v = get_string(key);
        if (!v) return fallback;
        try { return std::stod(*v); } catch (...) { return fallback; }
    }
    [[nodiscard]] bool get(const std::string& key, bool fallback) const {
        auto v = get_string(key);
        if (!v) return fallback;
        return *v == "1" || *v == "true";
    }

    void set(const std::string& key, const std::string& value) { set_string(key, value); }
    void set(const std::string& key, const char* value)        { set_string(key, value); }
    void set(const std::string& key, long value)               { set_string(key, std::to_string(value)); }
    void set(const std::string& key, double value)             { set_string(key, std::to_string(value)); }
    void set(const std::string& key, bool value)               { set_string(key, value ? "1" : "0"); }
};

// Default + mock implementation: process-memory backed.
class in_memory_preferences final : public preferences {
public:
    void set_string(const std::string& key, const std::string& value) override {
        store_[key] = value;
    }
    [[nodiscard]] std::optional<std::string>
    get_string(const std::string& key) const override {
        auto it = store_.find(key);
        return it == store_.end() ? std::nullopt : std::optional<std::string>{ it->second };
    }
    void remove(const std::string& key) override { store_.erase(key); }
    void clear() override { store_.clear(); }
    [[nodiscard]] bool contains(const std::string& key) const override {
        return store_.find(key) != store_.end();
    }

private:
    std::unordered_map<std::string, std::string> store_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_PREFERENCES_HPP
