#include "../common.hh"
#include "../util.hh"
#include "xconnection.hh"

#include <cstring>
#include <iterator>
#include <numeric>
#include <proc/readproc.h>
#include <sstream>
#include <unistd.h>

extern "C" {
#include <X11/XF86keysym.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/extensions/XRes.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <xkbcommon/xkbcommon-keysyms.h>
}

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

XConnection::XConnection()
    : mp_dpy(XOpenDisplay(NULL)),
      m_conn_number(XConnectionNumber(mp_dpy)),
      m_root(XDefaultRootWindow(mp_dpy)),
      m_check_window(XCreateWindow(
          mp_dpy,
          m_root,
          -1, -1,
          1, 1,
          CopyFromParent, 0, InputOnly,
          CopyFromParent, 0, NULL
      )),
      m_conn(XConnectionNumber(mp_dpy)),
      m_interned_atoms({}),
      m_atom_names({}),
      m_keys({}),
      m_keycodes({}),
      m_netwm_atoms({})
{
    static const std::unordered_map<NetWMID, const char*> NETWM_ATOM_NAMES({
        { NetWMID::NetSupported,                "_NET_SUPPORTED"                    },
        { NetWMID::NetClientList,               "_NET_CLIENT_LIST"                  },
        { NetWMID::NetNumberOfDesktops,         "_NET_NUMBER_OF_DESKTOPS"           },
        { NetWMID::NetCurrentDesktop,           "_NET_CURRENT_DESKTOP"              },
        { NetWMID::NetDesktopNames,             "_NET_DESKTOP_NAMES"                },
        { NetWMID::NetDesktopGeometry,          "_NET_DESKTOP_GEOMETRY"             },
        { NetWMID::NetDesktopViewport,          "_NET_DESKTOP_VIEWPORT"             },
        { NetWMID::NetWorkarea,                 "_NET_WORKAREA"                     },
        { NetWMID::NetActiveWindow,             "_NET_ACTIVE_WINDOW"                },
        { NetWMID::NetWMName,                   "_NET_WM_NAME"                      },
        { NetWMID::NetWMDesktop,                "_NET_WM_DESKTOP"                   },
        { NetWMID::NetWMStrut,                  "_NET_WM_STRUT"                     },
        { NetWMID::NetWMStrutPartial,           "_NET_WM_STRUT_PARTIAL"             },
        { NetWMID::NetWMFrameExtents,           "_NET_WM_FRAME_EXTENTS"             },
        { NetWMID::NetSupportingWMCheck,        "_NET_SUPPORTING_WM_CHECK"          },
        { NetWMID::NetWMState,                  "_NET_WM_STATE"                     },
        { NetWMID::NetWMWindowType,             "_NET_WM_WINDOW_TYPE"               },
        // root messages
        { NetWMID::NetWMCloseWindow,            "_NET_CLOSE_WINDOW"                 },
        { NetWMID::NetWMMoveResize,             "_NET_WM_MOVERESIZE"                },
        { NetWMID::NetRequestFrameExtents,      "_NET_REQUEST_FRAME_EXTENTS"        },
        { NetWMID::NetMoveResizeWindow,         "_NET_MOVERESIZE_WINDOW"            },
        // window states
        { NetWMID::NetWMStateFullscreen,        "_NET_WM_STATE_FULLSCREEN"          },
        { NetWMID::NetWMStateAbove,             "_NET_WM_STATE_ABOVE"               },
        { NetWMID::NetWMStateBelow,             "_NET_WM_STATE_BELOW"               },
        { NetWMID::NetWMStateDemandsAttention,  "_NET_WM_STATE_DEMANDS_ATTENTION"   },
        { NetWMID::NetWMStateHidden,            "_NET_WM_STATE_HIDDEN"              },
        // window types
        { NetWMID::NetWMWindowTypeDesktop,      "_NET_WM_WINDOW_TYPE_DESKTOP"       },
        { NetWMID::NetWMWindowTypeDock,         "_NET_WM_WINDOW_TYPE_DOCK"          },
        { NetWMID::NetWMWindowTypeToolbar,      "_NET_WM_WINDOW_TYPE_TOOLBAR"       },
        { NetWMID::NetWMWindowTypeMenu,         "_NET_WM_WINDOW_TYPE_MENU"          },
        { NetWMID::NetWMWindowTypeUtility,      "_NET_WM_WINDOW_TYPE_UTILITY"       },
        { NetWMID::NetWMWindowTypeSplash,       "_NET_WM_WINDOW_TYPE_SPLASH"        },
        { NetWMID::NetWMWindowTypeDialog,       "_NET_WM_WINDOW_TYPE_DIALOG"        },
        { NetWMID::NetWMWindowTypeDropdownMenu, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU" },
        { NetWMID::NetWMWindowTypePopupMenu,    "_NET_WM_WINDOW_TYPE_POPUP_MENU"    },
        { NetWMID::NetWMWindowTypeTooltip,      "_NET_WM_WINDOW_TYPE_TOOLTIP"       },
        { NetWMID::NetWMWindowTypeNotification, "_NET_WM_WINDOW_TYPE_NOTIFICATION"  },
        { NetWMID::NetWMWindowTypeNormal,       "_NET_WM_WINDOW_TYPE_NORMAL"        },
    });

    for (auto&& [id,name] : NETWM_ATOM_NAMES)
        m_netwm_atoms[id] = get_atom(name);

    for (std::size_t i = 0; i < 256; ++i)
        m_event_dispatcher[i] = &XConnection::on_unimplemented;

    m_event_dispatcher[ButtonPress] = &XConnection::on_button_press;
    m_event_dispatcher[ButtonRelease] = &XConnection::on_button_release;
    m_event_dispatcher[CirculateRequest] = &XConnection::on_circulate_request;
    m_event_dispatcher[ClientMessage] = &XConnection::on_client_message;
    m_event_dispatcher[ConfigureNotify] = &XConnection::on_configure_notify;
    m_event_dispatcher[ConfigureRequest] = &XConnection::on_configure_request;
    m_event_dispatcher[DestroyNotify] = &XConnection::on_destroy_notify;
    m_event_dispatcher[Expose] = &XConnection::on_expose;
    m_event_dispatcher[FocusIn] = &XConnection::on_focus_in;
    m_event_dispatcher[KeyPress] = &XConnection::on_key_press;
    m_event_dispatcher[MapNotify] = &XConnection::on_map_notify;
    m_event_dispatcher[MapRequest] = &XConnection::on_map_request;
    m_event_dispatcher[MappingNotify] = &XConnection::on_mapping_notify;
    m_event_dispatcher[MotionNotify] = &XConnection::on_motion_notify;
    m_event_dispatcher[PropertyNotify] = &XConnection::on_property_notify;
    m_event_dispatcher[UnmapNotify] = &XConnection::on_unmap_notify;

    int event_base, _error_base;
    if (XRRQueryExtension(mp_dpy, &event_base, &_error_base)) {
        m_event_dispatcher[event_base + RRScreenChangeNotify]
            = &XConnection::on_screen_change;
    }
}

XConnection::~XConnection()
{}


bool
XConnection::flush()
{
    XFlush(mp_dpy);
    return true;
}

winsys::Event
XConnection::step()
{
    next_event(m_current_event);

    if (m_current_event.type >= 0 && m_current_event.type <= 256)
        return (this->*(m_event_dispatcher[m_current_event.type]))();

    return std::monostate{};
}

std::vector<winsys::Screen>
XConnection::connected_outputs()
{
    XRRScreenResources* screen_resources = XRRGetScreenResources(mp_dpy, m_root);

    if (!screen_resources)
        return {};

    XRRCrtcInfo* crtc_info = NULL;
    std::vector<winsys::Screen> screens = {};

    for (int i = 0; i < screen_resources->ncrtc; ++i) {
        crtc_info = XRRGetCrtcInfo(
            mp_dpy,
            screen_resources,
            screen_resources->crtcs[i]
        );

        if (crtc_info->width > 0 && crtc_info->height > 0) {
            winsys::Region region = winsys::Region {
                winsys::Pos {
                    crtc_info->x,
                    crtc_info->y
                },
                winsys::Dim {
                    static_cast<int>(crtc_info->width),
                    static_cast<int>(crtc_info->height)
                }
            };

            screens.emplace_back(winsys::Screen(i, region));
        }
    }

    return screens;
}

std::vector<winsys::Window>
XConnection::top_level_windows()
{
    Window _w;
    Window* children;
    unsigned nchildren;

    XQueryTree(mp_dpy, m_root, &_w, &_w, &children, &nchildren);

    std::vector<winsys::Window> windows;
    for (std::size_t i = 0; i < nchildren; ++i)
        if (m_root != children[i])
            windows.push_back(children[i]);

    if (children)
        XFree(children);

    return windows;
}

winsys::Pos
XConnection::get_pointer_position()
{
    winsys::Window _r, _c;
    int rx, ry, _wx, _wy;
    unsigned _m;

    XQueryPointer(
        mp_dpy,
        m_root,
        &_r, &_c,
        &rx, &ry,
        &_wx, &_wy,
        &_m
    );

    return winsys::Pos { rx, ry };
}

void
XConnection::warp_pointer_center_of_window_or_root(std::optional<winsys::Window> window, winsys::Screen& screen)
{
    winsys::Pos pos;

    if (window) {
        XWindowAttributes wa;
        XGetWindowAttributes(mp_dpy, *window, &wa);

        pos = winsys::Pos::from_center_of_dim(
            winsys::Dim {
                wa.width,
                wa.height
            }
        );
    } else
        pos = winsys::Pos::from_center_of_dim(
            screen.placeable_region().dim
        );

    XWarpPointer(
        mp_dpy,
        None,
        window.value_or(m_root),
        0, 0, 0, 0,
        pos.x, pos.y
    );
}

void
XConnection::warp_pointer(winsys::Pos pos)
{
    XWarpPointer(
        mp_dpy,
        None,
        m_root,
        0, 0, 0, 0,
        pos.x, pos.y
    );
}

void
XConnection::warp_pointer_rpos(winsys::Window window, winsys::Pos pos)
{
    XWarpPointer(
        mp_dpy,
        None,
        window,
        0, 0, 0, 0,
        pos.x, pos.y
    );
}

void
XConnection::confine_pointer(winsys::Window window)
{
    if (!m_confined_to) {
        int status = XGrabPointer(
            mp_dpy,
            m_root,
            False,
            PointerMotionMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync,
            m_root,
            None,
            CurrentTime
        );

        if (status == 0) {
            XGrabKeyboard(
                mp_dpy,
                m_root,
                False,
                GrabModeAsync, GrabModeAsync,
                CurrentTime
            );

            m_confined_to = window;
        }
    }
}

bool
XConnection::release_pointer()
{
    if (m_confined_to) {
        XUngrabPointer(mp_dpy, CurrentTime);
        XUngrabKeyboard(mp_dpy, CurrentTime);

        m_confined_to = std::nullopt;
        return true;
    }

    return false;
}

void
XConnection::call_external_command(std::string& command)
{
    if (!fork()) {
        if (mp_dpy)
            close(m_conn_number);

        setsid();
        execl("/bin/sh", "/bin/sh", "-c", ("exec " + command).c_str(), NULL);
        exit(EXIT_SUCCESS);
    }
}

void
XConnection::cleanup()
{
    XUngrabKey(mp_dpy, AnyKey, AnyModifier, m_root);
    XDestroyWindow(mp_dpy, m_check_window);

    unset_card_property(m_root, "_NET_ACTIVE_WINDOW");
    unset_window_property(m_root, "_NET_SUPPORTING_WM_CHECK");
    unset_string_property(m_root, "_NET_WM_NAME");
    unset_string_property(m_root, "WN_CLASS");
    unset_string_property(m_root, "WN_CLASS");
    unset_atomlist_property(m_root, "_NET_SUPPORTED");
    unset_card_property(m_root, "_NET_WM_PID");
    unset_windowlist_property(m_root, "_NET_CLIENT_LIST");

    XCloseDisplay(mp_dpy);
}


// window manipulation
winsys::Window
XConnection::create_frame(winsys::Region region)
{
    winsys::Window window = XCreateSimpleWindow(
        mp_dpy, m_root,
        region.pos.x, region.pos.y,
        region.dim.w, region.dim.h,
        0, 0, 0
    );

    flush();

    return window;
}

void
XConnection::init_window(winsys::Window window, bool)
{
    static const long window_event_mask
        = PropertyChangeMask | StructureNotifyMask | FocusChangeMask;

    XSetWindowAttributes wa;
    wa.event_mask = window_event_mask;

    XChangeWindowAttributes(mp_dpy, window, CWEventMask, &wa);
}

void
XConnection::init_frame(winsys::Window window, bool focus_follows_mouse)
{
    static const long frame_event_mask
        = StructureNotifyMask | SubstructureNotifyMask | SubstructureRedirectMask
        | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

    XSetWindowAttributes wa;
    wa.event_mask = frame_event_mask;

    if (focus_follows_mouse)
        wa.event_mask |= EnterWindowMask;

    XChangeWindowAttributes(mp_dpy, window, CWEventMask, &wa);
}

void
XConnection::init_unmanaged(winsys::Window window)
{
    static const long unmanaged_event_mask = StructureNotifyMask;

    XSetWindowAttributes wa;
    wa.event_mask = unmanaged_event_mask;

    XChangeWindowAttributes(mp_dpy, window, CWEventMask, &wa);
}

void
XConnection::init_move(winsys::Window)
{
    static winsys::Window handle = create_handle();
    confine_pointer(handle);
}

void
XConnection::init_resize(winsys::Window)
{
    static winsys::Window handle = create_handle();
    confine_pointer(handle);
}

void
XConnection::cleanup_window(winsys::Window window)
{
    XDeleteProperty(mp_dpy, window, get_atom("_NET_WM_STATE"));
    XDeleteProperty(mp_dpy, window, get_atom("_NET_WM_DESKTOP"));
}

void
XConnection::map_window(winsys::Window window)
{
    XMapWindow(mp_dpy, window);
}

void
XConnection::unmap_window(winsys::Window window)
{
    XUnmapWindow(mp_dpy, window);
}

void
XConnection::reparent_window(winsys::Window window, winsys::Window parent, winsys::Pos pos)
{
    disable_substructure_events();
    XReparentWindow(mp_dpy, window, parent, pos.x, pos.y);
    enable_substructure_events();
}

void
XConnection::unparent_window(winsys::Window window, winsys::Pos pos)
{
    disable_substructure_events();
    XReparentWindow(mp_dpy, window, m_root, pos.x, pos.y);
    enable_substructure_events();
}

void
XConnection::destroy_window(winsys::Window window)
{
    XDestroyWindow(mp_dpy, window);
}

bool
XConnection::close_window(winsys::Window window)
{
    Atom* protocols;
    int n = 0;

    Atom delete_atom = get_atom("WM_DELETE_WINDOW");
    bool found = false;

    if (XGetWMProtocols(mp_dpy, window, &protocols, &n)) {
        while (!found && n--)
            found = delete_atom == protocols[n];

        XFree(protocols);
    }

    if (found) {
        XEvent event;
        event.type = ClientMessage;
        event.xclient.window = window;
        event.xclient.message_type = get_atom("WM_PROTOCOLS");
        event.xclient.format = 32;
        event.xclient.data.l[0] = get_atom("WM_DELETE_WINDOW");
        event.xclient.data.l[1] = CurrentTime;
        XSendEvent(mp_dpy, window, False, NoEventMask, &event);

        return true;
    }

    return false;
}

bool
XConnection::kill_window(winsys::Window window)
{
    if (!close_window(window)) {
        XGrabServer(mp_dpy);
        XSetErrorHandler(s_passthrough_error_handler);
        XSetCloseDownMode(mp_dpy, DestroyAll);
        XKillClient(mp_dpy, window);
        XSync(mp_dpy, False);
        XSetErrorHandler(s_default_error_handler);
        XUngrabServer(mp_dpy);
    }

    return true;
}

void
XConnection::place_window(winsys::Window window, winsys::Region& region)
{
    disable_substructure_events();
    XMoveResizeWindow(mp_dpy, window, region.pos.x, region.pos.y, region.dim.w, region.dim.h);
    enable_substructure_events();
}

void
XConnection::move_window(winsys::Window window, winsys::Pos pos)
{
    disable_substructure_events();
    XMoveWindow(mp_dpy, window, pos.x, pos.y);
    enable_substructure_events();
}

void
XConnection::resize_window(winsys::Window window, winsys::Dim dim)
{
    disable_substructure_events();
    XResizeWindow(mp_dpy, window, dim.w, dim.h);
    enable_substructure_events();
}

void
XConnection::focus_window(winsys::Window window)
{
    if (window == None)
        window = m_root;

    XSetInputFocus(mp_dpy, window, RevertToNone, CurrentTime);
}

void
XConnection::stack_window_above(winsys::Window window, std::optional<winsys::Window> sibling)
{
    long mask = CWStackMode;

    XWindowChanges wc;
    wc.stack_mode = Above;

    if (sibling) {
        wc.sibling = *sibling;
        mask |= CWSibling;
    }

    XConfigureWindow(mp_dpy, window, mask, &wc);
}

void
XConnection::stack_window_below(winsys::Window window, std::optional<winsys::Window> sibling)
{
    long mask = CWStackMode;

    XWindowChanges wc;
    wc.stack_mode = Below;

    if (sibling != std::nullopt) {
        wc.sibling = *sibling;
        mask |= CWSibling;
    }

    XConfigureWindow(mp_dpy, window, mask, &wc);
}

void
XConnection::insert_window_in_save_set(winsys::Window window)
{
    XChangeSaveSet(mp_dpy, window, SetModeInsert);
}

void
XConnection::grab_bindings(std::vector<winsys::KeyInput>& key_inputs, std::vector<winsys::MouseInput>& mouse_inputs)
{
    static const long mouse_event_mask =
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;

    auto modifier_to_x11 = [](winsys::Modifier modifier) -> std::size_t {
        switch (modifier) {
        case winsys::Modifier::Ctrl:       return ControlMask;
        case winsys::Modifier::Shift:      return ShiftMask;
        case winsys::Modifier::Alt:        return Mod1Mask;
        case winsys::Modifier::Super:      return Mod4Mask;
        case winsys::Modifier::NumLock:    return Mod2Mask;
        case winsys::Modifier::ScrollLock: return Mod5Mask;
        default: return 0;
        }
    };

    std::vector<std::size_t> modifiers_to_ignore = {
        0,
        Mod2Mask,
        Mod5Mask
    };

    for (auto& modifier : modifiers_to_ignore) {
        for (auto& key_input : key_inputs)
            XGrabKey(mp_dpy,
                get_keycode(key_input.key),
                std::accumulate(
                    key_input.modifiers.begin(),
                    key_input.modifiers.end(),
                    0,
                    [modifier_to_x11](std::size_t const& lhs, winsys::Modifier const& rhs) {
                        return lhs | modifier_to_x11(rhs);
                    }
                ) | modifier,
                m_root,
                True,
                GrabModeAsync,
                GrabModeAsync
            );

        for (auto& mouse_input : mouse_inputs)
            XGrabButton(mp_dpy,
                get_buttoncode(mouse_input.button),
                std::accumulate(
                    mouse_input.modifiers.begin(),
                    mouse_input.modifiers.end(),
                    0,
                    [modifier_to_x11](std::size_t const& lhs, winsys::Modifier const& rhs) {
                        return lhs | modifier_to_x11(rhs);
                    }
                ) | modifier,
                m_root,
                False,
                mouse_event_mask,
                GrabModeAsync,
                GrabModeAsync,
                None,
                None
            );
    }

    XSetWindowAttributes wa;
    wa.event_mask = PropertyChangeMask | SubstructureRedirectMask | StructureNotifyMask
        | ButtonPressMask | PointerMotionMask | FocusChangeMask;

    XChangeWindowAttributes(mp_dpy, m_root, CWEventMask, &wa);

    flush();
}

void
XConnection::regrab_buttons(winsys::Window window)
{
    static const long regrab_event_mask
        = ButtonPressMask | ButtonReleaseMask;

    XGrabButton(mp_dpy,
        AnyButton,
        AnyModifier,
        window,
        True,
        regrab_event_mask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None
    );
}

void
XConnection::ungrab_buttons(winsys::Window window)
{
    XUngrabButton(mp_dpy, AnyButton, AnyModifier, window);
}

void
XConnection::unfocus()
{
    XSetInputFocus(mp_dpy, m_root, m_check_window, CurrentTime);
}

void
XConnection::set_window_border_width(winsys::Window window, unsigned width)
{
    XSetWindowBorderWidth(mp_dpy, window, width);
}

void
XConnection::set_window_border_color(winsys::Window window, unsigned color)
{
    XSetWindowBorder(mp_dpy, window, color);
}

void
XConnection::set_window_background_color(winsys::Window window, unsigned color)
{
    XSetWindowBackground(mp_dpy, window, color);
    XClearWindow(mp_dpy, window);
}

void
XConnection::update_window_offset(winsys::Window window, winsys::Window frame)
{
    XWindowAttributes fa;
    XWindowAttributes wa;

    XGetWindowAttributes(mp_dpy, frame, &fa);
    XGetWindowAttributes(mp_dpy, window, &wa);

    XEvent event;
    event.type = ConfigureNotify;
    event.xconfigure.send_event = True;
    event.xconfigure.display = mp_dpy;
    event.xconfigure.event = window;
    event.xconfigure.window = window;
    event.xconfigure.x = fa.x + wa.x;
    event.xconfigure.y = fa.y + wa.x;
    event.xconfigure.width = wa.width;
    event.xconfigure.height = wa.height;
    event.xconfigure.border_width = 0;
    event.xconfigure.above = None;
    event.xconfigure.override_redirect = True;

    XSendEvent(mp_dpy, window, False, StructureNotifyMask, &event);
}

winsys::Window
XConnection::get_focused_window()
{
    winsys::Window window;
    int _i;

    XGetInputFocus(mp_dpy, &window, &_i);
    return window;
}

std::optional<winsys::Region>
XConnection::get_window_geometry(winsys::Window window)
{
    static XWindowAttributes wa;
    if (!XGetWindowAttributes(mp_dpy, window, &wa))
        return std::nullopt;

    return winsys::Region {
        winsys::Pos {
            wa.x,
            wa.y
        },
        winsys::Dim {
            wa.width,
            wa.height
        }
    };
}

std::optional<winsys::Pid>
XConnection::get_window_pid(winsys::Window window)
{
    XResClientIdSpec spec = {
        window,
        XRES_CLIENT_ID_PID_MASK
    };

    XResClientIdSpec client_specs[1] = { spec };

    long n_values = 0;
    XResClientIdValue* client_values = nullptr;

    std::optional<winsys::Pid> pid = std::nullopt;

    if (!XResQueryClientIds(mp_dpy, 1, client_specs, &n_values, &client_values))
        for (long i = 0; i < n_values; ++i)
            if ((client_values[i].spec.mask & XRES_CLIENT_ID_PID_MASK) != 0) {
                CARD32* client_value
                    = reinterpret_cast<CARD32*>(client_values[i].value);

                if (client_value) {
                    CARD32 value = *client_value;

                    if (value > 0) {
                        pid = static_cast<winsys::Pid>(value);
                        goto yield;
                    }
                }
            }
yield:
    XFree(client_values);
    return pid;
}

std::optional<winsys::Pid>
XConnection::get_ppid(std::optional<winsys::Pid> pid)
{
    if (!pid)
        return std::nullopt;

    proc_t proc;
    memset(&proc, 0, sizeof(proc));
    PROCTAB* ptab = openproc(PROC_FILLSTATUS | PROC_PID, &pid);

    std::optional<winsys::Pid> ppid = std::nullopt;

    if (readproc(ptab, &proc) != 0 && proc.ppid > 0) {
        ppid = proc.ppid;
        goto yield;
    }

yield:
    closeproc(ptab);
    return ppid;
}

bool
XConnection::must_manage_window(winsys::Window window)
{
    static const std::vector<winsys::WindowType> ignore_types
        = { winsys::WindowType::Dock, winsys::WindowType::Toolbar };

    XWindowAttributes wa;
    return XGetWindowAttributes(mp_dpy, window, &wa)
        && wa.c_class != InputOnly
        && !wa.override_redirect
        && !window_is_any_of_types(window, ignore_types);
}

bool
XConnection::must_free_window(winsys::Window window)
{
    static const std::vector<winsys::WindowState> free_states = {
        winsys::WindowState::Modal
    };

    static const std::vector<winsys::WindowType> free_types = {
        winsys::WindowType::Dialog,
        winsys::WindowType::Utility,
        winsys::WindowType::Toolbar,
        winsys::WindowType::Splash
    };

    std::optional<Index> desktop = get_window_desktop(window);

    if ((desktop && *desktop == 0xFFFFFFFF)
        || window_is_any_of_states(window, free_states)
        || window_is_any_of_types(window, free_types))
    {
        return true;
    }

    XWindowAttributes wa;
    XGetWindowAttributes(mp_dpy, window, &wa);

    std::optional<winsys::SizeHints> sh
        = get_icccm_window_size_hints(window, std::nullopt);

    if (sh) {
        if (sh->min_width && sh->min_height && sh->max_width && sh->max_height)
            return *sh->max_width > 0 && *sh->max_height > 0
                && *sh->max_width == *sh->min_width && *sh->max_height == *sh->min_height;
    }

    return false;
}

bool
XConnection::window_is_mappable(winsys::Window window)
{
    static const std::vector<winsys::WindowState> unmappable_states = {
        winsys::WindowState::Hidden
    };

    XWindowAttributes wa;
    XGetWindowAttributes(mp_dpy, window, &wa);

    std::optional<winsys::Hints> hints = get_icccm_window_hints(window);

    return (wa.c_class != InputOnly)
        && (hints && hints->initial_state && *hints->initial_state == winsys::IcccmWindowState::Normal)
        && !window_is_any_of_states(window, unmappable_states);
}

// ICCCM
void
XConnection::set_icccm_window_state(winsys::Window window, winsys::IcccmWindowState state)
{
    long window_state;

    switch (state) {
    case winsys::IcccmWindowState::Withdrawn: window_state = WithdrawnState; break;
    case winsys::IcccmWindowState::Normal:    window_state = NormalState;    break;
    case winsys::IcccmWindowState::Iconic:    window_state = IconicState;    break;
    default: return;
    }

    long data[] = { window_state, None };
    XChangeProperty(mp_dpy, window, get_atom("WM_STATE"), get_atom("WM_STATE"), 32,
        PropModeReplace, reinterpret_cast<const unsigned char*>(data), 2);
}

void
XConnection::set_icccm_window_hints(winsys::Window window, winsys::Hints hints)
{
    bool success = false;
    XWMHints hints_;

    XWMHints* x_hints = XGetWMHints(mp_dpy, window);
    if (x_hints) {
        std::memcpy(&hints_, x_hints, sizeof(XWMHints));
        XFree(x_hints);
        success = true;
    }

    if (!success)
        return;

    if (hints.urgent)
        hints_.flags |= XUrgencyHint;
    else
        hints_.flags &= ~XUrgencyHint;

    XSetWMHints(mp_dpy, window, &hints_);
}

std::string
XConnection::get_icccm_window_name(winsys::Window window)
{
    std::string name;
    char name_raw[512];

    if (!get_text_property(window, get_atom("_NET_WM_NAME"), name_raw, sizeof name_raw))
        get_text_property(window, XA_WM_NAME, name_raw, sizeof name_raw);

    if (name_raw[0] == '\0')
        strcpy(name_raw, "N/a");

    name.assign(name_raw);
    return name;
}

std::string
XConnection::get_icccm_window_class(winsys::Window window)
{
    std::string class_ = "N/a";

    XClassHint* hint = XAllocClassHint();
    XGetClassHint(mp_dpy, window, hint);

    if (hint->res_class) {
        class_.assign(hint->res_class);
        XFree(hint);
    }

    return class_;
}

std::string
XConnection::get_icccm_window_instance(winsys::Window window)
{
    std::string instance = "N/a";

    XClassHint* hint = XAllocClassHint();
    XGetClassHint(mp_dpy, window, hint);

    if (hint->res_name) {
        instance.assign(hint->res_name);
        XFree(hint);
    }

    return instance;
}

std::optional<winsys::Window>
XConnection::get_icccm_window_transient_for(winsys::Window window)
{
    winsys::Window transient = None;
    XGetTransientForHint(mp_dpy, window, &transient);

    return transient == None ? std::nullopt : std::optional(transient);
}

std::optional<winsys::Window>
XConnection::get_icccm_window_client_leader(winsys::Window window)
{
    winsys::Window leader = get_window_property(window, "WM_CLIENT_LEADER");

    if (!property_status_ok() || leader == None)
        return std::nullopt;

    return leader;
}

std::optional<winsys::Hints>
XConnection::get_icccm_window_hints(winsys::Window window)
{
    bool success = false;
    XWMHints hints;

    XWMHints* x_hints = XGetWMHints(mp_dpy, window);
    if (x_hints) {
        std::memcpy(&hints, x_hints, sizeof(XWMHints));
        XFree(x_hints);
        success = true;
    }

    if (!success)
        return std::nullopt;

    std::optional<winsys::IcccmWindowState> initial_state;

    switch (hints.initial_state) {
    case NormalState: initial_state = winsys::IcccmWindowState::Normal; break;
    case IconicState: initial_state = winsys::IcccmWindowState::Iconic; break;
    default: return std::nullopt;
    }

    std::optional<winsys::Window> group;

    if (hints.window_group)
        group = hints.window_group;

    return winsys::Hints {
        (hints.flags & XUrgencyHint) != 0,
        hints.input,
        initial_state,
        group
    };
}

std::optional<winsys::SizeHints>
XConnection::get_icccm_window_size_hints(winsys::Window window, std::optional<winsys::Dim> min_window_dim)
{
    XSizeHints sh;
    if (XGetNormalHints(mp_dpy, window, &sh) == 0)
        return std::nullopt;

    std::optional<winsys::Pos> pos = std::nullopt;
    std::optional<unsigned> sh_min_width = std::nullopt;
    std::optional<unsigned> sh_min_height = std::nullopt;
    std::optional<unsigned> sh_base_width = std::nullopt;
    std::optional<unsigned> sh_base_height = std::nullopt;
    std::optional<unsigned> max_width = std::nullopt;
    std::optional<unsigned> max_height = std::nullopt;

    bool by_user = (sh.flags & USPosition) != 0;

    if (sh.x > 0 || sh.y > 0)
        pos = winsys::Pos { sh.x, sh.y };

    if ((sh.flags & PMinSize) != 0) {
        if (sh.min_width > 0)
            sh_min_width = sh.min_width;

        if (sh.min_height > 0)
            sh_min_height = sh.min_height;
    }

    if ((sh.flags & PBaseSize) != 0) {
        if (sh.base_width > 0)
            sh_base_width = sh.base_width;

        if (sh.base_height > 0)
            sh_base_height = sh.base_height;
    }

    if ((sh.flags & PMaxSize) != 0) {
        if (sh.max_width > 0)
            max_width = sh.max_width;

        if (sh.max_height > 0)
            max_height = sh.max_height;
    }

    std::optional<unsigned> min_width = sh_min_width ? sh_min_width : sh_base_width;
    std::optional<unsigned> min_height = sh_min_height ? sh_min_height : sh_base_height;
    std::optional<unsigned> base_width = sh_base_width ? sh_base_width : sh_min_width;
    std::optional<unsigned> base_height = sh_base_height ? sh_base_height : sh_min_height;

    if (min_width) {
        if (min_window_dim) {
            if (min_width < min_window_dim->w)
                min_width = min_window_dim->w;
        } else if (*min_width <= 0)
            min_width = std::nullopt;
    }

    if (min_height) {
        if (min_window_dim) {
            if (min_height < min_window_dim->h)
                min_height = min_window_dim->h;
        } else if (*min_height <= 0)
            min_height = std::nullopt;
    }

    std::optional<unsigned> inc_width = std::nullopt;
    std::optional<unsigned> inc_height = std::nullopt;

    if ((sh.flags & PResizeInc) != 0) {
        if (sh.width_inc > 0 && sh.width_inc < 0xFFFF)
            inc_width = sh.width_inc;

        if (sh.height_inc > 0 && sh.height_inc < 0xFFFF)
            inc_height = sh.height_inc;
    }

    std::optional<double> min_ratio = std::nullopt;
    std::optional<winsys::Ratio> min_ratio_vulgar = std::nullopt;
    std::optional<double> max_ratio = std::nullopt;
    std::optional<winsys::Ratio> max_ratio_vulgar = std::nullopt;

    if ((sh.flags & PAspect) != 0) {
        if (sh.min_aspect.x > 0 && sh.min_aspect.y > 0)
            min_ratio = static_cast<double>(sh.min_aspect.x)
                / static_cast<double>(sh.min_aspect.y);

        min_ratio_vulgar = winsys::Ratio {
            sh.min_aspect.x,
            sh.min_aspect.y
        };

        if (sh.max_aspect.x > 0 && sh.max_aspect.y > 0)
            max_ratio = static_cast<double>(sh.max_aspect.x)
                / static_cast<double>(sh.max_aspect.y);

        max_ratio_vulgar = winsys::Ratio {
            sh.max_aspect.x,
            sh.max_aspect.y
        };
    }

    return winsys::SizeHints {
        by_user,
        pos,
        min_width,
        min_height,
        max_width,
        max_height,
        base_width,
        base_height,
        inc_width,
        inc_height,
        min_ratio,
        max_ratio,
        min_ratio_vulgar,
        max_ratio_vulgar,
    };
}


// EWMH
void
XConnection::init_for_wm(std::string const& wm_name, std::vector<std::string> const& desktop_names)
{
    if (!mp_dpy || !m_root)
        Util::die("unable to set up window manager");

    check_otherwm();

    map_window(m_check_window);
    stack_window_below(m_check_window, std::nullopt);

    XRRSelectInput(
        mp_dpy,
        m_check_window,
        RRScreenChangeNotifyMask
    );

    XSetWindowAttributes wa;
    wa.cursor = XCreateFontCursor(mp_dpy, XC_left_ptr);
    XChangeWindowAttributes(mp_dpy, m_root, CWCursor, &wa);

    std::vector<std::string> wm_class = { wm_name, wm_name };

    replace_string_property(m_check_window, "_NET_WM_NAME", wm_name);
    replace_stringlist_property(m_check_window, "_WM_CLASS", wm_class);
    replace_card_property(m_check_window, "_NET_WM_PID", getpid());
    replace_window_property(m_check_window, "_NET_SUPPORTING_WM_CHECK", m_check_window);

    replace_window_property(m_root, "_NET_SUPPORTING_WM_CHECK", m_check_window);
    replace_string_property(m_root, "_NET_WM_NAME", wm_name);
    replace_stringlist_property(m_root, "_WM_CLASS", wm_class);

    std::vector<Atom> supported_atoms;
    supported_atoms.reserve(NetWMID::NetLast);

    for (NetWMID i = NetWMID::NetFirst; i < NetWMID::NetLast; ++i)
        supported_atoms.push_back(get_netwm_atom(i));

    replace_atomlist_property(m_root, "_NET_SUPPORTED", supported_atoms);
    replace_card_property(m_root, "_NET_WM_PID", getpid());
    unset_window_property(m_root, "_NET_CLIENT_LIST");

    update_desktops(desktop_names);
}

void
XConnection::set_current_desktop(Index index)
{
    replace_card_property(m_root, "_NET_CURRENT_DESKTOP", index);
}

void
XConnection::set_root_window_name(std::string const& name)
{
    replace_string_property(m_root, "WM_NAME", name);
}

void
XConnection::set_window_desktop(winsys::Window window, Index index)
{
    replace_card_property(window, "_NET_WM_DESKTOP", index);
}

void
XConnection::set_window_state(winsys::Window window, winsys::WindowState state, bool on)
{
    Atom atom = get_atom_from_window_state(state);
    if (atom == 0)
        return;

    if (on) {
        std::vector<winsys::WindowState> check_state = { state };
        if (window_is_any_of_states(window, check_state))
            return;

        append_atomlist_property(window, "_NET_WM_STATE", atom);
    } else {
        std::vector<Atom> atoms
            = get_atomlist_property(window, "_NET_WM_STATE");

        if (!property_status_ok())
            return;

        atoms.erase(
            std::remove(
                atoms.begin(),
                atoms.end(),
                atom
            ),
            atoms.end()
        );

        replace_atomlist_property(window, "_NET_WM_STATE", atoms);
    }
}

void
XConnection::set_window_frame_extents(winsys::Window window, winsys::Extents extents)
{
    std::vector<unsigned long> frame_extents = {
        static_cast<unsigned>(extents.left),
        static_cast<unsigned>(extents.right),
        static_cast<unsigned>(extents.top),
        static_cast<unsigned>(extents.bottom)
    };

    replace_cardlist_property(window, "_NET_FRAME_EXTENTS", frame_extents);
}

void
XConnection::set_desktop_geometry(std::vector<winsys::Region> const& geometries)
{
    std::vector<unsigned long> values;
    values.reserve(2 * geometries.size());

    for (auto& geometry : geometries) {
        values.push_back(geometry.dim.w);
        values.push_back(geometry.dim.h);
    }

    replace_cardlist_property(m_root, "_NET_DESKTOP_GEOMETRY", values);
}

void
XConnection::set_desktop_viewport(std::vector<winsys::Region> const& viewports)
{
    std::vector<unsigned long> values;
    values.reserve(2 * viewports.size());

    for (auto& viewport : viewports) {
        values.push_back(viewport.pos.x);
        values.push_back(viewport.pos.y);
    }

    replace_cardlist_property(m_root, "_NET_DESKTOP_VIEWPORT", values);
}

void
XConnection::set_workarea(std::vector<winsys::Region> const& workareas)
{
    std::vector<unsigned long> values;
    values.reserve(4 * workareas.size());

    for (auto& workarea : workareas) {
        values.push_back(workarea.pos.x);
        values.push_back(workarea.pos.y);
        values.push_back(workarea.dim.w);
        values.push_back(workarea.dim.h);
    }

    replace_cardlist_property(m_root, "_NET_WORKAREA", values);
}

void
XConnection::update_desktops(std::vector<std::string> const& desktop_names)
{
    replace_card_property(m_root, "_NET_NUMBER_OF_DESKTOPS", desktop_names.size());
    replace_stringlist_property(m_root, "_NET_DESKTOP_NAMES", desktop_names);
}

void
XConnection::update_client_list(std::vector<winsys::Window> const& clients)
{
    replace_windowlist_property(m_root, "_NET_CLIENT_LIST", clients);
}

void
XConnection::update_client_list_stacking(std::vector<winsys::Window> const& clients)
{
    replace_windowlist_property(m_root, "_NET_CLIENT_LIST_STACKING", clients);
}

std::optional<std::vector<std::optional<winsys::Strut>>>
XConnection::get_window_strut(winsys::Window window)
{
    std::optional<std::vector<std::optional<winsys::Strut>>> struts_partial
        = get_window_strut_partial(window);

    if (struts_partial)
        return struts_partial;

    std::vector<unsigned long> strut_widths = get_cardlist_property(window, "_NET_WM_STRUT");

    if (!property_status_ok() || strut_widths.empty())
        return std::nullopt;

    std::vector<std::optional<winsys::Strut>> struts;
    struts.reserve(4);

    for (std::size_t i = 0; i < strut_widths.size() && i < 4; ++i) {
        std::optional<winsys::Strut> strut = std::nullopt;

        if (strut_widths[i] > 0)
            strut = winsys::Strut {
                window,
                static_cast<int>(strut_widths[i])
            };

        struts.push_back(strut);
    }

    return struts;
}

std::optional<std::vector<std::optional<winsys::Strut>>>
XConnection::get_window_strut_partial(winsys::Window window)
{
    std::vector<unsigned long> strut_widths = get_cardlist_property(window, "_NET_WM_STRUT_PARTIAL");

    if (!property_status_ok() || strut_widths.empty())
        return std::nullopt;

    std::vector<std::optional<winsys::Strut>> struts_partial;
    struts_partial.reserve(4);

    for (std::size_t i = 0; i < strut_widths.size() && i < 4; ++i) {
        std::optional<winsys::Strut> strut = std::nullopt;

        if (strut_widths[i] > 0)
            strut = winsys::Strut {
                window,
                static_cast<int>(strut_widths[i])
            };

        struts_partial.push_back(strut);
    }

    return struts_partial;
}

std::optional<Index>
XConnection::get_window_desktop(winsys::Window window)
{
    Index index = get_card_property(window, "_NET_WM_DESKTOP");

    if (!property_status_ok())
        return std::nullopt;

    return index;
}

winsys::WindowType
XConnection::get_window_preferred_type(winsys::Window window)
{
    std::vector<winsys::WindowType> window_types = get_window_types(window);

    if (window_types.size() > 0)
        return window_types[0];

    return winsys::WindowType::Normal;
}

std::vector<winsys::WindowType>
XConnection::get_window_types(winsys::Window window)
{
    std::vector<Atom> window_type_atoms = get_atomlist_property(window, "_NET_WM_WINDOW_TYPE");

    if (!property_status_ok())
        return {};

    std::vector<winsys::WindowType> window_types = {};

    std::transform(
        window_type_atoms.begin(),
        window_type_atoms.end(),
        std::back_inserter(window_types),
        [=, this](Atom atom) -> winsys::WindowType {
            return get_window_type_from_atom(atom);
        }
    );

    return window_types;
}

std::optional<winsys::WindowState>
XConnection::get_window_preferred_state(winsys::Window window)
{
    std::vector<winsys::WindowState> window_states = get_window_states(window);

    if (window_states.size() > 0)
        return window_states[0];

    return std::nullopt;
}

std::vector<winsys::WindowState>
XConnection::get_window_states(winsys::Window window)
{
    std::vector<Atom> window_state_atoms = get_atomlist_property(window, "_NET_WM_STATE");

    if (!property_status_ok())
        return {};

    std::vector<winsys::WindowState> window_states = {};

    std::transform(
        window_state_atoms.begin(),
        window_state_atoms.end(),
        std::back_inserter(window_states),
        [=, this](Atom atom) -> winsys::WindowState {
            return get_window_state_from_atom(atom);
        }
    );

    return window_states;
}

bool
XConnection::window_is_fullscreen(winsys::Window window)
{
    static const std::vector<winsys::WindowState> fullscreen_state
        = { winsys::WindowState::Fullscreen };

    return window_is_any_of_states(window, fullscreen_state);
}

bool
XConnection::window_is_above(winsys::Window window)
{
    static const std::vector<winsys::WindowState> above_state
        = { winsys::WindowState::Above_ };

    return window_is_any_of_states(window, above_state);
}

bool
XConnection::window_is_below(winsys::Window window)
{
    static const std::vector<winsys::WindowState> below_state
        = { winsys::WindowState::Below_ };

    return window_is_any_of_states(window, below_state);
}

bool
XConnection::window_is_sticky(winsys::Window window)
{
    static const std::vector<winsys::WindowState> sticky_state
        = { winsys::WindowState::Sticky };

    return window_is_any_of_states(window, sticky_state);
}


void
XConnection::enable_substructure_events()
{
    if (--m_substructure_level != 0)
        return;

    if ((m_prev_root_mask & SubstructureNotifyMask) == 0)
        return;

    XSelectInput(mp_dpy, m_root, m_prev_root_mask | SubstructureNotifyMask);
    XFlush(mp_dpy);
}

void
XConnection::disable_substructure_events()
{
    if (++m_substructure_level != 1)
        return;

    if ((m_prev_root_mask & SubstructureNotifyMask) == 0)
        return;

    XSelectInput(mp_dpy, m_root, m_prev_root_mask & ~SubstructureNotifyMask);
    XFlush(mp_dpy);
}

void
XConnection::next_event(XEvent& event)
{
    XNextEvent(mp_dpy, &event);
}

bool
XConnection::typed_event(XEvent& event, int type)
{
    return XCheckTypedEvent(mp_dpy, type, &event);
}

void
XConnection::last_typed_event(XEvent& event, int type)
{
    while (typed_event(event, type));
}

void
XConnection::sync(bool discard)
{
    XSync(mp_dpy, discard);
}

int
XConnection::pending()
{
    return XPending(mp_dpy);
}

winsys::Window
XConnection::create_handle()
{
    winsys::Window window = XCreateWindow(
        mp_dpy, m_root,
        -2, -2,
        1, 1,
        CopyFromParent, 0,
        InputOnly,
        CopyFromParent, 0, NULL
    );

    flush();

    return window;
}


Atom
XConnection::get_atom(std::string const& name)
{
    if (m_interned_atoms.count(name) > 0)
        return m_interned_atoms.at(name);

    Atom atom = XInternAtom(mp_dpy, name.c_str(), False);
    intern_atom(name, atom);
    return atom;
}

Atom
XConnection::get_netwm_atom(NetWMID const& id)
{
    if (m_netwm_atoms.count(id) > 0)
        return m_netwm_atoms.at(id);

    return NetWMID::NetLast;
}

void
XConnection::intern_atom(std::string const& name, Atom atom)
{
    m_interned_atoms[name] = atom;
    m_atom_names[atom] = name;
}

winsys::Key
XConnection::get_key(const std::size_t keycode)
{
    if (m_keys.count(keycode) > 0)
        return m_keys.at(keycode);

    return winsys::Key::Any;
}

std::size_t
XConnection::get_keycode(const winsys::Key key)
{
    if (m_keycodes.count(key) > 0)
        return m_keycodes.at(key);

    unsigned keycode = 0;

    switch (key) {
    case winsys::Key::BackSpace:        keycode = XKeysymToKeycode(mp_dpy, XK_BackSpace);            break;
    case winsys::Key::Tab:              keycode = XKeysymToKeycode(mp_dpy, XK_Tab);                  break;
    case winsys::Key::Clear:            keycode = XKeysymToKeycode(mp_dpy, XK_Clear);                break;
    case winsys::Key::Return:           keycode = XKeysymToKeycode(mp_dpy, XK_Return);               break;
    case winsys::Key::Shift:            keycode = XKeysymToKeycode(mp_dpy, XK_Shift_L);              break;
    case winsys::Key::Control:          keycode = XKeysymToKeycode(mp_dpy, XK_Control_L);            break;
    case winsys::Key::Alt:              keycode = XKeysymToKeycode(mp_dpy, XK_Alt_L);                break;
    case winsys::Key::Super:            keycode = XKeysymToKeycode(mp_dpy, XK_Super_L);              break;
    case winsys::Key::Menu:             keycode = XKeysymToKeycode(mp_dpy, XK_Menu);                 break;
    case winsys::Key::Pause:            keycode = XKeysymToKeycode(mp_dpy, XK_Pause);                break;
    case winsys::Key::CapsLock:         keycode = XKeysymToKeycode(mp_dpy, XK_Caps_Lock);            break;
    case winsys::Key::Escape:           keycode = XKeysymToKeycode(mp_dpy, XK_Escape);               break;
    case winsys::Key::Space:            keycode = XKeysymToKeycode(mp_dpy, XK_space);                break;
    case winsys::Key::ExclamationMark:  keycode = XKeysymToKeycode(mp_dpy, XK_exclam);               break;
    case winsys::Key::QuotationMark:    keycode = XKeysymToKeycode(mp_dpy, XK_quotedbl);             break;
    case winsys::Key::QuestionMark:     keycode = XKeysymToKeycode(mp_dpy, XK_question);             break;
    case winsys::Key::NumberSign:       keycode = XKeysymToKeycode(mp_dpy, XK_numbersign);           break;
    case winsys::Key::DollarSign:       keycode = XKeysymToKeycode(mp_dpy, XK_dollar);               break;
    case winsys::Key::PercentSign:      keycode = XKeysymToKeycode(mp_dpy, XK_percent);              break;
    case winsys::Key::AtSign:           keycode = XKeysymToKeycode(mp_dpy, XK_at);                   break;
    case winsys::Key::Ampersand:        keycode = XKeysymToKeycode(mp_dpy, XK_ampersand);            break;
    case winsys::Key::Apostrophe:       keycode = XKeysymToKeycode(mp_dpy, XK_apostrophe);           break;
    case winsys::Key::LeftParenthesis:  keycode = XKeysymToKeycode(mp_dpy, XK_parenleft);            break;
    case winsys::Key::RightParenthesis: keycode = XKeysymToKeycode(mp_dpy, XK_parenright);           break;
    case winsys::Key::LeftBracket:      keycode = XKeysymToKeycode(mp_dpy, XK_bracketleft);          break;
    case winsys::Key::RightBracket:     keycode = XKeysymToKeycode(mp_dpy, XK_bracketright);         break;
    case winsys::Key::LeftBrace:        keycode = XKeysymToKeycode(mp_dpy, XK_braceleft);            break;
    case winsys::Key::RightBrace:       keycode = XKeysymToKeycode(mp_dpy, XK_braceright);           break;
    case winsys::Key::Underscore:       keycode = XKeysymToKeycode(mp_dpy, XK_underscore);           break;
    case winsys::Key::Grave:            keycode = XKeysymToKeycode(mp_dpy, XK_grave);                break;
    case winsys::Key::Bar:              keycode = XKeysymToKeycode(mp_dpy, XK_bar);                  break;
    case winsys::Key::Tilde:            keycode = XKeysymToKeycode(mp_dpy, XK_asciitilde);           break;
    case winsys::Key::QuoteLeft:        keycode = XKeysymToKeycode(mp_dpy, XK_quoteleft);            break;
    case winsys::Key::Asterisk:         keycode = XKeysymToKeycode(mp_dpy, XK_asterisk);             break;
    case winsys::Key::Plus:             keycode = XKeysymToKeycode(mp_dpy, XK_plus);                 break;
    case winsys::Key::Comma:            keycode = XKeysymToKeycode(mp_dpy, XK_comma);                break;
    case winsys::Key::Minus:            keycode = XKeysymToKeycode(mp_dpy, XK_minus);                break;
    case winsys::Key::Period:           keycode = XKeysymToKeycode(mp_dpy, XK_period);               break;
    case winsys::Key::Slash:            keycode = XKeysymToKeycode(mp_dpy, XK_slash);                break;
    case winsys::Key::BackSlash:        keycode = XKeysymToKeycode(mp_dpy, XK_backslash);            break;
    case winsys::Key::Colon:            keycode = XKeysymToKeycode(mp_dpy, XK_colon);                break;
    case winsys::Key::SemiColon:        keycode = XKeysymToKeycode(mp_dpy, XK_semicolon);            break;
    case winsys::Key::Less:             keycode = XKeysymToKeycode(mp_dpy, XK_less);                 break;
    case winsys::Key::Equal:            keycode = XKeysymToKeycode(mp_dpy, XK_equal);                break;
    case winsys::Key::Greater:          keycode = XKeysymToKeycode(mp_dpy, XK_greater);              break;
    case winsys::Key::PageUp:           keycode = XKeysymToKeycode(mp_dpy, XK_Prior);                break;
    case winsys::Key::PageDown:         keycode = XKeysymToKeycode(mp_dpy, XK_Next);                 break;
    case winsys::Key::End:              keycode = XKeysymToKeycode(mp_dpy, XK_End);                  break;
    case winsys::Key::Home:             keycode = XKeysymToKeycode(mp_dpy, XK_Home);                 break;
    case winsys::Key::Left:             keycode = XKeysymToKeycode(mp_dpy, XK_Left);                 break;
    case winsys::Key::Up:               keycode = XKeysymToKeycode(mp_dpy, XK_Up);                   break;
    case winsys::Key::Right:            keycode = XKeysymToKeycode(mp_dpy, XK_Right);                break;
    case winsys::Key::Down:             keycode = XKeysymToKeycode(mp_dpy, XK_Down);                 break;
    case winsys::Key::Select:           keycode = XKeysymToKeycode(mp_dpy, XK_Select);               break;
    case winsys::Key::Print:            keycode = XKeysymToKeycode(mp_dpy, XK_Print);                break;
    case winsys::Key::Execute:          keycode = XKeysymToKeycode(mp_dpy, XK_Execute);              break;
    case winsys::Key::PrintScreen:      keycode = XKeysymToKeycode(mp_dpy, XK_Print);                break;
    case winsys::Key::Insert:           keycode = XKeysymToKeycode(mp_dpy, XK_Insert);               break;
    case winsys::Key::Delete:           keycode = XKeysymToKeycode(mp_dpy, XK_Delete);               break;
    case winsys::Key::Help:             keycode = XKeysymToKeycode(mp_dpy, XK_Help);                 break;
    case winsys::Key::Zero:             keycode = XKeysymToKeycode(mp_dpy, XK_0);                    break;
    case winsys::Key::One:              keycode = XKeysymToKeycode(mp_dpy, XK_1);                    break;
    case winsys::Key::Two:              keycode = XKeysymToKeycode(mp_dpy, XK_2);                    break;
    case winsys::Key::Three:            keycode = XKeysymToKeycode(mp_dpy, XK_3);                    break;
    case winsys::Key::Four:             keycode = XKeysymToKeycode(mp_dpy, XK_4);                    break;
    case winsys::Key::Five:             keycode = XKeysymToKeycode(mp_dpy, XK_5);                    break;
    case winsys::Key::Six:              keycode = XKeysymToKeycode(mp_dpy, XK_6);                    break;
    case winsys::Key::Seven:            keycode = XKeysymToKeycode(mp_dpy, XK_7);                    break;
    case winsys::Key::Eight:            keycode = XKeysymToKeycode(mp_dpy, XK_8);                    break;
    case winsys::Key::Nine:             keycode = XKeysymToKeycode(mp_dpy, XK_9);                    break;
    case winsys::Key::A:                keycode = XKeysymToKeycode(mp_dpy, XK_a);                    break;
    case winsys::Key::B:                keycode = XKeysymToKeycode(mp_dpy, XK_b);                    break;
    case winsys::Key::C:                keycode = XKeysymToKeycode(mp_dpy, XK_c);                    break;
    case winsys::Key::D:                keycode = XKeysymToKeycode(mp_dpy, XK_d);                    break;
    case winsys::Key::E:                keycode = XKeysymToKeycode(mp_dpy, XK_e);                    break;
    case winsys::Key::F:                keycode = XKeysymToKeycode(mp_dpy, XK_f);                    break;
    case winsys::Key::G:                keycode = XKeysymToKeycode(mp_dpy, XK_g);                    break;
    case winsys::Key::H:                keycode = XKeysymToKeycode(mp_dpy, XK_h);                    break;
    case winsys::Key::I:                keycode = XKeysymToKeycode(mp_dpy, XK_i);                    break;
    case winsys::Key::J:                keycode = XKeysymToKeycode(mp_dpy, XK_j);                    break;
    case winsys::Key::K:                keycode = XKeysymToKeycode(mp_dpy, XK_k);                    break;
    case winsys::Key::L:                keycode = XKeysymToKeycode(mp_dpy, XK_l);                    break;
    case winsys::Key::M:                keycode = XKeysymToKeycode(mp_dpy, XK_m);                    break;
    case winsys::Key::N:                keycode = XKeysymToKeycode(mp_dpy, XK_n);                    break;
    case winsys::Key::O:                keycode = XKeysymToKeycode(mp_dpy, XK_o);                    break;
    case winsys::Key::P:                keycode = XKeysymToKeycode(mp_dpy, XK_p);                    break;
    case winsys::Key::Q:                keycode = XKeysymToKeycode(mp_dpy, XK_q);                    break;
    case winsys::Key::R:                keycode = XKeysymToKeycode(mp_dpy, XK_r);                    break;
    case winsys::Key::S:                keycode = XKeysymToKeycode(mp_dpy, XK_s);                    break;
    case winsys::Key::T:                keycode = XKeysymToKeycode(mp_dpy, XK_t);                    break;
    case winsys::Key::U:                keycode = XKeysymToKeycode(mp_dpy, XK_u);                    break;
    case winsys::Key::V:                keycode = XKeysymToKeycode(mp_dpy, XK_v);                    break;
    case winsys::Key::W:                keycode = XKeysymToKeycode(mp_dpy, XK_w);                    break;
    case winsys::Key::X:                keycode = XKeysymToKeycode(mp_dpy, XK_x);                    break;
    case winsys::Key::Y:                keycode = XKeysymToKeycode(mp_dpy, XK_y);                    break;
    case winsys::Key::Z:                keycode = XKeysymToKeycode(mp_dpy, XK_z);                    break;
    case winsys::Key::NumPad0:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_0);                 break;
    case winsys::Key::NumPad1:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_1);                 break;
    case winsys::Key::NumPad2:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_2);                 break;
    case winsys::Key::NumPad3:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_3);                 break;
    case winsys::Key::NumPad4:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_4);                 break;
    case winsys::Key::NumPad5:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_5);                 break;
    case winsys::Key::NumPad6:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_6);                 break;
    case winsys::Key::NumPad7:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_7);                 break;
    case winsys::Key::NumPad8:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_8);                 break;
    case winsys::Key::NumPad9:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_9);                 break;
    case winsys::Key::Multiply:         keycode = XKeysymToKeycode(mp_dpy, XK_KP_Multiply);          break;
    case winsys::Key::Add:              keycode = XKeysymToKeycode(mp_dpy, XK_KP_Add);               break;
    case winsys::Key::Seperator:        keycode = XKeysymToKeycode(mp_dpy, XK_KP_Separator);         break;
    case winsys::Key::Subtract:         keycode = XKeysymToKeycode(mp_dpy, XK_KP_Subtract);          break;
    case winsys::Key::Decimal:          keycode = XKeysymToKeycode(mp_dpy, XK_KP_Decimal);           break;
    case winsys::Key::Divide:           keycode = XKeysymToKeycode(mp_dpy, XK_KP_Divide);            break;
    case winsys::Key::F1:               keycode = XKeysymToKeycode(mp_dpy, XK_F1);                   break;
    case winsys::Key::F2:               keycode = XKeysymToKeycode(mp_dpy, XK_F2);                   break;
    case winsys::Key::F3:               keycode = XKeysymToKeycode(mp_dpy, XK_F3);                   break;
    case winsys::Key::F4:               keycode = XKeysymToKeycode(mp_dpy, XK_F4);                   break;
    case winsys::Key::F5:               keycode = XKeysymToKeycode(mp_dpy, XK_F5);                   break;
    case winsys::Key::F6:               keycode = XKeysymToKeycode(mp_dpy, XK_F6);                   break;
    case winsys::Key::F7:               keycode = XKeysymToKeycode(mp_dpy, XK_F7);                   break;
    case winsys::Key::F8:               keycode = XKeysymToKeycode(mp_dpy, XK_F8);                   break;
    case winsys::Key::F9:               keycode = XKeysymToKeycode(mp_dpy, XK_F9);                   break;
    case winsys::Key::F10:              keycode = XKeysymToKeycode(mp_dpy, XK_F10);                  break;
    case winsys::Key::F11:              keycode = XKeysymToKeycode(mp_dpy, XK_F11);                  break;
    case winsys::Key::F12:              keycode = XKeysymToKeycode(mp_dpy, XK_F12);                  break;
    case winsys::Key::F13:              keycode = XKeysymToKeycode(mp_dpy, XK_F13);                  break;
    case winsys::Key::F14:              keycode = XKeysymToKeycode(mp_dpy, XK_F14);                  break;
    case winsys::Key::F15:              keycode = XKeysymToKeycode(mp_dpy, XK_F15);                  break;
    case winsys::Key::F16:              keycode = XKeysymToKeycode(mp_dpy, XK_F16);                  break;
    case winsys::Key::F17:              keycode = XKeysymToKeycode(mp_dpy, XK_F17);                  break;
    case winsys::Key::F18:              keycode = XKeysymToKeycode(mp_dpy, XK_F18);                  break;
    case winsys::Key::F19:              keycode = XKeysymToKeycode(mp_dpy, XK_F19);                  break;
    case winsys::Key::F20:              keycode = XKeysymToKeycode(mp_dpy, XK_F20);                  break;
    case winsys::Key::F21:              keycode = XKeysymToKeycode(mp_dpy, XK_F21);                  break;
    case winsys::Key::F22:              keycode = XKeysymToKeycode(mp_dpy, XK_F22);                  break;
    case winsys::Key::F23:              keycode = XKeysymToKeycode(mp_dpy, XK_F23);                  break;
    case winsys::Key::F24:              keycode = XKeysymToKeycode(mp_dpy, XK_F24);                  break;
    case winsys::Key::Numlock:          keycode = XKeysymToKeycode(mp_dpy, XK_Num_Lock);             break;
    case winsys::Key::ScrollLock:       keycode = XKeysymToKeycode(mp_dpy, XK_Scroll_Lock);          break;
    case winsys::Key::LeftShift:        keycode = XKeysymToKeycode(mp_dpy, XK_Shift_L);              break;
    case winsys::Key::RightShift:       keycode = XKeysymToKeycode(mp_dpy, XK_Shift_R);              break;
    case winsys::Key::LeftControl:      keycode = XKeysymToKeycode(mp_dpy, XK_Control_L);            break;
    case winsys::Key::RightContol:      keycode = XKeysymToKeycode(mp_dpy, XK_Control_R);            break;
    case winsys::Key::LeftAlt:          keycode = XKeysymToKeycode(mp_dpy, XK_Alt_L);                break;
    case winsys::Key::RightAlt:         keycode = XKeysymToKeycode(mp_dpy, XK_Alt_R);                break;
    case winsys::Key::LeftSuper:        keycode = XKeysymToKeycode(mp_dpy, XK_Super_L);              break;
    case winsys::Key::RightSuper:       keycode = XKeysymToKeycode(mp_dpy, XK_Super_R);              break;
    case winsys::Key::BrowserBack:      keycode = XKeysymToKeycode(mp_dpy, XF86XK_Back);             break;
    case winsys::Key::BrowserForward:   keycode = XKeysymToKeycode(mp_dpy, XF86XK_Forward);          break;
    case winsys::Key::BrowserRefresh:   keycode = XKeysymToKeycode(mp_dpy, XF86XK_Refresh);          break;
    case winsys::Key::BrowserStop:      keycode = XKeysymToKeycode(mp_dpy, XF86XK_Close);            break;
    case winsys::Key::BrowserSearch:    keycode = XKeysymToKeycode(mp_dpy, XF86XK_Search);           break;
    case winsys::Key::BrowserFavorites: keycode = XKeysymToKeycode(mp_dpy, XF86XK_Favorites);        break;
    case winsys::Key::BrowserHome:      keycode = XKeysymToKeycode(mp_dpy, XF86XK_HomePage);         break;
    case winsys::Key::VolumeMute:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_AudioMute);        break;
    case winsys::Key::VolumeDown:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_AudioLowerVolume); break;
    case winsys::Key::VolumeUp:         keycode = XKeysymToKeycode(mp_dpy, XF86XK_AudioRaiseVolume); break;
    case winsys::Key::MicMute:          keycode = XKeysymToKeycode(mp_dpy, XF86XK_AudioMicMute);     break;
    case winsys::Key::NextTrack:        keycode = XKeysymToKeycode(mp_dpy, XF86XK_AudioNext);        break;
    case winsys::Key::PreviousTrack:    keycode = XKeysymToKeycode(mp_dpy, XF86XK_AudioPrev);        break;
    case winsys::Key::StopMedia:        keycode = XKeysymToKeycode(mp_dpy, XF86XK_AudioStop);        break;
    case winsys::Key::PlayPause:        keycode = XKeysymToKeycode(mp_dpy, XF86XK_AudioPlay);        break;
    case winsys::Key::LaunchMail:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Mail);             break;
    case winsys::Key::SelectMedia:      keycode = XKeysymToKeycode(mp_dpy, XF86XK_AudioMedia);       break;
    case winsys::Key::LaunchAppA:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_LaunchA);          break;
    case winsys::Key::LaunchAppB:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_LaunchB);          break;
    case winsys::Key::LaunchAppC:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_LaunchC);          break;
    case winsys::Key::LaunchAppD:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_LaunchD);          break;
    case winsys::Key::LaunchAppE:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_LaunchE);          break;
    case winsys::Key::LaunchAppF:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_LaunchF);          break;
    case winsys::Key::LaunchApp0:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch0);          break;
    case winsys::Key::LaunchApp1:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch1);          break;
    case winsys::Key::LaunchApp2:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch2);          break;
    case winsys::Key::LaunchApp3:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch3);          break;
    case winsys::Key::LaunchApp4:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch4);          break;
    case winsys::Key::LaunchApp5:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch5);          break;
    case winsys::Key::LaunchApp6:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch6);          break;
    case winsys::Key::LaunchApp7:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch7);          break;
    case winsys::Key::LaunchApp8:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch8);          break;
    case winsys::Key::LaunchApp9:       keycode = XKeysymToKeycode(mp_dpy, XF86XK_Launch9);          break;
    case winsys::Key::BrightnessDown:   keycode = XKeysymToKeycode(mp_dpy, XKB_KEY_XF86MonBrightnessDown); break;
    case winsys::Key::BrightnessUp:     keycode = XKeysymToKeycode(mp_dpy, XKB_KEY_XF86MonBrightnessDown); break;
    case winsys::Key::KeyboardBrightnessDown: keycode = XKeysymToKeycode(mp_dpy, XKB_KEY_XF86KbdBrightnessDown); break;
    case winsys::Key::KeyboardBrightnessUp:   keycode = XKeysymToKeycode(mp_dpy, XKB_KEY_XF86KbdBrightnessDown); break;
    case winsys::Key::Any: return 0;
    }

    intern_key(keycode, key);
    return keycode;
}

