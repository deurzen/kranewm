#include "x_model.hh"
#include "common.hh"
#include "x_wrapper/attributes.hh"


bool
x_model::update_hints(client_ptr_t client, x_wrapper::sizehints_t sh)
{
    Size base{}, inc{}, max{}, min{};
    Range<float> aspect{};

    if (sh.get().flags & PBaseSize)
        base = {sh.get().base_width, sh.get().base_height};
    else if (sh.get().flags & PMinSize)
        base = {sh.get().min_width, sh.get().min_height};

    if (sh.get().flags & PResizeInc)
        inc = {sh.get().width_inc, sh.get().height_inc};

    if (sh.get().flags & PMaxSize)
        max = {sh.get().max_width, sh.get().max_height};

    if (sh.get().flags & PMinSize)
        min = {sh.get().min_width, sh.get().min_height};
    else if (sh.get().flags & PBaseSize)
        min = {sh.get().base_width, sh.get().base_height};

    if (sh.get().flags & PAspect)
        aspect = {static_cast<float>(sh.get().min_aspect.y) / sh.get().min_aspect.x,
            static_cast<float>(sh.get().max_aspect.y) / sh.get().max_aspect.x};

    bool changed = !(client->sizeconstraints == sizeconstraints_t{base, inc, max, min, aspect});
    client->sizeconstraints = {base, inc, max, min, aspect};

    return changed;
}

void
x_model::enter_move(client_ptr_t client, Pos pos)
{
    if (m_moveresize)
        return;

    m_moveresize = new moveresize_t(client, MR_MOVE, TOP_LEFT);
    m_pointer = pos;
}

void
x_model::enter_resize(client_ptr_t client, Pos pos)
{
    if (m_moveresize)
        return;

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    auto center_x = root_attrs.x() + root_attrs.w() / 2;
    auto center_y = root_attrs.y() + root_attrs.h() / 2;

    Corner corner = TOP_LEFT;
    if (pos.x > center_x && pos.y > center_y)
        corner = BOTTOM_RIGHT;
    else if (pos.x < center_x && pos.y > center_y)
        corner = BOTTOM_LEFT;
    else if (pos.x > center_x && pos.y < center_y)
        corner = TOP_RIGHT;

    m_moveresize = new moveresize_t(client, MR_RESIZE, corner);
    m_pointer = pos;
}

Pos
x_model::update_pointer(Pos pos)
{
    if (!m_moveresize)
        return m_pointer;

    Pos diff = {pos.x - m_pointer.x, pos.y - m_pointer.y};

    m_pointer.x = pos.x;
    m_pointer.y = pos.y;

    return diff;
}

void
x_model::exit_move_resize()
{
    if (!m_moveresize)
        return;

    delete m_moveresize;
    m_moveresize = nullptr;
}

bool
x_model::is_valid() const
{
    return m_moveresize->state != MR_INVALID;
}
