#include "x-events.hh"

#include "client-model.hh"
#include "decoration.hh"
#include "ewmh.hh"
#include "floating.hh"
#include "sidebar.hh"
#include "x-model.hh"

#include "x-data/attributes.hh"
#include "x-data/request.hh"
#include "x-data/hints.hh"


bool
x_events_t::step()
{
    x_data::next_event(m_current_event);

    switch (m_current_event.type()) {
    case ButtonPress:      on_button_press();      break;
    case ButtonRelease:    on_button_release();    break;
    case CirculateRequest: on_circulate_request(); break;
    case ClientMessage:    on_client_message();    break;
    case ConfigureNotify:  on_configure_notify();  break;
    case ConfigureRequest: on_configure_request(); break;
    case DestroyNotify:    on_destroy_notify();    break;
    case Expose:           on_expose();            break;
    case FocusIn:          on_focus_in();          break;
    case KeyPress:         on_key_press();         break;
    case MapNotify:        on_map_notify();        break;
    case MapRequest:       on_map_request();       break;
    case MappingNotify:    on_mapping_notify();    break;
    case MotionNotify:     on_motion_notify();     break;
    case PropertyNotify:   on_property_notify();   break;
    case UnmapNotify:      on_unmap_notify();      break;
    default: break;
    }

    return m_running;
}

void
x_events_t::register_window(x_data::window_t win)
{
    if (m_ewmh.check_apply_strut(win)) {
        m_clients.active_workspace()->arrange();
        m_clients.refullscreen_clients();
    }

    if (win.is_of_type("DOCK")) {
        m_ewmh.set_frame_extents(win, true);
        m_windowstack.add_to_stack({win, layer_t::dock});
        m_windowstack.apply(m_clients.active_workspace());
        return;
    }

    if (!x_data::should_manage(win)) {
        m_ewmh.set_frame_extents(win, true);

        if (win.is_of_state("BELOW"))
            m_windowstack.add_to_stack({win, layer_t::below});
        else if (win.is_of_type("DESKTOP"))
            m_windowstack.add_to_stack({win, layer_t::desktop});
        else if (win.is_of_type("NOTIFICATION"))
            m_windowstack.add_to_stack({win, layer_t::notification});
        else
            m_windowstack.add_to_stack({win, layer_t::above});

        m_windowstack.apply(m_clients.active_workspace());
        return;
    }

    rule_t rule = retrieve_rule(m_rules, win);
    if (x_data::has_property<x_data::cardinal_t>(win, "_NET_WM_DESKTOP")) {
        rule.workspace = x_data::get_property<x_data::cardinal_t>(win, "_NET_WM_DESKTOP")().get() + 1;
        if (!range_t<::std::size_t>::contains(1, USER_WORKSPACES.size(), rule.workspace))
            rule.workspace = 0;
    }

    if (x_data::get_sizehints(win).success() & USPosition)
        rule.center = false;

    x_data::window_t parent = x_data::get_transient_for(win);
    if (parent) {
        if (!x_data::should_manage(parent)) {
            m_windowstack.add_to_stack({parent, layer_t::above});
            m_ewmh.set_frame_extents(parent, true);
            return;
        }

        client_ptr_t client;
        if ((client = m_clients.win_client(parent))) {
            rule.floating = true;
            client_ptr_t child_client = create_client(win, rule);
            child_client->parent = client;
            client->children.insert(child_client);
            m_clients.manage_client(child_client, rule);
        } else {
            rule.floating = true;
            client = create_client(win, rule);
            m_clients.manage_client(client, rule);
        }

        m_ewmh.set_frame_extents(win);
        return;
    }

    m_clients.manage_client(create_client(win, rule), rule);
    m_ewmh.set_frame_extents(win);
}

void
x_events_t::on_button_press()
{
    XButtonEvent event = m_current_event.get().xbutton;
    x_data::window_t win = event.window;
    x_data::window_t subwin = event.subwindow;

    if (win.get() == x_data::g_root.get()) {
        if (subwin.get() == None) {
            m_input.process_mouse_input_global(event);
            return;
        } else
            win = subwin;
    }

    if (win.get() == m_sidebar.get_win().get()) {
        m_input.process_mouse_input_sidebar(event);
        return;
    }

    client_ptr_t client = m_clients.win_client(win);
    if (client) {
        m_input.grab_mousebinds();

        if (m_input.moves_focus(event))
            m_clients.focus(client);

        m_input.process_mouse_input_client(client, event);

        if (m_input.moves_focus(event))
            m_clients.focus(client);
    }
}

