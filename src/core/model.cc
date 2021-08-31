#include "../winsys/common.hh"
#include "../winsys/util.hh"
#include "defaults.hh"
#include "model.hh"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_set>
#include <vector>

extern "C" {
#include <sys/wait.h>
#include <unistd.h>
}

#ifdef ENABLE_IPC
const bool IPC_ENABLED = true;
#else
const bool IPC_ENABLED = false;
#endif

#ifdef DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

#include "spdlog/spdlog.h"

using namespace winsys;

static Model* g_instance;

Model::Model(Connection& conn)
    : m_conn(conn),
      m_running(true),
      m_partitions({}, true),
      m_contexts({}, true),
      m_workspaces({}, true),
      mp_partition(nullptr),
      mp_context(nullptr),
      mp_workspace(nullptr),
      mp_prev_partition(nullptr),
      mp_prev_context(nullptr),
      mp_prev_workspace(nullptr),
      m_move_buffer(Buffer::BufferKind::Move),
      m_resize_buffer(Buffer::BufferKind::Resize),
      m_stack({}),
      m_order({}),
      m_client_map({}),
      m_pid_map({}),
      m_fullscreen_map({}),
      m_sticky_clients({}),
      m_unmanaged_windows({}),
      m_ignored_producers({}),
      m_ignored_consumers({}),
      mp_focus(nullptr),
      mp_jumped_from(nullptr),
      m_key_bindings({
#define CALL(args) [](Model& model) {model.args;}
          { { Key::Q, { Main, Ctrl, Shift } },
              CALL(exit())
          },

          // client state modifiers
          { { Key::C, { Main } },
              CALL(kill_focus())
          },
          { { Key::Space, { Main, Shift } },
              CALL(set_floating_focus(Toggle::Reverse))
          },
          { { Key::F, { Main } },
              CALL(set_fullscreen_focus(Toggle::Reverse))
          },
          { { Key::X, { Main } },
              CALL(set_sticky_focus(Toggle::Reverse))
          },
          { { Key::F, { Main, Sec, Ctrl } },
              CALL(set_contained_focus(Toggle::Reverse))
          },
          { { Key::I, { Main, Sec, Ctrl } },
              CALL(set_invincible_focus(Toggle::Reverse))
          },
          { { Key::P, { Main, Sec, Ctrl } },
              CALL(set_producing_focus(Toggle::Reverse))
          },
          { { Key::Y, { Main } },
              CALL(set_iconify_focus(Toggle::Reverse))
          },
          { { Key::U, { Main } },
              CALL(pop_deiconify())
          },
          { { Key::U, { Main, Sec } },
              CALL(deiconify_all())
          },

          // free client arrangers
          { { Key::Space, { Main, Ctrl } },
              CALL(center_focus())
          },
          { { Key::H, { Main, Ctrl } },
              CALL(nudge_focus(Edge::Left, 15))
          },
          { { Key::J, { Main, Ctrl } },
              CALL(nudge_focus(Edge::Bottom, 15))
          },
          { { Key::K, { Main, Ctrl } },
              CALL(nudge_focus(Edge::Top, 15))
          },
          { { Key::L, { Main, Ctrl } },
              CALL(nudge_focus(Edge::Right, 15))
          },
          { { Key::H, { Main, Ctrl, Shift } },
              CALL(stretch_focus(Edge::Left, 15))
          },
          { { Key::J, { Main, Ctrl, Shift } },
              CALL(stretch_focus(Edge::Bottom, 15))
          },
          { { Key::K, { Main, Ctrl, Shift } },
              CALL(stretch_focus(Edge::Top, 15))
          },
          { { Key::L, { Main, Ctrl, Shift } },
              CALL(stretch_focus(Edge::Right, 15))
          },
          { { Key::Y, { Main, Ctrl, Shift } },
              CALL(stretch_focus(Edge::Left, -15))
          },
          { { Key::U, { Main, Ctrl, Shift } },
              CALL(stretch_focus(Edge::Bottom, -15))
          },
          { { Key::I, { Main, Ctrl, Shift } },
              CALL(stretch_focus(Edge::Top, -15))
          },
          { { Key::O, { Main, Ctrl, Shift } },
              CALL(stretch_focus(Edge::Right, -15))
          },
          { { Key::Left, { Main, Ctrl } },
              CALL(snap_focus(Edge::Left))
          },
          { { Key::Down, { Main, Ctrl } },
              CALL(snap_focus(Edge::Bottom))
          },
          { { Key::Up, { Main, Ctrl } },
              CALL(snap_focus(Edge::Top))
          },
          { { Key::Right, { Main, Ctrl } },
              CALL(snap_focus(Edge::Right))
          },

          // client order modifiers
          { { Key::J, { Main } },
              CALL(cycle_focus(Direction::Forward))
          },
          { { Key::K, { Main } },
              CALL(cycle_focus(Direction::Backward))
          },
          { { Key::J, { Main, Shift } },
              CALL(drag_focus(Direction::Forward))
          },
          { { Key::K, { Main, Shift } },
              CALL(drag_focus(Direction::Backward))
          },
          { { Key::SemiColon, { Main, Shift } },
              CALL(rotate_clients(Direction::Forward))
          },
          { { Key::Comma, { Main, Shift } },
              CALL(rotate_clients(Direction::Backward))
          },

          // client jump criteria
          { { Key::B, { Main } },
              CALL(jump_client({
                  SearchSelector::SelectionCriterium::ByClassEquals,
                  "qutebrowser"
              }))
          },
          { { Key::B, { Main, Shift } },
              CALL(jump_client({
                  SearchSelector::SelectionCriterium::ByClassEquals,
                  "Firefox"
              }))
          },
          { { Key::B, { Main, Ctrl } },
              CALL(jump_client({
                  SearchSelector::SelectionCriterium::ByClassContains,
                  "Chromium"
              }))
          },
          { { Key::Space, { Main, Sec } },
              CALL(jump_client({
                  SearchSelector::SelectionCriterium::ByClassEquals,
                  "Spotify"
              }))
          },
          { { Key::E, { Main } },
              CALL(jump_client({
                  SearchSelector::SelectionCriterium::ByNameContains,
                  "[vim]"
              }))
          },
          { { Key::Comma, { Main } },
              CALL(jump_client({
                  model.active_workspace(),
                  Workspace::ClientSelector::SelectionCriterium::AtFirst
              }))
          },
          { { Key::Period, { Main } },
              CALL(jump_client({
                  model.active_workspace(),
                  Workspace::ClientSelector::SelectionCriterium::AtMain
              }))
          },
          { { Key::Slash, { Main } },
              CALL(jump_client({
                  model.active_workspace(),
                  Workspace::ClientSelector::SelectionCriterium::AtLast
              }))
          },

          // workspace layout modifiers
          { { Key::F, { Main, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::Float))
          },
          { { Key::L, { Main, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::BLFloat))
          },
          { { Key::Z, { Main } },
              CALL(set_layout(LayoutHandler::LayoutKind::SingleFloat))
          },
          { { Key::Z, { Main, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::BLSingleFloat))
          },
          { { Key::M, { Main } },
              CALL(set_layout(LayoutHandler::LayoutKind::Monocle))
          },
          { { Key::G, { Main } },
              CALL(set_layout(LayoutHandler::LayoutKind::Center))
          },
          { { Key::T, { Main } },
              CALL(set_layout(LayoutHandler::LayoutKind::Stack))
          },
          { { Key::T, { Main, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::SStack))
          },
          { { Key::P, { Main, Ctrl, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::Paper))
          },
          { { Key::P, { Main, Sec, Ctrl, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::SPaper))
          },
          { { Key::B, { Main, Ctrl, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::BStack))
          },
          { { Key::B, { Main, Sec, Ctrl, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::SBStack))
          },
          { { Key::Y, { Main, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::Horz))
          },
          { { Key::Y, { Main, Ctrl } },
              CALL(set_layout(LayoutHandler::LayoutKind::SHorz))
          },
          { { Key::V, { Main, Shift } },
              CALL(set_layout(LayoutHandler::LayoutKind::Vert))
          },
          { { Key::V, { Main, Ctrl } },
              CALL(set_layout(LayoutHandler::LayoutKind::SVert))
          },
          { { Key::F, { Main, Ctrl, Shift } },
              CALL(set_layout_retain_region(LayoutHandler::LayoutKind::Float))
          },
          { { Key::Space, { Main } },
              CALL(toggle_layout())
          },

          // workspace layout data modifiers
          { { Key::Equal, { Main } },
              CALL(change_gap_size(2))
          },
          { { Key::Minus, { Main } },
              CALL(change_gap_size(-2))
          },
          { { Key::Equal, { Main, Shift } },
              CALL(reset_gap_size())
          },
          { { Key::I, { Main } },
              CALL(change_main_count(1))
          },
          { { Key::D, { Main } },
              CALL(change_main_count(-1))
          },
          { { Key::L, { Main } },
              CALL(change_main_factor(.05f))
          },
          { { Key::H, { Main } },
              CALL(change_main_factor(-.05f))
          },
          { { Key::PageUp, { Main, Shift } },
              CALL(change_margin(5))
          },
          { { Key::PageDown, { Main, Shift } },
              CALL(change_margin(-5))
          },
          { { Key::Left, { Main, Shift } },
              CALL(change_margin(Edge::Left, 5))
          },
          { { Key::Left, { Main, Ctrl, Shift } },
              CALL(change_margin(Edge::Left, -5))
          },
          { { Key::Up, { Main, Shift } },
              CALL(change_margin(Edge::Top, 5))
          },
          { { Key::Up, { Main, Ctrl, Shift } },
              CALL(change_margin(Edge::Top, -5))
          },
          { { Key::Right, { Main, Shift } },
              CALL(change_margin(Edge::Right, 5))
          },
          { { Key::Right, { Main, Ctrl, Shift } },
              CALL(change_margin(Edge::Right, -5))
          },
          { { Key::Down, { Main, Shift } },
              CALL(change_margin(Edge::Bottom, 5))
          },
          { { Key::Down, { Main, Ctrl, Shift } },
              CALL(change_margin(Edge::Bottom, -5))
          },
          { { Key::Comma, { Main, Ctrl, Shift } },
              CALL(cycle_layout_data(Direction::Backward))
          },
          { { Key::Period, { Main, Ctrl, Shift } },
              CALL(cycle_layout_data(Direction::Forward))
          },
          { { Key::Slash, { Main, Ctrl, Shift } },
              CALL(toggle_layout_data())
          },
          { { Key::Delete, { Main, Ctrl, Shift } },
              CALL(copy_data_from_prev_layout())
          },
          { { Key::Equal, { Main, Ctrl, Shift } },
              CALL(reset_margin())
          },
          { { Key::Equal, { Main, Sec, Ctrl, Shift } },
              CALL(reset_layout_data())
          },

          // workspace layout storage and retrieval
          { { Key::F1, { Main, Shift } },
              CALL(save_layout(0))
          },
          { { Key::F2, { Main, Shift } },
              CALL(save_layout(1))
          },
          { { Key::F3, { Main, Shift } },
              CALL(save_layout(2))
          },
          { { Key::F4, { Main, Shift } },
              CALL(save_layout(3))
          },
          { { Key::F5, { Main, Shift } },
              CALL(save_layout(4))
          },
          { { Key::F6, { Main, Shift } },
              CALL(save_layout(5))
          },
          { { Key::F7, { Main, Shift } },
              CALL(save_layout(6))
          },
          { { Key::F8, { Main, Shift } },
              CALL(save_layout(7))
          },
          { { Key::F9, { Main, Shift } },
              CALL(save_layout(8))
          },
          { { Key::F10, { Main, Shift } },
              CALL(save_layout(9))
          },
          { { Key::F11, { Main, Shift } },
              CALL(save_layout(10))
          },
          { { Key::F12, { Main, Shift } },
              CALL(save_layout(11))
          },
          { { Key::F1, { Main } },
              CALL(load_layout(0))
          },
          { { Key::F2, { Main } },
              CALL(load_layout(1))
          },
          { { Key::F3, { Main } },
              CALL(load_layout(2))
          },
          { { Key::F4, { Main } },
              CALL(load_layout(3))
          },
          { { Key::F5, { Main } },
              CALL(load_layout(4))
          },
          { { Key::F6, { Main } },
              CALL(load_layout(5))
          },
          { { Key::F7, { Main } },
              CALL(load_layout(6))
          },
          { { Key::F8, { Main } },
              CALL(load_layout(7))
          },
          { { Key::F9, { Main } },
              CALL(load_layout(8))
          },
          { { Key::F10, { Main } },
              CALL(load_layout(9))
          },
          { { Key::F11, { Main } },
              CALL(load_layout(10))
          },
          { { Key::F12, { Main } },
              CALL(load_layout(11))
          },

          // workspace activators
          { { Key::Escape, { Main } },
              CALL(toggle_workspace())
          },
          { { Key::RightBracket, { Main } },
              CALL(activate_next_workspace(Direction::Forward))
          },
          { { Key::LeftBracket, { Main } },
              CALL(activate_next_workspace(Direction::Backward))
          },
          { { Key::One, { Main } },
              CALL(activate_workspace(Util::Change<Index> { 0 }))
          },
          { { Key::Two, { Main } },
              CALL(activate_workspace(1))
          },
          { { Key::Three, { Main } },
              CALL(activate_workspace(2))
          },
          { { Key::Four, { Main } },
              CALL(activate_workspace(3))
          },
          { { Key::Five, { Main } },
              CALL(activate_workspace(4))
          },
          { { Key::Six, { Main } },
              CALL(activate_workspace(5))
          },
          { { Key::Seven, { Main } },
              CALL(activate_workspace(6))
          },
          { { Key::Eight, { Main } },
              CALL(activate_workspace(7))
          },
          { { Key::Nine, { Main } },
              CALL(activate_workspace(8))
          },
          { { Key::Zero, { Main } },
              CALL(activate_workspace(9))
          },

          // workspace client movers
          { { Key::RightBracket, { Main, Shift } },
              CALL(move_focus_to_next_workspace(Direction::Forward))
          },
          { { Key::LeftBracket, { Main, Shift } },
              CALL(move_focus_to_next_workspace(Direction::Backward))
          },
          { { Key::One, { Main, Shift } },
              CALL(move_focus_to_workspace(0))
          },
          { { Key::Two, { Main, Shift } },
              CALL(move_focus_to_workspace(1))
          },
          { { Key::Three, { Main, Shift } },
              CALL(move_focus_to_workspace(2))
          },
          { { Key::Four, { Main, Shift } },
              CALL(move_focus_to_workspace(3))
          },
          { { Key::Five, { Main, Shift } },
              CALL(move_focus_to_workspace(4))
          },
          { { Key::Six, { Main, Shift } },
              CALL(move_focus_to_workspace(5))
          },
          { { Key::Seven, { Main, Shift } },
              CALL(move_focus_to_workspace(6))
          },
          { { Key::Eight, { Main, Shift } },
              CALL(move_focus_to_workspace(7))
          },
          { { Key::Nine, { Main, Shift } },
              CALL(move_focus_to_workspace(8))
          },
          { { Key::Zero, { Main, Shift } },
              CALL(move_focus_to_workspace(9))
          },

          // screen region modifiers
          { { Key::V, { Main } },
              CALL(activate_screen_struts(Toggle::Reverse))
          },

          // external commands
#define CALL_EXTERNAL(command) CALL(spawn_external(#command))
          { { Key::PlayPause, {} },
              CALL_EXTERNAL(playerctl play-pause)
          },
          { { Key::P, { Main, Sec } },
              CALL_EXTERNAL(playerctl play-pause)
          },
          { { Key::PreviousTrack, {} },
              CALL_EXTERNAL(playerctl previous)
          },
          { { Key::K, { Main, Sec } },
              CALL_EXTERNAL(playerctl previous)
          },
          { { Key::NextTrack, {} },
              CALL_EXTERNAL(playerctl next)
          },
          { { Key::J, { Main, Sec } },
              CALL_EXTERNAL(playerctl next)
          },
          { { Key::StopMedia, {} },
              CALL_EXTERNAL(playerctl stop)
          },
          { { Key::BackSpace, { Main, Sec } },
              CALL_EXTERNAL(playerctl stop)
          },
          { { Key::VolumeMute, {} },
              CALL_EXTERNAL(amixer -D pulse sset Master toggle)
          },
          { { Key::VolumeDown, {} },
              CALL_EXTERNAL(amixer -D pulse sset Master 5%-)
          },
          { { Key::VolumeUp, {} },
              CALL_EXTERNAL(amixer -D pulse sset Master 5%+)
          },
          { { Key::VolumeMute, { Shift } },
              CALL_EXTERNAL(amixer -D pulse sset Capture toggle)
          },
          { { Key::MicMute, {} },
              CALL_EXTERNAL(amixer -D pulse sset Capture toggle)
          },
          { { Key::Return, { Main } },
              CALL_EXTERNAL(st)
          },
          { { Key::Return, { Main, Shift } },
              CALL(spawn_external("st -n " + WM_NAME + ":cf"))
          },
          { { Key::P, { Main } },
              CALL_EXTERNAL(dmenu_run)
          },
          { { Key::Q, { Main } },
              CALL_EXTERNAL(qutebrowser)
          },
          { { Key::Q, { Main, Shift } },
              CALL_EXTERNAL(firefox)
          },
          { { Key::Q, { Main, Ctrl } },
              CALL_EXTERNAL(chromium)
          },
          { { Key::P, { Main, Shift } },
              CALL_EXTERNAL($HOME/bin/dmenupass)
          },
          { { Key::P, { Main, Ctrl } },
              CALL_EXTERNAL($HOME/bin/dmenupass --copy)
          },
          { { Key::O, { Main, Shift } },
              CALL_EXTERNAL($HOME/bin/dmenunotify)
          },
          { { Key::G, { Main, Shift } },
              CALL_EXTERNAL($HOME/bin/grabcolor)
          },
          { { Key::PrintScreen, { Main } },
              CALL_EXTERNAL($HOME/bin/screenshot -s)
          },
          { { Key::PrintScreen, { Main, Shift } },
              CALL_EXTERNAL($HOME/bin/screenshot)
          },
#undef CALL_EXTERNAL
#undef CALL
      }),
      m_mouse_bindings({
          { { MouseInput::MouseInputTarget::Client, Button::Right, { Main, Ctrl } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.set_floating_client(Toggle::Reverse, client);

                return true;
             }
          },
          { { MouseInput::MouseInputTarget::Client, Button::Middle, { Main, Ctrl, Shift } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.set_fullscreen_client(Toggle::Reverse, client);

                  return true;
             }
          },
          { { MouseInput::MouseInputTarget::Client, Button::Middle, { Main } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.center_client(client);

                  return true;
             }
          },
          { { MouseInput::MouseInputTarget::Client, Button::ScrollDown, { Main, Ctrl, Shift } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.inflate_client(-16, client);

                  return true;
             }
          },
          { { MouseInput::MouseInputTarget::Client, Button::ScrollUp, { Main, Ctrl, Shift } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.inflate_client(16, client);

                  return true;
             }
          },
          { { MouseInput::MouseInputTarget::Client, Button::Left, { Main } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.start_moving(client);

                  return true;
             }
          },
          { { MouseInput::MouseInputTarget::Client, Button::Right, { Main } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.start_resizing(client);

                  return true;
             }
          },
          { { MouseInput::MouseInputTarget::Global, Button::ScrollDown, { Main } },
             [](Model& model, Client_ptr) {
                  model.cycle_focus(Direction::Forward);
                  return false;
             }
          },
          { { MouseInput::MouseInputTarget::Global, Button::ScrollUp, { Main } },
             [](Model& model, Client_ptr) {
                  model.cycle_focus(Direction::Backward);
                  return false;
             }
          },
          { { MouseInput::MouseInputTarget::Global, Button::ScrollDown, { Main, Shift } },
             [](Model& model, Client_ptr) {
                  model.activate_next_workspace(Direction::Forward);
                  return false;
             }
          },
          { { MouseInput::MouseInputTarget::Global, Button::ScrollUp, { Main, Shift } },
             [](Model& model, Client_ptr) {
                  model.activate_next_workspace(Direction::Backward);
                  return false;
             }
          },
          { { MouseInput::MouseInputTarget::Client, Button::Forward, { Main } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.move_client_to_next_workspace(Direction::Forward, client);

                  return false;
             }
          },
          { { MouseInput::MouseInputTarget::Client, Button::Backward, { Main } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.move_client_to_next_workspace(Direction::Backward, client);

                  return false;
             }
          },
          { { MouseInput::MouseInputTarget::Client, Button::Right, { Main, Ctrl, Shift } },
             [](Model& model, Client_ptr client) {
                  if (client)
                      model.kill_client(client);

                  return false;
             }
          },
          { { MouseInput::MouseInputTarget::Global, Button::Left, { Main, Sec, Ctrl } },
             [](Model& model, Client_ptr) {
                  model.spawn_external("st -n kranewm:cf");
                  return false;
             }
          },
      })
{
#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    g_instance = this;

    static const std::vector<std::string> context_names = {
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j"
    };

    static const std::vector<std::string> workspace_names = {
        "1:main", "2:web", "3:term", "4", "5", "6", "7", "8", "9", "10"
    };

    for (std::size_t i = 0; i < context_names.size(); ++i)
        m_contexts.insert_at_back(new Context(i, context_names[i]));

    for (std::size_t i = 0; i < workspace_names.size(); ++i)
        m_workspaces.insert_at_back(new Workspace(i, workspace_names[i]));

    if constexpr (IPC_ENABLED)
        m_conn.init_wm_ipc();

    acquire_partitions();
    m_conn.init_for_wm(workspace_names);

    m_contexts.activate_at_index(0);
    m_workspaces.activate_at_index(0);

    mp_context = *m_contexts.active_element();
    mp_workspace = *m_workspaces.active_element();

    m_conn.set_current_desktop(0);

    std::vector<KeyInput> key_inputs(m_key_bindings.size());
    std::vector<MouseInput> mouse_inputs(m_mouse_bindings.size());

    std::transform(
        m_key_bindings.begin(),
        m_key_bindings.end(),
        key_inputs.begin(),
        [](auto kv) -> KeyInput { return kv.first; }
    );

    std::transform(
        m_mouse_bindings.begin(),
        m_mouse_bindings.end(),
        mouse_inputs.begin(),
        [](auto kv) -> MouseInput { return kv.first; }
    );

    m_conn.grab_bindings(key_inputs, mouse_inputs);

    for (auto& window : m_conn.top_level_windows())
        manage(window, !m_conn.must_manage_window(window), true);

    { // user configurations
        std::stringstream configdir_ss;

        if (const char* env_xdgconf = std::getenv("XDG_CONFIG_HOME"))
            configdir_ss << env_xdgconf << "/" << WM_NAME << "/";
        else
            configdir_ss << "$HOME/.config/" << WM_NAME << "/";

        { // produce vector of to-ignore-{producers,consumers}
            std::ifstream in(configdir_ss.str() + std::string("consumeignore"));

            if (in.good()) {
                std::string line;

                while (std::getline(in, line)) {
                    std::string::size_type pos = line.find('#');

                    if (pos != std::string::npos)
                        line = line.substr(0, pos);

                    if (line.length() < 5)
                        continue;

                    line.erase(4, line.find_first_not_of(" \t\n\r\f\v"));
                    line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

                    if (line.length() < 5)
                        continue;

                    std::vector<SearchSelector_ptr>* ignored;
                    switch (line[0]) {
                    case '<': ignored = &m_ignored_producers; break;
                    case '>': ignored = &m_ignored_consumers; break;
                    default: continue;
                    }

                    SearchSelector::SelectionCriterium criterium;

                    switch (line[2]) {
                    case 'N':
                    {
                        switch (line[1]) {
                        case '=': criterium = SearchSelector::SelectionCriterium::ByNameEquals;   break;
                        case '~': criterium = SearchSelector::SelectionCriterium::ByNameContains; break;
                        default: continue;
                        }

                        break;
                    }
                    case 'C':
                    {
                        switch (line[1]) {
                        case '=': criterium = SearchSelector::SelectionCriterium::ByClassEquals;   break;
                        case '~': criterium = SearchSelector::SelectionCriterium::ByClassContains; break;
                        default: continue;
                        }

                        break;
                    }
                    case 'I':
                    {
                        switch (line[1]) {
                        case '=': criterium = SearchSelector::SelectionCriterium::ByInstanceEquals;   break;
                        case '~': criterium = SearchSelector::SelectionCriterium::ByInstanceContains; break;
                        default: continue;
                        }

                        break;
                    }
                    default: continue;
                    }

                    ignored->push_back(new SearchSelector{criterium, line.substr(4)});
                }
            }
        }

#ifndef DEBUG
        { // run user-configured autostart programs
            spawn_external(configdir_ss.str() + std::string("blocking_autostart"));
            spawn_external(configdir_ss.str() + std::string("nonblocking_autostart &"));
            spdlog::info("ran autostart scripts");
        }
#endif
    }

    spdlog::info("initialized " + WM_NAME);
}

