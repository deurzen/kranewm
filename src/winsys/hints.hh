#ifndef __WINSYS_HINTS_H_GUARD__
#define __WINSYS_HINTS_H_GUARD__

#include "geometry.hh"
#include "window.hh"

#include <optional>

namespace winsys
{

    struct SizeHints final
    {
        bool by_user;
        std::optional<Pos> pos;
        std::optional<int> min_width;
        std::optional<int> min_height;
        std::optional<int> max_width;
        std::optional<int> max_height;
        std::optional<int> base_width;
        std::optional<int> base_height;
        std::optional<int> inc_width;
        std::optional<int> inc_height;
        std::optional<double> min_ratio;
        std::optional<double> max_ratio;
        std::optional<Ratio> min_ratio_vulgar;
        std::optional<Ratio> max_ratio_vulgar;

        void apply(Dim&);
    };

    struct Hints final
    {
        bool urgent;
        std::optional<bool> input;
        std::optional<IcccmWindowState> initial_state;
        std::optional<Window> group;
    };

}

#endif//__WINSYS_HINTS_H_GUARD__
