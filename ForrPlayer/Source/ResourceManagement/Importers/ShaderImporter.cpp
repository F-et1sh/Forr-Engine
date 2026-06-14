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
    ShaderProgram shader{};

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

    // make sure that entry points' order is the same as shader types in fe::resource::ShaderProgram::ShaderType
    static constexpr const char* entry_point_names[] = {
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
        slang_module->findEntryPointByName(entry_point_name, &entry_point);

        if (entry_point) {
            entry_points.emplace_back(entry_point, static_cast<ShaderProgram::ShaderType>(i)); // temp, I guess
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

        SlangResult result = composed_program->getEntryPointCode(i,
                                                                 0,
                                                                 spirv_code.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("File -> Unified. Slang : Failed to get entry point code\nEntry point index : %i\nPath : %s", i, resource_full_path.string().c_str());
            return {};
        }

        const size_t   byte_size = spirv_code->getBufferSize();
        const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(spirv_code->getBufferPointer());

        ShaderProgram::ShaderType shader_type     = entry_point.shader_type;
        auto&                     source_code_dst = shader.source_codes[shader_type];

        source_code_dst.resize(byte_size, 0);
        std::memcpy(source_code_dst.data(), raw_data, byte_size);
    }

    auto ptr = storage.CreateResource(std::move(shader));
    return ptr;
}

void fe::ShaderImporter::CompileAndReflect(resource::ShaderProgram::SourceCodeStorage& dst, std::string_view src, GraphicsBackend graphics_backend) {
    static Slang::ComPtr<slang::IGlobalSession> global_session{};

    if (!global_session) {
        if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef()))) {
            fe::logging::error("Slang : Failed to create global session");
            return;
        }
    }

    dst.clear();

    slang::SessionDesc session_desc{};
    slang::TargetDesc  target_desc{};

    switch (graphics_backend) {
        case GraphicsBackend::OpenGL:
            target_desc.format  = SLANG_GLSL;
            target_desc.profile = global_session->findProfile("glsl_450");
            break;
        case GraphicsBackend::Vulkan:
            target_desc.format  = SLANG_SPIRV;
            target_desc.profile = global_session->findProfile("spirv_1_5");
            break;
        default:
            fe::logging::warning("The selected renderer backend %i was not found. Using the default one", graphics_backend);

            target_desc.format  = SLANG_GLSL;
            target_desc.profile = global_session->findProfile("glsl_450");
            break;
    }

    session_desc.targets     = &target_desc;
    session_desc.targetCount = 1;

    Slang::ComPtr<slang::ISession> session{};
    global_session->createSession(session_desc, session.writeRef());

    Slang::ComPtr<slang::IModule> slang_module{};
    Slang::ComPtr<slang::IBlob>   diagnostics_blob{};
    const char*                   module_name = "shader";
    const char*                   module_path = "shader.slang";

    slang_module = session->loadModuleFromSourceString(module_name, module_path, src.data(), diagnostics_blob.writeRef());
    assert(slang_module);

    std::vector<std::string_view> entry_points_names{
        "vertexMain",
        "fragmentMain",
        "ComputeMain"
    };

    std::vector<Slang::ComPtr<slang::IEntryPoint>> entry_points{};

    for (const auto& entry_point_name : entry_points_names) {
        Slang::ComPtr<slang::IEntryPoint> entry_point{};
        slang_module->findEntryPointByName(entry_point_name.data(), entry_point.writeRef());

        if (entry_point) entry_points.push_back(entry_point);
    }

    //std::vector<slang::IComponentType*> component_types{};
    //component_types.push_back(slang_module);
    //component_types.append_range(entry_points);

    //Slang::ComPtr<slang::IComponentType> composed_program{};
    //if (SLANG_FAILED(session->createCompositeComponentType(component_types.data(), component_types.size(), composed_program.writeRef()))) {
    //assert(false);
    //}

    //Slang::ComPtr<slang::IComponentType> linked_program{};
    //if (SLANG_FAILED(composed_program->link(linked_program.writeRef()))) {
    //assert(false);
    //}

    for (std::size_t i = 0; i < entry_points.size(); i++) {
        Slang::ComPtr<slang::IBlob> source_code{};

        auto& entry_point = entry_points[i];

        Slang::ComPtr<slang::IComponentType> linked_program{};
        entry_point->link(linked_program.writeRef());

        SlangResult result = linked_program->getEntryPointCode(0, // entryPointIndex
                                                               0, // targetIndex
                                                               source_code.writeRef());

        if (SLANG_FAILED(result)) {
            assert(false);
        }

        std::ofstream file("src_code" + std::to_string(i) + ".txt");
        file.write(reinterpret_cast<const char*>(source_code->getBufferPointer()), source_code->getBufferSize());
        file.close();

        // temp
        if (i == 0) {
            const size_t   byte_size = source_code->getBufferSize();
            const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(source_code->getBufferPointer());

            size_t words_count = (byte_size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
            dst[ShaderProgram::ShaderType::VERTEX].resize(words_count, 0);
            std::memcpy(dst[ShaderProgram::ShaderType::VERTEX].data(), raw_data, byte_size);
        }
        else if (i == 1) {
            const size_t   byte_size = source_code->getBufferSize();
            const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(source_code->getBufferPointer());

            size_t words_count = (byte_size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
            dst[ShaderProgram::ShaderType::FRAGMENT].resize(words_count, 0);
            std::memcpy(dst[ShaderProgram::ShaderType::FRAGMENT].data(), raw_data, byte_size);
        }
    }
}