Model::~Model()
{
    for (std::size_t i = 0; i < m_partitions.size(); ++i)
        delete m_partitions[i];

    for (std::size_t i = 0; i < m_contexts.size(); ++i)
        delete m_contexts[i];

    for (std::size_t i = 0; i < m_workspaces.size(); ++i)
        delete m_workspaces[i];

    std::unordered_set<Client_ptr> clients = {};

    for (auto [_,client] : m_client_map)
        clients.insert(client);

    for (Client_ptr client : clients)
        delete client;

    for (std::size_t i = 0; i < m_ignored_producers.size(); ++i)
        delete m_ignored_producers[i];

    for (std::size_t i = 0; i < m_ignored_consumers.size(); ++i)
        delete m_ignored_consumers[i];

    m_partitions.clear();
    m_contexts.clear();
    m_workspaces.clear();
    m_client_map.clear();
}


void
Model::run()
{
    while (m_running)
        if constexpr (IPC_ENABLED) {
            if (m_conn.check_progress()) {
                // process IPC message
                m_conn.process_messages(
                    [=,this](winsys::Message message) {
                        std::visit(m_message_visitor, message);
                    }
                );

                // process windowing system event
                m_conn.process_events(
                    [=,this](winsys::Event event) {
                        std::visit(m_event_visitor, event);
                    }
                );
            }
        } else
            std::visit(m_event_visitor, m_conn.step());
}


