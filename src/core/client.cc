#include "client.hh"

#include <iostream>

Client::Client(
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
)
    : window(window),
      frame(frame),
      name(name),
      class_(class_),
      instance(instance),
      partition(partition),
      context(context),
      workspace(workspace),
      free_region({}),
      tile_region({}),
      active_region({}),
      previous_region({}),
      inner_region({}),
      tile_decoration(winsys::Decoration::FREE_DECORATION),
      free_decoration(winsys::Decoration::FREE_DECORATION),
      active_decoration(winsys::Decoration::FREE_DECORATION),
      size_hints(std::nullopt),
      warp_pos(std::nullopt),
      leader(std::nullopt),
      parent(nullptr),
      children({}),
      producer(nullptr),
      consumers({}),
      focused(false),
      mapped(false),
      managed(true),
      urgent(false),
      floating(false),
      fullscreen(false),
      contained(false),
      invincible(false),
      sticky(false),
      iconifyable(true),
      iconified(false),
      disowned(false),
      producing(true),
      attaching(false),
      pid(pid),
      ppid(ppid),
      last_touched(std::chrono::steady_clock::now()),
      last_focused(std::chrono::steady_clock::now()),
      managed_since(std::chrono::steady_clock::now()),
      expected_unmap_count(0),
      m_outside_state(OutsideState::Unfocused)
{}

Client::~Client()
{}

Client::OutsideState
Client::get_outside_state() const
{
    if (urgent)
        return Client::OutsideState::Urgent;

    return m_outside_state;
}

void
Client::touch()
{
    last_touched = std::chrono::steady_clock::now();
}

void
Client::focus()
{
    focused = true;

    auto now = std::chrono::steady_clock::now();
    last_touched = now;
    last_focused = now;

    switch (m_outside_state) {
    case OutsideState::Unfocused:         m_outside_state = OutsideState::Focused;         return;
    case OutsideState::UnfocusedDisowned: m_outside_state = OutsideState::FocusedDisowned; return;
    case OutsideState::UnfocusedSticky:   m_outside_state = OutsideState::FocusedSticky;   return;
    default: return;
    }
}

void
Client::unfocus()
{
    focused = false;

    switch (m_outside_state) {
    case OutsideState::Focused:         m_outside_state = OutsideState::Unfocused;         return;
    case OutsideState::FocusedDisowned: m_outside_state = OutsideState::UnfocusedDisowned; return;
    case OutsideState::FocusedSticky:   m_outside_state = OutsideState::UnfocusedSticky;   return;
    default: return;
    }
}

void
Client::stick()
{
    sticky = true;

    switch (m_outside_state) {
    case OutsideState::Focused:   m_outside_state = OutsideState::FocusedSticky;   return;
    case OutsideState::Unfocused: m_outside_state = OutsideState::UnfocusedSticky; return;
    default: return;
    }
}

void
Client::unstick()
{
    sticky = false;

    switch (m_outside_state) {
    case OutsideState::FocusedSticky:   m_outside_state = OutsideState::Focused;   return;
    case OutsideState::UnfocusedSticky: m_outside_state = OutsideState::Unfocused; return;
    default: return;
    }
}

void
Client::disown()
{
    disowned = true;

    switch (m_outside_state) {
    case OutsideState::Focused:   m_outside_state = OutsideState::FocusedDisowned;   return;
    case OutsideState::Unfocused: m_outside_state = OutsideState::UnfocusedDisowned; return;
    default: return;
    }
}

void
Client::reclaim()
{
    disowned = false;

    switch (m_outside_state) {
    case OutsideState::FocusedDisowned:   m_outside_state = OutsideState::Focused;   return;
    case OutsideState::UnfocusedDisowned: m_outside_state = OutsideState::Unfocused; return;
    default: return;
    }
}

void
Client::expect_unmap()
{
    expected_unmap_count += 1;
}

bool
Client::consume_unmap_if_expecting()
{
    bool expecting = expected_unmap_count > 0;

    if (expecting)
        expected_unmap_count -= 1;

    return expecting;
}

void
Client::set_tile_region(winsys::Region& region)
{
    tile_region = region;
    set_active_region(region);
}

void
Client::set_free_region(winsys::Region& region)
{
    free_region = region;
    set_active_region(region);
}

void
Client::set_active_region(winsys::Region& region)
{
    previous_region = active_region;
    set_inner_region(region);
    active_region = region;
}


void
Client::set_tile_decoration(winsys::Decoration const& decoration)
{
    tile_decoration = decoration;
    active_decoration = decoration;
}

void
Client::set_free_decoration(winsys::Decoration const& decoration)
{
    free_decoration = decoration;
    active_decoration = decoration;
}

void
Client::set_inner_region(winsys::Region& region)
{
    if (active_decoration.frame) {
        winsys::Frame& frame = *active_decoration.frame;

        inner_region.pos.x = frame.extents.left;
        inner_region.pos.y = frame.extents.top;
        inner_region.dim.w = region.dim.w - frame.extents.left - frame.extents.right;
        inner_region.dim.h = region.dim.h - frame.extents.top - frame.extents.bottom;
    } else {
        inner_region.pos.x = 0;
        inner_region.pos.y = 0;
        inner_region.dim = region.dim;
    }
}
