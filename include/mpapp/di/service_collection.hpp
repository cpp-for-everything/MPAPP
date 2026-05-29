// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0011-dependency-injection.md
//
// `mpapp::service_collection` + `mpapp::service_provider` — a minimal
// dependency-injection container. Counterpart to MAUI's
// `IServiceCollection` / `IServiceProvider` (which sit under
// MauiAppBuilder). C++ has no reflection, so constructor injection is
// expressed through explicit factories (`add_singleton<T>([](auto& sp){
// return make_shared<T>(sp.get_required<Dep>()); })`); convenience
// overloads cover default-constructible services + interface→impl
// registration. Singletons are cached; transients are created per
// resolve. No macros (ADR-0009); header-only; platform-neutral.

#ifndef MPAPP_DI_SERVICE_COLLECTION_HPP
#define MPAPP_DI_SERVICE_COLLECTION_HPP

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace mpapp {

class service_provider;

enum class service_lifetime { singleton, transient };

namespace detail {
struct service_registration {
    service_lifetime lifetime = service_lifetime::singleton;
    std::function<std::shared_ptr<void>(service_provider&)> factory{};
    std::shared_ptr<void> singleton_instance{};  // cached for singletons
};
} // namespace detail

class service_collection {
public:
    service_collection() = default;

    // Singleton via factory (the constructor-injection path: the factory
    // pulls its dependencies from the passed service_provider).
    template <class T>
    service_collection& add_singleton(std::function<std::shared_ptr<T>(service_provider&)> factory) {
        regs_[std::type_index(typeid(T))] =
            make_reg<T>(service_lifetime::singleton, std::move(factory));
        return *this;
    }

    // Singleton from a pre-built instance.
    template <class T>
    service_collection& add_singleton(std::shared_ptr<T> instance) {
        detail::service_registration r;
        r.lifetime            = service_lifetime::singleton;
        r.singleton_instance  = std::static_pointer_cast<void>(instance);
        r.factory             = [inst = std::move(instance)](service_provider&) {
            return std::static_pointer_cast<void>(inst);
        };
        regs_[std::type_index(typeid(T))] = std::move(r);
        return *this;
    }

    // Singleton, default-constructed.
    template <class T>
        requires std::is_default_constructible_v<T>
    service_collection& add_singleton() {
        return add_singleton<T>(std::function<std::shared_ptr<T>(service_provider&)>{
            [](service_provider&) { return std::make_shared<T>(); } });
    }

    // Interface -> implementation (Impl default-constructible, derives Iface).
    template <class Iface, class Impl>
        requires (std::is_base_of_v<Iface, Impl> && std::is_default_constructible_v<Impl>)
    service_collection& add_singleton() {
        return add_singleton<Iface>(std::function<std::shared_ptr<Iface>(service_provider&)>{
            [](service_provider&) -> std::shared_ptr<Iface> { return std::make_shared<Impl>(); } });
    }

    // Transient via factory.
    template <class T>
    service_collection& add_transient(std::function<std::shared_ptr<T>(service_provider&)> factory) {
        regs_[std::type_index(typeid(T))] =
            make_reg<T>(service_lifetime::transient, std::move(factory));
        return *this;
    }

    // Transient, default-constructed.
    template <class T>
        requires std::is_default_constructible_v<T>
    service_collection& add_transient() {
        return add_transient<T>(std::function<std::shared_ptr<T>(service_provider&)>{
            [](service_provider&) { return std::make_shared<T>(); } });
    }

    [[nodiscard]] service_provider build();

private:
    template <class T>
    static detail::service_registration
    make_reg(service_lifetime lt, std::function<std::shared_ptr<T>(service_provider&)> f) {
        detail::service_registration r;
        r.lifetime = lt;
        r.factory  = [fn = std::move(f)](service_provider& sp) -> std::shared_ptr<void> {
            return std::static_pointer_cast<void>(fn(sp));
        };
        return r;
    }

    std::unordered_map<std::type_index, detail::service_registration> regs_{};
    friend class service_provider;
};

class service_provider {
public:
    // Resolve T. Singletons are created once + cached; transients are
    // created per call. Returns nullptr when T is not registered.
    template <class T>
    [[nodiscard]] std::shared_ptr<T> get() {
        auto it = regs_.find(std::type_index(typeid(T)));
        if (it == regs_.end()) {
            return nullptr;
        }
        detail::service_registration& reg = it->second;
        if (reg.lifetime == service_lifetime::singleton) {
            if (!reg.singleton_instance) {
                reg.singleton_instance = reg.factory(*this);
            }
            return std::static_pointer_cast<T>(reg.singleton_instance);
        }
        return std::static_pointer_cast<T>(reg.factory(*this));
    }

    // Like get<T>() but throws if T is not registered.
    template <class T>
    [[nodiscard]] std::shared_ptr<T> get_required() {
        auto p = get<T>();
        if (!p) {
            throw std::runtime_error(
                std::string{ "mpapp::service_provider: no registration for " } +
                typeid(T).name());
        }
        return p;
    }

    template <class T>
    [[nodiscard]] bool contains() const {
        return regs_.find(std::type_index(typeid(T))) != regs_.end();
    }

private:
    explicit service_provider(
        std::unordered_map<std::type_index, detail::service_registration> regs)
        : regs_{ std::move(regs) } {}

    std::unordered_map<std::type_index, detail::service_registration> regs_;
    friend class service_collection;
};

inline service_provider service_collection::build() {
    return service_provider{ std::move(regs_) };
}

} // namespace mpapp

#endif // MPAPP_DI_SERVICE_COLLECTION_HPP