void
XConnection::intern_key(const unsigned keycode, const winsys::Key key)
{
    m_keys[keycode] = key;
    m_keycodes[key] = keycode;
}

winsys::Button
XConnection::get_button(const unsigned buttoncode) const
{
    switch (buttoncode) {
    case 1: return winsys::Button::Left;
    case 2: return winsys::Button::Middle;
    case 3: return winsys::Button::Right;
    case 4: return winsys::Button::ScrollUp;
    case 5: return winsys::Button::ScrollDown;
    case 6: return winsys::Button::Backward;
    case 7: return winsys::Button::Forward;
    default: return winsys::Button::Left;
    }
}

unsigned
XConnection::get_buttoncode(const winsys::Button button) const
{
    switch (button) {
    case winsys::Button::Left:       return 1;
    case winsys::Button::Middle:     return 2;
    case winsys::Button::Right:      return 3;
    case winsys::Button::ScrollUp:   return 4;
    case winsys::Button::ScrollDown: return 5;
    case winsys::Button::Backward:   return 6;
    case winsys::Button::Forward:    return 7;
    default: return 0;
    }
}

winsys::WindowState
XConnection::get_window_state_from_atom(Atom atom)
{
    if (atom == get_atom("_NET_WM_STATE_MODAL"))             return winsys::WindowState::Modal;
    if (atom == get_atom("_NET_WM_STATE_STICKY"))            return winsys::WindowState::Sticky;
    if (atom == get_atom("_NET_WM_STATE_MAXIMIZEDVERT"))     return winsys::WindowState::MaximizedVert;
    if (atom == get_atom("_NET_WM_STATE_MAXIMIZEDHORZ"))     return winsys::WindowState::MaximizedHorz;
    if (atom == get_atom("_NET_WM_STATE_SHADED"))            return winsys::WindowState::Shaded;
    if (atom == get_atom("_NET_WM_STATE_SKIPTASKBAR"))       return winsys::WindowState::SkipTaskbar;
    if (atom == get_atom("_NET_WM_STATE_SKIPPAGER"))         return winsys::WindowState::SkipPager;
    if (atom == get_atom("_NET_WM_STATE_HIDDEN"))            return winsys::WindowState::Hidden;
    if (atom == get_atom("_NET_WM_STATE_FULLSCREEN"))        return winsys::WindowState::Fullscreen;
    if (atom == get_atom("_NET_WM_STATE_ABOVE"))             return winsys::WindowState::Above_;
    if (atom == get_atom("_NET_WM_STATE_BELOW"))             return winsys::WindowState::Below_;
    if (atom == get_atom("_NET_WM_STATE_DEMANDS_ATTENTION")) return winsys::WindowState::DemandsAttention;
    return winsys::WindowState::Hidden;
}

