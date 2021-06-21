#include "geometry.hh"

#include <algorithm>

using namespace winsys;

void
Region::apply_minimum_dim(const Dim& dim)
{
    this->dim.w = std::max(this->dim.w, dim.w);
    this->dim.h = std::max(this->dim.h, dim.h);
}

void
Region::apply_extents(const Extents& extents)
{
    pos.x -= extents.left;
    pos.y -= extents.top;
    dim.w += extents.left + extents.right;
    dim.h += extents.top + extents.bottom;
}

void
Region::remove_extents(const Extents& extents)
{
    pos.x += extents.left;
    pos.y += extents.top;
    dim.w -= extents.left + extents.right;
    dim.h -= extents.top + extents.bottom;
}
