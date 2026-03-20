/*===============================================

    Forr Engine

    File : VulkanResourceCreator.cpp
    Role : creates Vulkan resources from unified ones

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "VulkanResourceCreator.hpp"

fe::pointer<fe::VulkanTexture> fe::VulkanResourceCreator::CreateTexture(const resource::Texture& texture) {
    VulkanTexture this_texture{};

    

    auto ptr = m_Storage.m_Textures.create(this_texture);
    return ptr;
}
