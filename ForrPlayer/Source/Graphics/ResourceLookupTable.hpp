/*===============================================

    Forr Engine

    File : ResourceLookupTable.hpp
    Role : unified lookup table for CPU <-> GPU communication

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe {
    template<typename CPU, typename GPU>
    class ResourceLookupTable {
    public:
        ResourceLookupTable() = default;
        ~ResourceLookupTable() = default;

    private:
        std::unordered_map<uint64_t, uint64_t> m_Table{};
    };
}