void
Model::init_signals() const
{
    struct sigaction child_sa, exit_sa, ignore_sa;

    std::memset(&child_sa, 0, sizeof(child_sa));
    child_sa.sa_handler = &Model::wait_children;

    std::memset(&exit_sa, 0, sizeof(exit_sa));
    exit_sa.sa_handler = &Model::handle_signal;

    std::memset(&ignore_sa, 0, sizeof(ignore_sa));
    ignore_sa.sa_handler = SIG_IGN;

    sigaction(SIGCHLD, &child_sa,  NULL);
    sigaction(SIGINT,  &exit_sa,   NULL);
    sigaction(SIGHUP,  &exit_sa,   NULL);
    sigaction(SIGINT,  &exit_sa,   NULL);
    sigaction(SIGTERM, &exit_sa,   NULL);
    sigaction(SIGPIPE, &ignore_sa, NULL);
}

// window management actions

void
Model::acquire_partitions()
{
    std::size_t index = m_partitions.active_index();

    for (std::size_t i = 0; i < m_partitions.size(); ++i)
        delete m_partitions[i];

    m_partitions.clear();

    std::vector<Screen> connected_outputs = m_conn.connected_outputs();

    for (std::size_t i = 0; i < connected_outputs.size(); ++i)
        m_partitions.insert_at_back(new Partition(
            connected_outputs[i],
            i
        ));

    if (m_partitions.empty()) {
        spdlog::warn("could not acquire any partitions");
        return;
    }

    if (index < m_partitions.size())
        m_partitions.activate_at_index(index);
    else
        m_partitions.activate_at_index(0);

    mp_partition = *m_partitions.active_element();
    Screen& screen = mp_partition->screen();

    screen.compute_placeable_region();

    std::vector<Region> workspace_regions
        = { m_workspaces.size(), screen.full_region() };

    m_conn.set_desktop_geometry(workspace_regions);
    m_conn.set_desktop_viewport(workspace_regions);
    m_conn.set_workarea(workspace_regions);

    for (auto& [_,client] : m_client_map) {
        Region screen_region = screen.full_region();
        Region client_region = client->free_region;

        if (client_region.pos.x >= screen_region.dim.w)
            client_region.pos.x = screen_region.dim.w - client_region.dim.w;

        if (client_region.pos.y >= screen_region.dim.h)
            client_region.pos.y = screen_region.dim.h - client_region.dim.h;

        client->set_free_region(client_region);
    }

    spdlog::debug("acquired {} partitions", m_partitions.size());
}

const Screen&
Model::active_screen() const
{
    return mp_partition->const_screen();
}

Client_ptr
Model::get_client(Window window)
{
    return Util::retrieve(m_client_map, window).value_or(nullptr);
}

Client_ptr
Model::get_const_client(Window window) const
{
    return Util::const_retrieve(m_client_map, window).value_or(nullptr);
}


Client_ptr
Model::search_client(SearchSelector const& selector)
{
    static constexpr struct LastFocusedComparer final {
        bool
        operator()(const Client_ptr lhs, const Client_ptr rhs) const
        {
            return lhs->last_focused < rhs->last_focused;
        }
    } last_focused_comparer = {};

    static std::set<Client_ptr, LastFocusedComparer> clients
        = {{}, last_focused_comparer};

    clients.clear();

    switch (selector.criterium()) {
    case SearchSelector::SelectionCriterium::OnWorkspaceBySelector:
    {
        auto const& [index,selector_] = selector.workspace_selector();

        if (index <= m_workspaces.size()) {
            Workspace_ptr workspace = m_workspaces[index];
            std::optional<Client_ptr> client = workspace->find_client(selector_);

            if (client && (*client)->managed)
                clients.insert(*client);
        }

        break;
    }
    case SearchSelector::SelectionCriterium::ByNameEquals:
    {
        std::string const& name = selector.string_value();

        for (auto const&[_,client] : m_client_map)
            if (client->managed && client->name == name)
                clients.insert(client);

        break;
    }
    case SearchSelector::SelectionCriterium::ByClassEquals:
    {
        std::string const& class_ = selector.string_value();

        for (auto const&[_,client] : m_client_map)
            if (client->managed && client->class_ == class_)
                clients.insert(client);

        break;
    }
    case SearchSelector::SelectionCriterium::ByInstanceEquals:
    {
        std::string const& instance = selector.string_value();

        for (auto const&[_,client] : m_client_map)
            if (client->managed && client->instance == instance)
                clients.insert(client);

        break;
    }
    case SearchSelector::SelectionCriterium::ByNameContains:
    {
        std::string const& name = selector.string_value();

        for (auto const&[_,client] : m_client_map)
            if (client->managed && client->name.find(name) != std::string::npos)
                clients.insert(client);

        break;
    }
    case SearchSelector::SelectionCriterium::ByClassContains:
    {
        std::string const& class_ = selector.string_value();

        for (auto const&[_,client] : m_client_map)
            if (client->managed && client->class_.find(class_) != std::string::npos)
                clients.insert(client);

        break;
    }
    case SearchSelector::SelectionCriterium::ByInstanceContains:
    {
        std::string const& instance = selector.string_value();

        for (auto const&[_,client] : m_client_map)
            if (client->managed && client->instance.find(instance) != std::string::npos)
                clients.insert(client);

        break;
    }
    case SearchSelector::SelectionCriterium::ForCondition:
    {
        std::function<bool(const Client_ptr)> const& filter = selector.filter();

        for (auto const&[_,client] : m_client_map)
            if (client->managed && filter(client))
                clients.insert(client);

        break;
    }
    default: return nullptr;
    }

    return clients.empty()
        ? nullptr
        : *clients.rbegin();
}

bool
Model::client_matches_search(Client_ptr client, SearchSelector const& selector) const
{
    switch (selector.criterium()) {
    case SearchSelector::SelectionCriterium::OnWorkspaceBySelector:
    {
        // TODO
        break;
    }
    case SearchSelector::SelectionCriterium::ByNameEquals:
    {
        return client->managed
            && client->name == selector.string_value();
    }
    case SearchSelector::SelectionCriterium::ByClassEquals:
    {
        return client->managed
            && client->class_ == selector.string_value();
    }
    case SearchSelector::SelectionCriterium::ByInstanceEquals:
    {
        return client->managed
            && client->instance == selector.string_value();
    }
    case SearchSelector::SelectionCriterium::ByNameContains:
    {
        return client->managed
            && client->name.find(selector.string_value()) != std::string::npos;
    }
    case SearchSelector::SelectionCriterium::ByClassContains:
    {
        return client->managed
            && client->class_.find(selector.string_value()) != std::string::npos;
    }
    case SearchSelector::SelectionCriterium::ByInstanceContains:
    {
        return client->managed
            && client->instance.find(selector.string_value()) != std::string::npos;
    }
    case SearchSelector::SelectionCriterium::ForCondition:
    {
        return client->managed
            && selector.filter()(client);
    }
    default: return false;
    }

    return false;
}


Index
Model::active_partition() const
{
    return mp_partition->index();
}

Partition_ptr
Model::get_partition(Index index) const
{
    if (index < m_partitions.size())
        return m_partitions[index];

    return nullptr;
}


Index
Model::active_context() const
{
    return mp_context->index();
}

Context_ptr
Model::get_context(Index index) const
{
    if (index < m_contexts.size())
        return m_contexts[index];

    return nullptr;
}


Index
Model::active_workspace() const
{
    return mp_workspace->index();
}

Workspace_ptr
Model::get_workspace(Index index) const
{
    if (index < m_workspaces.size())
        return m_workspaces[index];

    return nullptr;
}


bool
Model::is_free(Client_ptr client) const
{
    return Client::is_free(client)
        || ((!client->fullscreen || client->contained)
            && (client->sticky
                    ? mp_workspace
                    : get_workspace(client->workspace)
               )->layout_is_free());
}

void
Model::place_client(Placement& placement)
{
    Client_ptr client = placement.client;

    if (!placement.region) {
        switch (placement.method) {
        case Placement::PlacementMethod::Free:
        {
            client->set_free_decoration(placement.decoration);
            break;
        }
        case Placement::PlacementMethod::Tile:
        {
            client->free_decoration = Decoration::FREE_DECORATION;
            client->set_tile_decoration(placement.decoration);
            break;
        }
        }

        unmap_client(client);
        return;
    }

    switch (placement.method) {
    case Placement::PlacementMethod::Free:
    {
        client->set_free_decoration(placement.decoration);
        client->set_free_region(*placement.region);
        break;
    }
    case Placement::PlacementMethod::Tile:
    {
        client->free_decoration = Decoration::FREE_DECORATION;
        client->set_tile_decoration(placement.decoration);
        client->set_tile_region(*placement.region);
        break;
    }
    }

    map_client(client);
    m_conn.place_window(client->window, client->inner_region);
    m_conn.place_window(client->frame, client->active_region);

    render_decoration(client);
    m_conn.update_window_offset(client->window, client->frame);
}

void
Model::map_client(Client_ptr client)
{
    if (!client->mapped) {
        client->mapped = true;
        m_conn.map_window(client->window);
        m_conn.map_window(client->frame);
        render_decoration(client);
    }
}

void
Model::unmap_client(Client_ptr client)
{
    if (client->mapped) {
        client->mapped = false;
        client->expect_unmap();
        m_conn.unmap_window(client->frame);
    }
}

void
Model::focus_client(Client_ptr client)
{
    if (!client->sticky) {
        activate_workspace(client->workspace);
        mp_workspace->activate_client(client);
    }

    unfocus_client(mp_focus);
    m_conn.ungrab_buttons(client->frame);

    client->focus();
    client->urgent = false;

    mp_focus = client;

    if (mp_workspace->layout_is_persistent() || mp_workspace->layout_is_single())
        apply_layout(mp_workspace);

    if (m_conn.get_focused_window() != client->window)
        m_conn.focus_window(client->window);

    render_decoration(client);
    apply_stack(mp_workspace);
}

void
Model::unfocus_client(Client_ptr client)
{
    if (!client)
        return;

    client->unfocus();
    m_conn.regrab_buttons(client->frame);
    render_decoration(client);
}

void
Model::sync_focus()
{
    Client_ptr active = mp_workspace->active();

    if (active && active != mp_focus)
        focus_client(active);
    else if (mp_workspace->empty()) {
        m_conn.unfocus();
        mp_focus = nullptr;
    }
}


void
Model::toggle_partition()
{
    if (mp_prev_partition)
        activate_partition(mp_prev_partition);
}

