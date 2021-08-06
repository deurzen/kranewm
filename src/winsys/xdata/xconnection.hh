#ifndef __WINSYS_XDATA_XCONNECTION_H_GUARD__
#define __WINSYS_XDATA_XCONNECTION_H_GUARD__

#include "../connection.hh"
#include "../event.hh"
#include "../input.hh"

#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>

extern "C" {
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/cursorfont.h>
#include <sys/un.h>
}

class XConnection final: public winsys::Connection
{
public:
    XConnection(const std::string_view);
    ~XConnection();

    virtual bool flush() override;
    virtual bool block() override;
    virtual void process_events(std::function<void(winsys::Event)>) override;
    virtual void process_messages(std::function<void(winsys::Message)>) override;
    virtual std::vector<winsys::Screen> connected_outputs() override;
    virtual std::vector<winsys::Window> top_level_windows() override;
    virtual winsys::Pos get_pointer_position() override;
    virtual void warp_pointer_center_of_window_or_root(std::optional<winsys::Window>, winsys::Screen&) override;
    virtual void warp_pointer(winsys::Pos) override;
    virtual void warp_pointer_rpos(winsys::Window, winsys::Pos) override;
    virtual void confine_pointer(winsys::Window) override;
    virtual bool release_pointer() override;
    virtual void call_external_command(std::string&) override;
    virtual void cleanup() override;

    // window manipulation
    virtual winsys::Window create_frame(winsys::Region) override;
    virtual void init_window(winsys::Window, bool) override;
    virtual void init_frame(winsys::Window, bool) override;
    virtual void init_unmanaged(winsys::Window) override;
    virtual void init_move(winsys::Window) override;
    virtual void init_resize(winsys::Window) override;
    virtual void cleanup_window(winsys::Window) override;
    virtual void map_window(winsys::Window) override;
    virtual void unmap_window(winsys::Window) override;
    virtual void reparent_window(winsys::Window, winsys::Window, winsys::Pos) override;
    virtual void unparent_window(winsys::Window, winsys::Pos) override;
    virtual void destroy_window(winsys::Window) override;
    virtual bool close_window(winsys::Window) override;
    virtual bool kill_window(winsys::Window) override;
    virtual void place_window(winsys::Window, winsys::Region&) override;
    virtual void move_window(winsys::Window, winsys::Pos) override;
    virtual void resize_window(winsys::Window, winsys::Dim) override;
    virtual void focus_window(winsys::Window) override;
    virtual void stack_window_above(winsys::Window, std::optional<winsys::Window>) override;
    virtual void stack_window_below(winsys::Window, std::optional<winsys::Window>) override;
    virtual void insert_window_in_save_set(winsys::Window) override;
    virtual void grab_bindings(std::vector<winsys::KeyInput>&, std::vector<winsys::MouseInput>&) override;
    virtual void regrab_buttons(winsys::Window) override;
    virtual void ungrab_buttons(winsys::Window) override;
    virtual void unfocus() override;
    virtual void set_window_border_width(winsys::Window, unsigned) override;
    virtual void set_window_border_color(winsys::Window, unsigned) override;
    virtual void set_window_background_color(winsys::Window, unsigned) override;
    virtual void update_window_offset(winsys::Window, winsys::Window) override;
    virtual winsys::Window get_focused_window() override;
    virtual std::optional<winsys::Region> get_window_geometry(winsys::Window) override;
    virtual std::optional<winsys::Pid> get_window_pid(winsys::Window) override;
    virtual std::optional<winsys::Pid> get_ppid(std::optional<winsys::Pid>) override;
    virtual bool must_manage_window(winsys::Window) override;
    virtual bool must_free_window(winsys::Window) override;
    virtual bool window_is_mappable(winsys::Window) override;

    // ICCCM
    virtual void set_icccm_window_state(winsys::Window, winsys::IcccmWindowState) override;
    virtual void set_icccm_window_hints(winsys::Window, winsys::Hints) override;
    virtual std::string get_icccm_window_name(winsys::Window) override;
    virtual std::string get_icccm_window_class(winsys::Window) override;
    virtual std::string get_icccm_window_instance(winsys::Window) override;
    virtual std::optional<winsys::Window> get_icccm_window_transient_for(winsys::Window) override;
    virtual std::optional<winsys::Window> get_icccm_window_client_leader(winsys::Window) override;
    virtual std::optional<winsys::Hints> get_icccm_window_hints(winsys::Window) override;
    virtual std::optional<winsys::SizeHints> get_icccm_window_size_hints(winsys::Window, std::optional<winsys::Dim>) override;

