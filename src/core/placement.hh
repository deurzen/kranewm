#ifndef __PLACEMENT_H_GUARD__
#define __PLACEMENT_H_GUARD__

#include "../winsys/geometry.hh"
#include "../winsys/decoration.hh"

#include <cstdlib>
#include <optional>

typedef class Client* Client_ptr;

struct PlacementTarget final
{
    enum class TargetType
    {
        Client,
        Tab,
        Layout
    };

    TargetType type;
    union
    {
        Client_ptr client;
        std::size_t tab;
    };
};

struct Placement final
{
    enum class PlacementMethod
    {
        Free,
        Tile,
    };

    PlacementMethod method;
    Client_ptr client;
    winsys::Decoration decoration;
    std::optional<winsys::Region> region;
};

#endif//__PLACEMENT_H_GUARD__
