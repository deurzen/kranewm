#ifndef __WINSYS_DECORATION_H_GUARD__
#define __WINSYS_DECORATION_H_GUARD__

#include "geometry.hh"

#include <optional>

namespace winsys
{

    typedef unsigned Color;

    struct ColorScheme final
    {
        static const ColorScheme DEFAULT_COLOR_SCHEME;

        Color focused;
        Color fdisowned;
        Color fsticky;
        Color unfocused;
        Color udisowned;
        Color usticky;
        Color urgent;
    };

    struct Border final
    {
        unsigned width;
        ColorScheme colors;
    };

    struct Frame final
    {
        Extents extents;
        ColorScheme colors;
    };

    struct Decoration final
    {
        static const Decoration NO_DECORATION;
        static const Decoration FREE_DECORATION;

        std::optional<Border> border;
        std::optional<Frame> frame;

        const Extents extents() const;
    };

}

#endif//__WINSYS_DECORATION_H_GUARD__
