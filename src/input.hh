#ifndef __KRANEWM_INPUT_GUARD__
#define __KRANEWM_INPUT_GUARD__

#include "common.hh"
#include "mousebind.hh"
#include "keybind.hh"
#include "process.hh"
#include "ipc.hh"

#include "x-data/event.hh"

#define YES true
#define NO  false
#define NOMASK 0

// fwd decls
class sidebar_t;
class client_model_t;
class windowstack_t;
class user_workspace_t;
typedef struct client_t* client_ptr_t;


class inputhandler_t
{
public:
    explicit inputhandler_t(sidebar_t& sidebar, client_model_t& clients,
        windowstack_t& windowstack, processjumplist_t& processes, bool& running)
        : m_sidebar(sidebar),
          m_clients(clients),
          m_windowstack(windowstack),
          m_processes(processes),
          m_running(running),
          m_processbinds({
              //  keysym  mask                 class
              { { XK_b,   MODMASK           }, "qutebrowser" },
              { { XK_b,   MODMASK|ShiftMask }, "firefox"     },
          }),
          m_mousebinds({
              //  keysym              mask                 client    operation                      focus
              { { BACKWARD_BUTTON,     NOMASK,              NO },  { mouseop_t::goto_next_ws,        NO } },
              { { FORWARD_BUTTON,      NOMASK,              NO },  { mouseop_t::goto_prev_ws,        NO } },
              { { SCROLL_DOWN_BUTTON, MODMASK,              NO },  { mouseop_t::goto_next_ws,        NO } },
              { { SCROLL_UP_BUTTON,   MODMASK,              NO },  { mouseop_t::goto_prev_ws,        NO } },
              { { SCROLL_DOWN_BUTTON, MODMASK|ShiftMask,    NO },  { mouseop_t::focus_fwd,           NO } },
              { { SCROLL_UP_BUTTON,   MODMASK|ShiftMask,    NO },  { mouseop_t::focus_bck,           NO } },
              { { SCROLL_DOWN_BUTTON, MODMASK|ControlMask,  NO },  { mouseop_t::goto_next_cx,        NO } },
              { { SCROLL_UP_BUTTON,   MODMASK|ControlMask,  NO },  { mouseop_t::goto_prev_cx,        NO } },
              { { LEFT_BUTTON,        MODMASK,             YES },  { mouseop_t::client_move,        YES } },
              { { RIGHT_BUTTON,       MODMASK,             YES },  { mouseop_t::client_resize,      YES } },
              { { MIDDLE_BUTTON,      MODMASK,             YES },  { mouseop_t::center_client,      YES } },
              { { BACKWARD_BUTTON,    MODMASK,             YES },  { mouseop_t::client_next_ws,     YES } },
              { { FORWARD_BUTTON,     MODMASK,             YES },  { mouseop_t::client_prev_ws,     YES } },
              { { SCROLL_DOWN_BUTTON, MODMASK,             YES },  { mouseop_t::goto_next_ws,        NO } },
              { { SCROLL_UP_BUTTON,   MODMASK,             YES },  { mouseop_t::goto_prev_ws,        NO } },
              { { SCROLL_DOWN_BUTTON, MODMASK|ShiftMask,   YES },  { mouseop_t::focus_fwd,           NO } },
              { { SCROLL_UP_BUTTON,   MODMASK|ShiftMask,   YES },  { mouseop_t::focus_bck,           NO } },
              { { MIDDLE_BUTTON,      MODMASK|ShiftMask,   YES },  { mouseop_t::toggle_float,       YES } },
              { { RIGHT_BUTTON,       MODMASK|ShiftMask,   YES },  { mouseop_t::toggle_fullscreen,  YES } },
              { { SCROLL_DOWN_BUTTON, MODMASK|ControlMask, YES },  { mouseop_t::goto_next_cx,        NO } },
              { { SCROLL_UP_BUTTON,   MODMASK|ControlMask, YES },  { mouseop_t::goto_prev_cx,        NO } },
              { { RIGHT_BUTTON,       MODMASK|ControlMask, YES },  { mouseop_t::toggle_disown,      YES } },
              { { SCROLL_DOWN_BUTTON, MODMASK|ControlMask, YES },  { mouseop_t::goto_next_cx,        NO } },
              { { SCROLL_UP_BUTTON,   MODMASK|ControlMask, YES },  { mouseop_t::goto_prev_cx,        NO } },
          }),
          m_keybinds({
              //  keysym           mask                   operation
              { { 0x1008ff13,      0 },                   keyop_t::volumeup                     },
              { { 0x1008ff11,      0 },                   keyop_t::volumedown                   },
              { { 0x1008ff12,      0 },                   keyop_t::volumemute                   },
              { { 0x1008ff14,      0 },                   keyop_t::audioplay                    },
              { { 0x1008ff15,      0 },                   keyop_t::audiostop                    },
              { { 0x1008ff16,      0 },                   keyop_t::audioprev                    },
              { { 0x1008ff17,      0 },                   keyop_t::audionext                    },
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
              { { XK_9,            MODMASK },             keyop_t::activate_ws_9                },
              { { XK_bracketleft,  MODMASK },             keyop_t::activate_prev_ws             },
              { { XK_bracketright, MODMASK },             keyop_t::activate_next_ws             },
              { { XK_c,            MODMASK },             keyop_t::kill_client                  },
              { { XK_j,            MODMASK },             keyop_t::focus_bck                    },
              { { XK_k,            MODMASK },             keyop_t::focus_fwd                    },
              { { XK_BackSpace,    MODMASK },             keyop_t::zoom                         },
              { { XK_Escape,       MODMASK },             keyop_t::toggle_workspace             },
              { { XK_comma,        MODMASK },             keyop_t::jump_master                  },
              { { XK_d,            MODMASK },             keyop_t::dec_nmaster                  },
              { { XK_h,            MODMASK },             keyop_t::dec_mfactor                  },
              { { XK_i,            MODMASK },             keyop_t::inc_nmaster                  },
              { { XK_l,            MODMASK },             keyop_t::inc_mfactor                  },
              { { XK_z,            MODMASK },             keyop_t::spawn_termregion             },
              { { XK_x,            MODMASK },             keyop_t::toggle_sticky                },
              { { XK_y,            MODMASK },             keyop_t::iconify_client               },
              { { XK_u,            MODMASK },             keyop_t::pop_deiconify                },
              { { XK_v,            MODMASK },             keyop_t::toggle_sidebar               },
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
              { { XK_r,            MODMASK },             keyop_t::spawn_ranger                 },
              { { XK_e,            MODMASK },             keyop_t::spawn_vifm                   },
              { { XK_w,            MODMASK },             keyop_t::spawn_qalculate              },
              { { XK_g,            MODMASK },             keyop_t::center                       },
              { { XK_minus,        MODMASK },             keyop_t::dec_gap_size                 },
              { { XK_equal,        MODMASK },             keyop_t::inc_gap_size                 },
              { { XK_F1,           MODMASK },             keyop_t::disown_client                },
              { { XK_F9,           MODMASK },             keyop_t::pop_reclaim_client           },
              { { XK_F4,           MODMASK },             keyop_t::toggle_below                 },
              { { XK_F5,           MODMASK },             keyop_t::toggle_above                 },
              { { XK_F10,          MODMASK },             keyop_t::apply_profile_1              },
              { { XK_F11,          MODMASK },             keyop_t::apply_profile_2              },
              { { XK_F12,          MODMASK },             keyop_t::apply_profile_3              },
              { { XK_space,        MODMASK|SECMASK },     keyop_t::cantatashow                  },
              { { XK_p,            MODMASK|SECMASK },     keyop_t::mpctoggle                    },
              { { XK_j,            MODMASK|SECMASK },     keyop_t::mpcnext                      },
              { { XK_k,            MODMASK|SECMASK },     keyop_t::mpcprev                      },
              { { XK_BackSpace,    MODMASK|SECMASK },     keyop_t::mpcstop                      },
              { { XK_r,            MODMASK|SECMASK },     keyop_t::mpcrandom                    },
              { { XK_s,            MODMASK|SECMASK },     keyop_t::mpcsingle                    },
              { { XK_1,            MODMASK|SECMASK },     keyop_t::deiconify_client_1           },
              { { XK_2,            MODMASK|SECMASK },     keyop_t::deiconify_client_2           },
              { { XK_3,            MODMASK|SECMASK },     keyop_t::deiconify_client_3           },
              { { XK_4,            MODMASK|SECMASK },     keyop_t::deiconify_client_4           },
              { { XK_5,            MODMASK|SECMASK },     keyop_t::deiconify_client_5           },
              { { XK_6,            MODMASK|SECMASK },     keyop_t::deiconify_client_6           },
              { { XK_7,            MODMASK|SECMASK },     keyop_t::deiconify_client_7           },
              { { XK_8,            MODMASK|SECMASK },     keyop_t::deiconify_client_8           },
              { { XK_9,            MODMASK|SECMASK },     keyop_t::deiconify_client_9           },
              { { XK_Return,       MODMASK|ShiftMask },   keyop_t::spawn_quickterm              },
              { { XK_semicolon,    MODMASK|ShiftMask },   keyop_t::inc_gap_size                 },
              { { XK_equal,        MODMASK|ShiftMask },   keyop_t::reset_gap_size               },
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
              { { XK_t,            MODMASK|ShiftMask },   keyop_t::stick                        },
              { { XK_d,            MODMASK|ShiftMask },   keyop_t::deck                         },
              { { XK_w,            MODMASK|ShiftMask },   keyop_t::sdeck                        },
              { { XK_g,            MODMASK|ShiftMask },   keyop_t::centerstack                  },
              { { XK_l,            MODMASK|ShiftMask },   keyop_t::pillar                       },
              { { XK_h,            MODMASK|ShiftMask },   keyop_t::column                       },
              { { XK_j,            MODMASK|ShiftMask },   keyop_t::move_client_bck              },
              { { XK_k,            MODMASK|ShiftMask },   keyop_t::move_client_fwd              },
              { { XK_space,        MODMASK|ShiftMask },   keyop_t::toggle_float                 },
              { { XK_p,            MODMASK|ShiftMask },   keyop_t::spawn_dmenupass              },
              { { XK_q,            MODMASK|ShiftMask },   keyop_t::spawn_sec_browser            },
              { { XK_F10,          MODMASK|ShiftMask },   keyop_t::save_profile_1               },
              { { XK_F11,          MODMASK|ShiftMask },   keyop_t::save_profile_2               },
              { { XK_F12,          MODMASK|ShiftMask },   keyop_t::save_profile_3               },
              { { XK_m,            MODMASK|ControlMask }, keyop_t::jump_to_marked_client        },
              { { XK_space,        MODMASK|ControlMask }, keyop_t::center_client                },
              { { XK_Up,           MODMASK|ControlMask }, keyop_t::snap_north                   },
              { { XK_Right,        MODMASK|ControlMask }, keyop_t::snap_east                    },
              { { XK_Down,         MODMASK|ControlMask }, keyop_t::snap_south                   },
              { { XK_Left,         MODMASK|ControlMask }, keyop_t::snap_west                    },
              { { XK_BackSpace,    MODMASK|ControlMask }, keyop_t::kill_client                  },
              { { XK_h,            MODMASK|ControlMask }, keyop_t::float_left_or_master_fwd     },
              { { XK_j,            MODMASK|ControlMask }, keyop_t::float_down_or_stack_bck      },
              { { XK_k,            MODMASK|ControlMask }, keyop_t::float_up_or_stack_fwd        },
              { { XK_l,            MODMASK|ControlMask }, keyop_t::float_right_or_master_bck    },
              { { XK_f,            MODMASK|ControlMask }, keyop_t::floating                     },
              { { XK_d,            MODMASK|ControlMask }, keyop_t::doubledeck                   },
              { { XK_w,            MODMASK|ControlMask }, keyop_t::sdoubledeck                  },
              { { XK_e,            MODMASK|ControlMask }, keyop_t::spawn_neomutt                },
              { { XK_s,            MODMASK|ControlMask }, keyop_t::spawn_sncli                  },
              { { XK_Return,       MODMASK|ControlMask }, keyop_t::spawn_quicktermtmux          },
              { { XK_r,            MODMASK|ControlMask }, keyop_t::spawn_rtv                    },
              { { XK_i,            MODMASK|ControlMask }, keyop_t::spawn_irssi                  },
              { { XK_n,            MODMASK|ControlMask }, keyop_t::spawn_nixnote                },
              { { XK_p,            MODMASK|ControlMask }, keyop_t::spawn_dmenupasscopy          },
              { { XK_o,            MODMASK|ControlMask }, keyop_t::spawn_dmenunotify            },
              { { XK_g,            MODMASK|ControlMask }, keyop_t::grid                         },
              { { XK_a,            MODMASK|ControlMask }, keyop_t::spawn_anki                   },
              { { XK_comma,        MODMASK|ControlMask }, keyop_t::clients_bck                  },
              { { XK_semicolon,    MODMASK|ControlMask }, keyop_t::clients_fwd                  },
              { { XK_1,            MODMASK|ControlMask }, keyop_t::activate_cx_1                },
              { { XK_2,            MODMASK|ControlMask }, keyop_t::activate_cx_2                },
              { { XK_3,            MODMASK|ControlMask }, keyop_t::activate_cx_3                },
              { { XK_4,            MODMASK|ControlMask }, keyop_t::activate_cx_4                },
              { { XK_5,            MODMASK|ControlMask }, keyop_t::activate_cx_5                },
              { { XK_6,            MODMASK|ControlMask }, keyop_t::activate_cx_6                },
              { { XK_7,            MODMASK|ControlMask }, keyop_t::activate_cx_7                },
              { { XK_8,            MODMASK|ControlMask }, keyop_t::activate_cx_8                },
              { { XK_9,            MODMASK|ControlMask }, keyop_t::activate_cx_9                },
              { { XK_bracketleft,  MODMASK|ControlMask }, keyop_t::activate_prev_cx             },
              { { XK_bracketright, MODMASK|ControlMask }, keyop_t::activate_next_cx             },
              { { XK_1,            MODMASK|ShiftMask|ControlMask }, keyop_t::client_to_cx_1     },
              { { XK_2,            MODMASK|ShiftMask|ControlMask }, keyop_t::client_to_cx_2     },
              { { XK_3,            MODMASK|ShiftMask|ControlMask }, keyop_t::client_to_cx_3     },
              { { XK_4,            MODMASK|ShiftMask|ControlMask }, keyop_t::client_to_cx_4     },
              { { XK_5,            MODMASK|ShiftMask|ControlMask }, keyop_t::client_to_cx_5     },
              { { XK_6,            MODMASK|ShiftMask|ControlMask }, keyop_t::client_to_cx_6     },
              { { XK_7,            MODMASK|ShiftMask|ControlMask }, keyop_t::client_to_cx_7     },
              { { XK_8,            MODMASK|ShiftMask|ControlMask }, keyop_t::client_to_cx_8     },
              { { XK_9,            MODMASK|ShiftMask|ControlMask }, keyop_t::client_to_cx_9     },
              { { XK_h,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_grow_left    },
              { { XK_j,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_grow_down    },
              { { XK_k,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_grow_up      },
              { { XK_l,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_grow_right   },
              { { XK_y,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_shrink_left  },
              { { XK_u,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_shrink_down  },
              { { XK_i,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_shrink_up    },
              { { XK_o,            MODMASK|ShiftMask|ControlMask }, keyop_t::float_shrink_right },
              { { XK_q,            MODMASK|ShiftMask|ControlMask }, keyop_t::quit               },
          })
    {
        for (auto&& [shortcut,name] : m_processbinds) {
            x_data::grab_key(shortcut.keysym, shortcut.mask);
            m_processes.activate_process_name(name);
        }

        for (auto&& [shortcut,_] : m_keybinds)
            x_data::grab_key(shortcut.keysym, shortcut.mask);

        for (auto&& [shortcut,_] : m_mousebinds)
            if (shortcut.mask) x_data::grab_button(shortcut.button, shortcut.mask);
    }

    bool moves_focus(XButtonEvent) const;

    void process_mouse_input_global(XButtonEvent);
    void process_mouse_input_client(client_ptr_t, XButtonEvent);

    void process_key_input_global(XKeyEvent);
    void process_key_input_client(client_ptr_t, XKeyEvent);

    void process_ipc_global(ipccommand_t);

private:
    void fork_external(::std::string&&);

    sidebar_t& m_sidebar;
    client_model_t& m_clients;
    windowstack_t& m_windowstack;
    processjumplist_t& m_processes;
    bool& m_running;
    processbinds_t m_processbinds;
    mousebinds_t m_mousebinds;
    keybinds_t m_keybinds;

};

#undef YES
#undef NO
#undef NOMASK

#endif//__KRANEWM_INPUT_GUARD__
