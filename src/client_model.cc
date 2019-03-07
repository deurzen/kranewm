#include "client_model.hh"


client_ptr_t
client_model::win_to_client(x_wrapper::window_t win)
{
    for (auto&& [_win,_client] : m_client_windows)
        if (_win.get() == win.get())
            return _client;

    return nullptr;
}

workspace_ptr_t
client_model::client_workspace(client_ptr_t client)
{
    if (m_client_workspaces.count(client))
        return m_client_workspaces[client];

    return nullptr;
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
