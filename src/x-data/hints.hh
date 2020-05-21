#ifndef __KRANEWM__X_DATA__HINTS__GUARD__
#define __KRANEWM__X_DATA__HINTS__GUARD__

#include "display.hh"

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
