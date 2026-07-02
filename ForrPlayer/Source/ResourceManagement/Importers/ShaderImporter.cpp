/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for Slang

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

#include <fstream>

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

    // Validation is temporarily turned off till I completely fix the shaders

    // validate
    //bool validation_result = ShaderImporter::validate(context);
    //if (!validation_result) {
    //    fe::logging::error("Slang -> Unified. Validation failed\nPath : %s", resource_full_path.string().c_str());
    //    return {};
    //}

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

    slang::ProgramLayout* program_layout = context.composed_program->getLayout(0); // 'target = 0'
    if (!program_layout) FORR_UNLIKELY {
        fe::logging::error("Slang -> Unified. Failed to get the layout of the composed program while reflection in fe::ShaderImporter::reflect()");
        return false;
    }

    unsigned int parameter_count = program_layout->getParameterCount();

    shader_program.reflected_parameters.reserve(parameter_count);

    for (unsigned int i = 0; i < parameter_count; i++) {
        slang::VariableLayoutReflection* variable_layout = program_layout->getParameterByIndex(i);
        if (!variable_layout) {
            fe::logging::warning("Slang -> Unified. slang::VariableLayoutReflection* variable_layout = program_layout->getParameterByIndex(i) was nullptr. i = %i", i);
            continue;
        }

        _slang_category category = variable_layout->getCategory();

        if (category == _slang_category::DescriptorTableSlot) {
            auto& parameter = shader_program.reflected_parameters.emplace_back();
            ShaderImporter::parseDescriptorTable(context, variable_layout, parameter);
        }
        else if (category == _slang_category::PushConstantBuffer) {
            ShaderImporter::parsePushConstant(context, variable_layout, shader_program.reflected_push_constants);
        }
        else {
            assert(false);
        }
    }

    return true;
}

//bool fe::ShaderImporter::validate(ShaderImportContext& context) {
//    // TODO : add define for shader to mute the warning message
//    // TODO : make validation more strict
//
//    auto& graphics_backend    = context.storage.GetContext().graphics_backend;
//    auto& reflected_resources = context.shader_program.reflected_resources;
//
//    struct ExpectedResource {
//        ShaderProgram::DescriptorType descriptor_type{};
//
//        uint32_t set{};
//        uint32_t binding{};
//
//        uint32_t members_count{};
//
//        std::string_view name{};
//    };
//
//    constexpr static std::array expected_resources_vulkan{
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 0, 1, "global_data" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 1, 1, "model_matrices" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 2, 1, "lights" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 1, 0, 1, "materials" },
//        ExpectedResource{ ShaderProgram::DescriptorType::PUSH_CONSTANT, 0, 0, 1, "push_constants" }
//    };
//
//    constexpr static std::array expected_resources_opengl{
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 0, 1, "global_data" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 1, 1, "model_matrices" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 2, 1, "lights" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 3, 1, "materials" },
//        ExpectedResource{ ShaderProgram::DescriptorType::PUSH_CONSTANT, 0, 0, 1, "push_constants" }
//    };
//
//    const auto& expected_resources = (graphics_backend == GraphicsBackend::OpenGL) ? expected_resources_opengl : expected_resources_vulkan;
//
//    for (const auto& expected_resource : expected_resources) {
//        auto it = reflected_resources.end();
//
//        if (expected_resource.descriptor_type == ShaderProgram::DescriptorType::PUSH_CONSTANT) {
//            it = std::ranges::find(reflected_resources, ShaderProgram::DescriptorType::PUSH_CONSTANT, &ShaderProgram::ReflectedResource::descriptor_type);
//        }
//        else {
//            it = std::ranges::find_if(reflected_resources, [&](const auto& resource) -> bool {
//                return resource.set == expected_resource.set &&
//                       resource.binding == expected_resource.binding;
//            });
//        }
//
//        if (it == reflected_resources.end()) {
//            fe::logging::warning("Slang -> Unified. Shader has no descriptor set %i ( \"%s\" ).\nIf you don't need it in the shader you don't have to add it",
//                                 expected_resource.set,
//                                 expected_resource.name.data());
//            continue;
//        }
//
//        if (it->descriptor_type != expected_resource.descriptor_type) {
//            fe::logging::error("Slang -> Unified. Shader descriptor set %i has descriptor type %i, but must be %i",
//                               expected_resource.set,
//                               it->descriptor_type,
//                               expected_resource.descriptor_type);
//            return false;
//        }
//
//        if (it->binding != expected_resource.binding) {
//            fe::logging::error("Slang -> Unified. Shader descriptor set %i has binding %i, but must be %i",
//                               expected_resource.set,
//                               it->binding,
//                               expected_resource.binding);
//            return false;
//        }
//
//        if (it->members.size() != expected_resource.members_count) {
//            fe::logging::error("Slang -> Unified. Shader descriptor set %i has members count %i, but must be %i",
//                               expected_resource.set,
//                               it->members.size(),
//                               expected_resource.members_count);
//            return false;
//        }
//
//        if (it->name != expected_resource.name) {
//            fe::logging::error("Slang -> Unified. Shader descriptor set %i has name %s, but must be \"%s\"",
//                               expected_resource.set,
//                               it->name.c_str(),
//                               expected_resource.name.data());
//            return false;
//        }
//    }
//
//    return true;
//}

