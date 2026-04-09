/*===============================================

    Forr Engine

    File : ShaderCompiler.cpp
    Role : shader compiler

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Graphics/Shaders/ShaderCompiler.hpp"

#include "shaderc/shaderc.hpp"

void fe::ShaderCompiler::Compile(std::vector<uint32_t>& dst, std::string_view src, resource::Shader::Type shader_type, GraphicsBackend graphics_backend) {
    //static shaderc::Compiler compiler{};

    //shaderc::CompileOptions  options{};
    //shaderc_shader_kind      kind{};

    //switch (graphics_backend) {
    //    case GraphicsBackend::OpenGL:

    //        options.AddMacroDefinition("FORR_USE_OPENGL");
    //        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

    //        break;
    //    case GraphicsBackend::Vulkan:

    //        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

    //        break;
    //    default:
    //        fe::logging::warning("The selected graphics backend %i for shader was not found. Using the default one", graphics_backend);

    //        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

    //        break;
    //}

    //options.SetForcedVersionProfile(450, shaderc_profile_core);

    //// clang-format off
    //switch (shader_type) {
    //    case resource::Shader::Type::VERTEX  : kind = shaderc_shader_kind::shaderc_glsl_vertex_shader  ; break;
    //    case resource::Shader::Type::FRAGMENT: kind = shaderc_shader_kind::shaderc_glsl_fragment_shader; break;
    //    default:
    //        fe::logging::warning("The selected shader type %i was not found. Using the default one", shader_type);
    //        kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
    //        break;
    //}
    //// clang-format on

    //auto result = compiler.CompileGlslToSpv(src.data(), src.size(), kind, "shader", options);

    //if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    //    fe::logging::error("Failed to compile a shader\nStage : %i\nMessage : %s", shader_type, result.GetErrorMessage().c_str());
    //    dst.clear();
    //    return;
    //}

    //dst.assign(result.cbegin(), result.cend());
}

