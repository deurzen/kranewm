#ifndef __KRANEWM__CLIENT_EVENTS__GUARD__
#define __KRANEWM__CLIENT_EVENTS__GUARD__

#include "sidebar.hh"
#include "client-model.hh"

// fwd decls
class changequeue_t;
class ewmh_t;
class x_model_t;
typedef class clientchange_t* clientchange_ptr_t;
typedef class client_t* client_ptr_t;
typedef class workspace_t* workspace_ptr_t;


class client_events_t
{
public:
    client_events_t(changequeue_t& changes, ewmh_t& ewmh, sidebar_t& sidebar,
        x_model_t& x, client_model_t& clients)
        : m_current_change(nullptr),
          m_changes(changes),
          m_ewmh(ewmh),
          m_sidebar(sidebar),
          m_x(x),
          m_clients(clients)
    {
        m_sidebar.set_context(m_clients.active_context());
    }

    void process_queued_changes();

private:
    void on_change_client_focus();
    void on_change_client_destroy();
    void on_change_client_workspace();
    void on_change_client_fullscreen();
    void on_change_client_above();
    void on_change_client_below();
    void on_change_client_urgent();
    void on_change_client_iconify();
    void on_change_client_disown();
    void on_change_client_sticky();

    void on_change_workspace_active();
    void on_change_context_active();

    void from_move_workspace(client_ptr_t, workspace_ptr_t);
    void from_resize_workspace(client_ptr_t, workspace_ptr_t);
    void from_user_workspace(client_ptr_t, workspace_ptr_t, workspace_ptr_t);

    void to_move_workspace(client_ptr_t, workspace_ptr_t);
    void to_resize_workspace(client_ptr_t, workspace_ptr_t);
    void to_user_workspace(client_ptr_t, workspace_ptr_t, workspace_ptr_t);

    template <typename container_t> void map_all(container_t, bool = false);
    template <typename container_t> void unmap_all(container_t, bool = false);

    clientchange_ptr_t m_current_change;

    changequeue_t& m_changes;
    ewmh_t& m_ewmh;
    sidebar_t& m_sidebar;
    x_model_t& m_x;
    client_model_t& m_clients;

};

#endif//__KRANEWM__CLIENT_EVENTS__GUARD__
