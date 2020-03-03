#include "layout.hh"
#include "ewmh.hh"
#include "workspace.hh"
#include "x-data/attributes.hh"

#include <algorithm>
#include <functional>
#include <cmath>


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

    auto root_attrs = x_data::get_attributes(x_data::g_root);
    ::std::size_t nmaster = ::std::min(static_cast<::std::size_t>(clients.size()), workspace.get_nmaster());
    int gapsize = workspace.get_gapsize();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - gapsize - (nmaster && nmaster != clients.size() ? 3 : 2),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - gapsize - 2
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (nmaster < clients.size() ? workspace.get_mfactor() : 1)) - gapsize,
        (nmaster > 0 ? static_cast<int>(screen_dim.h / nmaster) : 1) - gapsize
    };

    dim_t stack_dim = {
        screen_dim.w - (nmaster > 0 ? master_dim.w : 0) - (nmaster ? 2 : 1) * gapsize,
        screen_dim.h / static_cast<int>(nmaster < clients.size() ? (clients.size() - nmaster) : 1) - gapsize
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    pos_t stack_pos  = {
        (nmaster > 0 ? master_dim.w + 1: 0) + m_ewmh.get_left_strut() + (nmaster ? 2 : 1) * gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    if (workspace.is_mirrored() && clients.size() > nmaster && nmaster != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
    }

    { // tile master clients
        for (::std::size_t i = 0; nmaster && i < nmaster - 1; ++i) {
            clients[i]->resize(master_dim, true).move(master_pos, true);
            master_pos.y += master_dim.h + gapsize + 1;
        }

        if (nmaster)
            clients[nmaster - 1]->resize({master_dim.w,
                screen_dim.h + m_ewmh.get_top_strut() - master_pos.y}, true).move(master_pos, true);
    }

    { // tile stack clients
        for (::std::size_t i = nmaster; i < clients.size() - 1; ++i) {
            clients[i]->resize(stack_dim, true).move(stack_pos, true);
            stack_pos.y += stack_dim.h + gapsize + 1;
        }

        if (clients.size() > nmaster)
            clients.back()->resize({stack_dim.w,
                screen_dim.h + m_ewmh.get_top_strut() - stack_pos.y}, true).move(stack_pos, true);
    }
}

void
layouthandler_t::layout_stick(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_data::get_attributes(x_data::g_root);
    ::std::size_t nmaster = ::std::min(static_cast<::std::size_t>(clients.size()), workspace.get_nmaster());
    int gapsize = workspace.get_gapsize();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - 2 * gapsize - (nmaster && nmaster != clients.size() ? 3 : 2),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - 2 * gapsize - 2
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (nmaster < clients.size() ? workspace.get_mfactor() : 1))
            - (nmaster == clients.size() ? 2 : 1) * gapsize,
        (nmaster > 0 ? static_cast<int>(screen_dim.h / nmaster) : 1)
    };

    dim_t stack_dim = {
        screen_dim.w - (nmaster > 0 ? master_dim.w : 0) - 2 * gapsize,
        screen_dim.h / static_cast<int>(nmaster < clients.size() ? (clients.size() - nmaster) : 1)
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + 2 * gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    pos_t stack_pos  = {
        master_pos.x + (nmaster > 0 ? master_dim.w + 1: 0),
        m_ewmh.get_top_strut() + gapsize
    };

    if (workspace.is_mirrored() && clients.size() > nmaster && nmaster != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
    }

    { // tile master clients
        for (::std::size_t i = 0; nmaster && i < nmaster - 1; ++i) {
            clients[i]->resize(master_dim, true).move(master_pos, true);
            master_pos.y += master_dim.h + 1;
        }

        if (nmaster)
            clients[nmaster - 1]->resize({master_dim.w,
                screen_dim.h + m_ewmh.get_top_strut() + gapsize - master_pos.y}, true).move(master_pos, true);
    }

    { // tile stack clients
        for (::std::size_t i = nmaster; i < clients.size() - 1; ++i) {
            clients[i]->resize(stack_dim, true).move(stack_pos, true);
            stack_pos.y += stack_dim.h + 1;
        }

        if (clients.size() > nmaster)
            clients.back()->resize({stack_dim.w,
                screen_dim.h + m_ewmh.get_top_strut() + gapsize - stack_pos.y}, true).move(stack_pos, true);
    }
}

