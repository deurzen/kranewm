#ifndef __RULES_H_GUARD__
#define __RULES_H_GUARD__

#include "../winsys/common.hh"

#include <optional>

struct Rules
{
    Rules()
        : do_float(std::nullopt),
          do_center(std::nullopt),
          do_fullscreen(std::nullopt),
          to_partition(std::nullopt),
          to_context(std::nullopt),
          to_workspace(std::nullopt)
    {}

    ~Rules() = default;

    std::optional<bool> do_float;
    std::optional<bool> do_center;
    std::optional<bool> do_fullscreen;
    std::optional<Index> to_partition;
    std::optional<Index> to_context;
    std::optional<Index> to_workspace;

};

#endif//__RULES_H_GUARD__
