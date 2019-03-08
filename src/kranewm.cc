#include "kranewm.hh"
#include "x-wrapper/input.hh"
#include "x-wrapper/window.hh"
#include "x-wrapper/event.hh"
#include "x-wrapper/attributes.hh"
#include "common.hh"
#include "util.hh"
#include "workspace.hh"

#include <csignal>


int (*kranewm_t::m_xerrorxlib)(Display*, XErrorEvent*);


::std::unique_ptr<kranewm_t>
kranewm_t::init()
{
    if (!x_wrapper::g_dpy || (x_wrapper::g_root.get() == None))
        die("unable to open display");

    return ::std::make_unique<kranewm_t>();
}

void
kranewm_t::setup()
{
    signal(SIGCHLD, SIG_IGN); // prevent zombies
    check_otherwm(); // make sure another wm is not already running

    x_wrapper::select_input(x_wrapper::g_root, ButtonPressMask | PointerMotionMask
        | StructureNotifyMask | SubstructureNotifyMask | SubstructureRedirectMask);

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);

    auto sidebar = x_wrapper::create_window(true);
    sidebar.resize({SIDEBAR_WIDTH, root_attrs.get().height}).move({0, 0});
    m_ewmh.set_strut_property(sidebar, SIDEBAR_WIDTH, 0, 0, 0);
    sidebar.set_background_color(SIDEBAR_BG_COLOR);
    sidebar.map();

    m_ewmh.clear_client_list_property();

    m_ewmh.set_wm_name_property(x_wrapper::g_root, WMNAME);
    m_ewmh.set_wm_name_property(sidebar, WMNAME);

    m_ewmh.set_supporting_wm_check_property(x_wrapper::g_root, sidebar);
    m_ewmh.set_supporting_wm_check_property(sidebar, sidebar);

    m_ewmh.set_desktop_geometry_property();
    m_ewmh.set_desktop_viewport_property();
    m_ewmh.set_workarea_property();

    m_ewmh.set_number_of_desktops_property(USER_WORKSPACES.size());
    m_ewmh.set_current_desktop_property(0);

    ::std::vector<::std::string> desktop_names;
    for (auto&& [_,name] : USER_WORKSPACES)
        desktop_names.push_back(name);
    m_ewmh.set_desktop_names_property(desktop_names);

    auto existing_wins = x_wrapper::get_top_level_windows();
    ::std::for_each(existing_wins.begin(), existing_wins.end(),
        [=](x_wrapper::window_t win) { m_events.register_window(win); });

    // TODO process_client_events?
    x_wrapper::sync(true);

#ifndef DEBUG
    ::std::system("cd ~/.kranewm; ./blocking_autostart.sh");
    ::std::system("cd ~/.kranewm; ./nonblocking_autostart.sh &");
#endif
}

void
kranewm_t::run()
{
    while(m_events.step());
    x_wrapper::sync(false);
}

void
kranewm_t::check_otherwm()
{
    m_xerrorxlib = x_wrapper::set_error_handler(otherwmerror);
    x_wrapper::select_input(x_wrapper::g_root, SubstructureRedirectMask);
    x_wrapper::sync(false);
    x_wrapper::set_error_handler(x_wrapper::g_xerror);
    x_wrapper::sync(false);
}

int
kranewm_t::otherwmerror(Display*, XErrorEvent*)
{
    die("another window manager is already running");
    return -1;
}
