#ifndef __KRANEWM__X_EVENTS__GUARD__
#define __KRANEWM__X_EVENTS__GUARD__

#include "common.hh"
#include "rule.hh"
#include "keybind.hh"
#include "mousebind.hh"
#include "x-wrapper/event.hh"

#define ALL ""
#define YES true
#define NO  false
#define CURRENT 0
#define NOMASK 0

// Forward decls
class ewmh_t;
class client_model_t;
class x_model_t;

class x_events_t
{
public:
    explicit x_events_t(ewmh_t& ewmh, client_model_t& clients, x_model_t& x)
        : m_ewmh(ewmh),
          m_clients(clients),
          m_x(x),
          m_running(true),
          m_rules({
              //  class            inst             title      float center icon close  workspace
              { { "Firefox",       ALL,             ALL },   {  NO,  YES,   NO,   OFF,  CURRENT } },
              { { "Gimp",          ALL,             ALL },   { YES,   NO,   NO,   OFF,  4       } },
              { { "Qalculate-gtk", ALL,             ALL },   { YES,  YES,   NO,   OFF,  CURRENT } },
              { { "Rhythmbox",     ALL,             ALL },   { YES,  YES,   NO,  ONCE,  CURRENT } },
              { { "RuneLite",      ALL,             ALL },   { YES,  YES,   NO,   OFF,  CURRENT } },
              { { "URxvt",         "kranewm:float", ALL },   { YES,  YES,   NO,   OFF,  CURRENT } },
              { { "URxvt",         ALL,             ALL },   {  NO,  YES,   NO,   OFF,  CURRENT } },
          }),
          m_keybinds({
              //  keysym           mask                   operation
              { { 0x1008ff13,      0 },                   keyop_t::volumeup                     },
              { { 0x1008ff11,      0 },                   keyop_t::volumedown                   },
              { { 0x1008ff12,      0 },                   keyop_t::volumemute                   },
              { { 0x1008ff02,      0 },                   keyop_t::brightnessup15               },
              { { 0x1008ff03,      0 },                   keyop_t::brightnessdown15             },
              { { XK_Print,        0 },                   keyop_t::take_screenshot_sel          },
              { { 0x1008ff02,      ShiftMask },           keyop_t::brightnessup5                },
              { { 0x1008ff03,      ShiftMask },           keyop_t::brightnessdown5              },
              { { XK_Print,        ShiftMask },           keyop_t::take_screenshot              },
              { { XK_Return,       MODMASK },             keyop_t::spawn_terminal               },
              { { XK_1,            MODMASK },             keyop_t::activate_ws_1                },
              { { XK_2,            MODMASK },             keyop_t::activate_ws_2                },
              { { XK_3,            MODMASK },             keyop_t::activate_ws_3                },
              { { XK_4,            MODMASK },             keyop_t::activate_ws_4                },
              { { XK_5,            MODMASK },             keyop_t::activate_ws_5                },
              { { XK_6,            MODMASK },             keyop_t::activate_ws_6                },
              { { XK_7,            MODMASK },             keyop_t::activate_ws_7                },
              { { XK_8,            MODMASK },             keyop_t::activate_ws_8                },
              { { XK_bracketleft,  MODMASK },             keyop_t::activate_prev_ws             },
              { { XK_bracketright, MODMASK },             keyop_t::activate_next_ws             },
              { { XK_9,            MODMASK },             keyop_t::activate_ws_9                },
              { { XK_c,            MODMASK },             keyop_t::kill_client                  },
              { { XK_j,            MODMASK },             keyop_t::focus_bck                    },
              { { XK_k,            MODMASK },             keyop_t::focus_fwd                    },
              { { XK_BackSpace,    MODMASK },             keyop_t::zoom                         },
              { { XK_Escape,       MODMASK },             keyop_t::toggle_workspace             },
              { { XK_comma,        MODMASK },             keyop_t::jump_master                  },
              { { XK_d,            MODMASK },             keyop_t::dec_n_master                 },
              { { XK_h,            MODMASK },             keyop_t::dec_m_factor                 },
              { { XK_i,            MODMASK },             keyop_t::inc_n_master                 },
              { { XK_l,            MODMASK },             keyop_t::inc_m_factor                 },
              { { XK_z,            MODMASK },             keyop_t::toggle_shade                 },
              { { XK_x,            MODMASK },             keyop_t::toggle_iconify               },
              { { XK_m,            MODMASK },             keyop_t::monocle                      },
              { { XK_period,       MODMASK },             keyop_t::jump_stack                   },
              { { XK_slash,        MODMASK },             keyop_t::jump_last                    },
              { { XK_a,            MODMASK },             keyop_t::jump_pane                    },
              { { XK_t,            MODMASK },             keyop_t::tile                         },
              { { XK_s,            MODMASK },             keyop_t::mirror_workspace             },
              { { XK_p,            MODMASK },             keyop_t::spawn_dmenu                  },
              { { XK_q,            MODMASK },             keyop_t::spawn_browser                },
              { { XK_space,        MODMASK },             keyop_t::toggle_layout                },
              { { XK_f,            MODMASK },             keyop_t::toggle_fullscreen            },
              { { XK_e,            MODMASK },             keyop_t::spawn_neomutt                },
              { { XK_r,            MODMASK },             keyop_t::spawn_ranger                 },
              { { XK_w,            MODMASK },             keyop_t::spawn_qalculate              },
              { { XK_g,            MODMASK },             keyop_t::spawn_gpick                  },
              { { XK_minus,        MODMASK },             keyop_t::dec_gap_size                 },
              { { XK_plus,         MODMASK },             keyop_t::inc_gap_size                 },
              { { XK_1,            MODMASK|SECMASK },     keyop_t::deiconify_1                  },
              { { XK_2,            MODMASK|SECMASK },     keyop_t::deiconify_2                  },
              { { XK_3,            MODMASK|SECMASK },     keyop_t::deiconify_3                  },
              { { XK_4,            MODMASK|SECMASK },     keyop_t::deiconify_4                  },
              { { XK_5,            MODMASK|SECMASK },     keyop_t::deiconify_5                  },
              { { XK_6,            MODMASK|SECMASK },     keyop_t::deiconify_6                  },
              { { XK_7,            MODMASK|SECMASK },     keyop_t::deiconify_7                  },
              { { XK_8,            MODMASK|SECMASK },     keyop_t::deiconify_8                  },
              { { XK_9,            MODMASK|SECMASK },     keyop_t::deiconify_9                  },
              { { XK_space,        MODMASK|SECMASK },     keyop_t::rhythmboxshow                },
              { { XK_p,            MODMASK|SECMASK },     keyop_t::rhythmboxtoggle              },
              { { XK_j,            MODMASK|SECMASK },     keyop_t::rhythmboxnext                },
              { { XK_k,            MODMASK|SECMASK },     keyop_t::rhythmboxprev                },
              { { XK_BackSpace,    MODMASK|SECMASK },     keyop_t::rhythmboxstop                },
              { { XK_r,            MODMASK|SECMASK },     keyop_t::mpcrandom                    },
              { { XK_s,            MODMASK|SECMASK },     keyop_t::mpcsingle                    },
              { { XK_Return,       MODMASK|ShiftMask },   keyop_t::spawn_quickterm              },
              { { XK_m,            MODMASK|ShiftMask },   keyop_t::mark_client                  },
              { { XK_1,            MODMASK|ShiftMask },   keyop_t::client_to_ws_1               },
              { { XK_2,            MODMASK|ShiftMask },   keyop_t::client_to_ws_2               },
              { { XK_3,            MODMASK|ShiftMask },   keyop_t::client_to_ws_3               },
              { { XK_4,            MODMASK|ShiftMask },   keyop_t::client_to_ws_4               },
              { { XK_5,            MODMASK|ShiftMask },   keyop_t::client_to_ws_5               },
              { { XK_6,            MODMASK|ShiftMask },   keyop_t::client_to_ws_6               },
              { { XK_7,            MODMASK|ShiftMask },   keyop_t::client_to_ws_7               },
              { { XK_8,            MODMASK|ShiftMask },   keyop_t::client_to_ws_8               },
              { { XK_9,            MODMASK|ShiftMask },   keyop_t::client_to_ws_9               },
              { { XK_bracketleft,  MODMASK|ShiftMask },   keyop_t::client_to_prev_ws            },
              { { XK_bracketright, MODMASK|ShiftMask },   keyop_t::client_to_next_ws            },
              { { XK_d,            MODMASK|ShiftMask },   keyop_t::deck                         },
              { { XK_g,            MODMASK|ShiftMask },   keyop_t::grid                         },
              { { XK_j,            MODMASK|ShiftMask },   keyop_t::move_client_bck              },
              { { XK_k,            MODMASK|ShiftMask },   keyop_t::move_client_fwd              },
              { { XK_space,        MODMASK|ShiftMask },   keyop_t::toggle_float                 },
              { { XK_p,            MODMASK|ShiftMask },   keyop_t::spawn_dmenupass              },
              { { XK_q,            MODMASK|ShiftMask },   keyop_t::spawn_sec_browser            },
              { { XK_x,            MODMASK|ShiftMask },   keyop_t::pop_iconified                },
              { { XK_bracketleft,  MODMASK|ControlMask }, keyop_t::dec_m1_weight                },
              { { XK_bracketright, MODMASK|ControlMask }, keyop_t::inc_m1_weight                },
              { { XK_m,            MODMASK|ControlMask }, keyop_t::jump_to_marked_client        },
              { { XK_space,        MODMASK|ControlMask }, keyop_t::center_client                },
              { { XK_BackSpace,    MODMASK|ControlMask }, keyop_t::kill_client                  },
              { { XK_h,            MODMASK|ControlMask }, keyop_t::float_left_or_master_fwd     },
              { { XK_j,            MODMASK|ControlMask }, keyop_t::float_down_or_stack_bck      },
              { { XK_k,            MODMASK|ControlMask }, keyop_t::float_up_or_stack_fwd        },
              { { XK_l,            MODMASK|ControlMask }, keyop_t::float_right_or_master_bck    },
              { { XK_f,            MODMASK|ControlMask }, keyop_t::floating                     },
              { { XK_d,            MODMASK|ControlMask }, keyop_t::doubledeck                   },
              { { XK_s,            MODMASK|ControlMask }, keyop_t::spawn_sncli                  },
              { { XK_Return,       MODMASK|ControlMask }, keyop_t::spawn_quicktermtmux          },
              { { XK_r,            MODMASK|ControlMask }, keyop_t::spawn_rtv                    },
              { { XK_i,            MODMASK|ControlMask }, keyop_t::spawn_irssi                  },
              { { XK_n,            MODMASK|ControlMask }, keyop_t::spawn_newsboat               },
              { { XK_p,            MODMASK|ControlMask }, keyop_t::spawn_dmenupasscopy          },
              { { XK_w,            MODMASK|ControlMask }, keyop_t::spawn_sage                   },
              { { XK_F1,           MODMASK|ControlMask }, keyop_t::spawn_7lock                  },
              { { XK_1,            MODMASK|ControlMask }, keyop_t::jump_client_1                },
              { { XK_2,            MODMASK|ControlMask }, keyop_t::jump_client_2                },
              { { XK_3,            MODMASK|ControlMask }, keyop_t::jump_client_3                },
              { { XK_4,            MODMASK|ControlMask }, keyop_t::jump_client_4                },
              { { XK_5,            MODMASK|ControlMask }, keyop_t::jump_client_5                },
              { { XK_6,            MODMASK|ControlMask }, keyop_t::jump_client_6                },
              { { XK_7,            MODMASK|ControlMask }, keyop_t::jump_client_7                },
              { { XK_8,            MODMASK|ControlMask }, keyop_t::jump_client_8                },
              { { XK_9,            MODMASK|ControlMask }, keyop_t::jump_client_9                },
              { { XK_h,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_grow_left    },
              { { XK_j,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_grow_down    },
              { { XK_k,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_grow_up      },
              { { XK_l,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_grow_right   },
              { { XK_y,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_shrink_left  },
              { { XK_u,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_shrink_down  },
              { { XK_i,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_shrink_up    },
              { { XK_o,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_shrink_right },
              { { XK_q,            MODMASK|ShiftMask|ControlMask }, keyop_t::quit               },
          }),
          m_mousebinds({
              //  keysym              mask     client  operation
              { { FORWARD_BUTTON,     NOMASK,   NO },  mouseop_t::goto_next_ws   },
              { { BACKWARD_BUTTON,    NOMASK,   NO },  mouseop_t::goto_prev_ws   },
              { { SCROLL_UP_BUTTON,   MODMASK,  NO },  mouseop_t::goto_next_ws   },
              { { SCROLL_DOWN_BUTTON, MODMASK,  NO },  mouseop_t::goto_prev_ws   },
              { { LEFT_BUTTON,        MODMASK, YES },  mouseop_t::client_move    },
              { { RIGHT_BUTTON,       MODMASK, YES },  mouseop_t::client_resize  },
              { { MIDDLE_BUTTON,      MODMASK, YES },  mouseop_t::client_center  },
              { { FORWARD_BUTTON,     MODMASK, YES },  mouseop_t::client_next_ws },
              { { BACKWARD_BUTTON,    MODMASK, YES },  mouseop_t::client_prev_ws },
              { { SCROLL_UP_BUTTON,   MODMASK, YES },  mouseop_t::goto_next_ws   },
              { { SCROLL_DOWN_BUTTON, MODMASK, YES },  mouseop_t::goto_prev_ws   },
          })
    {
        for (auto&& [shortcut,_] : m_keybinds)
            x_wrapper::grab_key(shortcut.keysym, shortcut.mask);

        for (auto&& [shortcut,_] : m_mousebinds)
            x_wrapper::grab_button(shortcut.button, shortcut.mask);
    }

    bool step();
    void register_window(x_wrapper::window_t);

private:
    rule_t retrieve_rule(x_wrapper::window_t);

    void on_button_press();
    void on_button_release();
    void on_circulate_request();
    void on_client_message();
    void on_configure_notify();
    void on_configure_request();
    void on_destroy_notify();
    void on_expose();
    void on_focus_in();
    void on_key_press();
    void on_map_notify();
    void on_map_request();
    void on_motion_notify();
    void on_property_notify();
    void on_unmap_notify();

    void fork_external(::std::string&&);

    ewmh_t& m_ewmh;
    client_model_t& m_clients;
    x_model_t& m_x;
    bool m_running;
    x_wrapper::event_t m_current_event;
    rules_t m_rules;
    keybinds_t m_keybinds;
    mousebinds_t m_mousebinds;

};

#undef ALL
#undef YES
#undef NO
#undef CURRENT
#undef NOMASK

#endif//__KRANEWM__X_EVENTS__GUARD__
