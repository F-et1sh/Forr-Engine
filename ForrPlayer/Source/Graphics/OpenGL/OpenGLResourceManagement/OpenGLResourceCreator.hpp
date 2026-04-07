/*===============================================

    Forr Engine

    File : OpenGLResourceCreator.hpp
    Role : creates OpenGL resources from unified ones

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "OpenGLResourceStorage.hpp"

namespace fe {
    class OpenGLResourceCreator {
    public:
        OpenGLResourceCreator(ResourceManager& resource_manager, OpenGLResourceStorage& opengl_storage)
            : m_ResourceManager(resource_manager), m_OpenGLStorage(opengl_storage) {}
        ~OpenGLResourceCreator() = default;

        fe::pointer<OpenGLTexture> CreateResource(const resource::Texture& texture);
        fe::pointer<OpenGLModel> CreateResource(const resource::Model& model);
        fe::pointer<OpenGLMaterial> CreateResource(const resource::Material& material);

    private:
        fe::pointer<OpenGLMesh> createMesh(const resource::Model::Mesh& mesh);
        fe::pointer<OpenGLShaderProgram> createShaderProgram(std::vector<resource::Shader*> shaders);

    private:
        ResourceManager&       m_ResourceManager;
        OpenGLResourceStorage& m_OpenGLStorage;
    };
} // namespace fe
