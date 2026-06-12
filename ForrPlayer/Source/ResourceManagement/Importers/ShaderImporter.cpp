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

    std::ifstream file(resource_full_path, std::ios::binary | std::ios::ate);
    if (!file.good()) {
        fe::logging::error("File -> Unified. Failed to open shader file\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    std::streampos file_size{};

    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string source_code{};

    source_code.resize(file_size);
    file.read((char*) &source_code[0], file_size);

    // remove BOM characters
    if (source_code.size() >= 3 &&
        (unsigned char) source_code[0] == 0xEF &&
        (unsigned char) source_code[1] == 0xBB &&
        (unsigned char) source_code[2] == 0xBF) {
        source_code = source_code.substr(3);
    }

    const auto& resource_management_context = storage.GetContext();
    CompileAndReflect(shader.source_codes, source_code, resource_management_context.graphics_backend);

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

    slang::TargetDesc target_desc{};

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
            fe::logging::warning("The selected graphics backend %i for shader was not found. Using OpenGL by default", graphics_backend);
            target_desc.format  = SLANG_GLSL;
            target_desc.profile = global_session->findProfile("glsl_450");
            break;
    }

    slang::SessionDesc session_desc{};
    session_desc.targets     = &target_desc;
    session_desc.targetCount = 1;

    Slang::ComPtr<slang::ISession> session{};
    global_session->createSession(session_desc, session.writeRef());

    Slang::ComPtr<slang::IBlob> diagnostic_blob{};

    slang::IModule* slang_module = session->loadModuleFromSourceString(nullptr, nullptr, src.data(), diagnostic_blob.writeRef());

    if (!slang_module) {
        if (diagnostic_blob) {
            std::string_view errors(reinterpret_cast<const char*>(diagnostic_blob->getBufferPointer()), diagnostic_blob->getBufferSize());
            fe::logging::error("Slang Compilation Failed :\n%s", errors.data());
        }
        return;
    }

    struct EntryPointTarget {
        std::string_view name{};
        SlangStage       stage{};

        EntryPointTarget()  = default;
        ~EntryPointTarget() = default;

        EntryPointTarget(std::string_view name, SlangStage stage)
            : name(name), stage(stage) {}
    };

    static std::array<EntryPointTarget, 3> targets_to_find{
        EntryPointTarget{ "vertexMain", SLANG_STAGE_VERTEX },
        EntryPointTarget{ "fragmentMain", SLANG_STAGE_FRAGMENT },
        EntryPointTarget{ "computeMain", SLANG_STAGE_COMPUTE }
    };

    std::vector<slang::IComponentType*>            components{ slang_module };
    std::vector<Slang::ComPtr<slang::IEntryPoint>> entry_points{};

    for (const auto& target : targets_to_find) {
        Slang::ComPtr<slang::IEntryPoint> entry_point{};
        slang_module->findEntryPointByName(target.name.data(), entry_point.writeRef());

        if (entry_point) {
            entry_points.push_back(entry_point);
            components.push_back(entry_point.get());
        }
    }
    std::vector<ShaderProgram::ShaderType> active_stages;

    for (size_t i = 0; i < targets_to_find.size(); i++) {
        Slang::ComPtr<slang::IEntryPoint> entry_point{};
        slang_module->findEntryPointByName(targets_to_find[i].name.data(), entry_point.writeRef());
        if (entry_point) {
            entry_points.push_back(entry_point);
            active_stages.push_back(static_cast<ShaderProgram::ShaderType>(i));
        }
    }

    for (size_t i = 0; i < entry_points.size(); i++) {
        Slang::ComPtr<slang::IComponentType> linked_entry_point{};
        //linked_entry_point->link()

        //if (SLANG_FAILED(session->linkEntryPoint(entry_points[i].get(), linked_entry_point.writeRef(), diagnostic_blob.writeRef()))) {
        //    if (diagnostic_blob) {
        //        std::string_view errors(reinterpret_cast<const char*>(diagnostic_blob->getBufferPointer()), diagnostic_blob->getBufferSize());
        //        fe::logging::error("Slang LinkEntryPoint Failed for stage %i :\n%s", active_stages[i], errors.data());
        //    }
        //    continue;
        //}

        Slang::ComPtr<slang::IBlob> compiled_code;
        if (SLANG_SUCCEEDED(linked_entry_point->getEntryPointCode(0, 0, compiled_code.writeRef(), diagnostic_blob.writeRef()))) {

            const size_t   byte_size = compiled_code->getBufferSize();
            const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(compiled_code->getBufferPointer());

            auto shader_type = active_stages[i];

            switch (graphics_backend) {
                case GraphicsBackend::OpenGL: {
                    size_t words_count = (byte_size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
                    dst[shader_type].resize(words_count, 0);
                    std::memcpy(dst[shader_type].data(), raw_data, byte_size);
                } break;

                case GraphicsBackend::Vulkan: {
                    dst[shader_type].resize(byte_size / sizeof(uint32_t));
                    std::memcpy(dst[shader_type].data(), raw_data, byte_size);
                } break;

                default: {
                    fe::logging::warning("The selected graphics backend %i for shader was not found. Using OpenGL by default", graphics_backend);
                    size_t words_count = (byte_size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
                    dst[shader_type].resize(words_count, 0);
                    std::memcpy(dst[shader_type].data(), raw_data, byte_size);
                } break;
            }
        }
        else {
            if (diagnostic_blob) {
                std::string_view errors(reinterpret_cast<const char*>(diagnostic_blob->getBufferPointer()), diagnostic_blob->getBufferSize());
                fe::logging::error("Slang failed to get entry point code for stage %i :\n%s", active_stages[i], errors.data());
            }
        }
    }
}