void fe::ShaderImporter::decompile1(ShaderImportContext&                         context,
                                    slang::VariableLayoutReflection*             variable_layout,
                                    resource::ShaderProgram::ReflectedParameter& dst_parameter) {
    assert(variable_layout);

    // auto& dst_parameter = fe::resource::ShaderProgram::reflected_parameters.emplace_back()

    _slang_category c = variable_layout->getCategory(); // DescriptorTableSlot
    auto            n = variable_layout->getName();     // "global_data"

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    _slang_kind                  k           = type_layout->getKind();       // ConstantBuffer
    auto                         n2          = type_layout->getName();       // "ConstantBuffer"
    unsigned int                 field_count = type_layout->getFieldCount(); // 0
    size_t                       s           = type_layout->getSize();       // 0

    // auto& member = dst_parameter.members.emplace_back()

    slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
    _slang_kind                  k2                  = element_type_layout->getKind();       // Struct
    auto                         n3                  = element_type_layout->getName();       // "SceneData"
    unsigned int                 field_count2        = element_type_layout->getFieldCount(); // 3
    size_t                       s2                  = element_type_layout->getSize();       // 144

    // auto& member1 = member.members.emplace_back()

    slang::VariableLayoutReflection* field_variable_layout = element_type_layout->getFieldByIndex(0);

    {
        _slang_category f_c = field_variable_layout->getCategory(); // Uniform
        auto            f_n = field_variable_layout->getName();     // "projection_matrix"

        slang::TypeLayoutReflection* f_type_layout = field_variable_layout->getTypeLayout();
        _slang_kind                  f_k           = f_type_layout->getKind();       // Matrix
        auto                         f_n2          = f_type_layout->getName();       // "matrix"
        unsigned int                 f_field_count = f_type_layout->getFieldCount(); // 0
        size_t                       f_s           = f_type_layout->getSize();

        slang::TypeLayoutReflection* f_element_type_layout = f_type_layout->getElementTypeLayout();
        _slang_kind                  f_k2                  = f_element_type_layout->getKind();       // Vector
        auto                         f_n3                  = f_element_type_layout->getName();       // "vector"
        unsigned int                 f_field_count2        = f_element_type_layout->getFieldCount(); // 0
        size_t                       f_s2                  = f_element_type_layout->getSize();
    }

    // auto& member2 = member.members.emplace_back()

    slang::VariableLayoutReflection* field_variable_layout2 = element_type_layout->getFieldByIndex(1);

    {
        _slang_category f_c = field_variable_layout2->getCategory(); // Uniform
        auto            f_n = field_variable_layout2->getName();     // "view_matrix"

        slang::TypeLayoutReflection* f_type_layout = field_variable_layout2->getTypeLayout();
        _slang_kind                  f_k           = f_type_layout->getKind();       // Matrix
        auto                         f_n2          = f_type_layout->getName();       // "matrix"
        unsigned int                 f_field_count = f_type_layout->getFieldCount(); // 0

        slang::TypeLayoutReflection* f_element_type_layout = f_type_layout->getElementTypeLayout();
        _slang_kind                  f_k2                  = f_element_type_layout->getKind();       // Vector
        auto                         f_n3                  = f_element_type_layout->getName();       // "vector"
        unsigned int                 f_field_count2        = f_element_type_layout->getFieldCount(); // 0
    }

    // auto& member3 = member.members.emplace_back()

    slang::VariableLayoutReflection* field_variable_layout3 = element_type_layout->getFieldByIndex(2);

    {
        _slang_category f_c = field_variable_layout3->getCategory(); // Uniform
        auto            f_n = field_variable_layout3->getName();     // "lights_count"

        slang::TypeLayoutReflection* f_type_layout = field_variable_layout3->getTypeLayout();
        _slang_kind                  f_k           = f_type_layout->getKind();       // Scalar
        auto                         f_n2          = f_type_layout->getName();       // "uint"
        unsigned int                 f_field_count = f_type_layout->getFieldCount(); // 0

        slang::TypeLayoutReflection* f_element_type_layout = f_type_layout->getElementTypeLayout();
        _slang_kind                  f_k2                  = f_element_type_layout->getKind();       // None
        auto                         f_n3                  = f_element_type_layout->getName();       // nullptr
        unsigned int                 f_field_count2        = f_element_type_layout->getFieldCount(); // 0
    }
}

