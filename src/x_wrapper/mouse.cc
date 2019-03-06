#include "mouse.hh"
#include "display.hh"

using namespace x_wrapper;

window_t l_confined;

void
x_wrapper::grab_button(unsigned button, unsigned mask)
{
    static const ::std::vector<int> ignored_masks({
        0, LockMask, Mod2Mask,
        LockMask|Mod2Mask
    });

    for (auto& to_ignore : ignored_masks)
        XGrabButton(g_dpy, button, mask | to_ignore,
            g_root, True, ButtonPressMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None);
}

void
x_wrapper::confine_pointer(window_t win)
{
    if (l_confined.get() != None)
        return;

    XGrabPointer(g_dpy, win, False,
        PointerMotionMask | ButtonReleaseMask,
        GrabModeAsync, GrabModeAsync,
        None, None, CurrentTime);

    l_confined = win;
}

void
x_wrapper::release_pointer()
{
    if (l_confined.get() == None)
        return;

    XUngrabButton(g_dpy, AnyButton, AnyModifier, l_confined);
    l_confined = None;
}
