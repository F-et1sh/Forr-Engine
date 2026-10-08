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

        uint32_t dependency_count = loaded_module_raw->getDependencyFileCount();
        loaded_modules.reserve(loaded_modules.size() + dependency_count);
        for (uint32_t i = 0; i < dependency_count; i++) { // starting with '0', because it's already contains the main file
            const char* dependency_file = loaded_module_raw->getDependencyFilePath(i);

            Slang::ComPtr<slang::IBlob> load_diagnostics{};
            slang::IModule*             imported_module_raw = m_Session->loadModule(dependency_file, load_diagnostics.writeRef());
            if (imported_module_raw) {
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

    struct EntryPoint {
        Slang::ComPtr<slang::IComponentType> entry_point{};
        ShaderType                           shader_type{};
        std::string_view                     entry_point_name{};

        EntryPoint() = default;
        EntryPoint(slang::IComponentType* entry_point,
                   ShaderType             shader_type,
                   std::string_view       entry_point_name)
            : entry_point(entry_point),
              shader_type(shader_type),
              entry_point_name(entry_point_name) {}

        FORR_CLASS_MOVABLE(EntryPoint)
        FORR_CLASS_NONCOPYABLE(EntryPoint)
    };

    std::vector<EntryPoint> entry_points{};

    // collect all entry points
    entry_points.reserve(pipeline_desc.entry_points.size());
    for (const fe::hashed_string& entry_point_name : pipeline_desc.entry_points) {

        Slang::ComPtr<slang::IEntryPoint> found_entry_point{};
        ShaderType                        shader_type{};

        // find the module
        for (slang::IModule* loaded_module : loaded_modules) {
            // search for this entry point in shader's module
            SlangResult result = loaded_module->findEntryPointByName(entry_point_name.c_str(), found_entry_point.writeRef());

            // leave the loop if found successfully
            if (SLANG_SUCCEEDED(result)) break;
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
            fe::logging::error("Failed to specialize an entry point. Slang specialization parameters count was %i, while there was %i argument(s)",
                               slang_specialization_parameters_count,
                               it->arguments.size());
            continue;
        }

        std::vector<slang::SpecializationArg> specialization_arguments{};
        specialization_arguments.resize(it->arguments.size(), {});

        // this is needed because 'const char*' in 'slang::SpecializationArg::expr' must live till
        // calling the 'slang::IComponentType::specialize()' function
        std::vector<std::string> string_pool{};

        // collect specialization arguments
        string_pool.reserve(it->arguments.size());
        for (size_t i = 0; i < it->arguments.size(); i++) {
            const auto& argument       = it->arguments[i];
            auto&       slang_argument = specialization_arguments[i];

            if (std::holds_alternative<fe::hashed_string>(argument.value)) {
                fe::hashed_string type_name = std::get<fe::hashed_string>(argument.value);

                for (slang::IComponentType* component_type : component_types) {
                    slang::ProgramLayout*  module_layout   = component_type->getLayout();
                    slang::TypeReflection* type_reflection = module_layout->findTypeByName(type_name.c_str());

                    if (type_reflection) {
                        slang_argument = slang::SpecializationArg::fromType(type_reflection);
                    }
                }

                if (!slang_argument.type) {
                    fe::logging::error("Failed to find %s by name for specialization of entry point %s",
                                       type_name.c_str(),
                                       entry_point_name.c_str());
                    continue;
                }
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
        Slang::ComPtr<slang::IComponentType> specialized_entry_point{};
        Slang::ComPtr<slang::IBlob>          specialization_diagnostics{};

        SlangResult result = found_entry_point->specialize(specialization_arguments.data(),
                                                           specialization_arguments.size(),
                                                           specialized_entry_point.writeRef(),
                                                           specialization_diagnostics.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Failed to specialize an entry point %s\n%s",
                               entry_point_name.c_str(),
                               (const char*) specialization_diagnostics->getBufferPointer());
            continue;
        }

        // add completed entry point
        entry_points.emplace_back(EntryPoint{ specialized_entry_point.detach(), shader_type, entry_point_name });
    }

    // add all entry points
    component_types.reserve(component_types.size() + entry_points.size());
    for (EntryPoint& entry_point : entry_points) {
        component_types.emplace_back(entry_point.entry_point);
    }

    // create final composite
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

    // specialize global parameters
    Slang::ComPtr<slang::IComponentType> specialized_program = SlangParser::specialize(pipeline_desc, composed_program, component_types);
    if (!specialized_program) {
        return std::unexpected{ ShaderBuildErrors::FAILED_TO_SPECIALIZE_GLOBAL_PARAMETERS };
    }

    // extract source codes
    shader::ProgramSources source_codes{};

    for (size_t i = 0; i < entry_points.size(); i++) {
        const EntryPoint& entry_point = entry_points[i];

        Slang::ComPtr<slang::IBlob> spirv_code{};

        Slang::ComPtr<slang::IBlob> entry_point_code_diagnostics{};
        SlangResult                 result = specialized_program->getEntryPointCode(i, 0, spirv_code.writeRef(), entry_point_code_diagnostics.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Failed to get the %s entry point source code\n%s",
                               entry_point.entry_point_name.data(),
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

    // there is no need to search for entry points here
    std::vector<slang::IComponentType*> component_types{};

    std::vector<Slang::ComPtr<slang::IEntryPoint>> entry_points{};

    // load all dependencies and reflect entry points
    size_t dependency_count = slang_module->getDependencyFileCount();
    component_types.reserve(dependency_count);
    for (size_t i = 0; i < dependency_count; i++) { // starting with '0' here adds 'slang_module' itself too
        const char* dependency_file = slang_module->getDependencyFilePath(i);

        Slang::ComPtr<slang::IBlob> load_diagnostics{};
        slang::IModule*             imported_module = m_Session->loadModule(dependency_file, load_diagnostics.writeRef());
        if (imported_module) {
            slang::ShaderReflection* module_reflection = imported_module->getLayout();

            // fun though all defined entry points and collect information about them
            size_t defined_entry_point_count = imported_module->getDefinedEntryPointCount();
            shader_file_data.entry_points.reserve(defined_entry_point_count);
            for (size_t defined_entry_point_i = 0; defined_entry_point_i < defined_entry_point_count; defined_entry_point_i++) {

                Slang::ComPtr<slang::IEntryPoint> entry_point{};
                if (SLANG_FAILED(imported_module->getDefinedEntryPoint(defined_entry_point_i, entry_point.writeRef()))) {
                    fe::logging::error("Slang -> Unified. Failed to get defined entry point\nEntry point index : %i", defined_entry_point_i);
                    continue;
                }

                slang::FunctionReflection* function_reflection = entry_point->getFunctionReflection();
                if (!function_reflection) {
                    fe::logging::error("Slang -> Unified. Failed to get function reflection\nEntry point index : %i", defined_entry_point_i);
                    continue;
                }

                auto& this_entry_point = shader_file_data.entry_points.emplace_back();

                // collect name
                auto functoin_name_raw = function_reflection->getName();
                this_entry_point.name  = functoin_name_raw ? functoin_name_raw : m_UnknownVariableName;

                // collect generic arguments
                slang::GenericReflection* generic_reflection = function_reflection->getGenericContainer();

                size_t type_parameter_count = generic_reflection->getTypeParameterCount();
                this_entry_point.generic_arguments.reserve(type_parameter_count);
                for (size_t type_parameter_i = 0; type_parameter_i < type_parameter_count; type_parameter_i++) {

                    slang::VariableReflection* variable_reflection = generic_reflection->getTypeParameter(type_parameter_i);

                    auto& constraints = this_entry_point.generic_arguments.emplace_back();

                    size_t constraint_count = generic_reflection->getTypeParameterConstraintCount(variable_reflection);
                    constraints.reserve(constraint_count);
                    for (size_t constraint_i = 0; constraint_i < constraint_count; constraint_i++) {

                        slang::TypeReflection* type_reflection = generic_reflection->getTypeParameterConstraintType(variable_reflection, constraint_i);
                        auto                   type_name_raw   = type_reflection->getName();
                        std::string            name            = type_name_raw ? type_name_raw : m_UnknownVariableName;

                        constraints.emplace_back(std::move(name.c_str()));
                    }
                }

                // TODO : provide stage detection
                this_entry_point.stage_flags |= static_cast<uint32_t>(ShaderType::VERTEX);
                this_entry_point.stage_flags |= static_cast<uint32_t>(ShaderType::FRAGMENT);
                this_entry_point.stage_flags |= static_cast<uint32_t>(ShaderType::COMPUTE);
                this_entry_point.stage_flags |= static_cast<uint32_t>(ShaderType::GEOMETRY);
            }

            component_types.emplace_back(imported_module);
        }
        else {
            fe::logging::error("Slang -> Unified. Failed to load a slang dependency module. Continuing loading\n%s",
                               (const char*) load_diagnostics->getBufferPointer());
        }
    }

    Slang::ComPtr<slang::IComponentType> composed_program{};

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

    // reflect descriptors and push constants
    slang::ProgramLayout* layout          = composed_program->getLayout();
    unsigned int          parameter_count = layout->getParameterCount();

    if (parameter_count != 0) {
        shader_file_data.descriptor_layouts.reserve(parameter_count);
        shader_file_data.push_constants_layouts.reserve(parameter_count / 4);

        for (unsigned int i = 0; i < parameter_count; i++) {
            slang::VariableLayoutReflection* variable_layout = layout->getParameterByIndex(i);
            if (!variable_layout) {
                fe::logging::error("Slang -> Unified. Failed to reflect a variable\nslang::VariableLayoutReflection* variable_layout = context.root_layout->getParameterByIndex(i) was nullptr. i = %i",
                                   i);
                continue;
            }

            // this reflects descriptors and push constants
            SlangParser::parseVariableRecursive(variable_layout,
                                                shader_file_data.descriptor_layouts,
                                                shader_file_data.push_constants_layouts);
        }
    }

    // reflect structures
    slang::DeclReflection* module_reflection = slang_module->getModuleReflection();

    size_t children_count = module_reflection->getChildrenCount();
    shader_file_data.structure_layouts.reserve(children_count);

    // unwrap first module - it always exists, even if the file is empty
    auto list = module_reflection->getChildren();
    for (auto child : list) {

        SlangDeclKind kind = child->getKind();

        // there can be only 'Struct'
        if (kind != SlangDeclKind::Struct) continue;

        shader::ReflectedStructureLayout& material_layout = shader_file_data.structure_layouts.emplace_back();

        slang::TypeReflection*       type        = child->getType();
        slang::TypeLayoutReflection* type_layout = layout->getTypeLayout(type);

        material_layout.name = type_layout->getName();
        material_layout.size = type_layout->getSize();

        unsigned int field_count = type_layout->getFieldCount();
        material_layout.members.reserve(field_count);
        for (unsigned int i = 0; i < field_count; i++) {
            slang::VariableLayoutReflection* variable_layout = type_layout->getFieldByIndex(i);
            auto&                            member          = material_layout.members.emplace_back();

            SlangParser::parseMemberRecursive(variable_layout, static_cast<shader::ReflectedDataNode*>(&member));
        }
    }

    return shader_file_data;
}

FORR_NODISCARD Slang::ComPtr<slang::IComponentType> fe::SlangParser::specialize(const graphics::PipelineDesc&        pipeline_desc,
                                                                                slang::IComponentType*               composed_program,
                                                                                std::vector<slang::IComponentType*>& component_types) {
    // return if there is nothing to specialize
    if (!pipeline_desc.specialization.has_value()) return nullptr;

    if (!composed_program) {
        fe::logging::error("Failed to specialize global parameters. composed_program was nullptr");
        return nullptr;
    }

    const auto& specialization = pipeline_desc.specialization.value();

    uint32_t slang_specialization_parameters_count = composed_program->getSpecializationParamCount();

    // check if Slang and Unified match each other
    if (slang_specialization_parameters_count != specialization.global_arguments.size()) {
        fe::logging::error("Failed to specialize global parameters. Slang specialization parameters count was %i, while there was %i argument(s)",
                           slang_specialization_parameters_count,
                           specialization.global_arguments.size());
        return nullptr;
    }

    std::vector<slang::SpecializationArg> specialization_arguments{};
    specialization_arguments.resize(specialization.global_arguments.size(), {});

    // this is needed because 'const char*' in 'slang::SpecializationArg::expr' must live till
    // calling the 'slang::IComponentType::specialize()' function
    std::vector<std::string> string_pool{};

    // collect specialization arguments
    string_pool.reserve(specialization.global_arguments.size());
    for (size_t i = 0; i < specialization.global_arguments.size(); i++) {
        const auto& argument       = specialization.global_arguments[i];
        auto&       slang_argument = specialization_arguments[i];

        if (std::holds_alternative<fe::hashed_string>(argument.value)) {
            fe::hashed_string type_name = std::get<fe::hashed_string>(argument.value);

            for (slang::IComponentType* component_type : component_types) {
                slang::ProgramLayout*  module_layout   = component_type->getLayout();
                slang::TypeReflection* type_reflection = module_layout->findTypeByName(type_name.c_str());

                if (type_reflection) {
                    slang_argument = slang::SpecializationArg::fromType(type_reflection);
                }
            }

            if (!slang_argument.type) {
                fe::logging::error("Failed to find %s by name for specialization of global parameters",
                                   type_name.c_str());
                continue;
            }
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

    // specialize the composed program
    Slang::ComPtr<slang::IComponentType> specialized_program{};
    Slang::ComPtr<slang::IBlob>          specialization_diagnostics{};

    SlangResult result = composed_program->specialize(specialization_arguments.data(),
                                                      specialization_arguments.size(),
                                                      specialized_program.writeRef(),
                                                      specialization_diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        fe::logging::error("Failed to specialize global parameters\n%s",
                           (const char*) specialization_diagnostics->getBufferPointer());
        return nullptr;
    }

    return specialized_program;
}

void fe::SlangParser::parseVariableRecursive(slang::VariableLayoutReflection*             variable_layout,
                                             std::vector<shader::ReflectedDescriptor>&    descriptor_layouts,
                                             std::vector<shader::ReflectedPushConstants>& push_constants_layouts) {
    if (!variable_layout) {
        fe::logging::error("fe::SlangParser::parseVariableRecursive() : variable_layout was nullptr");
        return;
    }

    SlangCategory category = variable_layout->getCategory();

    switch (category) {
        case SlangCategory::DescriptorTableSlot: {
            auto& descriptor_layout = descriptor_layouts.emplace_back();
            SlangParser::parseDescriptorTable(variable_layout, descriptor_layout);
        } break;

        case SlangCategory::PushConstantBuffer: {
            auto& push_constants = push_constants_layouts.emplace_back();
            SlangParser::parsePushConstants(variable_layout, push_constants);
        } break;

            //case SlangCategory::GenericResource: {
            //    auto& descriptor_layout = descriptor_layouts.emplace_back();

            //    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();

            //    const char* type_name_raw = type_layout->getName();
            //    const char* name_raw      = variable_layout->getName();

            //    descriptor_layout.name            = name_raw ? name_raw : (type_name_raw ? type_name_raw : m_UnknownVariableName);
            //    descriptor_layout.descriptor_type = ShaderDescriptor::GENERIC;

            //    descriptor_layout.stage_flags |= static_cast<uint32_t>(ShaderType::VERTEX);
            //    descriptor_layout.stage_flags |= static_cast<uint32_t>(ShaderType::FRAGMENT);
            //    descriptor_layout.stage_flags |= static_cast<uint32_t>(ShaderType::COMPUTE);
            //    descriptor_layout.stage_flags |= static_cast<uint32_t>(ShaderType::GEOMETRY);
            //} break;

        case SlangCategory::SubElementRegisterSpace: {
            // TODO : rewrite this if needed

            auto                             type_layout             = variable_layout->getTypeLayout();
            slang::VariableLayoutReflection* element_variable_layout = type_layout->getElementVarLayout();
            slang::TypeLayoutReflection*     element_type_layout     = type_layout->getElementTypeLayout();

            auto& descriptor_layout = descriptor_layouts.emplace_back();

            const char* name_raw = variable_layout->getName();

            descriptor_layout.name            = name_raw ? name_raw : m_UnknownVariableName;
            descriptor_layout.descriptor_type = ShaderDescriptor::GENERIC;

            descriptor_layout.stage_flags |= static_cast<uint32_t>(ShaderType::VERTEX);
            descriptor_layout.stage_flags |= static_cast<uint32_t>(ShaderType::FRAGMENT);
            descriptor_layout.stage_flags |= static_cast<uint32_t>(ShaderType::COMPUTE);
            descriptor_layout.stage_flags |= static_cast<uint32_t>(ShaderType::GEOMETRY);

            //SlangParser::parseVariableRecursive(element_variable_layout, descriptor_layouts, push_constants_layouts);
        } break;

        default: {
            fe::logging::error("Slang -> Unified. Failed to reflect a variable\nUnknown slang::ParameterCategory : %i",
                               category);
        }
    }
}

void fe::SlangParser::parseDescriptorTable(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDescriptor& dst_descriptor) {
    if (!variable_layout) {
        fe::logging::error("fe::SlangParser::parseDescriptorTable() : variable_layout was nullptr");
        return;
    }

    if (variable_layout->getCategory() != SlangCategory::DescriptorTableSlot) {
        fe::logging::error("fe::SlangParser::parseDescriptorTable() : variable_layout->getCategory() wasn't SlangCategory::DescriptorTableSlot\nvariable_layout->getCategory() : %i",
                           static_cast<uint32_t>(variable_layout->getCategory()));
        return;
    }

    dst_descriptor.binding     = variable_layout->getOffset(slang::ParameterCategory::DescriptorTableSlot);
    dst_descriptor.set         = variable_layout->getBindingSpace(slang::ParameterCategory::DescriptorTableSlot);
    dst_descriptor.array_size  = 1;
    dst_descriptor.stage_flags = static_cast<uint32_t>(ShaderType::NONE);

    dst_descriptor.stage_flags |= static_cast<uint32_t>(ShaderType::VERTEX);
    dst_descriptor.stage_flags |= static_cast<uint32_t>(ShaderType::FRAGMENT);
    dst_descriptor.stage_flags |= static_cast<uint32_t>(ShaderType::COMPUTE);
    dst_descriptor.stage_flags |= static_cast<uint32_t>(ShaderType::GEOMETRY);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    SlangKind                    kind        = type_layout->getKind();

    switch (kind) {
        case SlangKind::ConstantBuffer:
        case SlangKind::ParameterBlock:
            dst_descriptor.descriptor_type = ShaderDescriptor::UNIFORM_BUFFER;
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
            break;

        case SlangKind::ShaderStorageBuffer:
            dst_descriptor.descriptor_type = ShaderDescriptor::STORAGE_BUFFER;
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
            break;

        case SlangKind::Array: {
            // if you got 'SlangKind::Array' here that means that this is a bindless parameter
            dst_descriptor.is_bindless = true;
            // when you pass 'slang::TypeLayoutReflection*' into 'fe::SlangParser::parseMemberRecursive()' instead of 'slang::VariableLayoutReflection*'
            //  you have to set 'array_size' yourself
            dst_descriptor.array_size = type_layout->getElementCount();

            slang::TypeLayoutReflection* array_element_type_layout = type_layout->getElementTypeLayout();
            SlangKind                    element_kind              = array_element_type_layout->getKind();

            if (element_kind == SlangKind::Resource) {
                SlangResourceShape shape      = array_element_type_layout->getResourceShape();
                unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

                if (shape_base >= SLANG_TEXTURE_1D && shape_base <= SLANG_TEXTURE_CUBE) {
                    bool is_read_write             = array_element_type_layout->getResourceAccess() == SLANG_RESOURCE_ACCESS_READ_WRITE;
                    dst_descriptor.descriptor_type = is_read_write ? ShaderDescriptor::STORAGE_IMAGE : ShaderDescriptor::COMBINED_IMAGE_SAMPLER;
                }
            }
            else {
                dst_descriptor.descriptor_type = ShaderDescriptor::UNIFORM_BUFFER;
            }

            if (element_kind != SlangKind::Resource &&
                element_kind != SlangKind::SamplerState) {
                SlangParser::parseMemberRecursive(array_element_type_layout, static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
            }
        } break;

        case SlangKind::Resource: {
            // when you pass 'slang::TypeLayoutReflection*' into 'fe::SlangParser::parseMemberRecursive()' instead of 'slang::VariableLayoutReflection*'
            //  you have to set 'array_size' yourself
            dst_descriptor.array_size = type_layout->getElementCount();

            SlangResourceShape shape      = type_layout->getResourceShape();
            unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

            SlangResourceAccess resource_access = type_layout->getResourceAccess();

            if (shape_base >= SLANG_TEXTURE_1D &&
                shape_base <= SLANG_TEXTURE_CUBE) {

                if (resource_access == SLANG_RESOURCE_ACCESS_READ_WRITE) {
                    dst_descriptor.descriptor_type = ShaderDescriptor::STORAGE_IMAGE;
                }
                else {
                    dst_descriptor.descriptor_type = ShaderDescriptor::COMBINED_IMAGE_SAMPLER;
                }
            }
            else if (shape_base == SLANG_STRUCTURED_BUFFER ||
                     shape_base == SLANG_BYTE_ADDRESS_BUFFER) {

                dst_descriptor.descriptor_type = ShaderDescriptor::STORAGE_BUFFER;
            }
            else if (shape_base == SLANG_ACCELERATION_STRUCTURE) {
                dst_descriptor.descriptor_type = ShaderDescriptor::ACCELERATION_STRUCTURE;
            }
            else {
                fe::logging::error("fe::SlangParser::parseDescriptorTable() : Unknown shape_base : %i",
                                   static_cast<uint32_t>(shape_base));
                return;
            }

            slang::TypeLayoutReflection* element_type = type_layout->getElementTypeLayout();
            if (element_type != nullptr) {
                SlangParser::parseMemberRecursive(element_type, static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
            }
        } break;

        case SlangKind::SamplerState:
            dst_descriptor.descriptor_type = ShaderDescriptor::SAMPLER;
            dst_descriptor.size            = 0;
            break;

        case SlangKind::Struct: {
            dst_descriptor.descriptor_type = ShaderDescriptor::STORAGE_BUFFER;
            dst_descriptor.size            = type_layout->getSize();
            dst_descriptor.array_size      = type_layout->getElementCount();
            SlangParser::parseMemberRecursive(type_layout, static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
            break;
        }

        default:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\nUnknown descriptor resource kind : %i", kind);
            dst_descriptor.descriptor_type = ShaderDescriptor::UNKNOWN;
            break;
    }

    // here we have to override the name, because 'fe::SlangParser::parseMemberRecursive()' setting 'dst_descriptor.name' as its
    //  struct's name, but I want to see its actual name.
    // For example :
    // ```slang
    // [[vk::binding(0, 0)]] ConstantBuffer<SceneData> global_data;
    // ```
    // 'fe::SlangParser::parseMemberRecursive()' will give us "SceneData", but I want to see name "global_data",
    //  that's why we set the name here
    const char* name = variable_layout->getName();
    if (name)
        dst_descriptor.name = name;
}

void fe::SlangParser::parsePushConstants(slang::VariableLayoutReflection* variable_layout, shader::ReflectedPushConstants& dst_push_constants) {
    if (!variable_layout) {
        fe::logging::error("fe::SlangParser::parsePushConstants() : variable_layout was nullptr");
        return;
    }

    if (variable_layout->getCategory() != SlangCategory::PushConstantBuffer) {
        fe::logging::error("fe::SlangParser::parsePushConstants() : variable_layout->getCategory() wasn't SlangCategory::PushConstantBuffer\nvariable_layout->getCategory() : %i",
                           static_cast<uint32_t>(variable_layout->getCategory()));
        return;
    }

    dst_push_constants.array_size  = 1;
    dst_push_constants.stage_flags = static_cast<uint32_t>(ShaderType::NONE);

    dst_push_constants.stage_flags |= static_cast<uint8_t>(ShaderType::VERTEX);
    dst_push_constants.stage_flags |= static_cast<uint8_t>(ShaderType::FRAGMENT);
    dst_push_constants.stage_flags |= static_cast<uint8_t>(ShaderType::COMPUTE);
    dst_push_constants.stage_flags |= static_cast<uint8_t>(ShaderType::GEOMETRY);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    SlangKind                    kind        = type_layout->getKind();

    switch (kind) {
        case SlangKind::ConstantBuffer:
        case SlangKind::ParameterBlock:
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_push_constants));
            break;

        case SlangKind::ShaderStorageBuffer:
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_push_constants));
            break;

        case SlangKind::Array: {
            dst_push_constants.array_size = type_layout->getElementCount();
            SlangParser::parseMemberRecursive(type_layout->getElementTypeLayout(), static_cast<shader::ReflectedDataNode*>(&dst_push_constants));
        } break;

        case SlangKind::Resource: {
            dst_push_constants.array_size = type_layout->getElementCount();
            SlangParser::parseMemberRecursive(type_layout->getElementTypeLayout(), static_cast<shader::ReflectedDataNode*>(&dst_push_constants));
        } break;

        case SlangKind::SamplerState:
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_push_constants));
            break;

        default:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\nUnknown descriptor resource kind : %i", kind);
            break;
    }

    // here we have to override the name, because 'fe::SlangParser::parseMemberRecursive()' setting 'dst_descriptor.name' as its
    //  struct's name, but I want to see its actual name.
    // For example :
    // ```slang
    // [[vk::binding(0, 0)]] ConstantBuffer<SceneData> global_data;
    // ```
    // 'fe::SlangParser::parseMemberRecursive()' will give us "SceneData", but I want to see name "global_data",
    //  that's why we set the name here
    const char* name = variable_layout->getName();
    if (name)
        dst_push_constants.name = name;
}

void fe::SlangParser::parseMemberRecursive(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDataNode* dst_reflected_data_node) {
    if (!variable_layout) {
        fe::logging::error("fe::SlangParser::parseMemberRecursive() : variable_layout was nullptr");
        return;
    }

    if (!dst_reflected_data_node) {
        fe::logging::error("fe::SlangParser::parseMemberRecursive() : dst_reflected_data_node was nullptr");
        return;
    }

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();

    dst_reflected_data_node->name = variable_layout->getName() ? variable_layout->getName() : type_layout->getName();

    if (auto* member = dynamic_cast<shader::ReflectedMember*>(dst_reflected_data_node)) {
        member->offset = variable_layout->getOffset();
    }

    SlangParser::parseMemberRecursive(type_layout, dst_reflected_data_node);
}

void fe::SlangParser::parseMemberRecursive(slang::TypeLayoutReflection* type_layout, shader::ReflectedDataNode* dst_reflected_data_node) {
    if (!type_layout) {
        fe::logging::error("fe::SlangParser::parseMemberRecursive() : type_layout was nullptr");
        return;
    }

    if (!dst_reflected_data_node) {
        fe::logging::error("fe::SlangParser::parseMemberRecursive() : dst_reflected_data_node was nullptr");
        return;
    }

    dst_reflected_data_node->size = type_layout->getSize();

    auto parse_recursive = [&](slang::TypeLayoutReflection*          type_layout,
                               std::vector<shader::ReflectedMember>& dst_members) {
        uint32_t field_count = type_layout->getFieldCount();
        dst_members.reserve(field_count);
        for (uint32_t i = 0; i < field_count; i++) {
            auto& member = dst_members.emplace_back();
            parseMemberRecursive(type_layout->getFieldByIndex(i), static_cast<shader::ReflectedDataNode*>(&member));
        }
    };

    SlangKind kind = type_layout->getKind();

    switch (kind) {
        case SlangKind::Struct:
            dst_reflected_data_node->type = ShaderValue::STRUCT;
            parse_recursive(type_layout, dst_reflected_data_node->members);
            break;

        case SlangKind::Array: {
            dst_reflected_data_node->type = ShaderValue::STRUCT;

            slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
            dst_reflected_data_node->array_size              = type_layout->getElementCount();

            if (element_type_layout->getKind() == SlangKind::Struct) {
                parse_recursive(element_type_layout, dst_reflected_data_node->members);
            }
            else {
                SlangParser::mapScalar(element_type_layout, dst_reflected_data_node->type);
            }
        } break;

            // clang-format off
        case SlangKind::Matrix        : SlangParser::mapMatrix(type_layout, dst_reflected_data_node->type); break;
        case SlangKind::Vector        : SlangParser::mapVector(type_layout, dst_reflected_data_node->type); break;
        case SlangKind::Scalar        : SlangParser::mapScalar(type_layout, dst_reflected_data_node->type); break;
        
        case SlangKind::SamplerState  : dst_reflected_data_node->type = ShaderValue::UINT64     ; break;
        case SlangKind::Pointer       : dst_reflected_data_node->type = ShaderValue::UINT_PTR   ; break;
            // clang-format on

        case SlangKind::Resource: {
            SlangResourceShape shape      = type_layout->getResourceShape();
            unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

            if (shape_base >= SLANG_TEXTURE_1D &&
                shape_base <= SLANG_TEXTURE_CUBE) {

                dst_reflected_data_node->type = ShaderValue::UINT32;
            }
            else if (shape_base == SLANG_STRUCTURED_BUFFER ||
                     shape_base == SLANG_BYTE_ADDRESS_BUFFER) {

                dst_reflected_data_node->type = ShaderValue::UINT_PTR;
            }
        } break;

        case SlangKind::Enum:
            SlangParser::mapScalar(type_layout, dst_reflected_data_node->type);

            if (dst_reflected_data_node->type == ShaderValue::UNKNOWN)
                dst_reflected_data_node->type = ShaderValue::INT32;
            break;

        default:
            fe::logging::warning("Slang -> Unified. Unhandled member kind %i for '%s'. Setting as UNKNOWN",
                                 kind,
                                 dst_reflected_data_node->name.c_str());
            dst_reflected_data_node->type = ShaderValue::UNKNOWN;
            break;
    }
}

void fe::SlangParser::mapMatrix(slang::TypeLayoutReflection* type_layout, shader::ValueType& type) {
    if (!type_layout) {
        fe::logging::error("fe::SlangParser::mapMatrix() : type_layout was nullptr");
        return;
    }

    uint32_t rows = type_layout->getRowCount();
    uint32_t cols = type_layout->getColumnCount();

    if (rows == 4 && cols == 4)
        type = ShaderValue::MAT4;
    else if (rows == 3 && cols == 3)
        type = ShaderValue::MAT3;
    else
        assert(false);
}

void fe::SlangParser::mapVector(slang::TypeLayoutReflection* type_layout, shader::ValueType& type) {
    if (!type_layout) {
        fe::logging::error("fe::SlangParser::mapVector() : type_layout was nullptr");
        return;
    }

    SlangScalar scalar          = type_layout->getScalarType();
    uint32_t    component_count = type_layout->getElementCount();

    switch (scalar) {
        case SlangScalar::Float32:
            if (component_count == 2) type = ShaderValue::FLOAT2;
            if (component_count == 3) type = ShaderValue::FLOAT3;
            if (component_count == 4) type = ShaderValue::FLOAT4;
            break;
        case SlangScalar::Int32:
            if (component_count == 2) type = ShaderValue::INT2;
            if (component_count == 3) type = ShaderValue::INT3;
            if (component_count == 4) type = ShaderValue::INT4;
            break;
        case SlangScalar::UInt32:
            if (component_count == 2) type = ShaderValue::UINT2;
            if (component_count == 3) type = ShaderValue::UINT3;
            if (component_count == 4) type = ShaderValue::UINT4;
            break;
        default:
            assert(false);
            break;
    }
}

void fe::SlangParser::mapScalar(slang::TypeLayoutReflection* type_layout, shader::ValueType& type) {
    if (!type_layout) {
        fe::logging::error("fe::SlangParser::mapScalar() : type_layout was nullptr");
        return;
    }

    SlangScalar scalar = type_layout->getScalarType();

    // clang-format off
    switch (scalar) {
        case SlangScalar::Void     : type = ShaderValue::VOID     ; break;
        case SlangScalar::Bool     : type = ShaderValue::BOOL     ; break;
        case SlangScalar::Int32    : type = ShaderValue::INT32    ; break;
        case SlangScalar::UInt32   : type = ShaderValue::UINT32   ; break;
        case SlangScalar::Int64    : type = ShaderValue::INT64    ; break;
        case SlangScalar::UInt64   : type = ShaderValue::UINT64   ; break;
        case SlangScalar::Float16  : type = ShaderValue::FLOAT16  ; break;
        case SlangScalar::Float32  : type = ShaderValue::FLOAT32  ; break;
        case SlangScalar::Float64  : type = ShaderValue::FLOAT64  ; break;
        case SlangScalar::Int8     : type = ShaderValue::INT8     ; break;
        case SlangScalar::UInt8    : type = ShaderValue::UINT8    ; break;
        case SlangScalar::Int16    : type = ShaderValue::INT16    ; break;
        case SlangScalar::UInt16   : type = ShaderValue::UINT16   ; break;
        case SlangScalar::IntPtr   : type = ShaderValue::INT_PTR  ; break;
        case SlangScalar::UIntPtr  : type = ShaderValue::UINT_PTR ; break;
        //case SlangScalar::BFloat16 : type = ShaderValue::BFLOAT16 ; break;
        //case SlangScalar::FloatE4M3: type = ShaderValue::FLOATE4M3; break;
        //case SlangScalar::FloatE5M2: type = ShaderValue::FLOATE5M2; break;
        case SlangScalar::None:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\ngot slang::TypeReflection::ScalarType::None.\nSetting type as fe::shader::ValueType::UNKNOWN");
            type = ShaderValue::UNKNOWN;
            break;
        default:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\ngot unknown slang::TypeReflection::ScalarType.\nSetting type as fe::shader::ValueType::UNKNOWN");
            type = ShaderValue::UNKNOWN;
            break;
    }
    // clang-format on
}
