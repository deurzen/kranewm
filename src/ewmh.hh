#ifndef __KRANEWM__EWMH__GUARD__
#define __KRANEWM__EWMH__GUARD__

#include "x-wrapper/window.hh"
#include "x-wrapper/property.hh"

#include <unordered_map>


enum NetWmIdentifier { // NetWM atom identifiers
    NetSupported = 0, NetFirst = NetSupported,
    NetClientList,
    NetClientListStacking,
    NetNumberOfDesktops,
    NetCurrentDesktop,
    NetDesktopNames,
    NetDesktopGeometry,
    NetDesktopViewport,
    NetWorkarea,
    NetActiveWindow,
    NetWmName,
    NetWmDesktop,
    NetWmStrut,
    NetWmStrutPartial,
    NetWmFrameExtents,
    NetSupportingWmCheck,
    NetWmState,
    NetWmWindowType,
    // window states
    NetWmStateFullscreen, NetWmStateFirst = NetWmStateFullscreen,
    NetWmStateDemandsAttention,
    NetWmStateShaded,
    NetWmStateHidden, NetWmStateLast = NetWmStateHidden,
    // window types
    NetWmWindowTypeDesktop, NetWmWindowTypeFirst = NetWmWindowTypeDesktop,
    NetWmWindowTypeDock,
    NetWmWindowTypeToolbar,
    NetWmWindowTypeMenu,
    NetWmWindowTypeUtility,
    NetWmWindowTypeSplash,
    NetWmWindowTypeDialog,
    NetWmWindowTypeDropdownMenu,
    NetWmWindowTypePopupMenu,
    NetWmWindowTypeTooltip,
    NetWmWindowTypeNotification,
    NetWmWindowTypeNormal, NetWmWindowTypeLast = NetWmWindowTypeNormal,
    NetLast
};

enum NetWmAction { // values used in XClientMessageEvent.data.l[{1,2}]
    NetRemove   = 0,
    NetAdd      = 1,
    NetToggle   = 2,
    NetNoAction
};


