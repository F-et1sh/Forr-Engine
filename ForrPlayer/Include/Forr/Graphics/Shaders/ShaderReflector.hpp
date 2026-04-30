/*===============================================

    Forr Engine

    File : ShaderReflector.hpp
    Role : creates resource::Shader's layout part

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/Resources.hpp"

namespace fe {
    class ShaderReflector {
    public:
        ShaderReflector()  = default;
        ~ShaderReflector() = default;

        static void Reflect(resource::Shader& shader, const std::filesystem::path& resource_full_path);

    private:
    };
} // namespace fe
