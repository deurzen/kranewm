#include "client-model.hh"

#include "changes.hh"
#include "process.hh"
#include "util.hh"

#include "x-data/attributes.hh"


client_ptr_t
client_model_t::win_client(x_data::window_t win)
{
    if (m_client_windows.find(win) != m_client_windows.end())
        return m_client_windows.at(win);

    return nullptr;
}

context_ptr_t
client_model_t::client_context(client_ptr_t client)
{
    if (m_client_contexts.find(client) != m_client_contexts.end())
        return m_client_contexts.at(client);

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
        return m_client_workspaces.at(client);

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


bool
client_model_t::is_unstuck(client_ptr_t client)
{
    bool is_not_noninwindow_fullscreen = !(!client->in_window && client->fullscreen);
    bool is_nonsticky_floating = (!client->sticky && client_user_workspace(client)->in_float_layout());
    bool is_sticky_floating = (client->sticky && active_workspace()->in_float_layout());

    return is_not_noninwindow_fullscreen
        && (client->floating || (is_nonsticky_floating || is_sticky_floating));
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
        if (client_context(client->parent) != client_context(client)) {
            change_active_context(client_context(client->parent));
            m_client_contexts[client] = client_context(client->parent);
        }

        if (!client->parent->sticky) {
            if (is_user_workspace(client_workspace(client->parent))
                && client_workspace(client->parent) != client_workspace(client))
            {
                change_active_workspace(client_user_workspace(client->parent));
            }

            rule.workspace = m_client_workspaces.at(client->parent)->get_number();
        } else
            rule.workspace = 0;
    }

    if (rule.center)
        client->center();

    if (rule.autoclose)
        client->win.force_close();

    auto context = m_current_context;
    if (!rule.nohint && rule.context && (rule.context != context->get_number())) {
        context = m_contexts.at(rule.context - 1);
        if (!context->is_initialized())
            context->initialize();
    }

    if (!rule.nohint && rule.workspace) {
        auto workspace = (*context->get_workspaces()).at(rule.workspace - 1);
        m_client_workspaces[client] = workspace;
        workspace->add_family(client);
        m_changequeue.add(change_client_workspace(client, NOWORKSPACE, workspace));
    } else {
        m_client_workspaces[client] = context->get_activated();
        context->get_activated()->add_family(client).arrange();
        m_changequeue.add(change_client_workspace(client, NOWORKSPACE, context->get_activated()));
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
    if (client->iconified)
        set_iconified(client, clientaction_t::remove, false);

    if (client->disowned)
        set_disowned(client, clientaction_t::remove, false);

    if (client->sticky)
        set_sticky(client, clientaction_t::remove, false);

    auto context = client_context(client);
    auto workspace = client_workspace(client);

    if (client == context->get_marked())
        context->set_marked();

    if (is_moveresize_workspace(workspace)) {
        if (is_resize_workspace(workspace))
            stop_resizing(client);
        else if (is_move_workspace(workspace))
            stop_moving(client);

        workspace = client_user_workspace(client);
    }

    if (client->parent) {
        focus(client->parent);
        client->parent->disown_child(client);
    }

    workspace->remove_family(client);
    sync_workspace_focus();

    erase_find(m_client_windows, client->frame);
    erase_find(m_client_windows, client->win);
    erase_find(m_client_contexts, client);
    erase_find(m_client_workspaces, client);
    erase_find(m_fullscreen_clients, client);
    erase_find(m_disowned_clients, client);
    erase_remove(m_managed_windows, client->win);
    m_processes.remove_process(client);

    m_changequeue.add(change_client_destroy(client, workspace));
}

void
client_model_t::focus(client_ptr_t client, bool ignore_unwind)
{
    if (!client)
        return;

    auto parent = (client->parent) ? client->parent : client;
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
    if (is_unstuck(client))
        client_to_workspace(client, m_move_workspace);
}

void
client_model_t::start_resizing(client_ptr_t client)
{
    if (is_unstuck(client))
        client_to_workspace(client, m_resize_workspace);
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
    for (auto& workspace : *m_user_workspaces)
        for (auto& client : workspace->get_all()) {
            pos_t new_pos = client->float_pos;

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
    for (auto [client,state] : m_fullscreen_clients)
        if (client->in_window) {
            if (is_unstuck(client))
                client->moveresize(client->float_pos, client->float_dim);
            else if (client->sticky)
                active_workspace()->arrange();
            else
                client_user_workspace(client)->arrange();
        } else {
            state.fullscreen = client->fullscreen;
            m_changequeue.add(change_client_fullscreen(client, state));
        }
}

void
client_model_t::save_profile(::std::size_t profile_index)
{
    if (!range_t<::std::size_t>::contains(1, m_profiles.size(), profile_index))
        return;

    m_profiles.at(profile_index - 1).save(m_current_workspace);
}

void
client_model_t::apply_profile(::std::size_t profile_index)
{
    if (!range_t<::std::size_t>::contains(1, m_profiles.size(), profile_index))
        return;

    auto profile = m_profiles.at(profile_index - 1);
    if (profile.is_set()) {
        profile.apply(m_current_workspace);
        m_current_workspace->arrange();
    }
}

void
client_model_t::client_to_workspace(client_ptr_t client, ::std::size_t workspace_nr)
{
    if (client->parent || client->sticky)
        return;

    if (range_t<::std::size_t>::contains(1, USER_WORKSPACES.size(), workspace_nr))
        client_to_workspace(client, m_user_workspaces->at(workspace_nr - 1));
}

void
client_model_t::client_to_workspace(client_ptr_t client, workspace_ptr_t to)
{
    auto from = client_workspace(client);

    if (from == to)
        return;

    if (is_user_workspace(from))
        to->add_family(client);

    if (is_user_workspace(to)) {
        from->remove_family(client);
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
client_model_t::change_active_workspace(::std::size_t workspace_nr, bool save_prev)
{
    if (range_t<::std::size_t>::contains(1, USER_WORKSPACES.size(), workspace_nr))
        change_active_workspace(m_user_workspaces->at(workspace_nr - 1), save_prev);
    else
        change_active_workspace(nullptr, save_prev);
}

void
client_model_t::change_active_workspace(user_workspace_ptr_t workspace, bool save_prev)
{
    if ((!workspace && !(workspace = m_current_context->get_previous()))
        || m_current_workspace == workspace)
    {
        return;
    }

    if (m_move_workspace->is_set())
        stop_moving(m_move_workspace->get());

    if (m_resize_workspace->is_set())
        stop_resizing(m_resize_workspace->get());

    { // do not save successive {next,prev}-ws
        static bool prev_ignored = false;
        if (!prev_ignored) {
            m_current_context->set_previous(m_current_workspace);
            prev_ignored = true;
        }

        if (!workspace || save_prev) {
            m_current_context->set_previous(m_current_workspace);
            prev_ignored = false;
        }
    }

    m_changequeue.add(change_workspace_active(m_current_workspace, workspace));
    m_current_workspace = workspace;
    m_current_context->set_activated(workspace);
    sync_workspace_focus(true);
}

void
client_model_t::client_to_context(client_ptr_t client, ::std::size_t context_nr)
{
    if (client->parent)
        return;

    if (range_t<::std::size_t>::contains(1, CONTEXTS.size(), context_nr--)) {
        if (!*m_contexts.at(context_nr))
            m_contexts.at(context_nr)->initialize();

        client_to_context(client, m_contexts.at(context_nr));
    }
}

void
client_model_t::client_to_context(client_ptr_t client, context_ptr_t to)
{
    auto from = client_context(client);

    if (from == to)
        return;

    auto from_workspace = client_user_workspace(client);
    auto to_workspace = to->get_workspaces()->at(from_workspace->get_index());

    m_client_contexts[client] = to;
    for (auto& child : client->children)
        m_client_contexts[child] = to;

    if (client->sticky) {
        for (auto& to_workspace : *to->get_workspaces())
            to_workspace->add_family(client);
        for (auto& from_workspace : *from->get_workspaces())
            from_workspace->remove_family(client);
    } else {
        to_workspace->add_family(client);
        from_workspace->remove_family(client);
    }

    m_client_workspaces[client] = to_workspace;
    for (auto& child : client->children)
        m_client_workspaces[child] = to_workspace;

    m_changequeue.add(change_client_context(client, from, to,
        from_workspace, to_workspace));

    sync_workspace_focus();
}

void
client_model_t::change_active_context(::std::size_t context_nr)
{
    if (range_t<::std::size_t>::contains(1, CONTEXTS.size(), context_nr--)) {
        if (!*m_contexts.at(context_nr))
            m_contexts.at(context_nr)->initialize();

        change_active_context(m_contexts.at(context_nr));
    }
}

void
client_model_t::change_active_context(context_ptr_t context)
{
    if (!context || context == m_current_context)
        return;

    for (auto& client : m_sticky_clients) {
        context_ptr_t current_client_context = client_context(client);
        if (current_client_context == m_current_context) {
            client->expect = clientexpect_t::withdraw;
            client->unmap();
        } else if (current_client_context == context) {
            client->expect = clientexpect_t::map;
            client->map();
        }
    }

    m_user_workspaces = context->get_workspaces();
    m_changequeue.add(change_context_active(m_current_context, context));
    m_current_context = context;
    change_active_workspace(context->get_activated(), false);
}

void
client_model_t::set_fullscreen(client_ptr_t client, clientaction_t action)
{
    switch (action) {
    case clientaction_t::add:
        {
            if (client->disowned || m_fullscreen_clients.count(client))
                return;

            client->fullscreen = true;
            m_changequeue.add(change_client_fullscreen(client, *client));
            m_fullscreen_clients[client] = *client;

            if (!client->in_window)
                client_user_workspace(client)->raise_client(client);
        }
        break;
    case clientaction_t::remove:
        {
            if (!m_fullscreen_clients.count(client))
                return;

            client->fullscreen = false;
            m_changequeue.add(change_client_fullscreen(client, m_fullscreen_clients.at(client)));
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
client_model_t::set_inwindow(client_ptr_t client, clientaction_t action)
{
    switch (action) {
    case clientaction_t::add:
        client->in_window = true;
        break;
    case clientaction_t::remove:
        {
            client->in_window = false;

            if (client->fullscreen) {
                m_fullscreen_clients.at(client).float_pos = client->float_pos;
                m_fullscreen_clients.at(client).float_dim = client->float_dim;
            }
        }
        break;
    case clientaction_t::toggle:
        set_inwindow(client, client->in_window
            ? clientaction_t::remove : clientaction_t::add);
        return;
    default: break;
    }

    m_windowstack.apply(client_user_workspace(client));
}

void
client_model_t::set_above(client_ptr_t client, clientaction_t action)
{
    switch (action) {
    case clientaction_t::add:
        {
            if (client->disowned || m_above_clients.count(client))
                return;

            if (!client->in_window && client->fullscreen)
                set_fullscreen(client, clientaction_t::remove);

            if (client->below)
                set_below(client, clientaction_t::remove);

            client->above = true;
            m_changequeue.add(change_client_above(client, *client));
            m_above_clients[client] = *client;
            m_windowstack.apply(client_user_workspace(client));
            /* client_user_workspace(client)->raise_client(client); */
        }
        break;
    case clientaction_t::remove:
        {
            if (!m_above_clients.count(client))
                return;

            client->above = false;
            m_changequeue.add(change_client_above(client, m_above_clients.at(client)));
            erase_find(m_above_clients, client);
            m_windowstack.apply(client_user_workspace(client));
        }
        break;
    case clientaction_t::toggle:
        set_above(client, client->above
            ? clientaction_t::remove : clientaction_t::add);
        return;
    default: break;
    }

    m_windowstack.apply(client_user_workspace(client));
}

void
client_model_t::set_below(client_ptr_t client, clientaction_t action)
{
    switch (action) {
    case clientaction_t::add:
        {
            if (client->disowned || m_below_clients.count(client))
                return;

            if (!client->in_window && client->fullscreen)
                set_fullscreen(client, clientaction_t::remove);

            if (client->above)
                set_above(client, clientaction_t::remove);

            client->below = true;
            m_changequeue.add(change_client_below(client, *client));
            m_below_clients[client] = *client;
            m_windowstack.apply(client_user_workspace(client));
            /* client_user_workspace(client)->raise_client(client); */
        }
        break;
    case clientaction_t::remove:
        {
            if (!m_below_clients.count(client))
                return;

            client->below = false;
            m_changequeue.add(change_client_below(client, m_below_clients.at(client)));
            erase_find(m_below_clients, client);
            m_windowstack.apply(client_user_workspace(client));
        }
        break;
    case clientaction_t::toggle:
        set_below(client, client->below
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
client_model_t::set_iconified(client_ptr_t client, clientaction_t action, bool by_user)
{
    if (client->sticky)
        return;

    switch (action) {
    case clientaction_t::add:
        {
            auto workspace = client_user_workspace(client);
            client->iconified = true;
            workspace->add_icon(client);
            workspace->remove_family(client);

            if (by_user)
                m_changequeue.add(change_client_iconify(client));
        }
        break;
    case clientaction_t::remove:
        {
            auto workspace = client_user_workspace(client);
            client->iconified = false;
            workspace->add_family(client);
            workspace->remove_icon(client);

            if (by_user)
                m_changequeue.add(change_client_iconify(client));
        }
        break;
    case clientaction_t::toggle:
        set_iconified(client, client->iconified
            ? clientaction_t::remove : clientaction_t::add);
        return;
    default: break;
    }
}

void
client_model_t::set_disowned(client_ptr_t client, clientaction_t action, bool by_user)
{
    switch (action) {
    case clientaction_t::add:
        {
            if (m_disowned_clients.count(client) || client->sticky)
                return;

            if (!client->in_window && client->fullscreen)
                set_fullscreen(client, clientaction_t::remove);

            auto workspace = client_user_workspace(client);
            client->disowned = true;
            workspace->remove_family(client);
            workspace->add_disowned(client);
            m_windowstack.apply(workspace);
            m_disowned_clients[client] = *client;

            if (by_user)
                m_changequeue.add(change_client_disown(client, m_disowned_clients.at(client)));
        }
        break;
    case clientaction_t::remove:
        {
            if (!m_disowned_clients.count(client))
                return;

            auto workspace = client_user_workspace(client);
            client->disowned = false;
            workspace->remove_disowned(client);
            workspace->add_family(client);
            m_windowstack.apply(workspace);

            if (by_user)
                m_changequeue.add(change_client_disown(client, m_disowned_clients.at(client)));

            erase_find(m_disowned_clients, client);
        }
        break;
    case clientaction_t::toggle:
        set_disowned(client, client->disowned
            ? clientaction_t::remove : clientaction_t::add);
        return;
    default: break;
    }
}

void
client_model_t::set_sticky(client_ptr_t client, clientaction_t action, bool by_user, bool child_spawn)
{
    if (by_user && client->parent)
        return;

    switch (action) {
    case clientaction_t::add:
        {
            client->sticky = true;
            for (auto& workspace : *client_context(client)->get_workspaces())
                if (workspace != m_current_workspace) {
                    if (!child_spawn)
                        workspace->add_family(client);

                    m_changequeue.add(change_client_workspace(client, workspace, nullptr));
                    sync_workspace_focus();
            }

            m_changequeue.add(change_client_sticky(client, m_client_contexts.at(client)));
            m_sticky_clients.push_back(client);

            for (auto& child : client->children)
                set_sticky(child, clientaction_t::add, false, true);
        }
        break;
    case clientaction_t::remove:
        {
            client->sticky = false;
            for (auto& workspace : *client_context(client)->get_workspaces())
                if (workspace != m_current_workspace)
                    workspace->remove_family(client);

            m_changequeue.add(change_client_sticky(client, m_client_contexts.at(client)));
            m_client_workspaces[client] = m_current_workspace;
            erase_remove(m_sticky_clients, client);

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
    m_current_context->set_marked(client);
}

void
client_model_t::jump_marked()
{
    if (!m_current_context->get_marked())
        return;

    client_ptr_t to_jump_to = m_current_context->get_marked();
    if (to_jump_to == m_focused_client && m_current_context->get_jumped_from()
        && m_client_workspaces.find(m_current_context->get_jumped_from()) != m_client_workspaces.end())
    {
        to_jump_to = m_current_context->get_jumped_from();
    }

    if (to_jump_to == m_focused_client)
        return;

    if (m_current_context->get_marked() != m_focused_client)
        m_current_context->set_jumped_from(m_focused_client);

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

    auto context = client_context(client);
    if (context != active_context())
        change_active_context(context);

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