void
Model::activate_next_partition(winsys::Direction direction)
{
    activate_partition(m_partitions.next_index(direction));;
}

void
Model::activate_partition(Util::Change<Index> index)
{
    if (index >= m_partitions.size())
        return;

    activate_partition(get_partition(index));
}


void
Model::activate_partition(Partition_ptr next_partition)
{
    if (next_partition == mp_partition)
        return;

    stop_moving();
    stop_resizing();

    // TODO
}


void
Model::toggle_context()
{
    if (mp_prev_context)
        activate_context(mp_prev_context);
}

void
Model::activate_next_context(winsys::Direction direction)
{
    activate_context(m_contexts.next_index(direction));;
}

void
Model::activate_context(Util::Change<Index> index)
{
    if (index >= m_contexts.size())
        return;

    activate_context(get_context(index));
}

void
Model::activate_context(Context_ptr next_context)
{
    if (next_context == mp_context)
        return;

    stop_moving();
    stop_resizing();

    // TODO
}


void
Model::toggle_workspace()
{
    if (mp_prev_workspace)
        activate_workspace(mp_prev_workspace);
}

void
Model::activate_next_workspace(Direction direction)
{
    activate_workspace(m_workspaces.next_index(direction));
}

void
Model::activate_workspace(Util::Change<Index> index)
{
    if (index >= m_workspaces.size())
        return;

    activate_workspace(get_workspace(index));
}

void
Model::activate_workspace(Workspace_ptr next_workspace)
{
    if (next_workspace == mp_workspace)
        return;

    stop_moving();
    stop_resizing();

    m_conn.set_current_desktop(next_workspace->index());

    Workspace_ptr prev_workspace = mp_workspace;
    mp_prev_workspace = prev_workspace;

    for (auto& client : next_workspace->clients())
        if (!client->mapped)
            map_client(client);

    for (auto& client : mp_workspace->clients())
        if (client->mapped && !client->sticky)
            unmap_client(client);

    for (auto& client : m_sticky_clients)
        client->workspace = next_workspace->index();

    m_workspaces.activate_element(next_workspace);
    mp_workspace = next_workspace;

    apply_layout(next_workspace);
    apply_stack(next_workspace);

    sync_focus();
}

void
Model::render_decoration(Client_ptr client)
{
    Decoration& decoration = client->active_decoration;

    std::optional<std::size_t> border_width = std::nullopt;
    std::optional<Color> border_color = std::nullopt;
    std::optional<Color> frame_color = std::nullopt;

    switch (client->get_outside_state()) {
    case Client::OutsideState::Focused:
    {
        if (decoration.border) {
            border_width = decoration.border->width;
            border_color = decoration.border->colors.focused;
        }

        if (decoration.frame)
            frame_color = decoration.frame->colors.focused;

        break;
    }
    case Client::OutsideState::FocusedDisowned:
    {
        if (decoration.border) {
            border_width = decoration.border->width;
            border_color = decoration.border->colors.fdisowned;
        }

        if (decoration.frame)
            frame_color = decoration.frame->colors.fdisowned;

        break;
    }
    case Client::OutsideState::FocusedSticky:
    {
        if (decoration.border) {
            border_width = decoration.border->width;
            border_color = decoration.border->colors.fsticky;
        }

        if (decoration.frame)
            frame_color = decoration.frame->colors.fsticky;

        break;
    }
    case Client::OutsideState::Unfocused:
    {
        if (decoration.border) {
            border_width = decoration.border->width;
            border_color = decoration.border->colors.unfocused;
        }

        if (decoration.frame)
            frame_color = decoration.frame->colors.unfocused;

        break;
    }
    case Client::OutsideState::UnfocusedDisowned:
    {
        if (decoration.border) {
            border_width = decoration.border->width;
            border_color = decoration.border->colors.udisowned;
        }

        if (decoration.frame)
            frame_color = decoration.frame->colors.udisowned;

        break;
    }
    case Client::OutsideState::UnfocusedSticky:
    {
        if (decoration.border) {
            border_width = decoration.border->width;
            border_color = decoration.border->colors.usticky;
        }

        if (decoration.frame)
            frame_color = decoration.frame->colors.usticky;

        break;
    }
    case Client::OutsideState::Urgent:
    {
        if (decoration.border) {
            border_width = decoration.border->width;
            border_color = decoration.border->colors.urgent;
        }

        if (decoration.frame)
            frame_color = decoration.frame->colors.urgent;

        break;
    }
    }

    if (border_width) {
        m_conn.set_window_border_width(client->frame, *border_width);
        m_conn.set_window_border_color(client->frame, *border_color);
    }

    if (frame_color)
        m_conn.set_window_background_color(client->frame, *frame_color);
}


Rules
Model::retrieve_rules(Client_ptr client) const
{
    static std::string prefix = WM_NAME + ":";
    Rules rules = {};

    if (client->instance.size() <= prefix.size())
        return rules;

    auto res = std::mismatch(
        prefix.begin(),
        prefix.end(),
        client->instance.begin()
    );

    if (res.first == prefix.end()) {
        bool invert = false;
        bool next_partition = false;
        bool next_context = false;
        bool next_workspace = false;

        for (auto iter = res.second; iter != client->instance.end(); ++iter) {
            if (*iter == '!') {
                invert = true;
                continue;
            }

            if (*iter == 'P') {
                next_partition = true;
                continue;
            }

            if (*iter == 'C') {
                next_context = true;
                continue;
            }

            if (*iter == 'W') {
                next_workspace = true;
                continue;
            }

            if (*iter == 'f')
                rules.do_float = !invert;

            if (*iter == 'F')
                rules.do_fullscreen = !invert;

            if (*iter == 'c')
                rules.do_center = !invert;

            if (*iter >= '0' && *iter <= '9') {
                if (next_partition)
                    rules.to_partition = *iter - '0';

                if (next_context)
                    rules.to_context = *iter - '0';

                if (next_workspace)
                    rules.to_workspace = *iter - '0';
            }

            invert = false;
            next_partition = false;
            next_context = false;
            next_workspace = false;
        }
    }

    return rules;
}


void
Model::manage(const Window window, const bool ignore, const bool may_map)
{
    std::optional<Region> window_geometry = m_conn.get_window_geometry(window);

    if (ignore || !window_geometry) {
        if (may_map && m_conn.window_is_mappable(window))
            m_conn.map_window(window);

        m_conn.init_unmanaged(window);
        m_unmanaged_windows.push_back(window);

        return;
    }

    std::optional<Pid> pid = m_conn.get_window_pid(window);
    std::optional<Pid> ppid = m_conn.get_ppid(pid);

    while (ppid && m_pid_map.count(*ppid) == 0)
        ppid = m_conn.get_ppid(ppid);

    Client_ptr producer = nullptr;

    if (ppid) {
        std::optional<Client_ptr> ppid_client = Util::retrieve(m_pid_map, *ppid);

        if (ppid_client)
            producer = *ppid_client;
    }

    std::string name = m_conn.get_icccm_window_name(window);
    std::string class_ = m_conn.get_icccm_window_class(window);
    std::string instance = m_conn.get_icccm_window_instance(window);

    std::unordered_set<WindowType> types = m_conn.get_window_types(window);
    std::unordered_set<WindowState> states = m_conn.get_window_states(window);

    Region geometry = *window_geometry;

    Window frame = m_conn.create_frame(geometry);

    bool center = false;
    bool floating = m_conn.must_free_window(window);
    bool fullscreen = m_conn.window_is_fullscreen(window);
    bool sticky = m_conn.window_is_sticky(window);

    Index workspace = m_conn.get_window_desktop(window).value_or(mp_workspace->index());
    Index context = workspace / m_workspaces.size();
    workspace %= m_workspaces.size();

    std::optional<Hints> hints = m_conn.get_icccm_window_hints(window);
    std::optional<SizeHints> size_hints
        = m_conn.get_icccm_window_size_hints(window, std::nullopt);

    if (size_hints) {
        size_hints->apply(geometry.dim);
        center = !size_hints->by_user;
    } else {
        geometry.apply_minimum_dim(Client::MIN_CLIENT_DIM);
        center = true;
    }

    center &= Pos::is_at_origin(geometry.pos);

    Extents extents = Decoration::FREE_DECORATION.extents();
    geometry.apply_extents(extents);

    std::optional<Window> parent = m_conn.get_icccm_window_transient_for(window);
    std::optional<Window> leader = m_conn.get_icccm_window_client_leader(window);

    Client_ptr client = new Client(
        window,
        frame,
        name,
        class_,
        instance,
        mp_partition->index(),
        context,
        workspace,
        pid,
        ppid
    );

    Rules rules = retrieve_rules(client);

    if (rules.to_context && *rules.to_context <= m_contexts.size())
        client->context = *rules.to_context;

    if (rules.to_workspace && *rules.to_workspace <= m_workspaces.size())
        client->workspace = *rules.to_workspace;

    if (parent) {
        Client_ptr parent_client = get_client(*parent);

        if (parent_client) {
            client->parent = parent_client;
            parent_client->children.push_back(client);
            floating = true;
        }

        m_stack.add_above_other(frame,
            parent_client ? parent_client->frame : *parent);
    }

    if (leader) {
        Client_ptr leader_client = get_client(*leader);

        if (leader_client) {
            client->leader = get_client(*leader);
            floating = true;
        }
    }

    if (center || (rules.do_center && *rules.do_center)) {
        const Region screen_region = active_screen().placeable_region();

        geometry.pos.x = screen_region.pos.x
            + (screen_region.dim.w - geometry.dim.w) / 2;

        geometry.pos.y = screen_region.pos.y
            + (screen_region.dim.h - geometry.dim.h) / 2;
    }

    client->set_free_region(geometry);
    client->floating = floating;
    client->fullscreen = fullscreen;
    client->urgent = hints ? hints->urgent : false;
    client->partition = m_partitions.active_index();
    client->context = m_contexts.active_index();
    client->workspace = m_workspaces.active_index();
    client->size_hints = size_hints;

    if (rules.do_float)
        client->floating = *rules.do_float;

    if (rules.do_fullscreen)
        client->fullscreen = *rules.do_fullscreen;

    if (rules.to_partition)
        client->partition = *rules.to_partition;

    if (rules.to_context)
        client->context = *rules.to_context;

    if (rules.to_workspace)
        client->workspace = *rules.to_workspace;

    if (pid)
        m_pid_map[*pid] = client;

    m_conn.place_window(frame, client->free_region);
    m_conn.unmap_window(window);
    m_conn.unmap_window(frame);
    m_conn.reparent_window(window, frame, Pos { extents.left, extents.top });

    m_client_map[window] = client;
    m_client_map[frame] = client;

    m_conn.insert_window_in_save_set(window);
    m_conn.init_window(window, false);
    m_conn.init_frame(frame, false);
    m_conn.set_window_border_width(window, 0);
    m_conn.set_window_desktop(window, workspace);
    m_conn.set_icccm_window_state(window, IcccmWindowState::Normal);

    mp_workspace->add_client(client);

    if (client->size_hints)
        client->size_hints->apply(client->free_region.dim);

    client->free_region.apply_minimum_dim(Client::MIN_CLIENT_DIM);

    apply_layout(client->workspace);

    if (!m_move_buffer.is_occupied()
        && !m_resize_buffer.is_occupied()
    ) {
        focus_client(client);
    }

    if (Util::contains(states, WindowState::DemandsAttention))
        handle_state_request({
            window,
            WindowState::DemandsAttention,
            Toggle::On,
            false
        });

    if (sticky)
        set_sticky_client(Toggle::On, client);

    if (fullscreen)
        set_fullscreen_client(Toggle::On, client);

    if (producer && producer->producing)
        consume_client(producer, client);
}

void
Model::unmanage(Client_ptr client)
{
    if (client->consume_unmap_if_expecting())
        return;

    for (Client_ptr consumer : client->consumers)
        check_unconsume_client(consumer);

    check_unconsume_client(client);

    set_sticky_client(winsys::Toggle::Off, client);

    Workspace_ptr workspace = get_workspace(client->workspace);

    m_conn.unparent_window(client->window, client->active_region.pos);

    m_conn.cleanup_window(client->window);
    m_conn.destroy_window(client->frame);

    workspace->remove_client(client);
    workspace->remove_icon(client);
    workspace->remove_disowned(client);

    if (client->pid)
        m_pid_map.erase(*client->pid);

    if (client->producer)
        Util::erase_remove(client->producer->consumers, client);

    if (client->parent)
        Util::erase_remove(client->parent->children, client);

    m_client_map.erase(client->window);
    m_client_map.erase(client->frame);

    m_fullscreen_map.erase(client);

    Util::erase_remove(m_sticky_clients, client);
    Util::erase_remove(m_order, client->frame);

    m_stack.remove_window(client->frame);

    if (client == mp_jumped_from)
        mp_jumped_from = nullptr;

    sync_focus();
    apply_layout(workspace);
}

