// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::secure_storage` — encrypted key/value store. Counterpart to
// MAUI Essentials `SecureStorage`. Interface + in-memory mock; real
// backends (Windows DataProtection/PasswordVault, Linux libsecret,
// Android Keystore-backed EncryptedSharedPreferences, Apple Keychain)
// implement the same interface, injected via DI (RFC-0011). MAUI's API
// is async; the mock surface is synchronous (the per-platform real
// layer can return a task<>). No macros.

#ifndef MPAPP_ESSENTIALS_SECURE_STORAGE_HPP
#define MPAPP_ESSENTIALS_SECURE_STORAGE_HPP

#include <optional>
#include <string>
#include <unordered_map>

namespace mpapp {

class secure_storage {
public:
    virtual ~secure_storage() = default;

    virtual void set(const std::string& key, const std::string& value)        = 0;
    [[nodiscard]] virtual std::optional<std::string>
        get(const std::string& key) const                                    = 0;
    virtual bool remove(const std::string& key)                               = 0;  // true if existed
    virtual void remove_all()                                                 = 0;
    [[nodiscard]] virtual bool contains(const std::string& key) const         = 0;
};

// Default + mock implementation: process-memory backed (NOT encrypted —
// stand-in for the secure native store; never use for real secrets).
class in_memory_secure_storage final : public secure_storage {
public:
    void set(const std::string& key, const std::string& value) override {
        store_[key] = value;
    }
    [[nodiscard]] std::optional<std::string> get(const std::string& key) const override {
        auto it = store_.find(key);
        return it == store_.end() ? std::nullopt : std::optional<std::string>{ it->second };
    }
    bool remove(const std::string& key) override { return store_.erase(key) > 0; }
    void remove_all() override { store_.clear(); }
    [[nodiscard]] bool contains(const std::string& key) const override {
        return store_.find(key) != store_.end();
    }

private:
    std::unordered_map<std::string, std::string> store_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_SECURE_STORAGE_HPP
