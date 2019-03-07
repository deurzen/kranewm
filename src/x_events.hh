#ifndef __KRANEWM__X_EVENTS__GUARD__
#define __KRANEWM__X_EVENTS__GUARD__

#include "common.hh"
#include "rule.hh"
#include "keybind.hh"
#include "mousebind.hh"
#include "x_wrapper/event.hh"

#define ALL ""
#define YES true
#define NO  false
#define CURRENT 0

// Forward decls
class ewmh;
class client_model;

class x_events
{
public:
    explicit x_events(ewmh& ewmh, client_model& clients)
        : m_ewmh(ewmh),
          m_clients(clients),
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
              { { 0x1008ff13,      0 },                   VOLUMEUP                     },
              { { 0x1008ff11,      0 },                   VOLUMEDOWN                   },
              { { 0x1008ff12,      0 },                   VOLUMEMUTE                   },
              { { 0x1008ff02,      0 },                   BRIGHTNESSUP15               },
              { { 0x1008ff03,      0 },                   BRIGHTNESSDOWN15             },
              { { XK_Print,        0 },                   TAKE_SCREENSHOT_SEL          },
              { { 0x1008ff02,      ShiftMask },           BRIGHTNESSUP5                },
              { { 0x1008ff03,      ShiftMask },           BRIGHTNESSDOWN5              },
              { { XK_Print,        ShiftMask },           TAKE_SCREENSHOT              },
              { { XK_Return,       MODMASK },             SPAWN_TERMINAL               },
              { { XK_1,            MODMASK },             ACTIVATE_WORKSPACE_1         },
              { { XK_2,            MODMASK },             ACTIVATE_WORKSPACE_2         },
              { { XK_3,            MODMASK },             ACTIVATE_WORKSPACE_3         },
              { { XK_4,            MODMASK },             ACTIVATE_WORKSPACE_4         },
              { { XK_5,            MODMASK },             ACTIVATE_WORKSPACE_5         },
              { { XK_6,            MODMASK },             ACTIVATE_WORKSPACE_6         },
              { { XK_7,            MODMASK },             ACTIVATE_WORKSPACE_7         },
              { { XK_8,            MODMASK },             ACTIVATE_WORKSPACE_8         },
              { { XK_bracketleft,  MODMASK },             ACTIVATE_PREV_WS             },
              { { XK_bracketright, MODMASK },             ACTIVATE_NEXT_WS             },
              { { XK_grave,        MODMASK },             TOGGLE_SCRATCHPAD_1          },
              { { XK_backslash,    MODMASK },             TOGGLE_SCRATCHPAD_2          },
              { { XK_9,            MODMASK },             ACTIVATE_WORKSPACE_9         },
              { { XK_c,            MODMASK },             KILL_CLIENT                  },
              { { XK_j,            MODMASK },             FOCUS_BCK                    },
              { { XK_k,            MODMASK },             FOCUS_FWD                    },
              { { XK_BackSpace,    MODMASK },             ZOOM                         },
              { { XK_Escape,       MODMASK },             TOGGLE_WORKSPACE             },
              { { XK_comma,        MODMASK },             JUMP_MASTER                  },
              { { XK_d,            MODMASK },             DEC_NMASTER                  },
              { { XK_h,            MODMASK },             DEC_MRATIO                   },
              { { XK_i,            MODMASK },             INC_NMASTER                  },
              { { XK_l,            MODMASK },             INC_MRATIO                   },
              { { XK_z,            MODMASK },             TOGGLE_SHADE                 },
              { { XK_x,            MODMASK },             TOGGLE_ICONIFY               },
              { { XK_m,            MODMASK },             MONOCLE                      },
              { { XK_period,       MODMASK },             JUMP_STACK                   },
              { { XK_slash,        MODMASK },             JUMP_LAST                    },
              { { XK_a,            MODMASK },             JUMP_PANE                    },
              { { XK_t,            MODMASK },             TILE                         },
              { { XK_s,            MODMASK },             SWAP_ORIENTATION             },
              { { XK_p,            MODMASK },             SPAWN_DMENU                  },
              { { XK_q,            MODMASK },             SPAWN_BROWSER                },
              { { XK_space,        MODMASK },             TOGGLE_LAYOUT                },
              { { XK_f,            MODMASK },             TOGGLE_FULLSCREEN            },
              { { XK_e,            MODMASK },             SPAWN_NEOMUTT                },
              { { XK_r,            MODMASK },             SPAWN_RANGER                 },
              { { XK_w,            MODMASK },             SPAWN_QALCULATE              },
              { { XK_g,            MODMASK },             SPAWN_GPICK                  },
              { { XK_semicolon,    MODMASK },             TOGGLE_SCRATCHPAD_1          },
              { { XK_space,        MODMASK|SECMASK },     RHYTHMBOXSHOW                },
              { { XK_p,            MODMASK|SECMASK },     RHYTHMBOXTOGGLE              },
              { { XK_j,            MODMASK|SECMASK },     RHYTHMBOXNEXT                },
              { { XK_k,            MODMASK|SECMASK },     RHYTHMBOXPREV                },
              { { XK_BackSpace,    MODMASK|SECMASK },     RHYTHMBOXSTOP                },
              { { XK_r,            MODMASK|SECMASK },     MPCRANDOM                    },
              { { XK_s,            MODMASK|SECMASK },     MPCSINGLE                    },
              { { XK_1,            MODMASK|SECMASK },     JUMP_CLIENT_1                },
              { { XK_2,            MODMASK|SECMASK },     JUMP_CLIENT_2                },
              { { XK_3,            MODMASK|SECMASK },     JUMP_CLIENT_3                },
              { { XK_4,            MODMASK|SECMASK },     JUMP_CLIENT_4                },
              { { XK_5,            MODMASK|SECMASK },     JUMP_CLIENT_5                },
              { { XK_6,            MODMASK|SECMASK },     JUMP_CLIENT_6                },
              { { XK_7,            MODMASK|SECMASK },     JUMP_CLIENT_7                },
              { { XK_8,            MODMASK|SECMASK },     JUMP_CLIENT_8                },
              { { XK_9,            MODMASK|SECMASK },     JUMP_CLIENT_9                },
              { { XK_Return,       MODMASK|ShiftMask },   SPAWN_QUICKTERM              },
              { { XK_m,            MODMASK|ShiftMask },   MARK_CLIENT                  },
              { { XK_1,            MODMASK|ShiftMask },   CLIENT_TO_WORKSPACE_1        },
              { { XK_2,            MODMASK|ShiftMask },   CLIENT_TO_WORKSPACE_2        },
              { { XK_3,            MODMASK|ShiftMask },   CLIENT_TO_WORKSPACE_3        },
              { { XK_4,            MODMASK|ShiftMask },   CLIENT_TO_WORKSPACE_4        },
              { { XK_5,            MODMASK|ShiftMask },   CLIENT_TO_WORKSPACE_5        },
              { { XK_6,            MODMASK|ShiftMask },   CLIENT_TO_WORKSPACE_6        },
              { { XK_7,            MODMASK|ShiftMask },   CLIENT_TO_WORKSPACE_7        },
              { { XK_8,            MODMASK|ShiftMask },   CLIENT_TO_WORKSPACE_8        },
              { { XK_9,            MODMASK|ShiftMask },   CLIENT_TO_WORKSPACE_9        },
              { { XK_grave,        MODMASK|ShiftMask },   CLIENT_TO_SCRATCHPAD_1       },
              { { XK_backslash,    MODMASK|ShiftMask },   CLIENT_TO_SCRATCHPAD_2       },
              { { XK_bracketleft,  MODMASK|ShiftMask },   CLIENT_TO_PREV_WORKSPACE     },
              { { XK_bracketright, MODMASK|ShiftMask },   CLIENT_TO_NEXT_WORKSPACE     },
              { { XK_d,            MODMASK|ShiftMask },   DECK                         },
              { { XK_g,            MODMASK|ShiftMask },   GRID                         },
              { { XK_j,            MODMASK|ShiftMask },   MOVE_CLIENT_BCK              },
              { { XK_k,            MODMASK|ShiftMask },   MOVE_CLIENT_FWD              },
              { { XK_space,        MODMASK|ShiftMask },   TOGGLE_FLOAT                 },
              { { XK_p,            MODMASK|ShiftMask },   SPAWN_DMENUPASS              },
              { { XK_q,            MODMASK|ShiftMask },   SPAWN_SEC_BROWSER            },
              { { XK_x,            MODMASK|ShiftMask },   POP_ICONIFIED                },
              { { XK_bracketleft,  MODMASK|ControlMask }, DEC_MFACTOR                  },
              { { XK_bracketright, MODMASK|ControlMask }, INC_MFACTOR                  },
              { { XK_m,            MODMASK|ControlMask }, JUMP_TO_MARKED               },
              { { XK_space,        MODMASK|ControlMask }, CENTER_CLIENT                },
              { { XK_BackSpace,    MODMASK|ControlMask }, KILL_CLIENT                  },
              { { XK_h,            MODMASK|ControlMask }, FLOAT_LEFT_OR_MASTER_FWD     },
              { { XK_j,            MODMASK|ControlMask }, FLOAT_DOWN_OR_STACK_BCK      },
              { { XK_k,            MODMASK|ControlMask }, FLOAT_UP_OR_STACK_FWD        },
              { { XK_l,            MODMASK|ControlMask }, FLOAT_RIGHT_OR_MASTER_BCK    },
              { { XK_f,            MODMASK|ControlMask }, FLOAT                        },
              { { XK_d,            MODMASK|ControlMask }, DOUBLEDECK                   },
              { { XK_s,            MODMASK|ControlMask }, SPAWN_SNCLI                  },
              { { XK_Return,       MODMASK|ControlMask }, SPAWN_QUICKTERMTMUX          },
              { { XK_r,            MODMASK|ControlMask }, SPAWN_RTV                    },
              { { XK_i,            MODMASK|ControlMask }, SPAWN_IRSSI                  },
              { { XK_n,            MODMASK|ControlMask }, SPAWN_NEWSBOAT               },
              { { XK_p,            MODMASK|ControlMask }, SPAWN_DMENUPASSCOPY          },
              { { XK_w,            MODMASK|ControlMask }, SPAWN_SAGE                   },
              { { XK_F1,           MODMASK|ControlMask }, SPAWN_7LOCK                  },
              { { XK_1,            MODMASK|ControlMask }, DEICONIFY_1                  },
              { { XK_2,            MODMASK|ControlMask }, DEICONIFY_2                  },
              { { XK_3,            MODMASK|ControlMask }, DEICONIFY_3                  },
              { { XK_4,            MODMASK|ControlMask }, DEICONIFY_4                  },
              { { XK_5,            MODMASK|ControlMask }, DEICONIFY_5                  },
              { { XK_6,            MODMASK|ControlMask }, DEICONIFY_6                  },
              { { XK_7,            MODMASK|ControlMask }, DEICONIFY_7                  },
              { { XK_8,            MODMASK|ControlMask }, DEICONIFY_8                  },
              { { XK_9,            MODMASK|ControlMask }, DEICONIFY_9                  },
              { { XK_h,            MODMASK|ShiftMask|ControlMask }, FLOAT_GROW_LEFT    },
              { { XK_j,            MODMASK|ShiftMask|ControlMask }, FLOAT_GROW_DOWN    },
              { { XK_k,            MODMASK|ShiftMask|ControlMask }, FLOAT_GROW_UP      },
              { { XK_l,            MODMASK|ShiftMask|ControlMask }, FLOAT_GROW_RIGHT   },
              { { XK_y,            MODMASK|ShiftMask|ControlMask }, FLOAT_SHRINK_LEFT  },
              { { XK_u,            MODMASK|ShiftMask|ControlMask }, FLOAT_SHRINK_DOWN  },
              { { XK_i,            MODMASK|ShiftMask|ControlMask }, FLOAT_SHRINK_UP    },
              { { XK_o,            MODMASK|ShiftMask|ControlMask }, FLOAT_SHRINK_RIGHT },
              { { XK_q,            MODMASK|ShiftMask|ControlMask }, QUIT               },
          }),
          m_mousebinds({
              //  keysym              mask     client  operation
              { { SCROLL_UP_BUTTON,   MODMASK,  NO },  GOTO_NEXT_WS   },
              { { SCROLL_DOWN_BUTTON, MODMASK,  NO },  GOTO_PREV_WS   },
              { { FORWARD_BUTTON,     MODMASK,  NO },  GOTO_NEXT_WS   },
              { { BACKWARD_BUTTON,    MODMASK,  NO },  GOTO_PREV_WS   },
              { { LEFT_BUTTON,        MODMASK, YES },  CLIENT_MOVE    },
              { { RIGHT_BUTTON,       MODMASK, YES },  CLIENT_RESIZE  },
              { { MIDDLE_BUTTON,      MODMASK, YES },  CLIENT_CENTER  },
              { { FORWARD_BUTTON,     MODMASK, YES },  CLIENT_NEXT_WS },
              { { BACKWARD_BUTTON,    MODMASK, YES },  CLIENT_PREV_WS },
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
    Rule retrieve_rule(x_wrapper::window_t);

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

    ewmh& m_ewmh;
    client_model& m_clients;
    bool m_running;
    x_wrapper::event_t m_current_event;
    Rules m_rules;
    KeyBinds m_keybinds;
    MouseBinds m_mousebinds;

};

#undef ALL
#undef YES
#undef NO
#undef CURRENT

#endif//__KRANEWM__X_EVENTS__GUARD__
