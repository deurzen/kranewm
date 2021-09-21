#ifndef __WINSYS_GEOMETRY_H_GUARD__
#define __WINSYS_GEOMETRY_H_GUARD__

#include "common.hh"
#include "window.hh"

namespace winsys
{

    enum class Edge
    {
        Left,
        Right,
        Top,
        Bottom
    };

    enum class Corner
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    enum class Direction
    {
        Forward,
        Backward
    };

    struct Dim final
    {
        int w;
        int h;
    };

    inline bool
    operator==(Dim const& lhs, Dim const& rhs)
    {
        return lhs.w == rhs.w && lhs.h == rhs.h;
    }

    struct Pos final
    {
        int x;
        int y;

        static Pos
        from_center_of_dim(Dim dim)
        {
            return Pos {
                static_cast<int>(dim.w / 2.f),
                static_cast<int>(dim.h / 2.f)
            };
        }

        static bool
        is_at_origin(Pos& pos)
        {
            return pos.x == 0 && pos.y == 0;
        }
    };

    inline bool
    operator==(Pos const& lhs, Pos const& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    struct Padding final
    {
        int left;
        int right;
        int top;
        int bottom;
    };

    typedef Padding Extents;

    struct Region final
    {
        Pos pos;
        Dim dim;

        void apply_minimum_dim(const Dim&);
        void apply_extents(const Extents&);
        void remove_extents(const Extents&);
    };

    inline bool
    operator==(Region const& lhs, Region const& rhs)
    {
        return lhs.pos == rhs.pos && lhs.dim == rhs.dim;
    }

    struct Distance final
    {
        int dx;
        int dy;
    };

    struct Ratio final
    {
        int numerator;
        int denominator;
    };

    struct Strut final
    {
        Window window;
        int width;
    };

}

#endif//__WINSYS_GEOMETRY_H_GUARD__
