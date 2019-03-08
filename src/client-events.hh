#ifndef __KRANEWM__CLIENT_EVENTS__GUARD__
#define __KRANEWM__CLIENT_EVENTS__GUARD__

#include "changes.hh"
#include "ewmh.hh"
#include "x-model.hh"
#include "client-model.hh"


class client_events_t
{
public:
    client_events_t(changequeue_t& changes, ewmh_t& ewmh, x_model_t& x, client_model_t& clients)
        : m_changes(changes), m_ewmh(ewmh), m_x(x), m_clients(clients) {}

    void process_queued_changes();

private:
    void start_moving(client_ptr_t);
    void start_resizing(client_ptr_t);

    changequeue_t& m_changes;
    ewmh_t& m_ewmh;
    x_model_t& m_x;
    client_model_t& m_clients;

};

#endif//__KRANEWM__CLIENT_EVENTS__GUARD__
