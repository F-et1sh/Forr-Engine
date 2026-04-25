/*===============================================

    Forr Engine

    File : OpenGLResourceManager.hpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceManager.hpp"
#include "Graphics/OpenGL/OpenGLTypes.hpp"

namespace fe {
    class OpenGLResourceManager {
    public:
        OpenGLResourceManager(ResourceManager& resource_manager)
            : m_ResourceManager(resource_manager) {}
        ~OpenGLResourceManager() = default;

        template <resource::resource_t T>
        GPUHandle<T> CreateResource(T& resource); // no need in FORR_NODISCARD

        template <resource::resource_t T>
        FORR_NODISCARD const T* GetResource(GPUHandle<T> handle) const;

    private:
        ResourceManager& m_ResourceManager;

        std::vector<OpenGLModel> m_StorageModels{};
    };
} // namespace fe
