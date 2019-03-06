#include "window.hh"
#include "event.hh"
#include "error.hh"

using namespace x_wrapper;


window_t
x_wrapper::create_window(bool do_not_manage)
{
    window_t win(XCreateSimpleWindow(g_dpy, g_root, -2, -2,
        1, 1, 1, 0x000000, 0x404040));

    if (do_not_manage) {
        XSetWindowAttributes wa;
        wa.override_redirect = true;
        XChangeWindowAttributes(g_dpy, win, CWOverrideRedirect, &wa);
    }

    return win;
}

void
x_wrapper::get_top_level_windows(::std::vector<window_t>& wins)
{
    Window _w;
    Window *children;
    unsigned nchildren;

    XQueryTree(g_dpy, g_root, &_w, &_w, &children, &nchildren);

    wins.clear();
    for (unsigned i = 0; i < nchildren; ++i)
        if (children[i] != g_root.get())
            wins.push_back(children[i]);

    if (children)
        XFree(children);
}


void
x_wrapper::window_t::close()
{
    event_t event = create_event(val, get_atom("WM_PROTOCOLS").get(),
        get_atom("WM_DELETE_WINDOW").get());
}

void
x_wrapper::window_t::force_close()
{
    int n;
    Atom* protocols;
    bool found = false;

    if (XGetWMProtocols(g_dpy, val, &protocols, &n)) {
        while (!found && n--)
            found = protocols[n] == get_atom("WM_DELETE_WINDOW").get();
        XFree(protocols);
    }

    if (found) {
        close();
    } else {
        XGrabServer(g_dpy);
        XSetErrorHandler(g_xerroroff);
        XSetCloseDownMode(g_dpy, DestroyAll);
        XKillClient(g_dpy, val);
        XSync(g_dpy, False);
        XSetErrorHandler(g_xerror);
        XUngrabServer(g_dpy);
    }
}
