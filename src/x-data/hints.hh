#ifndef __KRANEWM__X_DATA__HINTS__GUARD__
#define __KRANEWM__X_DATA__HINTS__GUARD__

#include "display.hh"

#include "common.hh"

extern "C" {
#include <X11/Xutil.h>
}


namespace x_data
{
    class wmhints_t
    {
    public:
        wmhints_t() = default;

        wmhints_t(XWMHints wmhints, bool _status)
          : val(wmhints),
            status(_status)
        {}

        operator XWMHints() const
        {
            return val;
        }

        inline XWMHints&
        get()
        {
            return val;
        }

        inline XWMHints*
        get_ptr()
        {
            return &val;
        }

        long&
        flags()
        {
            return val.flags;
        }

        inline bool
        success()
        {
            return status;
        }

    private:
        XWMHints val;
        bool status;

    };

    class sizehints_t
    {
    public:
        sizehints_t() = default;

        sizehints_t(XSizeHints sizehints, bool _status)
          : val(sizehints),
            status(_status)
        {}

        operator XSizeHints() const
        {
            return val;
        }

        inline XSizeHints&
        get()
        {
            return val;
        }

        inline XSizeHints*
        get_ptr()
        {
            return &val;
        }

        long&
        flags()
        {
            return val.flags;
        }

        bool
        flag_set(long flag)
        {
            return val.flags & flag;
        }

        dim_t
        max_dim()
        {
            return {val.max_width, val.max_height};
        }

        dim_t
        min_dim()
        {
            return {val.min_width, val.min_height};
        }

        dim_t
        base_dim()
        {
            return {val.base_width, val.base_height};
        }

        dim_t
        inc_dim()
        {
            return {val.width_inc, val.height_inc};
        }

        pos_t
        min_aspect()
        {
            return {val.min_aspect.x, val.min_aspect.y};
        }

        pos_t
        max_aspect()
        {
            return {val.max_aspect.x, val.max_aspect.y};
        }

        int
        gravity()
        {
            return val.win_gravity;
        }

        inline bool
        success()
        {
            return status;
        }

    private:
        XSizeHints val;
        bool status;

    };

    class window_t;

    extern wmhints_t get_wmhints(window_t&);
    extern void set_wmhints(window_t&, wmhints_t&);

    extern sizehints_t get_sizehints(window_t&);
}

#endif//__KRANEWM__X_DATA__HINTS__GUARD__
