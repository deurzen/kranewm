#ifndef __KRANEWM__X_DATA__COMMON__GUARD__
#define __KRANEWM__X_DATA__COMMON__GUARD__

#include <iostream>

extern "C" {
#include <X11/X.h>
}


const long REG_WIN_SELECTION = PropertyChangeMask;
const long REG_FRAME_SELECTION = FocusChangeMask
    | SubstructureRedirectMask | SubstructureNotifyMask;
const long NONSUBSTR_FRAME_SELECTION = (REG_FRAME_SELECTION & ~SubstructureRedirectMask)
    & ~SubstructureNotifyMask;

struct dim_t
{
    dim_t(int _w = 0, int _h = 0)
      : w(_w),
        h(_h)
    {}

    inline bool
    operator==(const dim_t& dim) const
    {
        return w == dim.w && h == dim.h;
    }

    inline bool
    operator!=(const dim_t& dim) const
    {
        return w != dim.w || h != dim.h;
    }

    inline dim_t
    operator+(const dim_t& dim) const
    {
        return {w + dim.w, h + dim.h};
    }

    inline dim_t
    operator-(const dim_t& dim) const
    {
        return {w - dim.w, h - dim.h};
    }

    int w;
    int h;

};

struct pos_t
{
    pos_t(int _x = 0, int _y = 0)
      : x(_x),
        y(_y)
    {}

    inline bool
    operator==(const pos_t& pos) const
    {
        return x == pos.x && y == pos.y;
    }

    inline bool
    operator!=(const pos_t& pos) const
    {
        return x != pos.x || y != pos.y;
    }

    inline pos_t
    operator+(const pos_t& pos) const
    {
        return {x + pos.x, y + pos.y};
    }

    inline pos_t
    operator-(const pos_t& pos) const
    {
        return {x - pos.x, y - pos.y};
    }

    inline pos_t
    operator+(const dim_t& dim) const
    {
        return {x + dim.w, y + dim.h};
    }

    inline pos_t
    operator-(const dim_t& dim) const
    {
        return {x - dim.w, y - dim.h};
    }

    int x;
    int y;

};

#endif//__KRANEWM__X_DATA__COMMON__GUARD__
