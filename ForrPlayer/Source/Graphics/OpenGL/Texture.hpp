/*===============================================

    Forr Engine

    File : Texture.hpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/


#pragma once
#include <glad/gl.h>

namespace fe {
    struct Texture {
    public:
        GLuint      id{};
        const char* type{};
        GLuint      unit{};

        Texture() = default;
        ~Texture() = default;
    };
} // namespace fe
