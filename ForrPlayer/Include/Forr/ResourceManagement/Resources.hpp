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
#include "Core/guid.hpp"

#include "Graphics/GPUTypes.hpp"

// namespace fe::resource:: means that the class is a
//  DOD structure, not a high level resource
namespace fe::resource {
#define FORR_RESOURCE_BODY(T) \
    FORR_CLASS_NONCOPYABLE(T) \
    FORR_CLASS_MOVABLE(T)     \
    GUID guid{}; // for future serialization

    struct FORR_API Texture {
        enum class ColorSpace : std::uint8_t {
            LINEAR,
            SRGB
        };
        enum class InternalFormat : std::uint8_t {
            RGBA8,
            RGB8,
            RG8,
            R8,
            SRGB8_ALPHA8,
            SRGB8
        };
        enum class DataFormat : std::uint8_t {
            RGBA,
            RGB,
            RG,
            RED
        };
        enum class MinFilter : std::uint8_t {
            NEAREST,
            LINEAR,
            NEAREST_MIPMAP_NEAREST,
            LINEAR_MIPMAP_NEAREST,
            NEAREST_MIPMAP_LINEAR,
            LINEAR_MIPMAP_LINEAR
        };
        enum class MagFilter : std::uint8_t {
            NEAREST,
            LINEAR,
        };
        enum class Wrap : std::uint8_t {
            CLAMP_TO_EDGE,
            MIRRORED_REPEAT,
            REPEAT
        };
        enum class Target : std::uint8_t {
            TEXTURE_2D,
            TEXTURE_3D,
            // TODO : add more
        };

        // R    - 1
        // RG   - 2
        // RGB  - 3
        // RGBA - 4
        uint8_t components{};

        MinFilter min_filter{ MinFilter::LINEAR_MIPMAP_LINEAR };
        MagFilter mag_filter{ MagFilter::LINEAR };
        Wrap      wrap_s{ Wrap::REPEAT };
        Wrap      wrap_t{ Wrap::REPEAT };

        InternalFormat internal_format{};
        DataFormat     data_format{};

        Target target{ Target::TEXTURE_2D };

        unsigned int width{};
        unsigned int height{};

        //fe::ArenaMarker offset{}; // TODO : think about using this instead of std::unique_ptr<>
        std::unique_ptr<unsigned char[]> bytes{};
        // size in bytes
        std::size_t size{};

        struct MipData {
            std::size_t  offset{};
            std::size_t  size{};
            unsigned int width{};
            unsigned int height{};
        };
        std::vector<MipData> mip_levels{};

        GPUHandle<Texture> gpu_handle{};

        Texture()  = default;
        ~Texture() = default;

        FORR_RESOURCE_BODY(Texture)
    };

    struct FORR_API ShaderProgram {
    public:
        enum class DescriptorType : std::uint8_t {
            UNIFORM_BUFFER,
            STORAGE_BUFFER,

            SAMPLED_IMAGE,
            SAMPLER,
            COMBINED_IMAGE_SAMPLER,
            STORAGE_IMAGE,

            ACCELERATION_STRUCTURE,

            UNKNOWN
        };

        // clang-format off
        enum class ValueType : std::uint8_t {
            VOID,

            BOOL,

            INT32, UINT32,

            INT64, UINT64,

            FLOAT16, FLOAT32, FLOAT64,

            INT8, UINT8, INT16, UINT16,

            INT_PTR, UINT_PTR,

            FLOAT2, FLOAT3, FLOAT4,

            INT2, INT3, INT4,

            UINT2, UINT3, UINT4,

            MAT3, MAT4,

            STRUCT,

            UNKNOWN
        };
        // clang-format on

        struct ReflectedMember; // forward declaration

        // base struct for reflection
        struct ReflectedDataNode {
            ValueType type{ ValueType::UNKNOWN };

            uint32_t array_size{ 1 };

            uint32_t size{};

            std::vector<ReflectedMember> members{};

            std::string name{};

            ReflectedDataNode() = default;
            ReflectedDataNode(ValueType type, uint32_t array_size, uint32_t size, std::vector<ReflectedMember> members, std::string name)
                : type(type), array_size(array_size), size(size), members(std::move(members)), name(std::move(name)) {}

            bool operator==(const ReflectedDataNode&) const noexcept = default;
        };

        // may be a field of a shader struct
        struct ReflectedMember : public ReflectedDataNode {
            uint32_t offset{};

            bool operator==(const ReflectedMember&) const noexcept = default;
        };

        // entry point : parameter
        struct ReflectedParameter : public ReflectedDataNode {
            DescriptorType descriptor_type{ DescriptorType::UNKNOWN };

            uint32_t set{};
            uint32_t binding{};

