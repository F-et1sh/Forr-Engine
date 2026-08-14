/*===============================================

    Forr Engine

    File : RenderSystem.cpp
    Role : renderer system

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ECS/Systems/RenderSystem.hpp"

struct fe::RenderSystem::Impl {
    uint32_t m_CurrentInstanceIndex{};

    ResourceManager&                       m_ResourceManager;
    std::reference_wrapper<entt::registry> m_Registry; // testing std::reference_wrapper<>
    std::reference_wrapper<IRenderer>      m_Renderer; // testing std::reference_wrapper<>
    RenderPacket&                          m_RenderPacket;

    std::vector<RenderMeshEntry> m_RenderMeshEntries{};

    std::unordered_map<fe::pointer<resource::Model>, std::vector<RenderMeshEntry>> m_Table{};

    Impl(ResourceManager& resource_manager, entt::registry& registry, IRenderer& renderer, RenderPacket& render_packet)
        : m_ResourceManager(resource_manager), m_Registry(registry), m_Renderer(renderer), m_RenderPacket(render_packet) {}
    ~Impl() = default;
};

fe::RenderSystem::RenderSystem(ResourceManager& resource_manager, entt::registry& registry, IRenderer& renderer, RenderPacket& render_packet) {
    m_Impl = std::make_unique<Impl>(resource_manager, registry, renderer, render_packet);
}

fe::RenderSystem::~RenderSystem() = default;

void fe::RenderSystem::Update() {
    //this->handleMeshComponents();
    //this->handleLightComponents();

    //std::ranges::sort(m_Impl->m_RenderPacket.draw_commands, [](const DrawCommand& a, const DrawCommand& b) {
    //    if (a.material_ptr != b.material_ptr)
    //        return a.material_ptr < b.material_ptr;
    //    return a.mesh_handle < b.mesh_handle;
    //});

    //// reset
    //m_Impl->m_CurrentInstanceIndex = 0;
}

//void fe::RenderSystem::handleMeshComponents() {
    //auto view = m_Impl->m_Registry.get().view<const TransformComponent, const MeshComponent>();

    //for (auto [entity, transform_component, mesh_component] : view.each()) {
    //    auto it = m_Impl->m_Table.find(mesh_component.model_ptr);

    //    if (it == m_Impl->m_Table.end())
    //        this->addEntry(mesh_component);

    //    this->addToDrawList(mesh_component.model_ptr, transform_component.transform);
    //}
//}

//void fe::RenderSystem::handleLightComponents() {
    //auto view = m_Impl->m_Registry.get().view<const TransformComponent, const LightComponent>();

    //for (auto [entity, transform_component, light_component] : view.each()) {
    //    auto& light = m_Impl->m_RenderPacket.lights.emplace_back();

    //    light.position        = transform_component.transform * glm::vec4(1.0f);
    //    light.direction       = glm::vec4(light_component.direction, 1.0f);
    //    light.color_intensity = glm::vec4(light_component.color, light_component.intensity);
    //}
//}

//void fe::RenderSystem::addEntry(const MeshComponent& mesh_component) {
    //auto& model = *m_Impl->m_ResourceManager.GetResource(mesh_component.model_ptr);

    //std::vector<RenderMeshEntry> enties{};
    //enties.reserve(model.meshes.size());

    //for (const auto& mesh : model.meshes) {
    //    for (const auto& primitive : mesh.primitives) {

    //        auto& entry = enties.emplace_back();

    //        entry.index_count  = primitive.index_count;
    //        entry.index_offset = primitive.index_offset;
    //        entry.material_ptr = mesh_component.material_override_ptr.packed() == ~0 // if overrided material is null
    //                                 ? primitive.material_ptr                        // ( TRUE )  select primitive's material
    //                                 : mesh_component.material_override_ptr;         // ( FALSE ) select overrided material
    //        entry.mesh_handle  = mesh.gpu_handle;
    //        entry.sort_key     = static_cast<uint64_t>(entry.material_ptr.packed()) << 16;
    //    }
    //}

    //m_Impl->m_Table.insert({ mesh_component.model_ptr, std::move(enties) });
//}

//void fe::RenderSystem::addToDrawList(fe::pointer<resource::Model> model_ptr, const glm::mat4& transform) {
    //auto it = m_Impl->m_Table.find(model_ptr);
    //if (it == m_Impl->m_Table.end()) return;

    //m_Impl->m_RenderPacket.object_transforms.push_back(transform);

    //for (const auto& entry : it->second) {
    //    auto& draw_command = m_Impl->m_RenderPacket.draw_commands.emplace_back();

    //    draw_command.instance_index = m_Impl->m_CurrentInstanceIndex;
    //    draw_command.index_count    = entry.index_count;
    //    draw_command.index_offset   = entry.index_offset;
    //    draw_command.material_ptr   = entry.material_ptr;
    //    draw_command.mesh_handle    = entry.mesh_handle;
    //    draw_command.sort_key       = entry.sort_key;
    //}

    //m_Impl->m_CurrentInstanceIndex++;
//}