winsys::WindowType
XConnection::get_window_type_from_atom(Atom atom)
{
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_DESKTOP"))      return winsys::WindowType::Desktop;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_DOCK"))         return winsys::WindowType::Dock;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_TOOLBAR"))      return winsys::WindowType::Toolbar;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_MENU"))         return winsys::WindowType::Menu;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_UTILITY"))      return winsys::WindowType::Utility;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_SPLASH"))       return winsys::WindowType::Splash;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_DIALOG"))       return winsys::WindowType::Dialog;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_DROPDOWNMENU")) return winsys::WindowType::DropdownMenu;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_POPUPMENU"))    return winsys::WindowType::PopupMenu;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_TOOLTIP"))      return winsys::WindowType::Tooltip;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_NOTIFICATION")) return winsys::WindowType::Notification;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_COMBO"))        return winsys::WindowType::Combo;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_DND"))          return winsys::WindowType::Dnd;
    if (atom == get_atom("_NET_WM_WINDOW_TYPE_NORMAL"))       return winsys::WindowType::Normal;
    return winsys::WindowType::Normal;
}

Atom
XConnection::get_atom_from_window_state(winsys::WindowState state)
{
    switch (state) {
    case winsys::WindowState::Modal:            return get_atom("_NET_WM_STATE_MODAL");
    case winsys::WindowState::Sticky:           return get_atom("_NET_WM_STATE_STICKY");
    case winsys::WindowState::MaximizedVert:    return get_atom("_NET_WM_STATE_MAXIMIZEDVERT");
    case winsys::WindowState::MaximizedHorz:    return get_atom("_NET_WM_STATE_MAXIMIZEDHORZ");
    case winsys::WindowState::Shaded:           return get_atom("_NET_WM_STATE_SHADED");
    case winsys::WindowState::SkipTaskbar:      return get_atom("_NET_WM_STATE_SKIPTASKBAR");
    case winsys::WindowState::SkipPager:        return get_atom("_NET_WM_STATE_SKIPPAGER");
    case winsys::WindowState::Hidden:           return get_atom("_NET_WM_STATE_HIDDEN");
    case winsys::WindowState::Fullscreen:       return get_atom("_NET_WM_STATE_FULLSCREEN");
    case winsys::WindowState::Above_:           return get_atom("_NET_WM_STATE_ABOVE");
    case winsys::WindowState::Below_:           return get_atom("_NET_WM_STATE_BELOW");
    case winsys::WindowState::DemandsAttention: return get_atom("_NET_WM_STATE_DEMANDS_ATTENTION");
    default: return 0;
    }
}

