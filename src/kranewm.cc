#include "kranewm.hh"
#include "x_wrapper/input.hh"
#include "x_wrapper/event.hh"
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


}

void
Kranewm::run()
{

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
