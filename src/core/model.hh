#ifndef __MODEL_H_GUARD__
#define __MODEL_H_GUARD__

#include "../winsys/connection.hh"
#include "../winsys/event.hh"
#include "bindings.hh"
#include "client.hh"
#include "config.hh"
#include "context.hh"
#include "cycle.hh"
#include "layout.hh"
#include "partition.hh"
#include "partition.hh"
#include "rules.hh"
#include "search.hh"
#include "stack.hh"
#include "workspace.hh"

#include <atomic>
#include <cstddef>
#include <unordered_map>
#include <vector>

class Model;
class Model final
{
public:
    Model(winsys::Connection&);
    ~Model();

    void run();

private:
    static void wait_children(int);
    static void handle_signal(int);

    void init_signals() const;

    void handle_mouse(winsys::MouseEvent);
    void handle_key(winsys::KeyEvent);
    void handle_map_request(winsys::MapRequestEvent);
    void handle_map(winsys::MapEvent);
    void handle_enter(winsys::EnterEvent);
    void handle_leave(winsys::LeaveEvent);
    void handle_destroy(winsys::DestroyEvent);
    void handle_expose(winsys::ExposeEvent);
    void handle_unmap(winsys::UnmapEvent);
    void handle_state_request(winsys::StateRequestEvent);
    void handle_focus_request(winsys::FocusRequestEvent);
    void handle_close_request(winsys::CloseRequestEvent);
    void handle_workspace_request(winsys::WorkspaceRequestEvent);
    void handle_placement_request(winsys::PlacementRequestEvent);
    void handle_grip_request(winsys::GripRequestEvent);
    void handle_restack_request(winsys::RestackRequestEvent);
    void handle_configure(winsys::ConfigureEvent);
    void handle_property(winsys::PropertyEvent);
    void handle_frame_extents_request(winsys::FrameExtentsRequestEvent);
    void handle_screen_change();

    void process_command(winsys::CommandMessage);
    void process_config(winsys::ConfigMessage);
    void process_client(winsys::WindowMessage);
    void process_workspace(winsys::WorkspaceMessage);
    void process_query(winsys::QueryMessage);

    void acquire_partitions();
    void resolve_active_partition(winsys::Pos);

    winsys::Screen& active_screen();
    const winsys::Screen& active_screen() const;

    Client_ptr get_client(winsys::Window);
    Client_ptr get_const_client(winsys::Window) const;

    Client_ptr search_client(SearchSelector const&);
    bool client_matches_search(Client_ptr, SearchSelector const&) const;

    Partition_ptr active_partition() const;
    Partition_ptr get_partition(Index) const;

    Context_ptr active_context() const;
    Context_ptr get_context(Index) const;

    Workspace_ptr active_workspace() const;
    Workspace_ptr get_workspace(Index) const;

    bool is_free(Client_ptr) const;

    void place_client(Placement&);

    void map_client(Client_ptr);
    void unmap_client(Client_ptr);

    void focus_client(Client_ptr);
    void unfocus_client(Client_ptr);
    void sync_focus();

    void attach_next_client();

    void toggle_partition();
    void activate_next_partition(winsys::Direction);
    void activate_partition(Util::Change<Index>);
    void activate_partition(Partition_ptr);

    void toggle_context();
    void activate_next_context(winsys::Direction);
    void activate_context(Util::Change<Index>);
    void activate_context(Context_ptr);

    void toggle_workspace();
    void toggle_workspace_current_context();
    void activate_next_workspace(winsys::Direction);
    void activate_next_workspace_current_context(winsys::Direction);
    void activate_workspace(Util::Change<Index>);
    void activate_workspace_current_context(Util::Change<Index>);
    void activate_workspace(Workspace_ptr);

    void render_decoration(Client_ptr);

    Rules retrieve_rules(Client_ptr) const;

    void manage(const winsys::Window, const bool, const bool);
    void unmanage(Client_ptr);

    void start_moving(Client_ptr);
    void start_resizing(Client_ptr);

    void stop_moving();
    void stop_resizing();

    void perform_move(winsys::Pos&);
    void perform_resize(winsys::Pos&);

    void apply_layout(Index);
    void apply_layout(Workspace_ptr);

    void apply_stack(Index);
    void apply_stack(Workspace_ptr);

    void cycle_focus(winsys::Direction);
    void drag_focus(winsys::Direction);

    void rotate_clients(winsys::Direction);

    void move_focus_to_next_workspace(winsys::Direction);
    void move_client_to_next_workspace(winsys::Direction, Client_ptr);
    void move_focus_to_workspace(Index);
    void move_client_to_workspace(Index, Client_ptr);

    void toggle_layout();
    void set_layout(LayoutHandler::LayoutKind);
    void set_layout_retain_region(LayoutHandler::LayoutKind);

    void toggle_layout_data();
    void cycle_layout_data(winsys::Direction);
    void copy_data_from_prev_layout();

    void change_gap_size(Util::Change<int>);
    void change_main_count(Util::Change<int>);
    void change_main_factor(Util::Change<float>);
    void change_margin(Util::Change<int>);
    void change_margin(winsys::Edge, Util::Change<int>);
    void reset_gap_size();
    void reset_margin();
    void reset_layout_data();

    void save_layout(std::size_t) const;
    void load_layout(std::size_t);

    void kill_focus();
    void kill_client(Client_ptr);

    void jump_client(SearchSelector const&);