void
x_events_t::on_button_release()
{
    x_data::window_t win = m_current_event.get().xbutton.window;

    if (!m_x.is_valid() || (win.get() != m_x.moveresize()->indicator.get()))
        return;

    auto client = m_x.moveresize()->client;
    auto attrs = x_data::get_attributes(client->frame);

    switch (m_x.moveresize()->state) {
    case moveresizestate_t::move:   m_clients.stop_moving(client, attrs);          break;
    case moveresizestate_t::resize: m_clients.stop_resizing(client, attrs, attrs); break;
    default: break;
    }
}

void
x_events_t::on_circulate_request()
{
    x_data::propagate_circulate_request(m_current_event);
}

void
x_events_t::on_client_message()
{
    XClientMessageEvent event = m_current_event.get().xclient;
    x_data::window_t win = event.window;
    client_ptr_t client = m_clients.win_client(win);

    if (!client)
        return;

    netwmid_t netwm_index;
    for (netwm_index = netwmid_t::netfirst; netwm_index < netwmid_t::netlast; ++netwm_index)
        if (event.message_type == m_ewmh.get_netwm_atom(netwm_index))
            break;
    if (netwm_index >= netwmid_t::netlast)
        return;

    switch (netwm_index) {
    case netwmid_t::netwmstate:
        {
            for (int property = 1; property <= 2; ++property) {
                if (event.data.l[property] == 0)
                    continue;

                if (event.data.l[0] >= static_cast<int>(netwmaction_t::netnoaction))
                    return;

                if ((Atom)event.data.l[property]
                    == m_ewmh.get_netwm_atom(netwmid_t::netwmstatefullscreen))
                {
                    m_clients.set_fullscreen(client, static_cast<clientaction_t>(event.data.l[0]));

                } else if ((Atom)event.data.l[property]
                    == m_ewmh.get_netwm_atom(netwmid_t::netwmstateabove))
                {
                    m_clients.set_above(client, static_cast<clientaction_t>(event.data.l[0]));

                } else if ((Atom)event.data.l[property]
                    == m_ewmh.get_netwm_atom(netwmid_t::netwmstatebelow))
                {
                    m_clients.set_below(client, static_cast<clientaction_t>(event.data.l[0]));

                } else if ((Atom)event.data.l[property]
                    == m_ewmh.get_netwm_atom(netwmid_t::netwmstatedemandsattention))
                {
                    m_clients.set_urgent(client, static_cast<clientaction_t>(event.data.l[0]));
                }

                m_sidebar.draw_clientstate();
            }
        }
        break;
    case netwmid_t::netactivewindow:
        {   // if pager or taskbar (source indicator = 2)
            if (event.data.l[0] == 2)
                m_clients.focus(client);
        }
        break;
    default: break;
    }
}

