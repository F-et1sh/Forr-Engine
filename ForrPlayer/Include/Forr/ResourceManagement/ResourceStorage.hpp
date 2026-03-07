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
        fe::pointer<T> GetResourcePointer(size_t index) { return m_TexturePointers[index]; }

        template <typename T>
        FORR_NODISCARD T* GetResource(fe::pointer<T> ptr) {
            assert(m_Textures.is_valid(ptr));
            return m_Textures.get(ptr);
        }

    private:
        fe::typed_pointer_storage<fe::resource::Texture> m_Textures{};
        std::vector<fe::pointer<fe::resource::Texture>>  m_TexturePointers{}; // temp

        friend class ResourceImporter;
    };
} // namespace fe
