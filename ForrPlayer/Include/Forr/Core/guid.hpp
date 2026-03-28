/*===============================================

    Forr Engine

    File : guid.hpp
    Role : GUID

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <random>
#include <string>
#include <sstream>
#include "attributes.hpp"

namespace fe {
    struct FORR_NODISCARD GUID {
    public:
        uint64_t high{};
        uint64_t low{};

        bool operator==(const GUID&) const = default;

        FORR_NODISCARD static GUID generate() {
            static std::random_device rd;
            static std::mt19937_64    eng(rd());

            return GUID{ eng(), eng() };
        }

        FORR_NODISCARD static GUID from_string(const std::string& str) {
            GUID guid{};

            auto dash = str.find('-');
            if (dash == std::string::npos)
                throw std::runtime_error("Invalid GUID format");

            std::string high_str = str.substr(0, dash);
            std::string low_str  = str.substr(dash + 1);

            guid.high = std::stoull(high_str, nullptr, 16);
            guid.low  = std::stoull(low_str, nullptr, 16);

            return guid;
        }

        FORR_NODISCARD std::string to_string() const {
            std::stringstream ss;
            ss << std::hex << high << "-" << low;
            return ss.str();
        }

        GUID() = default;
        GUID(uint64_t hight, uint64_t low) : high(high), low(low) {}
        ~GUID() = default;
    };
} // namespace fe

namespace std {
    template <>
    struct hash<fe::GUID> {
        std::size_t operator()(const fe::GUID& guid) const noexcept {
            size_t h1 = std::hash<uint64_t>{}(guid.high);
            size_t h2 = std::hash<uint64_t>{}(guid.low);

            return h1 ^ (h2 << 1);
        }
    };
} // namespace std
