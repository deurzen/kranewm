#ifndef __KRANEWM_INPUT_GUARD__
#define __KRANEWM_INPUT_GUARD__

#include "common.hh"
#include "floating.hh"
#include "ipc.hh"
#include "keybind.hh"
#include "mousebind.hh"
#include "process.hh"

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
              //  keysym  mask                           class
              { { XK_b,   MODMASK             },      "Firefox" },
              { { XK_b,   MODMASK|ShiftMask   },  "qutebrowser" },
              { { XK_b,   MODMASK|ControlMask },      "Spotify" },
          }),
          m_mousebinds({
              //  keysym              mask                                 target        operation                         nofocus
              { { LEFT_BUTTON,        MODMASK,              mousetarget_t::client  },  { commandop_t::clientmovemouse,          NO } },
              { { RIGHT_BUTTON,       MODMASK,              mousetarget_t::client  },  { commandop_t::clientresizemouse,        NO } },
              { { MIDDLE_BUTTON,      MODMASK,              mousetarget_t::client  },  { commandop_t::clientcenter,             NO } },
              { { BACKWARD_BUTTON,    MODMASK,              mousetarget_t::client  },  { commandop_t::clientnextworkspace,      NO } },
              { { FORWARD_BUTTON,     MODMASK,              mousetarget_t::client  },  { commandop_t::clientpreviousworkspace,  NO } },
              { { SCROLL_DOWN_BUTTON, MODMASK,              mousetarget_t::client  },  { commandop_t::focusbackward,           YES } },
              { { SCROLL_UP_BUTTON,   MODMASK,              mousetarget_t::client  },  { commandop_t::focusforward,            YES } },
              { { SCROLL_DOWN_BUTTON, MODMASK|ShiftMask,    mousetarget_t::client  },  { commandop_t::nextworkspace,           YES } },
              { { SCROLL_UP_BUTTON,   MODMASK|ShiftMask,    mousetarget_t::client  },  { commandop_t::previousworkspace,       YES } },
              { { MIDDLE_BUTTON,      MODMASK|ShiftMask,    mousetarget_t::client  },  { commandop_t::clientfloat,              NO } },
              { { RIGHT_BUTTON,       MODMASK|ShiftMask,    mousetarget_t::client  },  { commandop_t::clientfullscreen,         NO } },
              { { SCROLL_DOWN_BUTTON, MODMASK|ControlMask,  mousetarget_t::client  },  { commandop_t::nextcontext,             YES } },
              { { SCROLL_UP_BUTTON,   MODMASK|ControlMask,  mousetarget_t::client  },  { commandop_t::previouscontext,         YES } },
              { { RIGHT_BUTTON,       MODMASK|ControlMask,  mousetarget_t::client  },  { commandop_t::clientdisown,             NO } },
              { { BACKWARD_BUTTON,     NOMASK,              mousetarget_t::root    },  { commandop_t::nextworkspace,           YES } },
              { { FORWARD_BUTTON,      NOMASK,              mousetarget_t::root    },  { commandop_t::previousworkspace,       YES } },
              { { SCROLL_DOWN_BUTTON, MODMASK,              mousetarget_t::root    },  { commandop_t::focusbackward,           YES } },
              { { SCROLL_UP_BUTTON,   MODMASK,              mousetarget_t::root    },  { commandop_t::focusforward,            YES } },
              { { SCROLL_DOWN_BUTTON, MODMASK|ShiftMask,    mousetarget_t::root    },  { commandop_t::nextworkspace,           YES } },
              { { SCROLL_UP_BUTTON,   MODMASK|ShiftMask,    mousetarget_t::root    },  { commandop_t::previousworkspace,       YES } },
              { { SCROLL_DOWN_BUTTON, MODMASK|ControlMask,  mousetarget_t::root    },  { commandop_t::nextcontext,             YES } },
              { { SCROLL_UP_BUTTON,   MODMASK|ControlMask,  mousetarget_t::root    },  { commandop_t::previouscontext,         YES } },
              { { BACKWARD_BUTTON,     NOMASK,              mousetarget_t::sidebar },  { commandop_t::nextworkspace,           YES } },
              { { FORWARD_BUTTON,      NOMASK,              mousetarget_t::sidebar },  { commandop_t::previousworkspace,       YES } },
              { { SCROLL_DOWN_BUTTON,  NOMASK,              mousetarget_t::sidebar },  { commandop_t::focusbackward,           YES } },
              { { SCROLL_UP_BUTTON,    NOMASK,              mousetarget_t::sidebar },  { commandop_t::focusforward,            YES } },
          }),
          m_keybinds({
              //  keysym           mask                   operation
              { { 0x1008ff13,      0 },                   { "amixer -D pulse sset Master 5%+" }                                     },
              { { 0x1008ff11,      0 },                   { "amixer -D pulse sset Master 5%-" }                                     },
              { { 0x1008ff12,      0 },                   { "amixer -D pulse set Master 1+ toggle" }                                },
              { { 0x1008ff14,      0 },                   { "mpc toggle" }                                                          },
              { { 0x1008ff15,      0 },                   { "mpc stop" }                                                            },
              { { 0x1008ff16,      0 },                   { "mpc prev" }                                                            },
              { { 0x1008ff17,      0 },                   { "mpc next" }                                                            },
              { { 0x1008ff02,      0 },                   { "xbacklight -inc 10" }                                                  },
              { { 0x1008ff03,      0 },                   { "xbacklight -dec 10" }                                                  },
              { { XK_Print,        0 },                   { "maim -m 1 -s $(date +~/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)" } },
              { { 0x1008ff02,      ShiftMask },           { "xbacklight -inc 5" }                                                   },
              { { 0x1008ff03,      ShiftMask },           { "xbacklight -dec 5" }                                                   },
              { { XK_Print,        ShiftMask },           { "maim -m 1 $(date +~/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)" }    },
              { { XK_Return,       MODMASK },             { "st" }                                                                  },
              { { XK_asciitilde,   MODMASK },             { "skippy-xd --toggle-window-picker" }                                    },
              { { XK_1,            MODMASK },             { commandop_t::workspaceset, 1 }                                          },
              { { XK_2,            MODMASK },             { commandop_t::workspaceset, 2 }                                          },
              { { XK_3,            MODMASK },             { commandop_t::workspaceset, 3 }                                          },
              { { XK_4,            MODMASK },             { commandop_t::workspaceset, 4 }                                          },
              { { XK_5,            MODMASK },             { commandop_t::workspaceset, 5 }                                          },
              { { XK_6,            MODMASK },             { commandop_t::workspaceset, 6 }                                          },
              { { XK_7,            MODMASK },             { commandop_t::workspaceset, 7 }                                          },
              { { XK_8,            MODMASK },             { commandop_t::workspaceset, 8 }                                          },
              { { XK_9,            MODMASK },             { commandop_t::workspaceset, 9 }                                          },
              { { XK_bracketleft,  MODMASK },             { commandop_t::previousworkspace }                                        },
              { { XK_bracketright, MODMASK },             { commandop_t::nextworkspace }                                            },
              { { XK_c,            MODMASK },             { commandop_t::clientkill }                                               },
              { { XK_j,            MODMASK },             { commandop_t::focusbackward }                                            },
              { { XK_k,            MODMASK },             { commandop_t::focusforward }                                             },
              { { XK_BackSpace,    MODMASK },             { commandop_t::zoom }                                                     },
              { { XK_Escape,       MODMASK },             { commandop_t::workspaceset, 0 }                                          },
              { { XK_comma,        MODMASK },             { commandop_t::clientmasterjump }                                         },
              { { XK_d,            MODMASK },             { commandop_t::workspacenmaster, -1 }                                     },
              { { XK_h,            MODMASK },             { commandop_t::workspacemfactor, -.05f }                                  },
              { { XK_i,            MODMASK },             { commandop_t::workspacenmaster,  1 }                                     },
              { { XK_l,            MODMASK },             { commandop_t::workspacemfactor,  .05f }                                  },
              { { XK_z,            MODMASK },             { "~/bin/stregion" }                                                      },
              { { XK_x,            MODMASK },             { commandop_t::clientsticky }                                             },
              { { XK_y,            MODMASK },             { commandop_t::clienticonify }                                            },
              { { XK_u,            MODMASK },             { commandop_t::deiconifypop }                                             },
              { { XK_v,            MODMASK },             { commandop_t::sidebarshow }                                              },
              { { XK_m,            MODMASK },             { commandop_t::workspacelayout, layout_t::monocle }                       },
              { { XK_period,       MODMASK },             { commandop_t::clientstackjump }                                          },
              { { XK_slash,        MODMASK },             { commandop_t::clientlastjump }                                           },
              { { XK_a,            MODMASK },             { commandop_t::clientpanejump }                                           },
              { { XK_t,            MODMASK },             { commandop_t::workspacelayout, layout_t::tile }                          },
              { { XK_s,            MODMASK },             { commandop_t::workspacemirror }                                          },
              { { XK_p,            MODMASK },             { "/usr/local/bin/dmenu_run" }                                            },
              { { XK_q,            MODMASK },             { "qutebrowser" }                                                         },
              { { XK_space,        MODMASK },             { commandop_t::workspacelayout, layout_t::toggle }                        },
              { { XK_f,            MODMASK },             { commandop_t::clientfullscreen }                                         },
              { { XK_e,            MODMASK },             { "skippy-xd --toggle-window-picker" }                                    },
              { { XK_w,            MODMASK },             { "qalculate-gtk" }                                                       },
              { { XK_g,            MODMASK },             { commandop_t::workspacelayout, layout_t::center }                        },
              { { XK_minus,        MODMASK },             { commandop_t::workspacegapsize, -1 }                                     },
              { { XK_equal,        MODMASK },             { commandop_t::workspacegapsize,  1 }                                     },
              { { XK_F1,           MODMASK },             { commandop_t::clientdisown }                                             },
              { { XK_F9,           MODMASK },             { commandop_t::reclaimpop }                                               },
              { { XK_F4,           MODMASK },             { commandop_t::clientbelow }                                              },
              { { XK_F5,           MODMASK },             { commandop_t::clientabove }                                              },
              { { XK_F10,          MODMASK },             { commandop_t::profilesave, 0 }                                           },
              { { XK_F11,          MODMASK },             { commandop_t::profilesave, 1 }                                           },
              { { XK_F12,          MODMASK },             { commandop_t::profilesave, 2 }                                           },
              { { XK_p,            MODMASK|SECMASK },     { "mpc toggle" }                                                          },
              { { XK_j,            MODMASK|SECMASK },     { "mpc next" }                                                            },
              { { XK_k,            MODMASK|SECMASK },     { "mpc prev" }                                                            },
              { { XK_BackSpace,    MODMASK|SECMASK },     { "mpc stop" }                                                            },
              { { XK_r,            MODMASK|SECMASK },     { "mpc random" }                                                          },
              { { XK_s,            MODMASK|SECMASK },     { "mpc single" }                                                          },
              { { XK_1,            MODMASK|SECMASK },     { commandop_t::clientdeiconifyindex, 1 }                                  },
              { { XK_2,            MODMASK|SECMASK },     { commandop_t::clientdeiconifyindex, 2 }                                  },
              { { XK_3,            MODMASK|SECMASK },     { commandop_t::clientdeiconifyindex, 3 }                                  },
              { { XK_4,            MODMASK|SECMASK },     { commandop_t::clientdeiconifyindex, 4 }                                  },
              { { XK_5,            MODMASK|SECMASK },     { commandop_t::clientdeiconifyindex, 5 }                                  },
              { { XK_6,            MODMASK|SECMASK },     { commandop_t::clientdeiconifyindex, 6 }                                  },
              { { XK_7,            MODMASK|SECMASK },     { commandop_t::clientdeiconifyindex, 7 }                                  },
              { { XK_8,            MODMASK|SECMASK },     { commandop_t::clientdeiconifyindex, 8 }                                  },
              { { XK_9,            MODMASK|SECMASK },     { commandop_t::clientdeiconifyindex, 9 }                                  },
              { { XK_Return,       MODMASK|ShiftMask },   { "st -n \"kranewm:cf\"" }                                                },
              { { XK_equal,        MODMASK|ShiftMask },   { commandop_t::workspacegapsize, 0 }                                      },
              { { XK_m,            MODMASK|ShiftMask },   { commandop_t::clientmarkset }                                            },
              { { XK_1,            MODMASK|ShiftMask },   { commandop_t::clientworkspace, 1 }                                       },
              { { XK_2,            MODMASK|ShiftMask },   { commandop_t::clientworkspace, 2 }                                       },
              { { XK_3,            MODMASK|ShiftMask },   { commandop_t::clientworkspace, 3 }                                       },
              { { XK_4,            MODMASK|ShiftMask },   { commandop_t::clientworkspace, 4 }                                       },
              { { XK_5,            MODMASK|ShiftMask },   { commandop_t::clientworkspace, 5 }                                       },
              { { XK_6,            MODMASK|ShiftMask },   { commandop_t::clientworkspace, 6 }                                       },
              { { XK_7,            MODMASK|ShiftMask },   { commandop_t::clientworkspace, 7 }                                       },
              { { XK_8,            MODMASK|ShiftMask },   { commandop_t::clientworkspace, 8 }                                       },
              { { XK_9,            MODMASK|ShiftMask },   { commandop_t::clientworkspace, 9 }                                       },
              { { XK_bracketleft,  MODMASK|ShiftMask },   { commandop_t::clientpreviousworkspace }                                  },
              { { XK_bracketright, MODMASK|ShiftMask },   { commandop_t::clientnextworkspace }                                      },
              { { XK_t,            MODMASK|ShiftMask },   { commandop_t::workspacelayout, layout_t::stick }                         },
              { { XK_d,            MODMASK|ShiftMask },   { commandop_t::workspacelayout, layout_t::deck }                          },
              { { XK_w,            MODMASK|ShiftMask },   { commandop_t::workspacelayout, layout_t::sdeck }                         },
              { { XK_g,            MODMASK|ShiftMask },   { commandop_t::workspacelayout, layout_t::centerstack }                   },
              { { XK_l,            MODMASK|ShiftMask },   { commandop_t::workspacelayout, layout_t::pillar }                        },
              { { XK_h,            MODMASK|ShiftMask },   { commandop_t::workspacelayout, layout_t::column }                        },
              { { XK_j,            MODMASK|ShiftMask },   { commandop_t::clientmovebackward }                                       },
              { { XK_k,            MODMASK|ShiftMask },   { commandop_t::clientmoveforward }                                        },
              { { XK_v,            MODMASK|ShiftMask },   { commandop_t::sidebarshowall }                                           },
              { { XK_space,        MODMASK|ShiftMask },   { commandop_t::clientfloat }                                              },
              { { XK_p,            MODMASK|ShiftMask },   { "dmenupass" }                                                           },
              { { XK_q,            MODMASK|ShiftMask },   { "firefox" }                                                             },
              { { XK_F10,          MODMASK|ShiftMask },   { commandop_t::profileload, 0 }                                           },
              { { XK_F11,          MODMASK|ShiftMask },   { commandop_t::profileload, 1 }                                           },
              { { XK_F12,          MODMASK|ShiftMask },   { commandop_t::profileload, 2 }                                           },
              { { XK_m,            MODMASK|ControlMask }, { commandop_t::clientmarkjump }                                           },
              { { XK_space,        MODMASK|ControlMask }, { commandop_t::clientcenter }                                             },
              { { XK_Up,           MODMASK|ControlMask }, { commandop_t::clientsnapmovenorth }                                      },
              { { XK_Right,        MODMASK|ControlMask }, { commandop_t::clientsnapmoveeast }                                       },
              { { XK_Down,         MODMASK|ControlMask }, { commandop_t::clientsnapmovesouth }                                      },
              { { XK_Left,         MODMASK|ControlMask }, { commandop_t::clientsnapmovewest }                                       },
              { { XK_BackSpace,    MODMASK|ControlMask }, { commandop_t::clientkill }                                               },
              { { XK_h,            MODMASK|ControlMask }, { commandop_t::floatingconditional,
                                                              new commandbind_t{ commandop_t::clientmove, direction_t::left },
                                                              new commandbind_t{ commandop_t::masterforward }
                                                          } },
              { { XK_j,            MODMASK|ControlMask }, { commandop_t::floatingconditional,
                                                              new commandbind_t{ commandop_t::clientmove, direction_t::down },
                                                              new commandbind_t{ commandop_t::stackbackward }
                                                          } },
              { { XK_k,            MODMASK|ControlMask }, { commandop_t::floatingconditional,
                                                              new commandbind_t{ commandop_t::clientmove, direction_t::up },
                                                              new commandbind_t{ commandop_t::stackforward }
                                                          } },
              { { XK_l,            MODMASK|ControlMask }, { commandop_t::floatingconditional,
                                                              new commandbind_t{ commandop_t::clientmove, direction_t::right },
                                                              new commandbind_t{ commandop_t::masterbackward }
                                                          } },
              { { XK_f,            MODMASK|ControlMask }, { commandop_t::workspacelayout, layout_t::floating }                      },
              { { XK_d,            MODMASK|ControlMask }, { commandop_t::workspacelayout, layout_t::doubledeck }                    },
              { { XK_w,            MODMASK|ControlMask }, { commandop_t::workspacelayout, layout_t::sdoubledeck }                   },
              { { XK_e,            MODMASK|ControlMask }, { "st -g 140x42 -e zsh -i -c neomutt" }                                   },
              { { XK_s,            MODMASK|ControlMask }, { "st -g 80x42 -e zsh -i -c sncli" }                                      },
              { { XK_r,            MODMASK|ControlMask }, { "st -g 80x42 -e zsh -i -c rtv" }                                        },
              { { XK_i,            MODMASK|ControlMask }, { "st -g 80x42 -e zsh -i -c irssi" }                                      },
              { { XK_p,            MODMASK|ControlMask }, { "dmenupass --copy" }                                                    },
              { { XK_o,            MODMASK|ControlMask }, { "dmenunotify" }                                                         },
              { { XK_g,            MODMASK|ControlMask }, { commandop_t::workspacelayout, layout_t::grid }                          },
              { { XK_comma,        MODMASK|ControlMask }, { commandop_t::allbackward }                                              },
              { { XK_semicolon,    MODMASK|ControlMask }, { commandop_t::allforward }                                               },
              { { XK_1,            MODMASK|ControlMask }, { commandop_t::contextset, 1 }                                            },
              { { XK_2,            MODMASK|ControlMask }, { commandop_t::contextset, 2 }                                            },
              { { XK_3,            MODMASK|ControlMask }, { commandop_t::contextset, 3 }                                            },
              { { XK_4,            MODMASK|ControlMask }, { commandop_t::contextset, 4 }                                            },
              { { XK_5,            MODMASK|ControlMask }, { commandop_t::contextset, 5 }                                            },
              { { XK_6,            MODMASK|ControlMask }, { commandop_t::contextset, 6 }                                            },
              { { XK_7,            MODMASK|ControlMask }, { commandop_t::contextset, 7 }                                            },
              { { XK_8,            MODMASK|ControlMask }, { commandop_t::contextset, 8 }                                            },
              { { XK_9,            MODMASK|ControlMask }, { commandop_t::contextset, 9 }                                            },
              { { XK_bracketleft,  MODMASK|ControlMask }, { commandop_t::previouscontext }                                          },
              { { XK_bracketright, MODMASK|ControlMask }, { commandop_t::nextcontext }                                              },
              { { XK_1,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientcontext, 1 }                               },
              { { XK_2,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientcontext, 2 }                               },
              { { XK_3,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientcontext, 3 }                               },
              { { XK_4,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientcontext, 4 }                               },
              { { XK_5,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientcontext, 5 }                               },
              { { XK_6,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientcontext, 6 }                               },
              { { XK_7,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientcontext, 7 }                               },
              { { XK_8,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientcontext, 8 }                               },
              { { XK_9,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientcontext, 9 }                               },
              { { XK_h,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientgrow,   direction_t::left }                },
              { { XK_j,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientgrow,   direction_t::down }                },
              { { XK_k,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientgrow,   direction_t::up }                  },
              { { XK_l,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientgrow,   direction_t::right }               },
              { { XK_y,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientshrink, direction_t::left }                },
              { { XK_u,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientshrink, direction_t::down }                },
              { { XK_i,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientshrink, direction_t::up }                  },
              { { XK_o,            MODMASK|ShiftMask|ControlMask }, { commandop_t::clientshrink, direction_t::right }               },
              { { XK_f,            MODMASK|ShiftMask|ControlMask }, { commandop_t::workspacelayoutpreservedim, layout_t::floating } },
              { { XK_Up,           MODMASK|ShiftMask|ControlMask }, { commandop_t::clientsnapresizenorth }                          },
              { { XK_Right,        MODMASK|ShiftMask|ControlMask }, { commandop_t::clientsnapresizeeast }                           },
              { { XK_Down,         MODMASK|ShiftMask|ControlMask }, { commandop_t::clientsnapresizesouth }                          },
              { { XK_Left,         MODMASK|ShiftMask|ControlMask }, { commandop_t::clientsnapresizewest }                           },
              { { XK_q,            MODMASK|ShiftMask|ControlMask }, { commandop_t::quit }                                           },
          }),
          m_target(nullptr)
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
    void process_mouse_input_sidebar(XButtonEvent);
    void process_mouse_input_client(client_ptr_t, XButtonEvent);
    void process_key_input_global(XKeyEvent);
    void process_key_input_client(client_ptr_t, XKeyEvent);
    void process_ipc_global(ipccommand_t);

    void process_command(commandbind_t);

private:
    command_ptr_t create_command(commandbind_t);

    sidebar_t& m_sidebar;
    client_model_t& m_clients;
    windowstack_t& m_windowstack;
    processjumplist_t& m_processes;
    bool& m_running;
    processbinds_t m_processbinds;
    mousebinds_t m_mousebinds;
    keybinds_t m_keybinds;

    client_ptr_t m_target;

};

#undef YES
#undef NO
#undef NOMASK

#endif//__KRANEWM_INPUT_GUARD__
