/*===============================================

    Forr Engine

    File : string.hpp
    Role : string hasher and fixed-size string

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <string_view>
#include <array>
#include <algorithm>
#include "attributes.hpp"

namespace fe {
    using StringHash = std::uint32_t;

    FORR_NODISCARD constexpr StringHash const_hash(std::string_view str) noexcept {
        std::uint32_t hash = 2166136261u;
        for (const char c : str) {
            hash ^= static_cast<std::uint32_t>(static_cast<unsigned char>(c));
            hash *= 16777619u;
        }
        return hash;
    }

    template <std::size_t N>
    struct fixed_string {
        static_assert(N > 0, "String size must be at least 1 for the null-terminator.");
        
        std::array<char, N> data{};

        constexpr fixed_string(const char (&str)[N]) noexcept {
            std::copy_n(str, N, data.begin());
        }

        constexpr fixed_string(std::string_view str) noexcept {
            const std::size_t copy_size = std::min(N - 1, str.size());
            std::copy_n(str.data(), copy_size, data.begin());
            data[copy_size] = '\0';
        }

        FORR_NODISCARD constexpr operator std::string_view() const noexcept {
            return { data.data(), size() };
        }

        FORR_NODISCARD constexpr std::size_t size() const noexcept { return N - 1; }
        FORR_NODISCARD constexpr bool empty() const noexcept { return size() == 0; }
        
        FORR_NODISCARD constexpr const char* c_str() const noexcept { return data.data(); }
        FORR_NODISCARD constexpr const char* data_ptr() const noexcept { return data.data(); }

        template <std::size_t M>
        FORR_NODISCARD constexpr bool operator==(const fixed_string<M>& other) const noexcept {
            return std::string_view(*this) == std::string_view(other);
        }
    };

    template <std::size_t N>
    fixed_string(const char (&)[N]) -> fixed_string<N>;

} // namespace fe