void
Model::start_moving(Client_ptr client)
{
    if (m_move_buffer.is_occupied() || m_resize_buffer.is_occupied())
        return;

    m_move_buffer.set(
        client,
        Grip::Top | Grip::Left,
        m_conn.get_pointer_position(),
        client->free_region
    );

    m_conn.init_move(client->frame);
}

void
Model::start_resizing(Client_ptr client)
{
    if (m_move_buffer.is_occupied() || m_resize_buffer.is_occupied())
        return;

    Region region = client->free_region;
    Pos pos = m_conn.get_pointer_position();
    Pos center = Pos {
        region.pos.x + static_cast<int>(static_cast<float>(region.dim.w) / 2.f),
        region.pos.y + static_cast<int>(static_cast<double>(region.dim.h) / 2.f)
    };

    Grip grip = static_cast<Grip>(0);

    if (pos.x >= center.x)
        grip |= Grip::Right;
    else
        grip |= Grip::Left;

    if (pos.y >= center.y)
        grip |= Grip::Bottom;
    else
        grip |= Grip::Top;

    m_resize_buffer.set(
        client,
        grip,
        pos,
        region
    );

    m_conn.init_resize(client->frame);
}

void
Model::stop_moving()
{
    if (m_move_buffer.is_occupied()) {
        m_conn.release_pointer();
        m_move_buffer.unset();
    }
}

void
Model::stop_resizing()
{
    if (m_resize_buffer.is_occupied()) {
        m_conn.release_pointer();
        m_resize_buffer.unset();
    }
}


void
Model::perform_move(Pos& pos)
{
    if (!m_move_buffer.is_occupied())
        return;

    Client_ptr client = m_move_buffer.client();

    if (!client || !is_free(client)) {
        stop_moving();
        return;
    }

    Region client_region = *m_move_buffer.client_region();
    Pos grip_pos = *m_move_buffer.grip_pos();

    Region region = Region {
        Pos {
            client_region.pos.x + pos.x - grip_pos.x,
            client_region.pos.y + pos.y - grip_pos.y,
        },
        client->free_region.dim,
    };

    client->set_free_region(region);

    Placement placement = Placement {
        Placement::PlacementMethod::Free,
        client,
        client->free_decoration,
        region
    };

    place_client(placement);
}

void
Model::perform_resize(Pos& pos)
{
    if (!m_resize_buffer.is_occupied())
        return;

    Client_ptr client = m_resize_buffer.client();

    if (!client || !is_free(client)) {
        stop_resizing();
        return;
    }

    Region client_region = *m_resize_buffer.client_region();
    Pos grip_pos = *m_resize_buffer.grip_pos();
    Grip grip = *m_resize_buffer.grip();

    Region region = client->free_region;

    Decoration decoration = client->free_decoration;
    Extents extents = Extents { 0, 0, 0, 0 };

    if (decoration.border) {
        extents.left   += decoration.border->width;
        extents.top    += decoration.border->width;
        extents.right  += decoration.border->width;
        extents.bottom += decoration.border->width;
    }

    if (decoration.frame) {
        extents.left   += decoration.frame->extents.left;
        extents.top    += decoration.frame->extents.top;
        extents.right  += decoration.frame->extents.right;
        extents.bottom += decoration.frame->extents.bottom;
    }

    region.remove_extents(extents);

    int dx = pos.x - grip_pos.x;
    int dy = pos.y - grip_pos.y;

    int dest_w;
    int dest_h;

    if ((grip & Grip::Left) != 0)
        dest_w = client_region.dim.w - dx;
    else
        dest_w = client_region.dim.w + dx;

    if ((grip & Grip::Top) != 0)
        dest_h = client_region.dim.h - dy;
    else
        dest_h = client_region.dim.h + dy;

    region.dim.w = std::max(0, dest_w);
    region.dim.h = std::max(0, dest_h);

    if (client->size_hints)
        client->size_hints->apply(region.dim);

    region.apply_extents(extents);

    if ((grip & Grip::Top) != 0)
        region.pos.y
            = client_region.pos.y + (client_region.dim.h - region.dim.h);

    if ((grip & Grip::Left) != 0)
        region.pos.x
            = client_region.pos.x + (client_region.dim.w - region.dim.w);

    if (region == client->previous_region)
        return;

    Placement placement = Placement {
        Placement::PlacementMethod::Free,
        client,
        client->free_decoration,
        region
    };

    place_client(placement);
}


void
Model::apply_layout(Index index)
{
    if (index < m_workspaces.size())
        apply_layout(m_workspaces[index]);
}

void
Model::apply_layout(Workspace_ptr workspace)
{
    if (workspace != mp_workspace)
        return;

    for (auto& placement : workspace->arrange(active_screen().placeable_region()))
        place_client(placement);
}


void
Model::apply_stack(Index index)
{
    if (index < m_workspaces.size())
        apply_stack(m_workspaces[index]);
}

void
Model::apply_stack(Workspace_ptr workspace)
{
    static std::vector<Window> stack;

    if (workspace != mp_workspace)
        return;

    std::vector<Client_ptr> clients = workspace->stack_after_focus();

    auto fullscreen_iter = std::stable_partition(
        clients.begin(),
        clients.end(),
        [](Client_ptr client) -> bool {
            return client->fullscreen && !client->contained;
        }
    );

    auto free_iter = std::stable_partition(
        fullscreen_iter,
        clients.end(),
        [=,this](Client_ptr client) -> bool {
            return is_free(client);
        }
    );

    stack.reserve(3 * clients.size());
    stack.clear();

    Util::append(stack, m_stack.get_layer(StackHandler::StackLayer::Desktop));
    Util::append(stack, m_stack.get_layer(StackHandler::StackLayer::Below_));
    Util::append(stack, m_stack.get_layer(StackHandler::StackLayer::Dock));

    std::transform(
        free_iter,
        clients.end(),
        std::back_inserter(stack),
        [](Client_ptr client) -> Window {
            return client->frame;
        }
    );

    std::transform(
        clients.begin(),
        fullscreen_iter,
        std::back_inserter(stack),
        [](Client_ptr client) -> Window {
            return client->frame;
        }
    );

    std::transform(
        fullscreen_iter,
        free_iter,
        std::back_inserter(stack),
        [](Client_ptr client) -> Window {
            return client->frame;
        }
    );

    Util::append(stack, m_stack.get_layer(StackHandler::StackLayer::Above_));
    Util::append(stack, m_stack.get_layer(StackHandler::StackLayer::Notification));

    bool order_changed = false;
    std::optional<Window> prev_window
        = stack.empty() ? std::nullopt : std::optional(stack[0]);

    for (std::size_t i = 1; i < stack.size(); ++i) {
        if (!order_changed) {
            if (i < m_order.size())
                order_changed |= m_order[i] != stack[i];
            else
                order_changed = true;
        }

        if (order_changed)
            m_conn.stack_window_above(stack[i], prev_window);

        prev_window = stack[i];
    }

    if (!order_changed)
        return;

    m_order = stack;

    static constexpr struct ManagedSinceComparer final {
        bool
        operator()(const Client_ptr lhs, const Client_ptr rhs) const
        {
            return lhs->managed_since < rhs->managed_since;
        }
    } managed_since_comparer = {};

    static std::set<Client_ptr, ManagedSinceComparer> managed_since_clients
        = {{}, managed_since_comparer};

    managed_since_clients.clear();

    std::for_each(
        m_client_map.begin(),
        m_client_map.end(),
        [](auto kv) {
            managed_since_clients.insert(kv.second);
        }
    );

    static std::vector<Window> order_list;
    order_list.reserve(managed_since_clients.size());
    order_list.clear();

    std::transform(
        managed_since_clients.begin(),
        managed_since_clients.end(),
        std::back_inserter(order_list),
        [](Client_ptr client) -> Window {
            return client->window;
        }
    );

    m_conn.update_client_list(order_list);

    static constexpr struct LastFocusedComparer final {
        bool
        operator()(const Client_ptr lhs, const Client_ptr rhs) const
        {
            return lhs->last_focused < rhs->last_focused;
        }
    } last_focused_comparer = {};

    static std::set<Client_ptr, LastFocusedComparer> last_focused_clients
        = {{}, last_focused_comparer};

    last_focused_clients.clear();

    std::for_each(
        m_client_map.begin(),
        m_client_map.end(),
        [](auto kv) {
            last_focused_clients.insert(kv.second);
        }
    );

    order_list.reserve(last_focused_clients.size());
    order_list.clear();

    std::transform(
        last_focused_clients.begin(),
        last_focused_clients.end(),
        std::back_inserter(order_list),
        [](Client_ptr client) -> Window {
            return client->window;
        }
    );

    m_conn.update_client_list_stacking(order_list);
}


void
Model::cycle_focus(Direction direction)
{
    if (mp_workspace->size() <= 1)
        return;

    mp_workspace->cycle(direction);
    focus_client(mp_workspace->active());
    sync_focus();
}

void
Model::drag_focus(Direction direction)
{
    if (mp_workspace->size() <= 1)
        return;

    mp_workspace->drag(direction);
    focus_client(mp_workspace->active());
    sync_focus();

    apply_layout(mp_workspace);
}


void
Model::rotate_clients(Direction direction)
{
    if (mp_workspace->size() <= 1)
        return;

    mp_workspace->rotate(direction);
    focus_client(mp_workspace->active());
    sync_focus();

    apply_layout(mp_workspace);
}


void
Model::move_focus_to_next_workspace(Direction direction)
{
    if (mp_focus)
        move_client_to_next_workspace(direction, mp_focus);
}

void
Model::move_client_to_next_workspace(Direction direction, Client_ptr client)
{
    Index index = mp_workspace->index();

    switch (direction) {
    case Direction::Forward:
    {
        index = (index + 1) % m_workspaces.size();
        break;
    }
    case Direction::Backward:
    {
        index = (index == 0 ? m_workspaces.size() : index) - 1;
        break;
    }
    default: return;
    }

    move_client_to_workspace(index, client);
}

void
Model::move_focus_to_workspace(Index index)
{
    if (mp_focus)
        move_client_to_workspace(index, mp_focus);
}

void
Model::move_client_to_workspace(Index index, Client_ptr client)
{
    if (index >= m_workspaces.size() || index == mp_workspace->index() || client->sticky)
        return;

    client->workspace = index;
    Workspace_ptr workspace = get_workspace(index);

    unmap_client(client);

    workspace->add_client(client);
    apply_layout(workspace);
    apply_stack(workspace);

    mp_workspace->remove_client(client);
    apply_layout(mp_workspace);

    sync_focus();
}


void
Model::toggle_layout()
{
    mp_workspace->toggle_layout();
    apply_layout(mp_workspace);
    apply_stack(mp_workspace);
}

void
Model::set_layout(LayoutHandler::LayoutKind layout)
{
    mp_workspace->set_layout(layout);
    apply_layout(mp_workspace);
    apply_stack(mp_workspace);
}

void
Model::set_layout_retain_region(LayoutHandler::LayoutKind layout)
{
    const std::deque<Client_ptr>& clients = mp_workspace->clients();
    std::vector<Region> regions;

    bool was_tiled = !mp_workspace->layout_is_free();

    if (was_tiled) {
        regions.reserve(clients.size());

        std::transform(
            clients.begin(),
            clients.end(),
            std::back_inserter(regions),
            [=,this](Client_ptr client) -> Region {
                if (is_free(client))
                    return client->free_region;
                else
                    return client->tile_region;
            }
        );
    }

    mp_workspace->set_layout(layout);

    if (was_tiled && mp_workspace->layout_is_free())
        for (std::size_t i = 0; i < clients.size(); ++i)
            clients[i]->set_free_region(regions[i]);

    apply_layout(mp_workspace);
    apply_stack(mp_workspace);
}


void
Model::toggle_layout_data()
{
    mp_workspace->toggle_layout_data();
    apply_layout(mp_workspace);
}