Atom
XConnection::get_atom_from_window_type(winsys::WindowType type)
{
    switch (type) {
    case winsys::WindowType::Desktop:      return get_atom("_NET_WM_WINDOW_TYPE_DESKTOP");
    case winsys::WindowType::Dock:         return get_atom("_NET_WM_WINDOW_TYPE_DOCK");
    case winsys::WindowType::Toolbar:      return get_atom("_NET_WM_WINDOW_TYPE_TOOLBAR");
    case winsys::WindowType::Menu:         return get_atom("_NET_WM_WINDOW_TYPE_MENU");
    case winsys::WindowType::Utility:      return get_atom("_NET_WM_WINDOW_TYPE_UTILITY");
    case winsys::WindowType::Splash:       return get_atom("_NET_WM_WINDOW_TYPE_SPLASH");
    case winsys::WindowType::Dialog:       return get_atom("_NET_WM_WINDOW_TYPE_DIALOG");
    case winsys::WindowType::DropdownMenu: return get_atom("_NET_WM_WINDOW_TYPE_DROPDOWNMENU");
    case winsys::WindowType::PopupMenu:    return get_atom("_NET_WM_WINDOW_TYPE_POPUPMENU");
    case winsys::WindowType::Tooltip:      return get_atom("_NET_WM_WINDOW_TYPE_TOOLTIP");
    case winsys::WindowType::Notification: return get_atom("_NET_WM_WINDOW_TYPE_NOTIFICATION");
    case winsys::WindowType::Combo:        return get_atom("_NET_WM_WINDOW_TYPE_COMBO");
    case winsys::WindowType::Dnd:          return get_atom("_NET_WM_WINDOW_TYPE_DND");
    case winsys::WindowType::Normal:       return get_atom("_NET_WM_WINDOW_TYPE_NORMAL");
    default: return get_atom("_NET_WM_WINDOW_TYPE_NORMAL");
    }
}