            uint8_t stage_flags{};

            bool is_bindless{};

            ReflectedParameter() = default;
            ReflectedParameter(ReflectedDataNode data_node, DescriptorType descriptor_type, uint32_t set, uint32_t binding, uint8_t stage_flags, bool is_bindless)
                : ReflectedDataNode(std::move(data_node)), descriptor_type(descriptor_type), set(set), binding(binding), stage_flags(stage_flags), is_bindless(is_bindless) {}

            bool operator==(const ReflectedParameter&) const noexcept = default;
        };

        // entry point : push constants
        struct ReflectedPushConstants : public ReflectedDataNode {
            uint8_t stage_flags{};

            ReflectedPushConstants() = default;
            ReflectedPushConstants(ReflectedDataNode data_node, uint8_t stage_flags)
                : ReflectedDataNode::ReflectedDataNode(std::move(data_node)), stage_flags(stage_flags) {}
        };

        enum class ShaderType : std::uint8_t {
            NONE     = 0,
            VERTEX   = 1,
            GEOMETRY = 2,
            FRAGMENT = 3,
            COMPUTE  = 4,
        };

        using SourceCode        = std::vector<uint8_t>;
        using SourceCodeStorage = std::unordered_map<ShaderType, SourceCode>;

        GPUHandle<ShaderProgram> gpu_handle{};

        SourceCodeStorage source_codes{};

        std::vector<ReflectedParameter> reflected_parameters{};
        ReflectedPushConstants          reflected_push_constants{};

        ShaderProgram()  = default;
        ~ShaderProgram() = default;

        FORR_RESOURCE_BODY(ShaderProgram)
    };

    struct FORR_API Material {
    public:
        GPUHandle<Material> gpu_handle{};

        fe::pointer<fe::resource::ShaderProgram> shader_program_ptr{};

        struct Sampler {
            std::size_t          offset{};
            fe::pointer<Texture> texture_ptr{};
        };

        // this is needed to assign textures' data to the passing buffer
        // it works like this :
        //
        // buffer : [ data, data, null, data, null ]
        // samplers : [ offset 2, texture_ptr 0, offset 4, texture_ptr 2 ]
        //
        // then, in GPU resource manager, while creating analogue of this material,
        // it takes every sampler from 'samplers', creates its GPU analogue and assigns
        // it to the 'buffer' of this material, according to 'offset' of the sampler
        std::span<Sampler> samplers{};
        // this buffer contains all raw data you pass to the shader
        std::span<uint8_t> buffer{};

        Material()  = default;
        ~Material() = default;

        FORR_RESOURCE_BODY(Material)
    };

    struct FORR_API Model {
        struct FORR_API Mesh {
            struct FORR_API Primitive {
                fe::pointer<Material> material_ptr{};

                RenderMode render_mode{ RenderMode::TRIANGLES }; // triangles by default

                RenderIndexType index_type{ RenderIndexType::UNSIGNED_INT }; // this should be removed
                int             index_count{};
                int             index_offset{};
            };

            GPUHandle<Mesh> gpu_handle{};

            std::string name{};

            Vertices vertices{};
            Indices  indices{};

            std::vector<Primitive> primitives{};
            std::vector<float>     weights{}; // weights to be applied to the Morph Targets
        };

        struct FORR_API AnimationChannel {
            enum class TargetPath {
                TRANSLATION,
                ROTATION,
                SCALE,
                WEIGHTS
            };

            int        sampler{ -1 };
            int        target_node{ -1 };
            TargetPath target_path{}; // "rotation", "translation", "scale"
        };

        struct FORR_API AnimationSampler {
            enum class InterpolationMode {
                LINEAR,
                STEP,
                CUBICSPLINE
            };

            std::vector<float>     times{};
            std::vector<glm::vec4> values{}; // rotation as quat, translation/scale as vec3
            InterpolationMode      interpolation{ InterpolationMode::LINEAR };
        };

        struct FORR_API Animation {
            std::vector<AnimationChannel> channels{};
            std::vector<AnimationSampler> samplers{};
        };

        struct FORR_API Node {
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
        };

        struct FORR_API Skin {
            std::string            name{};
            std::vector<glm::mat4> inverse_bind_matrices{};
            int                    skeleton{ -1 }; // the index of the node used as a skeleton root
            std::vector<int>       joints{};       // indices of skeleton nodes

            std::vector<glm::mat4> bone_final_matrices{};
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

    template <typename T>
    concept resource_t =
        (std::is_same_v<T, Texture>) ||
        (std::is_same_v<T, ShaderProgram>) ||
        (std::is_same_v<T, Material>) ||
        (std::is_same_v<T, Model>);

#undef FORR_RESOURCE_BODY

} // namespace fe::resource
