/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for spv

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

#include <fstream>

#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"

using namespace fe::resource;

fe::pointer<fe::resource::ShaderProgram> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    ShaderProgram shader_program{};

    static Slang::ComPtr<slang::IGlobalSession> global_session{};
    if (!global_session) {
        if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef()))) {
            fe::logging::error("File -> Unified. Slang : Failed to create global session");
            return {};
        }
    }

    slang::SessionDesc session_desc{};
    slang::TargetDesc  target_desc{};

    const auto& resource_management_context = storage.GetContext();

    switch (resource_management_context.graphics_backend) {
        case GraphicsBackend::OpenGL:
            target_desc.format  = SLANG_GLSL;
            target_desc.profile = global_session->findProfile("glsl_450");
            break;
        case GraphicsBackend::Vulkan:
            target_desc.format  = SLANG_SPIRV;
            target_desc.profile = global_session->findProfile("spirv_1_5");
            break;
        default:
            fe::logging::warning("The selected renderer backend %i was not found. Using the default one", resource_management_context.graphics_backend);

            target_desc.format  = SLANG_GLSL;
            target_desc.profile = global_session->findProfile("glsl_450");
            break;
    }

    target_desc.flags = 0;

    session_desc.targets                  = &target_desc;
    session_desc.targetCount              = 1;
    session_desc.compilerOptionEntryCount = 0;

    Slang::ComPtr<slang::ISession> session{};
    if (SLANG_FAILED(global_session->createSession(session_desc, session.writeRef()))) {
        fe::logging::error("File -> Unified. Slang : Failed to create a session");
        return {};
    }

    slang::IModule* slang_module = nullptr;
    {
        slang_module = session->loadModule(resource_full_path.string().c_str());
        if (!slang_module) {
            fe::logging::error("File -> Unified. Slang : Failed to load a slang module\nPath : %s", resource_full_path.string().c_str());
            return {};
        }
    }

    std::vector<slang::IComponentType*> component_types{};
    component_types.emplace_back(slang_module);

    static constexpr std::array<std::string_view, 3> entry_point_names{
        "vertexMain",
        "fragmentMain",
        "computeMain"
    };

    struct EntryPoint {
        Slang::ComPtr<slang::IEntryPoint> entry_point{};
        ShaderProgram::ShaderType         shader_type{};

        EntryPoint()  = default;
        ~EntryPoint() = default;

        EntryPoint(slang::IEntryPoint* entry_point, ShaderProgram::ShaderType shader_type)
            : entry_point(entry_point), shader_type(shader_type) {}
    };

    std::vector<EntryPoint> entry_points{};

    for (std::uint32_t i = 0; i < std::size(entry_point_names); i++) {
        auto                entry_point_name = entry_point_names[i];
        slang::IEntryPoint* entry_point{};
        slang_module->findEntryPointByName(entry_point_name.data(), &entry_point);

        if (entry_point) {
            ShaderProgram::ShaderType shader_type{};

            if (entry_point_name == entry_point_names[0])
                shader_type = ShaderProgram::ShaderType::VERTEX;
            else if (entry_point_name == entry_point_names[1])
                shader_type = ShaderProgram::ShaderType::FRAGMENT;
            else if (entry_point_name == entry_point_names[2])
                shader_type = ShaderProgram::ShaderType::COMPUTE;

            entry_points.emplace_back(entry_point, shader_type);
            component_types.emplace_back(entry_point);
        }
    }

    Slang::ComPtr<slang::IComponentType> composed_program{};
    {
        SlangResult result = session->createCompositeComponentType(component_types.data(),
                                                                   component_types.size(),
                                                                   composed_program.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("File -> Unified. Slang : Failed to create a composed program\nPath : %s", resource_full_path.string().c_str());
            return {};
        }
    }

    for (std::size_t i = 0; i < entry_points.size(); i++) {
        const auto&                 entry_point = entry_points[i];
        Slang::ComPtr<slang::IBlob> spirv_code{};

        SlangResult result = composed_program->getEntryPointCode(i, 0, spirv_code.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("File -> Unified. Slang : Failed to get an entry point code\nEntry point index : %i\nPath : %s", i, resource_full_path.string().c_str());
            return {};
        }

        const size_t   byte_size = spirv_code->getBufferSize();
        const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(spirv_code->getBufferPointer());

        ShaderProgram::ShaderType shader_type     = entry_point.shader_type;
        auto&                     source_code_dst = shader_program.source_codes[shader_type];

        source_code_dst.resize(byte_size, 0);
        std::memcpy(source_code_dst.data(), raw_data, byte_size);
    }

    slang::ProgramLayout* program_layout = composed_program->getLayout(0); // 'target = 0'
    if (!program_layout) {
        fe::logging::error("File -> Unified. Slang : Failed to get the layout of the composed program while reflection\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    unsigned int parameter_count = program_layout->getParameterCount();



    auto ptr = storage.CreateResource(std::move(shader_program));
    return ptr;
}