bool
XConnection::property_status_ok()
{
    bool status_ok = m_property_status == 0;
    m_property_status = 0;

    return status_ok;
}

bool
XConnection::has_atom_property(winsys::Window window, Atom atom)
{
    int _i;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom returned_type = None;
    unsigned long n_items_returned = 0;

    return (XGetWindowProperty(mp_dpy, window, atom,
        0L, 32, False, XA_ATOM,
        &returned_type, &_i, &n_items_returned,
        &_ul, &ucp) == Success
        && ucp && XFree(ucp)
        && returned_type == XA_ATOM
        && n_items_returned > 0
    );
}

Atom
XConnection::get_atom_property(winsys::Window window, std::string const& name)
{
    int _i;
    unsigned long n;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom _a = None;
    Atom atom = None;

    m_property_status = XGetWindowProperty(
        mp_dpy,
        window,
        get_atom(name),
        0L, 32, False,
        XA_ATOM,
        &_a, &_i, &n, &_ul,
        &ucp
    );

	if (m_property_status == Success && ucp) {
		atom = *(Atom*)ucp;
		XFree(ucp);
	}

	return atom;
}

void
XConnection::replace_atom_property(winsys::Window window, std::string const& name, Atom atom)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_ATOM,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(&atom),
        1
    );
}

