#ifndef __KRANEWM__X_EVENTS__GUARD__
#define __KRANEWM__X_EVENTS__GUARD__

#include "common.hh"
#include "input.hh"
#include "rule.hh"
#include "x-data/event.hh"

#define ALL ""
#define YES true
#define NO  false
#define CURRENT 0

// fwd decls
class ewmh_t;
class ipc_t;
class sidebar_t;
class windowstack_t;
class processjumplist_t;
class client_model_t;
class x_model_t;


class x_events_t final
{
public:
    explicit x_events_t(ewmh_t& ewmh, sidebar_t& sidebar, windowstack_t& windowstack,
        processjumplist_t& processes,  client_model_t& clients, x_model_t& x,
        inputhandler_t& input, ipc_t& ipc, bool* running)
      : m_ewmh(ewmh),
        m_sidebar(sidebar),
        m_windowstack(windowstack),
        m_clients(clients),
        m_x(x),
        m_input(input),
        m_ipc(ipc),
        m_running(running),
        m_rules({
            //  class                  inst          title      float center close nohint workspace context
            { { "Anki",                ALL,          ALL },   { YES,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
            { { "Artha",               ALL,          ALL },   { YES,  YES,    OFF, YES,   CURRENT,  CURRENT } },
            { { "Gimp",                ALL,          ALL },   { YES,   NO,    OFF,  NO,         4,  CURRENT } },
            { { "GoldenDict",          ALL,          ALL },   { YES,  YES,    OFF, YES,   CURRENT,  CURRENT } },
            { { "Gpick",               ALL,          ALL },   { YES,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
            { { "Inkscape",            ALL,          ALL },   { YES,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
            { { "Qalculate-gtk",       ALL,          ALL },   { YES,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
            { { "Rhythmbox",           ALL,          ALL },   { YES,  YES,   ONCE,  NO,   CURRENT,  CURRENT } },
            { { "cantata",             ALL,          ALL },   { YES,  YES,    OFF, YES,   CURRENT,  CURRENT } },
            { { "feh",                 ALL,          ALL },   {  NO,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
            { { "firefox",             ALL,          ALL },   {  NO,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
            { { "Firefox",             ALL,          ALL },   {  NO,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
            { { "gnucash",             ALL,          ALL },   { YES,   NO,    OFF,  NO,         5,  CURRENT } },
            { { "mate-volume-control", ALL,          ALL },   { YES,  YES,    OFF, YES,   CURRENT,  CURRENT } },
            { { "nixnote2",            ALL,          ALL },   { YES,  YES,    OFF, YES,   CURRENT,  CURRENT } },
            { { "noson",               ALL,          ALL },   { YES,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
            { { "noson-gui",           ALL,          ALL },   { YES,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
            { { "st-256color",         ALL,          ALL },   {  NO,  YES,    OFF,  NO,   CURRENT,  CURRENT } },
        })
    {}

    void step();
    void register_window(x_data::window_t);

private:
    void on_button_press();
    void on_button_release();
    void on_circulate_request();
    void on_client_message();
    void on_configure_notify();
    void on_configure_request();
    void on_destroy_notify();
    void on_expose();
    void on_focus_in();
    void on_key_press();
    void on_map_notify();
    void on_map_request();
    void on_mapping_notify();
    void on_motion_notify();
    void on_property_notify();
    void on_unmap_notify();

    ewmh_t& m_ewmh;
    sidebar_t& m_sidebar;
    windowstack_t& m_windowstack;
    client_model_t& m_clients;
    x_model_t& m_x;
    inputhandler_t& m_input;
    ipc_t& m_ipc;
    bool* m_running;
    x_data::event_t m_current_event;
    rules_t m_rules;

};

#undef ALL
#undef YES
#undef NO
#undef CURRENT

#endif//__KRANEWM__X_EVENTS__GUARD__
