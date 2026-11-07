/*===============================================

    Forr Engine

    File : Camera.hpp
    Role : Abstract camera

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Forr/Core/attributes.hpp"
#include "Forr/Graphics/GPUTypes.hpp"

#pragma once

namespace fe {
    class FORR_API Camera {
    public:
    private:
        glm::mat4 m_Projection{};
    };
} // namespace fe
