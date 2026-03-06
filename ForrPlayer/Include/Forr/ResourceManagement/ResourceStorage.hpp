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

    private:
        fe::typed_pointer_storage<
            std::pair<
                fe::resource::Texture,
                fe::resource::TextureMeta>>
            m_Textures{};

        friend class ResourceImporter;
    };
} // namespace fe
