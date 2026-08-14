/*===============================================

    Forr Engine

    File : ResourceStorage.hpp
    Role : storage for resources and partly their data
        For example : fe::resource::Materil's 'samplers' and 'buffer' are kept here
        via 'fe::Arena' to decrease allocations count

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/pointer.hpp"
#include "Core/custom_allocators.hpp"
#include "Resources.hpp"

#include "ResourceManagementContext.hpp"

namespace fe {
    class ResourceImporter; // forward declaration

    class ResourceStorage {
    public:
        ResourceStorage(ResourceManagementContext& context) : m_Context(context) {}
        ~ResourceStorage() = default;

        template <resource::resource_t T>
        FORR_NODISCARD fe::pointer<T> CreateResource(T&& value) {
            auto& storage = this->GetStorage<T>();
            return storage.create(std::move(value));
        }

        template <resource::resource_t T>
        FORR_NODISCARD fe::pointer<T> CreateResource()
            requires std::default_initializable<T>
        {
            auto& storage = this->GetStorage<T>();
            return storage.create();
        }

        template <resource::resource_t T>
        FORR_NODISCARD T* GetResource(fe::pointer<T> ptr) {
            auto& storage = this->GetStorage<T>();
            if (!storage.is_valid(ptr)) return nullptr; // TODO : provide fallbacks
            return storage.get(ptr);
        }

        template <resource::resource_t T, typename Func>
        void RunForEach(Func&& func) {
            auto& storage = this->GetStorage<T>();
            storage.for_each(func);
        }

        // unsafe helper function
        template <resource::resource_t T>
        fe::typed_pointer_storage<T>& GetStorage() {
            if constexpr (false) {
            }
#define GENERATE_STORAGES(RESOURCE_NAME)                                 \
    else if constexpr (std::is_same_v<T, fe::resource::RESOURCE_NAME>) { \
        return m_Storage##RESOURCE_NAME;                                 \
    }
            FORR_RESOURCES_LIST(GENERATE_STORAGES)
#undef GENERATE_STORAGES
            else {
                static_assert(std::false_type::value && "Forgot to add new resource to FORR_RESOURCES_LIST");
            }
        }

        const ResourceManagementContext& GetContext() const noexcept {
            return m_Context;
        }

    private:
        ResourceManagementContext& m_Context;

#define GENERATE_STORAGES(RESOURCE_NAME) fe::typed_pointer_storage<fe::resource::RESOURCE_NAME> m_Storage##RESOURCE_NAME{};
        FORR_RESOURCES_LIST(GENERATE_STORAGES)
#undef GENERATE_STORAGES

        // you can call 'fe::Arena::reinitialize()' if you want increase or decrease arena size
        fe::Arena m_MaterialsBuffer{ 16 * 1024 };
        fe::Arena m_MaterialsSamplers{ 16 * 1024 };
    };
} // namespace fe