    // EWMH
    virtual void init_for_wm(std::string const&, std::vector<std::string> const&) override;
    virtual void set_current_desktop(Index) override;
    virtual void set_root_window_name(std::string const&) override;
    virtual void set_window_desktop(winsys::Window, Index) override;
    virtual void set_window_state(winsys::Window, winsys::WindowState, bool) override;
    virtual void set_window_frame_extents(winsys::Window, winsys::Extents) override;
    virtual void set_desktop_geometry(std::vector<winsys::Region> const&) override;
    virtual void set_desktop_viewport(std::vector<winsys::Region> const&) override;
    virtual void set_workarea(std::vector<winsys::Region> const&) override;
    virtual void update_desktops(std::vector<std::string> const&) override;
    virtual void update_client_list(std::vector<winsys::Window> const&) override;
    virtual void update_client_list_stacking(std::vector<winsys::Window> const&) override;
    virtual std::optional<std::vector<std::optional<winsys::Strut>>> get_window_strut(winsys::Window) override;
    virtual std::optional<std::vector<std::optional<winsys::Strut>>> get_window_strut_partial(winsys::Window) override;
    virtual std::optional<Index> get_window_desktop(winsys::Window) override;
    virtual std::unordered_set<winsys::WindowType> get_window_types(winsys::Window) override;
    virtual std::unordered_set<winsys::WindowState> get_window_states(winsys::Window) override;
    virtual bool window_is_fullscreen(winsys::Window) override;
    virtual bool window_is_above(winsys::Window) override;
    virtual bool window_is_below(winsys::Window) override;
    virtual bool window_is_sticky(winsys::Window) override;

private:
    static int s_otherwm_error_handler(Display*, XErrorEvent*);
    static int s_passthrough_error_handler(Display*, XErrorEvent*);
    static int s_default_error_handler(Display*, XErrorEvent*);

    enum NetWMID : int
    { // NetWM atom identifiers
        NetSupported = 0, NetFirst = NetSupported,
        NetClientList,
        NetNumberOfDesktops,
        NetCurrentDesktop,
        NetDesktopNames,
        NetDesktopGeometry,
        NetDesktopViewport,
        NetWorkarea,
        NetActiveWindow,
        NetWMName,
        NetWMDesktop,
        NetWMStrut,
        NetWMStrutPartial,
        NetWMFrameExtents,
        NetSupportingWMCheck,
        NetWMState,
        NetWMWindowType,
        // root messages
        NetWMCloseWindow, NetWMRootFirst = NetWMCloseWindow,
        NetWMMoveResize,
        NetRequestFrameExtents,
        NetMoveResizeWindow, NetWMRootLast = NetMoveResizeWindow,
        // window states
        NetWMStateFullscreen, NetWMStateFirst = NetWMStateFullscreen,
        NetWMStateAbove,
        NetWMStateBelow,
        NetWMStateDemandsAttention,
        NetWMStateHidden, NetWMStateLast = NetWMStateHidden,
        // window types
        NetWMWindowTypeDesktop, NetWMWindowTypeFirst = NetWMWindowTypeDesktop,
        NetWMWindowTypeDock,
        NetWMWindowTypeToolbar,
        NetWMWindowTypeMenu,
        NetWMWindowTypeUtility,
        NetWMWindowTypeSplash,
        NetWMWindowTypeDialog,
        NetWMWindowTypeDropdownMenu,
        NetWMWindowTypePopupMenu,
        NetWMWindowTypeTooltip,
        NetWMWindowTypeNotification,
        NetWMWindowTypeNormal, NetWMwindowtypelast = NetWMWindowTypeNormal,
        NetLast
    };

    enum class NetWMAction
    {
        NetRemove   = 0,
        NetAdd      = 1,
        NetToggle   = 2,
        NetNoAction
    };

    friend inline XConnection::NetWMID&
    operator++(XConnection::NetWMID& id)
    {
        return id = static_cast<XConnection::NetWMID>(static_cast<int>(id) + 1);
    }

    Display* mp_dpy;
    winsys::Window m_root;
    winsys::Window m_check_window;

    int m_dpy_fd;
    int m_sock_fd;
    int m_client_fd;
    int m_max_fd;

	fd_set m_descr;
	char m_sock_path[256];
	char m_state_path[256] = { 0 };
	struct sockaddr_un m_sock_addr;

    XEvent m_current_event;

    int m_substructure_level = 0;
    long m_prev_root_mask = 0;

    Status m_property_status = 0;

    std::optional<winsys::Window> m_confined_to;

    std::unordered_map<std::string, Atom> m_interned_atoms;
    std::unordered_map<Atom, std::string> m_atom_names;

    std::unordered_map<std::size_t, winsys::Key> m_keys;
    std::unordered_map<winsys::Key, std::size_t> m_keycodes;

    std::unordered_map<NetWMID, Atom> m_netwm_atoms;

    int (*m_checkwm_error_handler)(Display*, XErrorEvent*);

