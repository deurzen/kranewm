#ifndef __KRANEWM__EWMH__GUARD__
#define __KRANEWM__EWMH__GUARD__

#include "x-data/window.hh"
#include "x-data/property.hh"

#include <unordered_map>

#include <sys/types.h>
#include <unistd.h>


enum netwmid_t : int
{ // NetWM atom identifiers
    netsupported = 0, netfirst = netsupported,
    netclientlist,
    netnumberofdesktops,
    netcurrentdesktop,
    netdesktopnames,
    netdesktopgeometry,
    netdesktopviewport,
    networkarea,
    netactivewindow,
    netwmname,
    netwmdesktop,
    netwmstrut,
    netwmstrutpartial,
    netwmframeextents,
    netsupportingwmcheck,
    netwmstate,
    netwmwindowtype,
    // root messages
    netwmclosewindow, netwmrootfirst = netwmclosewindow,
    netwmrestackwindow, netwmrootlast = netwmrestackwindow,
    // window states
    netwmstatefullscreen, netwmstatefirst = netwmstatefullscreen,
    netwmstateabove,
    netwmstatebelow,
    netwmstatedemandsattention,
    netwmstatehidden, netwmstatelast = netwmstatehidden,
    // window types
    netwmwindowtypedesktop, netwmwindowtypefirst = netwmwindowtypedesktop,
    netwmwindowtypedock,
    netwmwindowtypetoolbar,
    netwmwindowtypemenu,
    netwmwindowtypeutility,
    netwmwindowtypesplash,
    netwmwindowtypedialog,
    netwmwindowtypedropdownmenu,
    netwmwindowtypepopupmenu,
    netwmwindowtypetooltip,
    netwmwindowtypenotification,
    netwmwindowtypenormal, netwmwindowtypelast = netwmwindowtypenormal,
    netlast
};

inline netwmid_t&
operator++(netwmid_t& id)
{
    return id = static_cast<netwmid_t>(static_cast<int>(id) + 1);
}

enum class netwmaction_t
{ // values used in XClientMessageEvent.data.l[{1,2}]
    netremove   = 0,
    netadd      = 1,
    nettoggle   = 2,
    netnoaction
};


