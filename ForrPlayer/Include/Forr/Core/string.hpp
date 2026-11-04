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
#include <string>
#include <string_view>
#include <array>
#include <algorithm>

#include "attributes.hpp"

namespace fe {
    using StringHash = uint64_t; // TODO : I don't like this name

    FORR_NODISCARD constexpr StringHash make_hash(std::string_view str) noexcept {
        std::uint64_t hash = 14695981039346656037ULL;
        for (const char c : str) {
            hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(c));
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    FORR_NODISCARD constexpr StringHash make_hash(std::wstring_view str) noexcept {
        std::uint64_t hash = 14695981039346656037ULL;
        for (const wchar_t c : str) {
            hash ^= static_cast<std::uint64_t>(static_cast<std::make_unsigned_t<wchar_t>>(c));
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    template <typename T>
    struct string_traits;

    template <>
    struct string_traits<std::string> {
        using char_type  = char;
        using view_type  = std::string_view;
        using owned_type = std::string; // the type without 'view'
    };

    template <>
    struct string_traits<std::string_view> {
        using char_type  = char;
        using view_type  = std::string_view;
        using owned_type = std::string; // the type without 'view'
    };

    template <>
    struct string_traits<std::wstring> {
        using char_type  = wchar_t;
        using view_type  = std::wstring_view;
        using owned_type = std::wstring; // the type without 'view'
    };

    template <>
    struct string_traits<std::wstring_view> {
        using char_type  = wchar_t;
        using view_type  = std::wstring_view;
        using owned_type = std::wstring; // the type without 'view'
    };

    template <typename T>
    concept storable_string = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> ||
                              std::is_same_v<T, std::wstring> || std::is_same_v<T, std::wstring_view>;

    // this is like std::string ( or std::string_view, std::wstring, std::wstring_view ) but it uses hash to compare
    template <storable_string T>
    class hashed_string_base {
    private:
        using traits = string_traits<T>;

    public:
        using char_type  = traits::char_type;
        using view_type  = traits::view_type;
        using owned_type = traits::owned_type;

    public:
        hashed_string_base()  = default;
        ~hashed_string_base() = default;

        FORR_CLASS_MOVABLE(hashed_string_base)
        FORR_CLASS_COPYABLE(hashed_string_base)

        constexpr hashed_string_base(const char_type* str) : m_string(str), m_hash(fe::make_hash(m_string)) {}
        constexpr explicit hashed_string_base(owned_type string) : m_string(std::move(string)), m_hash(fe::make_hash(m_string)) {}
        constexpr explicit hashed_string_base(view_type string_view) : m_string(string_view), m_hash(fe::make_hash(m_string)) {}

        FORR_NODISCARD operator view_type() const noexcept { return { m_string.data(), size() }; }
        FORR_NODISCARD operator owned_type() const noexcept { return m_string; }
        FORR_NODISCARD operator fe::StringHash() const noexcept {
            return m_hash;
        }

        FORR_NODISCARD std::size_t size() const noexcept { return m_string.size(); }
        FORR_NODISCARD bool        empty() const noexcept { return this->size() == 0; }

        FORR_NODISCARD const char_type* c_str() const noexcept
            requires std::is_same_v<T, owned_type>
        {
            return m_string.c_str();
        }

        FORR_NODISCARD const char_type* data() const noexcept { return m_string.data(); }
        FORR_NODISCARD fe::StringHash get_hash() const noexcept { return m_hash; }

        // this compares only hash
        template <typename U>
        FORR_NODISCARD bool operator==(const hashed_string_base<U>& other) const noexcept { return m_hash == other.m_hash; }

        FORR_NODISCARD bool operator==(const owned_type& string) const noexcept { return m_string == string; }
        FORR_NODISCARD bool operator==(const char_type* str) const noexcept { return m_string == str; }
        FORR_NODISCARD bool operator==(view_type string_view) const noexcept { return m_string == string_view; }

        hashed_string_base& operator=(const char_type* str) {
            m_string = str;
            m_hash   = fe::make_hash(str);
            return *this;
        }

        hashed_string_base& operator=(view_type string_view) {
            m_string = string_view;
            m_hash   = fe::make_hash(m_string);
            return *this;
        }

        hashed_string_base& operator=(const owned_type& string) {
            m_string = string;
            m_hash   = fe::make_hash(string);
            return *this;
        }

    private:
        T              m_string{};
        fe::StringHash m_hash{};

        friend class hashed_string_base;
    };

    using hashed_string      = hashed_string_base<std::string>;
    using hashed_string_view = hashed_string_base<std::string_view>;

    using hashed_wstring      = hashed_string_base<std::wstring>;
    using hashed_wstring_view = hashed_string_base<std::wstring_view>;

    template <std::size_t N, storable_string T>
        requires std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>
    class fixed_string_base {
    private:
        using traits = string_traits<T>;

    public:
        using char_type  = traits::char_type;
        using view_type  = traits::view_type;
        using owned_type = traits::owned_type;

        static_assert(N > 0, "String size must be at least 1 for the null-terminator.");

        fixed_string_base()  = default;
        ~fixed_string_base() = default;

        FORR_CLASS_MOVABLE(fixed_string_base)
        FORR_CLASS_COPYABLE(fixed_string_base)
        
        constexpr fixed_string_base(const char_type (&str)[N]) {
            std::copy_n(str, N, m_data.begin());
        }

        constexpr fixed_string_base(const char_type* str) {
            std::copy_n(str, N, m_data.begin());
        }

        constexpr fixed_string_base(view_type str) {
            const std::size_t copy_size = std::min(N - 1, str.size());
            std::copy_n(str.data(), copy_size, m_data.begin());
            m_data[copy_size] = '\0';
        }

        FORR_NODISCARD constexpr operator view_type() const noexcept{
            return { m_data.data(), size() };
        }

        FORR_NODISCARD constexpr std::size_t size() const noexcept { return N - 1; }
        FORR_NODISCARD constexpr bool        empty() const noexcept { return this->size() == 0; }

        FORR_NODISCARD constexpr const char_type* c_str() const noexcept { return m_data.data(); }
        FORR_NODISCARD constexpr const char_type* data_ptr() const noexcept { return m_data.data(); }

        FORR_NODISCARD constexpr bool operator==(const fixed_string_base<N, T>& other) const noexcept {
            return view_type(*this) == view_type(other);
        }

    private:
        std::array<char_type, N> m_data{};
    };

    template <std::size_t N, storable_string T>
        requires std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>
    fixed_string_base(const char (&)[N], T) -> fixed_string_base<N, T>;

    template <std::size_t N>
    using fixed_string = fixed_string_base<N, std::string>;

    template <std::size_t N>
    using fixed_wstring = fixed_string_base<N, std::wstring>;

    template <std::size_t N, storable_string T>
        requires std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>
    class fixed_hashed_string_base { // mostly for GUI
    private:
        using traits = string_traits<T>;

    public:
        using char_type  = traits::char_type;
        using view_type  = traits::view_type;
        using owned_type = traits::owned_type;

        static_assert(N > 0, "String size must be at least 1 for the null-terminator.");

        fixed_hashed_string_base()  = default;
        ~fixed_hashed_string_base() = default;

        FORR_CLASS_MOVABLE(fixed_hashed_string_base)
        FORR_CLASS_COPYABLE(fixed_hashed_string_base)

        constexpr fixed_hashed_string_base(const char_type (&str)[N])
            : m_string(str), m_hash(fe::make_hash(view_type(str))) {}

        constexpr fixed_hashed_string_base(view_type str)
            : m_string(str), m_hash(fe::make_hash(str)) {}

        FORR_NODISCARD constexpr                operator view_type() const noexcept { return { m_string.data_ptr(), size() }; }
        FORR_NODISCARD constexpr fe::StringHash get_hash() const noexcept { return m_hash; }

        FORR_NODISCARD constexpr const char_type* c_str() const noexcept { return m_string.c_str(); }
        FORR_NODISCARD constexpr std::size_t      size() const noexcept { return m_string.size(); }

        FORR_NODISCARD constexpr bool operator==(const fixed_hashed_string_base& other) const noexcept { return m_hash == other.m_hash; }

        fixed_hashed_string_base& operator=(view_type other_string) {
            m_string = other_string;
            m_hash   = fe::make_hash(other_string);
            return *this;
        }

    private:
        fe::fixed_string_base<N, T> m_string{};
        fe::StringHash              m_hash{};
    };

    template <std::size_t N, storable_string T>
        requires std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>
    fixed_hashed_string_base(const char (&)[N], T) -> fixed_hashed_string_base<N, T>;

    template <std::size_t N>
    using fixed_hashed_string = fixed_hashed_string_base<N, std::string>;

    template <std::size_t N>
    using fixed_hashed_wstring = fixed_hashed_string_base<N, std::wstring>;

} // namespace fe

template <fe::storable_string T>
struct std::hash<fe::hashed_string_base<T>> {
    std::size_t operator()(const fe::hashed_string_base<T>& handle) const {
        return handle.get_hash();
    }
};

template <size_t N, fe::storable_string T>
struct std::hash<fe::fixed_hashed_string_base<N, T>> {
    std::size_t operator()(const fe::fixed_hashed_string_base<N, T>& handle) const {
        return handle.get_hash();
    }
};