void
x_events_t::on_configure_request()
{
    x_data::window_t win = m_current_event.get().xconfigurerequest.window;
    client_ptr_t client = m_clients.win_client(win);

    if (!client) {
        int perm_flags = CWX | CWY | CWWidth | CWHeight;
        x_data::propagate_configure_request(m_current_event, perm_flags);
        return;
    }

    if (is_user_workspace(m_clients.client_workspace(client))) {
        auto workspace = user_workspace(m_clients.client_workspace(client));
        if (!(workspace->in_float_layout() || client->floating))
            return;
    }

    m_current_event.get().xconfigurerequest.width =
        ::std::max(m_current_event.get().xconfigurerequest.width, MIN_WINDOW_SIZE);

    m_current_event.get().xconfigurerequest.height =
        ::std::max(m_current_event.get().xconfigurerequest.height, MIN_WINDOW_SIZE);

    auto before_attrs = x_data::get_attributes(client->frame);

    if (win.get() == client->win.get()) {
        if (m_x.is_valid() && m_x.moveresize()->state == moveresizestate_t::resize)
            return;

        auto win_event = m_current_event;
        int conf_flags = win_event.get().xconfigurerequest.value_mask;
        int perm_flags = CWWidth | CWHeight;

        if (conf_flags & CWX)
            client->float_pos.x = m_current_event.get().xconfigurerequest.x;
        if (conf_flags & CWY)
            client->float_pos.y = m_current_event.get().xconfigurerequest.y;
        if (conf_flags & CWWidth)
            client->float_dim.w = m_current_event.get().xconfigurerequest.width;
        if (conf_flags & CWHeight)
            client->float_dim.h = m_current_event.get().xconfigurerequest.height + BORDER_HEIGHT;

        conf_flags &= perm_flags;
        if (conf_flags) {
            x_data::propagate_configure_request(win_event, conf_flags);
            update_offset(client);
        }

        m_current_event.get().xconfigurerequest.window = client->frame;
        m_current_event.get().xconfigurerequest.height += BORDER_HEIGHT;
    }

    int conf_flags = m_current_event.get().xconfigurerequest.value_mask;
    int perm_flags = CWX | CWY | CWWidth | CWHeight;

    conf_flags &= perm_flags;
    if (conf_flags)
        x_data::propagate_configure_request(m_current_event, conf_flags);

    if (m_x.is_valid()) {
        auto after_attrs = x_data::get_attributes(client->frame);

        pos_t pos;
        switch (m_x.moveresize()->grabbed_at) {
        case corner_t::top_left:
            pos = {before_attrs.x() + (before_attrs.w() - after_attrs.w()),
                before_attrs.y() + (before_attrs.h() - after_attrs.h())};
            break;
        case corner_t::top_right:
            pos = {before_attrs.x(),
                before_attrs.y() + (before_attrs.h() - after_attrs.h())};
            break;
        case corner_t::bottom_left:
            pos = {before_attrs.x() + (before_attrs.w() - after_attrs.w()),
                before_attrs.y()};
            break;
        case corner_t::bottom_right: // fallthrough
        default: pos = before_attrs; break;
        }

        if (!(conf_flags & (CWX | CWY))
            && !(dim_t{before_attrs.w(), before_attrs.h()}
                == dim_t{after_attrs.w(), after_attrs.h()}))
        {
            client->move(pos);
        }
    }
}

void
x_events_t::on_configure_notify()
{
    x_data::window_t win = m_current_event.get().xconfigure.window;
    client_ptr_t client = m_clients.win_client(win);

    if (win.get() == x_data::g_root.get()) {
        m_clients.wedge_clients();
        m_clients.active_workspace()->arrange();
    }

    if (!client || (m_x.is_valid() && m_x.moveresize()->state == moveresizestate_t::resize))
        return;

    auto sizehints = x_data::get_sizehints(client->win);
    if (sizehints.success())
        sizehints.get().flags = PSize;

    if (m_x.update_hints(client, sizehints)) {
        client->sizeconstraints.apply(client->pos, client->dim);
        client->resize(client->dim);
    }
}

void
x_events_t::on_destroy_notify()
{
    x_data::window_t win = m_current_event.get().xdestroywindow.window;
    client_ptr_t client = m_clients.win_client(win);

    if (!client) {
        if (m_ewmh.check_release_strut(win))
            m_clients.active_workspace()->arrange();
        m_ewmh.unregister_from_list(win);
        return;
    }

    x_data::select_input(client->win, 0);
    x_data::select_input(client->frame, 0);

    if (client->urgent)
        m_clients.client_user_workspace(client)->erase_urgent();

    m_clients.unmanage_client(client);
    m_clients.active_workspace()->arrange();
    m_ewmh.unregister_from_list(client->win);
}

void
x_events_t::on_expose()
{
    x_data::window_t win = m_current_event.get().xexpose.window;
    int count = m_current_event.get().xexpose.count;

    if (count == 0 && m_sidebar.get_win().get() == win.get())
        m_sidebar.draw();

    if (count == 0 && m_sidebar.get_unmappedsidebarwin().get() == win.get())
        m_sidebar.draw();

    x_data::sync(false);
}

void
x_events_t::on_focus_in()
{}

void
x_events_t::on_key_press()
{
    XKeyEvent event = m_current_event.get().xkey;

    client_ptr_t client = m_clients.focused_client();
    if (client)
        m_input.process_key_input_client(client, event);
    else
        m_input.process_key_input_global(event);
}