void
XConnection::unset_atom_property(winsys::Window window, std::string const& name)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_ATOM,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(0),
        0
    );
}

bool
XConnection::has_atomlist_property(winsys::Window window, Atom atom)
{
    int _i;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom returned_type = None;
    unsigned long n_items_returned = 0;

    return (XGetWindowProperty(mp_dpy, window, atom,
        0L, 32, False, XA_ATOM,
        &returned_type, &_i, &n_items_returned,
        &_ul, &ucp) == Success
        && ucp && XFree(ucp)
        && returned_type == XA_ATOM
        && n_items_returned > 0
    );
}

std::vector<Atom>
XConnection::get_atomlist_property(winsys::Window window, std::string const& name)
{
    int _i;
    unsigned long n;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom _a = None;
    std::vector<Atom> atomlist = {};

    m_property_status = XGetWindowProperty(
        mp_dpy,
        window,
        get_atom(name),
        0L, 32, False,
        XA_ATOM,
        &_a, &_i, &n, &_ul,
        &ucp
    );

	if (m_property_status == Success && ucp) {
        for (std::size_t i = 0; i < n; ++i)
            atomlist.push_back(((Atom*)ucp)[i]);

		XFree(ucp);
	}

	return atomlist;
}

void
XConnection::replace_atomlist_property(winsys::Window window, std::string const& name, std::vector<Atom> const& atomlist)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_ATOM,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(atomlist.data()),
        atomlist.size()
    );
}

void
XConnection::append_atomlist_property(winsys::Window window, std::string const& name, Atom atom)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_ATOM,
        32,
        PropModeAppend,
        reinterpret_cast<const unsigned char*>(&atom),
        1
    );
}

void
XConnection::unset_atomlist_property(winsys::Window window, std::string const& name)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_ATOM,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(0),
        0
    );
}

bool
XConnection::has_window_property(winsys::Window window, Atom atom)
{
    int _i;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom returned_type = None;
    unsigned long n_items_returned = 0;

    return (XGetWindowProperty(mp_dpy, window, atom,
        0L, 32, False, XA_WINDOW,
        &returned_type, &_i, &n_items_returned,
        &_ul, &ucp) == Success
        && ucp && XFree(ucp)
        && returned_type == XA_WINDOW
        && n_items_returned > 0
    );
}

winsys::Window
XConnection::get_window_property(winsys::Window window, std::string const& name)
{
    int _i;
    unsigned long n;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom _a = None;
    winsys::Window window_ = None;

    m_property_status = XGetWindowProperty(
        mp_dpy,
        window,
        get_atom(name),
        0L, 32, False,
        XA_WINDOW,
        &_a, &_i, &n, &_ul,
        &ucp
    );

	if (m_property_status == Success && ucp) {
		window_ = *(winsys::Window*)ucp;
		XFree(ucp);
	}

	return window_;
}

void
XConnection::replace_window_property(winsys::Window window, std::string const& name, winsys::Window window_)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_WINDOW,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(&window_),
        1
    );
}

void
XConnection::unset_window_property(winsys::Window window, std::string const& name)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_WINDOW,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(0),
        0
    );
}

bool
XConnection::has_windowlist_property(winsys::Window window, Atom atom)
{
    int _i;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom returned_type = None;
    unsigned long n_items_returned = 0;

    return (XGetWindowProperty(mp_dpy, window, atom,
        0L, 32, False, XA_WINDOW,
        &returned_type, &_i, &n_items_returned,
        &_ul, &ucp) == Success
        && ucp && XFree(ucp)
        && returned_type == XA_ATOM
        && n_items_returned > 0
    );
}

std::vector<winsys::Window>
XConnection::get_windowlist_property(winsys::Window window, std::string const& name)
{
    int _i;
    unsigned long n;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom _a = None;
    std::vector<winsys::Window> windowlist = {};

    m_property_status = XGetWindowProperty(
        mp_dpy,
        window,
        get_atom(name),
        0L, 32, False,
        XA_WINDOW,
        &_a, &_i, &n, &_ul,
        &ucp
    );

	if (m_property_status == Success && ucp) {
        for (std::size_t i = 0; i < n; ++i)
            windowlist.push_back(((winsys::Window*)ucp)[i]);

		XFree(ucp);
	}

	return windowlist;
}

void
XConnection::replace_windowlist_property(winsys::Window window, std::string const& name, std::vector<winsys::Window> const& windowlist)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_WINDOW,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(windowlist.data()),
        windowlist.size()
    );
}

void
XConnection::append_windowlist_property(winsys::Window window, std::string const& name, winsys::Window window_)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_WINDOW,
        32,
        PropModeAppend,
        reinterpret_cast<const unsigned char*>(&window_),
        1
    );
}

void
XConnection::unset_windowlist_property(winsys::Window window, std::string const& name)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_WINDOW,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(0),
        0
    );
}

bool
XConnection::has_string_property(winsys::Window window, Atom atom)
{
    int _i;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom returned_type = None;
    unsigned long n_items_returned = 0;

    return (XGetWindowProperty(mp_dpy, window, atom,
        0L, 8, False, get_atom("UTF8_STRING"),
        &returned_type, &_i, &n_items_returned,
        &_ul, &ucp) == Success
        && ucp && XFree(ucp)
        && returned_type == get_atom("UTF8_STRING")
        && n_items_returned > 0
    );
}

