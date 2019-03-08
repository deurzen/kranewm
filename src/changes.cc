#include "changes.hh"


bool
changequeue_t::has_next() const
{
    return !m_changes.empty();
}

clientchange_ptr_t
changequeue_t::next()
{
    if (has_next()) {
        auto change = m_changes.front();
        m_changes.pop();
        return change;
    }

    return nullptr;
}

void
changequeue_t::add(clientchange_ptr_t change)
{
    m_changes.push(change);
}

void
changequeue_t::flush()
{
    clientchange_ptr_t change;
    while ((change = next()))
        delete change;
}
