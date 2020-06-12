#include "window.hh"
#include "event.hh"
#include "error.hh"
#include "property.hh"
#include "attributes.hh"

#include <cstring>

using namespace x_data;


window_t
x_data::create_window(bool do_not_manage)
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

::std::vector<window_t>
x_data::get_top_level_windows()
{
    Window _w;
    Window *children;
    unsigned nchildren;

    XQueryTree(g_dpy, g_root, &_w, &_w, &children, &nchildren);

    ::std::vector<window_t> wins;
    for (unsigned i = 0; i < nchildren; ++i)
        if (g_root != children[i])
            wins.push_back(children[i]);

    if (children)
        XFree(children);

    return wins;
}

window_t
x_data::get_transient_for(window_t& trans)
{
    Window win = None;
    XGetTransientForHint(g_dpy, trans.get(), &win);
    return win;
}

bool
x_data::should_manage(window_t& win)
{
    auto attrs = x_data::get_attributes(win);
    return !(attrs.override_redirect()
        || attrs.c_class() == InputOnly);
}

window_t
x_data::get_input_focus()
{
    Window focused;
    int _i;
    XGetInputFocus(g_dpy, &focused, &_i);
    return focused;
}

bool
x_data::set_input_focus(window_t win)
{
    if (!win)
        win = g_root;

    XSetInputFocus(g_dpy, win.get(), RevertToNone, CurrentTime);
    return get_input_focus() == win;
}

void
x_data::select_input(window_t win, long mask)
{
    XSelectInput(g_dpy, win, mask);
}

void
x_data::restack_windows(Window* wins, int n)
{
    XRestackWindows(g_dpy, wins, n);
}



::std::string
x_data::window_t::get_name()
{
    ::std::string name;
    char name_arr[512];
    if (!get_text_property(val, get_atom("_NET_WM_NAME"), name_arr, sizeof name_arr))
        get_text_property(val, XA_WM_NAME, name_arr, sizeof name_arr);

    if (name_arr[0] == '\0')
        strcpy(name_arr, "No name");

    name.assign(name_arr);
    return name;
}

void
x_data::window_t::close()
{
    event_t event = create_event(val, get_atom("WM_PROTOCOLS").get(),
        get_atom("WM_DELETE_WINDOW").get());
    event.send(NoEventMask);
}

void
x_data::window_t::force_close()
{
    int n;
    Atom* protocols;
    bool found = false;

    if (XGetWMProtocols(g_dpy, val, &protocols, &n)) {
        while (!found && n--)
            found = get_atom("WM_DELETE_WINDOW") == protocols[n];
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

void
x_data::window_t::set_state(long state)
{
    long data[] = { state, None };
    XChangeProperty(g_dpy, val, get_atom("WM_STATE"), get_atom("WM_STATE"), 32,
        PropModeReplace, (unsigned char*) data, 2);
}

bool
x_data::window_t::is_of_type(::std::string&& type_name)
{
    atom_list_t type_atoms = get_property<atom_list_t>(val, "_NET_WM_WINDOW_TYPE");
    const atom_t type_id = get_atom("_NET_WM_WINDOW_TYPE_" + type_name);

    for (auto& atom : type_atoms.get_all())
        if (type_id == atom)
            return true;

    return false;
}

bool
x_data::window_t::is_of_state(::std::string&& state_name)
{
    atom_list_t state_atoms = get_property<atom_list_t>(val, "_NET_WM_STATE");
    const atom_t state_id = get_atom("_NET_WM_STATE_" + state_name);

    for (auto& atom : state_atoms.get_all())
        if (state_id == atom)
            return true;

    return false;
}
