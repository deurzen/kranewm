#include "kranec.hh"

#include "util.hh"

#include <cstdlib>

#include <sys/wait.h>


int (*kranec_t::m_xerrorxlib)(Display*, XErrorEvent*);

::std::unique_ptr<kranec_t>
kranec_t::init(int argc, char** argv)
{
    if (argc <= 1)
        die("no command provided");

    if (!x_data::g_dpy || !x_data::g_root)
        die("unable to open display");

    return ::std::make_unique<kranec_t>(argc, argv);
}

void
kranec_t::setup()
{
    x_data::set_error_handler(x_data::g_xerror);
    init_signals();
}

void
kranec_t::run()
{
    m_ipc.process_message();
}

void
kranec_t::exit()
{
    m_running = false;
}

void
kranec_t::wait_children(int sig)
{
    struct sigaction child_sa;

    memset(&child_sa, 0, sizeof(child_sa));
    child_sa.sa_handler = &kranec_t::wait_children;

    sigaction(SIGCHLD, &child_sa,  NULL);
    while (waitpid(-1, 0, WNOHANG) > 0);
}

void
kranec_t::handle_signal(int sig)
{
    if (sig == SIGHUP || sig == SIGINT || sig == SIGTERM)
        g_instance->exit();
}

void
kranec_t::check_kranewm()
{
    x_data::window_t sidebarwin = m_ewmh.get_supporting_wm_check_property();
    if (!sidebarwin)
        m_ipc.fail_ipc("no supported window manager running");
}

void
kranec_t::init_signals()
{
    struct sigaction child_sa, exit_sa, ignore_sa;

    memset(&child_sa, 0, sizeof(child_sa));
    child_sa.sa_handler = &kranec_t::wait_children;

    memset(&exit_sa, 0, sizeof(exit_sa));
    exit_sa.sa_handler = &kranec_t::handle_signal;

    memset(&ignore_sa, 0, sizeof(ignore_sa));
    ignore_sa.sa_handler = SIG_IGN;

    sigaction(SIGCHLD, &child_sa,  NULL);
    sigaction(SIGINT,  &exit_sa,   NULL);
    sigaction(SIGHUP,  &exit_sa,   NULL);
    sigaction(SIGINT,  &exit_sa,   NULL);
    sigaction(SIGTERM, &exit_sa,   NULL);
    sigaction(SIGPIPE, &ignore_sa, NULL);
}
