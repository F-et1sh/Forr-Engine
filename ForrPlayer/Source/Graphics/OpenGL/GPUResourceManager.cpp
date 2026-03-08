/*===============================================

    Forr Engine

    File : GPUResourceManager.cpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "GPUResourceManager.hpp"

fe::MeshID fe::GPUResourceManager::CreateTriangle() {
    std::filesystem::path path = PATH.getModelsPath() / "StatueOfLiberty" / "statue_of_liberty.glb";

    tinygltf::Model    model{};
    tinygltf::TinyGLTF loader{};
    std::string        error{};
    std::string        warning{};
    std::string        filename = path.string();
    bool               good     = false;

    if (path.extension() == ".gltf") {
        good = loader.LoadASCIIFromFile(&model, &error, &warning, filename);
    }
    else if (path.extension() == ".glb") {
        good = loader.LoadBinaryFromFile(&model, &error, &warning, filename);
    }

    if (!warning.empty())
        fe::logging::warning("%s\nPath : %s", warning.c_str(), filename.c_str());

    if (!error.empty())
        fe::logging::error("%s\nPath : %s", error.c_str(), filename.c_str());

    if (!good)
        fe::logging::error("Failed to load gLTF model\nPath : %s", filename.c_str());

    this->loadMeshes(model);

    ///

    /*std::vector<Vertex> triangle_vertices{
        Vertex{ glm::vec3{ -0.5f, -0.5f, 0.0f } },
        Vertex{ glm::vec3{ 0.0f, 0.5f, 0.0f } },
        Vertex{ glm::vec3{ 0.5f, -0.5f, 0.0f } }
    };

    std::vector<uint8_t> triangle_indices{
        0, 1, 2
    };

    auto& mesh = m_Meshes.emplace_back(triangle_vertices, triangle_indices);

    mesh.vao.bind();

    mesh.vbo.bind();

    constexpr GLsizei stride = sizeof(Vertex);

    mesh.vao.LinkAttrib(mesh.vbo, 0, 3, GL_FLOAT, stride, (void*) offsetof(Vertex, position));

    mesh.ebo.bind();

    mesh.vbo.unbind();
    mesh.vao.unbind();
    mesh.ebo.unbind();

    return m_Meshes.size() - 1;*/

    return 0;
}

void fe::GPUResourceManager::CreateTexture(const resource::Texture& texture) {
}

void fe::GPUResourceManager::loadMeshes(const tinygltf::Model& model) {
    m_Meshes.resize(model.meshes.size());
    for (size_t i = 0; i < model.meshes.size(); i++) {
        const tinygltf::Mesh& mesh      = model.meshes[i];
        auto&                 this_mesh = m_Meshes[i];

        this_mesh.name = mesh.name;
        this->loadPrimitives(model, this_mesh.primitives, mesh.primitives);
        this_mesh.weights = mesh.weights;
    }
}

void fe::GPUResourceManager::loadPrimitives(const tinygltf::Model& model, std::vector<Primitive>& this_primitives, const std::vector<tinygltf::Primitive>& primitives) {
    this_primitives.resize(primitives.size());
    for (size_t i = 0; i < primitives.size(); i++) {
        const tinygltf::Primitive& primitive      = primitives[i];
        auto&                      this_primitive = this_primitives[i];

        loadIndices(model, this_primitive, this_primitive.indices, primitive); // indices go first
        loadVertices(model, this_primitive.vertices, this_primitive.indices, primitive);
        //this_primitive.material = primitive.material;

        this_primitive.vao.Create();
        this_primitive.vao.Bind();

        this_primitive.vbo.Create(this_primitive.vertices);
        this_primitive.vbo.Bind();

        constexpr GLsizei stride = sizeof(Vertex);

        VAO::LinkAttrib(this_primitive.vbo, 0, 3, GL_FLOAT, stride, (void*) offsetof(Vertex, position));
        //VAO::LinkAttrib(this_primitive.vbo, 1, 3, GL_FLOAT, stride, (void*) offsetof(Vertex, normal));
        //VAO::LinkAttrib(this_primitive.vbo, 2, 2, GL_FLOAT, stride, (void*) offsetof(Vertex, texture_coord));

        this_primitive.vbo.Bind();
        //glVertexAttribIPointer(3, 4, GL_UNSIGNED_SHORT, stride, (void*) offsetof(Vertex, joints));
        //glEnableVertexAttribArray(3);

        //VAO::LinkAttrib(this_primitive.vbo, 4, 4, GL_FLOAT, stride, (void*) offsetof(Vertex, weights));
        //VAO::LinkAttrib(this_primitive.vbo, 5, 4, GL_FLOAT, stride, (void*) offsetof(Vertex, tangent));

        this_primitive.ebo.Create(this_primitive.indices);
        this_primitive.ebo.Bind();

        VBO::Unbind();
        VAO::Unbind();
        EBO::Unbind();

        switch (primitive.mode) {
            case TINYGLTF_MODE_POINTS:
                this_primitive.mode = RenderMode::POINTS;
                break;
            case TINYGLTF_MODE_LINE:
                this_primitive.mode = RenderMode::LINES;
                break;
            case TINYGLTF_MODE_LINE_LOOP:
                this_primitive.mode = RenderMode::LINE_LOOP;
                break;
            case TINYGLTF_MODE_LINE_STRIP:
                this_primitive.mode = RenderMode::LINE_STRIP;
                break;
            case TINYGLTF_MODE_TRIANGLES:
                this_primitive.mode = RenderMode::TRIANGLES;
                break;
            case TINYGLTF_MODE_TRIANGLE_STRIP:
                this_primitive.mode = RenderMode::TRIANGLE_STRIP;
                break;
            case TINYGLTF_MODE_TRIANGLE_FAN:
                this_primitive.mode = RenderMode::TRIANGLE_FAN;
                break;
            default:
                assert(false);
                break;
        }
    }
}

