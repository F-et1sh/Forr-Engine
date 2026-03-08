/*===============================================

    Forr Engine

    File : Model.hpp
    Role : Unified model, that loads gLTF format

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <string>
#include <variant>
#include "Core/pointer.hpp"
#include "GPUTypes.hpp"

namespace fe {
    //struct Mesh {
    //    std::string            name{};
    //    std::vector<Primitive> primitives{};
    //    std::vector<double>    weights{}; // weights to be applied to the Morph Targets

    //    Mesh()  = default;
    //    ~Mesh() = default;
    //};

    //struct AnimationChannel {
    //    int        sampler{ -1 };
    //    int        target_node{ -1 };
    //    TargetPath target_path{};

    //    AnimationChannel()  = default;
    //    ~AnimationChannel() = default;
    //};

    //struct AnimationSampler {
    //    enum class InterpolationMode {
    //        LINEAR,
    //        STEP,
    //        CUBICSPLINE
    //    };

    //    std::vector<float>     times{};
    //    std::vector<glm::vec4> values{}; // rotation as quat, translation/scale as vec3
    //    InterpolationMode      interpolation{ InterpolationMode::LINEAR };

    //    AnimationSampler()  = default;
    //    ~AnimationSampler() = default;
    //};

    //struct Animation {
    //    std::vector<AnimationChannel> channels{};
    //    std::vector<AnimationSampler> samplers{};

    //    Animation()  = default;
    //    ~Animation() = default;
    //};

    //struct Node {
    //    int camera{ -1 };
    //    int skin{ -1 };
    //    int mesh{ -1 };
    //    int light{ -1 };
    //    int emitter{ -1 };

    //    std::string      name{};
    //    std::vector<int> children{};

    //    glm::quat rotation{ 1, 0, 0, 0 }; // order : xyzw
    //    glm::vec3 scale{ 1, 1, 1 };
    //    glm::vec3 translation{ 0, 0, 0 };

    //    glm::mat4 local_matrix{ 1.0f };
    //    glm::mat4 global_matrix{ 1.0f };

    //    std::vector<double> weights{};

    //    Node()  = default;
    //    ~Node() = default;
    //};

    //struct Skin {
    //    std::string            name{};
    //    std::vector<glm::mat4> inverse_bind_matrices{};
    //    int                    skeleton{ -1 }; // the index of the node used as a skeleton root
    //    std::vector<int>       joints{};       // indices of skeleton nodes

    //    std::vector<glm::mat4> bone_final_matrices{};

    //    Skin()  = default;
    //    ~Skin() = default;
    //};
} // namespace fe
