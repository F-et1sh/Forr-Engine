/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for Slang

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

#include <iostream>

using namespace fe::resource;

namespace fe {
    using _shader_descriptor = ShaderProgram::DescriptorType;
    using _shader_value      = ShaderProgram::ValueType;
    using _shader_type       = ShaderProgram::ShaderType;

    using _slang_kind     = slang::TypeReflection::Kind;
    using _slang_scalar   = slang::TypeReflection::ScalarType;
    using _slang_category = slang::ParameterCategory;
} // namespace fe

fe::pointer<fe::resource::ShaderProgram> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    ShaderProgram shader_program{};

    ShaderImportContext context{ storage, resource_full_path, shader_program };

    // compile
    bool compilation_result = ShaderImporter::compile(context);
    if (!compilation_result) {
        fe::logging::error("Slang -> Unified. Failed to compile a shader program\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    // reflect
    bool reflection_result = ShaderImporter::reflect(context);
    if (!reflection_result) {
        fe::logging::error("Slang -> Unified. Failed to reflect a shader program\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    // validate
    bool validation_result = ShaderImporter::validate(context);
    if (!validation_result) {
        fe::logging::error("Slang -> Unified. Validation failed\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    auto ptr = storage.CreateResource(std::move(shader_program));
    return ptr;
}

bool fe::ShaderImporter::compile(ShaderImportContext& context) {
    static Slang::ComPtr<slang::IGlobalSession> global_session{};
    if (!global_session) {
        if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef()))) {
            fe::logging::error("Slang -> Unified. Failed to create global session");
            return false;
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

    if (SLANG_FAILED(global_session->createSession(session_desc, context.session.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to create a session");
        return false;
    }

    slang::IModule* slang_module = nullptr;

    slang_module = context.session->loadModule(context.resource_full_path.string().c_str());
    if (!slang_module) {
        fe::logging::error("Slang -> Unified. Failed to load a slang module");
        return false;
    }

    std::vector<slang::IComponentType*> component_types{};
    component_types.emplace_back(slang_module);

    std::vector<EntryPoint> entry_points{};

    for (std::uint32_t i = 0; i < std::size(ShaderImportContext::entry_point_names); i++) {
        auto                entry_point_name = ShaderImportContext::entry_point_names[i];
        slang::IEntryPoint* entry_point{};
        slang_module->findEntryPointByName(entry_point_name.data(), &entry_point);

        if (entry_point) {
            ShaderProgram::ShaderType shader_type{};

            if (entry_point_name == ShaderImportContext::entry_point_names[0])
                shader_type = ShaderProgram::ShaderType::VERTEX;
            else if (entry_point_name == ShaderImportContext::entry_point_names[1])
                shader_type = ShaderProgram::ShaderType::FRAGMENT;
            else if (entry_point_name == ShaderImportContext::entry_point_names[2])
                shader_type = ShaderProgram::ShaderType::COMPUTE;

            entry_points.emplace_back(entry_point, shader_type);
            component_types.emplace_back(entry_point);
        }
    }

    SlangResult result = context.session->createCompositeComponentType(component_types.data(), component_types.size(), context.composed_program.writeRef());
    if (SLANG_FAILED(result)) {
        fe::logging::error("Slang -> Unified. Failed to create a composed program");
        return false;
    }

    for (std::size_t i = 0; i < entry_points.size(); i++) {
        const auto&                 entry_point = entry_points[i];
        Slang::ComPtr<slang::IBlob> spirv_code{};

        SlangResult result = context.composed_program->getEntryPointCode(i, 0, spirv_code.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Slang -> Unified. Failed to get an entry point code\nEntry point index : %i", i);
            return false;
        }

        const size_t   byte_size = spirv_code->getBufferSize();
        const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(spirv_code->getBufferPointer());

        ShaderProgram::ShaderType shader_type     = entry_point.shader_type;
        auto&                     source_code_dst = context.shader_program.source_codes[shader_type];

        source_code_dst.resize(byte_size, 0);
        std::memcpy(source_code_dst.data(), raw_data, byte_size);
    }

    return true;
}

bool fe::ShaderImporter::reflect(ShaderImportContext& context) {
    auto& shader_program = context.shader_program;

    slang::ProgramLayout* program_layout = context.composed_program->getLayout();
    if (!program_layout) FORR_UNLIKELY {
        fe::logging::error("Slang -> Unified. Failed to reflect a shader\nFailed to get the layout of the composed program while reflection in fe::ShaderImporter::reflect()");
        return false;
    }

    unsigned int parameter_count = program_layout->getParameterCount();

    shader_program.reflected_parameters.reserve(parameter_count);

    for (unsigned int i = 0; i < parameter_count; i++) {
        slang::VariableLayoutReflection* variable_layout = program_layout->getParameterByIndex(i);
        if (!variable_layout) {
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\nslang::VariableLayoutReflection* variable_layout = program_layout->getParameterByIndex(i) was nullptr. i = %i", i);
            continue;
        }

        _slang_category category = variable_layout->getCategory();

        if (category == _slang_category::DescriptorTableSlot) {
            auto& parameter = shader_program.reflected_parameters.emplace_back();
            ShaderImporter::parseDescriptorTable(context, variable_layout, parameter);
        }
        else if (category == _slang_category::PushConstantBuffer) {
            ShaderImporter::parsePushConstant(variable_layout, shader_program.reflected_push_constants);
        }
        else {
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\nUnknown slang::ParameterCategory : %i", category);
            continue;
        }
    }

    return true;
}

bool fe::ShaderImporter::validate(ShaderImportContext& context) {
    // TODO : add define for shader to mute the warning message
    // TODO : optimize work with strings here
    // TODO : finish tihs validation somehow later

    const auto& shader_program = context.shader_program;

    const static ShaderProgram::ReflectedPushConstants expected_push_constants{
        { ShaderProgram::ValueType::INT32,
          1,
          4,
          {},
          "instance_index" },
        7
    };

    const static std::array expected_parameters{
        ShaderProgram::ReflectedParameter{
            { ShaderProgram::ValueType::STRUCT,
              1,
              144,
              { { { ShaderProgram::ValueType::MAT4,
                    1,
                    64,
                    {},
                    "projection_matrix" },
                  0 },
                { { ShaderProgram::ValueType::MAT4,
                    1,
                    64,
                    {},
                    "view_matrix" },
                  64 },
                { { ShaderProgram::ValueType::UINT32,
                    1,
                    4,
                    {},
                    "lights_count" },
                  128 } },
              "global_data" },
            ShaderProgram::DescriptorType::UNIFORM_BUFFER,
            0,
            0,
            7,
            false },

        ShaderProgram::ReflectedParameter{
            { ShaderProgram::ValueType::MAT4,
              0,
              64,
              {},
              "model_matrices" },
            ShaderProgram::DescriptorType::STORAGE_BUFFER,
            0,
            1,
            7,
            false },

            // ...
    };

    // you can change the name           --> warning
    // you cannot change everything else --> error

    checkDataNode(context,
                  "push constants",
                  static_cast<const resource::ShaderProgram::ReflectedDataNode*>(&shader_program.reflected_push_constants),
                  static_cast<const resource::ShaderProgram::ReflectedDataNode*>(&expected_push_constants));
    checkAndPrintProblem("push constants stage_flags", shader_program.reflected_push_constants.stage_flags, expected_push_constants.stage_flags, fe::logging::Severity::Warning);

    return true;
}

void fe::ShaderImporter::checkAndPrintProblem(const std::string&    field_name,
                                              auto                  changed_to,
                                              auto                  should_be,
                                              fe::logging::Severity severity) {
    constexpr bool is_string = std::is_convertible_v<decltype(changed_to), const std::string&>;

    if constexpr (is_string) {
        if (std::string(changed_to) == std::string(should_be)) return;
    }
    else {
        if (changed_to == should_be) return;
    }

    constexpr const char* fmt = is_string ? "%s" : "%i";

    std::string message = std::string("Slang -> Unified. Validation : field %s was changed to ") + fmt;
    message += (severity < fe::logging::Severity::Error) ? ", but it should be " : ", but it must be ";
    message += fmt;

    if constexpr (is_string) {
        fe::logging::message(severity, message.c_str(), field_name.c_str(), std::string_view(changed_to).data(), std::string_view(should_be).data());
    }
    else {
        fe::logging::message(severity, message.c_str(), field_name.c_str(), changed_to, should_be);
    }
}

void fe::ShaderImporter::checkDataNode(ShaderImportContext&                              context,
                                       const std::string&                                field_name,
                                       const resource::ShaderProgram::ReflectedDataNode* data_node,
                                       const resource::ShaderProgram::ReflectedDataNode* expected_data_node) {

    ShaderImporter::checkAndPrintProblem(field_name + std::string(" name"), data_node->name.c_str(), expected_data_node->name.c_str(), fe::logging::Severity::Warning);
    ShaderImporter::checkAndPrintProblem(field_name + std::string(" size"), data_node->size, expected_data_node->size, fe::logging::Severity::Error);
    ShaderImporter::checkAndPrintProblem(field_name + std::string(" array_size"), data_node->array_size, expected_data_node->array_size, fe::logging::Severity::Error);
    ShaderImporter::checkAndPrintProblem(field_name + std::string(" type"), static_cast<int>(data_node->type), static_cast<int>(expected_data_node->type), fe::logging::Severity::Error);
    ShaderImporter::checkAndPrintProblem(field_name + std::string(" members.size()"), data_node->members.size(), expected_data_node->members.size(), fe::logging::Severity::Error);
}

void fe::ShaderImporter::parseDescriptorTable(ShaderImportContext&                         context,
                                              slang::VariableLayoutReflection*             variable_layout,
                                              resource::ShaderProgram::ReflectedParameter& dst_parameter) {
    assert(variable_layout);
    assert(variable_layout->getCategory() == _slang_category::DescriptorTableSlot);

    dst_parameter.set         = variable_layout->getBindingSpace();
    dst_parameter.binding     = variable_layout->getBindingIndex();
    dst_parameter.array_size  = 1;
    dst_parameter.stage_flags = static_cast<uint32_t>(_shader_type::NONE);

    // this doesn't work
    //SlangStage stage = variable_layout->getStage();
    //dst_parameter.stage_flags =
    //    ((stage & SLANG_STAGE_VERTEX) ? static_cast<uint32_t>(_shader_type::VERTEX) : 0) |
    //    ((stage & SLANG_STAGE_FRAGMENT) ? static_cast<uint32_t>(_shader_type::FRAGMENT) : 0) |
    //    ((stage & SLANG_STAGE_COMPUTE) ? static_cast<uint32_t>(_shader_type::COMPUTE) : 0) |
    //    ((stage & SLANG_STAGE_GEOMETRY) ? static_cast<uint32_t>(_shader_type::GEOMETRY) : 0);

    dst_parameter.stage_flags |= static_cast<uint32_t>(_shader_type::VERTEX);
    dst_parameter.stage_flags |= static_cast<uint32_t>(_shader_type::FRAGMENT);
    dst_parameter.stage_flags |= static_cast<uint32_t>(_shader_type::COMPUTE);
    dst_parameter.stage_flags |= static_cast<uint32_t>(_shader_type::GEOMETRY);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    _slang_kind                  kind        = type_layout->getKind();

    // 24.06.2026 Slang has got a bug, so, now this will work like this
    //
    // TODO : update Slang submodule and remove this part
    //
    // 02.07.2026 Slang don't want to convert sets into OpenGL bindings, so, I have to hardcode this
    {
        //static uint32_t vulkan_set_counter     = 0;
        static uint32_t opengl_binding_counter = 0;

        if (dst_parameter.name == "scene_set") {
            //vulkan_set_counter     = 0;
            opengl_binding_counter = 0;
        }

        auto& graphics_backend = context.storage.GetContext().graphics_backend;

        if (graphics_backend == GraphicsBackend::OpenGL) {
            dst_parameter.set     = 0;
            dst_parameter.binding = opengl_binding_counter++;
        }
    }

    switch (kind) {
        case _slang_kind::ConstantBuffer:
        case _slang_kind::ParameterBlock:
            dst_parameter.descriptor_type = _shader_descriptor::UNIFORM_BUFFER;
            ShaderImporter::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
            break;

        case _slang_kind::ShaderStorageBuffer:
            dst_parameter.descriptor_type = _shader_descriptor::STORAGE_BUFFER;
            ShaderImporter::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
            break;

        case _slang_kind::Array: {
            // if you got '_slang_kind::Array' here that means that this is a bindless parameter
            dst_parameter.is_bindless = true;
            // when you pass 'slang::TypeLayoutReflection*' into 'fe::ShaderImporter::parseMemberRecursive()' instead of 'slang::VariableLayoutReflection*'
            //  you have to set 'array_size' yourself
            dst_parameter.array_size = type_layout->getElementCount();

            slang::TypeLayoutReflection* array_element_type_layout = type_layout->getElementTypeLayout();
            _slang_kind                  element_kind              = array_element_type_layout->getKind();

            if (element_kind == _slang_kind::Resource) {
                SlangResourceShape shape      = array_element_type_layout->getResourceShape();
                unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

                if (shape_base >= SLANG_TEXTURE_1D && shape_base <= SLANG_TEXTURE_CUBE) {
                    bool is_read_write            = array_element_type_layout->getResourceAccess() == SLANG_RESOURCE_ACCESS_READ_WRITE;
                    dst_parameter.descriptor_type = is_read_write ? _shader_descriptor::STORAGE_IMAGE : _shader_descriptor::COMBINED_IMAGE_SAMPLER;
                }
            }
            else {
                dst_parameter.descriptor_type = _shader_descriptor::UNIFORM_BUFFER;
            }

            ShaderImporter::parseMemberRecursive(array_element_type_layout, static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
        } break;

        case _slang_kind::Resource: {
            // when you pass 'slang::TypeLayoutReflection*' into 'fe::ShaderImporter::parseMemberRecursive()' instead of 'slang::VariableLayoutReflection*'
            //  you have to set 'array_size' yourself
            dst_parameter.array_size = type_layout->getElementCount();

            SlangResourceShape shape      = type_layout->getResourceShape();
            unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

            SlangResourceAccess resource_access = type_layout->getResourceAccess();

            if (shape_base >= SLANG_TEXTURE_1D &&
                shape_base <= SLANG_TEXTURE_CUBE) {

                if (resource_access == SLANG_RESOURCE_ACCESS_READ_WRITE) {
                    dst_parameter.descriptor_type = _shader_descriptor::STORAGE_IMAGE;
                }
                else {
                    dst_parameter.descriptor_type = _shader_descriptor::COMBINED_IMAGE_SAMPLER;
                }
            }
            else if (shape_base == SLANG_STRUCTURED_BUFFER ||
                     shape_base == SLANG_BYTE_ADDRESS_BUFFER) {

                dst_parameter.descriptor_type = _shader_descriptor::STORAGE_BUFFER;
            }
            else if (shape_base == SLANG_ACCELERATION_STRUCTURE) {
                dst_parameter.descriptor_type = _shader_descriptor::ACCELERATION_STRUCTURE;
            }
            else {
                assert(false);
            }

            ShaderImporter::parseMemberRecursive(type_layout->getElementTypeLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
        } break;

        case _slang_kind::SamplerState:
            dst_parameter.descriptor_type = _shader_descriptor::SAMPLER;
            ShaderImporter::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
            break;

        default:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\nUnknown descriptor resource kind : %i", kind);
            dst_parameter.descriptor_type = _shader_descriptor::UNKNOWN;
            break;
    }

    // here we have to override the name, because 'fe::ShaderImporter::parseMemberRecursive()' setting 'dst_parameter.name' as its
    //  struct's name, but I want to see its actual name.
    // For example :
    // ```slang
    // [[vk::binding(0, 0)]] ConstantBuffer<SceneData> global_data;
    // ```
    // 'fe::ShaderImporter::parseMemberRecursive()' will give us "SceneData", but I want to see name "global_data",
    //  that's why we set the name here
    dst_parameter.name = variable_layout->getName();
}

void fe::ShaderImporter::parsePushConstant(slang::VariableLayoutReflection* variable_layout, resource::ShaderProgram::ReflectedPushConstants& dst_push_constants) {
    assert(variable_layout);
    assert(variable_layout->getCategory() == _slang_category::PushConstantBuffer);

    dst_push_constants.array_size  = 1;
    dst_push_constants.stage_flags = static_cast<uint32_t>(_shader_type::NONE);

    // this doesn't work
    //SlangStage stage = variable_layout->getStage();
    //dst_push_constants.stage_flags =
    //    ((stage & SLANG_STAGE_VERTEX) ? static_cast<uint32_t>(_shader_type::VERTEX) : 0) |
    //    ((stage & SLANG_STAGE_FRAGMENT) ? static_cast<uint32_t>(_shader_type::FRAGMENT) : 0) |
    //    ((stage & SLANG_STAGE_COMPUTE) ? static_cast<uint32_t>(_shader_type::COMPUTE) : 0) |
    //    ((stage & SLANG_STAGE_GEOMETRY) ? static_cast<uint32_t>(_shader_type::GEOMETRY) : 0);

    dst_push_constants.stage_flags |= static_cast<uint32_t>(_shader_type::VERTEX);
    dst_push_constants.stage_flags |= static_cast<uint32_t>(_shader_type::FRAGMENT);
    dst_push_constants.stage_flags |= static_cast<uint32_t>(_shader_type::COMPUTE);
    dst_push_constants.stage_flags |= static_cast<uint32_t>(_shader_type::GEOMETRY);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    _slang_kind                  kind        = type_layout->getKind();

    switch (kind) {
        case _slang_kind::ConstantBuffer:
        case _slang_kind::ParameterBlock:
            ShaderImporter::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_push_constants));
            break;

        case _slang_kind::ShaderStorageBuffer:
            ShaderImporter::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_push_constants));
            break;

        case _slang_kind::Array: {
            dst_push_constants.array_size = type_layout->getElementCount();
            ShaderImporter::parseMemberRecursive(type_layout->getElementTypeLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_push_constants));
        } break;

        case _slang_kind::Resource: {
            dst_push_constants.array_size = type_layout->getElementCount();
            ShaderImporter::parseMemberRecursive(type_layout->getElementTypeLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_push_constants));
        } break;

        case _slang_kind::SamplerState:
            ShaderImporter::parseMemberRecursive(type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_push_constants));
            break;

        default:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\nUnknown descriptor resource kind : %i", kind);
            break;
    }

    // here we have to override the name, because 'fe::ShaderImporter::parseMemberRecursive()' setting 'dst_parameter.name' as its
    //  struct's name, but I want to see its actual name.
    // For example :
    // ```slang
    // [[vk::binding(0, 0)]] ConstantBuffer<SceneData> global_data;
    // ```
    // 'fe::ShaderImporter::parseMemberRecursive()' will give us "SceneData", but I want to see name "global_data",
    //  that's why we set the name here
    dst_push_constants.name = variable_layout->getName();
}

void fe::ShaderImporter::parseMemberRecursive(slang::VariableLayoutReflection* variable_layout, resource::ShaderProgram::ReflectedDataNode* dst_reflected_data_node) {
    assert(variable_layout);
    assert(dst_reflected_data_node);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();

    dst_reflected_data_node->name = variable_layout->getName() ? variable_layout->getName() : type_layout->getName();

    // if offset is not zero, then it is most likely a member aka a field of a struct,
    // so, we can cast it to 'resource::ShaderProgram::ReflectedMember*' and set its 'offset'
    if (variable_layout->getOffset() != 0) {
        resource::ShaderProgram::ReflectedMember* member = static_cast<resource::ShaderProgram::ReflectedMember*>(dst_reflected_data_node);
        member->offset                                   = variable_layout->getOffset();
    }

    ShaderImporter::parseMemberRecursive(type_layout, dst_reflected_data_node);
}

void fe::ShaderImporter::parseMemberRecursive(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ReflectedDataNode* dst_reflected_data_node) {
    assert(type_layout);
    assert(dst_reflected_data_node);

    dst_reflected_data_node->size = type_layout->getSize();

    auto parse_recursive = [](slang::TypeLayoutReflection*                           type_layout,
                              std::vector<resource::ShaderProgram::ReflectedMember>& dst_members) {
        uint32_t field_count = type_layout->getFieldCount();
        dst_members.reserve(field_count);
        for (uint32_t i = 0; i < field_count; i++) {
            resource::ShaderProgram::ReflectedMember& member = dst_members.emplace_back();
            parseMemberRecursive(type_layout->getFieldByIndex(i), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&member));
        }
    };

    _slang_kind kind = type_layout->getKind();

    switch (kind) {
        case _slang_kind::Struct:
            dst_reflected_data_node->type = _shader_value::STRUCT;
            parse_recursive(type_layout, dst_reflected_data_node->members);
            break;

        case _slang_kind::Array: {
            dst_reflected_data_node->type = _shader_value::STRUCT;

            slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
            dst_reflected_data_node->array_size              = type_layout->getElementCount();

            if (element_type_layout->getKind() == _slang_kind::Struct) {
                parse_recursive(element_type_layout, dst_reflected_data_node->members);
            }
            else {
                ShaderImporter::mapScalar(element_type_layout, dst_reflected_data_node->type);
            }
        } break;

            // clang-format off
        case _slang_kind::Matrix        : ShaderImporter::mapMatrix(type_layout, dst_reflected_data_node->type); break;
        case _slang_kind::Vector        : ShaderImporter::mapVector(type_layout, dst_reflected_data_node->type); break;
        case _slang_kind::Scalar        : ShaderImporter::mapScalar(type_layout, dst_reflected_data_node->type); break;
        
        case _slang_kind::SamplerState  : dst_reflected_data_node->type = _shader_value::UINT64     ; break;
        case _slang_kind::Pointer       : dst_reflected_data_node->type = _shader_value::UINT_PTR   ; break;
            // clang-format on

        case _slang_kind::Resource: {
            SlangResourceShape shape      = type_layout->getResourceShape();
            unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

            if (shape_base >= SLANG_TEXTURE_1D &&
                shape_base <= SLANG_TEXTURE_CUBE) {

                dst_reflected_data_node->type = _shader_value::UINT32;
            }
            else if (shape_base == SLANG_STRUCTURED_BUFFER ||
                     shape_base == SLANG_BYTE_ADDRESS_BUFFER) {

                dst_reflected_data_node->type = _shader_value::UINT_PTR;
            }
        } break;

        case _slang_kind::Enum:
            ShaderImporter::mapScalar(type_layout, dst_reflected_data_node->type);

            if (dst_reflected_data_node->type == _shader_value::UNKNOWN)
                dst_reflected_data_node->type = _shader_value::INT32;
            break;

        default:
            fe::logging::warning("Slang -> Unified. Unhandled member kind %i for '%s'. Setting as UNKNOWN", kind, dst_reflected_data_node->name.c_str());
            dst_reflected_data_node->type = _shader_value::UNKNOWN;
            break;
    }
}

