#ifndef MVG_XML_HPP_
#define MVG_XML_HPP_

#include "charconv.hpp"
#include "tree.hpp"

#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mvg::xml {

struct xml_version {
    int major;
    int minor;
};

namespace detail {

template<typename T, typename U, typename... Us>
struct is_one_of {
    static constexpr bool value =
        std::is_same<std::remove_cv_t<T>, std::remove_cv_t<U>>::value ||
        is_one_of<T, Us...>::value;
};

template<typename T, typename U>
struct is_one_of<T, U> {
    static constexpr bool value =
        std::is_same<std::remove_cv_t<T>, std::remove_cv_t<U>>::value;
};

} // namespace detail

#define XML_TPL_DECL                                                           \
    template<typename Num, typename Unsigned, typename Bool, typename Float,   \
             typename String>

#define XML_BASIC_TPL_ARG_LIST <int, unsigned int, bool, float, std::wstring>
#define XML_BASIC_TPL_ARG_LIST_NO_BRACKETS                                     \
    int, unsigned int, bool, float, std::wstring

#define XML_TPL_ARG_LIST <Num, Unsigned, Bool, Float, String>
#define XML_TPL_ARG_LIST_NO_BRACKETS Num, Unsigned, Bool, Float, String

XML_TPL_DECL
class xml_element {
public:
    // This should not be an unordered_map, since the keys are not guaranteed to
    // be unique
    using nested_elem_t = std::unordered_map<String, xml_element>;

    xml_element() = default;

    template<typename T>
    xml_element(String const& tag,
                T&& data,
                std::unordered_map<String, String> attributes) :
        tag_(tag),
        data_(std::forward<T>(data)), attributes_(attributes) {
        static_assert(
            detail::is_one_of<T, XML_TPL_ARG_LIST_NO_BRACKETS, nested_elem_t>,
            "Type T must be a valid xml type");
    }

    xml_element(xml_element const&) = default;
    xml_element(xml_element&&) = default;

    xml_element& operator=(xml_element const&) = default;
    xml_element& operator=(xml_element&&) = default;

    void set_tag(String const& t) { tag_ = t; }
    String tag() const { return tag_; }

    template<typename T>
    void set_data(T&& t) {
        data_ = t;
    }

    Num as_number() const { return std::get<Num>(data_); }
    Unsigned as_unsigned() const { return std::get<Unsigned>(data_); }
    Float as_float() const { return std::get<Float>(data_); }
    Bool as_bool() const { return std::get<Bool>(data_); }
    String as_string() const { return std::get<String>(data_); }

    auto& operator[](String const& nested_tag) {
        if (!std::holds_alternative<nested_elem_t>(data_)) {
            throw std::bad_variant_access();
        }

        return std::get<nested_elem_t>(data_).at(nested_tag);
    }

private:
    String tag_;
    std::variant<XML_TPL_ARG_LIST_NO_BRACKETS, nested_elem_t> data_;

    std::unordered_map<String, std::variant XML_TPL_ARG_LIST> attributes_;
};

XML_TPL_DECL
class basic_xml {
public:
    using number_t = Num;
    using unsigned_t = Unsigned;
    using bool_t = Bool;
    using float_t = Float;
    using string_t = String;

    using istringstream_t = std::basic_istringstream<
        typename string_t::value_type,
        std::char_traits<typename string_t::value_type>>;

    basic_xml() = default;
    basic_xml(basic_xml const&) = default;
    basic_xml(basic_xml&&) = default;

    basic_xml& operator=(basic_xml const&) = default;
    basic_xml& operator=(basic_xml&&) = default;

    static basic_xml parse(string_t const& xml) {
        istringstream_t sstream(xml);

        // The resulting xml
        basic_xml parsed;
        // Every valid xml document must start with a 'prolog'
        parse_prolog(sstream, parsed);
        return parsed;
    }

    xml_version version() { return version_; }

private:
    static void parse_prolog(istringstream_t& sstream, basic_xml& result) {
        // Example of a prolog:
        /*
         <?xml version="1.1"?>
        */
        // See the XML standard at:
        // https://www.w3.org/TR/2006/REC-xml11-20060816/#sec-prolog-dtd

        std::wstring buf;
        sstream >> buf;

        // Check if the first word of the XMl document is ' <?xml '. If not,
        // this is not a well-formed prolog

        if (buf != L"<?xml") {
            throw syntax_error(
                "Syntax error: Prolog: Expected XML declaration");
        }

        // Read the next part of the XML declaration. This should be '
        // version="x.y" '
        sstream >> buf;
        result.version_ = parse_version(buf);
    }

    static xml_version parse_version(std::wstring& buf) {
        // First, remove spaces in this part, because they will be annoying for
        // parsing
        buf.erase(std::remove(buf.begin(), buf.end(), L' '), buf.end());

        static constexpr std::size_t version_decl_size =
            9; // == strlen("version=\"");
        if (buf.substr(0, version_decl_size) != L"version=\"") {
            throw syntax_error(
                "Syntax error: Prolog: Expected version declaration");
        }

        try {
            // Find the version separator '.'
            std::size_t sep = buf.find_first_of('.');
            if (sep == std::wstring::npos) {
                throw syntax_error("Syntax error: Prolog: Expected symbol '.'");
            }

            // sep - version_decl_size is the length of the version number. The
            // version number starts after the version declaration, so at index
            // version_decl_size
            auto version_major = std::stoi(
                buf.substr(version_decl_size, sep - version_decl_size));

            // Get the second part of the version, following the same pattern
            std::size_t end_quote = buf.find_last_of(buf, L'\"');
            auto version_minor =
                std::stoi(buf.substr(sep + 1, end_quote - sep));

            return {version_major, version_minor};
        } catch (std::out_of_range const& e) {
            throw parse_error("Could not parse version number: '" +
                              charconv::string_convert<std::string>(
                                  buf.substr(version_decl_size)) +
                              "'. Version number too large");
        } catch (std::invalid_argument const& e) {
            throw syntax_error("Could not parse version number: '" +
                               charconv::string_convert<std::string>(
                                   buf.substr(version_decl_size)) +
                               "'. Not a number.");
        }
    }

    tree<xml_element XML_TPL_ARG_LIST> tree_;

    xml_version version_;
};

using xml = basic_xml XML_BASIC_TPL_ARG_LIST;

#undef XML_TPL_ARG_LIST
#undef XML_TPL_ARG_LIST_NO_BRACKETS
#undef XML_BASIC_TPL_ARG_LIST
#undef XML_BASIC_TPL_ARG_LIST_NO_BRACKETS
#undef XML_TPL_DECL

} // namespace mvg::xml

#endif
