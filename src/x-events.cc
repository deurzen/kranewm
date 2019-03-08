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
x_events::step()
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
x_events::register_window(x_wrapper::window_t win)
{
    if (m_ewmh.check_apply_strut(win))
        m_clients.active_workspace()->arrange();

    if (!x_wrapper::should_manage(win) || win.is_of_type("DOCK")) {
        m_ewmh.set_frame_extents(win, true);
        return;
    }

    Rule rule = retrieve_rule(win);
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

Rule
x_events::retrieve_rule(x_wrapper::window_t win)
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
x_events::on_button_press()
{
    x_wrapper::window_t win = m_current_event.get().xbutton.window;
    x_wrapper::window_t subwin = m_current_event.get().xbutton.subwindow;
    unsigned button = m_current_event.get().xbutton.button;
    unsigned mask = m_current_event.get().xbutton.state;

    if (win.get() == x_wrapper::g_root.get()) {
        if (subwin.get() == None) {
            if (m_mousebinds.count({button, mask, false})) {
                switch (m_mousebinds[{button, mask, false}]) {
                case GOTO_NEXT_WS: break; // TODO check mask. mask == 0 (no MOD pressed)?
                case GOTO_PREV_WS: break;
                default: break;
                }
            }
        } else
            win = subwin;
    }

    client_ptr_t client = m_clients.win_to_client(win);
    if (client) {
        m_clients.focus(client);
        if (m_mousebinds.count({button, mask, true}))
            switch (m_mousebinds[{button, mask, true}]) {
            case CLIENT_MOVE:    m_clients.start_moving(client);   break;
            case CLIENT_RESIZE:  m_clients.start_resizing(client); break;
            case CLIENT_CENTER:  client->center();                 break;
            case CLIENT_NEXT_WS:                                   break;
            case CLIENT_PREV_WS:                                   break;
            default: break;
            }
        return;
    }
}

void
x_events::on_button_release()
{
    x_wrapper::window_t win = m_current_event.get().xbutton.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (!m_x.is_valid() || (client != m_x.moveresize()->client))
        return;

    auto attrs = x_wrapper::get_attributes(client->frame);

    switch (m_x.moveresize()->state) {
    case MR_MOVE:   m_clients.stop_moving(client, attrs);          break;
    case MR_RESIZE: m_clients.stop_resizing(client, attrs, attrs); break;
    default: break;
    }
}

void
x_events::on_circulate_request()
{
    x_wrapper::propagate_circulate_request(m_current_event);
}

void
x_events::on_client_message()
{
    XClientMessageEvent event = m_current_event.get().xclient;
    x_wrapper::window_t win = event.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (!client)
        return;

    int netwm_index;
    for (netwm_index = 0; netwm_index < NetLast; ++netwm_index)
        if (event.message_type == m_ewmh.get_netwm_atom(netwm_index))
            break;
    if (netwm_index >= NetLast)
        return;

    switch (netwm_index) {
    case NetWmState:
        {
            for (int property = 1; property <= 2; ++property) {
                if (event.data.l[property] == 0)
                    continue;

                if ((Atom)event.data.l[property]
                    == m_ewmh.get_netwm_atom(NetWmStateFullscreen))
                {
                    if (event.data.l[0] >= NetNoAction)
                        return;
                    /* m_clients.toggle_fullscreen(client, event.data.l[0]); */
                } else if ((Atom)event.data.l[property]
                    == m_ewmh.get_netwm_atom(NetWmStateDemandsAttention))
                {
                    if (event.data.l[0] >= NetNoAction)
                        return;
                    /* m_clients.toggle_urgency(client, event.data.l[0]); */
                }
            }
        }
        break;
    case NetActiveWindow:
        {   // if pager or taskbar (source indicator = 2)
            if (event.data.l[0] == 2)
                m_clients.focus(client);
        }
        break;
    default: break;
    }
}

void
x_events::on_configure_request()
{
    x_wrapper::window_t win = m_current_event.get().xconfigurerequest.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (!client) {
        int perm_flags = CWX | CWY | CWWidth | CWHeight;
        x_wrapper::propagate_configure_request(m_current_event, perm_flags);
        return;
    }

    if (is_user_workspace(m_clients.client_workspace(client))) {
        auto workspace = dynamic_cast<user_workspace_ptr_t>(m_clients.client_workspace(client));
        if (!(workspace->in_float_layout() || client->floating))
            return;
    }

    m_current_event.get().xconfigurerequest.width =
        ::std::max(m_current_event.get().xconfigurerequest.width, MIN_WINDOW_SIZE);

    m_current_event.get().xconfigurerequest.height =
        ::std::max(m_current_event.get().xconfigurerequest.height, MIN_WINDOW_SIZE);

    auto before_attrs = x_wrapper::get_attributes(client->frame);

    if (win.get() == client->win.get()) {
        if (m_x.moveresize()->state == MR_RESIZE)
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

    auto after_attrs = x_wrapper::get_attributes(client->frame);

    Pos pos;
    switch (m_x.moveresize()->grabbed_at) {
    case TOP_LEFT:
        pos = {before_attrs.x() + (before_attrs.w() - after_attrs.w()),
            before_attrs.y() + (before_attrs.h() - after_attrs.h())};
        break;
    case TOP_RIGHT:
        pos = {before_attrs.x(),
            before_attrs.y() + (before_attrs.h() - after_attrs.h())};
        break;
    case BOTTOM_LEFT:
        pos = {before_attrs.x() + (before_attrs.w() - after_attrs.w()),
            before_attrs.y()};
        break;
    case BOTTOM_RIGHT: // fallthrough
    default: pos = before_attrs; break;
    }

    if (!(conf_flags & (CWX | CWY))
        && !(Size{before_attrs.w(), before_attrs.h()}
            == Size{after_attrs.w(), after_attrs.h()}))
    {
        client->move(pos);
    }
}

void
x_events::on_configure_notify()
{
    x_wrapper::window_t win = m_current_event.get().xconfigure.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (!client)
        return;

    auto sizehints = x_wrapper::get_sizehints(client->win);
    if (sizehints.success())
        sizehints.get().flags = PSize;

    if (m_x.update_hints(client, sizehints)) {
        client->sizeconstraints.apply(client->pos, client->size);
        client->resize(client->size);
    }
}

void
x_events::on_destroy_notify()
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
}

void
x_events::on_expose()
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
x_events::on_focus_in()
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
x_events::on_key_press()
{
    KeyShortcut shortcut{m_current_event.get().xkey};

    switch (m_keybinds[shortcut]) {
    case QUIT: m_running = false; return;
    case SPAWN_TERMINAL:      fork_external("/usr/bin/urxvt -geometry 80x22");                                break;
    case SPAWN_QUICKTERM:     fork_external("/usr/bin/term -name \"kranewm:float\" -geometry 80x22");         break;
    case SPAWN_QUICKTERMTMUX: fork_external("/usr/bin/term -geometry 80x22 -e tmux");                         break;
    case SPAWN_DMENU:         fork_external("/usr/local/bin/dmenu_run");                                      break;
    case SPAWN_DMENUPASS:     fork_external("/usr/bin/dmenupass");                                            break;
    case SPAWN_DMENUPASSCOPY: fork_external("/usr/bin/dmenupass --copy");                                     break;
    case SPAWN_BROWSER:       fork_external("/usr/bin/qutebrowser");                                          break;
    case SPAWN_SEC_BROWSER:   fork_external("/usr/bin/firefox");                                              break;
    case MPCTOGGLE:           fork_external("/usr/bin/mpc toggle");                                           break;
    case MPCNEXT:             fork_external("/usr/bin/mpc next");                                             break;
    case MPCPREV:             fork_external("/usr/bin/mpc prev");                                             break;
    case MPCSTOP:             fork_external("/usr/bin/mpc stop");                                             break;
    case RHYTHMBOXSHOW:       fork_external("/usr/bin/rhythmbox-client");                                     break;
    case RHYTHMBOXTOGGLE:     fork_external("/usr/bin/rhythmbox-client --play-pause");                        break;
    case RHYTHMBOXNEXT:       fork_external("/usr/bin/rhythmbox-client --next");                              break;
    case RHYTHMBOXPREV:       fork_external("/usr/bin/rhythmbox-client --previous");                          break;
    case RHYTHMBOXSTOP:       fork_external("/usr/bin/rhythmbox-client --stop");                              break;
    case MPCRANDOM:           fork_external("/usr/bin/mpc random");                                           break;
    case MPCSINGLE:           fork_external("/usr/bin/mpc single");                                           break;
    case VOLUMEUP:            fork_external("/usr/bin/pactl set-sink-volume 0 +10%");                         break;
    case VOLUMEDOWN:          fork_external("/usr/bin/pactl set-sink-volume 0 -10%");                         break;
    case VOLUMEMUTE:          fork_external("/usr/bin/pactl set-sink-mute 0 toggle");                         break;
    case BRIGHTNESSUP15:      fork_external("/usr/bin/light -A 15");                                          break;
    case BRIGHTNESSUP5:       fork_external("/usr/bin/light -A 5");                                           break;
    case BRIGHTNESSDOWN15:    fork_external("/usr/bin/light -U 15");                                          break;
    case TAKE_SCREENSHOT:
        fork_external("/usr/bin/maim $(date +/home/deurzen/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)");    break;
    case TAKE_SCREENSHOT_SEL:
        fork_external("/usr/bin/maim -s $(date +/home/deurzen/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)"); break;
    case SPAWN_NEOMUTT:       fork_external("/usr/bin/term -geometry 140x42 -e zsh -i -c neomutt");           break;
    case SPAWN_RANGER:        fork_external("/usr/bin/term -geometry 140x42 -e zsh -i -c ranger");            break;
    case SPAWN_SNCLI:         fork_external("/usr/bin/term -geometry 80x42 -e zsh -i -c sncli");              break;
    case SPAWN_RTV:           fork_external("/usr/bin/term -geometry 80x42 -e zsh -i -c rtv");                break;
    case SPAWN_IRSSI:         fork_external("/usr/bin/term -geometry 80x42 -e zsh -i -c irssi");              break;
    case SPAWN_NEWSBOAT:      fork_external("/usr/bin/term -geometry 80x42 -e zsh -i -c newsboat");           break;
    case SPAWN_SAGE:          fork_external("/usr/bin/term -geometry 80x22 -e zsh -i -c sage");               break;
    case SPAWN_GPICK:         fork_external("gpick");                                                         break;
    case SPAWN_QALCULATE:     fork_external("qalculate-gtk");                                                 break;
    case SPAWN_7LOCK:         fork_external("systemctl suspend");                                             break;


    /* case POP_ICONIFIED:        cm_.deiconify(0);                 break; */
    /* case DEICONIFY_1:          cm_.deiconify(1);                 break; */
    /* case DEICONIFY_2:          cm_.deiconify(2);                 break; */
    /* case DEICONIFY_3:          cm_.deiconify(3);                 break; */
    /* case DEICONIFY_4:          cm_.deiconify(4);                 break; */
    /* case DEICONIFY_5:          cm_.deiconify(5);                 break; */
    /* case DEICONIFY_6:          cm_.deiconify(6);                 break; */
    /* case DEICONIFY_7:          cm_.deiconify(7);                 break; */
    /* case DEICONIFY_8:          cm_.deiconify(8);                 break; */
    /* case DEICONIFY_9:          cm_.deiconify(9);                 break; */
    /* case ACTIVATE_WORKSPACE_1: cm_.change_active_workspace(1);   break; */
    /* case ACTIVATE_WORKSPACE_2: cm_.change_active_workspace(2);   break; */
    /* case ACTIVATE_WORKSPACE_3: cm_.change_active_workspace(3);   break; */
    /* case ACTIVATE_WORKSPACE_4: cm_.change_active_workspace(4);   break; */
    /* case ACTIVATE_WORKSPACE_5: cm_.change_active_workspace(5);   break; */
    /* case ACTIVATE_WORKSPACE_6: cm_.change_active_workspace(6);   break; */
    /* case ACTIVATE_WORKSPACE_7: cm_.change_active_workspace(7);   break; */
    /* case ACTIVATE_WORKSPACE_8: cm_.change_active_workspace(8);   break; */
    /* case ACTIVATE_WORKSPACE_9: cm_.change_active_workspace(9);   break; */
    /* case ACTIVATE_NEXT_WS:     cm_.goto_next_workspace();        break; */
    /* case ACTIVATE_PREV_WS:     cm_.goto_prev_workspace();        break; */
    /* case TOGGLE_SCRATCHPAD_1:  cm_.toggle_scratchpad(1);         break; */
    /* case TOGGLE_SCRATCHPAD_2:  cm_.toggle_scratchpad(2);         break; */
    /* case FLOAT:                cm_.change_layout(LT_FLOAT);      break; */
    /* case TILE:                 cm_.change_layout(LT_TILE);       break; */
    /* case DECK:                 cm_.change_layout(LT_DECK);       break; */
    /* case DOUBLEDECK:           cm_.change_layout(LT_DOUBLEDECK); break; */
    /* case GRID:                 cm_.change_layout(LT_GRID);       break; */
    /* case MONOCLE:              cm_.change_layout(LT_MONOCLE);    break; */
    /* case TOGGLE_LAYOUT:        cm_.change_layout(LT_TOGGLE);     break; */
    /* case SWAP_ORIENTATION:     cm_.swap_orientation();           break; */
    /* case FOCUS_BCK:            cm_.cycle_focus_backward();       break; */
    /* case FOCUS_FWD:            cm_.cycle_focus_forward();        break; */
    /* case ZOOM:                 cm_.zoom();                       break; */
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
    /* case INC_NMASTER:          cm_.change_n_master(1);           break; */
    /* case DEC_NMASTER:          cm_.change_n_master(-1);          break; */
    /* case INC_MRATIO:           cm_.change_m_ratio(0.05f);        break; */
    /* case DEC_MRATIO:           cm_.change_m_ratio(-0.05f);       break; */
    /* case INC_MFACTOR:          cm_.change_m_factor(0.05f);       break; */
    /* case DEC_MFACTOR:          cm_.change_m_factor(-0.05f);      break; */
    /* case INC_GAPSIZE:          cm_.change_gap_size(5);           break; */
    /* case DEC_GAPSIZE:          cm_.change_gap_size(-5);          break; */
    /* case JUMP_TO_MARKED:       cm_.jump_to_marked();             break; */
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
    case KILL_CLIENT:               client->win.force_close();                     break;
    case DOWN_STACK:                                                               break;
    case UP_STACK:                                                                 break;
    case DOWN_MASTER:                                                              break;
    case UP_MASTER:                                                                break;
    /* case MOVE_CLIENT_FWD:           cm_.move_focused_client_forward();             break; */
    /* case MOVE_CLIENT_BCK:           cm_.move_focused_client_backward();            break; */
    /* case TOGGLE_FLOAT:              cm_.toggle_float(client);                      break; */
    /* case TOGGLE_FULLSCREEN:         cm_.toggle_fullscreen(client);                 break; */
    /* case TOGGLE_SHADE:              cm_.toggle_shade(client);                      break; */
    /* case TOGGLE_ICONIFY:            cm_.toggle_iconify(client);                    break; */
    /* case CENTER_CLIENT:             cm_.center_client(client);                     break; */
    /* case CLIENT_TO_WORKSPACE_1:     cm_.client_to_workspace(client, 0u);           break; */
    /* case CLIENT_TO_WORKSPACE_2:     cm_.client_to_workspace(client, 1);            break; */
    /* case CLIENT_TO_WORKSPACE_3:     cm_.client_to_workspace(client, 2);            break; */
    /* case CLIENT_TO_WORKSPACE_4:     cm_.client_to_workspace(client, 3);            break; */
    /* case CLIENT_TO_WORKSPACE_5:     cm_.client_to_workspace(client, 4);            break; */
    /* case CLIENT_TO_WORKSPACE_6:     cm_.client_to_workspace(client, 5);            break; */
    /* case CLIENT_TO_WORKSPACE_7:     cm_.client_to_workspace(client, 6);            break; */
    /* case CLIENT_TO_WORKSPACE_8:     cm_.client_to_workspace(client, 7);            break; */
    /* case CLIENT_TO_WORKSPACE_9:     cm_.client_to_workspace(client, 8);            break; */
    /* case CLIENT_TO_SCRATCHPAD_1:    cm_.client_to_scratchpad(client, 0);           break; */
    /* case CLIENT_TO_SCRATCHPAD_2:    cm_.client_to_scratchpad(client, 1);           break; */
    /* case FLOAT_GROW_LEFT:           cm_.resize_floating_client(client, LEFT, 1);   break; */
    /* case FLOAT_GROW_DOWN:           cm_.resize_floating_client(client, DOWN, 1);   break; */
    /* case FLOAT_GROW_UP:             cm_.resize_floating_client(client, UP, 1);     break; */
    /* case FLOAT_GROW_RIGHT:          cm_.resize_floating_client(client, RIGHT, 1);  break; */
    /* case FLOAT_SHRINK_LEFT:         cm_.resize_floating_client(client, RIGHT, -1); break; */
    /* case FLOAT_SHRINK_DOWN:         cm_.resize_floating_client(client, UP, -1);    break; */
    /* case FLOAT_SHRINK_UP:           cm_.resize_floating_client(client, DOWN, -1);  break; */
    /* case FLOAT_SHRINK_RIGHT:        cm_.resize_floating_client(client, LEFT, -1);  break; */
    /* case MARK_CLIENT:               cm_.set_marked(client);                        break; */
    /* case CLIENT_TO_NEXT_WORKSPACE:  cm_.client_to_next_workspace(client);          break; */
    /* case CLIENT_TO_PREV_WORKSPACE:  cm_.client_to_prev_workspace(client);          break; */
    /* case FLOAT_LEFT_OR_MASTER_FWD: */
    /*     { */
    /*         if (cm_.scratchpad_active() */
    /*             || cm_.get_current_workspace()->layout == LT_FLOAT */
    /*             || client->floating) */
    /*         { */
    /*             cm_.move_floating_client(client, LEFT); */
    /*         } else */
    /*             cm_.rotate_master_forward(); */
    /*     } */
    /*     break; */
    /* case FLOAT_DOWN_OR_STACK_BCK: */
    /*     { */
    /*         if (cm_.scratchpad_active() */
    /*             || cm_.get_current_workspace()->layout == LT_FLOAT */
    /*             || client->floating) */
    /*         { */
    /*             cm_.move_floating_client(client, DOWN); */
    /*         } else */
    /*             cm_.rotate_stack_backward(); */
    /*     } */
    /*     break; */
    /* case FLOAT_UP_OR_STACK_FWD: */
    /*     { */
    /*         if (cm_.scratchpad_active() */
    /*             || cm_.get_current_workspace()->layout == LT_FLOAT */
    /*             || client->floating) */
    /*         { */
    /*             cm_.move_floating_client(client, UP); */
    /*         } else */
    /*             cm_.rotate_stack_forward(); */
    /*     } */
    /*     break; */
    /* case FLOAT_RIGHT_OR_MASTER_BCK: */
    /*     { */
    /*         if (cm_.scratchpad_active() */
    /*             || cm_.get_current_workspace()->layout == LT_FLOAT */
    /*             || client->floating) */
    /*         { */
    /*             cm_.move_floating_client(client, RIGHT); */
    /*         } else */
    /*             cm_.rotate_master_backward(); */
    /*     } */
    /*     break; */
    default: break;
    }
}

void
x_events::on_map_request()
{
    x_wrapper::window_t win = m_current_event.get().xmaprequest.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (client) {
        if (!client->redeem_expect(MAP)
            && is_moveresize_workspace(m_clients.client_workspace(client)))
        {
            m_x.exit_move_resize();
            auto attrs = x_wrapper::get_attributes(client->frame);
            m_clients.stop_moving(client, attrs);
            m_clients.stop_resizing(client, attrs, attrs);
        }

        return;
    }

    if (client && client->redeem_expect(MAP)) {
        return;
    }

    register_window(win);
}

void
x_events::on_map_notify()
{
    x_wrapper::window_t win = m_current_event.get().xmap.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (client) {
        if (!client->redeem_expect(MAP)
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
x_events::on_motion_notify()
{
    client_ptr_t client;
    if (!m_x.is_valid() || !(client = m_x.moveresize()->client))
        return;

    x_wrapper::last_typed_event(m_current_event, MotionNotify);

    auto client_attrs = x_wrapper::get_attributes(client->frame);

    Pos pos = client_attrs;
    Size size = client_attrs;
    Pos delta = m_x.update_pointer(x_wrapper::pointer_position());

    switch (m_x.moveresize()->state) {
    case MR_MOVE:   m_x.moveresize()->process_move_increment(pos, size, delta);   break;
    case MR_RESIZE: m_x.moveresize()->process_resize_increment(pos, size, delta); break;
    default: break;
    }
}

void
x_events::on_property_notify()
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
x_events::on_unmap_notify()
{
    x_wrapper::window_t win = m_current_event.get().xunmap.window;
    client_ptr_t client = m_clients.win_to_client(win);

    if (!client || client->redeem_expect(ICONIFY) || client->redeem_expect(WITHDRAW))
        return;

    /* if (client->iconified) */
    /*     cm_.toggle_iconify(client); */

    Pos pos = client->pos;
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
x_events::fork_external(::std::string&& command)
{
    if (!fork()) {
        execl("/bin/sh", "/bin/sh", "-c", ("exec " + command).c_str(), NULL);
        exit(1);
    }
}