void fe::ShaderImporter::decompile2(ShaderImportContext&                         context,
                                    slang::VariableLayoutReflection*             variable_layout,
                                    resource::ShaderProgram::ReflectedParameter& dst_parameter) {
    assert(variable_layout);

    // auto& dst_parameter = fe::resource::ShaderProgram::reflected_parameters.emplace_back()

    _slang_category c = variable_layout->getCategory(); // DescriptorTableSlot --> to know that this is an entry point
    auto            n = variable_layout->getName();     // "model_matrices"    --> to know that its name

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    _slang_kind                  k           = type_layout->getKind();       // Resource            --> to know that this is an SSBO ( using that SlangResourceShape and others )
    auto                         n2          = type_layout->getName();       // "StructuredBuffer"  --> unused
    unsigned int                 field_count = type_layout->getFieldCount(); // 0                   --> unused ( always unwrap till element type layout )
    size_t                       s           = type_layout->getSize();       // 0                   --> unused ( this don't give us an information about bindless, I guess )

    // auto& member = dst_parameter.members.emplace_back()

    slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
    _slang_kind                  k2                  = element_type_layout->getKind();       // Matrix      --> to know member's type
    auto                         n3                  = element_type_layout->getName();       // "matrix"    --> unused
    unsigned int                 field_count2        = element_type_layout->getFieldCount(); // 0           --> unused ( since we know member's type and it's not STRUCT or something )
    size_t                       s2                  = element_type_layout->getSize();       // 64          --> to know member's size
}

void fe::ShaderImporter::decompile3(ShaderImportContext&                         context,
                                    slang::VariableLayoutReflection*             variable_layout,
                                    resource::ShaderProgram::ReflectedParameter& dst_parameter) {
    assert(variable_layout);

    // [[vk::binding(0, 0)]] StructuredBuffer<Texture2D> textures[];

    // auto& dst_parameter = fe::resource::ShaderProgram::reflected_parameters.emplace_back()

    _slang_category c = variable_layout->getCategory(); // DescriptorTableSlot --> to know that this is an entry point
    auto            n = variable_layout->getName();     // "textures"    --> to know that its name

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    _slang_kind                  k           = type_layout->getKind();       // Array    --> to know that this is an SSBO ( using that SlangResourceShape and others )
    auto                         n2          = type_layout->getName();       // "Array"  --> unused
    unsigned int                 field_count = type_layout->getFieldCount(); // 0        --> unused ( always unwrap till element type layout )
    size_t                       s           = type_layout->getSize();       // 0        --> unused ( this don't give us an information about bindless, I guess )

    // auto& member = dst_parameter.members.emplace_back()

    slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
    _slang_kind                  k2                  = element_type_layout->getKind();       // Resource           --> to know member's type
    auto                         n3                  = element_type_layout->getName();       // "StructuredBuffer" --> unused
    unsigned int                 field_count2        = element_type_layout->getFieldCount(); // 0                  --> unused ( since we know member's type and it's not STRUCT or something )
    size_t                       s2                  = element_type_layout->getSize();       // 0                  --> to know member's size

    slang::TypeLayoutReflection* element_type_layout2 = element_type_layout->getElementTypeLayout();
    _slang_kind                  k3                   = element_type_layout2->getKind();       // Resource   --> to know member's type
    auto                         n4                   = element_type_layout2->getName();       // "_Texture" --> unused
    unsigned int                 field_count3         = element_type_layout2->getFieldCount(); // 0          --> unused ( since we know member's type and it's not STRUCT or something )
    size_t                       s3                   = element_type_layout2->getSize();       // 0          --> to know member's size
}

