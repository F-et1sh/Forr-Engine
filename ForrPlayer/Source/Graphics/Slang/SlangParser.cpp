/*===============================================

    Forr Engine

    File : SlangParser.cpp
    Role : this class compiles and reflects Slang shaders

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "SlangParser.hpp"

namespace fe {
    using ShaderDescriptor = shader::DescriptorType;
    using ShaderValue      = shader::ValueType;
    using ShaderType       = shader::StageBits;

    using SlangKind     = slang::TypeReflection::Kind;
    using SlangDeclKind = slang::DeclReflection::Kind;
    using SlangScalar   = slang::TypeReflection::ScalarType;
    using SlangCategory = slang::ParameterCategory;
} // namespace fe

fe::SlangParser::SlangParser(std::span<const char*> full_search_paths) {
    if (m_GlobalSession && m_Session) return;

    if (SLANG_FAILED(slang::createGlobalSession(m_GlobalSession.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to create global session");
        return;
    }

    slang::SessionDesc session_desc{};
    slang::TargetDesc  target_desc{};

    target_desc.format  = SLANG_SPIRV;
    target_desc.profile = m_GlobalSession->findProfile("spirv_1_5");
    target_desc.flags   = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

    target_desc.flags |= SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM;

    session_desc.targets                  = &target_desc;
    session_desc.targetCount              = 1;
    session_desc.compilerOptionEntryCount = 0;

    if (full_search_paths.empty()) {
        std::array<const char*, 1> paths{ PATH.getShadersPath().generic_string().c_str() };

        session_desc.searchPathCount = paths.size();
        session_desc.searchPaths     = paths.data();
    }
    else {
        session_desc.searchPathCount = full_search_paths.size();
        session_desc.searchPaths     = full_search_paths.data();
    }

    if (SLANG_FAILED(m_GlobalSession->createSession(session_desc, m_Session.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to create a session");
        return;
    }
}

std::expected<fe::shader::ProgramSources, fe::SlangParser::ShaderBuildErrors>
fe::SlangParser::BuildShaderSources(const graphics::PipelineDesc& pipeline_desc, const ResourceManager& resource_manager) {
    std::vector<Slang::ComPtr<slang::IModule>> loaded_modules{};

    loaded_modules.reserve(pipeline_desc.shader_file_data_ptrs.size());

    // find all modules
    for (auto shader_file_ptr : pipeline_desc.shader_file_data_ptrs) {
        auto shader_file_data_raw_ptr = resource_manager.GetResource(shader_file_ptr);
        if (!shader_file_data_raw_ptr) {
            fe::logging::warning("Failed to get shader file data resource\nShader file data ptr :\nindex = %i\ngeneration = %i.\nContinuing building the shader codes",
                                 static_cast<uint32_t>(shader_file_ptr.index()),
                                 static_cast<uint32_t>(shader_file_ptr.generation()));
            continue;
        }

        const auto& shader_file_data = *shader_file_data_raw_ptr;

        if (shader_file_data.slang_serialized_data.empty() ||
            shader_file_data.slang_serialized_data.data() == nullptr) {
            fe::logging::warning("Serialized Slang ( Unified ) -> Slang. Failed to deserialize shader file data's module. It was empty.\nContinuing building the shader codes");
            continue;
        }

        Slang::ComPtr<ISlangBlob> blob{};
        ISlangBlob*               blob_raw = slang_createBlob(shader_file_data.slang_serialized_data.data(),
                                                              shader_file_data.slang_serialized_data.size());
        blob.attach(blob_raw);

        Slang::ComPtr<slang::IBlob> load_diagnostics{};
        slang::IModule*             loaded_module_raw = m_Session->loadModuleFromIRBlob(shader_file_data.full_path.c_str(),
                                                                                        shader_file_data.full_path.c_str(),
                                                                                        blob_raw,
                                                                                        load_diagnostics.writeRef());

        if (!loaded_module_raw) {
            fe::logging::warning("Serialized Slang ( Unified ) -> Slang. Failed to deserialize shader file data's module. %s.\nContinuing building the shader codes",
                                 (const char*) load_diagnostics->getBufferPointer());
            continue;
        }

        loaded_modules.emplace_back(loaded_module_raw);

        uint32_t dependency_count = loaded_module_raw->getDependencyFileCount();
        loaded_modules.reserve(loaded_modules.size() + dependency_count);
        for (uint32_t i = 0; i < dependency_count; i++) {
            const char* dependency_file = loaded_module_raw->getDependencyFilePath(i);

            Slang::ComPtr<slang::IBlob> load_diagnostics{};
            slang::IModule*             imported_module_raw = m_Session->loadModule(dependency_file, load_diagnostics.writeRef());
            if (imported_module_raw) {
                int parameters_count = imported_module_raw->getSpecializationParamCount();
                loaded_modules.emplace_back(imported_module_raw);
            }
            else {
                fe::logging::warning("Slang -> Unified. Failed to load a slang dependency module. Continuing loading\n%s",
                                     (const char*) load_diagnostics->getBufferPointer());
                continue;
            }
        }
    }

    std::vector<slang::IComponentType*> component_types{};
    // add all loaded modules
    component_types.append_range(loaded_modules);

    // TODO : provide descriptors and push constants checking

    struct EntryPoint {
        Slang::ComPtr<slang::IEntryPoint> entry_point{};
        ShaderType                        shader_type{};
        std::string_view                  entry_point_name{};

        EntryPoint() = default;
        EntryPoint(slang::IEntryPoint* entry_point,
                   ShaderType          shader_type,
                   std::string_view    entry_point_name)
            : entry_point(entry_point),
              shader_type(shader_type),
              entry_point_name(entry_point_name) {}

        FORR_CLASS_MOVABLE(EntryPoint)
        FORR_CLASS_NONCOPYABLE(EntryPoint)
    };

    std::vector<EntryPoint> entry_points{};

    entry_points.reserve(pipeline_desc.entry_points.size());
    for (const fe::hashed_string& entry_point_name : pipeline_desc.entry_points) {

        Slang::ComPtr<slang::IEntryPoint> found_entry_point{};
        ShaderType                        shader_type{};

        // find the module
        for (slang::IModule* loaded_module : loaded_modules) {

            // TODO : remove
            fe::logging::debug("Searching entry point %s in module %s", entry_point_name.c_str(), loaded_module->getName());

            Slang::ComPtr<slang::IEntryPoint> entry_point{};
            SlangResult                       result = loaded_module->findEntryPointByName(entry_point_name.c_str(), entry_point.writeRef()); // find this entry point in shader's module

            if (SLANG_SUCCEEDED(result)) {
                if (found_entry_point) {
                    fe::logging::warning("Found another declaration of entry point %s", entry_point_name.c_str());
                    continue;
                }
                found_entry_point.attach(entry_point.detach());
            }
        }

        if (!found_entry_point) {
            fe::logging::error("Entry point %s was not found in any loaded modules", entry_point_name.c_str());
            continue;
        }

        // find the stage
        slang::ProgramLayout* entry_point_layout = found_entry_point->getLayout();

        if (!entry_point_layout && entry_point_layout->getEntryPointCount() <= 0) {
            fe::logging::error("Failed to get layout for entry point %s", entry_point_name.c_str());
            continue;
        }

        slang::EntryPointLayout* entry_point_reflection = entry_point_layout->getEntryPointByIndex(0);
        SlangStage               slang_stage            = entry_point_reflection->getStage();

        switch (slang_stage) {
                // clang-format off
            case SLANG_STAGE_VERTEX  : shader_type = ShaderType::VERTEX  ; break;
            case SLANG_STAGE_GEOMETRY: shader_type = ShaderType::GEOMETRY; break;
            case SLANG_STAGE_FRAGMENT: shader_type = ShaderType::FRAGMENT; break;
            case SLANG_STAGE_COMPUTE : shader_type = ShaderType::COMPUTE ; break;
            default:
                fe::logging::error("Unknown Slang stage for entry point %s. Skipping it", entry_point_name.c_str());
                continue;
                // clang-format on
        }

        // if there is nothing to specialize, then the entry point doesn't need to be specializaed.
        // that means it's completed and we can add it
        if (!pipeline_desc.specialization.has_value()) {
            entry_points.emplace_back(EntryPoint{ found_entry_point.detach(), shader_type, entry_point_name });
            continue;
        }

        const auto& specialization = pipeline_desc.specialization.value();

        // search for this entry point in the specialization
        auto it = std::ranges::find_if(specialization.entry_points, [&entry_point_name](const auto& e) -> bool {
            return e.name == entry_point_name;
        });

        uint32_t slang_specialization_parameters_count = found_entry_point->getSpecializationParamCount();

        // if didn't find this entry point in the specialization, then most likely it doesn't need to be specialized
        if (it == specialization.entry_points.end()) {
            if (slang_specialization_parameters_count != 0) {
                fe::logging::error("Failed to specialize an entry point %s. This entry point needs to be specialized with %i argument(s), but there is nothing about it in the specialization arguments.",
                                   entry_point_name.c_str(),
                                   slang_specialization_parameters_count);
                continue;
            }
            entry_points.emplace_back(EntryPoint{ found_entry_point.detach(), shader_type, entry_point_name });
            continue;
        }

        // check if Slang and Unified match each other
        if (slang_specialization_parameters_count != it->arguments.size()) {
            fe::logging::error("Failed to specialize an entry point. Slang specialization parameters count was %i, while you passed %i argument(s)",
                               slang_specialization_parameters_count,
                               it->arguments.size());
            continue;
        }

        std::vector<slang::SpecializationArg> specialization_arguments{};
        specialization_arguments.resize(it->arguments.size());

        // this is needed because 'const char*' in 'slang::SpecializationArg::expr' must live till
        // calling the 'slang::IComponentType::specialize()' function
        std::vector<std::string> string_pool{};

        // this is needed to find a type by name for specialization
        Slang::ComPtr<slang::IComponentType> composed_program{};

        // collect specialization arguments
        string_pool.reserve(it->arguments.size());
        for (size_t i = 0; i < it->arguments.size(); i++) {
            const auto& argument       = it->arguments[i];
            auto&       slang_argument = specialization_arguments[i];

            if (std::holds_alternative<fe::hashed_string>(argument.value)) {
                // initialize 'composed_program'
                if (!composed_program) {
                    Slang::ComPtr<slang::IBlob> composition_diagnostics{};
                    SlangResult                 composition_result = m_Session->createCompositeComponentType(component_types.data(),
                                                                                                             component_types.size(),
                                                                                                             composed_program.writeRef(),
                                                                                                             composition_diagnostics.writeRef());
                    if (SLANG_FAILED(composition_result)) {
                        fe::logging::error("Failed to create a composed program for specialization for entry point %s\n%s",
                                           entry_point_name.c_str(),
                                           (const char*) composition_diagnostics->getBufferPointer());
                        continue;
                    }
                }

                fe::hashed_string type_name = std::get<fe::hashed_string>(argument.value);

                slang::ProgramLayout*  program_layout  = composed_program->getLayout();
                slang::TypeReflection* type_reflection = program_layout->findTypeByName(type_name.c_str());

                if (!type_reflection) {
                    fe::logging::error("Failed find %s by name for specialization for entry point %s",
                                       type_name.c_str(),
                                       entry_point_name.c_str());
                    continue;
                }

                slang_argument = slang::SpecializationArg::fromType(type_reflection);
            }
            else {
                auto& expr_string = string_pool.emplace_back();

                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, bool>) {
                        expr_string = arg ? "true" : "false";
                    }
                    else if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
                                       std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
                        expr_string = std::to_string(arg);
                    }
                    else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                        expr_string = std::to_string(arg) + "f";
                    }
                },
                           argument.value);

                slang_argument = slang::SpecializationArg::fromExpr(expr_string.c_str());
            }
        }

        // specialize the entry point
        slang::IComponentType*      component_type_raw{};
        Slang::ComPtr<slang::IBlob> specialization_diagnostics{};

        SlangResult result = found_entry_point->specialize(specialization_arguments.data(),
                                                           specialization_arguments.size(),
                                                           &component_type_raw,
                                                           specialization_diagnostics.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Failed to specialize an entry point %s\n%s",
                               entry_point_name.c_str(),
                               (const char*) specialization_diagnostics->getBufferPointer());
            continue;
        }

        // add completed entry point
        entry_points.emplace_back(EntryPoint{ found_entry_point.detach(), shader_type, entry_point_name });
    }

    // add all entry points
    component_types.reserve(component_types.size() + entry_points.size());
    for (EntryPoint& entry_point : entry_points) {
        component_types.emplace_back(entry_point.entry_point);
    }

    // create final composite to extract the sources
    Slang::ComPtr<slang::IComponentType> composed_program{};

    Slang::ComPtr<slang::IBlob> composition_diagnostics{};
    SlangResult                 composition_result = m_Session->createCompositeComponentType(component_types.data(),
                                                                                             component_types.size(),
                                                                                             composed_program.writeRef(),
                                                                                             composition_diagnostics.writeRef());
    if (SLANG_FAILED(composition_result)) {
        fe::logging::error("Failed to create a composed program\n%s",
                           (const char*) composition_diagnostics->getBufferPointer());
        return std::unexpected{ ShaderBuildErrors::COMPOSITION_FAILED };
    }

    shader::ProgramSources source_codes{};

    for (size_t i = 0; i < entry_points.size(); i++) {
        const EntryPoint& entry_point = entry_points[i];

        Slang::ComPtr<slang::IBlob> spirv_code{};

        Slang::ComPtr<slang::IBlob> entry_point_code_diagnostics{};
        SlangResult                 result = composed_program->getEntryPointCode(i, 0, spirv_code.writeRef(), entry_point_code_diagnostics.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Failed to get the %s entry point source code\n%s",
                               entry_point.entry_point_name,
                               (const char*) entry_point_code_diagnostics->getBufferPointer());
            continue;
        }

        const size_t   byte_size = spirv_code->getBufferSize();
        const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(spirv_code->getBufferPointer());

        auto& source_code_dst = source_codes[entry_point.shader_type];

        source_code_dst.resize(byte_size, 0);
        std::memcpy(source_code_dst.data(), raw_data, byte_size);
    }

    return source_codes;
}

std::expected<fe::resource::ShaderFileData, fe::SlangParser::ShaderFileDataErrors>
fe::SlangParser::BuildShaderFileData(const std::filesystem::path& resource_full_path, const ResourceStorage& storage) {
    resource::ShaderFileData shader_file_data{};
    shader_file_data.full_path = resource_full_path.generic_string().c_str();

    // load module

    Slang::ComPtr<slang::IModule> slang_module{};

    Slang::ComPtr<slang::IBlob> load_diagnostics{};
    slang_module = m_Session->loadModule(resource_full_path.generic_string().c_str(), load_diagnostics.writeRef());
    if (!slang_module) {
        fe::logging::error("Slang -> Unified. Failed to load a slang module\n%s",
                           (const char*) load_diagnostics->getBufferPointer());
        return std::unexpected{ SlangParser::ShaderFileDataErrors::FAILED_TO_LOAD_SLANG_MODULE };
    }

    // extract serialized

    Slang::ComPtr<ISlangBlob> serialized_blob{};
    if (SLANG_FAILED(slang_module->serialize(serialized_blob.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to get serialized data from Slang");
        return std::unexpected{ SlangParser::ShaderFileDataErrors::FAILED_TO_GET_SERIALIZED_DATA };
    }

    const uint8_t* buffer_data = (const uint8_t*) serialized_blob->getBufferPointer();
    size_t         buffer_size = serialized_blob->getBufferSize();

    // we store this data to compile the shader later
    shader_file_data.slang_serialized_data.assign(buffer_data, buffer_data + buffer_size);

    // reflect

    Slang::ComPtr<slang::IComponentType> composed_program{};

    // there is no need to search for entry points here
    std::vector<slang::IComponentType*> component_types{};

    uint32_t dependency_count = slang_module->getDependencyFileCount();
    component_types.reserve(dependency_count);
    for (uint32_t i = 0; i < dependency_count; i++) { // starting with '0' here adds 'slang_module' itself too
        const char* dependency_file = slang_module->getDependencyFilePath(i);

        Slang::ComPtr<slang::IBlob> load_diagnostics{};
        slang::IModule*             imported_module = m_Session->loadModule(dependency_file, load_diagnostics.writeRef());
        if (imported_module) {
            int parameters_count = imported_module->getSpecializationParamCount();
            component_types.emplace_back(imported_module);
        }
        else {
            fe::logging::error("Slang -> Unified. Failed to load a slang dependency module. Continuing loading\n%s",
                               (const char*) load_diagnostics->getBufferPointer());
        }
    }

    Slang::ComPtr<slang::IBlob> composition_diagnostics{};
    SlangResult                 result = m_Session->createCompositeComponentType(component_types.data(),
                                                                                 component_types.size(),
                                                                                 composed_program.writeRef(),
                                                                                 composition_diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        fe::logging::error("Slang -> Unified. Failed to create a composed program\n%s",
                           (const char*) composition_diagnostics->getBufferPointer());
        return std::unexpected{ SlangParser::ShaderFileDataErrors::FAILED_TO_CREATE_COMPOSED_PROGRAM };
    }

    slang::ProgramLayout* layout = composed_program->getLayout();
    for (size_t i = 0; i < layout->getEntryPointCount(); i++) {
        slang::EntryPointReflection* entry_point_reflection = layout->getEntryPointByIndex(i);
        std::string entry_point_name = entry_point_reflection->getName();
        fe::logging::debug("Loaded entry point name : %s", entry_point_name.c_str());
    }

    return shader_file_data;
}
