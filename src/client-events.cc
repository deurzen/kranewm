#include "client-events.hh"

#include "changes.hh"
#include "ewmh.hh"
#include "sidebar.hh"
#include "floating.hh"
#include "x-model.hh"
#include "client-model.hh"

#include "x-data/event.hh"
#include "x-data/mouse.hh"


void
client_events_t::process_queued_changes()
{
    while ((m_current_change = m_changes.next())) {
        switch (m_current_change->type) {
        case change_t::client_focus:      on_change_client_focus();      break;
        case change_t::client_destroy:    on_change_client_destroy();    break;
        case change_t::client_fullscreen: on_change_client_fullscreen(); break;
        case change_t::client_above:      on_change_client_above();      break;
        case change_t::client_below:      on_change_client_below();      break;
        case change_t::client_urgent:     on_change_client_urgent();     break;
        case change_t::client_iconify:    on_change_client_iconify();    break;
        case change_t::client_disown:     on_change_client_disown();     break;
        case change_t::client_sticky:     on_change_client_sticky();     break;
        case change_t::client_workspace:  on_change_client_workspace();  break;
        case change_t::workspace_active:  on_change_workspace_active();  break;
        case change_t::context_active:    on_change_context_active();    break;
        default: break;
        }

        delete m_current_change;
    }
}


void
client_events_t::on_change_client_focus()
{
    auto change = change_client_focus(m_current_change);
    auto from   = change->from;
    auto to     = change->to;

    if (from) {
        m_ewmh.clear_active_window_property();
        if (from->disowned) from->frame.set_background_color(DIS_COLOR);
        else if (from->urgent) from->frame.set_background_color(URG_COLOR);
        else if (from->sticky) from->frame.set_background_color(REGSTICKY_COLOR);
        else from->frame.set_background_color(REG_COLOR);

        from->frame.grab();
    }

    if (to) {
        if (to->urgent)
            m_clients.set_urgent(to, clientaction_t::remove);

        if (x_data::set_input_focus(to->win)) {
            m_ewmh.set_active_window_property(to->win);
            if (to->sticky) to->frame.set_background_color(SELSTICKY_COLOR);
            else to->frame.set_background_color(SEL_COLOR);

            to->frame.ungrab();
            m_sidebar.draw_clientstate();
        } else {
            if (to->sticky) to->frame.set_background_color(REGSTICKY_COLOR);
            else to->frame.set_background_color(REG_COLOR);
            to->frame.grab();
        }
    } else {
        x_data::set_input_focus();
        m_sidebar.draw_clientstate();
    }
}

void
client_events_t::on_change_client_destroy()
{
    auto change    = change_client_destroy(m_current_change);
    auto client    = change->client;
    auto workspace = change->workspace;

    if (is_moveresize_workspace(workspace)) {
        x_data::release_pointer();
        m_x.exit_move_resize();
    }

    m_sidebar.draw();
    delete client;
}

void
client_events_t::on_change_client_fullscreen()
{
    auto change = change_client_fullscreen(m_current_change);
    auto client = change->client;
    auto former_state = change->former_state;

    if (client->fullscreen) {
        m_ewmh.set_window_state_property(client->win, "FULLSCREEN");
        auto root_attrs = x_data::get_attributes(x_data::g_root);
        auto workspace = m_clients.client_user_workspace(client);

        if (!client->in_window) {
            client->resize({root_attrs.w() - (workspace->has_sidebar() ? m_ewmh.get_left_strut() : 0)
                - m_ewmh.get_right_strut() - 2, root_attrs.h() + BORDER_HEIGHT}, true);

            client->move({(workspace->has_sidebar() ? m_ewmh.get_left_strut() : 0),
                -BORDER_HEIGHT - 1}, true);
        }
    } else {
        if (former_state.above)
            m_ewmh.set_window_state_property(client->win, "ABOVE");
        if (former_state.below)
            m_ewmh.set_window_state_property(client->win, "BELOW");
        else
            m_ewmh.set_window_state_property(client->win);

        client->floating = former_state.floating || client->disowned;
        client->float_pos = former_state.float_pos;
        client->float_dim = former_state.float_dim;
        client->pos = former_state.pos;
        client->dim = former_state.dim;

        if (!client->in_window) {
            if (client->floating
                || ((!client->sticky && m_clients.client_user_workspace(client)->in_float_layout())
                || (client->sticky && m_clients.active_workspace()->in_float_layout())))
            {
                client->resize(client->float_dim).move(client->float_pos);
            } else if (client->sticky)
                m_clients.active_workspace()->arrange();
            else
                m_clients.client_user_workspace(client)->arrange();
        }
    }
}