void fe::GPUResourceManager::loadVertices(const tinygltf::Model& model, Vertices& this_vertices, Indices& this_indices, const tinygltf::Primitive& primitive) {

    auto read_attribute = [&](const std::string& attribute_name, auto& data) {
        auto it = primitive.attributes.find(attribute_name);
        if (it == primitive.attributes.end()) {
            data.clear();
            return;
        }
        this->readAttribute(model, it->second, data);
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

        std::vector<uint32_t> indices_u32{};

        if (!std::holds_alternative<std::vector<uint32_t>>(this_indices)) {
            std::visit([&](auto const& indices) {
                indices_u32.reserve(indices.size());
                for (auto v : indices) {
                    indices_u32.emplace_back(static_cast<uint32_t>(v));
                }
            },
                       this_indices);
        }
        else {
            indices_u32 = std::get<std::vector<uint32_t>>(this_indices);
        }

        if (positions.empty() || normals.empty() || texture_coords.empty() || indices_u32.empty()) {
            fe::logging::warning("Skip tangent generation : missing data"); // TODO : rewrite
        }
        else {
            for (uint32_t idx : indices_u32) {
                if (idx >= positions.size() || idx >= normals.size() || idx >= texture_coords.size()) {
                    fe::logging::fatal("bad index %i", idx);
                }
            }

            //MikkUserData userdata{
            //    .positions      = &positions,
            //    .normals        = &normals,
            //    .texture_coords = &texture_coords,
            //    .tangents       = &tangents,
            //    .indices        = &indices_u32
            //};
            //
            //SMikkTSpaceInterface iface   = {};
            //iface.m_getNumFaces          = getNumberFaces;
            //iface.m_getNumVerticesOfFace = getNumberVerticesOfFace;
            //iface.m_getPosition          = getPosition;
            //iface.m_getNormal            = getNormal;
            //iface.m_getTexCoord          = getTextureCoord;
            //iface.m_setTSpaceBasic       = setTSpaceBasic;
            //
            //SMikkTSpaceContext context = {};
            //context.m_pInterface       = &iface;
            //context.m_pUserData        = &userdata;
            //
            //genTangSpaceDefault(&context);
        }
    }

    for (size_t i = 0; i < vertices_count; i++) {
        auto& v    = this_vertices[i];
        v.position = positions[i];
        if (i < normals.size()) {
            //v.normal = normals[i];
        }
        if (i < tangents.size()) {
            //v.tangent = tangents[i];
        }
        if (i < texture_coords.size()) {
            //v.texture_coord = texture_coords[i];
        }
        if (i < joints.size()) {
            //v.joints = joints[i];
        }
        if (i < weights.size()) {
            //v.weights = weights[i];
        }
    }
}

void fe::GPUResourceManager::loadIndices(const tinygltf::Model& model, Primitive& this_primitive, Indices& this_indices, const tinygltf::Primitive& primitive) {
    if (primitive.indices < 0) {
        throw std::runtime_error("Primitive has no indices"); // TODO : rewrite
    }

    const tinygltf::Accessor&   accessor    = model.accessors[primitive.indices];
    const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer&     buffer      = model.buffers[buffer_view.buffer];
    const uint8_t*              data_ptr    = buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;

    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            this_indices = std::vector<uint8_t>{};
            auto& vec    = std::get<std::vector<uint8_t>>(this_indices);
            vec.resize(accessor.count);
            memcpy(vec.data(), data_ptr, accessor.count * sizeof(uint8_t));

            this_primitive.index_type   = RenderIndexType::UNSIGNED_BYTE;
            this_primitive.index_count  = accessor.count;
            this_primitive.index_offset = 0;

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            this_indices = std::vector<uint16_t>{};
            auto& vec    = std::get<std::vector<uint16_t>>(this_indices);
            vec.resize(accessor.count);
            if (buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint16_t)) { // tightly packed
                memcpy(vec.data(), data_ptr, accessor.count * sizeof(uint16_t));
            }
            else {
                for (size_t i = 0; i < accessor.count; i++) {
                    vec[i] = *reinterpret_cast<const uint16_t*>(data_ptr + (i * buffer_view.byteStride));
                }
            }

            this_primitive.index_type   = RenderIndexType::UNSIGNED_SHORT;
            this_primitive.index_count  = accessor.count;
            this_primitive.index_offset = 0;

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
            this_indices = std::vector<uint32_t>{};
            auto& vec    = std::get<std::vector<uint32_t>>(this_indices);
            vec.resize(accessor.count);
            if (buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint32_t)) { // tightly packed
                memcpy(vec.data(), data_ptr, accessor.count * sizeof(uint32_t));
            }
            else {
                for (size_t i = 0; i < accessor.count; i++) {
                    vec[i] = *reinterpret_cast<const uint32_t*>(data_ptr + (i * buffer_view.byteStride));
                }
            }

            this_primitive.index_type   = RenderIndexType::UNSIGNED_INT;
            this_primitive.index_count  = accessor.count;
            this_primitive.index_offset = 0;

            break;
        }
        default:
            throw std::runtime_error("Unsupported index component type"); // TODO : rewrite
            break;
    }
}
