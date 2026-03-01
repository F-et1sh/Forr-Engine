/*===============================================

    Forr Engine

    File : Model.hpp
    Role : Unified model, that loads gLTF format

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe {
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
using Indices  = std::variant<
     std::vector<uint8_t>,
     std::vector<uint16_t>,
     std::vector<uint32_t>>;
}