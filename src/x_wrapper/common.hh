#ifndef __KRANEWM__X_WRAPPER__COMMON__GUARD__
#define __KRANEWM__X_WRAPPER__COMMON__GUARD__

#include <iostream>


struct Pos
{
    Pos(int _x = 0, int _y = 0)
      : x(_x),
        y(_y)
    {}

    inline bool operator==(const Pos& pos) const
    {
        return pos.x == x && pos.y == y;
    }

    int x;
    int y;
};

struct Size
{
    Size(int _w = 0, int _h = 0)
      : w(_w),
        h(_h)
    {}

    inline bool operator==(const Size& size) const
    {
        return size.w == w && size.h == h;
    }

    int w;
    int h;
};

#endif//__KRANEWM__X_WRAPPER__COMMON__GUARD__
