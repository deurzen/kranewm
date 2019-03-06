#ifndef __KRANEWM__X_WRAPPER__ATTRIBUTES__GUARD__
#define __KRANEWM__X_WRAPPER__ATTRIBUTES__GUARD__

extern "C" {
#include <X11/Xlib.h>
}

namespace x_wrapper
{
    class attributes_t
    {
    public:
        attributes_t(XWindowAttributes wa)
            : val(wa)
        {}

        operator XWindowAttributes() const { return val; }

    private:
        XWindowAttributes val;

    };

    class window_t;
    extern attributes_t get_attributes(window_t&);
}

#endif//__KRANEWM__X_WRAPPER__ATTRIBUTES__GUARD__