void
client_events_t::on_change_client_above()
{
    auto change = change_client_above(m_current_change);
    auto client = change->client;
    auto former_state = change->former_state;

    if (client->above) {
        m_ewmh.set_window_state_property(client->win, "ABOVE");
        client->set_float(clientaction_t::add).resize(client->float_dim).move(client->float_pos);
        m_clients.active_workspace()->raise_client(client);
        m_clients.active_workspace()->arrange();
    } else {
        m_ewmh.set_window_state_property(client->win);
        client->floating = former_state.floating;
        if (client->sticky)
            m_clients.active_workspace()->arrange();
        else
            m_clients.client_user_workspace(client)->arrange();
    }
}

void
client_events_t::on_change_client_below()
{
    auto change = change_client_below(m_current_change);
    auto client = change->client;
    auto former_state = change->former_state;

    if (client->below) {
        m_ewmh.set_window_state_property(client->win, "BELOW");
        client->set_float(clientaction_t::add).resize(client->float_dim).move(client->float_pos);
        m_clients.active_workspace()->raise_client(client);
        m_clients.active_workspace()->arrange();
    } else {
        m_ewmh.set_window_state_property(client->win);
        client->floating = former_state.floating;
        if (client->sticky)
            m_clients.active_workspace()->arrange();
        else
            m_clients.client_user_workspace(client)->arrange();
    }
}

void
client_events_t::on_change_client_urgent()
{
    auto change = change_client_urgent(m_current_change);
    auto client = change->client;

    if (client->focused) {
        client->urgent = false;
        client->frame.set_background_color(SEL_COLOR);
        m_clients.client_user_workspace(client)->erase_urgent();
        m_sidebar.draw_workspacenumbers();
    } else if (client->urgent) {
        client->frame.set_background_color(URG_COLOR);
        m_clients.client_user_workspace(client)->record_urgent();
        m_sidebar.draw_workspacenumbers();
    } else
        client->frame.set_background_color(REG_COLOR);

    x_data::wmhints_t hints = x_data::get_wmhints(client->win);
    if (hints.success())
        hints.get().flags = client->urgent
            ? (hints.get().flags | XUrgencyHint) : (hints.get().flags & ~XUrgencyHint);

    x_data::set_wmhints(client->win, hints);
}

void
client_events_t::on_change_client_iconify()
{
    auto change = change_client_iconify(m_current_change);
    auto client = change->client;

    if (client->iconified) {
        client->expect = clientexpect_t::iconify;
        client->unmap();
    } else {
        client->expect = clientexpect_t::deiconify;
        client->map();
    }

    m_clients.client_user_workspace(client)->arrange();
    m_clients.sync_workspace_focus();
    m_sidebar.draw();
}

void
client_events_t::on_change_client_disown()
{
    auto change = change_client_disown(m_current_change);
    auto client = change->client;
    auto former_state = change->former_state;

    if (client->disowned) {
        client->set_float(clientaction_t::add).resize(client->float_dim).move(client->float_pos);
        m_clients.active_workspace()->raise_client(client);
        m_clients.active_workspace()->arrange();
    } else {
        client->floating = former_state.floating;
        if (client->sticky)
            m_clients.active_workspace()->arrange();
        else
            m_clients.client_user_workspace(client)->arrange();
    }

    m_clients.sync_workspace_focus();
    m_sidebar.draw();
}

void
client_events_t::on_change_client_sticky()
{
    auto change    = change_client_sticky(m_current_change);
    auto client    = change->client;
    auto context   = change->context;

    if (client->sticky) {
        if (client->focused) client->frame.set_background_color(SELSTICKY_COLOR);
        else client->frame.set_background_color(REGSTICKY_COLOR);
        context->record_sticky();
    } else {
        if (client->focused) client->frame.set_background_color(SEL_COLOR);
        else client->frame.set_background_color(REG_COLOR);
        context->erase_sticky();
    }

    m_sidebar.draw_numbersticky();
    m_sidebar.draw_numberclients();
    m_sidebar.draw_workspacenumbers();
}

