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

inline constexpr void hash_combine(std::size_t& seed, std::size_t value) noexcept {
    seed ^= value + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
}

namespace std {
    template <>
    struct hash<fe::shader::ReflectedDescriptor> {
        std::size_t operator()(const fe::shader::ReflectedDescriptor& p) const {
            std::size_t seed{};

            hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(p.descriptor_type)));
            hash_combine(seed, std::hash<uint32_t>{}(p.set));
            hash_combine(seed, std::hash<uint32_t>{}(p.binding));
            hash_combine(seed, std::hash<uint32_t>{}(p.array_size * p.size));
            hash_combine(seed, std::hash<uint8_t>{}(p.stage_flags));

            hash_combine(seed, std::hash<std::string>{}(p.name));

            return seed;
        }
    };
} // namespace std

namespace fe {
    class OpenGLResourceManager {
    public:
        OpenGLResourceManager(ResourceManager& resource_manager)
            : m_ResourceManager(resource_manager) {}
        ~OpenGLResourceManager() = default;

        // this function won't return you 'GPUHandle<>'
        // you passing 'T&', which is not 'const' -> it sets 'GPUHandle<>' of the resource inside
        // Why : for example, 'fe::resource::Model' does not have 'GPUHandle<Model> gpu_handle' in it
        // instead, it has 'std::vector<Mesh>', which has 'GPUHandle<Mesh> gpu_handle' in it
        ///@{
        void CreateResource(resource::Model& model);
        void CreateResource(resource::Texture& texture);
        ///@}

        // this is for render graph
        size_t               CreateImage(const render_graph::ImageDesc& image_desc);
        const OpenGLTexture& GetImage(size_t texture_storage_index);

        const OpenGLPipeline& GetOrCreatePipeline(fe::pointer<resource::ShaderProgram> shader_program_ptr,
                                                  fe::pointer<resource::Material>      material_ptr);

        const OpenGLMesh&    GetResource(GPUHandle<resource::Model::Mesh> handle) const;
        const OpenGLTexture& GetResource(GPUHandle<resource::Texture> handle) const;

        GLuint GetShaderBuffer(shader::ReflectedDescriptor& parameter);

    private: // here functions, which used like helpers to create some resources that don't have thier own CPU realization.
             // The functions return 'GPUHandle<>' but you DON'T have to set 'GPUHandle<> gpu_handle' in the resources, the functions does it by themselves

        fe::GPUHandle<fe::resource::Model::Mesh> createMesh(resource::Model::Mesh& mesh);

    private: // helpers
        GLuint createShaderProgramRaw(const fe::shader::SourceCodeStorage& source_codes);

        // this function returns the index of the resource ( GPUHandle<>::index )
        // you DON'T have to set 'GPUHandle<> gpu_handle' in the resource manually, the function does it by itself
        template <typename CPU_T, typename GPU_T>
        size_t storeResource(GPUHandle<CPU_T>& gpu_handle_dst, GPU_T& gpu_resource, std::vector<GPU_T>& storage) {
            storage.emplace_back(std::move(gpu_resource));
            gpu_handle_dst.index = storage.size() - 1;
            return gpu_handle_dst.index;
        }

    private:
        ResourceManager& m_ResourceManager;

        // GPU analogue of CPU resources
        std::vector<OpenGLMesh>    m_StorageMeshes{};
        std::vector<OpenGLTexture> m_StorageTextures{};

        // shader buffers : SSBOs and UBOs
        std::unordered_map<shader::ReflectedDescriptor, gl::Buffer> m_ShaderBuffers{};

        // combined 'fe::pointer<resource::ShaderProgram>' and 'fe::pointer<resource::Material>' --> OpenGLPipeline
        std::unordered_map<size_t, OpenGLPipeline> m_Pipelines{};
    };
} // namespace fe