std::string
XConnection::get_string_property(winsys::Window window, std::string const& name)
{
    int _i;
    unsigned long n;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom _a = None;
    std::string str_ = {};

    m_property_status = XGetWindowProperty(
        mp_dpy,
        window,
        get_atom(name),
        0L, 8, False,
        get_atom("UTF8_STRING"),
        &_a, &_i, &n, &_ul,
        &ucp
    );

	if (m_property_status == Success && ucp) {
		str_ = std::string((char*)ucp);
		XFree(ucp);
	}

	return str_;
}

void
XConnection::replace_string_property(winsys::Window window, std::string const& name, std::string const& str_)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        get_atom("UTF8_STRING"),
        8,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(str_.c_str()),
        str_.length()
    );
}

void
XConnection::unset_string_property(winsys::Window window, std::string const& name)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        get_atom("UTF8_STRING"),
        8,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(0),
        0
    );
}

bool
XConnection::has_stringlist_property(winsys::Window window, Atom atom)
{
    int _i;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom returned_type = None;
    unsigned long n_items_returned = 0;

    return (XGetWindowProperty(mp_dpy, window, atom,
        0L, 8, False, get_atom("UTF8_STRING"),
        &returned_type, &_i, &n_items_returned,
        &_ul, &ucp) == Success
        && ucp && XFree(ucp)
        && returned_type == get_atom("UTF8_STRING")
        && n_items_returned > 0
    );
}

std::vector<std::string>
XConnection::get_stringlist_property(winsys::Window window, std::string const& name)
{
    int _i;
    unsigned long n;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom _a = None;
    std::vector<std::string> stringlist = {};

    m_property_status = XGetWindowProperty(
        mp_dpy,
        window,
        get_atom(name),
        0L, 8, False,
        get_atom("UTF8_STRING"),
        &_a, &_i, &n, &_ul,
        &ucp
    );

	if (m_property_status == Success && ucp) {
        for (std::size_t i = 0; i < n; ++i)
            stringlist.emplace_back(std::string(((char**)ucp)[i]));

		XFree(ucp);
	}

	return stringlist;
}

void
XConnection::replace_stringlist_property(winsys::Window window, std::string const& name, std::vector<std::string> const& stringlist)
{
    static char buffer[1024];

    std::size_t i = 0;
    for (auto& str_ : stringlist) {
        if (i == 1023)
            goto append;

        for (auto& c : str_) {
            if (i == 1023)
                goto append;

            buffer[i++] = c;
        }

        buffer[i++] = '\0';
    }

append:
    buffer[1023] = '\0';

    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        get_atom("UTF8_STRING"),
        8,
        PropModeAppend,
        reinterpret_cast<const unsigned char*>(buffer),
        i
    );
}

void
XConnection::append_stringlist_property(winsys::Window window, std::string const& name, std::string const& str_)
{
    static char buffer[1024];

    std::size_t i = 0;
    for (auto& c : str_) {
        if (i == 1023)
            break;

        buffer[i++] = c;
    }

    buffer[1023] = '\0';

    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        get_atom("UTF8_STRING"),
        8,
        PropModeAppend,
        reinterpret_cast<const unsigned char*>(buffer),
        i
    );
}

void
XConnection::unset_stringlist_property(winsys::Window window, std::string const& name)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        get_atom("UTF8_STRING"),
        8,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(0),
        0
    );
}

bool
XConnection::has_card_property(winsys::Window window, Atom atom)
{
    int _i;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom returned_type = None;
    unsigned long n_items_returned = 0;

    return (XGetWindowProperty(mp_dpy, window, atom,
        0L, 32, False, XA_CARDINAL,
        &returned_type, &_i, &n_items_returned,
        &_ul, &ucp) == Success
        && ucp && XFree(ucp)
        && returned_type == XA_CARDINAL
        && n_items_returned > 0
    );
}

unsigned long
XConnection::get_card_property(winsys::Window window, std::string const& name)
{
    int _i;
    unsigned long n;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom _a = None;
    unsigned long card = None;

    m_property_status = XGetWindowProperty(
        mp_dpy,
        window,
        get_atom(name),
        0L, 32, False,
        XA_CARDINAL,
        &_a, &_i, &n, &_ul,
        &ucp
    );

	if (m_property_status == Success && ucp) {
		card = *(winsys::Window*)ucp;
		XFree(ucp);
	}

	return card;
}

void
XConnection::replace_card_property(winsys::Window window, std::string const& name, unsigned long card)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_CARDINAL,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(&card),
        1
    );
}

void
XConnection::unset_card_property(winsys::Window window, std::string const& name)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_CARDINAL,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(0),
        0
    );
}

std::vector<unsigned long>
XConnection::get_cardlist_property(winsys::Window window, std::string const& name)
{
    int _i;
    unsigned long n;
    unsigned long _ul;
    unsigned char* ucp = nullptr;
    Atom _a = None;
    std::vector<unsigned long> cardlist = {};

    m_property_status = XGetWindowProperty(
        mp_dpy,
        window,
        get_atom(name),
        0L, 32, False,
        XA_CARDINAL,
        &_a, &_i, &n, &_ul,
        &ucp
    );

	if (m_property_status == Success && ucp) {
        for (std::size_t i = 0; i < n; ++i)
            cardlist.push_back(((unsigned long*)ucp)[i]);

		XFree(ucp);
	}

	return cardlist;
}

void
XConnection::replace_cardlist_property(winsys::Window window, std::string const& name, std::vector<unsigned long> const& cardlist)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_CARDINAL,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(cardlist.data()),
        cardlist.size()
    );
}

void
XConnection::append_cardlist_property(winsys::Window window, std::string const& name, unsigned long card)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_CARDINAL,
        32,
        PropModeAppend,
        reinterpret_cast<const unsigned char*>(&card),
        1
    );
}

void
XConnection::unset_cardlist_property(winsys::Window window, std::string const& name)
{
    XChangeProperty(
        mp_dpy,
        window,
        get_atom(name),
        XA_CARDINAL,
        32,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(0),
        0
    );
}

bool
XConnection::get_text_property(winsys::Window window, Atom atom, char* text, unsigned size)
{
    char** list = NULL;
    int n;
    XTextProperty name;

    if (!text || size == 0)
        return false;

    text[0] = '\0';

    if (!XGetTextProperty(mp_dpy, window, &name, atom) || !name.nitems)
        return false;

    if (name.encoding == XA_STRING)
        std::strncpy(text, (char*)name.value, size - 1);
    else if (XmbTextPropertyToTextList(mp_dpy, &name, &list, &n)
        >= Success && n > 0 && *list
    ) {
        std::strncpy(text, *list, size - 1);
        XFreeStringList(list);
    }

    text[size - 1] = '\0';
    XFree(name.value);
    return true;
}

bool
XConnection::window_is_any_of_states(winsys::Window window, std::vector<winsys::WindowState> const& free_states)
{
    std::vector<Atom> window_state_atoms
        = get_atomlist_property(window, "_NET_WM_STATE");

    if (!property_status_ok())
        return false;

    std::vector<Atom> free_state_atoms;
    free_state_atoms.reserve(free_states.size());

    std::transform(
        free_states.begin(),
        free_states.end(),
        std::back_inserter(free_state_atoms),
        [=, this](winsys::WindowState state) -> Atom {
            return get_atom_from_window_state(state);
        }
    );

    std::sort(window_state_atoms.begin(), window_state_atoms.end());
    std::sort(free_state_atoms.begin(), free_state_atoms.end());

    std::vector<Atom> found_atoms;
    std::set_intersection(
        window_state_atoms.begin(),
        window_state_atoms.end(),
        free_state_atoms.begin(),
        free_state_atoms.end(),
        std::back_inserter(found_atoms)
    );

    return found_atoms.size() > 0;
}

bool
XConnection::window_is_any_of_types(winsys::Window window, std::vector<winsys::WindowType> const& free_types)
{
    std::vector<Atom> window_type_atoms
        = get_atomlist_property(window, "_NET_WM_WINDOW_TYPE");

    if (!property_status_ok())
        return false;

    std::vector<Atom> free_type_atoms;
    free_type_atoms.reserve(free_types.size());

    std::transform(
        free_types.begin(),
        free_types.end(),
        std::back_inserter(free_type_atoms),
        [=, this](winsys::WindowType type) -> Atom {
            return get_atom_from_window_type(type);
        }
    );

    std::sort(window_type_atoms.begin(), window_type_atoms.end());
    std::sort(free_type_atoms.begin(), free_type_atoms.end());

    std::vector<Atom> found_atoms;
    std::set_intersection(window_type_atoms.begin(), window_type_atoms.end(),
        free_type_atoms.begin(), free_type_atoms.end(), std::back_inserter(found_atoms));

    return found_atoms.size() > 0;
}

void
XConnection::check_otherwm()
{
    XSetErrorHandler(XConnection::s_otherwm_error_handler);
    XSelectInput(mp_dpy, m_root, SubstructureRedirectMask);
    sync(false);
    XSetErrorHandler(s_default_error_handler);
    sync(false);
}


winsys::Event
XConnection::on_button_press()
{
    XButtonEvent event = m_current_event.xbutton;
    winsys::Window window = event.window;
    winsys::Window subwindow = event.subwindow;

    bool on_root = false;
    if (window == m_root && subwindow == None)
        on_root = true;

    if (window == m_root || window == None)
        window = subwindow;

    winsys::Button button;
    switch (event.button) {
    case 1: button = winsys::Button::Left;       break;
    case 2: button = winsys::Button::Middle;     break;
    case 3: button = winsys::Button::Right;      break;
    case 4: button = winsys::Button::ScrollUp;   break;
    case 5: button = winsys::Button::ScrollDown; break;
    case 6: button = winsys::Button::Backward;   break;
    case 7: button = winsys::Button::Forward;    break;
    default: return std::monostate{};
    }

    std::size_t x11_modifiers[] = {
        ControlMask,
        ShiftMask,
        Mod1Mask,
        Mod4Mask,
        Mod2Mask,
        Mod5Mask
    };

    auto x11_to_modifier = [](std::size_t x11_modifier) -> winsys::Modifier {
        switch (x11_modifier) {
        case ControlMask: return winsys::Modifier::Ctrl;
        case ShiftMask:   return winsys::Modifier::Shift;
        case Mod1Mask:    return winsys::Modifier::Alt;
        case Mod4Mask:    return winsys::Modifier::Super;
        case Mod2Mask:    return winsys::Modifier::NumLock;
        case Mod5Mask:    return winsys::Modifier::ScrollLock;
        default: return static_cast<winsys::Modifier>(0);
        }
    };

    std::unordered_set<winsys::Modifier> modifiers = {};
    for (auto& x11_modifier : x11_modifiers)
        if ((event.state & x11_modifier) > 0)
            modifiers.insert(x11_to_modifier(x11_modifier));

    return winsys::MouseEvent {
        {
            winsys::MouseCapture::MouseCaptureKind::Press,
            winsys::MouseInput {
                winsys::MouseInput::MouseInputTarget::Global,
                button,
                modifiers
            },
            window,
            winsys::Pos {
                event.x_root,
                event.y_root
            }
        },
        on_root
    };
}

winsys::Event
XConnection::on_button_release()
{
    XButtonEvent event = m_current_event.xbutton;
    winsys::Window window = event.window;
    winsys::Window subwindow = event.subwindow;

    bool on_root = false;
    if (window == m_root && subwindow == None)
        on_root = true;

    if (window == m_root || window == None)
        window = subwindow;

    winsys::Button button;
    switch (event.button) {
    case 1: button = winsys::Button::Left;       break;
    case 2: button = winsys::Button::Middle;     break;
    case 3: button = winsys::Button::Right;      break;
    case 4: button = winsys::Button::ScrollUp;   break;
    case 5: button = winsys::Button::ScrollDown; break;
    case 6: button = winsys::Button::Backward;   break;
    case 7: button = winsys::Button::Forward;    break;
    default: return std::monostate{};
    }

    std::size_t x11_modifiers[] = {
        ControlMask,
        ShiftMask,
        Mod1Mask,
        Mod4Mask,
        Mod2Mask,
        Mod5Mask
    };

    auto x11_to_modifier = [](std::size_t x11_modifier) -> winsys::Modifier {
        switch (x11_modifier) {
        case ControlMask: return winsys::Modifier::Ctrl;
        case ShiftMask:   return winsys::Modifier::Shift;
        case Mod1Mask:    return winsys::Modifier::Alt;
        case Mod4Mask:    return winsys::Modifier::Super;
        case Mod2Mask:    return winsys::Modifier::NumLock;
        case Mod5Mask:    return winsys::Modifier::ScrollLock;
        default: return static_cast<winsys::Modifier>(0);
        }
    };

    std::unordered_set<winsys::Modifier> modifiers = {};
    for (auto& x11_modifier : x11_modifiers)
        if ((event.state & x11_modifier) > 0)
            modifiers.insert(x11_to_modifier(x11_modifier));

    return winsys::MouseEvent {
        {
            winsys::MouseCapture::MouseCaptureKind::Release,
            winsys::MouseInput {
                winsys::MouseInput::MouseInputTarget::Global,
                button,
                modifiers
            },
            window,
            winsys::Pos {
                event.x_root,
                event.y_root
            }
        },
        on_root
    };
}

