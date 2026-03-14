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
        glm::mat4 model_matrix{};
        glm::mat4 view_matrix{};

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

    namespace resource {
        struct Material; // forward declaration
    }

    struct Primitive {
        Vertices vertices{};
        Indices  indices{};

        fe::pointer<fe::resource::Material> material_ptr{};

        RenderMode render_mode{ RenderMode::TRIANGLES }; // triangles by default

        RenderIndexType index_type{};
        int             index_count{}; // TODO : review this. Why you need index_count if you already using std::vector<Index>
        int             index_offset{};

        Primitive()  = default;
        ~Primitive() = default;
    };

    struct Mesh {
        std::string            name{};
        std::vector<Primitive> primitives{};
        std::vector<float>     weights{}; // weights to be applied to the Morph Targets

        Mesh()  = default;
        ~Mesh() = default;
    };

    struct AnimationChannel {
        enum class TargetPath {
            TRANSLATION,
            ROTATION,
            SCALE,
            WEIGHTS
        };

        int        sampler{ -1 };
        int        target_node{ -1 };
        TargetPath target_path{}; // "rotation", "translation", "scale"

        AnimationChannel()  = default;
        ~AnimationChannel() = default;
    };

    struct AnimationSampler {
        enum class InterpolationMode {
            LINEAR,
            STEP,
            CUBICSPLINE
        };

        std::vector<float>     times{};
        std::vector<glm::vec4> values{}; // rotation as quat, translation/scale as vec3
        InterpolationMode      interpolation{ InterpolationMode::LINEAR };

        AnimationSampler()  = default;
        ~AnimationSampler() = default;
    };

    struct Animation {
        std::vector<AnimationChannel> channels{};
        std::vector<AnimationSampler> samplers{};

        Animation()  = default;
        ~Animation() = default;
    };

    struct Node {
        // TODO : I think this shouldn't be just int
        int camera{ -1 };
        int skin{ -1 };
        int mesh{ -1 };
        int light{ -1 };
        int emitter{ -1 };
        //

        std::string      name{};
        std::vector<int> children{};

        glm::quat rotation{ 1, 0, 0, 0 }; // order : xyzw
        glm::vec3 scale{ 1, 1, 1 };
        glm::vec3 translation{ 0, 0, 0 };

        glm::mat4 local_matrix{ 1.0f };
        glm::mat4 global_matrix{ 1.0f };

        std::vector<float> weights{};

        Node()  = default;
        ~Node() = default;
    };

    struct Skin {
        std::string            name{};
        std::vector<glm::mat4> inverse_bind_matrices{};
        int                    skeleton{ -1 }; // the index of the node used as a skeleton root
        std::vector<int>       joints{};       // indices of skeleton nodes

        std::vector<glm::mat4> bone_final_matrices{};

        Skin()  = default;
        ~Skin() = default;
    };

} // namespace fe
