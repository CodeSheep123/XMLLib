#ifndef MVG_XML_CHARCONV_HPP_
#    define MVG_XML_CHARCONV_HPP_

#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-declarations"

#    include <cassert>
#    include <codecvt>
#    include <locale>
#    include <string>
#    include <string_view>

namespace mvg {

namespace charconv {

namespace detail {

bool is_ascii(wchar_t wc) { return wc >= 0 && wc <= 255; }

} // namespace detail

template<typename To, typename From>
To char_convert(From from) {
    static_assert(
        "No character conversion for character types To and From defined");
}

template<>
char char_convert<char, wchar_t>(wchar_t ch) {
    assert(detail::is_ascii(ch));

    return ch;
}

template<>
wchar_t char_convert<wchar_t, char>(char ch) {
    return ch;
}

template<typename To, typename From>
To string_convert(From from) {
    static_assert(
        "No character conversion for string types To and From defined");
    return {};
}

template<>
std::wstring string_convert<std::wstring, std::string_view>(std::string_view sv) {
    // std::codecvt_utf18 and std::wstring_convert are deprecated in C++17. We
    // are still using them here because according to
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0618r0.html
    // They will still remain in the library until a suitable replacement is
    // standardized.

    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    return converter.from_bytes(sv.data());
}

template<>
std::string string_convert<std::string, std::wstring_view>(std::wstring_view wsv) {
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    return converter.to_bytes(wsv.data());
}

} // namespace charconv

} // namespace mvg

#endif

#pragma clang diagnostic pop
