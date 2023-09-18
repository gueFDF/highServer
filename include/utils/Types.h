#ifndef TYPES
#define TYPES

#include <cstddef>
#include <cstring>
inline void memZero(void* p, size_t n) {
    memset(p, 0, n);
}
template <typename To, typename From>
inline To implicit_cast(From const& f) {
    return f;
}
#endif /* TYPES */
