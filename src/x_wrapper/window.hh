#ifndef __KRANEWM__X_WRAPPER__WINDOW__GUARD__
#define __KRANEWM__X_WRAPPER__WINDOW__GUARD__

#include "type.hh"

extern "C" {
#include <X11/X.h>
#include <X11/Xatom.h>
}


namespace x_wrapper
{

    class window_t : protected x_type
    {
    public:
        window_t() = default;

        operator Window() const { return val; }
        operator bool() const { return val != 0; }

        inline int size() const { return 1; }
        inline Atom type() const { return XA_WINDOW; }

    private:
        Window val;

    };
}

#endif//__KRANEWM__X_WRAPPER__WINDOW__GUARD__
