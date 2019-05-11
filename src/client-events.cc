#include "client-events.hh"
#include "changes.hh"
#include "ewmh.hh"
#include "sidebar.hh"
#include "x-model.hh"
#include "client-model.hh"
#include "x-wrapper/event.hh"
#include "x-wrapper/mouse.hh"


void
client_events_t::process_queued_changes()
{
    while ((m_current_change = m_changes.next())) {
        switch (m_current_change->type) {
        case change_t::client_focus:      on_change_client_focus();      break;
        case change_t::client_destroy:    on_change_client_destroy();    break;
        case change_t::client_fullscreen: on_change_client_fullscreen(); break;
        case change_t::client_urgent:     on_change_client_urgent();     break;
        case change_t::client_workspace:  on_change_client_workspace();  break;
        case change_t::workspace_active:  on_change_workspace_active();  break;
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

        from->frame.grab();
    }

    if (to) {
        if (to->urgent)
            m_clients.set_urgent(to, clientaction_t::remove);

        if (x_wrapper::set_input_focus(to->win)) {
            m_ewmh.set_active_window_property(to->win);
            to->frame.set_background_color(SEL_COLOR);
            to->frame.ungrab();

            if (to->fullscreen)
                m_sidebar.indicate_clientfullscreen().draw();
            else if (to->floating)
                m_sidebar.indicate_clientfloating().draw();
            else
                m_sidebar.indicate_clientnormal().draw();

        } else {
            to->frame.set_background_color(REG_COLOR);
            to->frame.grab();
        }

    } else
        x_wrapper::set_input_focus();
}

void
client_events_t::on_change_client_destroy()
{
    auto change    = change_client_destroy(m_current_change);
    auto client    = change->client;
    auto workspace = change->workspace;

    if (is_moveresize_workspace(workspace)) {
        x_wrapper::release_pointer();
        m_x.exit_move_resize();
    }

    if (!m_clients.focused_client())
        m_sidebar.indicate_clientnormal().draw();

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
        auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
        client->resize({root_attrs.w() - SIDEBAR_WIDTH - 1, root_attrs.h() + BORDER_HEIGHT}, true);
        client->move({m_ewmh.get_left_strut(), -BORDER_HEIGHT - 1}, true);
    } else {
        m_ewmh.set_window_state_property(client->win);
        client->floating = former_state.floating;
        client->float_pos = former_state.float_pos;
        client->float_dim = former_state.float_dim;
        client->pos = former_state.pos;
        client->dim = former_state.dim;
        if (m_clients.client_user_workspace(client)->in_float_layout() || client->floating)
            client->resize(client->float_dim).move(client->float_pos);
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
        m_sidebar.erase_urgent(m_clients.client_user_workspace(client)->get_number()).draw();
    } else if (client->urgent) {
        client->frame.set_background_color(URG_COLOR);
        m_sidebar.record_urgent(m_clients.client_user_workspace(client)->get_number()).draw();
    } else
        client->frame.set_background_color(REG_COLOR);

    x_wrapper::wmhints_t hints = x_wrapper::get_wmhints(client->win);
    if (hints.success())
        hints.get().flags = client->urgent
            ? (hints.get().flags | XUrgencyHint) : (hints.get().flags & ~XUrgencyHint);

    x_wrapper::set_wmhints(client->win, hints);
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

    if (from) {
        switch (from->get_type()) {
        case workspacetype_t::move:   from_move_workspace(client, from);     break;
        case workspacetype_t::resize: from_resize_workspace(client, from);   break;
        case workspacetype_t::user:   from_user_workspace(client, from, to); break;
        default: break;
        }
    } else
        client->frame.grab();

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

    { // circumvents X server race condition
        x_wrapper::sync(false);
        x_wrapper::event_t event;
        while (x_wrapper::typed_event(event, UnmapNotify)) {
            x_wrapper::window_t win = event.get().xunmap.window;
            client_ptr_t client = m_clients.win_to_client(win);

            if (client && !client->consume_expect(clientexpect_t::withdraw)) {
                client->unmap();
                client->win.reparent(client->pos);
                client->frame.destroy();
                x_wrapper::sync(false);
            }
        }
    }

    map_all(to->get_all());
    unmap_all(from->get_all());

    to->arrange();

    if (!m_clients.focused_client())
        m_sidebar.indicate_clientnormal();

    m_sidebar.set_layoutsymbol(to->get_layout());
    m_sidebar.set_workspacenumber(to->get_number());
    m_sidebar.set_numberclients(to->get_all().size()).draw();
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

    m_sidebar.erase_activity(user_workspace(from)->get_number()).draw();
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
    m_sidebar.set_numberclients(m_clients.active_workspace()->get_all().size());
    m_sidebar.record_activity(user_workspace(to)->get_number()).draw();
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
