/*===============================================

    Forr Engine

    File : ResourceCreator.cpp
    Role : creates engine-specific resources in the explorer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceCreator.hpp"

#include "Graphics/Shaders/ShaderCompiler.hpp"

void fe::ResourceCreator::CreateDefaultResources() {
    this->createDefaultMaterials();
}

void fe::ResourceCreator::CreateMaterial(fe::pointer<resource::Material> pointer, const std::filesystem::path& resource_full_path) {
    resource::Material& material = *m_Storage.GetResource(pointer);

    std::ofstream file(resource_full_path);
    if (!file.good()) {
        fe::logging::error("Unified -> %s. Failed create material\nPath : %s",
                           resource_full_path.extension().string().c_str(),
                           resource_full_path.string().c_str());
        return;
    }
}

constexpr static std::string_view GLTF_VERTEX_SHADER_SOURCE = R"(
    #version 450

    layout (location = 0) in vec3 a_Position;
    
    layout (binding = 0) uniform UBO {
    	mat4 projection_matrix;
    	mat4 view_matrix;
    	mat4 model_matrices[32];
    } ubo;
    
    #ifdef FORR_USE_OPENGL    
    uniform int model_index;
    #endif    

    void main() {
    	#ifdef FORR_USE_OPENGL
        gl_Position = ubo.projection_matrix * ubo.view_matrix * ubo.model_matrices[model_index] * vec4(a_Position.xyz, 1.0f);
        #endif
    }
)";

constexpr static std::string_view GLTF_FRAGMENT_SHADER_SOURCE = R"(
    #version 460
    layout(location = 0) out vec4 fragColor;
    
    layout (std140, binding = 1) uniform Material {
    	vec3 color;
    }material;
    
    void main() {
    	fragColor = vec4(material.color, 1.0f);
    }

)";

void fe::ResourceCreator::createDefaultShaders() {
    //resource::Shader gltf_vertex_shader{};
    //gltf_vertex_shader.type = resource::Shader::Type::VERTEX;
    //ShaderCompiler::Compile(gltf_vertex_shader.source_code, GLTF_VERTEX_SHADER_SOURCE, resource::Shader::Type::VERTEX, m_Context.graphics_backend);

    ////m_Context.default_gltf_vertex_shader = m_Storage.CreateResource(gltf_vertex_shader);

    //resource::Shader gltf_fragment_shader{};
    //gltf_fragment_shader.type = resource::Shader::Type::FRAGMENT;
    //ShaderCompiler::Compile(gltf_fragment_shader.source_code, GLTF_FRAGMENT_SHADER_SOURCE, resource::Shader::Type::FRAGMENT, m_Context.graphics_backend);
}

void fe::ResourceCreator::createDefaultMaterials() {
    /*resource::Material gltf_material{};
    gltf_material.vertex_shader_ptr   = m_Context.default_gltf_vertex_shader;
    gltf_material.fragment_shader_ptr = m_Context.default_gltf_fragment_shader;
    gltf_material.color               = { 0.6f, 0.7f, 1.0f };*/
}
