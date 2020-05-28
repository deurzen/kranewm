#ifndef __KRANEWM__X_DATA__ATTRIBUTES__GUARD__
#define __KRANEWM__X_DATA__ATTRIBUTES__GUARD__

#include "window.hh"

extern "C" {
#include <X11/Xlib.h>
}

namespace x_data
{
    class attributes_t
    {
    public:
        attributes_t(XWindowAttributes wa)
          : val(wa)
        {}

        operator XWindowAttributes() const
        {
            return val;
        }

        operator dim_t() const
        {
            return dim_t{val.width, val.height};
        }

        operator pos_t() const
        {
            return pos_t{val.x, val.y};
        }

        inline XWindowAttributes&
        get()
        {
            return val;
        }

        inline int
        x() const
        {
            return val.x;
        }

        inline int
        y() const
        {
            return val.y;
        }

        inline pos_t
        pos() const
        {
            return {val.x, val.y};
        }

        inline int
        w() const
        {
            return val.width;
        }

        inline int
        h() const
        {
            return val.height;
        }

        inline dim_t
        dim() const
        {
            return {val.width, val.height};
        }

    private:
        XWindowAttributes val;

    };

    class window_t;
    extern attributes_t get_attributes(window_t&);
}

#endif//__KRANEWM__X_DATA__ATTRIBUTES__GUARD__
