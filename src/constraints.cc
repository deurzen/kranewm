#include "constraints.hh"
#include "common.hh"
#include "x_wrapper/attributes.hh"

void
sizeconstraints_t::apply(Pos& pos, Size& size) const
{
    size.w = ::std::max(size.w, MIN_WINDOW_SIZE);
    size.h = ::std::max(size.h, MIN_WINDOW_SIZE);

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);

    if (pos.x >= root_attrs.w())
        pos.x = root_attrs.w() - size.w;

    if (pos.y >= root_attrs.h())
        pos.y = root_attrs.h() - size.h;

    if (pos.x + size.w <= 0)
        pos.x = 0;

    if (pos.y + size.h <= 0)
        pos.y = 0;

    bool base_is_min = base == min;
    if (!base_is_min) {
        size.w -= base.w;
        size.h -= base.h;
    }

    if (base_is_min) {
        size.w -= base.w;
        size.h -= base.h;
    }

    size.w = ::std::max(size.w + base.w, min.w);
    size.h = ::std::max(size.h + base.h, min.h);

    if (max.w)
        size.w = ::std::min(size.w, max.w);

    if (max.h)
        size.h = ::std::min(size.h, max.h);

}
