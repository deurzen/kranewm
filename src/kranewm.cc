#include "kranewm.hh"
#include "x-data/window.hh"
#include "x-data/event.hh"
#include "x-data/attributes.hh"
#include "common.hh"
#include "util.hh"
#include "workspace.hh"

#include <cstdlib>
#include <sstream>


int (*kranewm_t::m_xerrorxlib)(Display*, XErrorEvent*);


::std::unique_ptr<kranewm_t>
kranewm_t::init()
{
    if (!x_data::g_dpy || (x_data::g_root.get() == None))
        die("unable to open display");

    return ::std::make_unique<kranewm_t>();
}

void
kranewm_t::setup()
{
    check_otherwm(); // make sure another wm is not already running
    x_data::select_input(x_data::g_root, ButtonPressMask | PointerMotionMask
        | StructureNotifyMask | SubstructureNotifyMask | SubstructureRedirectMask
        | (m_ipc.is_enabled() * PropertyChangeMask));

    m_ewmh.set_wm_name_property(x_data::g_root, WMNAME);
    m_ewmh.set_class_property(x_data::g_root, WMNAME);
    m_ewmh.set_wm_pid_property(x_data::g_root, getpid());

    m_ewmh.clear_client_list_property();
    m_ewmh.set_desktop_geometry_property();
    m_ewmh.set_desktop_viewport_property();
    m_ewmh.set_workarea_property();

    m_ewmh.set_number_of_desktops_property(CONTEXTS.size() * USER_WORKSPACES.size());
    m_ewmh.set_current_desktop_property(0);

    ::std::vector<::std::string> desktop_names;
    for (auto&& [lr,_] : CONTEXTS)
        for (auto&& [nr,name] : USER_WORKSPACES)
            desktop_names.push_back(::std::string(1, lr)
                +::std::to_string(nr) + (!name.empty() ? ":" : "") + name);

    m_ewmh.set_desktop_names_property(desktop_names);

    auto existing_wins = x_data::get_top_level_windows();
    ::std::for_each(existing_wins.begin(), existing_wins.end(),
        [=](x_data::window_t win) { m_events.register_window(win); });

    m_changes.process_queued_changes();
    m_sidebar.draw();

    x_data::sync(true);

#ifndef DEBUG
    { // run user-configured autostart programs
        ::std::stringstream configdir_ss;
        if(const char* env_xdgconf = ::std::getenv("XDG_CONFIG_HOME"))
            configdir_ss << "cd " << env_xdgconf << "/" << WMNAME << ";";
        else
            configdir_ss << "cd ~/.config/" << WMNAME << ";";

        ::std::system((configdir_ss.str() + ::std::string("./blocking_autostart")).c_str());
        ::std::system((configdir_ss.str() + ::std::string("./nonblocking_autostart &")).c_str());
    }
#endif
}

void
kranewm_t::run()
{
    while(m_events.step())
        m_changes.process_queued_changes();

    x_data::sync(false);
}

void
kranewm_t::check_otherwm()
{
    m_xerrorxlib = x_data::set_error_handler(otherwmerror);
    x_data::select_input(x_data::g_root, SubstructureRedirectMask);
    x_data::sync(false);
    x_data::set_error_handler(x_data::g_xerror);
    x_data::sync(false);
}

int
kranewm_t::otherwmerror(Display*, XErrorEvent*)
{
    die("another window manager is already running");
    return -1;
}