void
layouthandler_t::layout_deck(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_data::get_attributes(x_data::g_root);
    ::std::size_t nmaster = ::std::min(static_cast<::std::size_t>(clients.size()), workspace.get_nmaster());
    int gapsize = workspace.get_gapsize();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - gapsize - (nmaster && nmaster != clients.size() ? 3 : 2),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - gapsize - 2
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (nmaster < clients.size() ? workspace.get_mfactor() : 1)) - gapsize,
        (nmaster > 0 ? static_cast<int>(screen_dim.h / nmaster) : 1) - gapsize
    };

    dim_t stack_dim = {
        screen_dim.w - (nmaster > 0 ? master_dim.w : 0) - (nmaster ? 2 : 1) * gapsize,
        screen_dim.h - gapsize
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    pos_t stack_pos  = {
        (nmaster > 0 ? master_dim.w + 1: 0) + m_ewmh.get_left_strut() + (nmaster ? 2 : 1) * gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    if (workspace.is_mirrored() && clients.size() > nmaster && nmaster != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
    }

    { // tile master clients
        for (::std::size_t i = 0; nmaster && i < nmaster - 1; ++i) {
            clients[i]->resize(master_dim, true).move(master_pos, true);
            master_pos.y += master_dim.h + gapsize + 1;
        }

        if (nmaster)
            clients[nmaster - 1]->resize({master_dim.w,
                screen_dim.h + m_ewmh.get_top_strut() - master_pos.y}, true).move(master_pos, true);
    }

    // tile stack clients
    for (::std::size_t i = nmaster; i < clients.size(); ++i)
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

    auto root_attrs = x_data::get_attributes(x_data::g_root);
    ::std::size_t nmaster = ::std::min(static_cast<::std::size_t>(clients.size()), workspace.get_nmaster());
    int gapsize = workspace.get_gapsize();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - gapsize - (nmaster && nmaster != clients.size() ? 3 : 2),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - gapsize - 2
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (nmaster < clients.size() ? workspace.get_mfactor() : 1)) - gapsize,
        screen_dim.h - gapsize
    };

    dim_t stack_dim = {
        screen_dim.w - (nmaster > 0 ? master_dim.w : 0) - (nmaster ? 2 : 1) * gapsize,
        screen_dim.h - gapsize
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    pos_t stack_pos  = {
        (nmaster > 0 ? master_dim.w + 1: 0) + m_ewmh.get_left_strut() + (nmaster ? 2 : 1) * gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    if (workspace.is_mirrored() && clients.size() > nmaster && nmaster != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
    }

    // tile master clients
    for (::std::size_t i = 0; i < nmaster; ++i)
        clients[i]->resize(master_dim, true).move(master_pos, true);

    // tile stack clients
    for (::std::size_t i = nmaster; i < clients.size(); ++i)
        clients[i]->resize(stack_dim, true).move(stack_pos, true);
}

void
layouthandler_t::layout_sdeck(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_data::get_attributes(x_data::g_root);
    ::std::size_t nmaster = ::std::min(static_cast<::std::size_t>(clients.size()), workspace.get_nmaster());
    int gapsize = workspace.get_gapsize();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - 2 * gapsize - (nmaster && nmaster != clients.size() ? 3 : 2),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - 2 * gapsize - 2
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (nmaster < clients.size() ? workspace.get_mfactor() : 1))
            - (nmaster == clients.size() ? 2 : 1) * gapsize,
        (nmaster > 0 ? static_cast<int>(screen_dim.h / nmaster) : 1)
    };

    dim_t stack_dim = {
        screen_dim.w - (nmaster > 0 ? master_dim.w : 0) - 2 * gapsize,
        screen_dim.h
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + 2 * gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    pos_t stack_pos  = {
        master_pos.x + (nmaster > 0 ? master_dim.w + 1: 0),
        m_ewmh.get_top_strut() + gapsize
    };

    if (workspace.is_mirrored() && clients.size() > nmaster && nmaster != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
    }

    { // tile master clients
        for (::std::size_t i = 0; nmaster && i < nmaster - 1; ++i) {
            clients[i]->resize(master_dim, true).move(master_pos, true);
            master_pos.y += master_dim.h + 1;
        }

        if (nmaster)
            clients[nmaster - 1]->resize({master_dim.w,
                screen_dim.h + m_ewmh.get_top_strut() + gapsize - master_pos.y}, true).move(master_pos, true);
    }

    // tile stack clients
    for (::std::size_t i = nmaster; i < clients.size(); ++i)
        clients[i]->resize(stack_dim, true).move(stack_pos, true);
}

