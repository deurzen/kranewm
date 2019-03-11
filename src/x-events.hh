#ifndef __KRANEWM__X_EVENTS__GUARD__
#define __KRANEWM__X_EVENTS__GUARD__

#include "common.hh"
#include "input.hh"
#include "rule.hh"
#include "x-wrapper/event.hh"

#define ALL ""
#define YES true
#define NO  false
#define CURRENT 0

// Forward decls
class ewmh_t;
class client_model_t;
class x_model_t;


class x_events_t
{
public:
    explicit x_events_t(ewmh_t& ewmh, client_model_t& clients, x_model_t& x)
        : m_ewmh(ewmh),
          m_clients(clients),
          m_x(x),
          m_running(true),
          m_inputhandler(clients, m_running),
          m_rules({
              //  class            inst             title      float center icon close  workspace
              { { "Firefox",       ALL,             ALL },   {  NO,  YES,   NO,   OFF,  CURRENT } },
              { { "Gimp",          ALL,             ALL },   { YES,   NO,   NO,   OFF,  4       } },
              { { "Qalculate-gtk", ALL,             ALL },   { YES,  YES,   NO,   OFF,  CURRENT } },
              { { "Rhythmbox",     ALL,             ALL },   { YES,  YES,   NO,  ONCE,  CURRENT } },
              { { "RuneLite",      ALL,             ALL },   { YES,  YES,   NO,   OFF,  CURRENT } },
              { { "URxvt",         "kranewm:float", ALL },   { YES,  YES,   NO,   OFF,  CURRENT } },
              { { "URxvt",         ALL,             ALL },   {  NO,  YES,   NO,   OFF,  CURRENT } },
          }) {}

    bool step();
    void register_window(x_wrapper::window_t);

private:
    rule_t retrieve_rule(x_wrapper::window_t);

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
    void on_motion_notify();
    void on_property_notify();
    void on_unmap_notify();

    ewmh_t& m_ewmh;
    client_model_t& m_clients;
    x_model_t& m_x;
    bool m_running;
    inputhandler_t m_inputhandler;
    x_wrapper::event_t m_current_event;
    rules_t m_rules;

};

#undef ALL
#undef YES
#undef NO
#undef CURRENT

#endif//__KRANEWM__X_EVENTS__GUARD__
