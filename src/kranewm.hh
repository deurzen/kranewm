#ifndef __KRANEWM__KRANEWM__GUARD__
#define __KRANEWM__KRANEWM__GUARD__

#include "x-data/display.hh"
#include "x-data/error.hh"

#include "config.hh"
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

#include <memory>


class kranewm_t
{
public:
    kranewm_t()
        : m_config(),
          m_ewmh(),
          m_ipc(),
          m_sidebar(m_ewmh),
          m_x(),
          m_changequeue(),
          m_clients(m_changequeue, m_ewmh, m_windowstack, m_processes),
          m_events(m_ewmh, m_ipc, m_sidebar, m_windowstack, m_processes, m_clients, m_x),
          m_changes(m_changequeue, m_ewmh, m_sidebar, m_x, m_clients) {}

    void setup();
    void run();

    static ::std::unique_ptr<kranewm_t> init();

private:
    static int otherwmerror(Display*, XErrorEvent*);
    static int (*m_xerrorxlib)(Display*, XErrorEvent*);

    void check_otherwm();

    confighandler_t m_config;
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

};

#endif//__KRANEWM__KRANEWM__GUARD__
