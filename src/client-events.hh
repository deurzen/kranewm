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
        : m_current_change(nullptr), m_changes(changes), m_ewmh(ewmh), m_x(x), m_clients(clients) {}

    void process_queued_changes();

private:
    void on_change_client_focus();
    void on_change_client_destroy();
    void on_change_client_workspace();

    void on_change_workspace_active();

    void from_move_workspace(client_ptr_t, workspace_ptr_t);
    void from_resize_workspace(client_ptr_t, workspace_ptr_t);
    void from_user_workspace(client_ptr_t, workspace_ptr_t, workspace_ptr_t);

    void to_move_workspace(client_ptr_t, workspace_ptr_t);
    void to_resize_workspace(client_ptr_t, workspace_ptr_t);
    void to_user_workspace(client_ptr_t, workspace_ptr_t, workspace_ptr_t);

    void map_all(const ::std::set<client_ptr_t>&);
    void unmap_all(const ::std::set<client_ptr_t>&);

    clientchange_ptr_t m_current_change;

    changequeue_t& m_changes;
    ewmh_t& m_ewmh;
    x_model_t& m_x;
    client_model_t& m_clients;

};

#endif//__KRANEWM__CLIENT_EVENTS__GUARD__
