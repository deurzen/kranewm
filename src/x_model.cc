#include "x_model.hh"
#include "common.hh"
#include "x_wrapper/attributes.hh"


bool
x_model::update_hints(client_ptr_t client, XSizeHints sh)
{
    Size base{}, inc{}, max{}, min{};
    Range<float> aspect{};

    if (sh.flags & PBaseSize)
        base = {sh.base_width, sh.base_height};
    else if (sh.flags & PMinSize)
        base = {sh.min_width, sh.min_height};

    if (sh.flags & PResizeInc)
        inc = {sh.width_inc, sh.height_inc};

    if (sh.flags & PMaxSize)
        max = {sh.max_width, sh.max_height};

    if (sh.flags & PMinSize)
        min = {sh.min_width, sh.min_height};
    else if (sh.flags & PBaseSize)
        min = {sh.base_width, sh.base_height};

    if (sh.flags & PAspect)
        aspect = {static_cast<float>(sh.min_aspect.y) / sh.min_aspect.x,
            static_cast<float>(sh.max_aspect.y) / sh.max_aspect.x};

    bool changed = !(client->size_constraints == SizeConstraints{base, inc, max, min, aspect});
    client->size_constraints = {base, inc, max, min, aspect};

    return changed;
}

void
x_model::apply_hints(Pos& pos, Size& size, SizeConstraints size_constraints) const
{
    size.w = ::std::max(size.w, MIN_WINDOW_SIZE);
    size.h = ::std::max(size.h, MIN_WINDOW_SIZE);

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);

    if (m_moveresize) {
        if (pos.x > root_attrs.w())
            pos.x = root_attrs.w() - size.w;

        if (pos.y > root_attrs.h())
            pos.y = root_attrs.h() - size.h;

        if (pos.x + size.w < 0)
            pos.x = 0;

        if (pos.y + size.h < 0)
            pos.y = 0;
    } else {
        if (pos.x >= root_attrs.w())
            pos.x = root_attrs.w() - size.w;

        if (pos.y >= root_attrs.h())
            pos.y = root_attrs.h() - size.h;

        if (pos.x + size.w <= 0)
            pos.x = 0;

        if (pos.y + size.h <= 0)
            pos.y = 0;
    }

    bool base_is_min = size_constraints.base == size_constraints.min;
    if (!base_is_min) {
        size.w -= size_constraints.base.w;
        size.h -= size_constraints.base.h;
    }

    if (base_is_min) {
        size.w -= size_constraints.base.w;
        size.h -= size_constraints.base.h;
    }

    size.w = ::std::max(size.w + size_constraints.base.w, size_constraints.min.w);
    size.h = ::std::max(size.h + size_constraints.base.h, size_constraints.min.h);

    if (size_constraints.max.w)
        size.w = ::std::min(size.w, size_constraints.max.w);

    if (size_constraints.max.h)
        size.h = ::std::min(size.h, size_constraints.max.h);
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

client_ptr_t
x_model::get_move_resize_client() const
{
    if (!m_moveresize)
        return nullptr;

    return m_moveresize->client;
}

MoveResizeState
x_model::get_move_resize_state() const
{
    if (!m_moveresize)
        return MR_INVALID;

    return m_moveresize->state;
}

Corner
x_model::get_move_resize_corner() const
{
    if (!m_moveresize)
        return NO_CORNER;

    return m_moveresize->grabbed;
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
