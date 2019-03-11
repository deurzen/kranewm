#include "client-events.hh"
#include "x-wrapper/mouse.hh"


void
client_events_t::process_queued_changes()
{
    while ((m_current_change = m_changes.next())) {
        switch (m_current_change->type) {
        case change_t::client_focus:     on_change_client_focus();     break;
        case change_t::client_destroy:   on_change_client_destroy();   break;
        case change_t::client_workspace: on_change_client_workspace(); break;
        case change_t::workspace_active: on_change_workspace_active(); break;
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
        if (from->urgent) from->frame.set_background_color(URG_COLOR);
        else from->frame.set_background_color(REG_COLOR);

        // TODO floating indicator unmap

        from->frame.grab();
        if (from->shaded) {
            x_wrapper::select_input(from->frame, SHADED_FRAME_SELECTION);
            from->frame.set_background_color(REG_SHADE_COLOR);
            // TODO draw shaded frame title
        } else
            x_wrapper::select_input(from->frame, REG_FRAME_SELECTION);
    }

    if (to) {
        if (x_wrapper::set_input_focus(to->win)) {
            m_ewmh.set_active_window_property(to->win);
            to->frame.set_background_color(SEL_COLOR);
            to->frame.ungrab();
        } else {
            to->frame.set_background_color(REG_COLOR);
            to->frame.grab();
        }

        // TODO floating indicator map

        if (to->shaded) to->frame.set_background_color(SEL_SHADE_COLOR);
    } else
        x_wrapper::set_input_focus();
}

void
client_events_t::on_change_client_destroy()
{

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

    if (to)
        to->arrange();
}

void
client_events_t::on_change_workspace_active()
{
    auto change = change_workspace_active(m_current_change);
    auto from   = change->from;
    auto to     = change->to;

    m_ewmh.set_current_desktop_property(to->get_number() - 1);

    map_all(to->get_all());
    unmap_all(from->get_all());

    to->arrange();

    // TODO iconified clients
}


void
client_events_t::from_move_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    map_all(client->children);
    x_wrapper::release_pointer();
    m_x.exit_move_resize();
}

void
client_events_t::from_resize_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    map_all(client->children);
    x_wrapper::release_pointer();
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
}

void
client_events_t::to_move_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    unmap_all(client->children);
    m_x.enter_move(client, x_wrapper::pointer_position());
    x_wrapper::confine_pointer(m_x.moveresize()->indicator);
}

void
client_events_t::to_resize_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    unmap_all(client->children);
    m_x.enter_resize(client, x_wrapper::pointer_position());
    x_wrapper::confine_pointer(m_x.moveresize()->indicator);
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
        m_ewmh.set_window_state_property(client->win);
    }

    m_ewmh.set_wm_desktop_property(client->win, user_workspace(to)->get_number() - 1);
}


template <typename container_t>
void
client_events_t::map_all(const container_t container)
{
    for (auto& c : container) {
        c->expect = clientexpect_t::map;
        c->map();
        map_all(c->children);
    }
}

template <typename container_t>
void
client_events_t::unmap_all(const container_t container)
{
    for (auto& c : container) {
        c->expect = clientexpect_t::withdraw;
        m_clients.unfocus_if_focused(c);
        c->unmap();
        unmap_all(c->children);
    }
}
