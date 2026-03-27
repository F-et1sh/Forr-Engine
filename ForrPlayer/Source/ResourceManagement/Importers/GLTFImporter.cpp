/*===============================================

    Forr Engine

    File : GLTFImporter.cpp
    Role : imports resources and their metadata. for gltf and glb

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "GLTFImporter.hpp"

#include "MikkTSpace.hpp"

void fe::GLTFImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    tinygltf::Model    model{};
    tinygltf::TinyGLTF loader{};
    std::string        error{};
    std::string        warning{};
    std::string        filename = resource_full_path.string();
    bool               good     = false;

    if (resource_full_path.extension() == ".gltf") {
        good = loader.LoadASCIIFromFile(&model, &error, &warning, filename);
    }
    else if (resource_full_path.extension() == ".glb") {
        good = loader.LoadBinaryFromFile(&model, &error, &warning, filename);
    }
    else {
        fe::logging::error("Failed to load GLTF model.\nWrong resource extension. It's not .gltf or .glb\nPath : %s", filename.c_str());
        return;
    }

    if (!warning.empty()) {
        fe::logging::warning(warning.c_str());
    }
    if (!error.empty()) {
        fe::logging::error("Failed to load GLTF model.\nGot an error : %s", error.c_str());
        return;
    }
    if (!good) {
        fe::logging::error("Failed to load GLTF model.\n\"good\" value is false : %s", error.c_str());
        return;
    }

    resource::Model this_model{};

    GLTFImportContext context{ model, this_model, storage };

    GLTFImporter::loadNodes(context);
    GLTFImporter::loadSceneRoots(context);
    GLTFImporter::loadSkins(context);
    GLTFImporter::loadTextures(context);
    GLTFImporter::loadMaterials(context);
    GLTFImporter::loadMeshes(context);
    GLTFImporter::loadAnimations(context);

    auto ptr = storage.CreateResource<resource::Model>(std::move(this_model)); // does not need to store this
}

void fe::GLTFImporter::loadNodes(GLTFImportContext& context) {
    context.this_model.nodes.resize(context.model.nodes.size());
    for (size_t i = 0; i < context.model.nodes.size(); i++) {
        const tinygltf::Node& node      = context.model.nodes[i];
        auto&                 this_node = context.this_model.nodes[i];

        this_node.camera   = node.camera;
        this_node.name     = node.name;
        this_node.skin     = node.skin;
        this_node.mesh     = node.mesh;
        this_node.light    = node.light;
        this_node.emitter  = node.emitter;
        this_node.children = node.children;
        fe::GLTFImporter::readVector(this_node.rotation, node.rotation);
        fe::GLTFImporter::readVector(this_node.scale, node.scale);
        fe::GLTFImporter::readVector(this_node.translation, node.translation);
        if (!node.matrix.empty()) {
            glm::mat4 m(1.0f);
            for (int i = 0; i < 16; i++) {
                m[i / 4][i % 4] = node.matrix[i];
            }
            this_node.local_matrix = m;
        }
        else {
            this_node.local_matrix = glm::translate(glm::mat4(1.0f), this_node.translation) * glm::mat4_cast(this_node.rotation) * glm::scale(glm::mat4(1.0f), this_node.scale);
        }
        this_node.weights.insert_range(this_node.weights.end(), node.weights);
    }
}

void fe::GLTFImporter::loadSceneRoots(GLTFImportContext& context) {
    int scene_index = context.model.defaultScene;

    if (scene_index < 0 && !context.model.scenes.empty()) {
        scene_index = 0;
    }
    if (scene_index >= 0) {
        context.this_model.scene_roots = context.model.scenes[scene_index].nodes;
    }
}

void fe::GLTFImporter::loadSkins(GLTFImportContext& context) {
    context.this_model.skins.resize(context.model.skins.size());
    for (size_t i = 0; i < context.model.skins.size(); i++) {
        const tinygltf::Skin& skin      = context.model.skins[i];
        auto&                 this_skin = context.this_model.skins[i];

        this_skin.name = skin.name;

        size_t accessor_index = skin.inverseBindMatrices;
        GLTFImporter::readAttribute(context.model, accessor_index, this_skin.inverse_bind_matrices);

        this_skin.skeleton = skin.skeleton;
        this_skin.joints   = skin.joints;
        this_skin.bone_final_matrices.resize(JOINTS_COUNT);
    }
}

void fe::GLTFImporter::loadMeshes(GLTFImportContext& context) {
    context.this_model.meshes.resize(context.model.meshes.size());
    for (size_t i = 0; i < context.model.meshes.size(); i++) {
        const tinygltf::Mesh& mesh      = context.model.meshes[i];
        auto&                 this_mesh = context.this_model.meshes[i];

        this_mesh.name = mesh.name;

        context.mesh_primitive_offset_index = {}; // reset this before loading new mesh

        const std::vector<tinygltf::Primitive>&        primitives      = mesh.primitives;
        std::vector<resource::Model::Mesh::Primitive>& this_primitives = this_mesh.primitives;

        this_primitives.resize(primitives.size());

        for (size_t i = 0; i < primitives.size(); i++) {
            const tinygltf::Primitive& primitive      = primitives[i];
            auto&                      this_primitive = this_primitives[i];

            this_primitive.material_ptr = context.GetMaterial(i);

            GLTFImporter::loadIndices(context, this_primitive, this_mesh.indices, primitive); // indices go first
            GLTFImporter::loadVertices(context, this_mesh.vertices, this_mesh.indices, primitive);

            // clang-format off
            switch (primitive.mode) {
                case TINYGLTF_MODE_POINTS        : this_primitive.render_mode = RenderMode::POINTS        ; break;
                case TINYGLTF_MODE_LINE          : this_primitive.render_mode = RenderMode::LINES         ; break;
                case TINYGLTF_MODE_LINE_LOOP     : this_primitive.render_mode = RenderMode::LINE_LOOP     ; break;
                case TINYGLTF_MODE_LINE_STRIP    : this_primitive.render_mode = RenderMode::LINE_STRIP    ; break;
                case TINYGLTF_MODE_TRIANGLES     : this_primitive.render_mode = RenderMode::TRIANGLES     ; break;
                case TINYGLTF_MODE_TRIANGLE_STRIP: this_primitive.render_mode = RenderMode::TRIANGLE_STRIP; break;
                case TINYGLTF_MODE_TRIANGLE_FAN  : this_primitive.render_mode = RenderMode::TRIANGLE_FAN  ; break;
                default:
                    fe::logging::warning("tinygltf -> Unified. Unsupported render mode %i. Using TRIANGLES as default", primitive.mode);
            }
            // clang-format on
        }

        this_mesh.weights.insert_range(this_mesh.weights.end(), mesh.weights);
    }
}

void fe::GLTFImporter::loadTextures(GLTFImportContext& context) {
    context.textures.resize(context.model.textures.size());

    for (size_t i = 0; i < context.model.textures.size(); i++) {
        context.textures[i] = createTexture(context.model, i, context.storage);
    }
}

void fe::GLTFImporter::loadMaterials(GLTFImportContext& context) {
    context.materials.resize(context.model.materials.size());

    for (size_t i = 0; i < context.model.materials.size(); i++) {
        context.materials[i] = GLTFImporter::createMaterial(context, i);
    }
}

void fe::GLTFImporter::loadVertices(GLTFImportContext& context, Vertices& this_vertices, Indices& this_indices, const tinygltf::Primitive& primitive) {

    auto read_attribute = [&](const std::string& attribute_name, auto& data) {
        auto it = primitive.attributes.find(attribute_name);
        if (it == primitive.attributes.end()) {
            data.clear();
            return;
        }
        GLTFImporter::readAttribute(context.model, it->second, data);
    };

    std::vector<glm::vec3> positions{};
    read_attribute("POSITION", positions);

    std::vector<glm::vec3> normals{};
    read_attribute("NORMAL", normals);

    std::vector<glm::vec4> tangents{};
    read_attribute("TANGENT", tangents);

    std::vector<glm::vec2> texture_coords{};
    read_attribute("TEXCOORD_0", texture_coords);

    std::vector<glm::u16vec4> joints{};
    read_attribute("JOINTS_0", joints);

    std::vector<glm::uvec4> weights{};
    read_attribute("WEIGHTS_0", weights);

    size_t vertices_count = positions.size();
    this_vertices.resize(vertices_count);

    if (tangents.empty()) {
        tangents.resize(vertices_count);

        if (positions.empty() || normals.empty() || texture_coords.empty() || this_indices.empty()) {
            fe::logging::warning("Skipping tangent generation. Needed data is missing");
        }
        else {
            bool good = true;

            for (uint32_t i : this_indices) {
                if (i >= positions.size() || i >= normals.size() || i >= texture_coords.size()) {
                    fe::logging::warning("Skipping tangent generation. Bad index : %i", i);
                    good = false;
                }
            }

            if (good) {
                MikkUserData user_data{
                    .positions      = &positions,
                    .normals        = &normals,
                    .texture_coords = &texture_coords,
                    .tangents       = &tangents,
                    .indices        = &this_indices
                };

                SMikkTSpaceInterface interface{};
                interface.m_getNumFaces          = getNumberFaces;
                interface.m_getNumVerticesOfFace = getNumberVerticesOfFace;
                interface.m_getPosition          = getPosition;
                interface.m_getNormal            = getNormal;
                interface.m_getTexCoord          = getTextureCoord;
                interface.m_setTSpaceBasic       = setTSpaceBasic;

                SMikkTSpaceContext context{};
                context.m_pInterface = &interface;
                context.m_pUserData  = &user_data;

                genTangSpaceDefault(&context);
            }
        }
    }

    for (size_t i = 0; i < vertices_count; i++) {
        auto& vertex    = this_vertices[i];
        vertex.position = positions[i];

        // TODO : support this

        //if (i < normals.size()) {
        //    v.normal = normals[i];
        //}
        //if (i < tangents.size()) {
        //    v.tangent = tangents[i];
        //}
        //if (i < texture_coords.size()) {
        //    v.texture_coord = texture_coords[i];
        //}
        //if (i < joints.size()) {
        //    v.joints = joints[i];
        //}
        //if (i < weights.size()) {
        //    v.weights = weights[i];
        //}
    }
}

void fe::GLTFImporter::loadIndices(GLTFImportContext& context, resource::Model::Mesh::Primitive& this_primitive, Indices& this_indices, const tinygltf::Primitive& primitive) {
    if (primitive.indices < 0) {
        fe::logging::error("Primitive has no indices");
        return;
    }

    const tinygltf::Accessor&   accessor    = context.model.accessors[primitive.indices];
    const tinygltf::BufferView& buffer_view = context.model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer&     buffer      = context.model.buffers[buffer_view.buffer];
    const uint8_t*              data_ptr    = buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;

    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            std::vector<uint8_t> vec{};
            vec.resize(accessor.count);
            memcpy(vec.data(), data_ptr, accessor.count * sizeof(uint8_t));

            this_indices.insert_range(this_indices.end(), vec);

            this_primitive.index_type   = RenderIndexType::UNSIGNED_INT; // TODO : maybe remove this ?
            this_primitive.index_count  = accessor.count;
            this_primitive.index_offset = context.mesh_primitive_offset_index;

            context.mesh_primitive_offset_index += accessor.count;

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            std::vector<uint16_t> vec{};
            vec.resize(accessor.count);
            if (buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint16_t)) { // tightly packed
                memcpy(vec.data(), data_ptr, accessor.count * sizeof(uint16_t));
            }
            else {
                for (size_t i = 0; i < accessor.count; i++) {
                    vec[i] = *reinterpret_cast<const uint16_t*>(data_ptr + (i * buffer_view.byteStride));
                }
            }

            this_indices.insert_range(this_indices.end(), vec);

            this_primitive.index_type   = RenderIndexType::UNSIGNED_INT; // TODO : maybe remove this ?
            this_primitive.index_count  = accessor.count;
            this_primitive.index_offset = context.mesh_primitive_offset_index;

            context.mesh_primitive_offset_index += accessor.count;

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
            std::vector<uint32_t> vec{};
            vec.resize(accessor.count);
            if (buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint32_t)) { // tightly packed
                memcpy(vec.data(), data_ptr, accessor.count * sizeof(uint32_t));
            }
            else {
                for (size_t i = 0; i < accessor.count; i++) {
                    vec[i] = *reinterpret_cast<const uint32_t*>(data_ptr + (i * buffer_view.byteStride));
                }
            }

            this_indices.insert_range(this_indices.end(), vec);

            this_primitive.index_type   = RenderIndexType::UNSIGNED_INT; // TODO : maybe remove this ?
            this_primitive.index_count  = accessor.count;
            this_primitive.index_offset = context.mesh_primitive_offset_index;

            context.mesh_primitive_offset_index += accessor.count;

            break;
        }
        default:
            fe::logging::error("tinygltf -> Unified. Unsupported index accessor's component type %i", accessor.componentType);
    }
}

using namespace fe::resource;

void fe::GLTFImporter::loadAnimations(GLTFImportContext& context) {
    context.this_model.animations.resize(context.model.animations.size());
    for (size_t i = 0; i < context.model.animations.size(); i++) {
        const tinygltf::Animation& animation      = context.model.animations[i];
        auto&                      this_animation = context.this_model.animations[i];

        this_animation.channels.resize(animation.channels.size());

        for (size_t j = 0; j < animation.channels.size(); j++) {
            const tinygltf::AnimationChannel& channel      = animation.channels[j];
            auto&                             this_channel = this_animation.channels[j];

            this_channel.sampler     = channel.sampler;
            this_channel.target_node = channel.target_node;

            if (channel.target_path == "translation") {
                this_channel.target_path = Model::AnimationChannel::TargetPath::TRANSLATION;
            }
            else if (channel.target_path == "rotation") {
                this_channel.target_path = Model::AnimationChannel::TargetPath::ROTATION;
            }
            else if (channel.target_path == "scale") {
                this_channel.target_path = Model::AnimationChannel::TargetPath::SCALE;
            }
            else if (channel.target_path == "weights") {
                this_channel.target_path = Model::AnimationChannel::TargetPath::WEIGHTS;
            }
        }

        this_animation.samplers.resize(animation.samplers.size());

        for (size_t j = 0; j < animation.samplers.size(); j++) {
            const tinygltf::AnimationSampler& sampler      = animation.samplers[j];
            auto&                             this_sampler = this_animation.samplers[j];

            fe::GLTFImporter::readAccessorFloat(context.model, sampler.input, this_sampler.times);
            fe::GLTFImporter::readAccessorVec4(context.model, sampler.output, this_sampler.values);

            if (sampler.interpolation == "LINEAR") {
                this_sampler.interpolation = Model::AnimationSampler::InterpolationMode::LINEAR;
            }
            else if (sampler.interpolation == "STEP") {
                this_sampler.interpolation = Model::AnimationSampler::InterpolationMode::STEP;
            }
            else if (sampler.interpolation == "CUBICSPLINE") {
                this_sampler.interpolation = Model::AnimationSampler::InterpolationMode::CUBICSPLINE;
            }
        }
    }
}

fe::pointer<Texture> fe::GLTFImporter::createTexture(const tinygltf::Model& model, uint32_t texture_index, ResourceStorage& storage) {
    if (texture_index >= model.textures.size()) {
        fe::logging::warning("tinygltf -> Unified. Failed to create a texture. Texture index : %i\nFallbacks are not ready yet", texture_index);
        return {}; // TODO : think about fallbacks
    }

    const tinygltf::Texture& texture = model.textures[texture_index];
    const tinygltf::Image&   image   = model.images[texture.source];
    tinygltf::Sampler        sampler{};

    resource::Texture::ColorSpace texture_color_space = resource::Texture::ColorSpace::LINEAR;

    if (texture.sampler >= 0) {
        sampler = model.samplers[texture.sampler];
    }
    else { // default settings
        sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
        sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
        sampler.wrapS     = TINYGLTF_TEXTURE_WRAP_REPEAT;
        sampler.wrapT     = TINYGLTF_TEXTURE_WRAP_REPEAT;
    }

    Texture this_texture{};

    if (texture_color_space == Texture::ColorSpace::SRGB) {
        if (image.component == 4) // number of color channels
            this_texture.internal_format = Texture::InternalFormat::SRGB8_ALPHA8;
        else
            this_texture.internal_format = Texture::InternalFormat::SRGB8;
    }
    else {
        // clang-format off
        switch (image.component) { // number of color channels
            case 4: this_texture.internal_format = Texture::InternalFormat::RGBA8; break;
            case 3: this_texture.internal_format = Texture::InternalFormat::RGB8 ; break;
            case 2: this_texture.internal_format = Texture::InternalFormat::RG8  ; break;
            case 1: this_texture.internal_format = Texture::InternalFormat::R8   ; break;
            default:
                fe::logging::warning("tinygltf -> Unified. Unsupported components ( number of color channels ) %i for internal format. Using RGBA8 as default", image.component);
                this_texture.internal_format = Texture::InternalFormat::RGBA8;
        }
        // clang-format on
    }

    // clang-format off
    switch (image.component) { // number of color channels
        case 4: this_texture.data_format = Texture::DataFormat::RGBA; break;
        case 3: this_texture.data_format = Texture::DataFormat::RGB ; break;
        case 2: this_texture.data_format = Texture::DataFormat::RG  ; break;
        case 1: this_texture.data_format = Texture::DataFormat::RED ; break;
        default:
            fe::logging::warning("tinygltf -> Unified. Unsupported components ( number of color channels ) %i for data format. Using RGBA as default", image.component);
            this_texture.data_format = Texture::DataFormat::RGBA;
    }
    // clang-format on

    // clang-format off
    switch (sampler.minFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST               : this_texture.min_filter = Texture::MinFilter::NEAREST               ; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR                : this_texture.min_filter = Texture::MinFilter::LINEAR                ; break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: this_texture.min_filter = Texture::MinFilter::NEAREST_MIPMAP_NEAREST; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST : this_texture.min_filter = Texture::MinFilter::LINEAR_MIPMAP_NEAREST ; break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR : this_texture.min_filter = Texture::MinFilter::NEAREST_MIPMAP_LINEAR ; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR  : this_texture.min_filter = Texture::MinFilter::LINEAR_MIPMAP_LINEAR  ; break;
        default:
            this_texture.min_filter = Texture::MinFilter::LINEAR_MIPMAP_LINEAR;
            fe::logging::warning("tinygltf -> Unified. Unsupported min filter %i. Using LINEAR_MIPMAP_LINEAR as default", sampler.minFilter);
    }
    // clang-format on

    // clang-format off
    switch (sampler.magFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST: this_texture.mag_filter = Texture::MagFilter::NEAREST; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR : this_texture.mag_filter = Texture::MagFilter::LINEAR ; break;
        default:
            this_texture.mag_filter = Texture::MagFilter::LINEAR;
            fe::logging::warning("tinygltf -> Unified. Unsupported mag filter %i. Using LINEAR as default", sampler.magFilter);
    }
    // clang-format on

    // clang-format off
    switch (sampler.wrapS) {
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE  : this_texture.wrap_s = Texture::Wrap::CLAMP_TO_EDGE  ; break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: this_texture.wrap_s = Texture::Wrap::MIRRORED_REPEAT; break;
        case TINYGLTF_TEXTURE_WRAP_REPEAT         : this_texture.wrap_s = Texture::Wrap::REPEAT         ; break;
        default:
            this_texture.wrap_s = Texture::Wrap::REPEAT;
            fe::logging::warning("tinygltf -> Unified. Unsupported wrap s %i. Using REPEAT as default", sampler.wrapS);
    }

    // clang-format off
    switch (sampler.wrapT) {
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE  : this_texture.wrap_t = Texture::Wrap::CLAMP_TO_EDGE  ; break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: this_texture.wrap_t = Texture::Wrap::MIRRORED_REPEAT; break;
        case TINYGLTF_TEXTURE_WRAP_REPEAT         : this_texture.wrap_t = Texture::Wrap::REPEAT         ; break;
        default:
            this_texture.wrap_t = Texture::Wrap::REPEAT;
            fe::logging::warning("tinygltf -> Unified. Unsupported wrap t %i. Using REPEAT as default", sampler.wrapT);
    }
    // clang-format on

    this_texture.components = static_cast<uint8_t>(image.component);
    this_texture.width      = image.width;
    this_texture.height     = image.height;
    this_texture.target     = Texture::Target::TEXTURE_2D; // TODO : for what this thing even needed ?

    size_t buffer_size = image.width * image.height * image.component;
    this_texture.bytes = std::make_unique<unsigned char[]>(buffer_size);

    if (!image.image.empty() && buffer_size == image.image.size()) {
        std::copy(image.image.begin(), image.image.end(), this_texture.bytes.get());
    }
    else {
        fe::logging::warning("tinygltf -> Unified. Failed to create a texture\nURI : %s", image.uri.c_str());
        return {}; // TODO : think about fallbacks
    }

    auto ptr = storage.CreateResource<Texture>(std::move(this_texture));

    // TODO : think about - Is this really so much needed ?
    // Texture::ColorSpace::SRGB <-> Texture::ColorSpace::LINEAR
    //
    //for (const auto& mesh : this_model.meshes) {
    //    for (const auto& primitive : mesh.primitives) {
    //        const auto& material = *storage.GetResource(primitive.material_ptr);
    //
    //        if (material.pbr_metallic_roughness.base_color_texture.texture_ptr == ptr ||
    //            material.emissive_texture.texture_ptr == ptr) {
    //
    //            texture_color_space = Texture::ColorSpace::SRGB;
    //
    //            if (texture_color_space == Texture::ColorSpace::SRGB) {
    //                if (image.component == 4) // number of color channels
    //                    this_texture.internal_format = Texture::InternalFormat::SRGB8_ALPHA8;
    //                else
    //                    this_texture.internal_format = Texture::InternalFormat::SRGB8;
    //            }
    //            else {
    //                // clang-format off
    //                switch (image.component) { // number of color channels
    //                    case 4: this_texture.internal_format = Texture::InternalFormat::RGBA8; break;
    //                    case 3: this_texture.internal_format = Texture::InternalFormat::RGB8 ; break;
    //                    case 2: this_texture.internal_format = Texture::InternalFormat::RG8  ; break;
    //                    case 1: this_texture.internal_format = Texture::InternalFormat::R8   ; break;
    //                    default:
    //                        fe::logging::warning("tinygltf -> Unified. Unsupported components ( number of color channels ) %i for internal format. Using RGBA8 as default", image.component);
    //                        this_texture.internal_format = Texture::InternalFormat::RGBA8;
    //                }
    //                // clang-format on
    //            }
    //        }
    //    }
    //}

    return ptr;
}

#undef OPAQUE

fe::pointer<Material> fe::GLTFImporter::createMaterial(GLTFImportContext& context, uint32_t tinygltf_material_index) {
    const tinygltf::Material& material = context.model.materials[tinygltf_material_index];
    Material                  this_material{};

//    this_material.name = material.name;
//
//    fe::GLTFImporter::readVector(this_material.emissive_factor, material.emissiveFactor);
//
//    // clang-format off
//    if      (material.alphaMode == "OPAQUE") this_material.alpha_mode = Material::AlphaMode::OPAQUE;
//    else if (material.alphaMode == "MASK"  ) this_material.alpha_mode = Material::AlphaMode::MASK;
//    else if (material.alphaMode == "BLEND" ) this_material.alpha_mode = Material::AlphaMode::BLEND;
//    else {
//        fe::logging::warning("tinygltf -> Unified. Unsupported material alpha mode %s. Using BLEND as default", material.alphaMode.c_str());
//        this_material.alpha_mode = Material::AlphaMode::BLEND;
//    }
//    // clang-format on
//
//    this_material.alpha_cutoff = material.alphaCutoff;
//    this_material.double_sided = material.doubleSided;
//    this_material.lods         = material.lods;
//
//#define THIS_PBR this_material.pbr_metallic_roughness
//#define PBR material.pbrMetallicRoughness
//
//    fe::GLTFImporter::readVector(THIS_PBR.base_color_factor, PBR.baseColorFactor);
//    THIS_PBR.base_color_texture.texture_ptr   = context.GetTexture(PBR.baseColorTexture.index);
//    THIS_PBR.base_color_texture.texture_coord = PBR.baseColorTexture.texCoord;
//
//    THIS_PBR.metallic_factor  = PBR.metallicFactor;
//    THIS_PBR.roughness_factor = PBR.roughnessFactor;
//
//    THIS_PBR.metallic_roughness_texture.texture_ptr   = context.GetTexture(PBR.metallicRoughnessTexture.index);
//    THIS_PBR.metallic_roughness_texture.texture_coord = PBR.metallicRoughnessTexture.texCoord;
//
//    this_material.normal_texture.texture_ptr   = context.GetTexture(material.normalTexture.index);
//    this_material.normal_texture.texture_coord = material.normalTexture.texCoord;
//    this_material.normal_texture.scale         = material.normalTexture.scale;
//
//    this_material.occlusion_texture.texture_ptr   = context.GetTexture(material.occlusionTexture.index);
//    this_material.occlusion_texture.texture_coord = material.occlusionTexture.texCoord;
//    this_material.occlusion_texture.strength      = material.occlusionTexture.strength;
//
//    this_material.emissive_texture.texture_ptr   = context.GetTexture(material.emissiveTexture.index);
//    this_material.emissive_texture.texture_coord = material.emissiveTexture.texCoord;

    auto ptr = context.storage.CreateResource<Material>(std::move(this_material));
    return ptr;
}

void fe::GLTFImporter::readVector(glm::vec2& dst, const std::vector<double>& src) {
    if (src.size() != 2) return;
    dst = glm::vec2(static_cast<float>(src[0]), static_cast<float>(src[1]));
}

void fe::GLTFImporter::readVector(glm::vec3& dst, const std::vector<double>& src) {
    if (src.size() != 3) return;
    dst = glm::vec3(static_cast<float>(src[0]), static_cast<float>(src[1]), static_cast<float>(src[2]));
}

void fe::GLTFImporter::readVector(glm::vec4& dst, const std::vector<double>& src) {
    if (src.size() != 4) return;
    dst = glm::vec4(static_cast<float>(src[0]), static_cast<float>(src[1]), static_cast<float>(src[2]), static_cast<float>(src[3]));
}

void fe::GLTFImporter::readVector(glm::quat& dst, const std::vector<double>& src) {
    if (src.size() != 4) return;
    dst = glm::quat(static_cast<float>(src[0]), static_cast<float>(src[1]), static_cast<float>(src[2]), static_cast<float>(src[3]));
}

FORR_NODISCARD float fe::GLTFImporter::readComponentAsFloat(const uint8_t* data, int component_type, bool normalized) {
    switch (component_type) {
        case TINYGLTF_COMPONENT_TYPE_FLOAT: {
            return *reinterpret_cast<const float*>(data);
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            uint16_t v = *reinterpret_cast<const uint16_t*>(data);
            return normalized ? (float) v / 65535.0f : (float) v;
        }
        case TINYGLTF_COMPONENT_TYPE_SHORT: {
            int16_t v = *reinterpret_cast<const int16_t*>(data);
            return normalized ? glm::clamp((float) v / 32767.0f, -1.0f, 1.0f) : (float) v;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            uint8_t v = *data;
            return normalized ? (float) v / 255.0f : (float) v;
        }
        case TINYGLTF_COMPONENT_TYPE_BYTE: {
            int8_t v = *reinterpret_cast<const int8_t*>(data);
            return normalized ? glm::clamp((float) v / 127.0f, -1.0f, 1.0f) : (float) v;
        }
        default:
            fe::logging::error("tinygltf -> Unified. Unsupported component type %i", component_type);
            return 0.0f;
    }
}

void fe::GLTFImporter::readAccessorVec4(const tinygltf::Model& model, int accessor_index, std::vector<glm::vec4>& out) {
    const auto& accessor    = model.accessors[accessor_index];
    const auto& buffer_view = model.bufferViews[accessor.bufferView];
    const auto& buffer      = model.buffers[buffer_view.buffer];

    const uint8_t* data = buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;

    int    num_components{};
    size_t component_size{};

    // clang-format off
    switch (accessor.type) {
        case TINYGLTF_TYPE_SCALAR: num_components = 1; break;
        case TINYGLTF_TYPE_VEC2  : num_components = 2; break;
        case TINYGLTF_TYPE_VEC3  : num_components = 3; break;
        case TINYGLTF_TYPE_VEC4  : num_components = 4; break;
        default:
            num_components = 0;
            fe::logging::error("tinygltf -> Unified. Unsupported accessor type %i", accessor.type);
            return;
    }
    // clang-format on

    // clang-format off
    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_FLOAT         : component_size = 4; break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: component_size = 2; break;
        case TINYGLTF_COMPONENT_TYPE_SHORT         : component_size = 2; break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE : component_size = 1; break;
        case TINYGLTF_COMPONENT_TYPE_BYTE          : component_size = 1; break;
        default:
            component_size = 0;
            fe::logging::error("tinygltf -> Unified. Unsupported component type %i", accessor.componentType);
            return;
    }
    // clang-format on

    size_t stride = (buffer_view.byteStride != 0U) ? buffer_view.byteStride : num_components * component_size;

    out.resize(accessor.count);

    for (size_t i = 0; i < accessor.count; i++) {

        glm::vec4 v(0.0f);

        const uint8_t* element = data + (i * stride);

        for (int component = 0; component < num_components; component++) {
            v[component] = readComponentAsFloat(
                element + (component * component_size),
                accessor.componentType,
                accessor.normalized);
        }

        out[i] = v;
    }
}

void fe::GLTFImporter::readAccessorFloat(const tinygltf::Model& model, int accessor_index, std::vector<float>& out) {
    std::vector<glm::vec4> temp{};
    readAccessorVec4(model, accessor_index, temp);

    out.resize(temp.size());
    for (size_t i = 0; i < temp.size(); i++) {
        out[i] = temp[i].x;
    }
}
