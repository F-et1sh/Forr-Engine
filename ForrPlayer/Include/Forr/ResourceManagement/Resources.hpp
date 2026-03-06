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
        uint32_t             width{};
        uint32_t             height{};
        TextureFormat        format{};
        std::vector<uint8_t> pixels{};

        Texture()  = default;
        ~Texture() = default;
    };

    struct TextureMeta {
        uint32_t      width{};
        uint32_t      height{};
        TextureFormat format{};
        bool          generate_mips{};

        TextureMeta()  = default;
        ~TextureMeta() = default;
    };
} // namespace fe::resource
