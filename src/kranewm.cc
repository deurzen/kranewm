#include "kranewm.hh"

#include "x-data/window.hh"
#include "x-data/event.hh"
#include "x-data/attributes.hh"

#include "common.hh"
#include "util.hh"
#include "workspace.hh"

#include <cstdlib>
#include <sstream>

#include <sys/wait.h>


int (*kranewm_t::m_xerrorxlib)(Display*, XErrorEvent*);


::std::unique_ptr<kranewm_t>
kranewm_t::init()
{
    if (!x_data::g_dpy || !x_data::g_root)
        die("unable to open display");

    return ::std::make_unique<kranewm_t>();
}

void
kranewm_t::setup()
{
    check_otherwm();
    init_signals();

    x_data::select_input(x_data::g_root, ButtonPressMask | PointerMotionMask
        | StructureNotifyMask | SubstructureNotifyMask | SubstructureRedirectMask);

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
    for (auto& win : existing_wins)
        m_events.register_window(win);

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
        ::std::system((configdir_ss.str() + ::std::string("./blocking_autostart")).c_str());
        ::std::system((configdir_ss.str() + ::std::string("./nonblocking_autostart &")).c_str());
#pragma GCC diagnostic pop
    }
#endif
}

void
kranewm_t::run()
{
    while (m_running) {
        if (reselect()) {
            if (ipc_received()) {
                m_ipc.handle_ipc();
                m_changes.process_queued_changes();
            }

            while (x_data::pending()) {
                m_events.step();
                m_changes.process_queued_changes();
            }
        }
    }

    x_data::sync(false);
}

void
kranewm_t::exit()
{
    m_running = false;
}


void
kranewm_t::wait_children(int sig)
{
    struct sigaction child_sa;

    memset(&child_sa, 0, sizeof(child_sa));
    child_sa.sa_handler = &kranewm_t::wait_children;

    sigaction(SIGCHLD, &child_sa,  NULL);
    while (waitpid(-1, 0, WNOHANG) > 0);
}

void
kranewm_t::handle_signal(int sig)
{
    if (sig == SIGHUP || sig == SIGINT || sig == SIGTERM)
        g_instance->exit();
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

void
kranewm_t::init_signals()
{
    struct sigaction child_sa, exit_sa, ignore_sa;

    memset(&child_sa, 0, sizeof(child_sa));
    child_sa.sa_handler = &kranewm_t::wait_children;

    memset(&exit_sa, 0, sizeof(exit_sa));
    exit_sa.sa_handler = &kranewm_t::handle_signal;

    memset(&ignore_sa, 0, sizeof(ignore_sa));
    ignore_sa.sa_handler = SIG_IGN;

    sigaction(SIGCHLD, &child_sa,  NULL);
    sigaction(SIGINT,  &exit_sa,   NULL);
    sigaction(SIGHUP,  &exit_sa,   NULL);
    sigaction(SIGINT,  &exit_sa,   NULL);
    sigaction(SIGTERM, &exit_sa,   NULL);
    sigaction(SIGPIPE, &ignore_sa, NULL);
}

int
kranewm_t::otherwmerror(Display*, XErrorEvent*)
{
    die("another window manager is already running");
    return -1;
}
