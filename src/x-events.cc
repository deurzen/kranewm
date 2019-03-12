#include "x-events.hh"
#include "decoration.hh"
#include "ewmh.hh"
#include "client-model.hh"
#include "x-model.hh"
#include "x-wrapper/attributes.hh"
#include "x-wrapper/request.hh"
#include "x-wrapper/hints.hh"


bool
x_events_t::step()
{
    x_wrapper::next_event(m_current_event);

    switch (m_current_event.type()) {
    case ButtonPress:      on_button_press();       break;
    case ButtonRelease:    on_button_release();     break;
    case CirculateRequest: on_circulate_request();  break;
    case ClientMessage:    on_client_message();     break;
    case ConfigureNotify:  on_configure_notify();  break;
    case ConfigureRequest: on_configure_request(); break;
    case DestroyNotify:    on_destroy_notify();    break;
    case Expose:           on_expose();            break;
    case FocusIn:          on_focus_in();          break;
    case KeyPress:         on_key_press();         break;
    case MapNotify:        on_map_notify();        break;
    case MapRequest:       on_map_request();       break;
    case MotionNotify:     on_motion_notify();     break;
    case PropertyNotify:   on_property_notify();   break;
    case UnmapNotify:      on_unmap_notify();      break;
    default: break;
    }

    return m_running;
}

