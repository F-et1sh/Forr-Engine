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
    using shader_descriptor = shader::DescriptorType;
    using shader_value      = shader::ValueType;
    using shader_type       = shader::StageBits;

    using slang_kind      = slang::TypeReflection::Kind;
    using slang_decl_kind = slang::DeclReflection::Kind;
    using slang_scalar    = slang::TypeReflection::ScalarType;
    using slang_category  = slang::ParameterCategory;
} // namespace fe

fe::SlangParser::SlangParser(GraphicsBackend graphics_backend) : m_GraphicsBackend(graphics_backend) {
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
    const char* search_paths = { (PATH.getDefaultShadersPath() / "PBRMaterial").generic_string().c_str() };

    session_desc.searchPathCount = 1;
    session_desc.searchPaths     = &search_paths;

    if (SLANG_FAILED(global_session->createSession(session_desc, m_Session.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to create a session");
        return;
    }
}

bool fe::SlangParser::LoadFromFile(const std::filesystem::path& resource_full_path) {
    Slang::ComPtr<slang::IBlob> load_diagnostics{};
    m_Module = m_Session->loadModule(resource_full_path.generic_string().c_str(), load_diagnostics.writeRef());
    if (!m_Module) {
        fe::logging::error("Slang -> Unified. Failed to load a slang module\n%s", (const char*) load_diagnostics->getBufferPointer());
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

bool fe::SlangParser::ComposeProgram() {
    // there is no need to search entry points here
    std::vector<slang::IComponentType*> component_types{};
    component_types.emplace_back(m_Module);

    Slang::ComPtr<slang::IBlob> composition_diagnostics{};
    SlangResult                 result = m_Session->createCompositeComponentType(component_types.data(),
                                                                                 component_types.size(),
                                                                                 m_CompusedProgram.writeRef(),
                                                                                 composition_diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        fe::logging::error("Slang -> Unified. Failed to create a composed program\n%s", (const char*) composition_diagnostics->getBufferPointer());
        return false;
    }

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

bool fe::SlangParser::ReflectPipeline(shader::ReflectedPipelineLayout& pipeline_layout) {
    bool result = false;

    slang::ProgramLayout* layout          = m_CompusedProgram->getLayout();
    unsigned int          parameter_count = layout->getParameterCount();

    if (parameter_count != 0) {
        pipeline_layout.descriptors.reserve(parameter_count);

        for (unsigned int i = 0; i < parameter_count; i++) {
            slang::VariableLayoutReflection* variable_layout = layout->getParameterByIndex(i);
            if (!variable_layout) {
                fe::logging::error("Slang -> Unified. Failed to reflect a variable\nslang::VariableLayoutReflection* variable_layout = context.root_layout->getParameterByIndex(i) was nullptr. i = %i", i);
                continue;
            }

            slang_category category = variable_layout->getCategory();

            if (category == slang_category::DescriptorTableSlot) {
                auto& parameter = pipeline_layout.descriptors.emplace_back();
                SlangParser::parseDescriptorTable(variable_layout, parameter);
                result = true;
            }
            else if (category == slang_category::PushConstantBuffer) {
                SlangParser::parsePushConstant(variable_layout, pipeline_layout.push_constants);
                result = true;
            }
            else {
                fe::logging::error("Slang -> Unified. Failed to reflect a variable\nUnknown slang::ParameterCategory : %i", category);
                continue;
            }
        }
    }

    return result;
}

bool fe::SlangParser::ReflectMaterials(std::unordered_map<std::string, shader::ReflectedMaterialLayout>& material_layouts) {
    bool result = false;

    slang::DeclReflection* module_reflection = m_Module->getModuleReflection();
    slang::ProgramLayout*  layout            = m_CompusedProgram->getLayout();

    // unwrap first module - it always exists, even if the file is empty
    auto list = module_reflection->getChildren();
    for (auto child : list) {

        slang_decl_kind kind = child->getKind();

        // there can be only 'Struct'
        if (kind != slang_decl_kind::Struct) continue;

        shader::ReflectedMaterialLayout material_layout{};

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

void fe::SlangParser::parseDescriptorTable(slang::VariableLayoutReflection* variable_layout,
                                           shader::ReflectedDescriptor&     dst_descriptor) {
    assert(variable_layout);
    assert(variable_layout->getCategory() == slang_category::DescriptorTableSlot);

    dst_descriptor.set         = variable_layout->getBindingSpace();
    dst_descriptor.binding     = variable_layout->getBindingIndex();
    dst_descriptor.array_size  = 1;
    dst_descriptor.stage_flags = static_cast<uint32_t>(shader_type::NONE);

    // this doesn't work
    //SlangStage stage = variable_layout->getStage();
    //dst_descriptor.stage_flags =
    //    ((stage & SLANG_STAGE_VERTEX) ? static_cast<uint32_t>(shader_type::VERTEX) : 0) |
    //    ((stage & SLANG_STAGE_FRAGMENT) ? static_cast<uint32_t>(shader_type::FRAGMENT) : 0) |
    //    ((stage & SLANG_STAGE_COMPUTE) ? static_cast<uint32_t>(shader_type::COMPUTE) : 0) |
    //    ((stage & SLANG_STAGE_GEOMETRY) ? static_cast<uint32_t>(shader_type::GEOMETRY) : 0);

    dst_descriptor.stage_flags |= static_cast<uint32_t>(shader_type::VERTEX);
    dst_descriptor.stage_flags |= static_cast<uint32_t>(shader_type::FRAGMENT);
    dst_descriptor.stage_flags |= static_cast<uint32_t>(shader_type::COMPUTE);
    dst_descriptor.stage_flags |= static_cast<uint32_t>(shader_type::GEOMETRY);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    slang_kind                  kind        = type_layout->getKind();

    // 24.06.2026 Slang has got a bug, so, now this will work like this
    //
    // TODO : update Slang submodule and remove this part
    //
    // 02.07.2026 Slang don't want to convert sets into OpenGL bindings, so, I have to hardcode this
    {
        //static uint32_t vulkan_set_counter     = 0;
        static uint32_t opengl_binding_counter = 0;

        if (dst_descriptor.name == "scene_set") {
            //vulkan_set_counter     = 0;
            opengl_binding_counter = 0;
        }

        if (m_GraphicsBackend == GraphicsBackend::OpenGL) {
            dst_descriptor.set     = 0;
            dst_descriptor.binding = opengl_binding_counter++;
        }
    }

    switch (kind) {
        case slang_kind::ConstantBuffer:
        case slang_kind::ParameterBlock:
            dst_descriptor.descriptor_type = shader_descriptor::UNIFORM_BUFFER;
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
            break;

        case slang_kind::ShaderStorageBuffer:
            dst_descriptor.descriptor_type = shader_descriptor::STORAGE_BUFFER;
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
            break;

        case slang_kind::Array: {
            // if you got 'slang_kind::Array' here that means that this is a bindless parameter
            dst_descriptor.is_bindless = true;
            // when you pass 'slang::TypeLayoutReflection*' into 'fe::SlangParser::parseMemberRecursive()' instead of 'slang::VariableLayoutReflection*'
            //  you have to set 'array_size' yourself
            dst_descriptor.array_size = type_layout->getElementCount();

            slang::TypeLayoutReflection* array_element_type_layout = type_layout->getElementTypeLayout();
            slang_kind                  element_kind              = array_element_type_layout->getKind();

            if (element_kind == slang_kind::Resource) {
                SlangResourceShape shape      = array_element_type_layout->getResourceShape();
                unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

                if (shape_base >= SLANG_TEXTURE_1D && shape_base <= SLANG_TEXTURE_CUBE) {
                    bool is_read_write             = array_element_type_layout->getResourceAccess() == SLANG_RESOURCE_ACCESS_READ_WRITE;
                    dst_descriptor.descriptor_type = is_read_write ? shader_descriptor::STORAGE_IMAGE : shader_descriptor::COMBINED_IMAGE_SAMPLER;
                }
            }
            else {
                dst_descriptor.descriptor_type = shader_descriptor::UNIFORM_BUFFER;
            }

            SlangParser::parseMemberRecursive(array_element_type_layout, static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
        } break;

        case slang_kind::Resource: {
            // when you pass 'slang::TypeLayoutReflection*' into 'fe::SlangParser::parseMemberRecursive()' instead of 'slang::VariableLayoutReflection*'
            //  you have to set 'array_size' yourself
            dst_descriptor.array_size = type_layout->getElementCount();

            SlangResourceShape shape      = type_layout->getResourceShape();
            unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

            SlangResourceAccess resource_access = type_layout->getResourceAccess();

            if (shape_base >= SLANG_TEXTURE_1D &&
                shape_base <= SLANG_TEXTURE_CUBE) {

                if (resource_access == SLANG_RESOURCE_ACCESS_READ_WRITE) {
                    dst_descriptor.descriptor_type = shader_descriptor::STORAGE_IMAGE;
                }
                else {
                    dst_descriptor.descriptor_type = shader_descriptor::COMBINED_IMAGE_SAMPLER;
                }
            }
            else if (shape_base == SLANG_STRUCTURED_BUFFER ||
                     shape_base == SLANG_BYTE_ADDRESS_BUFFER) {

                dst_descriptor.descriptor_type = shader_descriptor::STORAGE_BUFFER;
            }
            else if (shape_base == SLANG_ACCELERATION_STRUCTURE) {
                dst_descriptor.descriptor_type = shader_descriptor::ACCELERATION_STRUCTURE;
            }
            else {
                assert(false);
            }

            SlangParser::parseMemberRecursive(type_layout->getElementTypeLayout(), static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
        } break;

        case slang_kind::SamplerState:
            dst_descriptor.descriptor_type = shader_descriptor::SAMPLER;
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_descriptor));
            break;

        default:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\nUnknown descriptor resource kind : %i", kind);
            dst_descriptor.descriptor_type = shader_descriptor::UNKNOWN;
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
    dst_descriptor.name = variable_layout->getName();
}

void fe::SlangParser::parsePushConstant(slang::VariableLayoutReflection* variable_layout,
                                        shader::ReflectedPushConstants&  dst_push_constants) {
    assert(variable_layout);
    assert(variable_layout->getCategory() == slang_category::PushConstantBuffer);

    dst_push_constants.array_size  = 1;
    dst_push_constants.stage_flags = static_cast<uint32_t>(shader_type::NONE);

    // this doesn't work
    //SlangStage stage = variable_layout->getStage();
    //dst_push_constants.stage_flags =
    //    ((stage & SLANG_STAGE_VERTEX) ? static_cast<uint32_t>(shader_type::VERTEX) : 0) |
    //    ((stage & SLANG_STAGE_FRAGMENT) ? static_cast<uint32_t>(shader_type::FRAGMENT) : 0) |
    //    ((stage & SLANG_STAGE_COMPUTE) ? static_cast<uint32_t>(shader_type::COMPUTE) : 0) |
    //    ((stage & SLANG_STAGE_GEOMETRY) ? static_cast<uint32_t>(shader_type::GEOMETRY) : 0);

    dst_push_constants.stage_flags |= static_cast<uint32_t>(shader_type::VERTEX);
    dst_push_constants.stage_flags |= static_cast<uint32_t>(shader_type::FRAGMENT);
    dst_push_constants.stage_flags |= static_cast<uint32_t>(shader_type::COMPUTE);
    dst_push_constants.stage_flags |= static_cast<uint32_t>(shader_type::GEOMETRY);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    slang_kind                  kind        = type_layout->getKind();

    switch (kind) {
        case slang_kind::ConstantBuffer:
        case slang_kind::ParameterBlock:
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_push_constants));
            break;

        case slang_kind::ShaderStorageBuffer:
            SlangParser::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<shader::ReflectedDataNode*>(&dst_push_constants));
            break;

        case slang_kind::Array: {
            dst_push_constants.array_size = type_layout->getElementCount();
            SlangParser::parseMemberRecursive(type_layout->getElementTypeLayout(), static_cast<shader::ReflectedDataNode*>(&dst_push_constants));
        } break;

        case slang_kind::Resource: {
            dst_push_constants.array_size = type_layout->getElementCount();
            SlangParser::parseMemberRecursive(type_layout->getElementTypeLayout(), static_cast<shader::ReflectedDataNode*>(&dst_push_constants));
        } break;

        case slang_kind::SamplerState:
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
    dst_push_constants.name = variable_layout->getName();
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

    slang_kind kind = type_layout->getKind();

    switch (kind) {
        case slang_kind::Struct:
            dst_reflected_data_node->type = shader_value::STRUCT;
            parse_recursive(type_layout, dst_reflected_data_node->members);
            break;

        case slang_kind::Array: {
            dst_reflected_data_node->type = shader_value::STRUCT;

            slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
            dst_reflected_data_node->array_size              = type_layout->getElementCount();

            if (element_type_layout->getKind() == slang_kind::Struct) {
                parse_recursive(element_type_layout, dst_reflected_data_node->members);
            }
            else {
                SlangParser::mapScalar(element_type_layout, dst_reflected_data_node->type);
            }
        } break;

            // clang-format off
        case slang_kind::Matrix        : SlangParser::mapMatrix(type_layout, dst_reflected_data_node->type); break;
        case slang_kind::Vector        : SlangParser::mapVector(type_layout, dst_reflected_data_node->type); break;
        case slang_kind::Scalar        : SlangParser::mapScalar(type_layout, dst_reflected_data_node->type); break;
        
        case slang_kind::SamplerState  : dst_reflected_data_node->type = shader_value::UINT64     ; break;
        case slang_kind::Pointer       : dst_reflected_data_node->type = shader_value::UINT_PTR   ; break;
            // clang-format on

        case slang_kind::Resource: {
            SlangResourceShape shape      = type_layout->getResourceShape();
            unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

            if (shape_base >= SLANG_TEXTURE_1D &&
                shape_base <= SLANG_TEXTURE_CUBE) {

                dst_reflected_data_node->type = shader_value::UINT32;
            }
            else if (shape_base == SLANG_STRUCTURED_BUFFER ||
                     shape_base == SLANG_BYTE_ADDRESS_BUFFER) {

                dst_reflected_data_node->type = shader_value::UINT_PTR;
            }
        } break;

        case slang_kind::Enum:
            SlangParser::mapScalar(type_layout, dst_reflected_data_node->type);

            if (dst_reflected_data_node->type == shader_value::UNKNOWN)
                dst_reflected_data_node->type = shader_value::INT32;
            break;

        default:
            fe::logging::warning("Slang -> Unified. Unhandled member kind %i for '%s'. Setting as UNKNOWN", kind, dst_reflected_data_node->name.c_str());
            dst_reflected_data_node->type = shader_value::UNKNOWN;
            break;
    }
}

void fe::SlangParser::mapMatrix(slang::TypeLayoutReflection* type_layout, shader::ValueType& type) {
    assert(type_layout);

    uint32_t rows = type_layout->getRowCount();
    uint32_t cols = type_layout->getColumnCount();

    if (rows == 4 && cols == 4)
        type = shader_value::MAT4;
    else if (rows == 3 && cols == 3)
        type = shader_value::MAT3;
    else
        assert(false);
}

void fe::SlangParser::mapVector(slang::TypeLayoutReflection* type_layout, shader::ValueType& type) {
    assert(type_layout);

    slang_scalar scalar          = type_layout->getScalarType();
    uint32_t      component_count = type_layout->getElementCount();

    switch (scalar) {
        case slang_scalar::Float32:
            if (component_count == 2) type = shader_value::FLOAT2;
            if (component_count == 3) type = shader_value::FLOAT3;
            if (component_count == 4) type = shader_value::FLOAT4;
            break;
        case slang_scalar::Int32:
            if (component_count == 2) type = shader_value::INT2;
            if (component_count == 3) type = shader_value::INT3;
            if (component_count == 4) type = shader_value::INT4;
            break;
        case slang_scalar::UInt32:
            if (component_count == 2) type = shader_value::UINT2;
            if (component_count == 3) type = shader_value::UINT3;
            if (component_count == 4) type = shader_value::UINT4;
            break;
        default:
            assert(false);
            break;
    }
}

void fe::SlangParser::mapScalar(slang::TypeLayoutReflection* type_layout, shader::ValueType& type) {
    assert(type_layout);

    slang_scalar scalar = type_layout->getScalarType();

    // clang-format off
    switch (scalar) {
        case slang_scalar::Void     : type = shader_value::VOID     ; break;
        case slang_scalar::Bool     : type = shader_value::BOOL     ; break;
        case slang_scalar::Int32    : type = shader_value::INT32    ; break;
        case slang_scalar::UInt32   : type = shader_value::UINT32   ; break;
        case slang_scalar::Int64    : type = shader_value::INT64    ; break;
        case slang_scalar::UInt64   : type = shader_value::UINT64   ; break;
        case slang_scalar::Float16  : type = shader_value::FLOAT16  ; break;
        case slang_scalar::Float32  : type = shader_value::FLOAT32  ; break;
        case slang_scalar::Float64  : type = shader_value::FLOAT64  ; break;
        case slang_scalar::Int8     : type = shader_value::INT8     ; break;
        case slang_scalar::UInt8    : type = shader_value::UINT8    ; break;
        case slang_scalar::Int16    : type = shader_value::INT16    ; break;
        case slang_scalar::UInt16   : type = shader_value::UINT16   ; break;
        case slang_scalar::IntPtr   : type = shader_value::INT_PTR  ; break;
        case slang_scalar::UIntPtr  : type = shader_value::UINT_PTR ; break;
        //case slang_scalar::BFloat16 : type = shader_value::BFLOAT16 ; break;
        //case slang_scalar::FloatE4M3: type = shader_value::FLOATE4M3; break;
        //case slang_scalar::FloatE5M2: type = shader_value::FLOATE5M2; break;
        case slang_scalar::None:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\ngot slang::TypeReflection::ScalarType::None.\nSetting type as fe::shader::ValueType::UNKNOWN");
            type = shader_value::UNKNOWN;
            break;
        default:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\ngot unknown slang::TypeReflection::ScalarType.\nSetting type as fe::shader::ValueType::UNKNOWN");
            type = shader_value::UNKNOWN;
            break;
    }
    // clang-format on
}