    void set_floating_focus(winsys::Toggle);
    void set_floating_client(winsys::Toggle, Client_ptr);
    void set_fullscreen_focus(winsys::Toggle);
    void set_fullscreen_client(winsys::Toggle, Client_ptr);
    void set_sticky_focus(winsys::Toggle);
    void set_sticky_client(winsys::Toggle, Client_ptr);
    void set_contained_focus(winsys::Toggle);
    void set_contained_client(winsys::Toggle, Client_ptr);
    void set_invincible_focus(winsys::Toggle);
    void set_invincible_client(winsys::Toggle, Client_ptr);
    void set_producing_focus(winsys::Toggle);
    void set_producing_client(winsys::Toggle, Client_ptr);
    void set_iconifyable_focus(winsys::Toggle);
    void set_iconifyable_client(winsys::Toggle, Client_ptr);
    void set_iconify_focus(winsys::Toggle);
    void set_iconify_client(winsys::Toggle, Client_ptr);

    void consume_client(Client_ptr, Client_ptr);
    void check_unconsume_client(Client_ptr, bool = true);

    void center_focus();
    void center_client(Client_ptr);
    void nudge_focus(winsys::Edge, Util::Change<std::size_t>);
    void nudge_client(winsys::Edge, Util::Change<std::size_t>, Client_ptr);
    void stretch_focus(winsys::Edge, Util::Change<int>);
    void stretch_client(winsys::Edge, Util::Change<int>, Client_ptr);
    void inflate_focus(Util::Change<int>);
    void inflate_client(Util::Change<int>, Client_ptr);
    void snap_focus(winsys::Edge);
    void snap_client(winsys::Edge, Client_ptr);

    void activate_screen_struts(winsys::Toggle);

    void pop_deiconify();
    void deiconify_all();

    void spawn_external(std::string&&) const;

    void exit();

    winsys::Connection& m_conn;

    bool m_running;

    Cycle<Partition_ptr> m_partitions;
    Cycle<Context_ptr> m_contexts;
    Cycle<Workspace_ptr> m_workspaces;

    Partition_ptr mp_partition;
    Context_ptr mp_context;
    Workspace_ptr mp_workspace;

    Partition_ptr mp_prev_partition;
    Context_ptr mp_prev_context;
    Workspace_ptr mp_prev_workspace;

    std::atomic<Workspace_ptr> mp_attachment;

    Buffer m_move_buffer;
    Buffer m_resize_buffer;

    StackHandler m_stack;
    std::vector<winsys::Window> m_order;

    std::unordered_map<winsys::Window, Client_ptr> m_client_map;
    std::unordered_map<winsys::Pid, Client_ptr> m_pid_map;
    std::unordered_map<Client_ptr, winsys::Region> m_fullscreen_map;
    std::unordered_map<winsys::Window, std::vector<Client_ptr>> m_leader_map;

    std::vector<Client_ptr> m_sticky_clients;
    std::vector<winsys::Window> m_unmanaged_windows;

    Client_ptr mp_focus;
    Client_ptr mp_jumped_from;

    KeyBindings m_key_bindings;
    MouseBindings m_mouse_bindings;

    struct EventVisitor final
    {
        EventVisitor(Model& model): m_model(model) {}

        void operator()(std::monostate) {}

        void operator()(winsys::MouseEvent event) {
            m_model.handle_mouse(event);
        }

        void operator()(winsys::KeyEvent event) {
            m_model.handle_key(event);
        }

        void operator()(winsys::MapRequestEvent event) {
            m_model.handle_map_request(event);
        }

        void operator()(winsys::MapEvent event) {
            m_model.handle_map(event);
        }

        void operator()(winsys::EnterEvent event) {
            m_model.handle_enter(event);
        }

        void operator()(winsys::LeaveEvent event) {
            m_model.handle_leave(event);
        }

        void operator()(winsys::DestroyEvent event) {
            m_model.handle_destroy(event);
        }

        void operator()(winsys::ExposeEvent event) {
            m_model.handle_expose(event);
        }

        void operator()(winsys::UnmapEvent event) {
            m_model.handle_unmap(event);
        }

        void operator()(winsys::StateRequestEvent event) {
            m_model.handle_state_request(event);
        }

        void operator()(winsys::FocusRequestEvent event) {
            m_model.handle_focus_request(event);
        }

        void operator()(winsys::CloseRequestEvent event) {
            m_model.handle_close_request(event);
        }

        void operator()(winsys::WorkspaceRequestEvent event) {
            m_model.handle_workspace_request(event);
        }

        void operator()(winsys::PlacementRequestEvent event) {
            m_model.handle_placement_request(event);
        }

        void operator()(winsys::GripRequestEvent event) {
            m_model.handle_grip_request(event);
        }

        void operator()(winsys::RestackRequestEvent event) {
            m_model.handle_restack_request(event);
        }

        void operator()(winsys::ConfigureEvent event) {
            m_model.handle_configure(event);
        }

        void operator()(winsys::PropertyEvent event) {
            m_model.handle_property(event);
        }

        void operator()(winsys::FrameExtentsRequestEvent event) {
            m_model.handle_frame_extents_request(event);
        }

        void operator()(winsys::ScreenChangeEvent) {
            m_model.handle_screen_change();
        }

    private:
        Model& m_model;

    } m_event_visitor = EventVisitor(*this);

    struct MessageVisitor final
    {
        MessageVisitor(Model& model): m_model(model) {}

        void operator()(std::monostate) {}

        void operator()(winsys::CommandMessage message) {
            m_model.process_command(message);
        }

        void operator()(winsys::ConfigMessage message) {
            m_model.process_config(message);
        }

        void operator()(winsys::WindowMessage message) {
            m_model.process_client(message);
        }

        void operator()(winsys::WorkspaceMessage message) {
            m_model.process_workspace(message);
        }

        void operator()(winsys::QueryMessage message) {
            m_model.process_query(message);
        }

    private:
        Model& m_model;

    } m_message_visitor = MessageVisitor(*this);

    Config m_config;

};

#endif//__MODEL_H_GUARD__
