#include "x-events.hh"
#include "decoration.hh"
#include "ewmh.hh"
#include "client-model.hh"
#include "x-model.hh"
#include "x-wrapper/attributes.hh"
#include "x-wrapper/request.hh"
#include "x-wrapper/hints.hh"
#include "x-wrapper/input.hh"

#include <unistd.h>


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

    if (!x_wrapper::should_manage(win) || win.is_of_type("DOCK")) {
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
    x_wrapper::window_t win = m_current_event.get().xbutton.window;
    x_wrapper::window_t subwin = m_current_event.get().xbutton.subwindow;
    unsigned button = m_current_event.get().xbutton.button;
    unsigned mask = m_current_event.get().xbutton.state;

    if (win.get() == x_wrapper::g_root.get()) {
        if (subwin.get() == None) {
            if (m_mousebinds.count({button, mask, false}))
                switch (m_mousebinds[{button, mask, false}]) {
                case mouseop_t::goto_next_ws:
                {
                    unsigned workspace = m_clients.active_workspace()->get_number();
                    workspace %= USER_WORKSPACES.size();
                    m_clients.change_active_workspace(workspace + 1);
                }
                    return;
                case mouseop_t::goto_prev_ws:
                {
                    unsigned workspace = m_clients.active_workspace()->get_number() - 1;
                    workspace = (workspace == 0) ? USER_WORKSPACES.size() : workspace;
                    m_clients.change_active_workspace(workspace);
                }
                    return;
                default: break;
                }
        } else
            win = subwin;
    }

    client_ptr_t client = m_clients.win_to_client(win);
    if (client) {
        m_clients.focus(client);
        if (m_mousebinds.count({button, mask, true}))
            switch (m_mousebinds[{button, mask, true}]) {
            case mouseop_t::client_move:    m_clients.start_moving(client);   break;
            case mouseop_t::client_resize:  m_clients.start_resizing(client); break;
            case mouseop_t::client_center:  client->center();                 break;
            case mouseop_t::client_next_ws: // fallthrough
            case mouseop_t::goto_next_ws:
                {
                    unsigned workspace = m_clients.active_workspace()->get_number();
                    workspace %= USER_WORKSPACES.size();
                    m_clients.change_active_workspace(workspace + 1);
                }
                return;
            case mouseop_t::client_prev_ws: // fallthrough
            case mouseop_t::goto_prev_ws:
                {
                    unsigned workspace = m_clients.active_workspace()->get_number() - 1;
                    workspace = (workspace == 0) ? USER_WORKSPACES.size() : workspace;
                    m_clients.change_active_workspace(workspace);
                }
                return;
            default: break;
            }
        return;
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
    keyshortcut_t shortcut{m_current_event.get().xkey};

    switch (m_keybinds[shortcut]) {
    case keyop_t::quit: m_running = false; return;
    case keyop_t::spawn_terminal:      fork_external("/usr/bin/urxvt -geometry 80x22");                       break;
    case keyop_t::spawn_quickterm:     fork_external("/usr/bin/term -name \"kranewm:float\" -geometry 80x22");break;
    case keyop_t::spawn_quicktermtmux: fork_external("/usr/bin/term -geometry 80x22 -e tmux");                break;
    case keyop_t::spawn_dmenu:         fork_external("/usr/local/bin/dmenu_run");                             break;
    case keyop_t::spawn_dmenupass:     fork_external("/usr/bin/dmenupass");                                   break;
    case keyop_t::spawn_dmenupasscopy: fork_external("/usr/bin/dmenupass --copy");                            break;
    case keyop_t::spawn_browser:       fork_external("/usr/bin/qutebrowser");                                 break;
    case keyop_t::spawn_sec_browser:   fork_external("/usr/bin/firefox");                                     break;
    case keyop_t::mpctoggle:           fork_external("/usr/bin/mpc toggle");                                  break;
    case keyop_t::mpcnext:             fork_external("/usr/bin/mpc next");                                    break;
    case keyop_t::mpcprev:             fork_external("/usr/bin/mpc prev");                                    break;
    case keyop_t::mpcstop:             fork_external("/usr/bin/mpc stop");                                    break;
    case keyop_t::rhythmboxshow:       fork_external("/usr/bin/rhythmbox-client");                            break;
    case keyop_t::rhythmboxtoggle:     fork_external("/usr/bin/rhythmbox-client --play-pause");               break;
    case keyop_t::rhythmboxnext:       fork_external("/usr/bin/rhythmbox-client --next");                     break;
    case keyop_t::rhythmboxprev:       fork_external("/usr/bin/rhythmbox-client --previous");                 break;
    case keyop_t::rhythmboxstop:       fork_external("/usr/bin/rhythmbox-client --stop");                     break;
    case keyop_t::mpcrandom:           fork_external("/usr/bin/mpc random");                                  break;
    case keyop_t::mpcsingle:           fork_external("/usr/bin/mpc single");                                  break;
    case keyop_t::volumeup:            fork_external("/usr/bin/pactl set-sink-volume 0 +10%");                break;
    case keyop_t::volumedown:          fork_external("/usr/bin/pactl set-sink-volume 0 -10%");                break;
    case keyop_t::volumemute:          fork_external("/usr/bin/pactl set-sink-mute 0 toggle");                break;
    case keyop_t::brightnessup15:      fork_external("/usr/bin/light -A 15");                                 break;
    case keyop_t::brightnessup5:       fork_external("/usr/bin/light -A 5");                                  break;
    case keyop_t::brightnessdown15:    fork_external("/usr/bin/light -U 15");                                 break;
    case keyop_t::take_screenshot:
        fork_external("/usr/bin/maim $(date +/home/deurzen/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)");    break;
    case keyop_t::take_screenshot_sel:
        fork_external("/usr/bin/maim -s $(date +/home/deurzen/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)"); break;
    case keyop_t::spawn_neomutt:       fork_external("/usr/bin/term -geometry 140x42 -e zsh -i -c neomutt");  break;
    case keyop_t::spawn_ranger:        fork_external("/usr/bin/term -geometry 140x42 -e zsh -i -c ranger");   break;
    case keyop_t::spawn_sncli:         fork_external("/usr/bin/term -geometry 80x42 -e zsh -i -c sncli");     break;
    case keyop_t::spawn_rtv:           fork_external("/usr/bin/term -geometry 80x42 -e zsh -i -c rtv");       break;
    case keyop_t::spawn_irssi:         fork_external("/usr/bin/term -geometry 80x42 -e zsh -i -c irssi");     break;
    case keyop_t::spawn_newsboat:      fork_external("/usr/bin/term -geometry 80x42 -e zsh -i -c newsboat");  break;
    case keyop_t::spawn_sage:          fork_external("/usr/bin/term -geometry 80x22 -e zsh -i -c sage");      break;
    case keyop_t::spawn_gpick:         fork_external("gpick");                                                break;
    case keyop_t::spawn_qalculate:     fork_external("qalculate-gtk");                                        break;
    case keyop_t::spawn_7lock:         fork_external("systemctl suspend");                                    break;


    case keyop_t::activate_workspace_1: m_clients.change_active_workspace(1); break;
    case keyop_t::activate_workspace_2: m_clients.change_active_workspace(2); break;
    case keyop_t::activate_workspace_3: m_clients.change_active_workspace(3); break;
    case keyop_t::activate_workspace_4: m_clients.change_active_workspace(4); break;
    case keyop_t::activate_workspace_5: m_clients.change_active_workspace(5); break;
    case keyop_t::activate_workspace_6: m_clients.change_active_workspace(6); break;
    case keyop_t::activate_workspace_7: m_clients.change_active_workspace(7); break;
    case keyop_t::activate_workspace_8: m_clients.change_active_workspace(8); break;
    case keyop_t::activate_workspace_9: m_clients.change_active_workspace(9); break;
    case keyop_t::activate_next_ws:
        {
            unsigned workspace = m_clients.active_workspace()->get_number();
            workspace %= USER_WORKSPACES.size();
            m_clients.change_active_workspace(workspace + 1);
        }
        break;
    case keyop_t::activate_prev_ws:
        {
            unsigned workspace = m_clients.active_workspace()->get_number() - 1;
            workspace = (workspace == 0) ? USER_WORKSPACES.size() : workspace;
            m_clients.change_active_workspace(workspace);
        }
        break;
    /* case TOGGLE_SCRATCHPAD_1:  cm_.toggle_scratchpad(1);         break; */
    /* case TOGGLE_SCRATCHPAD_2:  cm_.toggle_scratchpad(2);         break; */
    case keyop_t::floating:   m_clients.active_workspace()->set_layout(layout_t::floating).arrange();   break;
    case keyop_t::tile:       m_clients.active_workspace()->set_layout(layout_t::tile).arrange();       break;
    case keyop_t::deck:       m_clients.active_workspace()->set_layout(layout_t::deck).arrange();       break;
    case keyop_t::doubledeck: m_clients.active_workspace()->set_layout(layout_t::doubledeck).arrange(); break;
    case keyop_t::grid:       m_clients.active_workspace()->set_layout(layout_t::grid).arrange();       break;
    case keyop_t::monocle:    m_clients.active_workspace()->set_layout(layout_t::monocle).arrange();    break;
    /* case TOGGLE_LAYOUT:        cm_.change_layout(LT_TOGGLE);     break; */
    /* case SWAP_ORIENTATION:     cm_.swap_orientation();           break; */
    case keyop_t::focus_bck:            m_clients.cycle_focus_backward();     break;
    case keyop_t::focus_fwd:            m_clients.cycle_focus_forward();      break;
    case keyop_t::zoom:
        {
            m_clients.active_workspace()->zoom().arrange();
            m_clients.sync_workspace_focus();
        }
        break;
    /* case JUMP_MASTER:          cm_.focus_jump(0);                break; */
    /* case JUMP_PANE:            cm_.pane_jump();                  break; */
    /* case JUMP_CLIENT_1:        cm_.focus_jump(0);                break; */
    /* case JUMP_CLIENT_2:        cm_.focus_jump(1);                break; */
    /* case JUMP_CLIENT_3:        cm_.focus_jump(2);                break; */
    /* case JUMP_CLIENT_4:        cm_.focus_jump(3);                break; */
    /* case JUMP_CLIENT_5:        cm_.focus_jump(4);                break; */
    /* case JUMP_CLIENT_6:        cm_.focus_jump(5);                break; */
    /* case JUMP_CLIENT_7:        cm_.focus_jump(6);                break; */
    /* case JUMP_CLIENT_8:        cm_.focus_jump(7);                break; */
    /* case JUMP_CLIENT_9:        cm_.focus_jump(8);                break; */
    case keyop_t::inc_m_factor:
        {
            float m_factor = m_clients.active_workspace()->get_m_factor();
            if (m_factor <= 0.85f)
                m_clients.active_workspace()->set_m_factor(m_factor + .05f).arrange();
        }
        break;
    case keyop_t::dec_m_factor:
        {
            float m_factor = m_clients.active_workspace()->get_m_factor();
            if (m_factor >= .15f)
                m_clients.active_workspace()->set_m_factor(m_factor - .05f).arrange();
        }
        break;
    case keyop_t::inc_n_master:
        {
            unsigned n_master = m_clients.active_workspace()->get_n_master();
            if (n_master < MAX_N_MASTER)
                m_clients.active_workspace()->set_n_master(++n_master).arrange();
        }
        break;
    case keyop_t::dec_n_master:
        {
            unsigned n_master = m_clients.active_workspace()->get_n_master();
            if (n_master > 0)
                m_clients.active_workspace()->set_n_master(--n_master).arrange();
        }
        break;
    case keyop_t::inc_gap_size:
        {
            int gap_size = m_clients.active_workspace()->get_gap_size();
            if (gap_size < MAX_GAP_SIZE)
                m_clients.active_workspace()->set_gap_size(++gap_size).arrange();
        }
        break;
    case keyop_t::dec_gap_size:
        {
            int gap_size = m_clients.active_workspace()->get_gap_size();
            if (gap_size > 0)
                m_clients.active_workspace()->set_gap_size(--gap_size).arrange();
        }
        break;
    case keyop_t::jump_to_marked_client: m_clients.jump_marked(); break;
    /* case TOGGLE_WORKSPACE: */
    /*     { */
    /*         if (cm_.scratchpad_active()) */
    /*             cm_.toggle_scratchpad(cm_.get_current_scratchpad()->number); */
    /*         else */
    /*             cm_.change_active_workspace(0); */
    /*     } */
    /*     break; */
    /* case JUMP_STACK: */
    /*     { */
    /*         if (!cm_.scratchpad_active()) */
    /*             cm_.focus_jump(cm_.get_current_workspace()->n_master); */
    /*     } */
    /*     break; */
    /* case JUMP_LAST: */
    /*     { */
    /*         if (!cm_.scratchpad_active()) */
    /*             cm_.focus_jump(cm_.get_current_workspace()->clients.size()-1); */
    /*     } */
    /*     break; */
    default: break;
    }


    client_ptr_t client = m_clients.focused_client();
    if (!client)
        return;

    switch (m_keybinds[shortcut]) {
    case keyop_t::kill_client:         client->win.force_close();                     break;
    case keyop_t::down_stack:                                                         break;
    case keyop_t::up_stack:                                                           break;
    case keyop_t::down_master:                                                        break;
    case keyop_t::up_master:                                                          break;
    /* case MOVE_CLIENT_FWD:           cm_.move_focused_client_forward();             break; */
    /* case MOVE_CLIENT_BCK:           cm_.move_focused_client_backward();            break; */
    /* case TOGGLE_FLOAT:              cm_.toggle_float(client);                      break; */
    /* case TOGGLE_FULLSCREEN:         cm_.toggle_fullscreen(client);                 break; */
    /* case TOGGLE_SHADE:              cm_.toggle_shade(client);                      break; */
    /* case TOGGLE_ICONIFY:            cm_.toggle_iconify(client);                    break; */
    case keyop_t::center_client:         client->center();                         break;
    case keyop_t::client_to_workspace_1: m_clients.client_to_workspace(client, 1); break;
    case keyop_t::client_to_workspace_2: m_clients.client_to_workspace(client, 2); break;
    case keyop_t::client_to_workspace_3: m_clients.client_to_workspace(client, 3); break;
    case keyop_t::client_to_workspace_4: m_clients.client_to_workspace(client, 4); break;
    case keyop_t::client_to_workspace_5: m_clients.client_to_workspace(client, 5); break;
    case keyop_t::client_to_workspace_6: m_clients.client_to_workspace(client, 6); break;
    case keyop_t::client_to_workspace_7: m_clients.client_to_workspace(client, 7); break;
    case keyop_t::client_to_workspace_8: m_clients.client_to_workspace(client, 8); break;
    case keyop_t::client_to_workspace_9: m_clients.client_to_workspace(client, 9); break;
    case keyop_t::float_grow_left:
        {
            if (!(client->floating || m_clients.active_workspace()->in_float_layout()))
                return;

            dim_t dim = client->float_dim;
            pos_t pos = client->float_pos;
            pos.x -= KB_RESIZE_INCREMENT;
            dim.w += KB_RESIZE_INCREMENT;
            client->resize(dim).move(pos);
        }
        break;
    case keyop_t::float_grow_down:
        {
            if (!(client->floating || m_clients.active_workspace()->in_float_layout()))
                return;

            dim_t dim = client->float_dim;
            dim.h += KB_RESIZE_INCREMENT;
            client->resize(dim);
        }
        break;
    case keyop_t::float_grow_up:
        {
            if (!(client->floating || m_clients.active_workspace()->in_float_layout()))
                return;

            dim_t dim = client->float_dim;
            pos_t pos = client->float_pos;
            pos.y -= KB_RESIZE_INCREMENT;
            dim.h += KB_RESIZE_INCREMENT;
            client->resize(dim).move(pos);
        }
        break;
    case keyop_t::float_grow_right:
        {
            if (!(client->floating || m_clients.active_workspace()->in_float_layout()))
                return;

            dim_t dim = client->float_dim;
            dim.w += KB_RESIZE_INCREMENT;
            client->resize(dim);
        }
        break;
    case keyop_t::float_shrink_left:
        {
            if (!(client->floating || m_clients.active_workspace()->in_float_layout()))
                return;

            dim_t dim = client->float_dim;
            pos_t pos = client->float_pos;
            pos.x += KB_RESIZE_INCREMENT;
            dim.w -= KB_RESIZE_INCREMENT;

            if (dim.w >= MIN_WINDOW_SIZE)
                client->resize(dim).move(pos);
        }
        break;
    case keyop_t::float_shrink_down:
        {
            if (!(client->floating || m_clients.active_workspace()->in_float_layout()))
                return;

            dim_t dim = client->float_dim;
            dim.h -= KB_RESIZE_INCREMENT;

            if (dim.h >= MIN_WINDOW_SIZE)
                client->resize(dim);
        }
        break;
    case keyop_t::float_shrink_up:
        {
            if (!(client->floating || m_clients.active_workspace()->in_float_layout()))
                return;

            dim_t dim = client->float_dim;
            pos_t pos = client->float_pos;
            pos.y += KB_RESIZE_INCREMENT;
            dim.h -= KB_RESIZE_INCREMENT;

            if (dim.h >= MIN_WINDOW_SIZE)
                client->resize(dim).move(pos);
        }
        break;
    case keyop_t::float_shrink_right:
        {
            if (!(client->floating || m_clients.active_workspace()->in_float_layout()))
                return;

            dim_t dim = client->float_dim;
            dim.w -= KB_RESIZE_INCREMENT;

            if (dim.w >= MIN_WINDOW_SIZE)
                client->resize(dim);
        }
        break;
    case keyop_t::float_left_or_master_fwd:
        {
            if (client->floating || m_clients.active_workspace()->in_float_layout()) {
                pos_t pos = client->float_pos;
                pos.x -= KB_MOVE_INCREMENT;
                client->move(pos);
            } else {
                // TODO master fwd
            }
        }
        break;
    case keyop_t::float_down_or_stack_bck:
        {
            if (client->floating || m_clients.active_workspace()->in_float_layout()) {
                pos_t pos = client->float_pos;
                pos.y += KB_MOVE_INCREMENT;
                client->move(pos);
            } else {
                // TODO stack bck
            }
        }
        break;
    case keyop_t::float_up_or_stack_fwd:
        {
            if (client->floating || m_clients.active_workspace()->in_float_layout()) {
                pos_t pos = client->float_pos;
                pos.y -= KB_MOVE_INCREMENT;
                client->move(pos);
            } else {
                // TODO stack fwd
            }
        }
        break;
    case keyop_t::float_right_or_master_bck:
        {
            if (client->floating || m_clients.active_workspace()->in_float_layout()) {
                pos_t pos = client->float_pos;
                pos.x += KB_MOVE_INCREMENT;
                client->move(pos);
            } else {
                // TODO master bck
            }
        }
        break;
    case keyop_t::mark_client: m_clients.set_marked(client); break;
    /* case CLIENT_TO_NEXT_WORKSPACE:  cm_.client_to_next_workspace(client);          break; */
    /* case CLIENT_TO_PREV_WORKSPACE:  cm_.client_to_prev_workspace(client);          break; */
    default: break;
    }
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

    if (!client || client->redeem_expect(clientexpect_t::iconify)
        || client->redeem_expect(clientexpect_t::withdraw))
    {
        return;
    }

    /* if (client->iconified) */
    /*     cm_.toggle_iconify(client); */

    pos_t pos = client->pos;
    x_wrapper::window_t frame = client->frame;

    /* if (client->floating) { */
    /*     xh_.destroy_window(client->float_indicator); */
    /*     client->float_indicator = None; */
    /* } */

    client->unmap();
    win.reparent(pos);
    frame.destroy();
}

void
x_events_t::fork_external(::std::string&& command)
{
    if (!fork()) {
        execl("/bin/sh", "/bin/sh", "-c", ("exec " + command).c_str(), NULL);
        exit(1);
    }
}
