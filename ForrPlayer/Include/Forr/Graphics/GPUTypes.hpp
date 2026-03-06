/*===============================================

    Forr Engine

    File : GPUTypes.hpp
    Role : Unified GPU types for every renderer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

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

        ShaderData()  = default;
        ~ShaderData() = default;
    };
#pragma pack(pop)
} // namespace fe