void
x_events_t::register_window(x_wrapper::window_t win)
{
    if (m_ewmh.check_apply_strut(win))
        m_clients.active_workspace()->arrange();

    if (win.is_of_type("DESKTOP"))
        m_windowstack.add_to_stack({win, layer_t::desktop});
    else if (win.is_of_type("BELOW"))
        m_windowstack.add_to_stack({win, layer_t::below});
    else if (win.is_of_type("NOTIFICATION"))
        m_windowstack.add_to_stack({win, layer_t::notification});
    else if (win.is_of_type("DOCK")) {
        m_windowstack.add_to_stack({win, layer_t::dock});
        m_ewmh.set_frame_extents(win, true);
        return;
    }

    if (!x_wrapper::should_manage(win)) {
        m_windowstack.add_to_stack({win, layer_t::floating});
        m_ewmh.set_frame_extents(win, true);
        return;
    }

    rule_t rule = retrieve_rule(win);
    if (x_wrapper::has_property<x_wrapper::cardinal_t>(win, "_NET_WM_DESKTOP"))
        rule.workspace = x_wrapper::get_property<x_wrapper::cardinal_t>(win, "_NET_WM_DESKTOP")();

    if (x_wrapper::get_sizehints(win).success() & USPosition)
        rule.center = false;

    x_wrapper::window_t parent = x_wrapper::get_transient_for(win);
    if (parent) {
        if (!x_wrapper::should_manage(parent)) {
            m_windowstack.add_to_stack({parent, layer_t::floating});
            m_ewmh.set_frame_extents(parent, true);
            return;
        }

        client_ptr_t client;
        if ((client = m_clients.win_to_client(parent))) {
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

rule_t
x_events_t::retrieve_rule(x_wrapper::window_t win)
{
    bool floating  = false;
    bool center    = false;
    bool iconify   = false;
    bool autoclose = false;
    unsigned workspace = 0;

    ::std::string cls = win.get_class();
    ::std::string inst = win.get_instance();
    ::std::string title = win.get_name();

    for (auto&& [rule_id,rule] : m_rules) {
        const ::std::string& rule_cls   = ::std::get<0>(rule_id);
        const ::std::string& rule_inst  = ::std::get<1>(rule_id);
        const ::std::string& rule_title = ::std::get<2>(rule_id);

        bool same_cls, same_inst, same_title;
        same_cls   = !rule_cls.compare(cls)     || rule_cls.empty();
        same_inst  = !rule_inst.compare(inst)   || rule_inst.empty();
        same_title = !rule_title.compare(title) || rule_title.empty();

        if (same_cls && same_inst && same_title) {
            if (rule.workspace != 0)
                workspace = rule.workspace;
            floating = rule.floating;
            center   = rule.center;
            iconify  = rule.iconify;
            if (rule.autoclose != OFF) {
                autoclose = true;
                if (rule.autoclose == ONCE)
                    rule.autoclose = OFF;
            }
            return {floating, center, false, iconify, autoclose, workspace};
        }
    }

    return {false, false, false, false, false, 0};
}

void
x_events_t::on_button_press()
{
    XButtonEvent event = m_current_event.get().xbutton;
    x_wrapper::window_t win = event.window;
    x_wrapper::window_t subwin = event.subwindow;

    if (win.get() == x_wrapper::g_root.get()) {
        if (subwin.get() == None)
            m_inputhandler.process_mouse_input_global(m_current_event.get().xbutton);
        else
            win = subwin;
    }

    client_ptr_t client = m_clients.win_to_client(win);
    if (client) {
        m_clients.focus(client);
        m_inputhandler.process_mouse_input_client(client, event);
    }
}

void
x_events_t::on_button_release()
{
    x_wrapper::window_t win = m_current_event.get().xbutton.window;

    if (!m_x.is_valid() || (win.get() != m_x.moveresize()->indicator.get()))
        return;

    auto client = m_x.moveresize()->client;
    auto attrs = x_wrapper::get_attributes(client->frame);

    switch (m_x.moveresize()->state) {
    case moveresizestate_t::move:   m_clients.stop_moving(client, attrs);          break;
    case moveresizestate_t::resize: m_clients.stop_resizing(client, attrs, attrs); break;
    default: break;
    }
}

void
x_events_t::on_circulate_request()
{
    x_wrapper::propagate_circulate_request(m_current_event);
}

void
x_events_t::on_client_message()
{
    XClientMessageEvent event = m_current_event.get().xclient;
    x_wrapper::window_t win = event.window;
    client_ptr_t client = m_clients.win_to_client(win);

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

                if ((Atom)event.data.l[property]
                    == m_ewmh.get_netwm_atom(netwmid_t::netwmstatefullscreen))
                {
                    if (event.data.l[0] >= static_cast<int>(netwmaction_t::netnoaction))
                        return;
                    /* m_clients.toggle_fullscreen(client, event.data.l[0]); */
                } else if ((Atom)event.data.l[property]
                    == m_ewmh.get_netwm_atom(netwmid_t::netwmstatedemandsattention))
                {
                    if (event.data.l[0] >= static_cast<int>(netwmaction_t::netnoaction))
                        return;
                    /* m_clients.toggle_urgency(client, event.data.l[0]); */
                }
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
    x_wrapper::window_t win = m_current_event.get().xconfigurerequest.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (!client) {
        int perm_flags = CWX | CWY | CWWidth | CWHeight;
        x_wrapper::propagate_configure_request(m_current_event, perm_flags);
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

    auto before_attrs = x_wrapper::get_attributes(client->frame);

    if (win.get() == client->win.get()) {
        if (m_x.is_valid() && m_x.moveresize()->state == moveresizestate_t::resize)
            return;

        auto win_event = m_current_event;
        int conf_flags = win_event.get().xconfigurerequest.value_mask;
        int perm_flags = CWWidth | CWHeight;

        conf_flags &= perm_flags;
        if (conf_flags)
            x_wrapper::propagate_configure_request(win_event, conf_flags);

        m_current_event.get().xconfigurerequest.window = client->frame;
        m_current_event.get().xconfigurerequest.height += BORDER_HEIGHT;
    }

    int conf_flags = m_current_event.get().xconfigurerequest.value_mask;
    int perm_flags = CWX | CWY | CWWidth | CWHeight;

    conf_flags &= perm_flags;
    if (conf_flags)
        x_wrapper::propagate_configure_request(m_current_event, conf_flags);

    if (m_x.is_valid()) {
        auto after_attrs = x_wrapper::get_attributes(client->frame);

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
    x_wrapper::window_t win = m_current_event.get().xconfigure.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (!client)
        return;

    auto sizehints = x_wrapper::get_sizehints(client->win);
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
    x_wrapper::window_t win = m_current_event.get().xdestroywindow.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (!client) {
        /* cm_.remove_always_on_top_window(win); */
        if (m_ewmh.check_release_strut(win))
            m_clients.active_workspace()->arrange();
        return;
    }

    x_wrapper::select_input(client->win, 0);
    x_wrapper::select_input(client->frame, 0);

    if (client->parent)
        client->parent->disown_child(client);

    m_clients.unmanage_client(client);
    m_clients.active_workspace()->arrange();
}

void
x_events_t::on_expose()
{
    x_wrapper::window_t win = m_current_event.get().xexpose.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (client && client->shaded)
        ; /* client->gch->redraw_all(); */

    /* if (cm_.is_icon(m_current_event.xexpose.window)) */
    /* { */
    /*     cm_.rerender_icons(); */
    /* } */

    /* if (m_current_event.xexpose.count == 0 */
    /*     && xh_.is_root_draw_win(m_current_event.xexpose.window)) */
    /* { */
    /*     xh_.redraw_root_draw_win(); */
    /* } */

    x_wrapper::sync(False);
}

void
x_events_t::on_focus_in()
{
    x_wrapper::window_t win = m_current_event.get().xfocus.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (client && client == m_clients.focused_client()) {
        /* if (client->floating */
        /*     || cm_.get_current_workspace()->layout == LT_FLOAT */
        /*     || cm_.get_current_workspace()->layout == LT_DECK */
        /*     || cm_.get_current_workspace()->layout == LT_DOUBLEDECK */
        /*     || cm_.get_current_workspace()->layout == LT_MONOCLE) */
        /* { */
        /*     xh_.raise_window(client->frame); */
        /*     for (const auto& child_client : client->children) */
        /*         xh_.raise_window(child_client->frame); */
        /* } */

        /* if (cm_.get_current_workspace()->layout == LT_DECK */
        /*     || cm_.get_current_workspace()->layout == LT_DOUBLEDECK */
        /*     || cm_.get_current_workspace()->layout == LT_MONOCLE) */
        /* { */
        /*     for (const auto& floating_client : cm_.get_floating_clients()) */
        /*         xh_.raise_window(floating_client->frame); */
        /* } */

        /* for (const auto& always_on_top_win : cm_.get_always_on_top_windows()) */
        /*     xh_.raise_window(always_on_top_win); */
    }
}

void
x_events_t::on_key_press()
{
    XKeyEvent event = m_current_event.get().xkey;

    m_inputhandler.process_key_input_global(event);
    client_ptr_t client = m_clients.focused_client();
    if (client)
        m_inputhandler.process_key_input_client(client, event);
}

void
x_events_t::on_map_request()
{
    x_wrapper::window_t win = m_current_event.get().xmaprequest.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (client) {
        if (!client->redeem_expect(clientexpect_t::map)
            && is_moveresize_workspace(m_clients.client_workspace(client)))
        {
            m_x.exit_move_resize();
            auto attrs = x_wrapper::get_attributes(client->frame);
            m_clients.stop_moving(client, attrs);
            m_clients.stop_resizing(client, attrs, attrs);
        }

        return;
    }

    register_window(win);
    win.map();
}

void
x_events_t::on_map_notify()
{
    x_wrapper::window_t win = m_current_event.get().xmap.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (client) {
        if (!client->redeem_expect(clientexpect_t::map)
            && is_moveresize_workspace(m_clients.client_workspace(client)))
        {
            m_x.exit_move_resize();
            auto attrs = x_wrapper::get_attributes(client->frame);
            m_clients.stop_moving(client, attrs);
            m_clients.stop_resizing(client, attrs, attrs);
        }

        return;
    }

    register_window(win);
}

void
x_events_t::on_motion_notify()
{
    client_ptr_t client;
    if (!m_x.is_valid() || !(client = m_x.moveresize()->client))
        return;

    x_wrapper::last_typed_event(m_current_event, MotionNotify);

    auto client_attrs = x_wrapper::get_attributes(client->frame);
    client_attrs.get().height -= BORDER_HEIGHT;

    pos_t pos = client_attrs;
    dim_t dim = client_attrs;
    pos_t delta = m_x.update_pointer(x_wrapper::pointer_position());

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
    x_wrapper::window_t win = event.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (client && event.state == PropertyNewValue) {
        if (event.atom == XA_WM_NORMAL_HINTS) {
            auto sizehints = x_wrapper::get_sizehints(win);

            if (!sizehints.get().flags)
                sizehints.get().flags = PSize;

            m_x.update_hints(client, sizehints);
        } else if ((event.atom == XA_WM_NAME || event.atom
            == x_wrapper::get_atom("_NET_WM_NAME").get()))
        {
            if (client->iconified)
                ; // handle iconified
            else if (client->shaded)
                ; // handle shaded
        }
    }

    if (event.atom == x_wrapper::get_atom("_NET_WM_STRUT").get()
        || event.atom == x_wrapper::get_atom("_NET_WM_STRUT_PARTIAL").get())
    {
        m_ewmh.check_release_strut(event.window);
        m_ewmh.check_apply_strut(event.window);
        m_clients.active_workspace()->arrange();
    }
}

void
x_events_t::on_unmap_notify()
{
    x_wrapper::window_t win = m_current_event.get().xunmap.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (!client) {
        m_windowstack.remove_from_stack(win);
        return;
    }

    if (client->redeem_expect(clientexpect_t::iconify)
        || client->redeem_expect(clientexpect_t::withdraw))
    {
        return;
    }

    /* if (client->iconified) */
    /*     cm_.toggle_iconify(client); */

    /* if (client->floating) { */
    /*     xh_.destroy_window(client->float_indicator); */
    /*     client->float_indicator = None; */
    /* } */

    pos_t pos = client->pos;
    x_wrapper::window_t frame = client->frame;

    client->unmap();
    win.reparent(pos);
    m_windowstack.remove_from_stack(win);
    frame.destroy();
}
