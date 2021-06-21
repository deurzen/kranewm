#include "hints.hh"

#include <algorithm>
#include <cmath>

using namespace winsys;

void
SizeHints::apply(Dim& dim)
{
    int dest_width = dim.w;
    int dest_height = dim.h;

    if (min_width)
        dest_width = std::max(dest_width, *min_width);

    if (min_height)
        dest_height = std::max(dest_height, *min_height);

    if (max_width)
        dest_width = std::min(dest_width, *max_width);

    if (max_height)
        dest_height = std::min(dest_height, *max_height);

    int base_width = this->base_width
        ? *this->base_width
        : 0;

    int base_height = this->base_height
        ? *this->base_height
        : 0;

    int width = base_width < dest_width
        ? dest_width - base_width
        : dest_width;

    int height = base_height < dest_height
        ? dest_height - base_height
        : dest_height;

    if (min_ratio || max_ratio) {
        if (height == 0)
            height = 1;

        double current_ratio
            = static_cast<double>(width) / static_cast<double>(height);

        std::optional<double> new_ratio = std::nullopt;

        if (min_ratio && current_ratio < *min_ratio)
            new_ratio = min_ratio;

        if (!new_ratio && max_ratio && current_ratio > *max_ratio)
            new_ratio = max_ratio;

        if (new_ratio) {
            height = std::round(static_cast<double>(width) / *new_ratio);
            width = std::round(static_cast<double>(height) * *new_ratio);

            dest_width = width + base_width;
            dest_height = height + base_height;
        }
    }

    if (inc_height && dest_height >= base_height) {
        dest_height -= base_height;
        dest_height -= dest_height % *inc_height;
        dest_height += base_height;
    }

    if (inc_width && dest_width >= base_width) {
        dest_width -= base_width;
        dest_width -= dest_width % *inc_width;
        dest_width += base_width;
    }

    dim.h = std::max(dest_height, 0);
    dim.w = std::max(dest_width, 0);
}
