#include "client-model.hh"

#include "changes.hh"
#include "process.hh"
#include "util.hh"

#include "x-data/attributes.hh"


client_ptr_t
client_model_t::win_to_client(x_data::window_t win)
{
    if (m_client_windows.find(win) != m_client_windows.end())
        return m_client_windows[win];

    return nullptr;
}

context_ptr_t
client_model_t::client_context(client_ptr_t client)
{
    if (m_client_contexts.find(client) != m_client_contexts.end())
        return m_client_contexts[client];

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

context_ptr_t
client_model_t::active_context() const
{
    return m_current_context;
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
    m_client_contexts[client]       = m_current_context;
    m_managed_windows.push_back(client->win);
    m_processes.add_process(client);

    if (client->parent) {
        if (!client->parent->sticky) {
            if (is_user_workspace(client_workspace(client->parent))
                && client_workspace(client->parent) != client_workspace(client))
            {
                change_active_workspace(client_user_workspace(client->parent));
            }

            rule.workspace = m_client_workspaces[client->parent]->get_number();
        } else
            rule.workspace = 0;
    }

    if (rule.center)
        client->center();

    if (rule.autoclose)
        client->win.force_close();

    if (!rule.nohint && rule.workspace) {
        auto workspace = m_user_workspaces[rule.workspace - 1];
        m_client_workspaces[client] = workspace;
        workspace->add_client(client);
        m_changequeue.add(change_client_workspace(client, nullptr, workspace));
    } else {
        m_client_workspaces[client] = m_current_workspace;
        m_current_workspace->add_client(client).arrange();
        m_changequeue.add(change_client_workspace(client, nullptr, m_current_workspace));
    }

    if (rule.fullscreen)
        set_fullscreen(client, clientaction_t::add);

    if (client->parent && client->parent->sticky)
        set_sticky(client, clientaction_t::add, false);

    focus(client);
}

void
client_model_t::unmanage_client(client_ptr_t client)
{
    if (client->sticky)
        set_sticky(client, clientaction_t::remove, false);

    auto workspace = client_workspace(client);

    if (client == m_marked_client)
        m_marked_client = nullptr;

    if (client->parent) {
        focus(client->parent);
        client->parent->disown_child(client);
    }

    if (is_moveresize_workspace(workspace)) {
        if (is_resize_workspace(workspace))
            stop_resizing(client);
        else if (is_move_workspace(workspace))
            stop_moving(client);

        workspace = client_user_workspace(client);
    }

    workspace->remove_client(client);
    sync_workspace_focus();

    erase_find(m_client_windows, client->frame);
    erase_find(m_client_windows, client->win);
    erase_find(m_client_contexts, client);
    erase_find(m_client_workspaces, client);
    erase_find(m_fullscreen_clients, client);
    erase_remove(m_managed_windows, client->win);
    m_processes.remove_process(client);

    m_changequeue.add(change_client_destroy(client, workspace));
}

void
client_model_t::focus(client_ptr_t client, bool ignore_unwind)
{
    if (!client)
        return;

    auto parent = client;
    if (client->parent)
        parent = client->parent;

    if (parent && !m_current_workspace->contains(parent))
        return;

    m_current_workspace->set_focused(client, ignore_unwind);
    m_changequeue.add(change_client_focus(m_focused_client, client));

    client->focused = true;
    if (m_focused_client && m_focused_client != client)
        m_focused_client->focused = false;

    m_windowstack.apply(m_current_workspace);
    m_focused_client = client;
    m_processes.relayer_process(client);
}

void
client_model_t::unfocus()
{
    if (!m_focused_client)
        return;

    m_current_workspace->unset_focused();
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
client_model_t::cycle_focus_forward()
{
    m_current_workspace->forward();
    sync_workspace_focus();
}

void
client_model_t::cycle_focus_backward()
{
    m_current_workspace->backward();
    sync_workspace_focus();
}

void
client_model_t::start_moving(client_ptr_t client)
{
    if (!client->fullscreen && (client->floating
        || ((!client->sticky && client_user_workspace(client)->in_float_layout())
        || (client->sticky && active_workspace()->in_float_layout()))))
    {
        client_to_workspace(client, m_move_workspace);
    }
}

void
client_model_t::start_resizing(client_ptr_t client)
{
    if (!client->fullscreen && (client->floating
        || ((!client->sticky && client_user_workspace(client)->in_float_layout())
        || (client->sticky && active_workspace()->in_float_layout()))))
    {
        client_to_workspace(client, m_resize_workspace);
    }
}

void
client_model_t::stop_moving(client_ptr_t client, pos_t pos)
{
    if (!is_move_workspace(client_workspace(client)))
        return;

    client_to_workspace(client, m_current_workspace);
}

void
client_model_t::stop_resizing(client_ptr_t client, pos_t pos, dim_t dim)
{
    if (!is_resize_workspace(client_workspace(client)))
        return;

    client_to_workspace(client, m_current_workspace);
}

void
client_model_t::wedge_clients()
{
    auto root_attrs = x_data::get_attributes(x_data::g_root);
    for (auto& workspace : m_user_workspaces)
        for (auto& client : workspace->get_all()) {
            pos_t new_pos = client->float_pos;;
            if (root_attrs.h() < (client->float_pos.y + client->float_dim.h))
                new_pos.y = root_attrs.h() - client->float_dim.h;
            if (root_attrs.w() < (client->float_pos.x + client->float_dim.w))
                new_pos.x = root_attrs.w() - client->float_dim.w;
            if (!(new_pos == client->float_pos))
                client->move(new_pos);
        }
}

void
client_model_t::refullscreen_clients()
{
    for (auto [client,state] : m_fullscreen_clients) {
        state.fullscreen = client->fullscreen;
        m_changequeue.add(change_client_fullscreen(client, state));
    }
}

void
client_model_t::client_to_workspace(client_ptr_t client, unsigned workspace_nr)
{
    if (client->parent || client->sticky)
        return;

    if (range_t<unsigned>::contains(1, USER_WORKSPACES.size(), workspace_nr))
        client_to_workspace(client, m_user_workspaces[workspace_nr - 1]);
}

void
client_model_t::client_to_workspace(client_ptr_t client, workspace_ptr_t to)
{

    auto from = client_workspace(client);

    if (from == to)
        return;

    if (is_user_workspace(from))
        to->add_client(client);

    if (is_user_workspace(to)) {
        from->remove_client(client);
        m_client_workspaces[client] = user_workspace(to);
        for (auto& child : client->children)
            m_client_workspaces[child] = user_workspace(to);
    }

    m_changequeue.add(change_client_workspace(client,
        (is_user_workspace(to) ? from : nullptr),
        (is_user_workspace(from) ? to : nullptr)));

    sync_workspace_focus();
}

void
client_model_t::change_active_workspace(unsigned workspace_nr, bool save_prev)
{
    if (range_t<unsigned>::contains(1, USER_WORKSPACES.size(), workspace_nr))
        change_active_workspace(m_user_workspaces[workspace_nr - 1], save_prev);
}

void
client_model_t::change_active_workspace(user_workspace_ptr_t workspace, bool save_prev)
{
    static user_workspace_ptr_t prev_workspace = nullptr;

    if ((!workspace && !(workspace = prev_workspace)) || m_current_workspace == workspace)
        return;

    if (m_move_workspace->is_set())
        stop_moving(m_move_workspace->get());

    if (m_resize_workspace->is_set())
        stop_resizing(m_resize_workspace->get());

    { // do not save successive {next,prev}-ws
        static bool prev_ignored = false;
        if (!prev_ignored) {
            prev_workspace = m_current_workspace;
            prev_ignored = true;
        }

        if (!workspace || save_prev) {
            prev_workspace = m_current_workspace;
            prev_ignored = false;
        }
    }

    m_changequeue.add(change_workspace_active(m_current_workspace, workspace));
    m_current_workspace = workspace;
    m_current_context->set_activated(workspace);
    sync_workspace_focus(true);
}

void
client_model_t::client_to_context(client_ptr_t client, unsigned context_nr)
{
    if (client->parent)
        return;

    if (range_t<unsigned>::contains(1, CONTEXTS.size(), context_nr--)) {
        if (!m_contexts[context_nr])
            m_contexts[context_nr]->initialize();

        client_to_context(client, m_contexts[context_nr]);
    }
}

void
client_model_t::client_to_context(client_ptr_t client, context_ptr_t context)
{

}

void
client_model_t::change_active_context(unsigned context_nr)
{
    if (range_t<unsigned>::contains(1, CONTEXTS.size(), context_nr--)) {
        if (!*m_contexts[context_nr])
            m_contexts[context_nr]->initialize();

        change_active_context(m_contexts[context_nr]);
    }
}

void
client_model_t::change_active_context(context_ptr_t context)
{
    if (context == m_current_context)
        return;

    m_current_context = context;
    m_user_workspaces = context->get_workspaces();
    user_workspace_ptr_t to = context->get_activated();
    change_active_workspace(to, false);
}

void
client_model_t::set_fullscreen(client_ptr_t client, clientaction_t action)
{
    switch (action) {
    case clientaction_t::add:
        {
            if (m_fullscreen_clients.count(client))
                return;

            client->fullscreen = true;
            m_changequeue.add(change_client_fullscreen(client, *client));
            m_fullscreen_clients[client] = *client;
            client_user_workspace(client)->raise_client(client);
        }
        break;
    case clientaction_t::remove:
        {
            if (!m_fullscreen_clients.count(client))
                return;

            client->fullscreen = false;
            m_changequeue.add(change_client_fullscreen(client, m_fullscreen_clients[client]));
            erase_find(m_fullscreen_clients, client);
        }
        break;
    case clientaction_t::toggle:
        set_fullscreen(client, client->fullscreen
            ? clientaction_t::remove : clientaction_t::add);
        return;
    default: break;
    }

    m_windowstack.apply(client_user_workspace(client));
}

void
client_model_t::set_urgent(client_ptr_t client, clientaction_t action)
{
    switch (action) {
    case clientaction_t::add:
        {
            client->urgent = true;
            m_changequeue.add(change_client_urgent(client));
        }
        break;
    case clientaction_t::remove:
        {
            client->urgent = false;
            m_changequeue.add(change_client_urgent(client));
        }
        break;
    case clientaction_t::toggle:
        set_urgent(client, client->urgent
            ? clientaction_t::remove : clientaction_t::add);
        return;
    default: break;
    }
}

void
client_model_t::set_sticky(client_ptr_t client, clientaction_t action, bool by_user)
{
    if (by_user && client->parent)
        return;

    switch (action) {
    case clientaction_t::add:
        {
            client->sticky = true;
            for (auto& workspace : m_user_workspaces)
                if (workspace != m_current_workspace) {
                    workspace->add_client(client);
                    m_changequeue.add(change_client_workspace(client,
                        workspace, nullptr));
                    sync_workspace_focus();
            }

            m_changequeue.add(change_client_sticky(client, m_client_contexts[client]));

            for (auto& child : client->children)
                set_sticky(child, clientaction_t::add, false);
        }
        break;
    case clientaction_t::remove:
        {
            client->sticky = false;
            for (auto& workspace : m_user_workspaces)
                if (workspace != m_current_workspace)
                    workspace->remove_client(client);

            m_changequeue.add(change_client_sticky(client, m_client_contexts[client]));
            m_client_workspaces[client] = m_current_workspace;

            for (auto& child : client->children)
                set_sticky(child, clientaction_t::remove, false);
        }
        break;
    case clientaction_t::toggle:
        set_sticky(client, client->sticky
            ? clientaction_t::remove : clientaction_t::add);
        return;
    default: break;
    }
}

void
client_model_t::set_marked(client_ptr_t client)
{
    m_marked_client = client;
}

void
client_model_t::jump_marked()
{
    static client_ptr_t prev_focused_client = nullptr;

    if (!m_marked_client)
        return;

    client_ptr_t to_jump_to = m_marked_client;
    if (m_marked_client == m_focused_client && prev_focused_client
        && m_client_workspaces.find(prev_focused_client) != m_client_workspaces.end())
    {
        to_jump_to = prev_focused_client;
    }

    if (to_jump_to == m_focused_client)
        return;

    if (m_marked_client != m_focused_client)
        prev_focused_client = m_focused_client;

    if (!is_user_workspace(client_workspace(to_jump_to)))
        return;

    auto workspace = client_user_workspace(to_jump_to);
    if (workspace != m_current_workspace)
        change_active_workspace(workspace);

    focus(to_jump_to);
    sync_workspace_focus();
}

void
client_model_t::jump_process(const ::std::string& name)
{
    auto process = m_processes.get_process(name);
    if (!process)
        return;

    auto client = process->target;
    if (client == focused_client())
        client = process->prev;

    if (!client || !client_workspace(client)
            || !is_user_workspace(client_workspace(client)))
    {
        return;
    }

    process->prev = focused_client();

    auto workspace = client_user_workspace(client);
    if (workspace != active_workspace())
        change_active_workspace(workspace);

    focus(client);
    sync_workspace_focus();
}


void
client_model_t::sync_workspace_focus(bool ignore_unwind)
{
    if (!m_current_workspace->empty()) {
        if (m_current_workspace->get_focused() != m_focused_client)
            focus(m_current_workspace->get_focused(), ignore_unwind);
    } else
        unfocus();
}
