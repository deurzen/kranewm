#include "layout.hh"
#include "ewmh.hh"
#include "workspace.hh"
#include "x-wrapper/attributes.hh"

#include <algorithm>
#include <functional>


void
layouthandler_t::layout_floating(const user_workspace_t& workspace) const
{
    for (auto& client : workspace.get_all())
        if (!client->fullscreen)
            client->resize(client->float_dim, true).move(client->float_pos, true);
}

void
layouthandler_t::layout_tile(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    unsigned n_master = ::std::min(static_cast<unsigned>(clients.size()), workspace.get_n_master());
    int gap_size = workspace.get_gap_size();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - gap_size,
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - gap_size
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (n_master < clients.size() ? workspace.get_m_factor() : 1)) - gap_size,
        (n_master > 0 ? static_cast<int>(screen_dim.h / n_master) : 1) - gap_size
    };

    dim_t stack_dim = {
        screen_dim.w - (n_master > 0 ? master_dim.w : 0) - (n_master ? 2 : 1) * gap_size,
        screen_dim.h / static_cast<int>(n_master < clients.size() ? (clients.size() - n_master) : 1) - gap_size
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + gap_size,
        m_ewmh.get_top_strut() + gap_size
    };

    pos_t stack_pos  = {
        (n_master > 0 ? master_dim.w + 1: 0) + m_ewmh.get_left_strut() + (n_master ? 2 : 1) * gap_size,
        m_ewmh.get_top_strut() + gap_size
    };

    if (workspace.is_mirrored() && clients.size() > n_master && n_master != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
    }

    for (size_t i = 0; n_master && i < n_master - 1; ++i) {
        clients[i]->resize(master_dim, true).move(master_pos, true);
        master_pos.y += master_dim.h + gap_size;
    }

    if (n_master)
        clients[n_master - 1]->resize({master_dim.w,
            screen_dim.h + m_ewmh.get_top_strut() - master_pos.y}, true).move(master_pos, true);

    for (size_t i = n_master; i < clients.size() - 1; ++i) {
        clients[i]->resize(stack_dim, true).move(stack_pos, true);
        stack_pos.y += stack_dim.h + gap_size;
    }

    if (clients.size() > n_master)
        clients.back()->resize({stack_dim.w,
            screen_dim.h + m_ewmh.get_top_strut() - stack_pos.y}, true).move(stack_pos, true);
}

void
layouthandler_t::layout_deck(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    unsigned n_master = ::std::min(static_cast<unsigned>(clients.size()), workspace.get_n_master());
    int gap_size = workspace.get_gap_size();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - gap_size,
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - gap_size
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (n_master < clients.size() ? workspace.get_m_factor() : 1)) - gap_size,
        (n_master > 0 ? static_cast<int>(screen_dim.h / n_master) : 1) - gap_size
    };

    dim_t stack_dim = {
        screen_dim.w - (n_master > 0 ? master_dim.w : 0) - (n_master ? 2 : 1) * gap_size,
        screen_dim.h - gap_size
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + gap_size,
        m_ewmh.get_top_strut() + gap_size
    };

    pos_t stack_pos  = {
        (n_master > 0 ? master_dim.w + 1: 0) + m_ewmh.get_left_strut() + (n_master ? 2 : 1) * gap_size,
        m_ewmh.get_top_strut() + gap_size
    };

    if (workspace.is_mirrored() && clients.size() > n_master && n_master != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
    }

    for (size_t i = 0; n_master && i < n_master - 1; ++i) {
        clients[i]->resize(master_dim, true).move(master_pos, true);
        master_pos.y += master_dim.h + gap_size;
    }

    if (n_master)
        clients[n_master - 1]->resize({master_dim.w,
            screen_dim.h + m_ewmh.get_top_strut() - master_pos.y}, true).move(master_pos, true);

    for (size_t i = n_master; i < clients.size(); ++i)
        clients[i]->resize(stack_dim, true).move(stack_pos, true);
}

void
layouthandler_t::layout_doubledeck(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    unsigned n_master = ::std::min(static_cast<unsigned>(clients.size()), workspace.get_n_master());
    int gap_size = workspace.get_gap_size();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - gap_size,
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - gap_size
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (n_master < clients.size() ? workspace.get_m_factor() : 1)) - gap_size,
        screen_dim.h - gap_size
    };

    dim_t stack_dim = {
        screen_dim.w - (n_master > 0 ? master_dim.w : 0) - (n_master ? 2 : 1) * gap_size,
        screen_dim.h - gap_size
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + gap_size,
        m_ewmh.get_top_strut() + gap_size
    };

    pos_t stack_pos  = {
        (n_master > 0 ? master_dim.w + 1: 0) + m_ewmh.get_left_strut() + (n_master ? 2 : 1) * gap_size,
        m_ewmh.get_top_strut() + gap_size
    };

    if (workspace.is_mirrored() && clients.size() > n_master && n_master != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
    }

    for (size_t i = 0; i < n_master; ++i)
        clients[i]->resize(master_dim, true).move(master_pos, true);

    for (size_t i = n_master; i < clients.size(); ++i)
        clients[i]->resize(stack_dim, true).move(stack_pos, true);
}

void
layouthandler_t::layout_grid(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    dim_t screen_dim = {root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut(),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut()};

    typedef ::std::deque<client_ptr_t>::reverse_iterator dcrit;
    typedef ::std::function<void(dcrit, dcrit, bool, pos_t, dim_t)> partition_func_t;

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
        {screen_dim.w, screen_dim.h} // tiled area
    );
}

void
layouthandler_t::layout_monocle(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    int gap_size = workspace.get_gap_size();

    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    dim_t screen_dim = {root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - 2 * gap_size,
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - 2 * gap_size};

    for (const auto& client : clients)
        client->resize(screen_dim, true).move({m_ewmh.get_left_strut() + gap_size,
            m_ewmh.get_top_strut() + gap_size}, true);
}

