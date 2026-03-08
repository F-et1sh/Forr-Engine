/*===============================================

    Forr Engine

    File : GPUTypes.hpp
    Role : Unified GPU types for every renderer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <variant>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

//#include "ResourceManagement/Resources.hpp" fuck this C++ - I can't to this

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
#pragma pack(pop) // pack(push, 1)

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

    enum class TargetPath {
        TRANSLATION,
        ROTATION,
        SCALE,
        WEIGHTS
    };

    enum class AlphaMode {
        OPAQUE,
        MASK,
        BLEND
    };

    using Vertices = std::vector<Vertex>;
    using Indices  = void*;

    struct Primitive {
        Vertices vertices{};
        Indices  indices{};

        //fe::pointer<fe::resource::Material> material{ -1 };

        RenderMode mode{ RenderMode::TRIANGLES }; // triangles by default

        RenderIndexType index_type{};
        size_t          index_count{};
        size_t          index_offset{};

        Primitive()  = default;
        ~Primitive() = default;
    };

} // namespace fe
