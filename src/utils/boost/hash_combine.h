// -*- mode: c++; -*-

#ifndef FREESPACE2_UTILS_BOOST_HASH_COMBINE_H
#define FREESPACE2_UTILS_BOOST_HASH_COMBINE_H

#include <cstddef>

#include <functional>

namespace boost {

template< class T >
inline void hash_combine (std::size_t& seed, const T& v) {
    std::hash< T > hasher;
    seed ^= hasher (v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace boost

#endif // FREESPACE2_UTILS_BOOST_HASH_COMBINE_H
