/*===============================================

    Forr Engine

    File : string.hpp
    Role : string hasher, 
           class that contaits std::string and its hash, mostly for GUI - hashed_string,
           fixed-size string and
           fixed hashed_string

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <string_view>
#include <array>
#include <algorithm>
#include "attributes.hpp"

namespace fe {
    using StringHash = uint64_t;

    FORR_NODISCARD constexpr StringHash string_hash(std::string_view str) noexcept {
        std::uint64_t hash = 14695981039346656037ULL;
        for (const char c : str) {
            hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(c));
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    class hashed_string { // mostly for GUI
    public:
        hashed_string()  = default;
        ~hashed_string() = default;

        hashed_string(std::string string) : string(std::move(string)), hash(fe::string_hash(string)) {}

        FORR_NODISCARD operator std::string_view() const noexcept { return { string.data(), size() }; }
        FORR_NODISCARD operator std::string() const noexcept { return string; }
        FORR_NODISCARD operator fe::StringHash() const noexcept { return hash; }

        FORR_NODISCARD std::size_t size() const noexcept { return string.size(); }
        FORR_NODISCARD bool        empty() const noexcept { return size() == 0; }

        FORR_NODISCARD const char* c_str() const noexcept { return string.c_str(); }
        FORR_NODISCARD const char* data_ptr() const noexcept { return string.data(); }

        FORR_NODISCARD fe::StringHash get_hash() const noexcept { return hash; }

        // this compares only hash
        FORR_NODISCARD bool operator==(const hashed_string& other) const noexcept { return hash == other.hash; }
        FORR_NODISCARD bool operator==(const std::string& other) const noexcept { return string == other; }

        hashed_string operator=(const hashed_string& other) noexcept {
            string = other.string;
            hash   = other.hash;
            return *this;
        }

        hashed_string operator=(const std::string& other_string) noexcept {
            string = other_string;
            hash   = fe::string_hash(other_string);
            return *this;
        }

    private:
        std::string    string{};
        fe::StringHash hash{};
    };

    template <std::size_t N>
    struct fixed_string {
        static_assert(N > 0, "String size must be at least 1 for the null-terminator.");

        std::array<char, N> data{};

        fixed_string()  = default;
        ~fixed_string() = default;

        constexpr fixed_string(const char (&str)[N]) noexcept {
            std::copy_n(str, N, data.begin());
        }

        constexpr fixed_string(const char* str) noexcept {
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
        FORR_NODISCARD constexpr bool        empty() const noexcept { return size() == 0; }

        FORR_NODISCARD constexpr const char* c_str() const noexcept { return data.data(); }
        FORR_NODISCARD constexpr const char* data_ptr() const noexcept { return data.data(); }

        template <std::size_t M>
        FORR_NODISCARD constexpr bool operator==(const fixed_string<M>& other) const noexcept {
            return std::string_view(*this) == std::string_view(other);
        }
    };

    template <std::size_t N>
    fixed_string(const char (&)[N]) -> fixed_string<N>;

    template <std::size_t N = 32>
    class fixed_hashed_string { // mostly for GUI
    public:
        fixed_hashed_string()  = default;
        ~fixed_hashed_string() = default;

        constexpr fixed_hashed_string(const char (&str)[N]) noexcept
            : string(str), hash(fe::string_hash(std::string_view(str))) {}

        constexpr fixed_hashed_string(std::string_view str) noexcept
            : string(str), hash(fe::string_hash(str)) {}

        FORR_NODISCARD constexpr                operator std::string_view() const noexcept { return { string.data_ptr(), size() }; }
        FORR_NODISCARD constexpr fe::StringHash get_hash() const noexcept { return hash; }

        FORR_NODISCARD constexpr const char* c_str() const noexcept { return string.c_str(); }
        FORR_NODISCARD constexpr std::size_t size() const noexcept { return string.size(); }

        FORR_NODISCARD constexpr bool operator==(const fixed_hashed_string& other) const noexcept { return hash == other.hash; }

        fixed_hashed_string& operator=(std::string_view other_string) noexcept {
            string = other_string;
            hash   = fe::string_hash(other_string);
            return *this;
        }

    private:
        fe::fixed_string<N> string{};
        fe::StringHash      hash{};
    };

    template <std::size_t N>
    fixed_hashed_string(const char (&)[N]) -> fixed_hashed_string<N>;

} // namespace fe
