// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0002-no-macros-in-public-api.md
//
// messenger — typed publish/subscribe message bus, mirroring
// CommunityToolkit.Mvvm's IMessenger / WeakReferenceMessenger semantics.
//
// Design constraints:
//   - No macros in the public API (ADR-0002).
//   - Header-only.
//   - Recipients are identified by an opaque address (const void*); the
//     messenger does not own recipient lifetimes (callers must unregister).
//   - Handlers are keyed per message type (std::type_index) and per
//     recipient. Multiple recipients may register for the same message type;
//     send() invokes every registered handler for that type.

#ifndef MPAPP_MVVM_MESSENGER_HPP
#define MPAPP_MVVM_MESSENGER_HPP

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mpapp {

// A typed message bus. Recipients register a handler for a given message type
// and are notified whenever a message of that type is sent.
class messenger {
public:
    messenger() = default;

    messenger(const messenger&)            = delete;
    messenger& operator=(const messenger&) = delete;
    messenger(messenger&&)                 = default;
    messenger& operator=(messenger&&)      = default;

    ~messenger() = default;

    // Register `recipient`'s handler for messages of type TMessage. If the
    // recipient already has a handler registered for this type, it is
    // replaced (last registration wins), matching IMessenger semantics.
    template <class TMessage>
    void register_handler(const void*                        recipient,
                          std::function<void(const TMessage&)> handler) {
        auto& entries = channels_[std::type_index(typeid(TMessage))];
        for (auto& entry : entries) {
            if (entry.recipient == recipient) {
                entry.invoke = wrap<TMessage>(std::move(handler));
                return;
            }
        }
        entries.push_back(entry_t{ recipient, wrap<TMessage>(std::move(handler)) });
    }

    // Deliver `message` to every recipient registered for its type. A copy of
    // the recipient list is taken first so a handler may safely (un)register
    // during dispatch without invalidating the iteration.
    template <class TMessage>
    void send(const TMessage& message) const {
        const auto it = channels_.find(std::type_index(typeid(TMessage)));
        if (it == channels_.end()) {
            return;
        }
        const std::vector<entry_t> snapshot = it->second;
        for (const auto& entry : snapshot) {
            entry.invoke(&message);
        }
    }

    // Remove `recipient`'s handler for messages of type TMessage. A no-op if
    // the recipient was not registered for that type.
    template <class TMessage>
    void unregister(const void* recipient) {
        const auto it = channels_.find(std::type_index(typeid(TMessage)));
        if (it == channels_.end()) {
            return;
        }
        erase_recipient(it->second, recipient);
        if (it->second.empty()) {
            channels_.erase(it);
        }
    }

    // Remove every handler `recipient` registered across all message types.
    void unregister_all(const void* recipient) {
        for (auto it = channels_.begin(); it != channels_.end();) {
            erase_recipient(it->second, recipient);
            if (it->second.empty()) {
                it = channels_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // True iff `recipient` currently has a handler registered for TMessage.
    template <class TMessage>
    [[nodiscard]] bool is_registered(const void* recipient) const {
        const auto it = channels_.find(std::type_index(typeid(TMessage)));
        if (it == channels_.end()) {
            return false;
        }
        for (const auto& entry : it->second) {
            if (entry.recipient == recipient) {
                return true;
            }
        }
        return false;
    }

private:
    using invoke_t = std::function<void(const void*)>;

    struct entry_t {
        const void* recipient = nullptr;
        invoke_t    invoke;
    };

    template <class TMessage>
    static invoke_t wrap(std::function<void(const TMessage&)> handler) {
        return [h = std::move(handler)](const void* message) {
            h(*static_cast<const TMessage*>(message));
        };
    }

    static void erase_recipient(std::vector<entry_t>& entries,
                                const void*           recipient) {
        for (auto e = entries.begin(); e != entries.end();) {
            if (e->recipient == recipient) {
                e = entries.erase(e);
            } else {
                ++e;
            }
        }
    }

    std::unordered_map<std::type_index, std::vector<entry_t>> channels_;
};

} // namespace mpapp

#endif // MPAPP_MVVM_MESSENGER_HPP
