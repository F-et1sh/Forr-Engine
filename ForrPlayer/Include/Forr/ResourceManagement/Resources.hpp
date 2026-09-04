/*===============================================

    Forr Engine

    File : Resources.hpp
    Role : Contain all resource structures.
        Resource Management system uses data-oriented design.
        namespace fe::resource:: means that the class is a DOD structure.
        Resource structure - means that the structure has its own extension, like .png or .gltf.
        If the resource has a metadata file near, a part of the structure will be filled from it.
        For example, if .png ( example.png ) hasn't data about "min filter", so, the member
            fe::resource::Texture::min_filter will be set to default, but if the resource manager finds
            a metadata file near ( example.png.fs ) it will fill the structure from it.

        // TODO : this rule might be removed. Better create one resource and store its 'sub resources' inside
        Sometimes one file ( extension ) can create multiple resources. For example, ShaderProgram and
            MaterialLayout are both created from .slang file - if it happends, you have to create a 
            structure, that will point to all of resources created by that 
            file ( ShaderFileData in the case of .slang )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include <span>
#include "Core/types.hpp"
#include "Core/guid.hpp"

#include "Graphics/GPUTypes.hpp"

// namespace fe::resource:: means that the class is a
//  DOD structure, not a high level resource
namespace fe::resource {
#define FORR_RESOURCE_BODY(T) \
    FORR_CLASS_NONCOPYABLE(T) \
    FORR_CLASS_MOVABLE(T)     \
    GUID guid{}; // for future serialization. TODO : move this to ResourceManager

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

        //fe::ArenaMarker offset{}; // TODO : think about using this instead of std::unique_ptr<> | TODO : std::span<> instead
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

    /* forward declarations */
    struct ShaderFileData;
    struct DescriptorsLayout;

    struct FORR_API ShaderProgram {
    public:
        using DescriptorsLayoutPtr = fe::pointer<resource::DescriptorsLayout>;
        // unspecialized type name -> ( optinal ) type name that specializes
        using SpecializePair = std::pair<fe::hashed_string, std::optional<fe::hashed_string>>;

        fe::pointer<ShaderFileData>         shader_file_data_ptr{};
        std::optional<DescriptorsLayoutPtr> descriptors_layout_ptr{};

        // shader stage -> { specialized ( or unspecialized ) pairs }
        std::unordered_map<shader::StageBits, std::vector<SpecializePair>> specialized_types{};

        //VertexLayout vertex_layout{};

        ShaderProgram()  = default;
        ~ShaderProgram() = default;

        FORR_RESOURCE_BODY(ShaderProgram)
    };

    struct FORR_API DescriptorsLayout {
    public:
        fe::pointer<ShaderFileData>        shader_file_data_ptr{};
        shader::ReflectedDescriptorsLayout reflected_layout{};

        DescriptorsLayout() = default;
        DescriptorsLayout(shader::ReflectedDescriptorsLayout reflected_layout, fe::pointer<ShaderFileData> shader_file_data_ptr)
            : reflected_layout(std::move(reflected_layout)), shader_file_data_ptr(shader_file_data_ptr) {}
        ~DescriptorsLayout() = default;

        FORR_RESOURCE_BODY(DescriptorsLayout)
    };

    // TODO : maybe rename this to 'StructureLayout'
    struct FORR_API MaterialLayout {
        fe::pointer<ShaderFileData>      shader_file_data_ptr{};
        shader::ReflectedStructureLayout reflected_layout{};

        MaterialLayout() = default;
        MaterialLayout(shader::ReflectedStructureLayout reflected_layout, fe::pointer<ShaderFileData> shader_file_data_ptr)
            : reflected_layout(std::move(reflected_layout)), shader_file_data_ptr(shader_file_data_ptr) {}
        ~MaterialLayout() = default;

        FORR_RESOURCE_BODY(MaterialLayout)
    };

    struct FORR_API Material {
    public:
        // TODO : remove this
        fe::pointer<fe::resource::MaterialLayout> layout_ptr{};

        struct MaterialLayoutKey {
            fe::pointer<resource::ShaderFileData> shader_file_data{};
            size_t                                structure_layout_storage_index{};
        };

        MaterialLayoutKey layout_key{};

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
        // both of this 'std::span's are stored in 'fe::ResourceStorage' like the structure they're in
        std::span<Sampler> samplers{};
        // this buffer contains all raw data you pass to the shader
        // both of this 'std::span's are stored in 'fe::ResourceStorage' like the structure they're in
        std::span<std::byte> buffer{};

        // basically, render pass sets pipeline flags, but if there is a material - it can override them
        PipelineFlags pipeline_flags_override{};

        GPUHandle<Material> gpu_handle{};

        Material()  = default;
        ~Material() = default;

        FORR_RESOURCE_BODY(Material)
    };

    // this is a basic structure, created by 'fe::ShaderImporter', while importing a file
    struct FORR_API ShaderFileData {
    public:
        // initial resources
        std::vector<shader::ReflectedDescriptor>      descriptor_layouts{};
        std::vector<shader::ReflectedPushConstants>   push_constants_layouts{};
        std::vector<shader::ReflectedStructureLayout> structure_layouts{};
        std::vector<shader::ReflectedEntryPoint>      entry_points{};

        // created resources
        struct FORR_API ShaderProgram {
            fe::pointer<resource::ShaderFileData> shader_file_data_ptr{};

            std::vector<size_t> descriptor_layout_indices{};
            std::vector<size_t> push_constants_layout_indices{};
            std::vector<size_t> entry_point_indices{};

            GPUHandle<ShaderProgram> gpu_handle{};
        };

        std::vector<ShaderProgram> shader_programs{};

        // this is needed to not accses disk twice
        std::vector<uint8_t> slang_serialized_data{}; // serialized module
        // this is used as an unique index in 'slang::ISession::loadModuleFromIRBlob()'
        std::string full_path{};

        ShaderFileData()  = default;
        ~ShaderFileData() = default;

        FORR_RESOURCE_BODY(ShaderFileData)
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

#define FORR_RESOURCES_LIST(X) \
    X(Texture)                 \
    X(ShaderProgram)           \
    X(DescriptorsLayout)       \
    X(MaterialLayout)          \
    X(Material)                \
    X(ShaderFileData)          \
    X(Model)

#define GENERATE_CONCEPT(RESOURCE_NAME) (std::is_same_v<T, RESOURCE_NAME>) ||
    template <typename T>
    concept resource_t = FORR_RESOURCES_LIST(GENERATE_CONCEPT) false;
#undef GENERATE_CONCEPT

#undef FORR_RESOURCE_BODY

} // namespace fe::resource
