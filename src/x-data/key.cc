#include "key.hh"
#include "mask.hh"
#include "display.hh"
#include "window.hh"

#include <vector>


KeySym
x_data::get_keysym(int keycode)
{
    KeySym* keysyms;
    int keysyms_per_keycode;

    keysyms = XGetKeyboardMapping(g_dpy, keycode, 1, &keysyms_per_keycode);

    KeySym result;
    if (keysyms_per_keycode > 0) {
        result = keysyms[0];
        XFree(keysyms);
        return result;
    }

    return 0;
}

void
x_data::refresh_keyboard_mapping(XMappingEvent& event)
{
    XRefreshKeyboardMapping(&event);
}

void
x_data::grab_key(KeySym key, unsigned mask)
{
    int code = XKeysymToKeycode(g_dpy, key);
    for (auto& to_ignore : g_ignored_masks)
        XGrabKey(g_dpy, code, mask | to_ignore, g_root,
            True, GrabModeAsync, GrabModeAsync);
}

void
x_data::grab_keycode(int keycode, unsigned mask)
{
    for (auto& to_ignore : g_ignored_masks)
        XGrabKey(g_dpy, keycode, mask | to_ignore, g_root,
            True, GrabModeAsync, GrabModeAsync);
}
