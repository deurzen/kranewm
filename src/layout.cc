#include "layout.hh"
#include "workspace.hh"
#include "x-wrapper/attributes.hh"

#include <functional>


void
layouthandler_t::layout_floating(const user_workspace_t& workspace) const
{
    for (auto& client : workspace.get_all())
        client->resize(client->float_dim).move(client->float_pos);
}

void
layouthandler_t::layout_tile(const user_workspace_t& workspace) const
{

}

void
layouthandler_t::layout_deck(const user_workspace_t& workspace) const
{

}

void
layouthandler_t::layout_doubledeck(const user_workspace_t& workspace) const
{

}

void
layouthandler_t::layout_grid(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating; }), clients.end());


    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    dim_t screen = {root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut(),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut()};

    typedef ::std::deque<client_ptr_t>::reverse_iterator dcrit;
    typedef ::std::function<void(dcrit, dcrit, bool, pos_t, dim_t)>
        partition_func_t;

    partition_func_t partition_space = [=, &partition_space](
        dcrit from, dcrit to, bool vsplit, pos_t ppos, dim_t pdim)
    {
        if (to - from < 0)
            return;

        if (to == from) {
            (*from)->resize(pdim, true).move(ppos, true);
            return;
        }

        if (vsplit) {
            partition_space(from, from + ((to-from) / 2),
                !vsplit,
                {ppos.x + (pdim.w / 2), ppos.y},
                {pdim.w / 2, pdim.h}
            );
            partition_space(from + ((to-from) / 2) + 1, to,
                !vsplit,
                ppos,
                {pdim.w / 2, pdim.h}
            );
        } else {
            partition_space(from, from + ((to-from) / 2),
                !vsplit,
                {ppos.x, ppos.y + (pdim.h / 2)},
                {pdim.w, pdim.h / 2 + 1}
            );
            partition_space(from + ((to-from) / 2) + 1, to,
                !vsplit,
                ppos,
                {pdim.w, pdim.h / 2 - 1}
            );
        }
    };

    partition_space(clients.rbegin(), clients.rend() - 1,
        true, // start with vertical split
        {m_ewmh.get_left_strut(), m_ewmh.get_top_strut()}, // tiled origin
        {screen.w, screen.h} // tiled area
    );
}

void
layouthandler_t::layout_monocle(const user_workspace_t& workspace) const
{

}