void fe::ShaderImporter::parseDescriptorTable(ShaderImportContext&                         context,
                                              slang::VariableLayoutReflection*             variable_layout,
                                              resource::ShaderProgram::ReflectedParameter& dst_parameter) {
    assert(variable_layout);
    assert(variable_layout->getCategory() == _slang_category::DescriptorTableSlot);

    dst_parameter.set         = variable_layout->getBindingSpace();
    dst_parameter.binding     = variable_layout->getBindingIndex();
    dst_parameter.stage_flags = static_cast<uint32_t>(_shader_type::NONE);

    SlangStage stage = variable_layout->getStage(); // does not work

    dst_parameter.stage_flags =
        ((stage & SLANG_STAGE_VERTEX) ? static_cast<uint32_t>(_shader_type::VERTEX) : 0) |
        ((stage & SLANG_STAGE_FRAGMENT) ? static_cast<uint32_t>(_shader_type::FRAGMENT) : 0) |
        ((stage & SLANG_STAGE_COMPUTE) ? static_cast<uint32_t>(_shader_type::COMPUTE) : 0) |
        ((stage & SLANG_STAGE_GEOMETRY) ? static_cast<uint32_t>(_shader_type::GEOMETRY) : 0);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
    _slang_kind                  kind        = type_layout->getKind();

    // 24.06.2026 Slang has got a bug, so, now this will work like this
    //
    // TODO : update Slang submodule and remove this part
    {
        static uint32_t vulkan_set_counter = 0;

        if (dst_parameter.name == "scene_set")
            vulkan_set_counter = 0;

        if (kind == _slang_kind::ParameterBlock) {
            dst_parameter.set     = vulkan_set_counter++;
            dst_parameter.binding = 0;
        }
        else if (kind == _slang_kind::ConstantBuffer ||
                 kind == _slang_kind::ShaderStorageBuffer) {
            dst_parameter.set     = vulkan_set_counter;
            dst_parameter.binding = variable_layout->getBindingIndex();
        }
        else {
            dst_parameter.set     = variable_layout->getBindingSpace();
            dst_parameter.binding = variable_layout->getBindingIndex();
        }

        auto& graphics_backend = context.storage.GetContext().graphics_backend;

        if (graphics_backend == GraphicsBackend::OpenGL) {
            dst_parameter.set     = 0;
            dst_parameter.binding = variable_layout->getBindingIndex();
        }
    }

    switch (kind) {
        case _slang_kind::ConstantBuffer:
        case _slang_kind::ParameterBlock:
            dst_parameter.descriptor_type = _shader_descriptor::UNIFORM_BUFFER;
            ShaderImporter::parseMemberRecursive(context, type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
            break;

        case _slang_kind::ShaderStorageBuffer:
            dst_parameter.descriptor_type = _shader_descriptor::STORAGE_BUFFER;
            ShaderImporter::parseMemberRecursive(context, type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
            break;

        case _slang_kind::Array:
            dst_parameter.descriptor_type = _shader_descriptor::STORAGE_BUFFER;
            dst_parameter.is_bindless     = true;
            ShaderImporter::parseMemberRecursive(context, type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
            break;

        case _slang_kind::Resource: {
            fe::logging::debug("got resource");
            ShaderImporter::parseMemberRecursive(context, type_layout->getElementTypeLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
        } break;

        case _slang_kind::SamplerState:
            dst_parameter.descriptor_type = _shader_descriptor::SAMPLER;
            ShaderImporter::parseMemberRecursive(context, type_layout->getElementVarLayout(), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&dst_parameter));
            break;

        default:
            fe::logging::error("Slang -> Unified. Failed to reflect resource : Unknown descriptor resource kind : %i", kind);
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

void fe::ShaderImporter::parsePushConstant(ShaderImportContext&                             context,
                                           slang::VariableLayoutReflection*                 variable_layout,
                                           resource::ShaderProgram::ReflectedPushConstants& dst_push_constants) {
    assert(variable_layout);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();

    dst_push_constants.name        = variable_layout->getName();
    dst_push_constants.size        = static_cast<uint32_t>(type_layout->getSize());
    dst_push_constants.stage_flags = static_cast<uint32_t>(_shader_type::NONE);

    SlangStage stage = variable_layout->getStage();

    dst_push_constants.stage_flags =
        ((stage & SLANG_STAGE_VERTEX) ? static_cast<uint32_t>(_shader_type::VERTEX) : 0) |
        ((stage & SLANG_STAGE_FRAGMENT) ? static_cast<uint32_t>(_shader_type::FRAGMENT) : 0) |
        ((stage & SLANG_STAGE_COMPUTE) ? static_cast<uint32_t>(_shader_type::COMPUTE) : 0) |
        ((stage & SLANG_STAGE_GEOMETRY) ? static_cast<uint32_t>(_shader_type::GEOMETRY) : 0);

    uint32_t field_count = type_layout->getFieldCount();
    for (uint32_t i = 0; i < field_count; i++) {
        //parseMemberRecursive(type_layout->getFieldByIndex(i), dst_push_constants.members);
    }

    if (dst_push_constants.size == 0 && !dst_push_constants.members.empty()) {
        auto& last              = dst_push_constants.members.back();
        dst_push_constants.size = last.offset + last.size;
    }
}

void fe::ShaderImporter::parseMemberRecursive(ShaderImportContext&                        context,
                                              slang::VariableLayoutReflection*            variable_layout,
                                              resource::ShaderProgram::ReflectedDataNode* dst_reflected_data_node) {
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

    ShaderImporter::parseMemberRecursive(context, type_layout, dst_reflected_data_node);
}

void fe::ShaderImporter::parseMemberRecursive(ShaderImportContext&                        context,
                                              slang::TypeLayoutReflection*                type_layout,
                                              resource::ShaderProgram::ReflectedDataNode* dst_reflected_data_node) {
    assert(type_layout);
    assert(dst_reflected_data_node);

    dst_reflected_data_node->array_size = 1;
    dst_reflected_data_node->size       = type_layout->getSize();

    auto parse_recursive = [&context](slang::TypeLayoutReflection*                           type_layout,
                                      std::vector<resource::ShaderProgram::ReflectedMember>& dst_members) {
        uint32_t field_count = type_layout->getFieldCount();
        dst_members.reserve(field_count);
        for (uint32_t i = 0; i < field_count; i++) {
            resource::ShaderProgram::ReflectedMember& member = dst_members.emplace_back();
            parseMemberRecursive(context, type_layout->getFieldByIndex(i), static_cast<resource::ShaderProgram::ReflectedDataNode*>(&member));
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

            fe::logging::debug("%s", type_layout->getName());
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

void fe::ShaderImporter::parseTypeLayout(ShaderImportContext&                         context,
                                         slang::VariableLayoutReflection*             variable_layout,
                                         resource::ShaderProgram::ReflectedParameter& dst_parameter) {
    assert(variable_layout);

    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();

    _slang_kind kind = type_layout->getKind();

    switch (kind) {
        case _slang_kind::ConstantBuffer:
        case _slang_kind::ParameterBlock:
            dst_parameter.descriptor_type = _shader_descriptor::UNIFORM_BUFFER;
            break;

        case _slang_kind::ShaderStorageBuffer:
            dst_parameter.descriptor_type = _shader_descriptor::STORAGE_BUFFER;
            break;

        case _slang_kind::Array:
            dst_parameter.descriptor_type = _shader_descriptor::STORAGE_BUFFER;
            dst_parameter.is_bindless     = true;
            break;

        case _slang_kind::Resource: {
            fe::logging::debug("got resource");
        } break;

        case _slang_kind::SamplerState:
            dst_parameter.descriptor_type = _shader_descriptor::SAMPLER;
            break;

        default:
            fe::logging::error("Slang -> Unified. Failed to reflect resource : Unknown descriptor resource kind : %i", kind);
            dst_parameter.descriptor_type = _shader_descriptor::UNKNOWN;
            break;
    }

    ShaderImporter::parseElementTypeLayout(context, type_layout->getElementTypeLayout(), dst_parameter);
}

void fe::ShaderImporter::parseElementTypeLayout(ShaderImportContext&                         context,
                                                slang::TypeLayoutReflection*                 element_type_layout,
                                                resource::ShaderProgram::ReflectedParameter& dst_parameter) {
    assert(element_type_layout);

    // if size is zero, then it is bindless
    dst_parameter.is_bindless = (element_type_layout->getSize() == 0);

    auto& member      = dst_parameter.members.emplace_back();
    member.name       = element_type_layout->getName();
    member.offset     = 0;
    member.array_size = 1;
    member.size       = static_cast<uint32_t>(element_type_layout->getSize());

    _slang_kind kind = element_type_layout->getKind();

    auto parse_recursive = [](slang::TypeLayoutReflection*                           layout,
                              std::vector<resource::ShaderProgram::ReflectedMember>& dst_members) {
        uint32_t field_count = layout->getFieldCount();
        for (uint32_t i = 0; i < field_count; i++) {
            //parseMemberRecursive(layout->getFieldByIndex(i), dst_members);
        }
    };

    switch (kind) {
        case _slang_kind::Struct:
            member.type = _shader_value::STRUCT;
            parse_recursive(element_type_layout, member.members);
            break;

        case _slang_kind::Array: {
            //member.type = _shader_value::STRUCT;

            //slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();

            //if (element_type_layout) {
            //    if (element_type_layout->getKind() == _slang_kind::Struct) {
            //        parse_recursive(element_type_layout, member.members);
            //    }
            //    else {
            //        ShaderImporter::mapScalar(element_type_layout, member.type);
            //    }
            //}
        } break;

        case _slang_kind::Matrix:
            ShaderImporter::mapMatrix(element_type_layout, member.type);
            break;
        case _slang_kind::Vector:
            ShaderImporter::mapVector(element_type_layout, member.type);
            break;
        case _slang_kind::Scalar:
            ShaderImporter::mapScalar(element_type_layout, member.type);
            break;

        case _slang_kind::Resource:
            member.type = _shader_value::UINT32;
            break;

        case _slang_kind::SamplerState:
            member.type = _shader_value::UINT64;
            break;

        case _slang_kind::Pointer:
            member.type = _shader_value::UINT_PTR;
            break;

        case _slang_kind::Enum:
            ShaderImporter::mapScalar(element_type_layout, member.type);
            if (member.type == _shader_value::UNKNOWN) member.type = _shader_value::INT32;
            break;

        case _slang_kind::ConstantBuffer:
        case _slang_kind::ShaderStorageBuffer:
        case _slang_kind::ParameterBlock: {
            fe::logging::debug("got ConstantBuffer or ShaderStorageBuffer or ParameterBlock");

            //member.type                                      = _shader_value::STRUCT;
            //slang::TypeLayoutReflection* element_type_layout = element_type_layout->getElementTypeLayout();
            //if (element_type_layout) parse_recursive(element_type_layout, member.members);
        } break;

        default:
            //fe::logging::warning("Slang -> Unified. Unhandled member kind %i for '%s'. Setting as UNKNOWN", kind, member.name.c_str());
            //member.type = _shader_value::UNKNOWN;
            break;
    }
}

//void fe::ShaderImporter::parseMemberRecursive(slang::VariableLayoutReflection*                       variable_layout,
//                                              std::vector<resource::ShaderProgram::ReflectedMember>& dst_members) {
//    assert(variable_layout);
//
//    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
//
//    auto& member       = dst_members.emplace_back();
//    member.name        = variable_layout->getName();
//    member.offset      = static_cast<uint32_t>(variable_layout->getOffset());
//    member.array_count = 1;
//    member.size        = static_cast<uint32_t>(type_layout->getSize());
//
//    _slang_kind kind = type_layout->getKind();
//
//    auto parse_recursive = [](slang::TypeLayoutReflection*                           layout,
//                              std::vector<resource::ShaderProgram::ReflectedMember>& dst_members) {
//        uint32_t field_count = layout->getFieldCount();
//        for (uint32_t i = 0; i < field_count; i++) {
//            //parseMemberRecursive(layout->getFieldByIndex(i), dst_members);
//        }
//    };
//
//    switch (kind) {
//        case _slang_kind::Struct:
//            member.type = _shader_value::STRUCT;
//            parse_recursive(type_layout, member.members);
//            break;
//
//        case _slang_kind::Array: {
//            member.type = _shader_value::STRUCT;
//
//            slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
//
//            if (element_type_layout) {
//                if (element_type_layout->getKind() == _slang_kind::Struct) {
//                    parse_recursive(element_type_layout, member.members);
//                }
//                else {
//                    ShaderImporter::mapScalar(element_type_layout, member.type);
//                }
//            }
//        } break;
//
//        case _slang_kind::Matrix:
//            ShaderImporter::mapMatrix(type_layout, member.type);
//            break;
//        case _slang_kind::Vector:
//            ShaderImporter::mapVector(type_layout, member.type);
//            break;
//        case _slang_kind::Scalar:
//            ShaderImporter::mapScalar(type_layout, member.type);
//            break;
//
//        case _slang_kind::Resource:
//            member.type = _shader_value::UINT32;
//            break;
//
//        case _slang_kind::SamplerState:
//            member.type = _shader_value::UINT64;
//            break;
//
//        case _slang_kind::Pointer:
//            member.type = _shader_value::UINT_PTR;
//            break;
//
//        case _slang_kind::Enum:
//            ShaderImporter::mapScalar(type_layout, member.type);
//            if (member.type == _shader_value::UNKNOWN) member.type = _shader_value::INT32;
//            break;
//
//        case _slang_kind::ConstantBuffer:
//        case _slang_kind::ShaderStorageBuffer:
//        case _slang_kind::ParameterBlock: {
//            member.type                                      = _shader_value::STRUCT;
//            slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
//            if (element_type_layout) parse_recursive(element_type_layout, member.members);
//        } break;
//
//        default:
//            fe::logging::warning("Slang -> Unified. Unhandled member kind %i for '%s'. Setting as UNKNOWN", kind, member.name.c_str());
//            member.type = _shader_value::UNKNOWN;
//            break;
//    }
//}

void fe::ShaderImporter::setupDescriptorType(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::DescriptorType& dst_descriptor_type) {
    _slang_kind kind = type_layout->getKind();

    switch (kind) {
        case _slang_kind::ConstantBuffer:
        case _slang_kind::ParameterBlock:
            dst_descriptor_type = _shader_descriptor::UNIFORM_BUFFER;
            break;

        case _slang_kind::ShaderStorageBuffer:
            dst_descriptor_type = _shader_descriptor::STORAGE_BUFFER;
            break;

        case _slang_kind::Resource: {
            SlangResourceShape shape      = type_layout->getResourceShape();
            unsigned int       shape_base = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

            if (shape_base == SLANG_TEXTURE_2D) {
                dst_descriptor_type = _shader_descriptor::COMBINED_IMAGE_SAMPLER;
            }
            else if (shape_base == SLANG_STRUCTURED_BUFFER) {
                dst_descriptor_type = _shader_descriptor::STORAGE_BUFFER;
            }
            else {
                dst_descriptor_type = _shader_descriptor::SAMPLED_IMAGE;
            }
        } break;

        case _slang_kind::SamplerState:
            dst_descriptor_type = _shader_descriptor::SAMPLER;
            break;

        default:
            fe::logging::error("Slang -> Unified. Failed to reflect resource : Unknown descriptor resource kind : %i", kind);
            dst_descriptor_type = _shader_descriptor::UNKNOWN;
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
            fe::logging::error("Slang -> Unified. Failed to reflect resource : got slang::TypeReflection::ScalarType::None.\nSetting type as fe::resource::ShaderProgram::ValueType::UNKNOWN");
            type = _shader_value::UNKNOWN;
            break;
        default:
            fe::logging::error("Slang -> Unified. Failed to reflect resource : got unknown slang::TypeReflection::ScalarType.\nSetting type as fe::resource::ShaderProgram::ValueType::UNKNOWN");
            type = _shader_value::UNKNOWN;
            break;
    }
    // clang-format on
}
