/*===============================================

    Forr Engine

    File : pointer.hpp
    Role : slot map

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <concepts>
#include <atomic>

#include "Forr/Core/attributes.hpp"
#include "Forr/Core/custom_allocators.hpp"
#include "Forr/Core/logging.hpp"
#include "Forr/Core/type_registry.hpp"

namespace fe {
    class EventBus {
    private:
        struct EventHeader {
            type_registry<EventBus>::type_id event_type_id{};
            size_t                           event_structure_offset{}; // starts from std::vector<>::data()
            size_t                           next_header_offset{};     // starts from std::vector<>::data()
        };

        struct Listener {
            using SubscriptionID = size_t;
            using Callback       = std::move_only_function<void(const void*) const>;

            SubscriptionID subscription_id{};
            Callback       callback{};
        };
        using Listeners = std::vector<Listener>;

        template <typename ClassType, typename... Args>
        struct ListenerCallbackTraitsBase {
            static_assert(sizeof...(Args) == 1, "The listener method must accept only 1 argument - the event");
            using ClassT = ClassType;
            using EventT = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
        };

        template <typename T>
        struct ListenerCallbackTraitsFn : ListenerCallbackTraitsFn<decltype(&T::operator())> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct ListenerCallbackTraitsFn<ReturnType (ClassType::*)(Args...)> : ListenerCallbackTraitsBase<ClassType, Args...> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct ListenerCallbackTraitsFn<ReturnType (ClassType::*)(Args...) const> : ListenerCallbackTraitsBase<ClassType, Args...> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct ListenerCallbackTraitsFn<ReturnType (ClassType::*)(Args...) noexcept> : ListenerCallbackTraitsBase<ClassType, Args...> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct ListenerCallbackTraitsFn<ReturnType (ClassType::*)(Args...) const noexcept> : ListenerCallbackTraitsBase<ClassType, Args...> {};

    public:
        EventBus()  = default;
        ~EventBus() = default;

        FORR_CLASS_MOVABLE(EventBus)
        FORR_CLASS_NONCOPYABLE(EventBus)

        template <typename Event>
            requires std::is_trivially_copyable_v<Event> && std::is_trivially_destructible_v<Event>
        void Enqueue(const Event& event) {
            const type_registry<EventBus>::type_id id = type_registry<EventBus>::get_id<Event>();

            size_t storage_size                  = m_Storage.size();
            size_t aligned_header_offset         = fe::align_up(alignof(EventHeader), storage_size);
            size_t aligned_event_offset          = fe::align_up(alignof(Event), aligned_header_offset + sizeof(EventHeader));
            size_t aligned_next_structure_offset = fe::align_up(alignof(EventHeader), aligned_event_offset + sizeof(Event));

            m_Storage.resize(aligned_next_structure_offset);

            EventHeader header{
                .event_type_id          = id,
                .event_structure_offset = aligned_event_offset,
                .next_header_offset     = aligned_next_structure_offset
            };

            std::memcpy(&m_Storage[aligned_header_offset], &header, sizeof(EventHeader));
            std::memcpy(&m_Storage[aligned_event_offset], &event, sizeof(Event));
        }

        // @param instance functor class, which is invoked with 'const EventT&'
        // @returns a subscription index to unsubscribe ( fe::EventBus::Unsubscribe() ) if needed
        template <auto Func, typename ClassT>
        Listener::SubscriptionID Subscribe(ClassT& instance) {
            using Traits = ListenerCallbackTraitsFn<decltype(Func)>;
            using EventT = Traits::EventT;

            static_assert(std::is_same_v<typename Traits::ClassT, ClassT>, "The provided instance does not match the class of the function");

            type_registry<EventBus>::type_id type_id = type_registry<EventBus>::get_id<EventT>();

            if (type_id >= m_Listeners.size()) {
                m_Listeners.resize(type_id + 1);
            }

            auto subscription_id = S_SUBSCRIPTION_COUNTER.fetch_add(1, std::memory_order_relaxed);

            m_Listeners[type_id].push_back({ .subscription_id = subscription_id,
                                             .callback        = [&instance](const void* event) { (instance.*Func)(*static_cast<const EventT*>(event)); } });

            return subscription_id;
        }

        // @param lambda invoked with 'const EventT&'
        // @returns a subscription index to unsubscribe ( fe::EventBus::Unsubscribe() ) if needed
        template <typename LambdaT>
        Listener::SubscriptionID Subscribe(LambdaT&& lambda) {
            using Traits = ListenerCallbackTraitsFn<std::decay_t<LambdaT>>;
            using EventT = Traits::EventT;

            auto type_id = type_registry<EventBus>::get_id<EventT>();
            if (type_id >= m_Listeners.size()) {
                m_Listeners.resize(type_id + 1);
            }

            auto subscription_id = S_SUBSCRIPTION_COUNTER.fetch_add(1, std::memory_order_relaxed);

            m_Listeners[type_id].push_back({ .subscription_id = subscription_id,
                                             .callback        = [func = std::forward<LambdaT>(lambda)](const void* event) { func(*static_cast<const EventT*>(event)); } });

            return subscription_id;
        }

        void Unsubscribe(Listener::SubscriptionID subscription_id) {
            for (auto& event_listeners : m_Listeners) {
                for (auto it = event_listeners.begin(); it != event_listeners.end(); it++) {

                    if (it->subscription_id == subscription_id) {
                        event_listeners.erase(it);
                        return;
                    }
                }
            }
        }

        void DispatchAll() const {
            if (m_Storage.empty()) return;

            const uint8_t* buffer     = m_Storage.data();
            size_t         total_size = m_Storage.size();
            size_t         offset     = 0;

            while (offset < total_size) {
                const EventHeader& header = *std::launder(reinterpret_cast<const EventHeader*>(&buffer[offset]));

                if (header.event_type_id < m_Listeners.size()) {

                    const void* event = &buffer[header.event_structure_offset];
                    for (const auto& listener : m_Listeners[header.event_type_id]) {
                        listener.callback(event);
                    }
                }

                offset = header.next_header_offset;
            }
        }

        void Clear() noexcept { m_Storage.clear(); }

    private:
        std::vector<uint8_t>   m_Storage{};
        std::vector<Listeners> m_Listeners{};

        inline static std::atomic<Listener::SubscriptionID> S_SUBSCRIPTION_COUNTER{};
    };
} // namespace fe
