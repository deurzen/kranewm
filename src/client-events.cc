#include "client-events.hh"
#include "x-wrapper/mouse.hh"
#include "x-wrapper/input.hh"


void
client_events_t::process_queued_changes()
{
    while ((m_current_change = m_changes.next())) {
        switch (m_current_change->type) {
        case CLIENT_FOCUS_CHANGE:     on_change_client_focus();     break;
        case CLIENT_DESTROY_CHANGE:   on_change_client_destroy();   break;
        case CLIENT_WORKSPACE_CHANGE: on_change_client_workspace(); break;
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

    if (from)
        switch (from->get_type()) {
        case MOVE_WORKSPACE:   from_move_workspace(client, from);   break;
        case RESIZE_WORKSPACE: from_resize_workspace(client, from); break;
        case USER_WORKSPACE:   from_user_workspace(client, from);   break;
        default: break;
        }

    if (to) {
        switch (to->get_type()) {
        case MOVE_WORKSPACE:   to_move_workspace(client, to);       break;
        case RESIZE_WORKSPACE: to_resize_workspace(client, to);     break;
        case USER_WORKSPACE:   to_user_workspace(client, to);       break;
        default: break;
        }

        to->arrange();
    }
}


void
client_events_t::from_move_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    for (auto& child : client->children)
        child->map();
    x_wrapper::release_pointer();
    m_x.exit_move_resize();
}

void
client_events_t::from_resize_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    for (auto& child : client->children)
        child->map();
    x_wrapper::release_pointer();
    m_x.exit_move_resize();
}

void
client_events_t::from_user_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
}

void
client_events_t::to_move_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    for (auto& child : client->children)
        child->unmap();
    m_x.enter_move(client, x_wrapper::pointer_position());
    x_wrapper::confine_pointer(m_x.moveresize()->indicator);
}

void
client_events_t::to_resize_workspace(client_ptr_t client, workspace_ptr_t workspace)
{
    for (auto& child : client->children)
        child->unmap();
    m_x.enter_resize(client, x_wrapper::pointer_position());
    x_wrapper::confine_pointer(m_x.moveresize()->indicator);
}

void
client_events_t::to_user_workspace(client_ptr_t client, workspace_ptr_t _workspace)
{
    auto workspace = user_workspace(_workspace);

    client->map();
    m_ewmh.set_window_state_property(client->win);
    m_ewmh.set_wm_desktop_property(client->win,
        USER_WORKSPACES.size() + workspace->get_number() - 1);
}
