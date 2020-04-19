#ifndef __KRANEWM__X_DATA__KEY__GUARD__
#define __KRANEWM__X_DATA__KEY__GUARD__

extern "C" {
#include <X11/Xutil.h>
}


namespace x_data
{
    extern KeySym get_keysym(int);
    extern void refresh_keyboard_mapping(XMappingEvent&);

    extern void ungrab_grabbed_keys();
    extern void grab_key(KeySym, unsigned);
    extern void grab_keycode(int, unsigned);
}

#endif//__KRANEWM__X_DATA__KEY__GUARD__
