/*===============================================

    Forr Engine

    File : string.hpp
    Role : everything with strings. the project primarily uses std::string and assumes it is encoded in UTF-8
           there are a lot of tools, which done with 'wchar_t', but now it's deprecated - use just std::string

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <string>
#include <string_view>
#include <array>
#include <algorithm>
#include <unordered_map>
#include <span>

#include "attributes.hpp"
#include "custom_allocators.hpp"

namespace fe {
    using StringHash                       = uint64_t; // TODO : I don't like this name
    static constexpr StringHash HASH_START = 14695981039346656037ULL;
    static constexpr StringHash HASH_SHIFT = 1099511628211ULL;

    FORR_NODISCARD constexpr std::uint64_t hash_codepoint(std::uint64_t hash, std::uint32_t code_point) noexcept {
        static constexpr StringHash BITS_IN_BYTE = 8;
        static constexpr StringHash ONE_BYTE     = 0xFF;

        for (size_t i = 0; i < 4; i++) {
            hash ^= (code_point & ONE_BYTE);
            hash *= HASH_SHIFT;
            code_point >>= BITS_IN_BYTE;
        }

        return hash;
    }

    FORR_NODISCARD constexpr std::uint64_t make_hash(std::u32string_view str) noexcept {
        std::uint64_t hash = HASH_START;
        for (std::uint32_t code_point : str) {
            hash = hash_codepoint(hash, code_point);
        }
        return hash;
    }

    FORR_NODISCARD constexpr std::uint64_t make_hash(std::u16string_view str) noexcept {
        std::uint64_t hash = HASH_START;
        for (size_t i = 0; i < str.size(); i++) {
            std::uint32_t code_point = str[i];

            // read the next element, if there is a symbol, which requires more bytes
            if (code_point >= 0xD800 &&
                code_point <= 0xDBFF &&
                i < str.size() - 1) {

                std::uint32_t low = str[i++];
                code_point        = 0x10000 + ((code_point - 0xD800) << 10) + (low - 0xDC00);
            }

            hash = hash_codepoint(hash, code_point);
        }
        return hash;
    }

    FORR_NODISCARD constexpr std::uint64_t make_hash(std::string_view str) noexcept {
        std::uint64_t hash = HASH_START;
        std::uint32_t codepoint{};
        std::uint32_t state{};

        for (const char c : str) {
            std::uint8_t byte = static_cast<std::uint8_t>(c);

            if (byte < 0x80) { // ASCII
                hash  = hash_codepoint(hash, byte);
                state = 0;
            }
            else if ((byte & 0xC0) == 0x80) {
                codepoint = (byte & 0x3F) | (codepoint << 6);
                if (state-- == 0) {
                    hash = hash_codepoint(hash, codepoint);
                }
            }
            else {
                if ((byte & 0xE0) == 0xC0) { // 2 bytes
                    codepoint = byte & 0x1F;
                    state     = 1;
                }
                else if ((byte & 0xF0) == 0xE0) { // 3 bytes
                    codepoint = byte & 0x0F;
                    state     = 2;
                }
                else if ((byte & 0xF8) == 0xF0) { // 4 bytes
                    codepoint = byte & 0x07;
                    state     = 3;
                }
            }
        }
        return hash;
    }

    // @brief create a hash from string
    // @tparam Element char, char8_t, char16_t, char32_t or wchar_t
    //
    // @details the function guaratees that char, char8_t, char16_t, char32_t
    //          and wchar_t will result the same hash, if there are the same symbols in them
    //
    // "test" == u8"test" == u16"test" == u32"test" == L"test"
    //   ^          ^           ^            ^            ^
    //  char      char8_t    char16_t     char32_t     wchar_t
    //   1b         1b          2b           4b          2b
    //
    template <class Element = char>
    FORR_NODISCARD constexpr StringHash make_hash(std::basic_string_view<Element> str) noexcept {
        return fe::make_hash(std::basic_string_view<Element>(str));
    }

    template <class Element = char>
    FORR_NODISCARD constexpr StringHash make_hash(const Element* str) noexcept {
        return fe::make_hash(std::basic_string_view<Element>(str));
    }

    template <class Element, bool IsStringView = false>
    class basic_hashed_string {
    private:
        using StringT     = std::basic_string<Element>;
        using StringViewT = std::basic_string_view<Element>;
        using CharT       = Element;

    public:
        basic_hashed_string()  = default;
        ~basic_hashed_string() = default;

        FORR_CLASS_MOVABLE(basic_hashed_string)
        FORR_CLASS_COPYABLE(basic_hashed_string)

        constexpr basic_hashed_string(const CharT* str) : m_string(str), m_hash(fe::make_hash<Element>(m_string)) {}
        constexpr basic_hashed_string(StringViewT string_view) : m_string(string_view), m_hash(fe::make_hash<Element>(m_string)) {}
        constexpr explicit basic_hashed_string(StringT string) : m_string(std::move(string)), m_hash(fe::make_hash<Element>(m_string)) {}

        FORR_NODISCARD operator StringViewT() const noexcept { return StringViewT{ m_string.data(), size() }; }
        FORR_NODISCARD operator StringT() const noexcept { return m_string.data(); }
        FORR_NODISCARD operator fe::StringHash() const noexcept {
            return m_hash;
        }

        FORR_NODISCARD std::size_t size() const noexcept { return m_string.size(); }
        FORR_NODISCARD bool        empty() const noexcept { return this->size() == 0; }

        FORR_NODISCARD const CharT* c_str() const noexcept
            requires(!IsStringView)
        {
            return m_string.c_str();
        }

        FORR_NODISCARD const CharT* data() const noexcept { return m_string.data(); }
        FORR_NODISCARD fe::StringHash get_hash() const noexcept { return m_hash; }

        // this compares only hash
        template <typename T2, bool IsStringView2>
        FORR_NODISCARD bool operator==(const basic_hashed_string<T2, IsStringView2>& other) const noexcept { return m_hash == other.m_hash; }

        FORR_NODISCARD bool operator==(const StringT& string) const noexcept { return m_string == string; }
        FORR_NODISCARD bool operator==(const CharT* str) const noexcept { return m_string == str; }
        FORR_NODISCARD bool operator==(StringViewT string_view) const noexcept { return m_string == string_view; }

        basic_hashed_string& operator=(const CharT* str) {
            m_string = str;
            m_hash   = fe::make_hash<Element>(str);
            return *this;
        }

        basic_hashed_string& operator=(StringViewT string_view) {
            m_string = string_view;
            m_hash   = fe::make_hash<Element>(m_string);
            return *this;
        }

        basic_hashed_string& operator=(const StringT& string) {
            m_string = string;
            m_hash   = fe::make_hash<Element>(string);
            return *this;
        }

    private:
        std::conditional_t<IsStringView, StringViewT, StringT> m_string{};
        fe::StringHash                                         m_hash{};

        friend class basic_hashed_string;
    };

    using hashed_string      = basic_hashed_string<char, false>;
    using hashed_string_view = basic_hashed_string<char, true>;

    using hashed_u8string      = basic_hashed_string<char8_t, false>;
    using hashed_u8string_view = basic_hashed_string<char8_t, true>;

    using hashed_u16string      = basic_hashed_string<char16_t, false>;
    using hashed_u16string_view = basic_hashed_string<char16_t, true>;

    using hashed_u32string      = basic_hashed_string<char32_t, false>;
    using hashed_u32string_view = basic_hashed_string<char32_t, true>;

    using hashed_wstring      = basic_hashed_string<wchar_t, false>;
    using hashed_wstring_view = basic_hashed_string<wchar_t, true>;

    template <std::size_t N, class Element, bool IsStringView = false>
    class basic_fixed_string {
    private:
        using StringT     = std::basic_string<Element>;
        using StringViewT = std::basic_string_view<Element>;
        using CharT       = Element;

    public:
        static_assert(N > 0, "String size must be at least 1 for the null-terminator.");

        basic_fixed_string()  = default;
        ~basic_fixed_string() = default;

        FORR_CLASS_MOVABLE(basic_fixed_string)
        FORR_CLASS_COPYABLE(basic_fixed_string)

        constexpr basic_fixed_string(const CharT (&str)[N]) {
            std::copy_n(str, N, m_data.begin());
        }

        constexpr basic_fixed_string(const CharT* str) {
            std::copy_n(str, N, m_data.begin());
        }

        constexpr basic_fixed_string(StringViewT str) {
            const std::size_t copy_size = std::min(N - 1, str.size());
            std::copy_n(str.data(), copy_size, m_data.begin());
            m_data[copy_size] = '\0';
        }

        FORR_NODISCARD constexpr operator StringViewT() const noexcept {
            return { m_data.data(), size() };
        }

        FORR_NODISCARD constexpr std::size_t size() const noexcept { return N - 1; }
        FORR_NODISCARD constexpr bool        empty() const noexcept { return this->size() == 0; }

        FORR_NODISCARD constexpr const CharT* c_str() const noexcept { return m_data.data(); }
        FORR_NODISCARD constexpr const CharT* data_ptr() const noexcept { return m_data.data(); }

        template <class Element2, bool IsStringView2>
        FORR_NODISCARD constexpr bool operator==(const basic_fixed_string<N, Element2, IsStringView2>& other) const noexcept {
            return StringViewT(*this) == StringViewT(other);
        }

    private:
        std::array<CharT, N> m_data{};
    };

    template <std::size_t N, class Element, bool IsStringView = false>
    basic_fixed_string(const char (&)[N], Element) -> basic_fixed_string<N, Element, IsStringView>;

    template <std::size_t N>
    using fixed_string = basic_fixed_string<N, char, false>;
    template <std::size_t N>
    using fixed_string_view = basic_fixed_string<N, char, true>;

    template <std::size_t N>
    using fixed_u8string = basic_fixed_string<N, char8_t, false>;
    template <std::size_t N>
    using fixed_u8string_view = basic_fixed_string<N, char8_t, true>;

    template <std::size_t N>
    using fixed_u16string = basic_fixed_string<N, char16_t, false>;
    template <std::size_t N>
    using fixed_u16string_view = basic_fixed_string<N, char16_t, true>;

    template <std::size_t N>
    using fixed_u32string = basic_fixed_string<N, char32_t, false>;
    template <std::size_t N>
    using fixed_u32string_view = basic_fixed_string<N, char32_t, true>;

    template <std::size_t N>
    using fixed_wstring = basic_fixed_string<N, wchar_t, false>;
    template <std::size_t N>
    using fixed_wstring_view = basic_fixed_string<N, wchar_t, true>;

    template <std::size_t N, class Element, bool IsStringView = false>
    class basic_fixed_hashed_string { // mostly for GUI
    private:
        using StringT     = std::basic_string<Element>;
        using StringViewT = std::basic_string_view<Element>;
        using CharT       = Element;

    public:
        static_assert(N > 0, "String size must be at least 1 for the null-terminator.");

        basic_fixed_hashed_string()  = default;
        ~basic_fixed_hashed_string() = default;

        FORR_CLASS_MOVABLE(basic_fixed_hashed_string)
        FORR_CLASS_COPYABLE(basic_fixed_hashed_string)

        constexpr basic_fixed_hashed_string(const CharT (&str)[N])
            : m_fixed_string(str), m_hash(fe::make_hash<Element>(StringViewT(str))) {}

        constexpr basic_fixed_hashed_string(StringViewT str)
            : m_fixed_string(str), m_hash(fe::make_hash<Element>(str)) {}

        FORR_NODISCARD constexpr                operator StringViewT() const noexcept { return { m_fixed_string.c_str(), size() }; }
        FORR_NODISCARD constexpr fe::StringHash get_hash() const noexcept { return m_hash; }

        FORR_NODISCARD constexpr const CharT* c_str() const noexcept { return m_fixed_string.c_str(); }
        FORR_NODISCARD constexpr std::size_t  size() const noexcept { return m_fixed_string.size(); }

        template <class Element2, bool IsStringView2>
        FORR_NODISCARD constexpr bool operator==(const basic_fixed_hashed_string<N, Element2, IsStringView2>& other) const noexcept {
            return m_hash == other.m_hash;
        }

        basic_fixed_hashed_string& operator=(StringViewT other_string) {
            m_fixed_string = other_string;
            m_hash         = fe::make_hash<Element>(other_string);
            return *this;
        }

    private:
        fe::basic_fixed_string<N, Element, IsStringView> m_fixed_string{};
        fe::StringHash                                   m_hash{};
    };

    template <std::size_t N, class Element, bool IsStringView = false>
    basic_fixed_hashed_string(const char (&)[N], Element) -> basic_fixed_hashed_string<N, Element, IsStringView>;

    template <std::size_t N>
    using fixed_hashed_string = basic_fixed_hashed_string<N, char, false>;
    template <std::size_t N>
    using fixed_hashed_string_view = basic_fixed_hashed_string<N, char, true>;

    template <std::size_t N>
    using fixed_hashed_u8string = basic_fixed_hashed_string<N, char8_t, false>;
    template <std::size_t N>
    using fixed_hashed_u8string_view = basic_fixed_hashed_string<N, char8_t, true>;

    template <std::size_t N>
    using fixed_hashed_u16string = basic_fixed_hashed_string<N, char16_t, false>;
    template <std::size_t N>
    using fixed_hashed_u16string_view = basic_fixed_hashed_string<N, char16_t, true>;

    template <std::size_t N>
    using fixed_hashed_u32string = basic_fixed_hashed_string<N, char32_t, false>;
    template <std::size_t N>
    using fixed_hashed_u32string_view = basic_fixed_hashed_string<N, char32_t, true>;

    template <std::size_t N>
    using fixed_hashed_wstring = basic_fixed_hashed_string<N, wchar_t, false>;
    template <std::size_t N>
    using fixed_hashed_wstring_view = basic_fixed_hashed_string<N, wchar_t, true>;

    class string_pool {
    public:
        std::string_view get_or_add(std::string_view str) {
            fe::StringHash hash = fe::make_hash(str);

            auto it = m_hash_view_table.find(hash);
            if (it != m_hash_view_table.end()) {
                return it->second;
            }

            char* arena_storage = reinterpret_cast<char*>(m_arena.allocate(str.size() + 1)); // null-terminator
            if (arena_storage == nullptr) {
                return std::string_view{};
            }
            std::memcpy(arena_storage, str.data(), str.size());
            arena_storage[str.size()] = '\0';

            std::string_view allocated_view(arena_storage, str.size());
            m_hash_view_table[hash] = allocated_view;

            return allocated_view;
        }

        std::string_view find(fe::StringHash hash) const {
            auto it = m_hash_view_table.find(hash);
            return it != m_hash_view_table.end() ? it->second : std::string_view{};
        }

    private:
        void increse_arena_capacity() {
            
        }

    private:
        std::unordered_map<fe::StringHash, std::string_view> m_hash_view_table{};
        fe::Arena                                            m_arena{ 1024 * 4 };
    };

} // namespace fe

namespace std {
    template <class Element, bool IsStringView>
    struct std::hash<fe::basic_hashed_string<Element, IsStringView>> {
        using is_transparent = void;

        std::size_t operator()(const fe::basic_hashed_string<Element, IsStringView>& handle) const noexcept {
            return handle.get_hash();
        }

        template <class Element2>
        std::size_t operator()(const fe::basic_hashed_string<Element2, IsStringView>& handle) const noexcept {
            return handle.get_hash();
        }
    };

    template <std::size_t N, class Element, bool IsStringView>
    struct std::hash<fe::basic_fixed_string<N, Element, IsStringView>> {
        using is_transparent = void;

        std::size_t operator()(const fe::basic_fixed_string<N, Element, IsStringView>& handle) const noexcept {
            return handle.get_hash();
        }

        template <class Element2>
        std::size_t operator()(const fe::basic_fixed_string<N, Element2, IsStringView>& handle) const noexcept {
            return handle.get_hash();
        }
    };

    template <std::size_t N, class Element, bool IsStringView>
    struct std::hash<fe::basic_fixed_hashed_string<N, Element, IsStringView>> {
        using is_transparent = void;

        std::size_t operator()(const fe::basic_fixed_hashed_string<N, Element, IsStringView>& handle) const noexcept {
            return handle.get_hash();
        }

        template <class Element2>
        std::size_t operator()(const fe::basic_fixed_hashed_string<N, Element2, IsStringView>& handle) const noexcept {
            return handle.get_hash();
        }
    };
} // namespace std

namespace fe {
    struct transparent_equal {
        using is_transparent = void;

        template <typename T1, typename T2>
        bool operator()(const T1& lhs, const T2& rhs) const noexcept {
            return lhs == rhs;
        }
    };
} // namespace fe