class ewmh_t final
{
public:
    ewmh_t()
      : strut{None, None, None, None, 0, 0, 0, 0}
    {
        static const ::std::unordered_map<netwmid_t, const char*> NETWM_ATOM_NAMES({
            { netwmid_t::netsupported,                "_NET_SUPPORTED"                    },
            { netwmid_t::netclientlist,               "_NET_CLIENT_LIST"                  },
            { netwmid_t::netnumberofdesktops,         "_NET_NUMBER_OF_DESKTOPS"           },
            { netwmid_t::netcurrentdesktop,           "_NET_CURRENT_DESKTOP"              },
            { netwmid_t::netdesktopnames,             "_NET_DESKTOP_NAMES"                },
            { netwmid_t::netdesktopgeometry,          "_NET_DESKTOP_GEOMETRY"             },
            { netwmid_t::netdesktopviewport,          "_NET_DESKTOP_VIEWPORT"             },
            { netwmid_t::networkarea,                 "_NET_WORKAREA"                     },
            { netwmid_t::netactivewindow,             "_NET_ACTIVE_WINDOW"                },
            { netwmid_t::netwmname,                   "_NET_WM_NAME"                      },
            { netwmid_t::netwmdesktop,                "_NET_WM_DESKTOP"                   },
            { netwmid_t::netwmstrut,                  "_NET_WM_STRUT"                     },
            { netwmid_t::netwmstrutpartial,           "_NET_WM_STRUT_PARTIAL"             },
            { netwmid_t::netwmframeextents,           "_NET_WM_FRAME_EXTENTS"             },
            { netwmid_t::netsupportingwmcheck,        "_NET_SUPPORTING_WM_CHECK"          },
            { netwmid_t::netwmstate,                  "_NET_WM_STATE"                     },
            { netwmid_t::netwmwindowtype,             "_NET_WM_WINDOW_TYPE"               },
            // root messages
            { netwmid_t::netwmclosewindow,            "_NET_CLOSE_WINDOW"                 },
            { netwmid_t::netwmrestackwindow,          "_NET_RESTACK_WINDOW"               },
            // window states
            { netwmid_t::netwmstatefullscreen,        "_NET_WM_STATE_FULLSCREEN"          },
            { netwmid_t::netwmstateabove,             "_NET_WM_STATE_ABOVE"               },
            { netwmid_t::netwmstatebelow,             "_NET_WM_STATE_BELOW"               },
            { netwmid_t::netwmstatedemandsattention,  "_NET_WM_STATE_DEMANDS_ATTENTION"   },
            { netwmid_t::netwmstatehidden,            "_NET_WM_STATE_HIDDEN"              },
            // window types
            { netwmid_t::netwmwindowtypedesktop,      "_NET_WM_WINDOW_TYPE_DESKTOP"       },
            { netwmid_t::netwmwindowtypedock,         "_NET_WM_WINDOW_TYPE_DOCK"          },
            { netwmid_t::netwmwindowtypetoolbar,      "_NET_WM_WINDOW_TYPE_TOOLBAR"       },
            { netwmid_t::netwmwindowtypemenu,         "_NET_WM_WINDOW_TYPE_MENU"          },
            { netwmid_t::netwmwindowtypeutility,      "_NET_WM_WINDOW_TYPE_UTILITY"       },
            { netwmid_t::netwmwindowtypesplash,       "_NET_WM_WINDOW_TYPE_SPLASH"        },
            { netwmid_t::netwmwindowtypedialog,       "_NET_WM_WINDOW_TYPE_DIALOG"        },
            { netwmid_t::netwmwindowtypedropdownmenu, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU" },
            { netwmid_t::netwmwindowtypepopupmenu,    "_NET_WM_WINDOW_TYPE_POPUP_MENU"    },
            { netwmid_t::netwmwindowtypetooltip,      "_NET_WM_WINDOW_TYPE_TOOLTIP"       },
            { netwmid_t::netwmwindowtypenotification, "_NET_WM_WINDOW_TYPE_NOTIFICATION"  },
            { netwmid_t::netwmwindowtypenormal,       "_NET_WM_WINDOW_TYPE_NORMAL"        },
        });

        for (auto&& [id,atom_name] : NETWM_ATOM_NAMES)
            m_netwm_atoms[id] = x_data::get_atom(atom_name);

        Atom supported_atoms[netwmid_t::netlast];
        for (netwmid_t i = netwmid_t::netfirst; i < netwmid_t::netlast; ++i)
            supported_atoms[i] = get_netwm_atom(i);

        x_data::replace_property<x_data::atom_list_t>(x_data::g_root,
            {"_NET_SUPPORTED", {supported_atoms, netwmid_t::netlast}});
    }


    inline int
    get_top_strut() const
    {
        return strut.top_height;
    }

    inline int
    get_bottom_strut() const
    {
        return strut.bottom_height;
    }

    inline int
    get_left_strut() const
    {
        return strut.left_width;
    }

    inline int
    get_right_strut() const
    {
        return strut.right_width;
    }

    inline x_data::window_t
    get_top_window() const
    {
        return strut.top_window;
    }

    inline x_data::window_t
    get_bottom_window() const
    {
        return strut.bottom_window;
    }

    inline x_data::window_t
    get_left_window() const
    {
        return strut.left_window;
    }

    inline x_data::window_t
    get_right_window() const
    {
        return strut.right_window;
    }

    void set_strut_property(x_data::window_t, unsigned, unsigned, unsigned, unsigned);
    bool check_apply_strut(x_data::window_t);
    bool check_release_strut(x_data::window_t);

    Atom get_netwm_atom(netwmid_t);

    void set_supporting_wm_check_property(x_data::window_t, x_data::window_t);
    void set_number_of_desktops_property(unsigned&&);
    void set_current_desktop_property(unsigned&&);
    void set_desktop_names_property(::std::vector<::std::string>&);
    void set_frame_extents(x_data::window_t, bool = false);

    void set_active_window_property(x_data::window_t);
    void set_window_state_property(x_data::window_t, netwmid_t = netwmid_t::netlast);
    void set_window_state_property(x_data::window_t, const ::std::string&);
    void set_window_type_property(x_data::window_t, const ::std::string&);

    void set_wm_name_property(x_data::window_t, const ::std::string&);
    void set_class_property(x_data::window_t, const ::std::string&, ::std::optional<const ::std::string> = ::std::nullopt);

    void set_wm_desktop_property(x_data::window_t, unsigned);
    void set_wm_pid_property(x_data::window_t, unsigned);

    void set_desktop_geometry_property();
    void set_desktop_viewport_property();
    void set_workarea_property();

    void clear_client_list_property();
    void clear_active_window_property();
    void clear_workarea_property();

    void register_to_list(x_data::window_t);
    void unregister_from_list(x_data::window_t);

    void append_client_list_property(x_data::window_t);
    void set_client_list_property(::std::vector<x_data::window_t>&);

private:
    ::std::map<netwmid_t, Atom> m_netwm_atoms;

    ::std::vector<x_data::window_t> m_registered_windows;

    struct {
        x_data::window_t top_window;
        x_data::window_t bottom_window;
        x_data::window_t left_window;
        x_data::window_t right_window;
        unsigned top_height;
        unsigned bottom_height;
        unsigned left_width;
        unsigned right_width;
    } strut;

};


#endif//__KRANEWM__EWMH__GUARD__
