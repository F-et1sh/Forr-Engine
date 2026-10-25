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

namespace fe {
    class EventBus {
    private:
        // get unique id for every event's structure
        struct TypeRegistry {
        public:
            template <typename T>
            static size_t get_id() noexcept {
                static const size_t id = S_COUNTER.fetch_add(1, std::memory_order_relaxed);
                return id;
            }

        private:
            inline static std::atomic<size_t> S_COUNTER{};
        };

        struct EventHeader {
            size_t event_type_id{};
            size_t event_structure_offset{}; // starts from std::vector<>::data()
            size_t next_header_offset{};     // starts from std::vector<>::data()
        };

        struct Listener {
            using SubscriptionID = size_t;
            using Callback       = void (*)(void*, const void*);
            using DestructorFunc = void (*)(void*);

            SubscriptionID subscription_id{};

            void*          instance{};
            Callback       callback{};
            DestructorFunc destructor{};
        };
        using Listeners = std::vector<Listener>;

        template <typename T>
        struct ListenerCallbackTraitsFn {
            using Traits = ListenerCallbackTraitsFn<decltype(&T::operator())>; // for lambdas
            using ClassT = T;
            using EventT = Traits::EventT;
        };

        template <typename ClassType, typename ReturnType, typename... Args>
        struct ListenerCallbackTraitsFn<ReturnType (ClassType::*)(Args...)> {
            static_assert(sizeof...(Args) == 1, "The listener method must accept only 1 argument - the event");

            using ClassT = ClassType;
            using EventT = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
        };

        template <typename ClassType, typename ReturnType, typename... Args>
        struct ListenerCallbackTraitsFn<ReturnType (ClassType::*)(Args...) const> {
            static_assert(sizeof...(Args) == 1, "The listener method must accept only 1 argument - the event");

            using ClassT = ClassType;
            using EventT = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
        };

        template <typename ClassType, typename ReturnType, typename... Args>
        struct ListenerCallbackTraitsFn<ReturnType (ClassType::*)(Args...) noexcept> {
            static_assert(sizeof...(Args) == 1, "The listener method must accept only 1 argument - the event");

            using ClassT = ClassType;
            using EventT = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
        };

        template <typename ClassType, typename ReturnType, typename... Args>
        struct ListenerCallbackTraitsFn<ReturnType (ClassType::*)(Args...) const noexcept> {
            static_assert(sizeof...(Args) == 1, "The listener method must accept only 1 argument - the event");

            using ClassT = ClassType;
            using EventT = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
        };

    public:
        EventBus() = default;
        ~EventBus() {
            for (auto& event_listeners : m_listeners) {
                for (auto& listener : event_listeners) {

                    if (listener.destructor) {

                        listener.destructor(listener.instance);
                        ::operator delete(listener.instance);
                    }
                }
            }
        }

        FORR_CLASS_MOVABLE(EventBus)
        FORR_CLASS_NONCOPYABLE(EventBus)

        template <typename Event>
            requires std::is_trivially_copyable_v<Event> && std::is_trivially_destructible_v<Event>
        void enqueue(const Event& event) {
            const size_t id = TypeRegistry::get_id<Event>();

            size_t storage_size                  = m_storage.size();
            size_t aligned_header_offset         = fe::align_up(alignof(EventHeader), storage_size);
            size_t aligned_event_offset          = fe::align_up(alignof(Event), aligned_header_offset + sizeof(EventHeader));
            size_t aligned_next_structure_offset = fe::align_up(alignof(EventHeader), aligned_event_offset + sizeof(Event));

            m_storage.resize(aligned_next_structure_offset);

            EventHeader header{
                .event_type_id          = id,
                .event_structure_offset = aligned_event_offset,
                .next_header_offset     = aligned_next_structure_offset
            };

            std::memcpy(&m_storage[aligned_header_offset], &header, sizeof(EventHeader));
            std::memcpy(&m_storage[aligned_event_offset], &event, sizeof(Event));
        }

        template <auto Func>
        Listener::SubscriptionID subscribe(ListenerCallbackTraitsFn<decltype(Func)>::ClassT& instance) {
            using Traits = ListenerCallbackTraitsFn<decltype(Func)>;
            using ClassT = Traits::ClassT;
            using EventT = Traits::EventT;

            size_t type_id = TypeRegistry::get_id<EventT>();

            if (type_id >= m_listeners.size()) {
                m_listeners.resize(type_id + 1);
            }

            Listener listener{
                .instance = &instance,
                .callback = [](void* instance, const void* event_data) {
                    auto*       object = static_cast<ClassT*>(instance);
                    const auto* event  = static_cast<const EventT*>(event_data);
                    (object->*Func)(*event);
                },
                .destructor = nullptr
            };

            m_listeners[type_id].push_back(listener);

            return S_SUBSCRIPTION_COUNTER.fetch_add(1, std::memory_order_relaxed);
        }

        template <typename LambdaT>
        Listener::SubscriptionID subscribe(LambdaT&& lambda) {
            using Traits = ListenerCallbackTraitsFn<std::decay_t<LambdaT>>;
            using ClassT = Traits::ClassT;
            using EventT = Traits::EventT;

            size_t type_id = TypeRegistry::get_id<EventT>();

            if (type_id >= m_listeners.size()) {
                m_listeners.resize(type_id + 1);
            }

            using CleanLambdaT             = std::decay_t<LambdaT>;
            void*         allocated_lambda = ::operator new(sizeof(CleanLambdaT));
            CleanLambdaT* lambda_ptr       = new (allocated_lambda) CleanLambdaT(std::forward<LambdaT>(lambda));

            Listener listener{
                .instance   = lambda_ptr,
                .callback   = [](void* instance, const void* event_data) {
                    auto*       lambda = static_cast<ClassT*>(instance);
                    const auto* event  = static_cast<const EventT*>(event_data);
                    (*lambda)(*event); },
                .destructor = [](void* instance) { std::destroy_at(static_cast<ClassT*>(instance)); }
            };

            m_listeners[type_id].push_back(listener);

            return S_SUBSCRIPTION_COUNTER.fetch_add(1, std::memory_order_relaxed);
        }

        void unsubscribe(Listener::SubscriptionID subscription_id) {
            for (auto& event_listeners : m_listeners) {
                for (auto it = event_listeners.begin(); it != event_listeners.end(); it++) {
                    
                    if (it->subscription_id == subscription_id) {

                        if (it->destructor) {
                            it->destructor(it->instance);
                            ::operator delete(it->instance);
                        }

                        event_listeners.erase(it);
                        return;
                    }
                }
            }
        }

        void dispatch_all() const {
            if (m_storage.empty()) return;

            const uint8_t* buffer     = m_storage.data();
            size_t         total_size = m_storage.size();
            size_t         offset     = 0;

            while (offset < total_size) {
                const EventHeader& header = *std::launder(reinterpret_cast<const EventHeader*>(&buffer[offset]));

                if (header.event_type_id < m_listeners.size()) {

                    const void* event = &buffer[header.event_structure_offset];
                    for (const auto& listener : m_listeners[header.event_type_id]) {
                        listener.callback(listener.instance, event);
                    }
                }

                offset = header.next_header_offset;
            }
        }

        void clear() noexcept { m_storage.clear(); }

    private:
        std::vector<uint8_t>   m_storage{};
        std::vector<Listeners> m_listeners{};

        inline static std::atomic<Listener::SubscriptionID> S_SUBSCRIPTION_COUNTER{};
    };
} // namespace fe
