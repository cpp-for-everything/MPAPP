// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::contacts` — read-only access to the device contact list plus an
// interactive single-contact picker. Counterpart to MAUI Essentials
// `Contacts`. Abstract interface + an in-memory mock implementation whose
// contact list and canned pick result are fully settable so tests can drive
// every code path without a real device. Real per-platform backends
// (Android ContactsContract, iOS CNContactStore, Windows People API)
// implement the same interface and are injected via the DI container
// (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_CONTACTS_HPP
#define MPAPP_ESSENTIALS_CONTACTS_HPP

#include <optional>
#include <string>
#include <vector>

namespace mpapp {

// A single contact record (mirrors MAUI's Contact).
struct contact {
    std::string display_name{};
    std::string given_name{};
    std::string family_name{};
    std::vector<std::string> emails{};
    std::vector<std::string> phones{};

    bool operator==(const contact&) const = default;
};

// Abstract contacts interface.
class contacts {
public:
    virtual ~contacts() = default;

    // Returns all available contacts. May return an empty list if none are
    // present or if access was denied by the platform.
    [[nodiscard]] virtual std::vector<contact> get_all() const = 0;

    // Presents the platform contact-picker UI and returns the selected
    // contact, or std::nullopt if the user cancelled or the feature is not
    // supported on this platform.
    virtual std::optional<contact> pick() = 0;
};

// Mock / in-memory implementation. State is fully settable so unit tests
// can drive every code path:
//   - set_contacts(...)  replaces the canned contact list returned by get_all().
//   - set_pick_result(…) sets the contact returned by the next pick() call.
//   - pick_count()       returns the number of times pick() has been called.
class mock_contacts final : public contacts {
public:
    mock_contacts() = default;

    // ---- Settable test state -----------------------------------------------

    // Replace the canned list that get_all() returns.
    void set_contacts(std::vector<contact> list) {
        contacts_ = std::move(list);
    }

    // Set the contact that the next pick() will return. Pass std::nullopt to
    // simulate the user cancelling the picker or a not-supported platform.
    void set_pick_result(std::optional<contact> result) {
        pick_result_ = std::move(result);
    }

    // ---- Observation --------------------------------------------------------

    // Number of times pick() has been called since construction (or last reset).
    [[nodiscard]] int pick_count() const noexcept { return pick_count_; }

    // ---- contacts interface -------------------------------------------------

    [[nodiscard]] std::vector<contact> get_all() const override {
        return contacts_;
    }

    std::optional<contact> pick() override {
        ++pick_count_;
        return pick_result_;
    }

private:
    std::vector<contact>    contacts_{};
    std::optional<contact>  pick_result_{ std::nullopt };
    int                     pick_count_{ 0 };
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_CONTACTS_HPP
