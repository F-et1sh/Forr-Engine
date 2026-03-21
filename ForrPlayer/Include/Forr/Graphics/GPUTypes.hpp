/*===============================================

    Forr Engine

    File : GPUTypes.hpp
    Role : Unified GPU types for every renderer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/pointer.hpp"

#include <variant>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace fe {
    //#pragma pack(push, 1) // disabled for now
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

    using Index = uint32_t; // convert all to uint32_t ( at least for now )

    struct ShaderData {
        glm::mat4 projection_matrix{};
        glm::mat4 view_matrix{};
        glm::mat4 model_matrices[32];

        ShaderData()  = default;
        ~ShaderData() = default;
    };
    //#pragma pack(pop) // pack(push, 1) // disabled for now

    enum class RenderMode {
        POINTS,
        LINES,
        LINE_LOOP,
        LINE_STRIP,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
    };

    enum class RenderIndexType {
        UNSIGNED_BYTE,
        UNSIGNED_SHORT,
        UNSIGNED_INT,
    };

    using Vertices = std::vector<Vertex>;
    using Indices  = std::vector<Index>;
} // namespace fe
