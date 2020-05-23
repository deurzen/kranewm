#include "context.hh"


void
context_t::initialize()
{
    for (auto&& [nr,name] : USER_WORKSPACES)
        m_workspaces.push_back(new user_workspace_t{nr, name.c_str(), m_ewmh});

    m_activated = m_workspaces.front();
    m_is_initialized = true;
}

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

size_t
context_t::get_index() const
{
    return m_letter - 'a';
}

const ::std::vector<user_workspace_ptr_t>*
context_t::get_workspaces()
{
    return &m_workspaces;
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

const user_workspace_ptr_t
context_t::get_previous() const
{
    return m_previous;
}

void
context_t::set_activated(user_workspace_ptr_t workspace)
{
    if (m_workspaces[workspace->get_index()] == workspace)
        m_activated = workspace;
}

void
context_t::set_previous(user_workspace_ptr_t workspace)
{
    if (m_workspaces[workspace->get_index()] == workspace)
        m_previous = workspace;
}

const client_ptr_t
context_t::get_marked() const
{
    return m_marked;
}

const client_ptr_t
context_t::get_jumped_from() const
{
    return m_jumped_from;
}

void
context_t::set_marked(client_ptr_t client)
{
    m_marked = client;
}

void
context_t::set_jumped_from(client_ptr_t client)
{
    m_jumped_from = client;
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
context_t::add_all_client(client_ptr_t client)
{
    for (auto& workspace : m_workspaces)
        workspace->add_family(client);
    return *this;
}

context_t&
context_t::remove_all_client(client_ptr_t client)
{
    for (auto& workspace : m_workspaces)
        workspace->remove_family(client);
    return *this;
}


context_t&
context_t::set_all_nmaster(::std::size_t nmaster)
{
    for (auto& workspace : m_workspaces)
        workspace->set_nmaster(nmaster);
    return *this;
}

context_t&
context_t::set_all_gapsize(::std::size_t gapsize)
{
    for (auto& workspace : m_workspaces)
        workspace->set_gapsize(gapsize);
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

::std::size_t
context_t::get_nsticky() const
{
    return m_nsticky;
}

::std::size_t
context_t::get_nnonsticky(::std::size_t workspace_nr) const
{
    return m_workspaces[workspace_nr]->get_all().size() - m_nsticky;
}

void
context_t::record_sticky(::std::optional<unsigned> number)
{
    if (number)
        m_nsticky += *number;
    else
        ++m_nsticky;
}

void
context_t::erase_sticky(::std::optional<unsigned> number)
{
    if (m_nsticky) {
        if (number && m_nsticky >= *number) {
            if (m_nsticky > *number)
                m_nsticky -= *number;
            else
                m_nsticky = 0;
        } else
            --m_nsticky;
    }
}
