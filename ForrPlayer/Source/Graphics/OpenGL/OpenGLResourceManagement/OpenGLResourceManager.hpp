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

#include "OpenGLResourceCreator.hpp"
#include "Graphics/GPUResourceLookupTable.hpp"

namespace fe {
    template <typename T>
    concept opengl_resource_t =
        (std::is_same_v<T, OpenGLTexture>) ||
        (std::is_same_v<T, OpenGLMesh>) ||
        //(std::is_same_v<T, OpenGLMaterial>) || // TODO : provide materials
        (std::is_same_v<T, OpenGLModel>);

    class OpenGLResourceManager {
    public:
        OpenGLResourceManager(ResourceManager& resource_manager)
            : m_ResourceManager(resource_manager) {}
        ~OpenGLResourceManager() = default;

        template <resource::resource_t T>
        auto CreateResource(fe::pointer<T> resource_ptr) {
            const auto& resource = *m_ResourceManager.GetResource(resource_ptr);
            auto        ptr      = m_Importer.CreateResource(resource); // does not need to store this fe::pointer
            m_LookupTable.Insert(resource_ptr, ptr);
            return ptr;
        }

        template <opengl_resource_t T>
        FORR_NODISCARD const T* GetResource(fe::pointer<T> pointer) const {
            const auto& storage = m_Storage.GetStorage<T>();

            auto resource = storage.get(pointer);
            return resource;
        }

        template <resource::resource_t T>
        FORR_NODISCARD auto GetGPUPointer(fe::pointer<T> pointer) {
            uint64_t packed = m_LookupTable.GetPackedPointerGPU(pointer);

            if constexpr (std::is_same_v<T, resource::Texture>) {
                return fe::pointer<OpenGLTexture>{ packed };
            }
            else if constexpr (std::is_same_v<T, resource::Model>) {
                return fe::pointer<OpenGLModel>{ packed };
            }
            else {
                assert(false);
                return {};
            }
        }

    private:
        ResourceManager& m_ResourceManager;

        OpenGLResourceStorage  m_Storage{};
        GPUResourceLookupTable m_LookupTable{};
        OpenGLResourceCreator  m_Importer{ m_ResourceManager, m_Storage };
    };
} // namespace fe
