#ifndef MVG_INDEX_HPP_
#define MVG_INDEX_HPP_

#include <cstddef>

#include "exception.hpp"

namespace mvg::xml {

class index {
public:
    using index_type = std::size_t;

    static constexpr index_type invalid = static_cast<index_type>(-1);

    constexpr index() : idx(invalid) {}
    constexpr index(index const& rhs) : idx(rhs.idx) {}
    constexpr index(index_type i) : idx(i) {}

    constexpr index& operator=(index const& rhs) {
        idx = rhs.idx;
        return *this;
    }

    constexpr index& operator=(index_type i) {
        idx = i;
        return *this;
    }

    constexpr index_type get() const { return idx; }

    // Only exists to be able to easily use operators like operator++,
    // operator+=, etc
    operator index_type&() { return idx; }

private:
    index_type idx;
};

template<typename C>
auto& checked_at(C& c, index i) {
    if (i >= c.size()) { throw std::out_of_range("index out of range"); }

    return c[i];
}

template<typename C>
auto const& checked_at(C const& c, index i) {
    if (i >= c.size()) { throw std::out_of_range("index out of range"); }

    return c[i];
}

} // namespace mvg::xml

#endif