void
Model::cycle_layout_data(Direction direction)
{
    mp_workspace->cycle_layout_data(direction);
    apply_layout(mp_workspace);
}

void
Model::copy_data_from_prev_layout()
{
    mp_workspace->copy_data_from_prev_layout();
    apply_layout(mp_workspace);
}


void
Model::change_gap_size(Util::Change<int> change)
{
    mp_workspace->change_gap_size(change);
    apply_layout(mp_workspace);
}

void
Model::change_main_count(Util::Change<int> change)
{
    mp_workspace->change_main_count(change);
    apply_layout(mp_workspace);
}

void
Model::change_main_factor(Util::Change<float> change)
{
    mp_workspace->change_main_factor(change);
    apply_layout(mp_workspace);
}

void
Model::change_margin(Util::Change<int> change)
{
    mp_workspace->change_margin(change);
    apply_layout(mp_workspace);
}

void
Model::change_margin(Edge edge, Util::Change<int> change)
{
    mp_workspace->change_margin(edge, change);
    apply_layout(mp_workspace);
}

void
Model::reset_gap_size()
{
    mp_workspace->reset_gap_size();
    apply_layout(mp_workspace);
}

void
Model::reset_margin()
{
    mp_workspace->reset_margin();
    apply_layout(mp_workspace);
}

void
Model::reset_layout_data()
{
    mp_workspace->reset_layout_data();
    apply_layout(mp_workspace);
}


void
Model::save_layout(std::size_t number) const
{
    mp_workspace->save_layout(number);
}

void
Model::load_layout(std::size_t number)
{
    mp_workspace->load_layout(number);
    apply_layout(mp_workspace);
    apply_stack(mp_workspace);
}


void
Model::kill_focus()
{
    if (mp_focus)
        kill_client(mp_focus);
}

void
Model::kill_client(Client_ptr client)
{
    if (!client->invincible) {
        m_conn.unmap_window(client->frame);
        m_conn.kill_window(client->window);
        m_conn.flush();
    }
}


void
Model::jump_client(SearchSelector const& selector)
{
    Client_ptr client = search_client(selector);

    if (client) {
        if (client == mp_focus) {
            if (mp_jumped_from && client != mp_jumped_from)
                client = mp_jumped_from;
        }

        if (mp_focus)
            mp_jumped_from = mp_focus;

        focus_client(client);
    }
}


void
Model::set_floating_focus(Toggle toggle)
{
    if (mp_focus)
        set_floating_client(toggle, mp_focus);
}

void
Model::set_floating_client(Toggle toggle, Client_ptr client)
{
    switch (toggle) {
    case Toggle::On:      client->floating = true;              break;
    case Toggle::Off:     client->floating = false;             break;
    case Toggle::Reverse: client->floating = !client->floating; break;
    default: return;
    }

    apply_layout(client->workspace);
    apply_stack(client->workspace);
}

void
Model::set_fullscreen_focus(Toggle toggle)
{
    if (mp_focus)
        set_fullscreen_client(toggle, mp_focus);
}

void
Model::set_fullscreen_client(Toggle toggle, Client_ptr client)
{
    switch (toggle) {
    case Toggle::On:
    {
        if (client->fullscreen)
            return;

        client->fullscreen = true;

        m_conn.set_window_state(
            client->window,
            WindowState::Fullscreen,
            true
        );

        Workspace_ptr workspace = get_workspace(client->workspace);

        apply_layout(workspace);
        apply_stack(workspace);
        render_decoration(client);

        m_fullscreen_map[client] = client->free_region;

        return;
    }
    case Toggle::Off:
    {
        if (!client->fullscreen)
            return;

        if (!client->contained)
            client->set_free_region(m_fullscreen_map.at(client));

        client->fullscreen = false;

        m_conn.set_window_state(
            client->window,
            WindowState::Fullscreen,
            false
        );

        Workspace_ptr workspace = get_workspace(client->workspace);

        apply_layout(workspace);
        apply_stack(workspace);
        render_decoration(client);

        m_fullscreen_map.erase(client);

        return;
    }
    case Toggle::Reverse:
    {
        set_fullscreen_client(
            client->fullscreen
            ? Toggle::Off
            : Toggle::On,
            client
        );

        return;
    }
    default: return;
    }
}

void
Model::set_sticky_focus(Toggle toggle)
{
    if (mp_focus)
        set_sticky_client(toggle, mp_focus);
}

void
Model::set_sticky_client(Toggle toggle, Client_ptr client)
{
    switch (toggle) {
    case Toggle::On:
    {
        if (client->sticky)
            return;

        std::for_each(
            m_workspaces.begin(),
            m_workspaces.end(),
            [client](Workspace_ptr workspace) {
                if (workspace->index() != client->workspace)
                    workspace->add_client(client);
            }
        );

        m_conn.set_window_state(
            client->window,
            WindowState::Sticky,
            true
        );

        Workspace_ptr workspace = get_workspace(client->workspace);

        client->stick();

        apply_layout(workspace);
        render_decoration(client);

        return;
    }
    case Toggle::Off:
    {
        if (!client->sticky)
            return;

        std::for_each(
            m_workspaces.begin(),
            m_workspaces.end(),
            [=,this](Workspace_ptr workspace) {
                if (workspace->index() != mp_workspace->index()) {
                    workspace->remove_client(client);
                    workspace->remove_icon(client);
                    workspace->remove_disowned(client);
                } else {
                    client->workspace = workspace->index();
                }
            }
        );

        m_conn.set_window_state(
            client->window,
            WindowState::Sticky,
            false
        );

        client->unstick();

        apply_layout(mp_workspace);
        render_decoration(client);

        return;
    }
    case Toggle::Reverse:
    {
        set_sticky_client(
            client->sticky
            ? Toggle::Off
            : Toggle::On,
            client
        );

        return;
    }
    default: return;
    }
}

void
Model::set_contained_focus(Toggle toggle)
{
    if (mp_focus)
        set_contained_client(toggle, mp_focus);
}

void
Model::set_contained_client(Toggle toggle, Client_ptr client)
{
    switch (toggle) {
    case Toggle::On:
    {
        client->contained = true;

        Workspace_ptr workspace = get_workspace(client->workspace);

        apply_layout(workspace);
        apply_stack(workspace);

        return;
    }
    case Toggle::Off:
    {
        client->contained = false;

        Workspace_ptr workspace = get_workspace(client->workspace);

        apply_layout(workspace);
        apply_stack(workspace);

        return;
    }
    case Toggle::Reverse:
    {
        set_contained_client(
            client->contained
            ? Toggle::Off
            : Toggle::On,
            client
        );

        return;
    }
    default: return;
    }
}

void
Model::set_invincible_focus(Toggle toggle)
{
    if (mp_focus)
        set_invincible_client(toggle, mp_focus);
}

void
Model::set_invincible_client(Toggle toggle, Client_ptr client)
{
    if (toggle == Toggle::Reverse)
        set_invincible_client(
            client->invincible
            ? Toggle::Off
            : Toggle::On,
            client
        );
    else
        client->invincible
            = toggle == Toggle::On ? true : false;
}

void
Model::set_producing_focus(Toggle toggle)
{
    if (mp_focus)
        set_producing_client(toggle, mp_focus);
}

void
Model::set_producing_client(Toggle toggle, Client_ptr client)
{
    if (toggle == Toggle::Reverse)
        set_producing_client(
            client->producing
            ? Toggle::Off
            : Toggle::On,
            client
        );
    else
        client->producing
            = toggle == Toggle::On ? true : false;
}

void
Model::set_iconifyable_focus(Toggle toggle)
{
    if (mp_focus)
        set_iconifyable_client(toggle, mp_focus);
}

void
Model::set_iconifyable_client(Toggle toggle, Client_ptr client)
{
    if (toggle == Toggle::Reverse)
        set_iconifyable_client(
            client->iconifyable
            ? Toggle::Off
            : Toggle::On,
            client
        );
    else
        client->iconifyable
            = toggle == Toggle::On ? true : false;
}

void
Model::set_iconify_focus(Toggle toggle)
{
    if (mp_focus)
        set_iconify_client(toggle, mp_focus);
}

void
Model::set_iconify_client(Toggle toggle, Client_ptr client)
{
    switch (toggle) {
    case Toggle::On:
    {
        if (client->iconified)
            return;

        Workspace_ptr workspace = get_workspace(client->workspace);
        workspace->client_to_icon(client);

        m_conn.set_icccm_window_state(
            client->window,
            IcccmWindowState::Iconic
        );

        unmap_client(client);

        apply_layout(workspace);
        sync_focus();

        client->iconified = true;

        return;
    }
    case Toggle::Off:
    {
        if (!client->iconified)
            return;

        Workspace_ptr workspace = get_workspace(client->workspace);
        workspace->icon_to_client(client);

        m_conn.set_icccm_window_state(
            client->window,
            IcccmWindowState::Normal
        );

        client->iconified = false;

        apply_layout(workspace);
        sync_focus();

        return;
    }
    case Toggle::Reverse:
    {
        set_iconify_client(
            client->iconified
            ? Toggle::Off
            : Toggle::On,
            client
        );

        return;
    }
    default: return;
    }
}


void
Model::consume_client(Client_ptr producer, Client_ptr client)
{
    static std::unordered_set<std::string> ignored_producers_memoized = {};
    static std::unordered_set<std::string> ignored_consumers_memoized = {};
    static std::unordered_set<std::string> allowed_producers_memoized = {};
    static std::unordered_set<std::string> allowed_consumers_memoized = {};

    Workspace_ptr pworkspace = get_workspace(producer->workspace);
    Workspace_ptr cworkspace = get_workspace(client->workspace);

    if (client->producer
        || !cworkspace->contains(client)
        || !pworkspace->contains(producer)
    ) {
        return;
    }

    std::string producer_handle = producer->name
        + ":" + producer->class_
        + ":" + producer->instance;

    std::string consumer_handle = client->name
        + ":" + client->class_
        + ":" + client->instance;

    if (!Util::contains(allowed_producers_memoized, producer_handle)) {
        if (Util::contains(ignored_producers_memoized, producer_handle))
            return;

        for (SearchSelector_ptr selector : m_ignored_producers)
            if (client_matches_search(producer, *selector)) {
                ignored_producers_memoized.insert(producer_handle);
                return;
            }
                allowed_producers_memoized.insert(producer_handle);
    }

    if (!Util::contains(allowed_consumers_memoized, producer_handle)) {
        if (Util::contains(ignored_consumers_memoized, consumer_handle))
            return;

        for (SearchSelector_ptr selector : m_ignored_consumers)
            if (client_matches_search(client, *selector)) {
                ignored_consumers_memoized.insert(consumer_handle);
                return;
            } else
                allowed_consumers_memoized.insert(consumer_handle);
    }

    if (m_move_buffer.client() == producer)
        stop_moving();

    if (m_resize_buffer.client() == producer)
        stop_resizing();

    unmap_client(producer);

    if (producer->consumers.size() == 0) {
        if (pworkspace == cworkspace) {
            cworkspace->remove_client(client);
            pworkspace->replace_client(producer, client);
        } else
            pworkspace->remove_client(producer);

        apply_layout(pworkspace);
        apply_layout(cworkspace);
    }

    client->producer = producer;
    producer->managed = false;
    producer->consumers.push_back(client);

    sync_focus();
    apply_stack(pworkspace);
    apply_stack(cworkspace);
}

void
Model::check_unconsume_client(Client_ptr client)
{
    Client_ptr producer = client->producer;

    if (!producer || producer->managed)
        return;

    Workspace_ptr cworkspace = get_workspace(client->workspace);

    client->producer = nullptr;
    producer->managed = true;
    Util::erase_remove(producer->consumers, client);

    if (producer->consumers.size() == 0) {
        if (cworkspace->contains(client)) {
            cworkspace->replace_client(client, producer);
            producer->workspace = cworkspace->index();
        } else {
            mp_workspace->add_client(producer);
            producer->workspace = mp_workspace->index();
        }

        apply_layout(mp_workspace);
        apply_layout(cworkspace);
    }

    sync_focus();
    apply_stack(mp_workspace);
    apply_stack(cworkspace);
}


void
Model::center_focus()
{
    if (mp_focus)
        center_client(mp_focus);
}