void
layouthandler_t::layout_sdoubledeck(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_data::get_attributes(x_data::g_root);
    ::std::size_t nmaster = ::std::min(static_cast<::std::size_t>(clients.size()), workspace.get_nmaster());
    int gapsize = workspace.get_gapsize();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - 2 * gapsize - (nmaster && nmaster != clients.size() ? 3 : 2),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - 2 * gapsize - 2
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (nmaster < clients.size() ? workspace.get_mfactor() : 1))
            - (nmaster == clients.size() ? 2 : 1) * gapsize,
        screen_dim.h
    };

    dim_t stack_dim = {
        screen_dim.w - (nmaster > 0 ? master_dim.w : 0) - 2 * gapsize,
        screen_dim.h
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + 2 * gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    pos_t stack_pos  = {
        master_pos.x + (nmaster > 0 ? master_dim.w + 1: 0),
        m_ewmh.get_top_strut() + gapsize
    };

    if (workspace.is_mirrored() && clients.size() > nmaster && nmaster != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
    }

    // tile master clients
    for (::std::size_t i = 0; i < nmaster; ++i)
        clients[i]->resize(master_dim, true).move(master_pos, true);

    // tile stack clients
    for (::std::size_t i = nmaster; i < clients.size(); ++i)
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

    auto root_attrs = x_data::get_attributes(x_data::g_root);
    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut(),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut()
    };

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
layouthandler_t::layout_pillar(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_data::get_attributes(x_data::g_root);
    ::std::size_t nmaster = ::std::min(static_cast<::std::size_t>(clients.size()), workspace.get_nmaster());
    ::std::size_t n_stack = clients.size() - nmaster;
    int gapsize = workspace.get_gapsize();

    bool has_leftstack = n_stack;
    bool has_rightstack = n_stack > 1;

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - (nmaster == clients.size() ? 1 : 2)
            * gapsize - (clients.size() > 1 && nmaster && nmaster != clients.size() ? 3 : 2),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - gapsize - 2
    };

    dim_t master_dim = {
        static_cast<int>(n_stack ? screen_dim.w * workspace.get_mfactor() : screen_dim.w) - gapsize,
        static_cast<int>(screen_dim.h / (nmaster > 1 ? nmaster : 1)) - gapsize
    };

    dim_t leftstack_dim;
    if (nmaster) {
        leftstack_dim.w = (screen_dim.w - master_dim.w) / (has_rightstack ? 2 : 1) - gapsize;
    } else {
        screen_dim.w += gapsize;
        leftstack_dim.w = screen_dim.w / (has_rightstack ? 2 : 1) - gapsize - (clients.size() == 1 ? 0 : 1);
    }
    leftstack_dim.h = screen_dim.h / (has_leftstack ? ::std::ceil((float)n_stack / 2) : 1) - gapsize;

    dim_t rightstack_dim;
    if (nmaster) {
        rightstack_dim.w = (screen_dim.w - master_dim.w) / (has_rightstack ? 2 : 1) - gapsize;
    } else {
        rightstack_dim.w = screen_dim.w / (has_rightstack ? 2 : 1) - gapsize;
    }
    rightstack_dim.h = screen_dim.h / (has_rightstack ? ::std::floor((float)n_stack / 2) : 1) - gapsize;

    pos_t leftstack_pos  = {
        m_ewmh.get_left_strut() + gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    pos_t master_pos = {
        leftstack_pos.x + leftstack_dim.w + gapsize + 1,
        m_ewmh.get_top_strut() + gapsize
    };

    pos_t rightstack_pos  = {
        master_pos.x + master_dim.w + gapsize + 1,
        m_ewmh.get_top_strut() + gapsize
    };

    if (!nmaster) {
        rightstack_pos.x = leftstack_pos.x + leftstack_dim.w + 1 + gapsize;
    }

    if (workspace.is_mirrored() && clients.size() > nmaster && nmaster != 0 && has_rightstack) {
        ::std::swap(leftstack_dim.w, rightstack_dim.w);
        ::std::swap(leftstack_pos.x, rightstack_pos.x);
    }

    if (!has_rightstack && nmaster) {
        master_pos.x = leftstack_pos.x;
        leftstack_pos.x = m_ewmh.get_left_strut() + master_dim.w + 1 + 2 * gapsize;
    }

    { // tile master clients (center pillar)
        for (::std::size_t i = 0; nmaster && i < nmaster - 1; ++i) {
            clients[i]->resize(master_dim, true).move(master_pos, true);
            master_pos.y += master_dim.h + gapsize + 1;
        }

        if (nmaster)
            clients[nmaster - 1]->resize({master_dim.w,
                screen_dim.h + m_ewmh.get_top_strut() - master_pos.y}, true).move(master_pos, true);
    }

    { // tile stack clients (left pillar)
        for (::std::size_t i = 0; ::std::ceil((float)n_stack / 2) && i < ::std::ceil((float)n_stack / 2) - 1; ++i) {
            clients[nmaster + i]->resize(leftstack_dim, true).move(leftstack_pos, true);
            leftstack_pos.y += leftstack_dim.h + gapsize + 1;
        }

        if (has_leftstack && ::std::ceil((float)n_stack / 2))
            clients[nmaster + ::std::ceil((float)n_stack / 2) - 1]->resize({leftstack_dim.w,
                screen_dim.h + m_ewmh.get_top_strut() - leftstack_pos.y}, true).move(leftstack_pos, true);
    }

    { // tile stack clients (right pillar)
        if (has_rightstack) {
            for (::std::size_t i = 0; ::std::floor((float)n_stack / 2) && i < ::std::floor((float)n_stack / 2) - 1; ++i) {
                clients[clients.size() - ::std::floor((float)n_stack / 2) + i]->resize(rightstack_dim,
                    true).move(rightstack_pos, true);
                rightstack_pos.y += rightstack_dim.h + gapsize + 1;
            }

            if (clients.size() && ::std::floor((float)n_stack / 2))
                clients[clients.size() - 1]->resize({rightstack_dim.w,
                    screen_dim.h + m_ewmh.get_top_strut() - rightstack_pos.y}, true).move(rightstack_pos, true);
        }
    }
}

