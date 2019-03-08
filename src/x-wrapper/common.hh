#ifndef __KRANEWM__X_WRAPPER__COMMON__GUARD__
#define __KRANEWM__X_WRAPPER__COMMON__GUARD__

#include <iostream>


struct pos_t
{
    pos_t(int _x = 0, int _y = 0)
      : x(_x),
        y(_y)
    {}

    inline bool operator==(const pos_t& pos) const
    {
        return pos.x == x && pos.y == y;
    }

    int x;
    int y;
};

struct dim_t
{
    dim_t(int _w = 0, int _h = 0)
      : w(_w),
        h(_h)
    {}

    inline bool operator==(const dim_t& size) const
    {
        return size.w == w && size.h == h;
    }

    int w;
    int h;
};

#endif//__KRANEWM__X_WRAPPER__COMMON__GUARD__
