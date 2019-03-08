#include "client_model.hh"

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
    if (m_client_workspaces.find(client) != m_client_workspaces.end())
        return m_client_workspaces[client];

    return nullptr;
}

workspace_ptr_t
client_model::active_workspace() const
{
    return m_current_workspace;
}


void
client_model::register_client(client_ptr_t)
{
    /* auto &ptr = _object_cache[name]; */
    /* if (!ptr) ptr = new Object; */
    /* return ptr; */
}

void
client_model::unregister_client(client_ptr_t)
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
client_model::stop_moving(client_ptr_t client, Pos pos)
{

}

void
client_model::start_resizing(client_ptr_t client)
{

}

void
client_model::stop_resizing(client_ptr_t client, Pos pos, Size size)
{

}
