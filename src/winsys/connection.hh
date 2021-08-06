#ifndef __WINSYS_CONNECTION_H_GUARD__
#define __WINSYS_CONNECTION_H_GUARD__

#include "common.hh"
#include "event.hh"
#include "hints.hh"
#include "message.hh"
#include "screen.hh"
#include "window.hh"

#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace winsys
{

    class Connection
    {
    public:
        virtual ~Connection() {};

        virtual bool flush() = 0;
        virtual bool block() = 0;
        virtual void process_events(std::function<void(Event)>) = 0;
        virtual void process_messages(std::function<void(Message)>) = 0;
        virtual std::vector<Screen> connected_outputs() = 0;
        virtual std::vector<Window> top_level_windows() = 0;
        virtual Pos get_pointer_position() = 0;
        virtual void warp_pointer_center_of_window_or_root(std::optional<Window>, Screen&) = 0;
        virtual void warp_pointer(Pos) = 0;
        virtual void warp_pointer_rpos(Window, Pos) = 0;
        virtual void confine_pointer(Window) = 0;
        virtual bool release_pointer() = 0;
        virtual void call_external_command(std::string&) = 0;
        virtual void cleanup() = 0;

        // window manipulation
        virtual Window create_frame(Region) = 0;
        virtual void init_window(Window, bool) = 0;
        virtual void init_frame(Window, bool) = 0;
        virtual void init_unmanaged(Window) = 0;
        virtual void init_move(Window) = 0;
        virtual void init_resize(Window) = 0;
        virtual void cleanup_window(Window) = 0;
        virtual void map_window(Window) = 0;
        virtual void unmap_window(Window) = 0;
        virtual void reparent_window(Window, Window, Pos) = 0;
        virtual void unparent_window(Window, Pos) = 0;
        virtual void destroy_window(Window) = 0;
        virtual bool close_window(Window) = 0;
        virtual bool kill_window(Window) = 0;
        virtual void place_window(Window, Region&) = 0;
        virtual void move_window(Window, Pos) = 0;
        virtual void resize_window(Window, Dim) = 0;
        virtual void focus_window(Window) = 0;
        virtual void stack_window_above(Window, std::optional<Window>) = 0;
        virtual void stack_window_below(Window, std::optional<Window>) = 0;
        virtual void insert_window_in_save_set(Window) = 0;
        virtual void grab_bindings(std::vector<KeyInput>&, std::vector<MouseInput>&) = 0;
        virtual void regrab_buttons(Window) = 0;
        virtual void ungrab_buttons(Window) = 0;
        virtual void unfocus() = 0;
        virtual void set_window_border_width(Window, unsigned) = 0;
        virtual void set_window_border_color(Window, unsigned) = 0;
        virtual void set_window_background_color(Window, unsigned) = 0;
        virtual void update_window_offset(Window, Window) = 0;
        virtual Window get_focused_window() = 0;
        virtual std::optional<Region> get_window_geometry(Window) = 0;
        virtual std::optional<Pid> get_window_pid(Window) = 0;
        virtual std::optional<Pid> get_ppid(std::optional<Pid>) = 0;
        virtual bool must_manage_window(Window) = 0;
        virtual bool must_free_window(Window) = 0;
        virtual bool window_is_mappable(Window) = 0;

        // ICCCM
        virtual void set_icccm_window_state(Window, IcccmWindowState) = 0;
        virtual void set_icccm_window_hints(Window, Hints) = 0;
        virtual std::string get_icccm_window_name(Window) = 0;
        virtual std::string get_icccm_window_class(Window) = 0;
        virtual std::string get_icccm_window_instance(Window) = 0;
        virtual std::optional<Window> get_icccm_window_transient_for(Window) = 0;
        virtual std::optional<Window> get_icccm_window_client_leader(Window) = 0;
        virtual std::optional<Hints> get_icccm_window_hints(Window) = 0;
        virtual std::optional<SizeHints> get_icccm_window_size_hints(Window, std::optional<Dim>) = 0;

        // EWMH
        virtual void init_for_wm(std::string const&, std::vector<std::string> const&) = 0;
        virtual void set_current_desktop(Index) = 0;
        virtual void set_root_window_name(std::string const&) = 0;
        virtual void set_window_desktop(Window, Index) = 0;
        virtual void set_window_state(Window, WindowState, bool) = 0;
        virtual void set_window_frame_extents(Window, Extents) = 0;
        virtual void set_desktop_geometry(std::vector<Region> const&) = 0;
        virtual void set_desktop_viewport(std::vector<Region> const&) = 0;
        virtual void set_workarea(std::vector<Region> const&) = 0;
        virtual void update_desktops(std::vector<std::string> const&) = 0;
        virtual void update_client_list(std::vector<Window> const&) = 0;
        virtual void update_client_list_stacking(std::vector<Window> const&) = 0;
        virtual std::optional<std::vector<std::optional<Strut>>> get_window_strut(Window) = 0;
        virtual std::optional<std::vector<std::optional<Strut>>> get_window_strut_partial(Window) = 0;
        virtual std::optional<Index> get_window_desktop(Window) = 0;
        virtual std::unordered_set<WindowType> get_window_types(Window) = 0;
        virtual std::unordered_set<WindowState> get_window_states(Window) = 0;
        virtual bool window_is_fullscreen(Window) = 0;
        virtual bool window_is_above(Window) = 0;
        virtual bool window_is_below(Window) = 0;
        virtual bool window_is_sticky(Window) = 0;

    };

}

#endif//__WINSYS_CONNECTION_H_GUARD__
