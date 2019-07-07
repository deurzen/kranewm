#include "input.hh"
#include "sidebar.hh"
#include "client-model.hh"
#include "x-data/window.hh"

#include <unistd.h>


void
inputhandler_t::process_mouse_input_global(XButtonEvent event)
{
    if (m_mousebinds.count({event.button, event.state, false}))
        switch (m_mousebinds[{event.button, event.state, false}]) {
            case mouseop_t::goto_next_ws:
                {
                    unsigned workspace = m_clients.active_workspace()->get_number();
                    workspace %= USER_WORKSPACES.size();
                    m_clients.change_active_workspace(workspace + 1, false);
                }
                return;
            case mouseop_t::goto_prev_ws:
                {
                    unsigned workspace = m_clients.active_workspace()->get_number() - 1;
                    workspace = (workspace == 0) ? USER_WORKSPACES.size() : workspace;
                    m_clients.change_active_workspace(workspace, false);
                }
                return;
            default: break;
        }
}

void
inputhandler_t::process_mouse_input_client(client_ptr_t client, XButtonEvent event)
{
    if (m_mousebinds.count({event.button, event.state, true}))
        switch (m_mousebinds[{event.button, event.state, true}]) {
        case mouseop_t::client_move:    m_clients.start_moving(client);   break;
        case mouseop_t::client_resize:  m_clients.start_resizing(client); break;
        case mouseop_t::client_center:
            {
                if (client->fullscreen)
                    break;

                if (m_clients.client_user_workspace(client)->in_float_layout() || client->floating) {
                    client->center();
                } else if (!client->parent){
                    client->set_float(clientaction_t::add).resize(client->float_dim).move(client->float_pos);
                    m_clients.active_workspace()->raise_client(client);
                    m_windowstack.apply(m_clients.active_workspace());
                    m_clients.active_workspace()->arrange();
                    m_sidebar.indicate_clientfloating().draw();
                }
            }
            break;
        case mouseop_t::client_next_ws:
            {
                unsigned workspace = m_clients.active_workspace()->get_number();
                workspace %= USER_WORKSPACES.size();
                m_clients.client_to_workspace(client, workspace + 1);
            } // fallthrough
        case mouseop_t::goto_next_ws:
            {
                unsigned workspace = m_clients.active_workspace()->get_number();
                workspace %= USER_WORKSPACES.size();
                m_clients.change_active_workspace(workspace + 1, false);
            }
            return;
        case mouseop_t::client_prev_ws:
            {
                unsigned workspace = m_clients.active_workspace()->get_number() - 1;
                workspace = (workspace == 0) ? USER_WORKSPACES.size() : workspace;
                m_clients.client_to_workspace(client, workspace);
            } // fallthrough
        case mouseop_t::goto_prev_ws:
            {
                unsigned workspace = m_clients.active_workspace()->get_number() - 1;
                workspace = (workspace == 0) ? USER_WORKSPACES.size() : workspace;
                m_clients.change_active_workspace(workspace, false);
            }
            return;
        default: break;
        }
}

