/*===============================================

    Forr Engine

    File : Resources.hpp
    Role : Contain all resource structures.
        Resource Management system uses data-oriented design.
        namespace fe::resource:: means that the class is a DOD structure

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include "Core/types.hpp"

// namespace fe::resource:: means that the class is a
//  DOD structure, not a high level resource
namespace fe::resource {
    struct Texture {
        unsigned int                     width{};
        unsigned int                     height{};
        unsigned int                     components{};
        std::unique_ptr<unsigned char[]> bytes{};

        TextureMinFilter min_filter{ TextureMinFilter::LINEAR_MIPMAP_LINEAR };
        TextureMagFilter mag_filter{ TextureMagFilter::LINEAR };
        TextureWrap      wrap_s{ TextureWrap::REPEAT };
        TextureWrap      wrap_t{ TextureWrap::REPEAT };

        TextureInternalFormat internal_format{};
        TextureDataFormat     data_format{};

        Texture()  = default;
        ~Texture() = default;

        FORR_CLASS_NONCOPYABLE(Texture);
        FORR_CLASS_MOVABLE(Texture)
    };
} // namespace fe::resource
