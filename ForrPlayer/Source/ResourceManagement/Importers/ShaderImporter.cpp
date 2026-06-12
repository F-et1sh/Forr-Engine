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

fe::pointer<fe::resource::Shader> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    Shader shader{};

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
    CompileAndReflect(shader.source_code, source_code, resource_management_context.graphics_backend);

    auto ptr = storage.CreateResource(std::move(shader));
    return ptr;
}

void fe::ShaderImporter::CompileAndReflect(std::vector<uint32_t>& dst, std::string_view src, GraphicsBackend graphics_backend) {
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

    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    slang_module->findEntryPointByName("main", entryPoint.writeRef());

    if (!entryPoint) {
        fe::logging::error("Slang : Entry point 'main' not found in shader");
        return;
    }

    slang::IComponentType*               components[] = { slang_module, entryPoint.get() };
    Slang::ComPtr<slang::IComponentType> program;
    session->createCompositeComponentType(components, 2, program.writeRef(), diagnostic_blob.writeRef());

    Slang::ComPtr<slang::IBlob> compiled_code{};
    int                         entry_point_index = 0;
    int                         target_index      = 0;

    if (SLANG_FAILED(program->getEntryPointCode(entry_point_index, target_index, compiled_code.writeRef(), diagnostic_blob.writeRef()))) {
        if (diagnostic_blob) {
            std::string_view errors(reinterpret_cast<const char*>(diagnostic_blob->getBufferPointer()), diagnostic_blob->getBufferSize());
            fe::logging::error("Slang Linker Failed :\n%s", errors.data());
        }
        return;
    }

    const size_t   byte_size = compiled_code->getBufferSize();
    const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(compiled_code->getBufferPointer());

    switch (graphics_backend) {
        case GraphicsBackend::OpenGL: {
            size_t words_count = (byte_size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
            dst.resize(words_count, 0);
            std::memcpy(dst.data(), raw_data, byte_size);
        } break;

        case GraphicsBackend::Vulkan: {
            dst.resize(byte_size / sizeof(uint32_t));
            std::memcpy(dst.data(), raw_data, byte_size);
        } break;

        default: {
            fe::logging::warning("The selected graphics backend %i for shader was not found. Using OpenGL by default", graphics_backend);
            size_t words_count = (byte_size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
            dst.resize(words_count, 0);
            std::memcpy(dst.data(), raw_data, byte_size);
        } break;
    }
}
