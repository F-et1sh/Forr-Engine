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
        FORR_NODISCARD T* GetResource(fe::pointer<T> ptr) {
            auto& storage = this->get_storage<T>();
            assert(storage.is_valid(ptr));
            return storage.get(ptr);
        }

        template<typename T>
        FORR_NODISCARD const std::vector<T>& GetStorage() const noexcept { return m_Textures.get_storage(); }

    private:
        template <typename T>
        fe::typed_pointer_storage<T>& get_storage() {
            if constexpr (std::is_same_v<T, fe::resource::Texture>)
                return m_Textures;
            /*else if constexpr (std::is_same_v<T, fe::resource::Mesh>) // TODO : add this
                return m_Meshes;*/
        }

    private:
        fe::typed_pointer_storage<fe::resource::Texture> m_Textures{};

        friend class ResourceImporter;
    };
} // namespace fe
