#include "client-model.hh"
#include "util.hh"

client_ptr_t
client_model_t::win_to_client(x_wrapper::window_t win)
{
    if (m_client_windows.find(win) != m_client_windows.end())
        return m_client_windows[win];

    return nullptr;
}

workspace_ptr_t
client_model_t::client_workspace(client_ptr_t client)
{
    if (m_move_workspace->get() == client)
        return m_move_workspace;

    if (m_resize_workspace->get() == client)
        return m_resize_workspace;

    return client_user_workspace(client);
}

user_workspace_ptr_t
client_model_t::client_user_workspace(client_ptr_t client)
{
    if (m_client_workspaces.find(client) != m_client_workspaces.end())
        return m_client_workspaces[client];

    return nullptr;
}

user_workspace_ptr_t
client_model_t::active_workspace() const
{
    return m_current_workspace;
}

client_ptr_t
client_model_t::focused_client() const
{
    return m_focused_client;
}


void
client_model_t::manage_client(client_ptr_t client, rule_t rule)
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
        auto workspace = m_user_workspaces[rule.workspace - 1];
        m_client_workspaces[client] = workspace;
        workspace->add_client(client);
        m_changequeue.add(change_client_workspace(client, nullptr, workspace));
    } else {
        m_client_workspaces[client] = m_current_workspace;
        m_current_workspace->add_client(client).arrange();
        m_changequeue.add(change_client_workspace(client, nullptr, m_current_workspace));
    }

    focus(client);
}

void
client_model_t::unmanage_client(client_ptr_t client)
{
    auto workspace = client_workspace(client);

    if (client == m_marked_client)
        m_marked_client = nullptr;

    if (client->parent) {
        focus(client->parent);
        client->parent->children.erase(client);
    }

    workspace->remove_client(client);
    sync_workspace_focus();

    erase_find(m_client_windows, client->win);
    erase_find(m_client_windows, client->frame);
    erase_find(m_client_workspaces, client);
    erase_remove(m_managed_windows, client->win);

    m_changequeue.add(change_client_destroy(client, workspace));
}

void
client_model_t::focus(client_ptr_t client)
{
    if (!client)
        return;

    auto parent = client;
    if (client->parent)
        parent = client->parent;

    if (parent && !m_current_workspace->contains(parent))
        return;

    m_current_workspace->set(client);
    m_changequeue.add(change_client_focus(m_focused_client, client));
    m_focused_client = client;
}

void
client_model_t::unfocus()
{
    if (!m_focused_client)
        return;

    m_current_workspace->unset();
    m_changequeue.add(change_client_focus(m_focused_client, nullptr));
    m_focused_client = nullptr;
}

void
client_model_t::unfocus_if_focused(client_ptr_t client)
{
    if (m_focused_client == client)
        unfocus();
}

void
client_model_t::start_moving(client_ptr_t client)
{
    client_to_workspace(client, m_move_workspace);
}

void
client_model_t::stop_moving(client_ptr_t client, pos_t pos)
{
    if (!is_move_workspace(client_workspace(client)))
        return;

    client_to_workspace(client, m_current_workspace);
}

void
client_model_t::start_resizing(client_ptr_t client)
{
    client_to_workspace(client, m_resize_workspace);
}

void
client_model_t::stop_resizing(client_ptr_t client, pos_t pos, dim_t dim)
{
    if (!is_resize_workspace(client_workspace(client)))
        return;

    client_to_workspace(client, m_current_workspace);
}

void
client_model_t::client_to_workspace(client_ptr_t client, unsigned workspace_nr)
{
    if (range_t<unsigned>::contains(1, USER_WORKSPACES.size(), workspace_nr))
        client_to_workspace(client, m_user_workspaces[workspace_nr - 1]);
}

void
client_model_t::client_to_workspace(client_ptr_t client, workspace_ptr_t to)
{
    auto from = client_workspace(client);

    if (from == to)
        return;

    to->add_client(client);

    auto _from = from;
    if (is_user_workspace(to)) {
        from->remove_client(client);
        m_client_workspaces[client] = user_workspace(to);
    } else
        _from = nullptr;

    if (is_moveresize_workspace(from))
        to = nullptr;

    m_changequeue.add(change_client_workspace(client, _from, to));
    sync_workspace_focus();
}

void
client_model_t::change_active_workspace(unsigned workspace_nr)
{
    if (range_t<unsigned>::contains(1, USER_WORKSPACES.size(), workspace_nr))
        change_active_workspace(m_user_workspaces[workspace_nr - 1]);
}

void
client_model_t::change_active_workspace(user_workspace_ptr_t workspace)
{
    if (m_current_workspace == workspace)
        return;

    m_changequeue.add(change_workspace_active(m_current_workspace, workspace));
    m_current_workspace = workspace;
    sync_workspace_focus();
}


void
client_model_t::sync_workspace_focus()
{
    if (!m_current_workspace->empty()) {
        if (m_current_workspace->get_focused() != m_focused_client)
            focus(m_current_workspace->get_focused());
    } else
        unfocus();
}
