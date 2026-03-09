/*===============================================

    Forr Engine

    File : MikkTSpace.hpp
    Role : A common standard for tangent space used in baking tools to produce normal maps. More information can be found at http://www.mikktspace.com/.

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "mikktspace.h"

struct MikkUserData {
    std::vector<glm::vec3>* positions;
    std::vector<glm::vec3>* normals;
    std::vector<glm::vec2>* texture_coords;
    std::vector<glm::vec4>* tangents; // output
    std::vector<uint32_t>*  indices;
};

int getNumberFaces(const SMikkTSpaceContext* p_context) {
    auto* data = (MikkUserData*) p_context->m_pUserData;
    return data->indices->size() / 3;
}

int getNumberVerticesOfFace(const SMikkTSpaceContext* /*unused*/, int /*unused*/) {
    return 3;
}

void getPosition(const SMikkTSpaceContext* p_context, float position[3], int face, int vertex) {
    auto*    data  = (MikkUserData*) p_context->m_pUserData;
    uint32_t index = (*data->indices)[(face * 3) + vertex];
    auto&    p     = (*data->positions)[index];
    position[0]    = p.x;
    position[1]    = p.y;
    position[2]    = p.z;
}

void getNormal(const SMikkTSpaceContext* p_context, float normal[3], int face, int vertex) {
    auto*    data  = (MikkUserData*) p_context->m_pUserData;
    uint32_t index = (*data->indices)[(face * 3) + vertex];
    auto&    p     = (*data->normals)[index];
    normal[0]      = p.x;
    normal[1]      = p.y;
    normal[2]      = p.z;
}

void getTextureCoord(const SMikkTSpaceContext* p_context, float texture_coord[2], int face, int vertex) {
    auto*    data    = (MikkUserData*) p_context->m_pUserData;
    uint32_t index   = (*data->indices)[(face * 3) + vertex];
    auto&    p       = (*data->texture_coords)[index];
    texture_coord[0] = p.x;
    texture_coord[1] = p.y;
}

void setTSpaceBasic(const SMikkTSpaceContext* p_context,
                    const float               tangent[3],
                    const float               sign,
                    const int                 face,
                    const int                 vertex) {
    auto*    data            = (MikkUserData*) p_context->m_pUserData;
    uint32_t index           = (*data->indices)[(face * 3) + vertex];
    (*data->tangents)[index] = glm::vec4(tangent[0], tangent[1], tangent[2], sign);
}
