#ifndef __RULES_H_GUARD__
#define __RULES_H_GUARD__

#include "../winsys/geometry.hh"
#include "client.hh"
#include "defaults.hh"

#include <optional>
#include <vector>
#include <iostream>

struct Rules
{
    Rules()
        : do_focus(std::nullopt),
          do_float(std::nullopt),
          do_center(std::nullopt),
          do_fullscreen(std::nullopt),
          to_partition(std::nullopt),
          to_context(std::nullopt),
          to_workspace(std::nullopt),
          snap_edges(std::nullopt)
    {}

    ~Rules() = default;

    std::optional<bool> do_focus;
    std::optional<bool> do_float;
    std::optional<bool> do_center;
    std::optional<bool> do_fullscreen;
    std::optional<Index> to_partition;
    std::optional<Index> to_context;
    std::optional<Index> to_workspace;
    std::optional<std::vector<winsys::Edge>> snap_edges;

    static Rules
    parse_rules(std::string_view rule, bool ignore_prefix = false)
    {
        static std::string prefix = WM_NAME + ":";
        static auto snapedge_list_handler = [](Rules& rules, auto iter) {
            switch (*iter) {
            case 'l': rules.snap_edges->push_back(winsys::Edge::Left);   return;
            case 't': rules.snap_edges->push_back(winsys::Edge::Top);    return;
            case 'r': rules.snap_edges->push_back(winsys::Edge::Right);  return;
            case 'b': rules.snap_edges->push_back(winsys::Edge::Bottom); return;
            default: return;
            }
        };

        Rules rules = {};

        if (!ignore_prefix && rule.size() <= prefix.size())
            return rules;

        auto res = std::mismatch(
            prefix.begin(),
            prefix.end(),
            rule.begin()
        );

        if (ignore_prefix || res.first == prefix.end()) {
            bool invert = false;
            bool next_partition = false;
            bool next_context = false;
            bool next_workspace = false;

            std::optional<decltype(snapedge_list_handler)> list_handler = std::nullopt;

            for (auto iter = res.second; iter != rule.end(); ++iter) {
                if (*iter == '.') {
                    list_handler = std::nullopt;
                    continue;
                }

                if (list_handler) {
                    (*list_handler)(rules, iter);
                    continue;
                }

                if (*iter == '!') {
                    invert = true;
                    continue;
                }

                if (*iter == 'P') {
                    next_partition = true;
                    continue;
                }

                if (*iter == 'C') {
                    next_context = true;
                    continue;
                }

                if (*iter == 'W') {
                    next_workspace = true;
                    continue;
                }

                if (*iter == '@')
                    rules.do_focus = !invert;

                if (*iter == 'f')
                    rules.do_float = !invert;

                if (*iter == 'F')
                    rules.do_fullscreen = !invert;

                if (*iter == 'c')
                    rules.do_center = !invert;

                if (*iter >= '0' && *iter <= '9') {
                    if (next_partition)
                        rules.to_partition = *iter - '0';

                    if (next_context)
                        rules.to_context = *iter - '0';

                    if (next_workspace)
                        rules.to_workspace = *iter - '0';
                }

                if (*iter == 'S') {
                    if (!rules.snap_edges)
                        rules.snap_edges = std::vector<winsys::Edge>{};

                    list_handler = snapedge_list_handler;
                }

                invert = false;
                next_partition = false;
                next_context = false;
                next_workspace = false;
            }
        }

        return rules;
    }

    static Rules
    merge_rules(Rules const& base, Rules const& merger)
    {
        Rules rules = base;

        if (merger.do_focus)
            rules.do_focus = merger.do_focus;

        if (merger.do_float)
            rules.do_float = merger.do_float;

        if (merger.do_center)
            rules.do_center = merger.do_center;

        if (merger.do_fullscreen)
            rules.do_fullscreen = merger.do_fullscreen;

        if (merger.to_partition)
            rules.to_partition = merger.to_partition;

        if (merger.to_context)
            rules.to_context = merger.to_context;

        if (merger.to_workspace)
            rules.to_workspace = merger.to_workspace;

        if (merger.snap_edges)
            rules.snap_edges = merger.snap_edges;

        return rules;
    }
};

#endif//__RULES_H_GUARD__
