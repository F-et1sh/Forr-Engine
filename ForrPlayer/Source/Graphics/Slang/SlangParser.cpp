/*===============================================

    Forr Engine

    File : SlangParser.cpp
    Role : this class compiles and reflects Slang shaders

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "SlangParser.hpp"

fe::SlangParser::SlangParser() {
    static Slang::ComPtr<slang::IGlobalSession> global_session{};
    if (!global_session) {
        if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef()))) {
            fe::logging::error("Slang -> Unified. Failed to create global session");
            return;
        }
    }

    slang::SessionDesc session_desc{};
    slang::TargetDesc  target_desc{};

    target_desc.format  = SLANG_SPIRV;
    target_desc.profile = global_session->findProfile("spirv_1_5");
    target_desc.flags   = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

    target_desc.flags |= SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM;

    session_desc.targets                  = &target_desc;
    session_desc.targetCount              = 1;
    session_desc.compilerOptionEntryCount = 0;

    // TODO : provide an interface for this
    const char* search_paths = { (PATH.getDefaultShadersPath() / "PBRMaterial").string().c_str() };

    session_desc.searchPathCount = 1;
    session_desc.searchPaths     = &search_paths;

    if (SLANG_FAILED(global_session->createSession(session_desc, m_Session.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to create a session");
        return;
    }
}

bool fe::SlangParser::LoadFromFileAndReflect(const std::filesystem::path& resource_full_path, resource::ShaderReflectedData& shader_reflected_data) {
    Slang::ComPtr<slang::IBlob> diagnostics{};
    m_Module = m_Session->loadModule(resource_full_path.generic_string().c_str(), diagnostics.writeRef());
    if (!m_Module) {
        fe::logging::error("Slang -> Unified. Failed to load a slang module\n%s", (const char*) diagnostics->getBufferPointer());
        return false;
    }

    Slang::ComPtr<ISlangBlob> serialized_blob{};
    if (SLANG_FAILED(m_Module->serialize(serialized_blob.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to get serialized data from Slang");
        return false;
    }

    const uint8_t* buffer_data = (const uint8_t*) serialized_blob->getBufferPointer();
    size_t         buffer_size = serialized_blob->getBufferSize();

    // we store this data to compile the shader later
    shader_reflected_data.slang_serialized_data.assign(buffer_data, buffer_data + buffer_size);

    m_RootLayout = m_Module->getLayout();

    std::vector<slang::IComponentType*> component_types{};
    component_types.emplace_back(m_Module);

    std::vector<EntryPoint> entry_points = std::move(this->findEntryPoints(component_types));

    if (!entry_points.empty()) {
        SlangResult result = m_Session->createCompositeComponentType(component_types.data(), component_types.size(), m_CompusedProgram.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Slang -> Unified. Failed to create a composed program");
            return false;
        }

        m_RootLayout = m_CompusedProgram->getLayout();
    }

    this->reflect(shader_reflected_data);

    return true;
}

std::vector<fe::EntryPoint> fe::SlangParser::findEntryPoints(std::vector<slang::IComponentType*>& component_types) {
    std::vector<EntryPoint> entry_points{};

    for (std::uint32_t i = 0; i < std::size(ENTRY_POINT_NAMES); i++) {
        auto                entry_point_name = ENTRY_POINT_NAMES[i];
        slang::IEntryPoint* entry_point{};
        m_Module->findEntryPointByName(entry_point_name.data(), &entry_point);

        if (entry_point) {
            shader::StageBits shader_type{};

            if (entry_point_name == ENTRY_POINT_NAMES[0])
                shader_type = shader::StageBits::VERTEX;
            else if (entry_point_name == ENTRY_POINT_NAMES[1])
                shader_type = shader::StageBits::FRAGMENT;
            else if (entry_point_name == ENTRY_POINT_NAMES[2])
                shader_type = shader::StageBits::COMPUTE;

            entry_points.emplace_back(entry_point, shader_type);
            component_types.emplace_back(entry_point);
        }
    }

    return entry_points;
}

bool fe::SlangParser::reflect(resource::ShaderReflectedData& shader_reflected_data) {
    slang::DeclReflection* module_reflection = m_Module->getModuleReflection();

    auto list = module_reflection->getChildren();
    for (auto child : list) {
        //fe::logging::debug(child->getName() ? child->getName() : "");
    }

    return true;
}

bool fe::SlangParser::reflectMaterial(shader::ReflectedMaterialLayout& material_layout) {
    return true;
}