void fe::ShaderImporter::mapMatrix(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ValueType& type) {
    assert(type_layout);

    uint32_t rows = type_layout->getRowCount();
    uint32_t cols = type_layout->getColumnCount();

    if (rows == 4 && cols == 4)
        type = _shader_value::MAT4;
    else if (rows == 3 && cols == 3)
        type = _shader_value::MAT3;
    else
        assert(false);
}

void fe::ShaderImporter::mapVector(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ValueType& type) {
    assert(type_layout);

    _slang_scalar scalar          = type_layout->getScalarType();
    uint32_t      component_count = type_layout->getElementCount();

    switch (scalar) {
        case _slang_scalar::Float32:
            if (component_count == 2) type = _shader_value::FLOAT2;
            if (component_count == 3) type = _shader_value::FLOAT3;
            if (component_count == 4) type = _shader_value::FLOAT4;
            break;
        case _slang_scalar::Int32:
            if (component_count == 2) type = _shader_value::INT2;
            if (component_count == 3) type = _shader_value::INT3;
            if (component_count == 4) type = _shader_value::INT4;
            break;
        case _slang_scalar::UInt32:
            if (component_count == 2) type = _shader_value::UINT2;
            if (component_count == 3) type = _shader_value::UINT3;
            if (component_count == 4) type = _shader_value::UINT4;
            break;
        default:
            assert(false);
            break;
    }
}

