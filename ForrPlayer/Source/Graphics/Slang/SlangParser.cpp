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

    if (SLANG_FAILED(global_session->createSession(session_desc, m_Session.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to create a session");
        return;
    }
}

bool fe::SlangParser::LoadFromFileAndReflect(const std::filesystem::path& resource_full_path, resource::ShaderReflectedData& shader_reflected_data) {
    m_Module = m_Session->loadModule(resource_full_path.string().c_str());
    if (!m_Module) {
        fe::logging::error("Slang -> Unified. Failed to load a slang module");
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
    std::vector<EntryPoint>             entry_points = std::move(this->findEntryPoints(component_types));

    if (!entry_points.empty()) {
        Slang::ComPtr<slang::IComponentType> composed_program{};

        SlangResult result = m_Session->createCompositeComponentType(component_types.data(), component_types.size(), composed_program.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Slang -> Unified. Failed to create a composed program");
            return false;
        }

        m_RootLayout = composed_program->getLayout();
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

    
    return true;
}

bool fe::SlangParser::reflectMaterial(shader::ReflectedMaterialLayout& material_layout) {
    return true;
}

bool fe::SlangParser::WriteSerializedData(resource::ShaderReflectedData& shader_reflected_data) {
    Slang::ComPtr<ISlangBlob> serialized_blob{};
    if (SLANG_FAILED(m_Module->serialize(serialized_blob.writeRef()))) {
        return false;
    }

    const uint8_t* buffer_data = (const uint8_t*) serialized_blob->getBufferPointer();
    size_t         buffer_size = serialized_blob->getBufferSize();

    // we store this data to compile the shader later
    shader_reflected_data.slang_serialized_data.assign(buffer_data, buffer_data + buffer_size);

    m_RootLayout = m_Module->getLayout();

    return true;
}

std::vector<fe::EntryPoint> fe::SlangParser::FindEntryPoints() {
    std::vector<slang::IComponentType*> component_types{};
    component_types.emplace_back(m_Module);

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

bool fe::SlangParser::WriteReflectedData(resource::ShaderReflectedData& shader_reflected_data) {
    std::vector<slang::IComponentType*> component_types{};
    component_types.emplace_back(m_Module);

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

    if (!entry_points.empty()) {
        Slang::ComPtr<slang::IComponentType> composed_program{};

        SlangResult result = m_Session->createCompositeComponentType(component_types.data(), component_types.size(), composed_program.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Slang -> Unified. Failed to create a composed program");
            return false;
        }

        for (std::size_t i = 0; i < entry_points.size(); i++) {
            const auto&                 entry_point = entry_points[i];
            Slang::ComPtr<slang::IBlob> spirv_code{};

            SlangResult result = composed_program->getEntryPointCode(i, 0, spirv_code.writeRef());
            if (SLANG_FAILED(result)) {
                fe::logging::error("Slang -> Unified. Failed to get an entry point code\nEntry point index : %i", i);
                return false;
            }

            const size_t   byte_size = spirv_code->getBufferSize();
            const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(spirv_code->getBufferPointer());

            shader::StageBits shader_type     = entry_point.shader_type;
            auto&             source_code_dst = context.shader_program.source_codes[shader_type];

            source_code_dst.resize(byte_size, 0);
            std::memcpy(source_code_dst.data(), raw_data, byte_size);
        }

        slang::ProgramLayout* program_layout = composed_program->getLayout();
        if (!program_layout) FORR_UNLIKELY {
            fe::logging::error("Slang -> Unified. Failed to get the layout of the composed program in fe::ShaderImporter::compile()");
            return false;
        }

        context.root_layout = composed_program->getLayout();
    }

    return true;
}
