#include "ewmh.hh"
#include "common.hh"
#include "decoration.hh"
#include "util.hh"
#include "workspace.hh"

#include "x_wrapper/attributes.hh"

#include <algorithm>
#include <cstring>


Atom
ewmh::get_netwm_atom(int index)
{
    if (netwm_atoms_.count(index) > 0)
    return netwm_atoms_[index];
    return NetLast;
}

void
ewmh::set_wm_name_property(x_wrapper::window_t win, ::std::string name)
{
    x_wrapper::replace_property<x_wrapper::string_t>(win, {"_NET_WM_NAME", name});
}

void
ewmh::set_supporting_wm_check_property(x_wrapper::window_t win, x_wrapper::window_t check_win)
{
    x_wrapper::replace_property<x_wrapper::window_t>(win, {"_NET_SUPPORTING_WM_CHECK", check_win});
}

void
ewmh::set_number_of_desktops_property(unsigned&& n)
{
    x_wrapper::replace_property<x_wrapper::cardinal_t>(x_wrapper::g_root, {"_NET_NUMBER_OF_DESKTOPS", n});
}

void
ewmh::set_current_desktop_property(unsigned&& i)
{
    x_wrapper::replace_property<x_wrapper::cardinal_t>(x_wrapper::g_root, {"_NET_CURRENT_DESKTOP", i});
}

void
ewmh::set_desktop_names_property(::std::vector<::std::string>& names)
{
    x_wrapper::string_list_t names_list(names);
    x_wrapper::set_text_property(x_wrapper::g_root, {"_NET_DESKTOP_NAMES", names_list});
}

void
ewmh::set_frame_extents(x_wrapper::window_t win, bool overridden)
{
    static long frame_extents_normal[] = {
        0, // left
        0, // right
        BORDER_HEIGHT, // top
        0  // bottom
    };

    static long frame_extents_overridden[] = {
        0, // left
        0, // right
        0, // top
        0  // bottom
    };

    x_wrapper::replace_property<x_wrapper::cardinal_list_t>(win, {"_NET_WM_FRAME_EXTENTS",
        {&(overridden ? frame_extents_overridden : frame_extents_normal), 4}});
}

void
ewmh::set_active_window_property(x_wrapper::window_t win)
{
    x_wrapper::replace_property<x_wrapper::window_t>(x_wrapper::g_root, {"_NET_ACTIVE_WINDOW", win});
}

void
ewmh::set_window_state_property(x_wrapper::window_t win, int i)
{
    if (Range<int>::contains(NetWmStateFirst, NetWmStateLast, i))
        x_wrapper::replace_property<x_wrapper::atom_t>(win, {"_NET_WM_STATE", netwm_atoms_[i]});
    else
        x_wrapper::unset_property<x_wrapper::atom_t>(win, {"_NET_WM_STATE"});
}

void
ewmh::set_wm_desktop_property(x_wrapper::window_t win, unsigned workspace_nr)
{
    x_wrapper::replace_property<x_wrapper::cardinal_t>(win, {"_NET_WM_DESKTOP", workspace_nr});
}

void
ewmh::set_desktop_geometry_property()
{
    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    x_wrapper::replace_property<x_wrapper::cardinal_t>(x_wrapper::g_root,
        {"_NET_DESKTOP_GEOMETRY", (unsigned) root_attrs.get().width});
    x_wrapper::append_property<x_wrapper::cardinal_t>(x_wrapper::g_root,
        {"_NET_DESKTOP_GEOMETRY", (unsigned) root_attrs.get().height});
}

void
ewmh::set_desktop_viewport_property()
{
    x_wrapper::remove_property(x_wrapper::g_root, "_NET_DESKTOP_VIEWPORT");
    for (size_t i = 0; i < USER_WORKSPACES.size(); ++i) {
        x_wrapper::append_property<x_wrapper::cardinal_t>(x_wrapper::g_root, {"_NET_DESKTOP_VIEWPORT", 0});
        x_wrapper::append_property<x_wrapper::cardinal_t>(x_wrapper::g_root, {"_NET_DESKTOP_VIEWPORT", 0});
    }
}

void
ewmh::set_workarea_property()
{
    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    x_wrapper::remove_property(x_wrapper::g_root, "_NET_WORKAREA");
    for (size_t i = 0; i < USER_WORKSPACES.size(); ++i) {
        x_wrapper::append_property<x_wrapper::cardinal_t>(x_wrapper::g_root, {"_NET_WORKAREA",
            (strut.left_window.get() != None) ? strut.left_width : SIDEBAR_WIDTH});
        x_wrapper::append_property<x_wrapper::cardinal_t>(x_wrapper::g_root, {"_NET_WORKAREA",
            strut.top_height});
        x_wrapper::append_property<x_wrapper::cardinal_t>(x_wrapper::g_root, {"_NET_WORKAREA",
            root_attrs.get().width - ((strut.left_window.get() != None) ? strut.left_width : SIDEBAR_WIDTH)});
        x_wrapper::append_property<x_wrapper::cardinal_t>(x_wrapper::g_root, {"_NET_WORKAREA",
            root_attrs.get().height - strut.top_height});
    }
}

