#ifndef __KRANEWM__KEYBIND__GUARD__
#define __KRANEWM__KEYBIND__GUARD__

#include "x-data/key.hh"

#include <unordered_map>

enum class keyop_t
{
    noop,
    quit, zoom, toggle_float, toggle_fullscreen,
    toggle_shade, center_client,
    toggle_iconify, pop_iconified,
    deiconify_1, deiconify_2, deiconify_3,
    deiconify_4, deiconify_5, deiconify_6,
    deiconify_7, deiconify_8, deiconify_9,
    spawn_terminal, spawn_quickterm, spawn_quicktermtmux,
    spawn_dmenupass, spawn_dmenupasscopy, spawn_dmenu,
    spawn_browser, spawn_sec_browser,
    spawn_neomutt, spawn_ranger, spawn_vifm,
    spawn_sncli, spawn_rtv,
    spawn_7lock, spawn_sage, spawn_gpick, spawn_qalculate,
    spawn_irssi, spawn_newsboat, spawn_nixnote,
    cantatashow, rhythmboxshow,
    rhythmboxtoggle, rhythmboxnext, rhythmboxprev, rhythmboxstop,
    mpctoggle, mpcnext, mpcprev, mpcstop, mpcrandom, mpcsingle,
    volumeup, volumedown, volumemute,
    audioplay, audiostop, audioprev, audionext,
    brightnessup15, brightnessdown15,
    brightnessup5, brightnessdown5,
    take_screenshot, take_screenshot_sel,
    kill_client, move_client_fwd, move_client_bck,
    float_grow_up, float_grow_down,
    float_grow_left, float_grow_right,
    float_shrink_up, float_shrink_down,
    float_shrink_left, float_shrink_right,
    float_up_or_stack_fwd, float_down_or_stack_bck,
    float_left_or_master_fwd, float_right_or_master_bck,
    clients_fwd, clients_bck,
    mark_client, jump_to_marked_client,
    inc_nmaster, dec_nmaster,
    inc_m1weight, dec_m1weight,
    inc_mfactor, dec_mfactor,
    inc_gap_size, dec_gap_size, reset_gap_size,
    floating, tile, stick, deck, doubledeck,
    grid, pillar, column, monocle, center,
    mirror_workspace, toggle_layout,
    jump_master, jump_stack, jump_last, jump_pane,
    jump_client_1, jump_client_2, jump_client_3,
    jump_client_4, jump_client_5, jump_client_6,
    jump_client_7, jump_client_8, jump_client_9,
    focus_fwd, focus_bck,
    down_stack, up_stack,
    down_master, up_master,
    activate_ws_1, activate_ws_2, activate_ws_3,
    activate_ws_4, activate_ws_5, activate_ws_6,
    activate_ws_7, activate_ws_8, activate_ws_9,
    toggle_workspace,
    activate_next_ws, activate_prev_ws,
    client_to_next_ws, client_to_prev_ws,
    client_to_ws_1, client_to_ws_2, client_to_ws_3,
    client_to_ws_4, client_to_ws_5, client_to_ws_6,
    client_to_ws_7, client_to_ws_8, client_to_ws_9,
};

struct keyshortcut_t
{
    keyshortcut_t(KeySym _keysym, unsigned _mask)
        : keysym(_keysym),
          mask(_mask) {}

    keyshortcut_t(XKeyEvent event)
        : keysym(x_data::get_keysym(event.keycode)),
          mask(event.state) {}

    inline bool operator==(const keyshortcut_t& ks) const
    {
        return ks.keysym == keysym && ks.mask == mask;
    }

    KeySym keysym;
    unsigned mask;
};

namespace std
{
    template <>
    struct hash<keyshortcut_t>
    {
        std::size_t operator()(const keyshortcut_t& ks) const
        {
            return ks.keysym + 10000 * ks.mask;
        }
    };
}

typedef ::std::unordered_map<keyshortcut_t, keyop_t> keybinds_t;

#endif//__KRANEWM__KEYBIND__GUARD__
