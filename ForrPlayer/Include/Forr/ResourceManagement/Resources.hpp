/*===============================================

    Forr Engine

    File : Resources.hpp
    Role : Contain all resource structures.
        Resource Management system uses data-oriented design.
        namespace fe::resource:: means that the class is a DOD structure.
        Resource structure - means that the structure has its own extension, like .png or .gltf.
        If the resource has a metadata file near, a part of the structure will be filled from it.
        For example, if .png ( example.png ) hasn't data about "min filter", so, the member
            fe::resource::Texture::min_filter will be set to default, but if the resource manager find
            a metadata file near ( example.png.fs ) it will fill the structure from it.

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include "Core/types.hpp"

#include "Graphics/GPUTypes.hpp"

// namespace fe::resource:: means that the class is a
//  DOD structure, not a high level resource
namespace fe::resource {
#define FORR_RESOURCE_BODY(T) \
    FORR_CLASS_NONCOPYABLE(T) \
    FORR_CLASS_MOVABLE(T)

    struct Texture {
        enum class ColorSpace {
            LINEAR,
            SRGB
        };

        enum class InternalFormat {
            RGBA8,
            RGB8,
            RG8,
            R8,
            SRGB8_ALPHA8,
            SRGB8
        };

        enum class DataFormat {
            RGBA,
            RGB,
            RG,
            RED
        };

        enum class MinFilter {
            NEAREST,
            LINEAR,
            NEAREST_MIPMAP_NEAREST,
            LINEAR_MIPMAP_NEAREST,
            NEAREST_MIPMAP_LINEAR,
            LINEAR_MIPMAP_LINEAR
        };

        enum class MagFilter {
            NEAREST,
            LINEAR,
        };

        enum class Wrap {
            CLAMP_TO_EDGE,
            MIRRORED_REPEAT,
            REPEAT
        };

        enum class Target {
            TEXTURE_2D,
            TEXTURE_3D,
            // TODO : add more
        };

        uint8_t      components{};
        unsigned int width{};
        unsigned int height{};

        MinFilter min_filter{ MinFilter::LINEAR_MIPMAP_LINEAR };
        MagFilter mag_filter{ MagFilter::LINEAR };
        Wrap      wrap_s{ Wrap::REPEAT };
        Wrap      wrap_t{ Wrap::REPEAT };

        InternalFormat internal_format{};
        DataFormat     data_format{};

        Target target{ Target::TEXTURE_2D };

        std::unique_ptr<unsigned char[]> bytes{};
        //fe::ArenaMarker offset{}; // TODO : think about using this instead of std::unique_ptr<>

        Texture()  = default;
        ~Texture() = default;

        FORR_RESOURCE_BODY(Texture)
    };

    struct Material {
        enum class AlphaMode {
            OPAQUE,
            MASK,
            BLEND
        };

        struct TextureInfo {
            fe::pointer<Texture> texture_ptr{};
            int                  texture_coord{ 0 }; // TEXCOORD_0 by default

            TextureInfo()          = default;
            virtual ~TextureInfo() = default;
        };

        struct NormalTextureInfo : public TextureInfo {
            float scale{ 1.0f };

            NormalTextureInfo()  = default;
            ~NormalTextureInfo() = default;
        };

        struct OcclusionTextureInfo : public TextureInfo {
            float strength{ 1.0f };

            OcclusionTextureInfo()  = default;
            ~OcclusionTextureInfo() = default;
        };

        struct PbrMetallicRoughness {
            glm::vec4   base_color_factor{ 1.0f, 1.0f, 1.0f, 1.0f }; // default [1,1,1,1]
            TextureInfo base_color_texture{};
            float       metallic_factor{ 1.0f };  // default 1
            float       roughness_factor{ 1.0f }; // default 1
            TextureInfo metallic_roughness_texture{};

            PbrMetallicRoughness()  = default;
            ~PbrMetallicRoughness() = default;
        };

        std::string name{};

        glm::vec3        emissive_factor{ 0.0f, 0.0f, 0.0f }; // default [0,0,0]
        AlphaMode        alpha_mode{ AlphaMode::OPAQUE };     // default - OPAQUE
        float            alpha_cutoff{ 0.5f };                // default 0.5f
        bool             double_sided{ false };               // default false
        std::vector<int> lods{};                              // level of detail materials ( MSFT_lod )

        PbrMetallicRoughness pbr_metallic_roughness{};

        NormalTextureInfo    normal_texture{};
        OcclusionTextureInfo occlusion_texture{};
        TextureInfo          emissive_texture{};

        Material()  = default;
        ~Material() = default;

        FORR_RESOURCE_BODY(Material)
    };

    struct Model {
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

        std::vector<Node>      nodes{};
        std::vector<int>       scene_roots{};
        std::vector<Skin>      skins{};
        std::vector<Mesh>      meshes{};
        std::vector<Animation> animations{};

        Model()  = default;
        ~Model() = default;

        FORR_RESOURCE_BODY(Model)
    };

#undef FORR_RESOURCE_BODY

} // namespace fe::resource
