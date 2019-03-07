#include "kranewm.hh"
#include "x_wrapper/input.hh"
#include "x_wrapper/event.hh"
#include "x_wrapper/attributes.hh"
#include "common.hh"
#include "util.hh"

#include <csignal>


int (*Kranewm::m_xerrorxlib)(Display*, XErrorEvent*);


::std::unique_ptr<Kranewm>
Kranewm::init()
{
    if (!x_wrapper::g_dpy || (x_wrapper::g_root.get() == None))
        die("unable to open display");

    return ::std::make_unique<Kranewm>();
}

void
Kranewm::setup()
{
    signal(SIGCHLD, SIG_IGN); // prevent zombies
    check_otherwm(); // make sure another wm is not already running

    x_wrapper::select_input(x_wrapper::g_root, ButtonPressMask | PointerMotionMask
        | StructureNotifyMask | SubstructureNotifyMask | SubstructureRedirectMask);

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);

    auto root_draw_win = x_wrapper::create_window(true);
    root_draw_win.resize({SIDEBAR_WIDTH, root_attrs.get().height}).move({0, 0});
    m_ewmh.set_strut_property(root_draw_win, SIDEBAR_WIDTH, 0, 0, 0);
    root_draw_win.set_background_color(REG_ROOT_BG_COLOR);
    root_draw_win.map();

    m_ewmh.clear_client_list_property();

    m_ewmh.set_wm_name_property(x_wrapper::g_root, WMNAME);
    m_ewmh.set_wm_name_property(root_draw_win, WMNAME);

    m_ewmh.set_supporting_wm_check_property(x_wrapper::g_root, root_draw_win);
    m_ewmh.set_supporting_wm_check_property(root_draw_win, root_draw_win);

    m_ewmh.set_desktop_geometry_property();
    m_ewmh.set_desktop_viewport_property();
    m_ewmh.set_workarea_property();

    m_ewmh.set_number_of_desktops_property(USER_WORKSPACES.size());
    m_ewmh.set_current_desktop_property(0);

    ::std::vector<::std::string> desktop_names;
    for (auto&& [_,name] : USER_WORKSPACES)
        desktop_names.push_back(name);
    m_ewmh.set_desktop_names_property(desktop_names);


    x_wrapper::sync(true);

#ifndef DEBUG
    ::std::system("cd ~/.kranewm; ./blocking_autostart.sh");
    ::std::system("cd ~/.kranewm; ./nonblocking_autostart.sh &");
#endif
}

void
Kranewm::run()
{
    while(true);
}

void
Kranewm::check_otherwm()
{
    m_xerrorxlib = x_wrapper::set_error_handler(otherwmerror);
    x_wrapper::select_input(x_wrapper::g_root, SubstructureRedirectMask);
    x_wrapper::sync(false);
    x_wrapper::set_error_handler(x_wrapper::g_xerror);
    x_wrapper::sync(false);
}

int
Kranewm::otherwmerror(Display*, XErrorEvent*)
{
    die("another window manager is already running");
    return -1;
}