void
inputhandler_t::process_key_input_global(XKeyEvent event)
{
    if (m_processbinds.count(event)) {
        m_clients.jump_process(m_processbinds[event]);
        return;
    }

    switch (m_keybinds[event]) {
    case keyop_t::quit: m_running = false; return;
    case keyop_t::spawn_terminal:      fork_external("/usr/local/bin/st");                                     break;
    case keyop_t::spawn_termregion:    fork_external("~/bin/stregion");                                        break;
    case keyop_t::spawn_quickterm:     fork_external("/usr/local/bin/st -n \"kranewm:cf\"");                   break;
    case keyop_t::spawn_quicktermtmux: fork_external("/usr/local/bin/st -e tmux");                             break;
    case keyop_t::spawn_dmenu:         fork_external("/usr/local/bin/dmenu_run");                              break;
    case keyop_t::spawn_dmenupass:     fork_external("/usr/bin/dmenupass");                                    break;
    case keyop_t::spawn_dmenupasscopy: fork_external("/usr/bin/dmenupass --copy");                             break;
    case keyop_t::spawn_browser:       fork_external("/usr/bin/qutebrowser");                                  break;
    case keyop_t::spawn_sec_browser:   fork_external("/usr/bin/firefox");                                      break;
    case keyop_t::audioplay: // fallthrough
    case keyop_t::mpctoggle:           fork_external("/usr/bin/mpc toggle");                                   break;
    case keyop_t::audionext: // fallthrough
    case keyop_t::mpcnext:             fork_external("/usr/bin/mpc next");                                     break;
    case keyop_t::audioprev: // fallthrough
    case keyop_t::mpcprev:             fork_external("/usr/bin/mpc prev");                                     break;
    case keyop_t::audiostop: // fallthrough
    case keyop_t::mpcstop:             fork_external("/usr/bin/mpc stop");                                     break;
    case keyop_t::mpcrandom:           fork_external("/usr/bin/mpc random");                                   break;
    case keyop_t::mpcsingle:           fork_external("/usr/bin/mpc single");                                   break;
    case keyop_t::cantatashow:         fork_external("/usr/bin/cantata");                                      break;
    case keyop_t::rhythmboxshow:       fork_external("/usr/bin/rhythmbox-client");                             break;
    case keyop_t::rhythmboxtoggle:     fork_external("/usr/bin/rhythmbox-client --play-pause");                break;
    case keyop_t::rhythmboxnext:       fork_external("/usr/bin/rhythmbox-client --next");                      break;
    case keyop_t::rhythmboxprev:       fork_external("/usr/bin/rhythmbox-client --previous");                  break;
    case keyop_t::rhythmboxstop:       fork_external("/usr/bin/rhythmbox-client --stop");                      break;
    case keyop_t::volumeup:            fork_external("amixer -D pulse sset Master 5%+");                       break;
    case keyop_t::volumedown:          fork_external("amixer -D pulse sset Master 5%-");                       break;
    case keyop_t::volumemute:          fork_external("amixer -D pulse set Master 1+ toggle");                  break;
    case keyop_t::brightnessup15:      fork_external("/usr/bin/light -A 15");                                  break;
    case keyop_t::brightnessup5:       fork_external("/usr/bin/light -A 5");                                   break;
    case keyop_t::brightnessdown15:    fork_external("/usr/bin/light -U 15");                                  break;
    case keyop_t::take_screenshot:
        fork_external("/usr/bin/maim -m 1 $(date +~/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)");            break;
    case keyop_t::take_screenshot_sel:
        fork_external("/usr/bin/maim -m 1 -s $(date +~/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)");         break;
    case keyop_t::spawn_neomutt:       fork_external("/usr/local/bin/st -g 140x42 -e zsh -i -c neomutt");      break;
    case keyop_t::spawn_ranger:        fork_external("/usr/local/bin/st -g 140x42 -e zsh -i -c ranger");       break;
    case keyop_t::spawn_vifm:          fork_external("/usr/local/bin/st -e zsh -i -c vifmrun");                break;
    case keyop_t::spawn_sncli:         fork_external("/usr/local/bin/st -g 80x42 -e zsh -i -c sncli");         break;
    case keyop_t::spawn_rtv:           fork_external("/usr/local/bin/st -g 80x42 -e zsh -i -c rtv");           break;
    case keyop_t::spawn_irssi:         fork_external("/usr/local/bin/st -g 80x42 -e zsh -i -c irssi");         break;
    case keyop_t::spawn_newsboat:      fork_external("/usr/local/bin/st -g 80x42 -e zsh -i -c newsboat");      break;
    case keyop_t::spawn_sage:          fork_external("/usr/local/bin/st -g 80x22 -e zsh -i -c sage");          break;
    case keyop_t::spawn_gpick:         fork_external("gpick");                                                 break;
    case keyop_t::spawn_nixnote:       fork_external("nixnote2 show_window");                                  break;
    case keyop_t::spawn_qalculate:     fork_external("qalculate-gtk");                                         break;
    case keyop_t::spawn_7lock:         fork_external("systemctl suspend");                                     break;


    case keyop_t::activate_ws_1: m_clients.change_active_workspace(1); break;
    case keyop_t::activate_ws_2: m_clients.change_active_workspace(2); break;
    case keyop_t::activate_ws_3: m_clients.change_active_workspace(3); break;
    case keyop_t::activate_ws_4: m_clients.change_active_workspace(4); break;
    case keyop_t::activate_ws_5: m_clients.change_active_workspace(5); break;
    case keyop_t::activate_ws_6: m_clients.change_active_workspace(6); break;
    case keyop_t::activate_ws_7: m_clients.change_active_workspace(7); break;
    case keyop_t::activate_ws_8: m_clients.change_active_workspace(8); break;
    case keyop_t::activate_ws_9: m_clients.change_active_workspace(9); break;
    case keyop_t::activate_next_ws:
        {
            unsigned workspace = m_clients.active_workspace()->get_number();
            workspace %= USER_WORKSPACES.size();
            m_clients.change_active_workspace(workspace + 1, false);
        }
        break;
    case keyop_t::activate_prev_ws:
        {
            unsigned workspace = m_clients.active_workspace()->get_number() - 1;
            workspace = (workspace == 0) ? USER_WORKSPACES.size() : workspace;
            m_clients.change_active_workspace(workspace, false);
        }
        break;
    case keyop_t::floating:
        {
            m_clients.active_workspace()->set_layout(layout_t::floating);
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::floating).draw();
        }
        break;
    case keyop_t::tile:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::tile).arrange();
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::tile).draw();
        }
        break;
    case keyop_t::stick:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::stick).arrange();
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::stick).draw();
        }
        break;
    case keyop_t::deck:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::deck).arrange();
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::deck).draw();
        }
        break;
    case keyop_t::doubledeck:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::doubledeck).arrange();
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::doubledeck).draw();
        }
        break;
    case keyop_t::grid:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::grid).arrange();
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::grid).draw();
        }
        break;
    case keyop_t::pillar:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::pillar);
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::pillar).draw();
        }
        break;
    case keyop_t::column:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::column);
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::column).draw();
        }
        break;
    case keyop_t::monocle:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::monocle);
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::monocle).draw();
        }
        break;
    case keyop_t::center:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::center);
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();
            m_sidebar.set_layoutsymbol(layout_t::center).draw();
        }
        break;
    case keyop_t::toggle_layout:
        {
            auto workspace = m_clients.active_workspace();
            workspace->set_layout(layout_t::toggle);
            m_windowstack.apply(m_clients.active_workspace());
            workspace->arrange();
            m_sidebar.set_layoutsymbol(m_clients.active_workspace()->get_layout()).draw();
        }
        break;
    case keyop_t::mirror_workspace: m_clients.active_workspace()->mirror().arrange();                   break;
    case keyop_t::focus_bck:  m_clients.cycle_focus_backward();                                         break;
    case keyop_t::focus_fwd:  m_clients.cycle_focus_forward();                                          break;
    case keyop_t::jump_to_marked_client: m_clients.jump_marked();                                       break;
    case keyop_t::toggle_workspace: m_clients.change_active_workspace();                                break;
    case keyop_t::zoom:
        {
            m_clients.active_workspace()->zoom();
            m_clients.sync_workspace_focus();
        }
        break;
    case keyop_t::jump_master:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (!clients.empty()) {
                m_clients.active_workspace()->set_focused(clients[0]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_stack:
        {
            auto clients = m_clients.active_workspace()->get_all();
            unsigned nmaster = m_clients.active_workspace()->get_nmaster();
            if (!clients.empty() && nmaster < clients.size()) {
                m_clients.active_workspace()->set_focused(clients[nmaster]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_last:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (!clients.empty()) {
                m_clients.active_workspace()->set_focused(clients[clients.size() - 1]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_pane:
        {
            m_clients.active_workspace()->jump_pane();
            m_clients.sync_workspace_focus();
        }
        break;
    case keyop_t::jump_client_1:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (clients.size() > 0) {
                m_clients.active_workspace()->set_focused(clients[0]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_client_2:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (clients.size() > 1) {
                m_clients.active_workspace()->set_focused(clients[1]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_client_3:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (clients.size() > 2) {
                m_clients.active_workspace()->set_focused(clients[2]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_client_4:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (clients.size() > 3) {
                m_clients.active_workspace()->set_focused(clients[3]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_client_5:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (clients.size() > 4) {
                m_clients.active_workspace()->set_focused(clients[4]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_client_6:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (clients.size() > 5) {
                m_clients.active_workspace()->set_focused(clients[5]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_client_7:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (clients.size() > 6) {
                m_clients.active_workspace()->set_focused(clients[6]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_client_8:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (clients.size() > 7) {
                m_clients.active_workspace()->set_focused(clients[7]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::jump_client_9:
        {
            auto clients = m_clients.active_workspace()->get_all();
            if (clients.size() > 8) {
                m_clients.active_workspace()->set_focused(clients[8]);
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::inc_mfactor:
        {
            float mfactor = m_clients.active_workspace()->get_mfactor();
            if (mfactor <= 0.85f)
                m_clients.active_workspace()->set_mfactor(mfactor + .05f).arrange();
        }
        break;
    case keyop_t::dec_mfactor:
        {
            float mfactor = m_clients.active_workspace()->get_mfactor();
            if (mfactor >= .15f)
                m_clients.active_workspace()->set_mfactor(mfactor - .05f).arrange();
        }
        break;
    case keyop_t::inc_nmaster:
        {
            unsigned nmaster = m_clients.active_workspace()->get_nmaster();
            if (nmaster < MAX_NMASTER)
                m_clients.active_workspace()->set_nmaster(++nmaster).arrange();
        }
        break;
    case keyop_t::dec_nmaster:
        {
            unsigned nmaster = m_clients.active_workspace()->get_nmaster();
            if (nmaster > 0)
                m_clients.active_workspace()->set_nmaster(--nmaster).arrange();
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
    case keyop_t::reset_gap_size:
        m_clients.active_workspace()->set_gap_size(0).arrange();
        break;
    default: break;
    }
}

void
inputhandler_t::process_key_input_client(client_ptr_t client, XKeyEvent event)
{
    switch (m_keybinds[event]) {
    case keyop_t::kill_client: client->win.force_close();                         break;
    case keyop_t::down_stack:                                                     break;
    case keyop_t::up_stack:                                                       break;
    case keyop_t::down_master:                                                    break;
    case keyop_t::up_master:                                                      break;
    case keyop_t::move_client_fwd: m_clients.active_workspace()->move_forward();  break;
    case keyop_t::move_client_bck: m_clients.active_workspace()->move_backward(); break;
    case keyop_t::toggle_float:
        {
            if (client->parent || client->fullscreen)
                return;

            client->set_float(clientaction_t::toggle).resize(client->float_dim).move(client->float_pos);
            m_clients.active_workspace()->raise_client(client);
            m_windowstack.apply(m_clients.active_workspace());
            m_clients.active_workspace()->arrange();

            if (client->floating)
                m_sidebar.indicate_clientfloating().draw();
            else
                m_sidebar.indicate_clientnormal().draw();
        }
        break;
    case keyop_t::toggle_sticky:
        {
            if (client->parent)
                return;

            m_clients.set_sticky(client, clientaction_t::toggle);
        }
        break;
    case keyop_t::toggle_fullscreen:
        {
            m_clients.set_fullscreen(client, clientaction_t::toggle);

            if (client->fullscreen)
                m_sidebar.indicate_clientfullscreen().draw();
            else if (client->floating)
                m_sidebar.indicate_clientfloating().draw();
            else
                m_sidebar.indicate_clientnormal().draw();
        }
        break;
    case keyop_t::center_client:
        {
            if ((m_clients.client_user_workspace(client)->in_float_layout() || client->floating)
                && !client->fullscreen)
            {
                client->center();
            }
        }
        break;
    case keyop_t::snap_north:
        {
            if ((m_clients.client_user_workspace(client)->in_float_layout() || client->floating)
                && !client->fullscreen)
            {
                client->snap(snapedge_t::north);
            }
        }
        break;
    case keyop_t::snap_east:
        {
            if ((m_clients.client_user_workspace(client)->in_float_layout() || client->floating)
                && !client->fullscreen)
            {
                client->snap(snapedge_t::east);
            }
        }
        break;
    case keyop_t::snap_south:
        {
            if ((m_clients.client_user_workspace(client)->in_float_layout() || client->floating)
                && !client->fullscreen)
            {
                client->snap(snapedge_t::south);
            }
        }
        break;
    case keyop_t::snap_west:
        {
            if ((m_clients.client_user_workspace(client)->in_float_layout() || client->floating)
                && !client->fullscreen)
            {
                client->snap(snapedge_t::west);
            }
        }
        break;
    case keyop_t::mark_client: m_clients.set_marked(client); break;
    case keyop_t::client_to_ws_1:
        {
            m_clients.client_to_workspace(client, 1);
            m_clients.active_workspace()->arrange();
        }
        break;
    case keyop_t::client_to_ws_2:
        {
            m_clients.client_to_workspace(client, 2);
            m_clients.active_workspace()->arrange();
        }
        break;
    case keyop_t::client_to_ws_3:
        {
            m_clients.client_to_workspace(client, 3);
            m_clients.active_workspace()->arrange();
        }
        break;
    case keyop_t::client_to_ws_4:
        {
            m_clients.client_to_workspace(client, 4);
            m_clients.active_workspace()->arrange();
        }
        break;
    case keyop_t::client_to_ws_5:
        {
            m_clients.client_to_workspace(client, 5);
            m_clients.active_workspace()->arrange();
        }
        break;
    case keyop_t::client_to_ws_6:
        {
            m_clients.client_to_workspace(client, 6);
            m_clients.active_workspace()->arrange();
        }
        break;
    case keyop_t::client_to_ws_7:
        {
            m_clients.client_to_workspace(client, 7);
            m_clients.active_workspace()->arrange();
        }
        break;
    case keyop_t::client_to_ws_8:
        {
            m_clients.client_to_workspace(client, 8);
            m_clients.active_workspace()->arrange();
        }
        break;
    case keyop_t::client_to_ws_9:
        {
            m_clients.client_to_workspace(client, 9);
            m_clients.active_workspace()->arrange();
        }
        break;
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
                m_clients.active_workspace()->rotate_master_forward();
                auto clients = m_clients.active_workspace()->get_all();

                if (clients.size() && m_clients.active_workspace()->stack_focused()) {
                    m_clients.active_workspace()->raise_client(clients[0]);
                    m_windowstack.apply(m_clients.active_workspace());
                }

                m_clients.focus(m_clients.active_workspace()->get_focused());
                m_clients.sync_workspace_focus();
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
                m_clients.active_workspace()->rotate_stack_backward();
                unsigned nmaster = m_clients.active_workspace()->get_nmaster();
                auto clients = m_clients.active_workspace()->get_all();

                if (clients.size() && nmaster < clients.size() && m_clients.active_workspace()->master_focused()) {
                    m_clients.active_workspace()->raise_client(clients[nmaster]);
                    m_windowstack.apply(m_clients.active_workspace());
                }

                m_clients.focus(m_clients.active_workspace()->get_focused());
                m_clients.sync_workspace_focus();
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
                m_clients.active_workspace()->rotate_stack_forward();
                unsigned nmaster = m_clients.active_workspace()->get_nmaster();
                auto clients = m_clients.active_workspace()->get_all();

                if (clients.size() && nmaster < clients.size()  && m_clients.active_workspace()->master_focused()) {
                    m_clients.active_workspace()->raise_client(clients[nmaster]);
                    m_windowstack.apply(m_clients.active_workspace());
                }

                m_clients.focus(m_clients.active_workspace()->get_focused());
                m_clients.sync_workspace_focus();
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
                m_clients.active_workspace()->rotate_master_backward();
                auto clients = m_clients.active_workspace()->get_all();

                if (clients.size() && m_clients.active_workspace()->stack_focused()) {
                    m_clients.active_workspace()->raise_client(clients[0]);
                    m_windowstack.apply(m_clients.active_workspace());
                }

                m_clients.focus(m_clients.active_workspace()->get_focused());
                m_clients.sync_workspace_focus();
            }
        }
        break;
    case keyop_t::clients_fwd:
        {
            m_clients.active_workspace()->rotate_clients_forward();
            unsigned nmaster = m_clients.active_workspace()->get_nmaster();
            auto clients = m_clients.active_workspace()->get_all();

            if (clients.size() && m_clients.active_workspace()->stack_focused()) {
                m_clients.active_workspace()->raise_client(clients[0]);
                m_windowstack.apply(m_clients.active_workspace());
            } else if (clients.size() && nmaster < clients.size()  && m_clients.active_workspace()->master_focused()) {
                m_clients.active_workspace()->raise_client(clients[nmaster]);
                m_windowstack.apply(m_clients.active_workspace());
            }

            m_clients.focus(m_clients.active_workspace()->get_focused());
            m_clients.sync_workspace_focus();
        }
        break;
    case keyop_t::clients_bck:
        {
            m_clients.active_workspace()->rotate_clients_backward();
            unsigned nmaster = m_clients.active_workspace()->get_nmaster();
            auto clients = m_clients.active_workspace()->get_all();

            if (clients.size() && m_clients.active_workspace()->stack_focused()) {
                m_clients.active_workspace()->raise_client(clients[0]);
                m_windowstack.apply(m_clients.active_workspace());
            } else if (clients.size() && nmaster < clients.size()  && m_clients.active_workspace()->master_focused()) {
                m_clients.active_workspace()->raise_client(clients[nmaster]);
                m_windowstack.apply(m_clients.active_workspace());
            }

            m_clients.focus(m_clients.active_workspace()->get_focused());
            m_clients.sync_workspace_focus();
        }
        break;
    case keyop_t::client_to_next_ws:
        {
            unsigned workspace = m_clients.active_workspace()->get_number();
            workspace %= USER_WORKSPACES.size();
            m_clients.client_to_workspace(client, workspace + 1);
            m_clients.active_workspace()->arrange();
        }
        break;
    case keyop_t::client_to_prev_ws:
        {
            unsigned workspace = m_clients.active_workspace()->get_number() - 1;
            workspace = (workspace == 0) ? USER_WORKSPACES.size() : workspace;
            m_clients.client_to_workspace(client, workspace);
            m_clients.active_workspace()->arrange();
        }
        break;
    default: break;
    }
}


void
inputhandler_t::fork_external(::std::string&& command)
{
    if (!fork()) {
        execl("/bin/sh", "/bin/sh", "-c", ("exec " + command).c_str(), NULL);
        exit(1);
    }
}
