#include "xml.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>

namespace xml = mvg::xml;
namespace fs = std::filesystem;

std::wstring load_from_file(fs::path path) {
    // Due to a compiler bug, std::istream cannot be used with char16_t and
    // char32_t, so I'm using wchar_t instead
    std::basic_ifstream<wchar_t, std::char_traits<wchar_t>> file(path);
    if (!file.good()) {
        throw std::runtime_error("Failed to open file at path: " +
                                 path.generic_string());
    }

    // Read the entire file into a string
    std::wstring str(
        (std::istreambuf_iterator<wchar_t, std::char_traits<wchar_t>>(file)),
        std::istreambuf_iterator<wchar_t, std::char_traits<wchar_t>>());

    return str;
}

int main() {
    try {
        std::wstring source = load_from_file("test_xml.xml");
        xml::xml x = xml::xml::parse(source);

		std::cout << "XML version: " << x.version().major << "."
                  << x.version().minor << "\n";

    } catch (mvg::xml::parse_error const& e) {

        std::cout << "XML error: " << e.what() << "\n";
    }

    std::cin.ignore(32767, '\n');

    return 0;
}
