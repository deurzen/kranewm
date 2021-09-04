#ifndef __RULES_H_GUARD__
#define __RULES_H_GUARD__

#include "../winsys/geometry.hh"
#include "client.hh"
#include "defaults.hh"

#include <optional>
#include <vector>

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
    parse_rules(Client_ptr client)
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

        if (client->instance.size() <= prefix.size())
            return rules;

        auto res = std::mismatch(
                prefix.begin(),
                prefix.end(),
                client->instance.begin()
                );

        if (res.first == prefix.end()) {
            bool invert = false;
            bool next_partition = false;
            bool next_context = false;
            bool next_workspace = false;

            std::optional<decltype(snapedge_list_handler)> list_handler = std::nullopt;

            for (auto iter = res.second; iter != client->instance.end(); ++iter) {
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
};

#endif//__RULES_H_GUARD__