void
ewmh::clear_client_list_property()
{
    x_wrapper::remove_property(x_wrapper::g_root, "_NET_CLIENT_LIST");
}

void
ewmh::clear_workarea_property()
{
    x_wrapper::remove_property(x_wrapper::g_root, "_NET_WORKAREA");
}

void
ewmh::clear_active_window_property()
{
    x_wrapper::remove_property(x_wrapper::g_root, "_NET_ACTIVE_WINDOW");
}

void
ewmh::set_strut_property(x_wrapper::window_t win, unsigned left, unsigned right,
    unsigned top, unsigned bottom)
{
    x_wrapper::remove_property(win, "_NET_WM_STRUT");
    x_wrapper::remove_property(win, "_NET_WM_STRUT_PARTIAL");

    x_wrapper::append_property<x_wrapper::cardinal_t>(win, {"_NET_WM_STRUT", left});
    x_wrapper::append_property<x_wrapper::cardinal_t>(win, {"_NET_WM_STRUT", right});
    x_wrapper::append_property<x_wrapper::cardinal_t>(win, {"_NET_WM_STRUT", top});
    x_wrapper::append_property<x_wrapper::cardinal_t>(win, {"_NET_WM_STRUT", bottom});

    x_wrapper::append_property<x_wrapper::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", left});
    x_wrapper::append_property<x_wrapper::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", right});
    x_wrapper::append_property<x_wrapper::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", top});
    x_wrapper::append_property<x_wrapper::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", bottom});

    for (size_t i = 0; i < 8; ++i)
        x_wrapper::append_property<x_wrapper::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", 0});
}

bool
ewmh::check_apply_strut(x_wrapper::window_t win)
{
    ::std::vector<unsigned> partial_strut_vals;
    ::std::vector<unsigned> strut_vals;

    if (x_wrapper::has_property<x_wrapper::cardinal_t>(win, "_NET_WM_STRUT_PARTIAL"))
        partial_strut_vals = x_wrapper::get_property<x_wrapper::cardinal_list_t>(win,
            {"_NET_WM_STRUT_PARTIAL"}).get_data().get();

    if (x_wrapper::has_property<x_wrapper::cardinal_list_t>(win, "_NET_WM_STRUT"))
        strut_vals = x_wrapper::get_property<x_wrapper::cardinal_list_t>(win,
            {"_NET_WM_STRUT"}).get_data().get();

    if (partial_strut_vals.empty() && strut_vals.empty())
        return false;

    if (partial_strut_vals.size() >= 4 && strut_vals.size() == 4) {
        if (strut_vals[0] == 0)
            strut_vals[0] = partial_strut_vals[0];
        if (strut_vals[1] == 0)
            strut_vals[1] = partial_strut_vals[0];
        if (strut_vals[2] == 0)
            strut_vals[2] = partial_strut_vals[0];
        if (strut_vals[3] == 0)
            strut_vals[3] = partial_strut_vals[0];
    }

    if (strut.left_window.get() == None
        && strut_vals.size() > 0
        && strut_vals[0] > 0)
    {
        strut.left_width = strut_vals[0];
        strut.left_window = win;
    } else if (strut.right_window.get() == None
        && strut_vals.size() > 1
        && strut_vals[1] > 0)
    {
        strut.right_width = strut_vals[1];
        strut.right_window = win;
    } else if (strut.top_window.get() == None
        && strut_vals.size() > 2
        && strut_vals[2] > 0)
    {
        strut.top_height = strut_vals[2];
        strut.top_window = win;
    } else if (strut.bottom_window.get() == None
        && strut_vals.size() > 3
        && strut_vals[3] > 0)
    {
        strut.bottom_height = strut_vals[3];
        strut.bottom_window = win;
    } else
        return false;

    set_workarea_property();
    return true;
}

bool
ewmh::check_release_strut(x_wrapper::window_t win)
{
    bool is_strut_win = false;
    if (strut.left_window == win) {
        strut.left_width = 0;
        strut.left_window = None;
        is_strut_win = true;
    } else if (strut.right_window == win) {
        strut.right_width = 0;
        strut.right_window = None;
        is_strut_win = true;
    } else if (strut.top_window == win) {
        strut.top_height = 0;
        strut.top_window = None;
        is_strut_win = true;
    } else if (strut.bottom_window == win) {
        strut.bottom_height = 0;
        strut.bottom_window = None;
        is_strut_win = true;
    }

    set_workarea_property();
    return is_strut_win;
}
