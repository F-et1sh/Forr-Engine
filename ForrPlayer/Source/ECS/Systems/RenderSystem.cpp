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

    // temp
    // TODO : move DrawCommand generation to another place
    // Variants :
    // 1.

    for (auto [entity, transform_component, mesh_component] : view.each()) {
        if (mesh_component.mesh_index == ~0) {
            const auto& model = *m_ResourceManager.GetResource(mesh_component.model_ptr);
            for (const auto& mesh : model.meshes) {
                for (const auto& primitive : mesh.primitives) {
                    DrawCommand& draw_command    = m_DrawCommands.emplace_back();
                    draw_command.index_count     = primitive.index_count;
                    draw_command.index_offset    = primitive.index_offset;
                    const auto& material         = *m_ResourceManager.GetResource(primitive.material_ptr);
                    draw_command.material_handle = material.gpu_handle;
                    draw_command.mesh_handle     = mesh.gpu_handle;
                    draw_command.transform       = transform_component.transform;
                    draw_command.sort_key        = draw_command.material_handle.index << 16;
                }
            }
        }
        else {
            const auto& model = *m_ResourceManager.GetResource(mesh_component.model_ptr);
            const auto& mesh  = model.meshes[mesh_component.mesh_index];
            for (const auto& primitive : mesh.primitives) {
                DrawCommand& draw_command    = m_DrawCommands.emplace_back();
                draw_command.index_count     = primitive.index_count;
                draw_command.index_offset    = primitive.index_offset;
                const auto& material         = *m_ResourceManager.GetResource(primitive.material_ptr);
                draw_command.material_handle = material.gpu_handle;
                draw_command.mesh_handle     = mesh.gpu_handle;
                draw_command.transform       = transform_component.transform;
                draw_command.sort_key        = draw_command.material_handle.index << 16;
            }
        }
    }
}
