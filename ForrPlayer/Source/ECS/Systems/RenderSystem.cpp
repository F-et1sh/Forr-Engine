/*===============================================

    Forr Engine

    File : RenderSystem.cpp
    Role : renderer system. Without EnTT for now

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ECS/Systems/RenderSystem.hpp"

void fe::RenderSystem::Update() {
    auto view = m_Registry.get().view<const TransformComponent, const MeshComponent>();

    for (auto [entity, transform_component, mesh_component] : view.each()) {
        auto it = m_Table.find(mesh_component.model_ptr);

        if (it == m_Table.end())
            this->addEntry(mesh_component.model_ptr);

        this->addToDrawList(mesh_component.model_ptr, transform_component.transform);
    }
}

void fe::RenderSystem::addEntry(fe::pointer<resource::Model> model_ptr) {
    auto& model = *m_ResourceManager.GetResource(model_ptr);

    std::vector<RenderMeshEntry> enties{};
    enties.reserve(model.meshes.size());

    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {

            const auto& material = *m_ResourceManager.GetResource(primitive.material_ptr);

            auto& entry           = enties.emplace_back();
            entry.index_count     = primitive.index_count;
            entry.index_offset    = primitive.index_offset;
            entry.material_handle = material.gpu_handle;
            entry.mesh_handle     = mesh.gpu_handle;
            entry.sort_key        = 16 << material.gpu_handle.index;
        }
    }
}

void fe::RenderSystem::addToDrawList(fe::pointer<resource::Model> model_ptr, const glm::mat4& transform) {
    auto it = m_Table.find(model_ptr);
    if (it == m_Table.end()) {
        fe::logging::error("Cannot draw the model without RenderMeshEntry");
        return;
    }

    for (const auto& entry : it->second) {
        auto& draw_command           = m_DrawCommands.emplace_back();
        draw_command.index_count     = entry.index_count;
        draw_command.index_offset    = entry.index_offset;
        draw_command.material_handle = entry.material_handle;
        draw_command.mesh_handle     = entry.mesh_handle;
        draw_command.sort_key        = entry.sort_key;
        draw_command.transform       = transform;
    }
}
