/*===============================================

    Forr Engine

    File : OpenGLResourceStorage.hpp
    Role : stores OpenGL resources

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceManager.hpp"
#include "Graphics/OpenGL/OpenGLTypes.hpp"

namespace fe {
    struct OpenGLResourceStorage {
    public:
        fe::typed_pointer_storage<OpenGLTexture>       m_Textures{};
        fe::typed_pointer_storage<OpenGLModel>         m_Models{};
        fe::typed_pointer_storage<OpenGLMesh>          m_Meshes{};
        fe::typed_pointer_storage<OpenGLShaderProgram> m_Shaders{};
        fe::typed_pointer_storage<OpenGLMaterial>      m_Materials{};

        template <typename T>
        const fe::typed_pointer_storage<T>& GetStorage() const {
            if constexpr (std::is_same_v<T, OpenGLTexture>) {
                return m_Textures;
            }
            else if constexpr (std::is_same_v<T, OpenGLModel>) {
                return m_Models;
            }
            else if constexpr (std::is_same_v<T, OpenGLMesh>) {
                return m_Meshes;
            }
            else if constexpr (std::is_same_v<T, OpenGLShaderProgram>) {
                return m_Shaders;
            }
            else if constexpr (std::is_same_v<T, OpenGLMaterial>) {
                return m_Materials;
            }
            else {
                assert(false);
                return {};
            }
        }

        OpenGLResourceStorage()  = default;
        ~OpenGLResourceStorage() = default;
    };
} // namespace fe
