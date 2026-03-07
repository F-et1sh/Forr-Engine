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
    class Texture {
    public:
        Texture(const char* image, const char* texType, GLuint slot);

        // Assigns a texture unit to a texture
        void texUnit(Shader& shader, const char* uniform, GLuint unit);
        // Binds a texture
        void Bind();
        // Unbinds a texture
        void Unbind();
        // Deletes a texture
        void Delete();

    private:
        GLuint      id{};
        const char* type{};
        GLuint      unit{};
    };
} // namespace fe