void
layouthandler_t::layout_column(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    auto root_attrs = x_data::get_attributes(x_data::g_root);
    ::std::size_t nmaster = ::std::min(static_cast<::std::size_t>(clients.size()), workspace.get_nmaster());
    int gapsize = workspace.get_gapsize();

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - gapsize - (nmaster && nmaster != clients.size() ? 3 : 2),
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - gapsize - 2
    };

    dim_t master_dim = {
        static_cast<int>(screen_dim.w * (nmaster < clients.size() ? workspace.get_mfactor() : 1)),
        screen_dim.h - gapsize
    };

    if (!(!nmaster || nmaster == clients.size()))
        master_dim.w -= nmaster;

    dim_t stack_dim = {
        screen_dim.w - (nmaster > 0 ? master_dim.w : 0) - gapsize,
        screen_dim.h / static_cast<int>(nmaster < clients.size() ? (clients.size() - nmaster) : 1) - gapsize
    };

    pos_t master_pos = {
        m_ewmh.get_left_strut() + gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    pos_t stack_pos  = {
        (nmaster > 0 ? master_dim.w + 1: 0) + m_ewmh.get_left_strut() + gapsize,
        m_ewmh.get_top_strut() + gapsize
    };

    if (workspace.is_mirrored() && clients.size() > nmaster && nmaster != 0) {
        ::std::swap(master_dim.w, stack_dim.w);
        ::std::swap(master_pos.x, stack_pos.x);
        stack_dim.w -= gapsize;
        master_dim.w += gapsize;
    }

    if (nmaster) {
        master_dim.w = master_dim.w / nmaster - gapsize;
    }

    { // tile master clients
        for (::std::size_t i = 0; nmaster && i < nmaster - 1; ++i) {
            clients[i]->resize(master_dim, true).move(master_pos, true);
            master_pos.x += master_dim.w + gapsize + 1;
        }

        if (nmaster) {
            if (nmaster == clients.size())
                clients[nmaster - 1]->resize({screen_dim.w + m_ewmh.get_left_strut() - master_pos.x,
                    master_dim.h}, true).move(master_pos, true);
            else
                clients[nmaster - 1]->resize(master_dim, true).move(master_pos, true);
        }
    }

    { // tile stack clients
        for (::std::size_t i = nmaster; i < clients.size() - 1; ++i) {
            clients[i]->resize(stack_dim, true).move(stack_pos, true);
            stack_pos.y += stack_dim.h + gapsize + 1;
        }

        if (clients.size() > nmaster)
            clients.back()->resize({stack_dim.w,
                screen_dim.h + m_ewmh.get_top_strut() - stack_pos.y}, true).move(stack_pos, true);
    }
}

