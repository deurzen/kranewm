#include "ewmh.hh"

#include "common.hh"
#include "context.hh"
#include "decoration.hh"
#include "util.hh"
#include "workspace.hh"

#include "x-data/attributes.hh"

#include <algorithm>
#include <cstring>


Atom
ewmh_t::get_netwm_atom(netwmid_t index)
{
    if (m_netwm_atoms.count(index) > 0)
        return m_netwm_atoms[index];
    return netwmid_t::netlast;
}

void
ewmh_t::set_supporting_wm_check_property(x_data::window_t win, x_data::window_t check_win)
{
    x_data::replace_property<x_data::window_t>(win, {"_NET_SUPPORTING_WM_CHECK", check_win});
}

void
ewmh_t::set_number_of_desktops_property(unsigned&& n)
{
    x_data::replace_property<x_data::cardinal_t>(x_data::g_root, {"_NET_NUMBER_OF_DESKTOPS", n});
}

void
ewmh_t::set_current_desktop_property(unsigned&& i)
{
    x_data::replace_property<x_data::cardinal_t>(x_data::g_root, {"_NET_CURRENT_DESKTOP", i});
}

void
ewmh_t::set_desktop_names_property(::std::vector<::std::string>& names)
{
    x_data::string_list_t names_list(names);
    x_data::set_text_property(x_data::g_root, {"_NET_DESKTOP_NAMES", names_list});
}

void
ewmh_t::set_frame_extents(x_data::window_t win, bool overridden)
{
    static CARD32 frame_extents_normal[] = {
        0, // left
        0, // right
        BORDER_HEIGHT, // top
        0  // bottom
    };

    static CARD32 frame_extents_overridden[] = {
        0, // left
        0, // right
        0, // top
        0  // bottom
    };

    x_data::remove_property(win, "_NET_WM_FRAME_EXTENTS");
    for (::std::size_t i = 0; i < 4; ++i)
        x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_FRAME_EXTENTS",
            overridden ? frame_extents_overridden[i] : frame_extents_normal[i]});
}

void
ewmh_t::set_active_window_property(x_data::window_t win)
{
    x_data::replace_property<x_data::window_t>(x_data::g_root, {"_NET_ACTIVE_WINDOW", win});
}

void
ewmh_t::set_window_state_property(x_data::window_t win, netwmid_t i)
{
    if (range_t<int>::contains(netwmid_t::netwmstatefirst, netwmid_t::netwmstatelast, i))
        x_data::replace_property<x_data::atom_t>(win, {"_NET_WM_STATE", m_netwm_atoms[i]});
    else
        x_data::unset_property<x_data::atom_t>(win, {"_NET_WM_STATE"});
}

void
ewmh_t::set_window_state_property(x_data::window_t win, const ::std::string& state)
{
    x_data::replace_property<x_data::atom_t>(win,
        {"_NET_WM_STATE", x_data::get_atom("_NET_WM_STATE_" + state)});
}

void
ewmh_t::set_window_type_property(x_data::window_t win, const ::std::string& type)
{
    x_data::replace_property<x_data::atom_t>(win,
        {"_NET_WM_WINDOW_TYPE", x_data::get_atom("_NET_WM_WINDOW_TYPE_" + type)});
}

void
ewmh_t::set_wm_name_property(x_data::window_t win, const ::std::string& name)
{
    x_data::replace_property<x_data::string_t>(win, {"_NET_WM_NAME", name});
}

void
ewmh_t::set_class_property(x_data::window_t win, const ::std::string& inst, ::std::optional<const ::std::string> cls)
{
    x_data::replace_property<x_data::string_t>(win, {"WM_CLASS", inst + '\0' + (cls ? *cls : inst)});
}

void
ewmh_t::set_wm_desktop_property(x_data::window_t win, unsigned workspace_nr)
{
    x_data::replace_property<x_data::cardinal_t>(win, {"_NET_WM_DESKTOP", workspace_nr});
}

void
ewmh_t::set_wm_pid_property(x_data::window_t win, unsigned pid)
{
    x_data::replace_property<x_data::cardinal_t>(win, {"_NET_WM_PID", pid});
}

void
ewmh_t::set_desktop_geometry_property()
{
    auto root_attrs = x_data::get_attributes(x_data::g_root);
    x_data::replace_property<x_data::cardinal_t>(x_data::g_root,
        {"_NET_DESKTOP_GEOMETRY", (unsigned) root_attrs.w()});
    x_data::append_property<x_data::cardinal_t>(x_data::g_root,
        {"_NET_DESKTOP_GEOMETRY", (unsigned) root_attrs.h()});
}

void
ewmh_t::set_desktop_viewport_property()
{
    x_data::remove_property(x_data::g_root, "_NET_DESKTOP_VIEWPORT");
    for (::std::size_t i = 0; i < CONTEXTS.size() * USER_WORKSPACES.size(); ++i) {
        x_data::append_property<x_data::cardinal_t>(x_data::g_root, {"_NET_DESKTOP_VIEWPORT", 0});
        x_data::append_property<x_data::cardinal_t>(x_data::g_root, {"_NET_DESKTOP_VIEWPORT", 0});
    }
}

