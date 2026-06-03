/*===============================================

    Forr Engine

    File : VulkanResourceManager.hpp
    Role : GPU Resource Manager ( for Vulkan )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceManager.hpp"
#include "VulkanTypes.hpp"
#include "VulkanContext.hpp"

namespace fe {
    class VulkanResourceManager {
    public:
        VulkanResourceManager(VulkanContext& context, ResourceManager& resource_manager)
            : m_Context(context), m_ResourceManager(resource_manager) {}
        ~VulkanResourceManager() = default;

        // this function won't return you 'GPUHandle<>'
        // you passing 'T&', which is not 'const' -> it sets 'GPUHandle<>' of the resource inside
        // Why : for example, 'fe::resource::Model' does not have 'GPUHandle<Model> gpu_handle' in it
        // instead, it has 'std::vector<Mesh>', which has 'GPUHandle<Mesh> gpu_handle' in it
        ///@{
        void CreateResource(resource::Model& model);
        void CreateResource(resource::Material& material);
        void CreateResource(resource::Texture& texture);
        ///@} <-- This does not work. What is wrong with MS Visual Studio 2022 ?

        const VulkanMesh&     GetResource(GPUHandle<resource::Model::Mesh> handle) const;
        const VulkanMaterial& GetResource(GPUHandle<resource::Material> handle) const;
        const VulkanTexture&  GetResource(GPUHandle<resource::Texture> handle) const;

    private:
        fe::GPUHandle<fe::resource::Model::Mesh> createMesh(resource::Model::Mesh& mesh);
        VkDescriptorSetLayout                    createDescriptorSetLayout(const resource::Material& material);
        VkPipelineLayout                         createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts_raw);
        VkPipeline                               createPipeline(VkPipelineLayout pipeline_layout_raw, const resource::Material& material);
        fe::vk::ShaderModule                     createShaderModule(fe::pointer<fe::resource::Shader> shader_ptr);
        void                                     generateMipmaps(VkCommandBuffer command_buffer, vk::Image& image, uint32_t width, uint32_t height, uint32_t mip_levels);

        VkDescriptorType toVkDescriptorType(resource::Shader::DescriptorType descriptor_type) const;

    private:
        // this function returns the index of the resource ( GPUHandle<>::index )
        // you DON'T have to set 'GPUHandle<> gpu_handle' in the resources by yourself, the function does it by itself
        template <typename T, typename GPU_T>
        size_t storeResource(GPUHandle<T>& gpu_handle_dst, GPU_T& gpu_resource, std::vector<GPU_T>& storage) {
            storage.emplace_back(std::move(gpu_resource));
            gpu_handle_dst.index = storage.size() - 1;
            return gpu_handle_dst.index;
        }

    private:
        VulkanContext&   m_Context;
        ResourceManager& m_ResourceManager;

        std::vector<VulkanMaterial> m_StorageMaterials{};
        std::vector<VulkanMesh>     m_StorageMeshes{};
        std::vector<VulkanTexture>  m_StorageTextures{};
    };
} // namespace fe
