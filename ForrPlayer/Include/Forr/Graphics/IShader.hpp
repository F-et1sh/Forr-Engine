/*===============================================

    Forr Engine

    File : IShader.hpp
    Role : Shader interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <filesystem>

namespace fe {
    struct FORR_API ShaderDesc {
        std::filesystem::path vertex_relative_path{};
        std::filesystem::path fragment_relative_path{};

        ShaderDesc()  = default;
        ~ShaderDesc() = default;
    };

    class FORR_API IShader {
    public:
        IShader()          = default;
        virtual ~IShader() = default;

        virtual void Initialize(ShaderDesc& desc) = 0;

        virtual void Bind()   = 0;
        virtual void Unbind() = 0;

    private:
    };
} // namespace fe
