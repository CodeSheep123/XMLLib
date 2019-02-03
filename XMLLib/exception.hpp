#ifndef MVG_XML_EXCEPTION_HPP_
#define MVG_XML_EXCEPTION_HPP_

#include <exception>
#include <stdexcept>

namespace mvg::xml {

struct nullptr_exception : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct parse_error : std::runtime_error {
    using std::runtime_error::runtime_error;

    // TODO: Add line number
};

struct syntax_error : parse_error {
    using parse_error::parse_error;
};

} // namespace mvg::xml

#endif