void
Model::center_client(Client_ptr client)
{
    if (!is_free(client))
        return;

    const Region screen_region = active_screen().placeable_region();
    Region region = client->free_region;

    region.pos.x = screen_region.pos.x
        + (screen_region.dim.w - region.dim.w) / 2;

    region.pos.y = screen_region.pos.y
        + (screen_region.dim.h - region.dim.h) / 2;

    Placement placement = Placement {
        Placement::PlacementMethod::Free,
        client,
        client->free_decoration,
        region
    };

    place_client(placement);
}

void
Model::nudge_focus(Edge edge, Util::Change<std::size_t> change)
{
    if (mp_focus)
        nudge_client(edge, change, mp_focus);
}

void
Model::nudge_client(Edge edge, Util::Change<std::size_t> change, Client_ptr client)
{
    if (!is_free(client))
        return;

    Region region = client->free_region;

    switch (edge) {
    case Edge::Left:
    {
        region.pos.x -= change;
        break;
    }
    case Edge::Top:
    {
        region.pos.y -= change;
        break;
    }
    case Edge::Right:
    {
        region.pos.x += change;
        break;
    }
    case Edge::Bottom:
    {
        region.pos.y += change;
        break;
    }
    default: return;
    }

    Placement placement = Placement {
        Placement::PlacementMethod::Free,
        client,
        client->free_decoration,
        region
    };

    place_client(placement);
}

void
Model::stretch_focus(Edge edge, Util::Change<int> change)
{
    if (mp_focus)
        stretch_client(edge, change, mp_focus);
}

void
Model::stretch_client(Edge edge, Util::Change<int> change, Client_ptr client)
{
    if (!is_free(client))
        return;

    Decoration decoration = client->free_decoration;
    Extents extents = Extents { 0, 0, 0, 0 };

    if (decoration.border) {
        extents.left   += decoration.border->width;
        extents.top    += decoration.border->width;
        extents.right  += decoration.border->width;
        extents.bottom += decoration.border->width;
    }

    if (decoration.frame) {
        extents.left   += decoration.frame->extents.left;
        extents.top    += decoration.frame->extents.top;
        extents.right  += decoration.frame->extents.right;
        extents.bottom += decoration.frame->extents.bottom;
    }

    Region region = client->free_region;
    region.remove_extents(extents);

    switch (edge) {
    case Edge::Left:
    {
        if (!(change < 0 && -change >= region.dim.h)) {
            if (region.dim.w + change <= Client::MIN_CLIENT_DIM.w) {
                region.pos.x -= Client::MIN_CLIENT_DIM.w - region.dim.w;
                region.dim.w = Client::MIN_CLIENT_DIM.w;
            } else {
                region.pos.x -= change;
                region.dim.w += change;
            }
        }

        break;
    }
    case Edge::Top:
    {
        if (!(change < 0 && -change >= region.dim.h)) {
            if (region.dim.h + change <= Client::MIN_CLIENT_DIM.h) {
                region.pos.y -= Client::MIN_CLIENT_DIM.h - region.dim.h;
                region.dim.h = Client::MIN_CLIENT_DIM.h;
            } else {
                region.pos.y -= change;
                region.dim.h += change;
            }
        }

        break;
    }
    case Edge::Right:
    {
        if (!(change < 0 && -change >= region.dim.w)) {
            if (region.dim.w + change <= Client::MIN_CLIENT_DIM.w)
                region.dim.w = Client::MIN_CLIENT_DIM.w;
            else
                region.dim.w += change;
        }

        break;
    }
    case Edge::Bottom:
    {
        if (!(change < 0 && -change >= region.dim.h)) {
            if (region.dim.h + change <= Client::MIN_CLIENT_DIM.h)
                region.dim.h = Client::MIN_CLIENT_DIM.h;
            else
                region.dim.h += change;
        }

        break;
    }
    default: return;
    }

    region.apply_extents(extents);

    Placement placement = Placement {
        Placement::PlacementMethod::Free,
        client,
        client->free_decoration,
        region
    };

    place_client(placement);
}

void
Model::inflate_focus(Util::Change<int> change)
{
    if (mp_focus)
        inflate_client(change, mp_focus);
}

void
Model::inflate_client(Util::Change<int> change, Client_ptr client)
{
    if (!is_free(client))
        return;

    Decoration decoration = client->free_decoration;
    Extents extents = Extents { 0, 0, 0, 0 };

    if (decoration.border) {
        extents.left   += decoration.border->width;
        extents.top    += decoration.border->width;
        extents.right  += decoration.border->width;
        extents.bottom += decoration.border->width;
    }

    if (decoration.frame) {
        extents.left   += decoration.frame->extents.left;
        extents.top    += decoration.frame->extents.top;
        extents.right  += decoration.frame->extents.right;
        extents.bottom += decoration.frame->extents.bottom;
    }

    Region region = client->free_region;
    region.remove_extents(extents);

    double ratio = static_cast<double>(region.dim.w)
        / static_cast<double>(region.dim.w + region.dim.h);

    double width_inc = ratio * change;
    double height_inc = change - width_inc;

    int dw = std::lround(width_inc);
    int dh = std::lround(height_inc);

    if ((dw < 0 && -dw >= region.dim.w)
        || (dh < 0 && -dh >= region.dim.h)
        || (region.dim.w + dw <= Client::MIN_CLIENT_DIM.w)
        || (region.dim.h + dh <= Client::MIN_CLIENT_DIM.h))
    {
        return;
    }

    region.dim.w += dw;
    region.dim.h += dh;

    region.apply_extents(extents);

    int dx = region.dim.w - client->free_region.dim.w;
    int dy = region.dim.h - client->free_region.dim.h;

    dx = std::lround(dx / static_cast<double>(2));
    dy = std::lround(dy / static_cast<double>(2));

    region.pos.x -= dx;
    region.pos.y -= dy;

    client->set_free_region(region);

    Placement placement = Placement {
        Placement::PlacementMethod::Free,
        client,
        client->free_decoration,
        region
    };

    place_client(placement);
}

void
Model::snap_focus(Edge edge)
{
    if (mp_focus)
        snap_client(edge, mp_focus);
}

void
Model::snap_client(Edge edge, Client_ptr client)
{
    if (!is_free(client))
        return;

    const Region screen_region = active_screen().placeable_region();
    Region region = client->free_region;

    switch (edge) {
    case Edge::Left:
    {
        region.pos.x = screen_region.pos.x;

        break;
    }
    case Edge::Top:
    {
        region.pos.y = screen_region.pos.y;

        break;
    }
    case Edge::Right:
    {
        region.pos.x = std::max(
            0,
            (screen_region.dim.w + screen_region.pos.x) - region.dim.w
        );

        break;
    }
    case Edge::Bottom:
    {
        region.pos.y = std::max(
            0,
            (screen_region.dim.h + screen_region.pos.y) - region.dim.h
        );

        break;
    }
    }

    Placement placement = Placement {
        Placement::PlacementMethod::Free,
        client,
        client->free_decoration,
        region
    };

    place_client(placement);
}


void
Model::activate_screen_struts(winsys::Toggle toggle)
{
    Screen& screen = mp_partition->screen();

    switch (toggle) {
    case Toggle::On:
    {
        if (screen.showing_struts())
            return;

        for (auto& strut : screen.show_and_get_struts(true))
            m_conn.map_window(strut);

        apply_layout(mp_workspace);

        return;
    }
    case Toggle::Off:
    {
        if (!screen.showing_struts())
            return;

        for (auto& strut : screen.show_and_get_struts(false))
            m_conn.unmap_window(strut);

        apply_layout(mp_workspace);

        return;
    }
    case Toggle::Reverse:
    {
        activate_screen_struts(
            screen.showing_struts()
            ? Toggle::Off
            : Toggle::On
        );

        return;
    }
    default: return;
    }
}


void
Model::pop_deiconify()
{
    std::optional<Client_ptr> icon = mp_workspace->pop_icon();

    if (icon)
        set_iconify_client(Toggle::Off, *icon);
}

void
Model::deiconify_all()
{
    for (std::size_t i = 0; i < mp_workspace->size(); ++i)
        pop_deiconify();
}


void
Model::spawn_external(std::string&& command) const
{
    spdlog::info("calling external command: " + command);
    m_conn.call_external_command(command);
}


void
Model::exit()
{
    for (auto& [window,client] : m_client_map)
        m_conn.unparent_window(client->window, client->free_region.pos);

    m_conn.cleanup();
    m_running = false;

    spdlog::info("exiting " + WM_NAME);
}


// handlers

void
Model::handle_mouse(MouseEvent event)
{
    MouseInput input = event.capture.input;

    switch (event.capture.kind) {
    case MouseCapture::MouseCaptureKind::Release:
    {
        stop_moving();
        stop_resizing();

        return;
    }
    case MouseCapture::MouseCaptureKind::Motion:
    {
        perform_move(event.capture.root_rpos);
        perform_resize(event.capture.root_rpos);

        return;
    }
    default: break;
    }

    Client_ptr client
        = event.capture.window
        ? get_client(*event.capture.window)
        : nullptr;

#define CALL_MUST_FOCUS(binding) \
    ((*binding)(*this, client) && client && client != mp_focus)
    { // global binding
        input.target = MouseInput::MouseInputTarget::Global;
        auto binding = Util::retrieve(m_mouse_bindings, input);

        if (binding) {
            if (CALL_MUST_FOCUS(binding))
                focus_client(client);

            return;
        }
    }

    if (event.on_root) { // root binding
        input.target = MouseInput::MouseInputTarget::Root;
        auto binding = Util::retrieve(m_mouse_bindings, input);

        if (binding) {
            if (CALL_MUST_FOCUS(binding))
                focus_client(client);

            return;
        }
    }

    { // client binding
        input.target = MouseInput::MouseInputTarget::Client;
        auto binding = Util::retrieve(m_mouse_bindings, input);

        if (binding) {
            if (CALL_MUST_FOCUS(binding))
                focus_client(client);

            return;
        }
    }
#undef CALL_MUST_FOCUS

    if (client && client != mp_focus)
        focus_client(client);
}

void
Model::handle_key(KeyEvent event)
{
    auto binding = Util::retrieve(m_key_bindings, event.capture.input);

    if (binding)
        (*binding)(*this);
}

void
Model::handle_map_request(MapRequestEvent event)
{
    bool must_restack = false;
    bool may_map = true;

    std::optional<std::vector<std::optional<Strut>>> struts
        = m_conn.get_window_strut(event.window);

    if (struts) {
        Screen& screen = mp_partition->screen();
        screen.add_struts(*struts);

        if (screen.showing_struts()) {
            screen.compute_placeable_region();

            if (m_conn.window_is_mappable(event.window))
                m_conn.map_window(event.window);

            apply_layout(mp_workspace);
            must_restack = true;
        } else
            may_map = false;
    }

    std::unordered_set<WindowType> types
        = m_conn.get_window_types(event.window);

    std::unordered_set<WindowState> states
        = m_conn.get_window_states(event.window);

    std::optional<Region> region
        = m_conn.get_window_geometry(event.window);

    std::optional<StackHandler::StackLayer> layer = std::nullopt;

    if (Util::contains(states, WindowState::Below_))
        layer = StackHandler::StackLayer::Below_;
    else if (Util::contains(types, WindowType::Desktop))
        layer = StackHandler::StackLayer::Desktop;
    else if (Util::contains(types, WindowType::Dock)) {
        Screen& screen = mp_partition->screen();

        if (region && !screen.contains_strut(event.window)) {
            const Region screen_region = screen.full_region();
            std::optional<Edge> edge = std::nullopt;
            std::optional<Strut> strut = std::nullopt;

            if (Pos::is_at_origin(region->pos)) {
                if (region->dim.w == screen_region.dim.w) {
                    edge = Edge::Top;
                    strut = Strut { event.window, region->dim.h };
                } else if (region->dim.h == screen_region.dim.h) {
                    edge = Edge::Left;
                    strut = Strut { event.window, region->dim.w };
                } else if (region->dim.w > screen_region.dim.h) {
                    edge = Edge::Top;
                    strut = Strut { event.window, region->dim.h };
                } else if (region->dim.w < screen_region.dim.h) {
                    edge = Edge::Left;
                    strut = Strut { event.window, region->dim.w };
                }
            }

            if (!strut) {
                if (region->pos.y == region->dim.h) {
                    edge = Edge::Bottom;
                    strut = Strut { event.window, screen_region.dim.h - region->dim.h };
                } else if (region->pos.x == region->dim.w) {
                    edge = Edge::Right;
                    strut = Strut { event.window, screen_region.dim.w - region->dim.w };
                }
            }

            if (strut) {
                screen.add_strut(*edge, *strut);

                if (screen.showing_struts()) {
                    screen.compute_placeable_region();
                    m_conn.map_window(strut->window);

                    apply_layout(mp_workspace);
                } else
                    may_map = false;
            }
        }

        layer = StackHandler::StackLayer::Dock;
    } else if (Util::contains(types, WindowType::Notification))
        layer = StackHandler::StackLayer::Notification;
    else if (Util::contains(states, WindowState::Above_))
        layer = StackHandler::StackLayer::Above_;

    if (layer) {
        m_stack.add_window(event.window, *layer);
        must_restack = true;
    }

    if (must_restack)
        apply_stack(mp_workspace);

    if (!may_map)
        m_conn.unmap_window(event.window);

    if (!(m_client_map.count(event.window) > 0))
        manage(event.window, event.ignore, may_map);
}

