#include "client-model.hh"

client_ptr_t
client_model::win_to_client(x_wrapper::window_t win)
{
    if (m_client_windows.find(win) != m_client_windows.end())
        return m_client_windows[win];

    return nullptr;
}

workspace_ptr_t
client_model::client_workspace(client_ptr_t client)
{
    if (m_move_workspace->get() == client)
        return m_move_workspace;

    if (m_resize_workspace->get() == client)
        return m_resize_workspace;

    return client_user_workspace(client);
}

user_workspace_ptr_t
client_model::client_user_workspace(client_ptr_t client)
{
    if (m_client_workspaces.find(client) != m_client_workspaces.end())
        return m_client_workspaces[client];

    return nullptr;
}

user_workspace_ptr_t
client_model::active_workspace() const
{
    return m_current_workspace;
}

client_ptr_t
client_model::focused_client() const
{
    return m_focused_client;
}


void
client_model::manage_client(client_ptr_t client, rule rule)
{
    m_client_windows[client->frame] = client;
    m_client_windows[client->win]   = client;
    m_managed_windows.push_back(client->win);

    if (client->parent) {
        // deiconify parent
        if (is_user_workspace(client_workspace(client->parent))
            && client_workspace(client->parent) != client_workspace(client))
        {
            change_active_workspace(client_user_workspace(client->parent));
        }
    }

    if (rule.center)
        client->center();

    if (rule.workspace) {

    } else {
        m_client_workspaces[client] = m_current_workspace;
        m_current_workspace->add_client(client).arrange();
        client->map();
    }

}

void
client_model::unmanage_client(client_ptr_t)
{

}

void
client_model::focus(client_ptr_t client)
{

}

void
client_model::start_moving(client_ptr_t client)
{
    /* if (!(client->floating */
    /*     || )) */
    /*     return; */

    /* if (client->fullscreen) */
    /*     return; */

    /* if (!is_visible(client)) */
    /*     return; */

    /* if (move_workspace_->occupied() || resize_workspace_->occupied()) */
    /*     return; */

    /* client_to_workspace(client, move_workspace_); */
}

void
client_model::stop_moving(client_ptr_t client, pos_t pos)
{
// make sure in move_ws (x_events::map_window)

}

void
client_model::start_resizing(client_ptr_t client)
{

}

void
client_model::stop_resizing(client_ptr_t client, pos_t pos, dim_t size)
{
// make sure in resize_ws (x_events::map_window)

}

void
client_model::change_active_workspace(unsigned workspace_nr)
{

}

void
client_model::change_active_workspace(user_workspace_ptr_t workspace)
{

}
