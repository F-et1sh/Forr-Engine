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
            void* object{};
            void (*callback)(const void*);
        };
        using Listeners = std::vector<Listener>;

    public:
        EventBus()  = default;
        ~EventBus() = default;

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

        template <typename T, typename Func>
        void subscribe(Func&& callback) {
            uint32_t type_id = TypeRegistry::get_id<T>();
            if (type_id >= m_listeners.size()) {
                m_listeners.resize(type_id + 1);
            }
            Listener wrapper = [this_callback = std::forward<Func>(callback)](const void* data) {
                this_callback(*reinterpret_cast<const T*>(data));
            };
            m_listeners[type_id].push_back(wrapper);
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
                    for (const auto& handler : m_listeners[header.event_type_id]) {
                        handler.callback(std::launder(event));
                    }
                }

                offset = header.next_header_offset;
            }
        }

        void clear() noexcept { m_storage.clear(); }

    private:
        std::vector<uint8_t>   m_storage{};
        std::vector<Listeners> m_listeners{};
    };
} // namespace fe
