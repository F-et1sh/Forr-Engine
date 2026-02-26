/*===============================================

    Forr Engine

    File : VKStructures.hpp
    Role : RAII structures

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "VKTypes.hpp"

namespace fe {
    struct Image { // temp
        fe::vk::Image        image;
        fe::vk::DeviceMemory memory;
        fe::vk::ImageView    image_view;

        Image()  = default;
        ~Image() = default;
    };
} // namespace fe
