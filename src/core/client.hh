#ifndef __CLIENT_H_GUARD__
#define __CLIENT_H_GUARD__

#include "../winsys/common.hh"
#include "../winsys/decoration.hh"
#include "../winsys/geometry.hh"
#include "../winsys/hints.hh"
#include "../winsys/window.hh"

#include <chrono>
#include <optional>
#include <string>
#include <vector>

typedef class Partition* Partition_ptr;
typedef class Context* Context_ptr;
typedef class Workspace* Workspace_ptr;

typedef struct Client* Client_ptr;
typedef struct Client final
{
    enum class OutsideState
    {
        Focused,
        FocusedDisowned,
        FocusedSticky,
        Unfocused,
        UnfocusedDisowned,
        UnfocusedSticky,
        Urgent
    };

    static constexpr winsys::Dim MIN_CLIENT_DIM = winsys::Dim { 25, 10 };
    static constexpr winsys::Dim PREFERRED_CLIENT_DIM = winsys::Dim { 480, 260 };

    static bool
    is_free(Client_ptr client)
    {
        return (client->floating && (!client->fullscreen || client->contained))
            || !client->managed
            || client->disowned;
    }

    Client(
        winsys::Window window,
        winsys::Window frame,
        std::string name,
        std::string class_,
        std::string instance,
        Partition_ptr partition,
        Context_ptr context,
        Workspace_ptr workspace,
        std::optional<winsys::Pid> pid,
        std::optional<winsys::Pid> ppid
    );

    ~Client();

    Client(const Client&) = default;
    Client& operator=(const Client&) = default;

    OutsideState get_outside_state() const;

    void touch();
    void focus();
    void unfocus();

    void stick();
    void unstick();

    void disown();
    void reclaim();

    void set_urgent();
    void unset_urgent();

    void expect_unmap();
    bool consume_unmap_if_expecting();

    void set_tile_region(winsys::Region&);
    void set_free_region(winsys::Region&);

    void set_tile_decoration(winsys::Decoration const&);
    void set_free_decoration(winsys::Decoration const&);

    winsys::Window window;
    winsys::Window frame;
    std::string name;
    std::string class_;
    std::string instance;
    Partition_ptr partition;
    Context_ptr context;
    Workspace_ptr workspace;
    winsys::Region free_region;
    winsys::Region tile_region;
    winsys::Region active_region;
    winsys::Region previous_region;
    winsys::Region inner_region;
    winsys::Decoration tile_decoration;
    winsys::Decoration free_decoration;
    winsys::Decoration active_decoration;
    std::optional<winsys::SizeHints> size_hints;
    std::optional<winsys::Pos> warp_pos;
    std::optional<winsys::Window> leader;
    Client_ptr parent;
    std::vector<Client_ptr> children;
    Client_ptr producer;
    std::vector<Client_ptr> consumers;
    bool focused;
    bool mapped;
    bool managed;
    bool urgent;
    bool floating;
    bool fullscreen;
    bool contained;
    bool invincible;
    bool sticky;
    bool iconifyable;
    bool iconified;
    bool disowned;
    bool producing;
    bool attaching;
    std::optional<winsys::Pid> pid;
    std::optional<winsys::Pid> ppid;
    std::chrono::time_point<std::chrono::steady_clock> last_touched;
    std::chrono::time_point<std::chrono::steady_clock> last_focused;
    std::chrono::time_point<std::chrono::steady_clock> managed_since;
    std::size_t expected_unmap_count;

private:
    OutsideState m_outside_state;

    void set_inner_region(winsys::Region&);
    void set_active_region(winsys::Region&);

}* Client_ptr;

inline bool
operator==(const Client& lhs, const Client& rhs)
{
    return lhs.window == rhs.window;
}

namespace std
{
    template <>
    struct hash<Client>
    {
        std::size_t
        operator()(const Client& client) const
        {
            return std::hash<winsys::Window>{}(client.window);
        }
    };
}

#endif//__CLIENT_H_GUARD__
