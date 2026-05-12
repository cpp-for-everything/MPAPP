// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0009-public-api-template-wrappers-only.md
//
// Minimal intrusive signal/slot.
//
// Design constraints (from T-0002 and ADR-0009):
//   - No std::function (no type-erased heap allocation in the hot path).
//   - Subscription lifetime is owned by the subscriber via an embedded node.
//     The slot node owns no heap memory; it is just a linked-list link plus
//     a function pointer and a captured 'this' pointer (void*).
//   - Auto-unsubscribe on slot destruction (RAII).
//   - emit() walks the intrusive list and dispatches via the stored thunk;
//     no allocations, no virtual calls.

#ifndef MPAPP_SIGNAL_HPP
#define MPAPP_SIGNAL_HPP

#include <cstddef>

namespace mpapp {

template <class... Args>
class signal;

// A slot is the subscriber-owned node. Subscribers embed one of these per
// connection they want to keep. Its destructor unsubscribes if connected.
template <class... Args>
class signal_slot {
public:
    signal_slot() noexcept = default;

    signal_slot(const signal_slot&) = delete;
    signal_slot& operator=(const signal_slot&) = delete;

    // Move would require updating the signal's prev_->next pointer, which is
    // not needed for the spike. Disallow.
    signal_slot(signal_slot&&) = delete;
    signal_slot& operator=(signal_slot&&) = delete;

    ~signal_slot() { disconnect(); }

    bool connected() const noexcept { return owner_ != nullptr; }

    void disconnect() noexcept {
        if (owner_ == nullptr) {
            return;
        }
        // Unlink from the doubly-linked-via-prev_next_ list. When connected,
        // prev_next_ always points to either the signal's head_ field or the
        // previous slot's next_, so no nullptr check is needed here.
        *prev_next_ = next_;
        if (next_ != nullptr) {
            next_->prev_next_ = prev_next_;
        }
        owner_ = nullptr;
    }

private:
    friend class signal<Args...>;

    using thunk_t = void (*)(void* captured, Args... args);

    signal<Args...>* owner_     = nullptr;
    signal_slot**    prev_next_ = nullptr; // address of the pointer that targets us
    signal_slot*     next_      = nullptr;
    thunk_t          thunk_     = nullptr;
    void*            captured_  = nullptr;
};

template <class... Args>
class signal {
public:
    using slot_type = signal_slot<Args...>;

    signal() noexcept = default;

    signal(const signal&)            = delete;
    signal& operator=(const signal&) = delete;
    signal(signal&&)                 = delete;
    signal& operator=(signal&&)      = delete;

    ~signal() {
        // Detach every still-live slot so its destructor becomes a no-op.
        for (slot_type* s = head_; s != nullptr; s = s->next_) {
            s->owner_ = nullptr;
        }
    }

    // Subscribe `slot` to this signal. `fn` may be a lambda, free function,
    // or any callable; it is stored by reference-of-callable inside the slot
    // node via a generated thunk. The slot must outlive the connection (or
    // be explicitly disconnect()ed). The callable F must outlive the slot.
    template <class F>
    void subscribe(slot_type& slot, F& fn) noexcept {
        connect_impl(slot, &fn, +[](void* captured, Args... args) {
            (*static_cast<F*>(captured))(static_cast<Args>(args)...);
        });
    }

    // Overload for rvalue callable — accept it by reference so the user
    // can write `sig.subscribe(slot, [](...){ ... });` only when the
    // lambda is stored elsewhere. For temporaries, prefer the form that
    // takes a function pointer.
    void subscribe(slot_type& slot, void (*fp)(Args...)) noexcept {
        connect_impl(slot, reinterpret_cast<void*>(fp),
                     +[](void* captured, Args... args) {
                         auto* f = reinterpret_cast<void (*)(Args...)>(captured);
                         f(static_cast<Args>(args)...);
                     });
    }

    void emit(Args... args) const {
        // Snapshot `next_` before invoking the thunk so a slot can safely
        // disconnect itself inside its own callback.
        slot_type* cur = head_;
        while (cur != nullptr) {
            slot_type* next = cur->next_;
            cur->thunk_(cur->captured_, static_cast<Args>(args)...);
            cur = next;
        }
    }

    std::size_t subscriber_count() const noexcept {
        std::size_t n = 0;
        for (slot_type* p = head_; p != nullptr; p = p->next_) {
            ++n;
        }
        return n;
    }

private:
    void connect_impl(slot_type& slot, void* captured,
                      typename slot_type::thunk_t thunk) noexcept {
        slot.disconnect();
        slot.owner_     = this;
        slot.thunk_     = thunk;
        slot.captured_  = captured;
        slot.prev_next_ = &head_;
        slot.next_      = head_;
        if (head_ != nullptr) {
            head_->prev_next_ = &slot.next_;
        }
        head_ = &slot;
    }

    slot_type* head_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_SIGNAL_HPP
