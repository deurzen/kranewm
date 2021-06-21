#ifndef __WINSYS_COMMON_H_GUARD__
#define __WINSYS_COMMON_H_GUARD__

#include <cstddef>

typedef std::size_t Ident;
typedef std::size_t Index;

namespace winsys
{

    typedef unsigned Pid;

    enum class Toggle
    {
        On,
        Off,
        Reverse
    };

}

#endif//__WINSYS_COMMON_H_GUARD__