void
ewmh_t::set_workarea_property()
{
    auto root_attrs = x_data::get_attributes(x_data::g_root);
    x_data::remove_property(x_data::g_root, "_NET_WORKAREA");
    for (::std::size_t i = 0; i < CONTEXTS.size() * USER_WORKSPACES.size(); ++i) {
        x_data::append_property<x_data::cardinal_t>(x_data::g_root, {"_NET_WORKAREA",
            strut.left_window ? strut.left_width : SIDEBAR_WIDTH});
        x_data::append_property<x_data::cardinal_t>(x_data::g_root, {"_NET_WORKAREA",
            strut.top_height});
        x_data::append_property<x_data::cardinal_t>(x_data::g_root, {"_NET_WORKAREA",
            root_attrs.w() - (strut.left_window ? strut.left_width : SIDEBAR_WIDTH)});
        x_data::append_property<x_data::cardinal_t>(x_data::g_root, {"_NET_WORKAREA",
            root_attrs.h() - strut.top_height});
    }
}

void
ewmh_t::clear_client_list_property()
{
    x_data::remove_property(x_data::g_root, "_NET_CLIENT_LIST");
}

void
ewmh_t::clear_workarea_property()
{
    x_data::remove_property(x_data::g_root, "_NET_WORKAREA");
}

void
ewmh_t::clear_active_window_property()
{
    x_data::remove_property(x_data::g_root, "_NET_ACTIVE_WINDOW");
}

void
ewmh_t::set_strut_property(x_data::window_t win, unsigned left, unsigned right,
    unsigned top, unsigned bottom)
{
    x_data::remove_property(win, "_NET_WM_STRUT");
    x_data::remove_property(win, "_NET_WM_STRUT_PARTIAL");

    x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_STRUT", left});
    x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_STRUT", right});
    x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_STRUT", top});
    x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_STRUT", bottom});

    x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", left});
    x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", right});
    x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", top});
    x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", bottom});

    for (::std::size_t i = 0; i < 8; ++i)
        x_data::append_property<x_data::cardinal_t>(win, {"_NET_WM_STRUT_PARTIAL", 0});
}

bool
ewmh_t::check_apply_strut(x_data::window_t win)
{
    ::std::vector<unsigned long> partial_strut_vals;
    ::std::vector<unsigned long> strut_vals;

    if (x_data::has_property<x_data::cardinal_t>(win, "_NET_WM_STRUT_PARTIAL"))
        partial_strut_vals = x_data::get_property<x_data::cardinal_list_t>(win,
            {"_NET_WM_STRUT_PARTIAL"}).get_data().get();

    if (x_data::has_property<x_data::cardinal_list_t>(win, "_NET_WM_STRUT"))
        strut_vals = x_data::get_property<x_data::cardinal_list_t>(win,
            {"_NET_WM_STRUT"}).get_data().get();

    if (partial_strut_vals.empty() && strut_vals.empty())
        return false;

    if (partial_strut_vals.size() >= 4 && strut_vals.size() == 4)
        for (::std::size_t i = 0; i < 4; ++i)
            if (strut_vals[i] == 0)
                strut_vals[i] = partial_strut_vals[i];

    if (!strut.left_window
        && strut_vals.size() > 0 && strut_vals[0] > 0)
    {
        strut.left_width = strut_vals[0];
        strut.left_window = win;
    } else if (!strut.right_window
        && strut_vals.size() > 1 && strut_vals[1] > 0)
    {
        strut.right_width = strut_vals[1];
        strut.right_window = win;
    } else if (!strut.top_window
        && strut_vals.size() > 2 && strut_vals[2] > 0)
    {
        strut.top_height = strut_vals[2];
        strut.top_window = win;
    } else if (!strut.bottom_window
        && strut_vals.size() > 3 && strut_vals[3] > 0)
    {
        strut.bottom_height = strut_vals[3];
        strut.bottom_window = win;
    } else
        return false;

    set_workarea_property();
    return true;
}

bool
ewmh_t::check_release_strut(x_data::window_t win)
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


void
ewmh_t::register_to_list(x_data::window_t win)
{
    m_registered_windows.push_back(win);
    append_client_list_property(win);
}

void
ewmh_t::unregister_from_list(x_data::window_t win)
{
    erase_remove(m_registered_windows, win);
    set_client_list_property(m_registered_windows);
}

void
ewmh_t::append_client_list_property(x_data::window_t win)
{
    x_data::append_property<x_data::window_t>(x_data::g_root,
        {"_NET_CLIENT_LIST", win});
}

void
ewmh_t::set_client_list_property(::std::vector<x_data::window_t>& wins)
{
    clear_client_list_property();

    for (auto& win : wins)
        append_client_list_property(win);
}