class ewmh
{
public:
    ewmh()
      : strut{None, None, None, None, 0, 0, 0, 0}
    {
        static const ::std::unordered_map<int, const char*> NETWM_ATOM_NAMES({
            { NetSupported,                "_NET_SUPPORTED"                    },
            { NetClientList,               "_NET_CLIENT_LIST"                  },
            { NetClientListStacking,       "_NET_CLIENT_LIST_STACKING"         },
            { NetNumberOfDesktops,         "_NET_NUMBER_OF_DESKTOPS"           },
            { NetCurrentDesktop,           "_NET_CURRENT_DESKTOP"              },
            { NetDesktopNames,             "_NET_DESKTOP_NAMES"                },
            { NetDesktopGeometry,          "_NET_DESKTOP_GEOMETRY"             },
            { NetDesktopViewport,          "_NET_DESKTOP_VIEWPORT"             },
            { NetWorkarea,                 "_NET_WORKAREA"                     },
            { NetActiveWindow,             "_NET_ACTIVE_WINDOW"                },
            { NetWmName,                   "_NET_WM_NAME"                      },
            { NetWmDesktop,                "_NET_WM_DESKTOP"                   },
            { NetWmStrut,                  "_NET_WM_STRUT"                     },
            { NetWmStrutPartial,           "_NET_WM_STRUT_PARTIAL"             },
            { NetWmFrameExtents,           "_NET_WM_FRAME_EXTENTS"             },
            { NetSupportingWmCheck,        "_NET_SUPPORTING_WM_CHECK"          },
            { NetWmState,                  "_NET_WM_STATE"                     },
            { NetWmWindowType,             "_NET_WM_WINDOW_TYPE"               },
            // window states
            { NetWmStateFullscreen,        "_NET_WM_STATE_FULLSCREEN"          },
            { NetWmStateDemandsAttention,  "_NET_WM_STATE_DEMANDS_ATTENTION"   },
            { NetWmStateShaded,            "_NET_WM_STATE_SHADED"              },
            { NetWmStateHidden,            "_NET_WM_STATE_HIDDEN"              },
            // window types
            { NetWmWindowTypeDesktop,      "_NET_WM_WINDOW_TYPE_DESKTOP"       },
            { NetWmWindowTypeDock,         "_NET_WM_WINDOW_TYPE_DOCK"          },
            { NetWmWindowTypeToolbar,      "_NET_WM_WINDOW_TYPE_TOOLBAR"       },
            { NetWmWindowTypeMenu,         "_NET_WM_WINDOW_TYPE_MENU"          },
            { NetWmWindowTypeUtility,      "_NET_WM_WINDOW_TYPE_UTILITY"       },
            { NetWmWindowTypeSplash,       "_NET_WM_WINDOW_TYPE_SPLASH"        },
            { NetWmWindowTypeDialog,       "_NET_WM_WINDOW_TYPE_DIALOG"        },
            { NetWmWindowTypeDropdownMenu, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU" },
            { NetWmWindowTypePopupMenu,    "_NET_WM_WINDOW_TYPE_POPUP_MENU"    },
            { NetWmWindowTypeTooltip,      "_NET_WM_WINDOW_TYPE_TOOLTIP"       },
            { NetWmWindowTypeNotification, "_NET_WM_WINDOW_TYPE_NOTIFICATION"  },
            { NetWmWindowTypeNormal,       "_NET_WM_WINDOW_TYPE_NORMAL"        },
        });

        for (auto&& [index,atom_name] : NETWM_ATOM_NAMES)
            netwm_atoms_[index] = x_wrapper::get_atom(atom_name);

        Atom supported_atoms[NetLast];
        for (int i = 0; i < NetLast; ++i)
            supported_atoms[i] = get_netwm_atom(i);

        x_wrapper::replace_property<x_wrapper::atom_list_t>(x_wrapper::g_root,
            {"_NET_SUPPORTED", {supported_atoms, NetLast}});
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

    inline x_wrapper::window_t
    get_top_window() const
    {
        return strut.top_window;
    }

    inline x_wrapper::window_t
    get_bottom_window() const
    {
        return strut.bottom_window;
    }

    inline x_wrapper::window_t
    get_left_window() const
    {
        return strut.left_window;
    }

    inline x_wrapper::window_t
    get_right_window() const
    {
        return strut.right_window;
    }

    void set_strut_property(x_wrapper::window_t, unsigned, unsigned, unsigned, unsigned);
    bool check_apply_strut(x_wrapper::window_t);
    bool check_release_strut(x_wrapper::window_t);

    Atom get_netwm_atom(int);

    void set_wm_name_property(x_wrapper::window_t, ::std::string);
    void set_supporting_wm_check_property(x_wrapper::window_t, x_wrapper::window_t);
    void set_number_of_desktops_property(unsigned&&);
    void set_current_desktop_property(unsigned&&);
    void set_desktop_names_property(::std::vector<::std::string>&);
    void set_frame_extents(x_wrapper::window_t, bool = false);

    void set_active_window_property(x_wrapper::window_t);
    void set_window_state_property(x_wrapper::window_t, int = NetLast);

    void set_wm_desktop_property(x_wrapper::window_t, unsigned);

    void set_desktop_geometry_property();
    void set_desktop_viewport_property();
    void set_workarea_property();

    void clear_client_list_property();
    void clear_active_window_property();
    void clear_workarea_property();


private:
    ::std::map<int, Atom> netwm_atoms_;

    struct {
        x_wrapper::window_t top_window;
        x_wrapper::window_t bottom_window;
        x_wrapper::window_t left_window;
        x_wrapper::window_t right_window;
        unsigned top_height;
        unsigned bottom_height;
        unsigned left_width;
        unsigned right_width;
    } strut;


};


#endif//__KRANEWM__EWMH__GUARD__
