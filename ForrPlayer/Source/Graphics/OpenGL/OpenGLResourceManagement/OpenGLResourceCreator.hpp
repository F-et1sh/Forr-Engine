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
        OpenGLResourceCreator(ResourceManager& resource_manager, OpenGLResourceStorage& storage)
            : m_ResourceManager(resource_manager), m_Storage(storage) {}
        ~OpenGLResourceCreator() = default;

        FORR_NODISCARD fe::pointer<OpenGLTexture> CreateResource(const resource::Texture& texture);
        FORR_NODISCARD fe::pointer<OpenGLModel> CreateResource(const resource::Model& model);

    private:
        FORR_NODISCARD fe::pointer<OpenGLMesh> createMesh(const resource::Model::Mesh& mesh);

    private:
        ResourceManager&       m_ResourceManager;
        OpenGLResourceStorage& m_Storage;
    };
} // namespace fe
