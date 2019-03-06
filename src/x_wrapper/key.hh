#ifndef __KRANEWM__X_WRAPPER__KEY__GUARD__
#define __KRANEWM__X_WRAPPER__KEY__GUARD__

extern "C" {
#include <X11/Xutil.h>
}


namespace x_wrapper
{
    extern KeySym get_keysym(int);

    extern void grab_key(KeySym, unsigned);
    extern void grab_keycode(int, unsigned);
}

#endif//__KRANEWM__X_WRAPPER__KEY__GUARD__