void fe::ShaderImporter::mapScalar(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ValueType& type) {
    assert(type_layout);

    _slang_scalar scalar = type_layout->getScalarType();

    // clang-format off
    switch (scalar) {
        case _slang_scalar::Void     : type = _shader_value::VOID     ; break;
        case _slang_scalar::Bool     : type = _shader_value::BOOL     ; break;
        case _slang_scalar::Int32    : type = _shader_value::INT32    ; break;
        case _slang_scalar::UInt32   : type = _shader_value::UINT32   ; break;
        case _slang_scalar::Int64    : type = _shader_value::INT64    ; break;
        case _slang_scalar::UInt64   : type = _shader_value::UINT64   ; break;
        case _slang_scalar::Float16  : type = _shader_value::FLOAT16  ; break;
        case _slang_scalar::Float32  : type = _shader_value::FLOAT32  ; break;
        case _slang_scalar::Float64  : type = _shader_value::FLOAT64  ; break;
        case _slang_scalar::Int8     : type = _shader_value::INT8     ; break;
        case _slang_scalar::UInt8    : type = _shader_value::UINT8    ; break;
        case _slang_scalar::Int16    : type = _shader_value::INT16    ; break;
        case _slang_scalar::UInt16   : type = _shader_value::UINT16   ; break;
        case _slang_scalar::IntPtr   : type = _shader_value::INT_PTR  ; break;
        case _slang_scalar::UIntPtr  : type = _shader_value::UINT_PTR ; break;
        //case _slang_scalar::BFloat16 : type = _shader_value::BFLOAT16 ; break;
        //case _slang_scalar::FloatE4M3: type = _shader_value::FLOATE4M3; break;
        //case _slang_scalar::FloatE5M2: type = _shader_value::FLOATE5M2; break;
        case _slang_scalar::None:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\ngot slang::TypeReflection::ScalarType::None.\nSetting type as fe::resource::ShaderProgram::ValueType::UNKNOWN");
            type = _shader_value::UNKNOWN;
            break;
        default:
            fe::logging::error("Slang -> Unified. Failed to reflect a shader\ngot unknown slang::TypeReflection::ScalarType.\nSetting type as fe::resource::ShaderProgram::ValueType::UNKNOWN");
            type = _shader_value::UNKNOWN;
            break;
    }
    // clang-format on
}
