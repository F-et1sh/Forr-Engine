/*===============================================

    Forr Engine

    File : ResourceStorage.hpp
    Role : storage for resources

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/pointer.hpp"
#include "Resources.hpp"

namespace fe {
    class ResourceImporter; // forward declaration

    class ResourceStorage {
    public:
        ResourceStorage()  = default;
        ~ResourceStorage() = default;

        template <typename T>
        FORR_NODISCARD fe::pointer<T> CreateResource(const T& value) {
            auto& storage = this->GetStorage<T>();
            return storage.create(value);
        }

        template <typename T>
        FORR_NODISCARD fe::pointer<T> CreateResource(T&& value) {
            auto& storage = this->GetStorage<T>();
            return storage.create(std::move(value));
        }

        template <typename T>
        FORR_NODISCARD fe::pointer<T> CreateResource()
            requires std::default_initializable<T>
        {
            auto& storage = this->GetStorage<T>();
            return storage.create();
        }

        template <typename T>
        FORR_NODISCARD T* GetResource(fe::pointer<T> ptr) {
            auto& storage = this->GetStorage<T>();
            if (!storage.is_valid(ptr)) return nullptr; // TODO : provide fallbacks
            return storage.get(ptr);
        }

        template <typename T, typename Func>
        void RunForEach(Func&& func) {
            auto& storage = this->GetStorage<T>();
            storage.for_each(func);
        }

        // unsafe helper function
        template <typename T>
        fe::typed_pointer_storage<T>& GetStorage() {
            if constexpr (std::is_same_v<T, fe::resource::Texture>)
                return m_Textures;
            else if constexpr (std::is_same_v<T, fe::resource::Material>)
                return m_Materials;
            else if constexpr (std::is_same_v<T, fe::resource::Model>)
                return m_Models;
        }

    private:
        fe::typed_pointer_storage<fe::resource::Texture>  m_Textures{};
        fe::typed_pointer_storage<fe::resource::Material> m_Materials{};
        fe::typed_pointer_storage<fe::resource::Model>    m_Models{};
    };
} // namespace fe
