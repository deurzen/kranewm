#include "x-model.hh"

#include "client.hh"
#include "common.hh"
#include "constraints.hh"
#include "floating.hh"

#include "x-data/attributes.hh"


bool
x_model_t::update_hints(client_ptr_t client, x_data::sizehints_t sh)
{
    dim_t base{}, inc{}, max{}, min{};
    range_t<float> aspect{};

    if (sh.flag_set(PBaseSize))
        base = sh.base_dim();
    else if (sh.flag_set(PMinSize))
        base = sh.min_dim();

    if (sh.flag_set(PResizeInc))
        inc = sh.inc_dim();

    if (sh.flag_set(PMaxSize))
        max = sh.max_dim();

    if (sh.flag_set(PMinSize))
        min = sh.min_dim();
    else if (sh.flag_set(PBaseSize))
        min = sh.base_dim();

    if (sh.flag_set(PAspect))
        aspect = {static_cast<float>(sh.min_aspect().y) / sh.min_aspect().x,
            static_cast<float>(sh.max_aspect().y) / sh.max_aspect().x};

    bool changed = !(client->sizeconstraints == sizeconstraints_t{base, inc, max, min, aspect});
    client->sizeconstraints = {base, inc, max, min, aspect};

    return changed;
}

void
x_model_t::enter_move(client_ptr_t client, pos_t pos)
{
    if (m_moveresize)
        return;

    m_moveresize = new moveresize_t(client, moveresizestate_t::move, corner_t::top_left);
    m_pointer = pos;
}

void
x_model_t::enter_resize(client_ptr_t client, pos_t pos)
{
    if (m_moveresize)
        return;

    auto attrs = x_data::get_attributes(client->frame);
    auto center_x = attrs.x() + attrs.w() / 2;
    auto center_y = attrs.y() + attrs.h() / 2;

    corner_t corner = corner_t::top_left;
    if (pos.x > center_x && pos.y > center_y)
        corner = corner_t::bottom_right;
    else if (pos.x < center_x && pos.y > center_y)
        corner = corner_t::bottom_left;
    else if (pos.x > center_x && pos.y < center_y)
        corner = corner_t::top_right;

    m_moveresize = new moveresize_t(client, moveresizestate_t::resize, corner);
    m_pointer = pos;
}

pos_t
x_model_t::update_pointer(pos_t pos)
{
    if (!m_moveresize)
        return m_pointer;

    pos_t diff = {pos.x - m_pointer.x, pos.y - m_pointer.y};

    m_pointer.x = pos.x;
    m_pointer.y = pos.y;

    return diff;
}

void
x_model_t::exit_move_resize()
{
    if (!m_moveresize)
        return;

    delete m_moveresize;
    m_moveresize = nullptr;
}

bool
x_model_t::is_valid() const
{
    return m_moveresize && m_moveresize->state != moveresizestate_t::invalid;
}
