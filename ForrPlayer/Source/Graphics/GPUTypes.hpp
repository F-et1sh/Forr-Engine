/*===============================================

    Forr Engine

    File : GPUTypes.hpp
    Role : GPU Types for OpenGL

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <glm/glm.hpp>

namespace fe {
#pragma pack(push, 1)
    struct Vertex {
        glm::vec3 position{};
        //glm::vec3    normal;
        //glm::vec2    texture_coord;
        //glm::u16vec4 joints;
        //glm::vec4    weights;
        //glm::vec4    tangent;

        Vertex(const glm::vec3& position)
            : position(position) {}

        Vertex()  = default;
        ~Vertex() = default;
    };

    struct ShaderData {
        glm::mat4 projection_matrix{};
        glm::mat4 model_matrix{};
        glm::mat4 view_matrix{};

        ShaderData() = default;
        ~ShaderData() = default;
	};
#pragma pack(pop)
} // namespace fe