void
Model::handle_map(MapEvent)
{}

void
Model::handle_enter(EnterEvent event)
{
    Client_ptr client = get_client(event.window);

    if (!client || client == mp_focus)
        return;

    unfocus_client(mp_focus);
    focus_client(client);
}

void
Model::handle_leave(LeaveEvent event)
{
    Client_ptr client = get_client(event.window);

    if (!client)
        return;

    unfocus_client(client);
}

void
Model::handle_destroy(DestroyEvent event)
{
    Screen& screen = mp_partition->screen();

    if (screen.contains_strut(event.window)) {
        screen.remove_strut(event.window);
        screen.compute_placeable_region();

        apply_layout(mp_workspace);
        apply_stack(mp_workspace);
    }

    m_stack.remove_window(event.window);

    Util::erase_remove(m_unmanaged_windows, event.window);

    Client_ptr client = get_client(event.window);

    if (!client)
        return;

    unmanage(client);
}

void
Model::handle_expose(ExposeEvent)
{}

void
Model::handle_unmap(UnmapEvent event)
{
    if (Util::contains(m_unmanaged_windows, event.window))
        return;

    handle_destroy(DestroyEvent { event.window });
}

void
Model::handle_state_request(StateRequestEvent event)
{
    Client_ptr client = get_client(event.window);

    if (!client)
        return;

    switch (event.state) {
    case WindowState::Fullscreen:
    {
        set_fullscreen_client(event.action, client);
        return;
    }
    case WindowState::Sticky:
    {
        set_sticky_client(event.action, client);
        return;
    }
    case WindowState::DemandsAttention:
    {
        bool value;

        switch (event.action) {
        case Toggle::On:      value = true;            break;
        case Toggle::Off:     value = false;           break;
        case Toggle::Reverse: value = !client->urgent; break;
        default: return;
        }

        m_conn.set_icccm_window_hints(client->window, Hints {
            value,
            std::nullopt,
            std::nullopt,
            std::nullopt
        });

        client->urgent = value;
        render_decoration(client);

        return;
    }
    default: break;
    }
}

void
Model::handle_focus_request(FocusRequestEvent event)
{
    Client_ptr client = get_client(event.window);

    if (!client || event.on_root)
        return;

    focus_client(client);
}

void
Model::handle_close_request(CloseRequestEvent event)
{
    if (!event.on_root)
        m_conn.kill_window(event.window);
}

void
Model::handle_workspace_request(WorkspaceRequestEvent event)
{
    if (!event.on_root)
        activate_workspace(event.index);
}

void
Model::handle_placement_request(PlacementRequestEvent event)
{
    if (!event.pos && !event.dim)
        return;

    Client_ptr client = get_client(event.window);

    if (!client) {
        std::optional<Region> geometry = m_conn.get_window_geometry(event.window);

        if (geometry) {
            if (event.pos)
                geometry->pos = *event.pos;

            if (event.dim)
                geometry->dim = *event.dim;

            m_conn.place_window(event.window, *geometry);
        }

        return;
    }

    if (!is_free(client))
        return;

    Decoration decoration = client->free_decoration;
    Extents extents = Extents { 0, 0, 0, 0 };

    if (decoration.border) {
        extents.left   += decoration.border->width;
        extents.top    += decoration.border->width;
        extents.right  += decoration.border->width;
        extents.bottom += decoration.border->width;
    }

    if (decoration.frame) {
        extents.left   += decoration.frame->extents.left;
        extents.top    += decoration.frame->extents.top;
        extents.right  += decoration.frame->extents.right;
        extents.bottom += decoration.frame->extents.bottom;
    }

    Region region;

    if (event.window == client->window) {
        Pos pos;
        Dim dim;

        if (event.pos)
            pos = Pos {
                event.pos->x - extents.left,
                event.pos->y - extents.top
            };
        else
            pos = client->free_region.pos;

        if (event.dim)
            dim = Dim {
                event.dim->w + extents.left + extents.right,
                event.dim->h + extents.top + extents.bottom
            };
        else
            dim = client->free_region.dim;

        region = Region {
            pos, dim
        };
    } else {
        region = Region {
            event.pos.value_or(client->free_region.pos),
            event.dim.value_or(client->free_region.dim)
        };
    }

    region.remove_extents(extents);

    if (client->size_hints)
        client->size_hints->apply(region.dim);

    region.apply_minimum_dim(Client::MIN_CLIENT_DIM);
    region.apply_extents(extents);

    client->set_free_region(region);

    Placement placement = {
        Placement::PlacementMethod::Free,
        client,
        client->free_decoration,
        region
    };

    place_client(placement);
}

void
Model::handle_grip_request(GripRequestEvent event)
{
    Client_ptr client = get_client(event.window);

    if (!client)
        return;

    stop_moving();
    stop_resizing();

    if (event.grip) {

        m_resize_buffer.set(
            client,
            *event.grip,
            m_conn.get_pointer_position(),
            client->free_region
        );

        m_conn.init_resize(client->frame);
    } else
        start_moving(client);
}

void
Model::handle_restack_request(RestackRequestEvent)
{}

void
Model::handle_configure(ConfigureEvent event)
{
    if (event.on_root) {
        acquire_partitions();
        apply_layout(m_workspaces.active_index());
    }
}

void
Model::handle_property(PropertyEvent event)
{
    switch (event.kind) {
    case PropertyKind::Name:
    {
        Client_ptr client = get_client(event.window);

        if (!client)
            return;

        client->name = m_conn.get_icccm_window_name(event.window);

        return;
    }
    case PropertyKind::Class:
    {
        Client_ptr client = get_client(event.window);

        if (!client)
            return;

        client->class_ = m_conn.get_icccm_window_class(event.window);
        client->instance = m_conn.get_icccm_window_instance(event.window);

        return;
    }
    case PropertyKind::Size:
    {
        Client_ptr client = get_client(event.window);

        if (!client)
            return;

        client->size_hints
            = m_conn.get_icccm_window_size_hints(event.window, Client::MIN_CLIENT_DIM);

        std::optional<Region> geometry = m_conn.get_window_geometry(event.window);

        if (!geometry)
            return;

        Region region = *geometry;

        if (client->size_hints)
            client->size_hints->apply(region.dim);

        region.apply_minimum_dim(Client::MIN_CLIENT_DIM);

        Decoration decoration = client->free_decoration;
        Extents extents = Extents { 0, 0, 0, 0 };

        if (decoration.border) {
            extents.left   += decoration.border->width;
            extents.top    += decoration.border->width;
            extents.right  += decoration.border->width;
            extents.bottom += decoration.border->width;
        }

        if (decoration.frame) {
            extents.left   += decoration.frame->extents.left;
            extents.top    += decoration.frame->extents.top;
            extents.right  += decoration.frame->extents.right;
            extents.bottom += decoration.frame->extents.bottom;
        }

        region.pos = client->free_region.pos;
        region.dim.w += extents.left + extents.right;
        region.dim.h += extents.top + extents.bottom;

        client->set_free_region(region);

        if (client->managed)
            apply_layout(client->workspace);

        return;
    }
    case PropertyKind::Strut:
    {
        std::optional<std::vector<std::optional<Strut>>> struts
            = m_conn.get_window_strut(event.window);

        if (struts) {
            Screen& screen = mp_partition->screen();

            screen.remove_strut(event.window);
            screen.add_struts(*struts);
            screen.compute_placeable_region();

            apply_layout(mp_workspace);
            apply_stack(mp_workspace);
        }

        return;
    }
    default: break;
    }
}

void
Model::handle_frame_extents_request(FrameExtentsRequestEvent event)
{
    Client_ptr client = get_client(event.window);

    Extents extents = Extents { 0, 0, 0, 0 };

    if (client) {
        Decoration decoration = client->active_decoration;

        if (decoration.border) {
            extents.left   += decoration.border->width;
            extents.top    += decoration.border->width;
            extents.right  += decoration.border->width;
            extents.bottom += decoration.border->width;
        }

        if (decoration.frame) {
            extents.left   += decoration.frame->extents.left;
            extents.top    += decoration.frame->extents.top;
            extents.right  += decoration.frame->extents.right;
            extents.bottom += decoration.frame->extents.bottom;
        }
    }

    m_conn.set_window_frame_extents(event.window, extents);
}

void
Model::handle_screen_change()
{
    acquire_partitions();
    apply_layout(m_workspaces.active_index());
}


void
Model::process_command(winsys::CommandMessage message)
{
    static const std::unordered_map<std::string_view, winsys::Direction> directions = {
        { "Forward",  winsys::Direction::Forward },
        { "forward",  winsys::Direction::Forward },
        { "fwd",      winsys::Direction::Forward },
        { "f",        winsys::Direction::Forward },
        { "Backward", winsys::Direction::Backward },
        { "backward", winsys::Direction::Backward },
        { "bwd",      winsys::Direction::Backward },
        { "b",        winsys::Direction::Backward },
    };

    static const std::unordered_map<std::string_view, std::function<void(void)>> commands = {
        { "toggle_partition",
            [&,this]() {
                toggle_partition();
            }
        },
        { "activate_next_partition",
            [&,this]() {
                if (!message.args.empty() && directions.count(message.args.front()))
                    activate_next_partition(directions.at(message.args.front()));
            }
        },
        { "activate_partition",
            [&,this]() {
                if (!message.args.empty()) {
                    std::istringstream index_stream(message.args.front());
                    Index index = 0;

                    if (index_stream >> index)
                        activate_partition(index);
                }
            }
        },

        { "toggle_context",
            [&,this]() {
                toggle_context();
            }
        },
        { "activate_next_context",
            [&,this]() {
                if (!message.args.empty() && directions.count(message.args.front()))
                    activate_next_context(directions.at(message.args.front()));
            }
        },
        { "activate_context",
            [&,this]() {
                if (!message.args.empty()) {
                    std::istringstream index_stream(message.args.front());
                    Index index = 0;

                    if (index_stream >> index)
                        activate_context(index);
                }
            }
        },

        { "toggle_workspace",
            [&,this]() {
                toggle_workspace();
            }
        },
        { "activate_next_workspace",
            [&,this]() {
                if (!message.args.empty() && directions.count(message.args.front()))
                    activate_next_workspace(directions.at(message.args.front()));
            }
        },
        { "activate_workspace",
            [&,this]() {
                if (!message.args.empty()) {
                    std::istringstream index_stream(message.args.front());
                    Index index = 0;

                    if (index_stream >> index)
                        activate_workspace(index);
                }
            }
        },
    };

    if (!message.args.empty()) {
        std::string command = message.args.front();
        message.args.pop_front();

        if (commands.count(command) > 0)
            commands.at(command)();
    }
}

void
Model::process_config(winsys::ConfigMessage)
{
    // TODO
}

void
Model::process_client(winsys::WindowMessage)
{
    // TODO
}

void
Model::process_workspace(winsys::WorkspaceMessage)
{
    // TODO
}

void
Model::process_query(winsys::QueryMessage)
{
    // TODO
}


// static methods

void
Model::wait_children(int)
{
    struct sigaction child_sa;

    std::memset(&child_sa, 0, sizeof(child_sa));
    child_sa.sa_handler = &Model::wait_children;

    sigaction(SIGCHLD, &child_sa,  NULL);
    while (waitpid(-1, 0, WNOHANG) > 0);
}

void
Model::handle_signal(int sig)
{
    if (sig == SIGHUP || sig == SIGINT || sig == SIGTERM)
        g_instance->exit();
}
