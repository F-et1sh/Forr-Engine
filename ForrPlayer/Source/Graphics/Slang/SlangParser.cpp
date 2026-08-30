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

    if (full_search_paths.empty()) {
        std::array<const char*, 1> paths{ PATH.getShadersPath().generic_string().c_str() };

        session_desc.searchPathCount = paths.size();
        session_desc.searchPaths     = paths.data();
    }
    else {
        session_desc.searchPathCount = full_search_paths.size();
        session_desc.searchPaths     = full_search_paths.data();
    }

    if (SLANG_FAILED(global_session->createSession(session_desc, m_Session.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to create a session");
        return;
    }
}

bool fe::SlangParser::LoadFromFile(const std::filesystem::path& resource_full_path) {
    Slang::ComPtr<slang::IBlob> load_diagnostics{};
    m_Module = m_Session->loadModule(resource_full_path.generic_string().c_str(), load_diagnostics.writeRef());
    if (!m_Module) {
        fe::logging::error("Slang -> Unified. Failed to load a slang module\n%s",
                           (const char*) load_diagnostics->getBufferPointer());
        return false;
    }

    return true;
}

bool fe::SlangParser::ExtractSerializedData(std::vector<uint8_t>& dst_vector) {
    Slang::ComPtr<ISlangBlob> serialized_blob{};
    if (SLANG_FAILED(m_Module->serialize(serialized_blob.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to get serialized data from Slang");
        return false;
    }

    const uint8_t* buffer_data = (const uint8_t*) serialized_blob->getBufferPointer();
    size_t         buffer_size = serialized_blob->getBufferSize();

    // we store this data to compile the shader later
    dst_vector.assign(buffer_data, buffer_data + buffer_size);

    return true;
}

bool fe::SlangParser::ComposeProgram(GraphicsBackend graphics_backend, bool do_all) {
    // there is no need to search for entry points here
    std::vector<slang::IComponentType*> component_types{};
    if (do_all) component_types.emplace_back(m_Module);

    uint32_t dependency_count = m_Module->getDependencyFileCount();
    component_types.reserve(dependency_count);
    for (uint32_t i = static_cast<uint32_t>(!do_all); i < dependency_count; i++) { // start from '1', because 'm_Module' is already added
        const char* dependency_file = m_Module->getDependencyFilePath(i);

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
                                                                                 m_ComposedProgram.writeRef(),
                                                                                 composition_diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        fe::logging::error("Slang -> Unified. Failed to create a composed program\n%s",
                           (const char*) composition_diagnostics->getBufferPointer());
        return false;
    }

    std::expected<Slang::ComPtr<slang::IComponentType>, fe::SlangParser::SpecializationErrors> expected_result =
        this->specializeGraphicsBackend(m_ComposedProgram.get(), graphics_backend);

    //if (expected_result.has_value()) {
    //    Slang::ComPtr<slang::IComponentType> specialized_program = std::move(expected_result.value());
    //    m_ComposedProgram.swap(specialized_program);
    //}

    return true;
}

std::expected<Slang::ComPtr<slang::IComponentType>, fe::SlangParser::SpecializationErrors>
fe::SlangParser::specializeGraphicsBackend(slang::IComponentType* component_type,
                                           GraphicsBackend        graphics_backend) {

    int unspecialized_parameter_count = component_type->getSpecializationParamCount();
    if (unspecialized_parameter_count == 0) {
        return std::unexpected{ SlangParser::SpecializationErrors::NO_PARAMETERS }; // there is nothing to specialize
    }

    slang::ProgramLayout*                 layout = component_type->getLayout();
    std::vector<slang::SpecializationArg> specialization_args{};

#define SPECIALIZATION_ARGUMENT_INSTANCE(TYPE_NAME)                      \
    {                                                                    \
        slang::TypeReflection* type = layout->findTypeByName(TYPE_NAME); \
        if (type) {                                                      \
            auto& argument = specialization_args.emplace_back();         \
            argument.kind  = slang::SpecializationArg::Kind::Type;       \
            argument.type  = type;                                       \
        }                                                                \
    }

    switch (graphics_backend) {
        case GraphicsBackend::OpenGL: {
            SPECIALIZATION_ARGUMENT_INSTANCE("OpenGLBuffer");
        } break;

        case GraphicsBackend::Vulkan: {
            SPECIALIZATION_ARGUMENT_INSTANCE("VulkanBuffer");
        } break;

        default:
            fe::logging::error("Slang -> Unified. Failed to specialize a program. Unsupported graphics backend %i",
                               graphics_backend);
            return std::unexpected{ SlangParser::SpecializationErrors::UNSUPPORTED_GRAPHICS_BACKEND };
    }

#undef SPECIALIZATION_ARGUMENT_INSTANCE

    Slang::ComPtr<slang::IComponentType> specialized_program{};
    Slang::ComPtr<slang::IBlob>          diagnostics{};
    SlangResult                          result = component_type->specialize(specialization_args.data(),
                                                                             specialization_args.size(),
                                                                             specialized_program.writeRef(),
                                                                             diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        fe::logging::error("Slang -> Unified. Failed to specialize a program\n%s",
                           (const char*) diagnostics->getBufferPointer());
        return std::unexpected{ SlangParser::SpecializationErrors::SPECIALIZATION_FAILED };
    }

    return specialized_program;
}

bool fe::SlangParser::parseDescriptorRecursive(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDescriptorsLayout& descriptors_layout) {
    bool result = false;

    SlangCategory category = variable_layout->getCategory();

    switch (category) {
        case SlangCategory::DescriptorTableSlot: {
            auto& parameter = descriptors_layout.descriptors.emplace_back();
            SlangParser::parseDescriptorTable(variable_layout, parameter);
            result = true;
        } break;

        case SlangCategory::PushConstantBuffer: {
            SlangParser::parsePushConstant(variable_layout, descriptors_layout.push_constants);
            result = true;
        } break;

        case SlangCategory::SubElementRegisterSpace: {
            auto                             type_layout             = variable_layout->getTypeLayout();
            slang::VariableLayoutReflection* element_variable_layout = type_layout->getElementVarLayout();

            SlangCategory element_variable_category = element_variable_layout->getCategory();

            const char*      name_raw = variable_layout->getName();
            std::string_view name     = name_raw ? name_raw : "No name";

            switch (element_variable_category) {
                case SlangCategory::GenericResource: {
                    auto& parameter = descriptors_layout.descriptors.emplace_back();

                    parameter.name            = name;
                    parameter.descriptor_type = ShaderDescriptor::GENERIC;
                } break;

                case SlangCategory::DescriptorTableSlot: {
                    auto& parameter = descriptors_layout.descriptors.emplace_back();
                    SlangParser::parseDescriptorTable(element_variable_layout, parameter);
                    parameter.name = name;
                } break;

                case SlangCategory::PushConstantBuffer: {
                    SlangParser::parsePushConstant(element_variable_layout, descriptors_layout.push_constants);
                    descriptors_layout.push_constants.name = name;
                } break;

                default: {
                    fe::logging::error("Slang -> Unified. Unsupported slang::ParameterCategory %i while reflecting a slang::ParameterCategory::SubElementRegisterSpace",
                                       category);
                    return result;
                }
            }

            result = true;

        } break;

        default: {
            fe::logging::error("Slang -> Unified. Failed to reflect a variable\nUnknown slang::ParameterCategory : %i",
                               category);
        }
    }

    return result;
}

bool fe::SlangParser::ReflectDescriptors(shader::ReflectedDescriptorsLayout& descriptors_layout) {
    bool result = false;

    slang::ProgramLayout* layout          = m_ComposedProgram->getLayout();
    unsigned int          parameter_count = layout->getParameterCount();

    if (parameter_count != 0) {
        descriptors_layout.descriptors.reserve(parameter_count);

        for (unsigned int i = 0; i < parameter_count; i++) {
            slang::VariableLayoutReflection* variable_layout = layout->getParameterByIndex(i);
            if (!variable_layout) {
                fe::logging::error("Slang -> Unified. Failed to reflect a variable\nslang::VariableLayoutReflection* variable_layout = context.root_layout->getParameterByIndex(i) was nullptr. i = %i",
                                   i);
                continue;
            }

            // if this return 'true', 'result' will turn into 'true', otherwise - won't change its value
            result |= this->parseDescriptorRecursive(variable_layout, descriptors_layout);
        }
    }

    return result;
}

bool fe::SlangParser::IsPipeline() {
    for (uint32_t i = 0; i < std::size(ENTRY_POINT_NAMES); i++) {
        auto                entry_point_name = ENTRY_POINT_NAMES[i];
        slang::IEntryPoint* entry_point{};
        m_Module->findEntryPointByName(entry_point_name.data(), &entry_point);

        // a shader can have zero descriptors, but if there is at least one entry point - it is a pipeline
        if (entry_point)
            return true;
    }

    return false;
}

bool fe::SlangParser::ReflectMaterials(std::unordered_map<fe::hashed_string, shader::ReflectedStructureLayout>& material_layouts) {
    bool result = false;

    slang::DeclReflection* module_reflection = m_Module->getModuleReflection();
    slang::ProgramLayout*  layout            = m_ComposedProgram->getLayout();

    // unwrap first module - it always exists, even if the file is empty
    auto list = module_reflection->getChildren();
    for (auto child : list) {

        SlangDeclKind kind = child->getKind();

        // there can be only 'Struct'
        if (kind != SlangDeclKind::Struct) continue;

        // TODO : check that material is devired from 'IMaterial'

        shader::ReflectedStructureLayout material_layout{};

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

        material_layouts.emplace(material_layout.name, material_layout);

        result = true;
    }

    return result;
}

fe::shader::SourceCodeStorage fe::SlangParser::CombineAndCompileShader(const resource::ShaderProgram& shader_program,
                                                                       const resource::Material&      material,
                                                                       ResourceManager&               resource_manager) {
    // this function doesn't use 'm_Module', use local modules instead

    shader::SourceCodeStorage           source_codes{};
    std::vector<slang::IComponentType*> component_types{};

    // handle material

    const auto&                   material_layout_resource = *resource_manager.GetResource(material.layout_ptr);
    Slang::ComPtr<slang::IModule> material_module          = this->deserializeModule(material_layout_resource.shader_file_data_ptr, resource_manager);

    m_Module.attach(material_module.detach());

    this->ComposeProgram(resource_manager.GetContext().graphics_backend, true); // TODO : pass current graphics backend through the arguments

    Slang::ComPtr<slang::IComponentType> material_component_type{};
    material_component_type.attach(m_ComposedProgram.detach());

    component_types.emplace_back(material_component_type.get());

    // handle shader

    Slang::ComPtr<slang::IModule> shader_module = this->deserializeModule(shader_program.shader_file_data_ptr, resource_manager);

    m_Module.attach(shader_module.detach());

    this->ComposeProgram(resource_manager.GetContext().graphics_backend, true); // TODO : pass current graphics backend through the arguments

    Slang::ComPtr<slang::IComponentType> shader_component_type{};
    shader_component_type.attach(m_ComposedProgram.detach());
    shader_module.attach(m_Module.detach());

    //component_types.emplace_back(shader_component_type.get()); // already added by material, because they are the same file

    // find entry points

    slang::ProgramLayout*  material_layout = material_component_type->getLayout();
    slang::TypeReflection* material_type   = material_layout->findTypeByName(material_layout_resource.reflected_layout.name.c_str());

    struct EntryPoint {
        Slang::ComPtr<slang::IComponentType> entry_point{};
        ShaderType                           shader_type{};

        EntryPoint() = default;
        EntryPoint(slang::IComponentType* entry_point, ShaderType shader_type)
            : entry_point(entry_point), shader_type(shader_type) {}

        FORR_CLASS_MOVABLE(EntryPoint)
        FORR_CLASS_NONCOPYABLE(EntryPoint)
    };

    std::vector<EntryPoint> entry_points{};

    for (std::uint32_t i = 0; i < std::size(SlangParser::ENTRY_POINT_NAMES); i++) {
        std::string_view entry_point_name = SlangParser::ENTRY_POINT_NAMES[i];

        Slang::ComPtr<slang::IEntryPoint> entry_point{};
        SlangResult                       result = shader_module->findEntryPointByName(entry_point_name.data(), entry_point.writeRef()); // find this entry point in shader's module

        if (SLANG_FAILED(result)) continue;

        ShaderType shader_type{};

        if (entry_point_name == SlangParser::ENTRY_POINT_NAMES[0]) { // vertex
            shader_type = ShaderType::VERTEX;
        }
        else if (entry_point_name == SlangParser::ENTRY_POINT_NAMES[1]) { // fragment
            shader_type = ShaderType::FRAGMENT;
        }
        else if (entry_point_name == SlangParser::ENTRY_POINT_NAMES[2]) { // compute
            shader_type = ShaderType::COMPUTE;
        }

        if (entry_point->getSpecializationParamCount() > 0) { // specialize only if needed

            std::array<slang::SpecializationArg, 1> specialization_args{};
            specialization_args[0].kind = slang::SpecializationArg::Kind::Type;
            specialization_args[0].type = material_type;

            slang::IComponentType*      component_type_raw{};
            Slang::ComPtr<slang::IBlob> diagnostics{};

            int parameters_count = entry_point->getSpecializationParamCount();

            SlangResult result = entry_point->specialize(specialization_args.data(),
                                                         specialization_args.size(),
                                                         &component_type_raw,
                                                         diagnostics.writeRef());
            if (SLANG_FAILED(result)) {
                fe::logging::error("Serialized Slang ( Unified ) -> Slang. Failed to specialize an entry point. Continuing collecting entry points.\n%s",
                                   (const char*) diagnostics->getBufferPointer());
                continue;
            }

            entry_points.emplace_back(EntryPoint{ component_type_raw, shader_type });
        }
        else {
            entry_points.emplace_back(EntryPoint{ entry_point, shader_type });
        }
    }

    for (EntryPoint& entry_point : entry_points) {
        component_types.emplace_back(entry_point.entry_point);
    }

    // create composite

    Slang::ComPtr<slang::IBlob> composition_diagnostics{};
    SlangResult                 composition_result = m_Session->createCompositeComponentType(component_types.data(),
                                                                                             component_types.size(),
                                                                                             m_ComposedProgram.writeRef(),
                                                                                             composition_diagnostics.writeRef());
    if (SLANG_FAILED(composition_result)) {
        fe::logging::error("Serialized Slang ( Unified ) -> Slang. Failed to create a composed program\n%s",
                           (const char*) composition_diagnostics->getBufferPointer());
        return {};
    }

    // extract source codes

    for (size_t i = 0; i < entry_points.size(); i++) {
        const EntryPoint& entry_point = entry_points[i];

        Slang::ComPtr<slang::IBlob> spirv_code{};

        Slang::ComPtr<slang::IBlob> entry_point_code_diagnostics{};
        SlangResult                 result = m_ComposedProgram->getEntryPointCode(i, 0, spirv_code.writeRef(), entry_point_code_diagnostics.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Serialized Slang ( Unified ) -> Slang. Failed to get an entry point code. Continuing compilation\nEntry point index : %i\n%s",
                               i,
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

void fe::SlangParser::parseDescriptorTable(slang::VariableLayoutReflection* variable_layout,
                                           shader::ReflectedDescriptor&     dst_descriptor) {
    assert(variable_layout);
    assert(variable_layout->getCategory() == SlangCategory::DescriptorTableSlot);

    dst_descriptor.binding     = variable_layout->getOffset(slang::ParameterCategory::DescriptorTableSlot);
    dst_descriptor.set         = variable_layout->getBindingSpace(slang::ParameterCategory::DescriptorTableSlot);
    dst_descriptor.array_size  = 1;
    dst_descriptor.stage_flags = static_cast<uint32_t>(ShaderType::NONE);

    // this doesn't work
    //SlangStage stage = variable_layout->getStage();
    //dst_descriptor.stage_flags =
    //    ((stage & SLANG_STAGE_VERTEX) ? static_cast<uint32_t>(ShaderType::VERTEX) : 0) |
    //    ((stage & SLANG_STAGE_FRAGMENT) ? static_cast<uint32_t>(ShaderType::FRAGMENT) : 0) |
    //    ((stage & SLANG_STAGE_COMPUTE) ? static_cast<uint32_t>(ShaderType::COMPUTE) : 0) |
    //    ((stage & SLANG_STAGE_GEOMETRY) ? static_cast<uint32_t>(ShaderType::GEOMETRY) : 0);

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
                assert(false);
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

void fe::SlangParser::parsePushConstant(slang::VariableLayoutReflection* variable_layout,
                                        shader::ReflectedPushConstants&  dst_push_constants) {
    assert(variable_layout);
    assert(variable_layout->getCategory() == SlangCategory::PushConstantBuffer);

    dst_push_constants.array_size  = 1;
    dst_push_constants.stage_flags = static_cast<uint32_t>(ShaderType::NONE);

    // this doesn't work
    //SlangStage stage = variable_layout->getStage();
    //dst_push_constants.stage_flags =
    //    ((stage & SLANG_STAGE_VERTEX) ? static_cast<uint32_t>(ShaderType::VERTEX) : 0) |
    //    ((stage & SLANG_STAGE_FRAGMENT) ? static_cast<uint32_t>(ShaderType::FRAGMENT) : 0) |
    //    ((stage & SLANG_STAGE_COMPUTE) ? static_cast<uint32_t>(ShaderType::COMPUTE) : 0) |
    //    ((stage & SLANG_STAGE_GEOMETRY) ? static_cast<uint32_t>(ShaderType::GEOMETRY) : 0);

    dst_push_constants.stage_flags |= static_cast<uint32_t>(ShaderType::VERTEX);
    dst_push_constants.stage_flags |= static_cast<uint32_t>(ShaderType::FRAGMENT);
    dst_push_constants.stage_flags |= static_cast<uint32_t>(ShaderType::COMPUTE);
    dst_push_constants.stage_flags |= static_cast<uint32_t>(ShaderType::GEOMETRY);

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

void fe::SlangParser::parseMemberRecursive(slang::VariableLayoutReflection* variable_layout,
                                           shader::ReflectedDataNode*       dst_reflected_data_node) {
    assert(variable_layout);
    assert(dst_reflected_data_node);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();

    dst_reflected_data_node->name = variable_layout->getName() ? variable_layout->getName() : type_layout->getName();

    shader::ReflectedMember* member = static_cast<shader::ReflectedMember*>(dst_reflected_data_node);
    member->offset                  = variable_layout->getOffset();

    SlangParser::parseMemberRecursive(type_layout, dst_reflected_data_node);
}

void fe::SlangParser::parseMemberRecursive(slang::TypeLayoutReflection* type_layout, shader::ReflectedDataNode* dst_reflected_data_node) {
    assert(type_layout);
    assert(dst_reflected_data_node);

    dst_reflected_data_node->size = type_layout->getSize();

    auto parse_recursive = [&](slang::TypeLayoutReflection*          type_layout,
                               std::vector<shader::ReflectedMember>& dst_members) {
        uint32_t field_count = type_layout->getFieldCount();
        dst_members.reserve(field_count);
        for (uint32_t i = 0; i < field_count; i++) {
            shader::ReflectedMember& member = dst_members.emplace_back();
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
    assert(type_layout);

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
    assert(type_layout);

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
    assert(type_layout);

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

Slang::ComPtr<slang::IModule> fe::SlangParser::deserializeModule(fe::pointer<resource::ShaderFileData> shader_file_data_ptr,
                                                                 ResourceManager&                      resource_manager) {
    const auto& file_data = *resource_manager.GetResource(shader_file_data_ptr);

    if (file_data.slang_serialized_data.empty() ||
        file_data.slang_serialized_data.data() == nullptr) {
        fe::logging::error("Serialized Slang ( Unified ) -> Slang. Failed to load material's slang module. Material's serialized data was empty");
        return nullptr;
    }

    Slang::ComPtr<ISlangBlob> blob{};
    ISlangBlob*               blob_raw = slang_createBlob(file_data.slang_serialized_data.data(),
                                                          file_data.slang_serialized_data.size());
    blob.attach(blob_raw);

    Slang::ComPtr<slang::IBlob>   load_diagnostics{};
    Slang::ComPtr<slang::IModule> loaded_module{};
    slang::IModule*               loaded_module_raw = m_Session->loadModuleFromIRBlob(file_data.full_path.c_str(),
                                                                                      file_data.full_path.c_str(),
                                                                                      blob_raw,
                                                                                      load_diagnostics.writeRef());

    if (!loaded_module_raw) {
        fe::logging::error("Serialized Slang ( Unified ) -> Slang. Failed to load Material's slang module\n%s",
                           (const char*) load_diagnostics->getBufferPointer());
        return nullptr;
    }

    loaded_module.attach(loaded_module_raw);

    return loaded_module;
}
