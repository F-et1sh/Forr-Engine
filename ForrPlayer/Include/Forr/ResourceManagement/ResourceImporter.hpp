/*===============================================

    Forr Engine

    File : ResourceImporter.hpp
    Role : imports resources and their metadata

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceStorage.hpp"

namespace fe {
    class ResourceImporter {
    public:
        ResourceImporter(ResourceStorage& storage) : m_Storage(storage) {}
        ~ResourceImporter() = default;

    private:
        ResourceStorage& m_Storage;
    };
} // namespace fe
