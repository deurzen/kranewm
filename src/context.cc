#include "context.hh"

void
context_t::arrange() const
{
    for (auto& workspace : m_workspaces)
        workspace->arrange();
}

char
context_t::get_letter() const
{
    return m_letter;
}

::std::vector<user_workspace_ptr_t>&
context_t::get_workspaces()
{
    return m_workspaces;
}


bool
context_t::empty() const
{
    for (auto& workspace : m_workspaces)
        if (!workspace->empty())
            return false;

    return true;
}

bool
context_t::contains(client_ptr_t client) const
{
    for (auto& workspace : m_workspaces)
        if (workspace->contains(client))
            return true;

    return false;
}


const user_workspace_ptr_t
context_t::get_activated() const
{
    return m_activated;
}

void
context_t::set_activated(user_workspace_ptr_t workspace)
{
    m_activated = workspace;
}


context_t&
context_t::add_workspace(user_workspace_ptr_t workspace)
{
    m_workspaces.push_back(workspace);
    return *this;
}

context_t&
context_t::remove_workspace(user_workspace_ptr_t workspace)
{
    erase_remove(m_workspaces, workspace);
    return *this;
}


context_t&
context_t::next()
{
    unsigned workspace = m_activated->get_number();
    workspace %= m_workspaces.size();
    m_activated = m_workspaces[workspace];
    return *this;
}

context_t&
context_t::prev()
{
    unsigned workspace = m_activated->get_number() - 1;
    workspace = (workspace == 0) ? m_workspaces.size() : workspace;
    m_activated = m_workspaces[workspace];
    return *this;
}

context_t&
context_t::move_up()
{

    return *this;
}

context_t&
context_t::move_down()
{

    return *this;
}


context_t&
context_t::rotate_forward()
{
    if (m_workspaces.size() > 1) {
        auto from_it = m_workspaces.begin();
        auto to_it   = m_workspaces.end() - 1; // TODO check
        ::std::rotate(from_it, from_it + 1, to_it);
    }
    return *this;
}

context_t&
context_t::rotate_backward()
{ // TODO check/rewrite
    if (m_workspaces.size() > 1) {
        auto from_it = m_workspaces.rbegin();
        auto to_it   = m_workspaces.rend() - 1;
        ::std::rotate(from_it, from_it + 1, to_it);
    }
    return *this;
}


context_t&
context_t::add_all_client(client_ptr_t client)
{
    for (auto& workspace : m_workspaces)
        workspace->add_client(client);
    return *this;
}

context_t&
context_t::remove_all_client(client_ptr_t client)
{
    for (auto& workspace : m_workspaces)
        workspace->remove_client(client);
    return *this;
}


context_t&
context_t::set_all_nmaster(unsigned nmaster)
{
    for (auto& workspace : m_workspaces)
        workspace->set_nmaster(nmaster);
    return *this;
}

context_t&
context_t::set_all_gap_size(unsigned gap_size)
{
    for (auto& workspace : m_workspaces)
        workspace->set_gap_size(gap_size);
    return *this;
}

context_t&
context_t::set_all_mfactor(float mfactor)
{
    for (auto& workspace : m_workspaces)
        workspace->set_mfactor(mfactor);
    return *this;
}

context_t&
context_t::set_all_layout(layout_t layout)
{
    for (auto& workspace : m_workspaces)
        workspace->set_layout(layout);
    return *this;
}
