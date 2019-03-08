#include "constraints.hh"
#include "common.hh"
#include "x-wrapper/attributes.hh"

void
sizeconstraints_t::apply(pos_t& pos, dim_t& dim) const
{
    dim.w = ::std::max(dim.w, MIN_WINDOW_SIZE);
    dim.h = ::std::max(dim.h, MIN_WINDOW_SIZE);

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);

    if (pos.x >= root_attrs.w())
        pos.x = root_attrs.w() - dim.w;

    if (pos.y >= root_attrs.h())
        pos.y = root_attrs.h() - dim.h;

    if (pos.x + dim.w <= 0)
        pos.x = 0;

    if (pos.y + dim.h <= 0)
        pos.y = 0;

    if (base == min) {
        dim.w -= base.w;
        dim.h -= base.h;
    }

    dim.w = ::std::max(dim.w + base.w, min.w);
    dim.h = ::std::max(dim.h + base.h, min.h);

    if (max.w)
        dim.w = ::std::min(dim.w, max.w);

    if (max.h)
        dim.h = ::std::min(dim.h, max.h);
}