    template <class T>
    XEvent
    create_event(winsys::Window window, Atom type, T data)
    {
        XEvent event;
        event.type = ClientMessage;
        event.xclient.window = window;
        event.xclient.message_type = type;
        event.xclient.format = 32;
        event.xclient.data.l[0] = data;
        event.xclient.data.l[1] = CurrentTime;
        return event;
    }

    void enable_substructure_events();
    void disable_substructure_events();

    void next_event(XEvent&);
    bool typed_event(XEvent&, int);
    void last_typed_event(XEvent&, int);

    void sync(bool);
    int pending();

    winsys::Window create_handle();

    Atom get_atom(std::string const&);
    Atom get_netwm_atom(NetWMID const&);
    void intern_atom(std::string const&, Atom);

    winsys::Key get_key(const std::size_t);
    std::size_t get_keycode(const winsys::Key);
    void intern_key(const unsigned, const winsys::Key);
    winsys::Button get_button(const unsigned) const;
    unsigned get_buttoncode(const winsys::Button) const;

    winsys::WindowState get_window_state_from_atom(Atom);
    winsys::WindowType get_window_type_from_atom(Atom);
    Atom get_atom_from_window_state(winsys::WindowState);
    Atom get_atom_from_window_type(winsys::WindowType);

    bool property_status_ok();

    bool has_atom_property(winsys::Window, Atom);
    bool has_atomlist_property(winsys::Window, Atom);
    bool has_window_property(winsys::Window, Atom);
    bool has_windowlist_property(winsys::Window, Atom);
    bool has_string_property(winsys::Window, Atom);
    bool has_stringlist_property(winsys::Window, Atom);
    bool has_card_property(winsys::Window, Atom);
    bool has_cardlist_property(winsys::Window, Atom);

    Atom get_atom_property(winsys::Window, std::string const&);
    std::vector<Atom> get_atomlist_property(winsys::Window, std::string const&);
    winsys::Window get_window_property(winsys::Window, std::string const&);
    std::vector<winsys::Window> get_windowlist_property(winsys::Window, std::string const&);
    std::string get_string_property(winsys::Window, std::string const&);
    std::vector<std::string> get_stringlist_property(winsys::Window, std::string const&);
    unsigned long get_card_property(winsys::Window, std::string const&);
    std::vector<unsigned long> get_cardlist_property(winsys::Window, std::string const&);

    bool get_text_property(winsys::Window, Atom, char*, unsigned);

    void replace_atom_property(winsys::Window, std::string const&, Atom);
    void replace_atomlist_property(winsys::Window, std::string const&, std::vector<Atom> const&);
    void replace_window_property(winsys::Window, std::string const&, winsys::Window);
    void replace_windowlist_property(winsys::Window, std::string const&, std::vector<winsys::Window> const&);
    void replace_string_property(winsys::Window, std::string const&, std::string const&);
    void replace_stringlist_property(winsys::Window, std::string const&, std::vector<std::string> const&);
    void replace_card_property(winsys::Window, std::string const&, const unsigned long);
    void replace_cardlist_property(winsys::Window, std::string const&, std::vector<unsigned long> const&);

    void append_atomlist_property(winsys::Window, std::string const&, Atom);
    void append_windowlist_property(winsys::Window, std::string const&, winsys::Window);
    void append_stringlist_property(winsys::Window, std::string const&, std::string const&);
    void append_cardlist_property(winsys::Window, std::string const&, const unsigned long);

    void unset_atom_property(winsys::Window, std::string const&);
    void unset_atomlist_property(winsys::Window, std::string const&);
    void unset_window_property(winsys::Window, std::string const&);
    void unset_windowlist_property(winsys::Window, std::string const&);
    void unset_string_property(winsys::Window, std::string const&);
    void unset_stringlist_property(winsys::Window, std::string const&);
    void unset_card_property(winsys::Window, std::string const&);
    void unset_cardlist_property(winsys::Window, std::string const&);

    bool window_is_any_of_states(winsys::Window, std::vector<winsys::WindowState> const&);
    bool window_is_any_of_types(winsys::Window, std::vector<winsys::WindowType> const&);

    void check_otherwm();

    // event dispatching logic
    winsys::Event (XConnection::*m_event_dispatcher[256])();

    winsys::Event on_button_press();
    winsys::Event on_button_release();
    winsys::Event on_circulate_request();
    winsys::Event on_client_message();
    winsys::Event on_configure_notify();
    winsys::Event on_configure_request();
    winsys::Event on_destroy_notify();
    winsys::Event on_expose();
    winsys::Event on_focus_in();
    winsys::Event on_key_press();
    winsys::Event on_map_notify();
    winsys::Event on_map_request();
    winsys::Event on_mapping_notify();
    winsys::Event on_motion_notify();
    winsys::Event on_property_notify();
    winsys::Event on_screen_change();
    winsys::Event on_unmap_notify();

    winsys::Event on_unimplemented();
};

#endif//__WINSYS_XDATA_XCONNECTION_H_GUARD__
