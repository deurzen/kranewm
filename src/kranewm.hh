#ifndef __KRANEWM__KRANEWM__GUARD__
#define __KRANEWM__KRANEWM__GUARD__

#include "x-data/display.hh"
#include "x-data/error.hh"

#include "ewmh.hh"
#include "ipc.hh"
#include "sidebar.hh"
#include "stack.hh"
#include "x-model.hh"
#include "changes.hh"
#include "client-model.hh"
#include "x-events.hh"
#include "client-events.hh"
#include "process.hh"

#include <csignal>
#include <memory>


typedef class kranewm_t* kranewm_ptr_t;
static kranewm_ptr_t g_instance;

class kranewm_t
{
public:
    kranewm_t()
      : m_running(true),
        m_ewmh(),
        m_ipc(),
        m_sidebar(m_ewmh),
        m_x(),
        m_changequeue(),
        m_clients(m_changequeue, m_ewmh, m_windowstack, m_processes),
        m_events(m_ewmh, m_ipc, m_sidebar, m_windowstack, m_processes, m_clients, m_x, &m_running),
        m_changes(m_changequeue, m_ewmh, m_sidebar, m_x, m_clients),
        m_lock({.x11_fd = x_data::g_connection, .ipc_fd = m_ipc.get_sock_fd()})
    {
        // global instance to enable stateful signal handling
        g_instance = this;
    }

    ~kranewm_t()
    {
        x_data::g_dpy.close();
    }

    void setup();
    void run();
    void exit();

    static ::std::unique_ptr<kranewm_t> init();

private:
    static int otherwmerror(Display*, XErrorEvent*);
    static int (*m_xerrorxlib)(Display*, XErrorEvent*);

    static void wait_children(int);
    static void handle_signal(int);

    void check_otherwm();
    void init_signals();

    bool
    ipc_received() const
    {
        return FD_ISSET(m_lock.ipc_fd, &m_lock.event_fds);
    }

    bool
    x11_received() const
    {
        return FD_ISSET(m_lock.x11_fd, &m_lock.event_fds);
    }

    bool
    reselect()
    {
        static int max_fd = ::std::max(m_lock.x11_fd, m_lock.ipc_fd);

        FD_ZERO(&m_lock.event_fds);
        FD_SET(m_lock.x11_fd, &m_lock.event_fds);

        if (m_ipc.is_enabled())
            FD_SET(m_lock.ipc_fd, &m_lock.event_fds);

        return (select(max_fd + 1, &m_lock.event_fds, NULL, NULL, NULL) > 0);
    }


    bool m_running;

    ewmh_t m_ewmh;
    ipc_t m_ipc;
    sidebar_t m_sidebar;
    windowstack_t m_windowstack;
    processjumplist_t m_processes;
    x_model_t m_x;
    changequeue_t m_changequeue;
    client_model_t m_clients;
    x_events_t m_events;
    client_events_t m_changes;

    struct {
        const int x11_fd, ipc_fd;
        fd_set event_fds;
    } m_lock;

};

#endif//__KRANEWM__KRANEWM__GUARD__