void
layouthandler_t::layout_monocle(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    int gapsize = workspace.get_gapsize();
    auto root_attrs = x_data::get_attributes(x_data::g_root);

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - 2 * gapsize - 2,
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - 2 * gapsize - 2
    };

    for (const auto& client : clients)
        client->resize(screen_dim, true).move({m_ewmh.get_left_strut() + gapsize,
            m_ewmh.get_top_strut() + gapsize}, true);
}

void
layouthandler_t::layout_center(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    int gapsize = workspace.get_gapsize();
    float mfactor = workspace.get_mfactor();
    auto root_attrs = x_data::get_attributes(x_data::g_root);

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - 2,
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - 2
    };

    dim_t center_dim = {
        (int)((screen_dim.w) * (mfactor > 0 ? mfactor : 1.0)) - gapsize,
        screen_dim.h - 3 * gapsize
    };

    pos_t center_pos = {
        m_ewmh.get_left_strut() + screen_dim.w / 2 - center_dim.w / 2,
        m_ewmh.get_top_strut() + screen_dim.h / 2 - center_dim.h / 2
    };

    for (const auto& client : clients)
        client->resize(center_dim, true).move(center_pos, true);
}

void
layouthandler_t::layout_centerstack(const user_workspace_t& workspace) const
{
    auto clients = workspace.get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t client) { return client->floating || client->fullscreen; }), clients.end());

    if (clients.empty())
        return;

    int gapsize = workspace.get_gapsize();
    float mfactor = workspace.get_mfactor();
    auto root_attrs = x_data::get_attributes(x_data::g_root);

    dim_t screen_dim = {
        root_attrs.w() - m_ewmh.get_left_strut() - m_ewmh.get_right_strut() - 2,
        root_attrs.h() - m_ewmh.get_top_strut() - m_ewmh.get_bottom_strut() - 2
    };

    dim_t center_dim = {
        (int)((screen_dim.w) * (mfactor > 0 ? mfactor : 1.0)) - gapsize,
        static_cast<int>((screen_dim.h - 3 * gapsize) / clients.size())
    };

    pos_t center_pos = {
        m_ewmh.get_left_strut() + screen_dim.w / 2 - center_dim.w / 2,
        m_ewmh.get_top_strut() + screen_dim.h / 2 - (int)(center_dim.h * clients.size()) / 2
    };

    for (const auto& client : clients) {
        client->resize(center_dim, true).move(center_pos, true);
        center_pos.y += center_dim.h + 1;
    }
}