void
client_events_t::on_change_client_workspace()
{
    auto change = change_client_workspace(m_current_change);
    auto client = change->client;
    auto from   = change->from;
    auto to     = change->to;

    if (to)
        switch (to->get_type()) {
        case workspacetype_t::move:   to_move_workspace(client, to);       break;
        case workspacetype_t::resize: to_resize_workspace(client, to);     break;
        case workspacetype_t::user:   to_user_workspace(client, from, to); break;
        default: break;
        }

    if (from)
        switch (from->get_type()) {
        case workspacetype_t::move:   from_move_workspace(client, from);     break;
        case workspacetype_t::resize: from_resize_workspace(client, from);   break;
        case workspacetype_t::user:   from_user_workspace(client, from, to); break;
        default: break;
        }
    else
        client->frame.grab();
}

void
client_events_t::on_change_workspace_active()
{
    auto change = change_workspace_active(m_current_change);
    auto from   = change->from;
    auto to     = change->to;

    m_ewmh.set_current_desktop_property(to->get_number() - 1);

    { // circumvents X server race condition
        x_data::sync(false);
        x_data::event_t event;
        while (x_data::typed_event(event, UnmapNotify)) {
            x_data::window_t win = event.get().xunmap.window;
            client_ptr_t client = m_clients.win_client(win);

            if (client && !client->consume_expect(clientexpect_t::withdraw)) {
                client->unmap();
                client->win.reparent(client->pos);
                client->frame.destroy();
                x_data::sync(false);
            }
        }
    }

    map_all(to->get_all());
    map_all(to->get_disowned());

    unmap_all(from->get_all());
    unmap_all(from->get_disowned());

    to->arrange();
    m_sidebar.draw();
}

void
client_events_t::on_change_context_active()
{
    auto change = change_context_active(m_current_change);
    auto to     = change->to;

    m_sidebar.set_context(to);
    m_sidebar.draw();
}


void
client_events_t::from_move_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    map_all(client->children, true);
    x_data::release_pointer();
    m_x.exit_move_resize();
}

void
client_events_t::from_resize_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    map_all(client->children, true);
    x_data::release_pointer();
    m_x.exit_move_resize();
}

void
client_events_t::from_user_workspace(client_ptr_t client, workspace_ptr_t from, workspace_ptr_t to)
{
    auto current = m_clients.active_workspace();
    if (from == current && to != current) {
        client->expect = clientexpect_t::withdraw;
        client->unmap();
        m_clients.unfocus_if_focused(client);
        unmap_all(client->children);
    }

    m_sidebar.draw_workspacenumbers();
    m_sidebar.draw_numbersticky();
    m_sidebar.draw_numberclients();
}

void
client_events_t::to_move_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    unmap_all(client->children, true);
    m_x.enter_move(client, x_data::pointer_position());
    x_data::confine_pointer(m_x.moveresize()->indicator);
}

void
client_events_t::to_resize_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    unmap_all(client->children, true);
    m_x.enter_resize(client, x_data::pointer_position());
    x_data::confine_pointer(m_x.moveresize()->indicator);
}

void
client_events_t::to_user_workspace(client_ptr_t client, workspace_ptr_t from, workspace_ptr_t to)
{
    auto current = m_clients.active_workspace();
    if (from != current && to == current) {
        client->expect = clientexpect_t::map;
        client->map();
        map_all(client->children);
        m_clients.focus(client);
    }

    m_ewmh.set_wm_desktop_property(client->win, user_workspace(to)->get_number() - 1);
    m_sidebar.draw_workspacenumbers();
    m_sidebar.draw_numbersticky();
    m_sidebar.draw_numberclients();
}


template <typename container_t>
void
client_events_t::map_all(const container_t container, bool sticky_also)
{
    for (auto& c : container)
        if (sticky_also || !c->sticky){
            c->expect = clientexpect_t::map;
            c->map();
            map_all(c->children);
        }
}

template <typename container_t>
void
client_events_t::unmap_all(const container_t container, bool sticky_also)
{
    for (auto& c : container)
        if (sticky_also || !c->sticky) {
            c->expect = clientexpect_t::withdraw;
            m_clients.unfocus_if_focused(c);
            c->unmap();
            unmap_all(c->children);
        }
}