void
x_events_t::on_map_request()
{
    x_data::window_t win = m_current_event.get().xmaprequest.window;
    client_ptr_t client = m_clients.win_client(win);

    if (client) {
        if (!client->consume_expect(clientexpect_t::map)
            && is_moveresize_workspace(m_clients.client_workspace(client)))
        {
            m_x.exit_move_resize();
            auto attrs = x_data::get_attributes(client->frame);
            m_clients.stop_moving(client, attrs);
            m_clients.stop_resizing(client, attrs, attrs);
        }

        if (win == client->frame)
            client->consume_expect(clientexpect_t::deiconify);

        return;
    }

    register_window(win);
    win.map();
    m_ewmh.register_to_list(win);
}

void
x_events_t::on_map_notify()
{
    x_data::window_t win = m_current_event.get().xmap.window;
    client_ptr_t client = m_clients.win_client(win);

    if (client) {
        if (!client->consume_expect(clientexpect_t::map)
            && is_moveresize_workspace(m_clients.client_workspace(client)))
        {
            m_x.exit_move_resize();
            auto attrs = x_data::get_attributes(client->frame);
            m_clients.stop_moving(client, attrs);
            m_clients.stop_resizing(client, attrs, attrs);
        }

        if (win == client->frame)
            client->consume_expect(clientexpect_t::deiconify);

        return;
    }

    register_window(win);
}

void
x_events_t::on_mapping_notify()
{
    XMappingEvent event = m_current_event.get().xmapping;

    x_data::refresh_keyboard_mapping(event);
    if (event.request == MappingKeyboard)
        m_input.grab_keybinds();
}

void
x_events_t::on_motion_notify()
{
    client_ptr_t client;
    if (!m_x.is_valid() || !(client = m_x.moveresize()->client))
        return;

    x_data::last_typed_event(m_current_event, MotionNotify);

    auto client_attrs = x_data::get_attributes(client->frame);
    client_attrs.get().height -= BORDER_HEIGHT;

    pos_t pos = client_attrs;
    dim_t dim = client_attrs;
    pos_t delta = m_x.update_pointer(x_data::pointer_position());

    switch (m_x.moveresize()->state) {
    case moveresizestate_t::move:   m_x.moveresize()->process_move_increment(pos, dim, delta);   break;
    case moveresizestate_t::resize: m_x.moveresize()->process_resize_increment(pos, dim, delta); break;
    default: break;
    }
}

void
x_events_t::on_property_notify()
{
    auto event = m_current_event.get().xproperty;
    x_data::window_t win = event.window;
    client_ptr_t client = m_clients.win_client(win);

    if (client && event.state == PropertyNewValue) {
        if (event.atom == XA_WM_NORMAL_HINTS) {
            auto sizehints = x_data::get_sizehints(win);

            if (!sizehints.get().flags)
                sizehints.get().flags = PSize;

            m_x.update_hints(client, sizehints);
        } else if ((event.atom == XA_WM_NAME || event.atom
            == x_data::get_atom("_NET_WM_NAME").get()))
        {
            // handle iconified, shaded (if ever implemented)
        }
    }

    if (event.atom == x_data::get_atom("_NET_WM_STRUT").get()
        || event.atom == x_data::get_atom("_NET_WM_STRUT_PARTIAL").get())
    {
        m_ewmh.check_release_strut(event.window);
        m_ewmh.check_apply_strut(event.window);
        m_clients.active_workspace()->arrange();
    }

    if (IPC_ENABLED && (win.get() == x_data::g_root.get()) && m_ipc.assert_target(m_current_event))
        m_input.process_ipc_global(m_ipc.resolve_operation(m_current_event));
}

void
x_events_t::on_unmap_notify()
{
    x_data::window_t win = m_current_event.get().xunmap.window;
    client_ptr_t client = m_clients.win_client(win);

    if (!client) {
        m_windowstack.remove_from_stack(win);
        return;
    }

    x_data::sync(false);

    if (client->consume_expect(clientexpect_t::withdraw)
        || (win == client->frame && client->consume_expect(clientexpect_t::iconify)))
    {
        return;
    }

    client->unmap();
    client->win.reparent(client->pos);
    client->frame.destroy();
}