winsys::Event
XConnection::on_circulate_request()
{
    XCirculateSubwindows(
        mp_dpy,
        m_current_event.xcirculaterequest.window,
        m_current_event.xcirculaterequest.place
            == PlaceOnTop ?  RaiseLowest : LowerHighest
    );

    return std::monostate{};
}

winsys::Event
XConnection::on_client_message()
{
    XClientMessageEvent event = m_current_event.xclient;
    winsys::Window window = event.window;

    NetWMID id;
    for (id = NetWMID::NetFirst; id < NetWMID::NetLast; ++id)
        if (event.message_type == get_netwm_atom(id))
            break;

    if (id >= NetWMID::NetLast)
        return std::monostate{};

    switch (id) {
    case NetWMID::NetWMState:
    {
        for (std::size_t property = 1; property <= 2; ++property) {
            if (event.data.l[property] == 0)
                continue;

            if (event.data.l[0] >= static_cast<int>(NetWMAction::NetNoAction))
                break;

            Atom atom = static_cast<Atom>(event.data.l[property]);
            winsys::WindowState state;

            if (atom == get_netwm_atom(NetWMID::NetWMStateFullscreen))
                state = winsys::WindowState::Fullscreen;
            else if (atom == get_netwm_atom(NetWMID::NetWMStateAbove))
                state = winsys::WindowState::Above_;
            else if (atom == get_netwm_atom(NetWMID::NetWMStateBelow))
                state = winsys::WindowState::Below_;
            else if (atom == get_netwm_atom(NetWMID::NetWMStateDemandsAttention))
                state = winsys::WindowState::DemandsAttention;
            else
                 return std::monostate{};

            winsys::Toggle toggle;
            switch (event.data.l[0]) {
            case 0: toggle = winsys::Toggle::Off; break;
            case 1: toggle = winsys::Toggle::On; break;
            case 2: toggle = winsys::Toggle::Reverse; break;
            default: return std::monostate{};
            }

            return winsys::StateRequestEvent {
                window,
                state,
                toggle,
                window == m_root
            };
        }

        break;
    }
    case NetWMID::NetMoveResizeWindow:
    {
        std::optional<winsys::Pos> pos = {};
        std::optional<winsys::Dim> dim = {};

        pos = winsys::Pos {
            static_cast<int>(event.data.l[1]),
            static_cast<int>(event.data.l[2])
        };

        if (event.data.l[3] > 0 && event.data.l[4] > 0)
            dim = winsys::Dim {
                static_cast<int>(event.data.l[3]),
                static_cast<int>(event.data.l[4])
            };

        return winsys::PlacementRequestEvent {
            window,
            pos,
            dim,
            window == m_root
        };
    }
    case NetWMID::NetWMMoveResize:
    {
        winsys::Grip grip = static_cast<winsys::Grip>(0);

        switch (event.data.l[2]) {
        case 0: grip |= winsys::Grip::Left;   // fallthrough
        case 1: grip |= winsys::Grip::Top;    break;

        case 2: grip |= winsys::Grip::Top;    // fallthrough
        case 3: grip |= winsys::Grip::Right;  break;

        case 4: grip |= winsys::Grip::Right;  // fallthrough
        case 5: grip |= winsys::Grip::Bottom; break;

        case 6: grip |= winsys::Grip::Left;   // fallthrough
        case 7: grip |= winsys::Grip::Bottom; break;
        case 8: break;
        default: return std::monostate{};
        }

        return winsys::GripRequestEvent {
            event.window,
            winsys::Pos {
                static_cast<int>(event.data.l[0]),
                static_cast<int>(event.data.l[1])
            },
            grip == static_cast<winsys::Grip>(0)
                ? std::nullopt
                : std::optional(grip),
            event.window == m_root
        };
    }
    case NetWMID::NetActiveWindow:
    {
        if (event.data.l[0] <= 2)
            return winsys::FocusRequestEvent {
                window,
                window == m_root
            };

        break;
    }
    case NetWMID::NetWMCloseWindow:
    return winsys::CloseRequestEvent {
        window,
        window == m_root
    };
    case NetWMID::NetRequestFrameExtents:
    return winsys::FrameExtentsRequestEvent {
        window,
        window == m_root
    };
    case NetWMID::NetCurrentDesktop:
    return winsys::WorkspaceRequestEvent {
        std::nullopt,
        static_cast<unsigned>(event.data.l[0]),
        window == m_root
    };
    default: break;
    }

    return std::monostate{};
}

winsys::Event
XConnection::on_configure_notify()
{
    XConfigureEvent event = m_current_event.xconfigure;
    winsys::Window window = event.window;

    return winsys::ConfigureEvent {
        window,
        winsys::Region {
            winsys::Pos { event.x, event.y },
            winsys::Dim {
                static_cast<int>(event.width),
                static_cast<int>(event.height)
            }
        },
        window == m_root
    };
}

winsys::Event
XConnection::on_configure_request()
{
    XConfigureRequestEvent event = m_current_event.xconfigurerequest;
    winsys::Window window = event.window;
    winsys::Window sibling = event.above;

    std::optional<winsys::Region> region
        = get_window_geometry(window);

    if (!region)
        return std::monostate{};

    std::optional<int> x = std::nullopt;
    std::optional<int> y = std::nullopt;
    std::optional<int> w = std::nullopt;
    std::optional<int> h = std::nullopt;

    if ((event.value_mask & CWX) != 0)
        x = event.x;

    if ((event.value_mask & CWY) != 0)
        y = event.y;

    if ((event.value_mask & CWWidth) != 0)
        w = event.width;

    if ((event.value_mask & CWHeight) != 0)
        h = event.height;

    std::optional<winsys::Pos> pos = std::nullopt;
    std::optional<winsys::Dim> dim = std::nullopt;

    if (x || y)
        pos = winsys::Pos {
            x ? *x : region->pos.x,
            y ? *y : region->pos.y
        };

    if (w || h)
        dim = winsys::Dim {
            w ? *w : region->dim.w,
            h ? *h : region->dim.h
        };

    if (pos || dim)
        return winsys::PlacementRequestEvent {
            window,
            pos,
            dim,
            window == m_root
        };

    if ((event.value_mask & CWStackMode) != 0) {
        if (sibling != None)
            switch (event.detail) {
            case Above:
            return winsys::RestackRequestEvent {
                window,
                sibling,
                winsys::StackMode::Above_,
                window == m_root
            };
            case Below:
            return winsys::RestackRequestEvent {
                window,
                sibling,
                winsys::StackMode::Below_,
                window == m_root
            };
            default: break;
            }
    }

    return std::monostate{};
}

winsys::Event
XConnection::on_destroy_notify()
{
    return winsys::DestroyEvent {
        m_current_event.xdestroywindow.window
    };
}

winsys::Event
XConnection::on_expose()
{
    return winsys::ExposeEvent {
        m_current_event.xexpose.window
    };
}

winsys::Event
XConnection::on_focus_in()
{
    return std::monostate{};
}

winsys::Event
XConnection::on_key_press()
{
    XKeyEvent event = m_current_event.xkey;
    winsys::Window window = event.window;
    winsys::Window subwindow = event.subwindow;

    if (window == m_root || window == None)
        window = subwindow;

    winsys::Key key = get_key(event.keycode);

    std::size_t x11_modifiers[] = {
        ControlMask,
        ShiftMask,
        Mod1Mask,
        Mod4Mask,
        Mod2Mask,
        Mod5Mask
    };

    auto x11_to_modifier = [](std::size_t x11_modifier) -> winsys::Modifier {
        switch (x11_modifier) {
        case ControlMask: return winsys::Modifier::Ctrl;
        case ShiftMask:   return winsys::Modifier::Shift;
        case Mod1Mask:    return winsys::Modifier::Alt;
        case Mod4Mask:    return winsys::Modifier::Super;
        case Mod2Mask:    return winsys::Modifier::NumLock;
        case Mod5Mask:    return winsys::Modifier::ScrollLock;
        default: return static_cast<winsys::Modifier>(0);
        }
    };

    std::unordered_set<winsys::Modifier> modifiers = {};
    for (auto& x11_modifier : x11_modifiers)
        if ((event.state & x11_modifier) != 0)
            modifiers.insert(x11_to_modifier(x11_modifier));

    return winsys::KeyEvent {
        {
            winsys::KeyInput {
                key,
                modifiers
            },
            window
        }
    };
}

winsys::Event
XConnection::on_map_notify()
{
    XMapEvent event = m_current_event.xmap;
    winsys::Window window = event.window;

    return winsys::MapEvent {
        window,
        !must_manage_window(event.window)
    };
}

winsys::Event
XConnection::on_map_request()
{
    XMapRequestEvent event = m_current_event.xmaprequest;
    winsys::Window window = event.window;

    return winsys::MapRequestEvent {
        window,
        !must_manage_window(event.window)
    };
}

winsys::Event
XConnection::on_mapping_notify()
{
    XMappingEvent event = m_current_event.xmapping;
    if (event.request == MappingKeyboard)
        XRefreshKeyboardMapping(&event);

    return std::monostate{};
}

winsys::Event
XConnection::on_motion_notify()
{
    last_typed_event(m_current_event, MotionNotify);

    XMotionEvent event = m_current_event.xmotion;
    winsys::Window window = event.window;
    winsys::Window subwindow = event.subwindow;

    bool on_root = false;
    if (window == m_root && subwindow == None)
        on_root = true;

    if (window == m_root || window == None)
        window = subwindow;

    std::size_t x11_modifiers[] = {
        ControlMask,
        ShiftMask,
        Mod1Mask,
        Mod4Mask,
        Mod2Mask,
        Mod5Mask
    };

    auto x11_to_modifier = [](std::size_t x11_modifier) -> winsys::Modifier {
        switch (x11_modifier) {
        case ControlMask: return winsys::Modifier::Ctrl;
        case ShiftMask:   return winsys::Modifier::Shift;
        case Mod1Mask:    return winsys::Modifier::Alt;
        case Mod4Mask:    return winsys::Modifier::Super;
        case Mod2Mask:    return winsys::Modifier::NumLock;
        case Mod5Mask:    return winsys::Modifier::ScrollLock;
        default: return static_cast<winsys::Modifier>(0);
        }
    };

    std::unordered_set<winsys::Modifier> modifiers = {};
    for (auto& x11_modifier : x11_modifiers)
        if ((event.state & x11_modifier) != 0)
            modifiers.insert(x11_to_modifier(x11_modifier));

    return winsys::MouseEvent {
        {
            winsys::MouseCapture::MouseCaptureKind::Motion,
            winsys::MouseInput {
                winsys::MouseInput::MouseInputTarget::Global,
                winsys::Button::Left,
                modifiers
            },
            window,
            winsys::Pos {
                event.x_root,
                event.y_root
            }
        },
        on_root
    };
}

winsys::Event
XConnection::on_property_notify()
{
    XPropertyEvent event = m_current_event.xproperty;
    winsys::Window window = event.window;

    if (event.state == PropertyNewValue) {
        switch (event.atom) {
        case XA_WM_NAME:
        return winsys::PropertyEvent {
            window,
            winsys::PropertyKind::Name,
            window == m_root
        };
        case XA_WM_CLASS:
        return winsys::PropertyEvent {
            window,
            winsys::PropertyKind::Class,
            window == m_root
        };
        case XA_WM_NORMAL_HINTS:
        return winsys::PropertyEvent {
            window,
            winsys::PropertyKind::Size,
            window == m_root
        };
        }

        if (event.atom == get_atom("_NET_WM_NAME")) {
            return winsys::PropertyEvent {
                window,
                winsys::PropertyKind::Name,
                window == m_root
            };
        }
    }

    if (event.atom == get_atom("_NET_WM_STRUT_PARTIAL")
        || event.atom == get_atom("_NET_WM_STRUT"))
    {
        return winsys::PropertyEvent {
            window,
            winsys::PropertyKind::Strut,
            window == m_root
        };
    }

    return std::monostate{};
}

winsys::Event
XConnection::on_unmap_notify()
{
    XMapEvent event = m_current_event.xmap;
    winsys::Window window = event.window;

    return winsys::UnmapEvent {
        window,
        !must_manage_window(window)
    };
}

winsys::Event
XConnection::on_screen_change()
{
    XRRUpdateConfiguration(&m_current_event);
    return winsys::ScreenChangeEvent {};
}

winsys::Event
XConnection::on_unimplemented()
{
    return std::monostate{};
}


int
XConnection::s_otherwm_error_handler(Display*, XErrorEvent*)
{
    Util::die("another window manager is already running");
    return -1;
}

int
XConnection::s_default_error_handler(Display*, XErrorEvent* error)
{
    static const std::unordered_map<int, int> permissible_errors({
        { X_GrabButton,        BadAccess   },
        { X_GrabKey,           BadAccess   },
        { X_CopyArea,          BadDrawable },
        { X_PolyFillRectangle, BadDrawable },
        { X_PolySegment,       BadDrawable },
        { X_PolyText8,         BadDrawable },
        { X_ConfigureWindow,   BadMatch    },
        { X_SetInputFocus,     BadMatch    },
    });

    if (error->error_code == BadWindow)
        return 0;

    for (auto reqerr_pair : permissible_errors)
        if (error->request_code == reqerr_pair.first && error->error_code == reqerr_pair.second)
            return 0;

    return -1;
}

int
XConnection::s_passthrough_error_handler(Display*, XErrorEvent*)
{
    return 0;
}

#pragma GCC diagnostic pop
