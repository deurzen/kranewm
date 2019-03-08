#include "key.hh"
#include "display.hh"
#include "window.hh"

#include <vector>


KeySym
x_wrapper::get_keysym(int keycode)
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
x_wrapper::grab_key(KeySym key, unsigned mask)
{
    static const ::std::vector<int> ignored_masks({
        0, LockMask, Mod2Mask,
        LockMask|Mod2Mask
    });

    int code = XKeysymToKeycode(g_dpy, key);
    for (auto& to_ignore : ignored_masks)
        XGrabKey(g_dpy, code, mask | to_ignore, g_root,
            True, GrabModeAsync, GrabModeAsync);
}

void
x_wrapper::grab_keycode(int keycode, unsigned mask)
{
    static const ::std::vector<int> ignored_masks({
        0, LockMask, Mod2Mask,
        LockMask|Mod2Mask
    });

    for (auto& to_ignore : ignored_masks)
        XGrabKey(g_dpy, keycode, mask | to_ignore, g_root,
            True, GrabModeAsync, GrabModeAsync);
}
