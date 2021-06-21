#ifndef __JUMP_H_GUARD__
#define __JUMP_H_GUARD__

#include "../winsys/common.hh"
#include "client.hh"
#include "workspace.hh"

#include <functional>
#include <string>
#include <utility>

class JumpSelector final
{
public:
    enum class SelectionCriterium
    {
        OnWorkspaceBySelector,
        ByNameEquals,
        ByClassEquals,
        ByInstanceEquals,
        ByNameContains,
        ByClassContains,
        ByInstanceContains,
        ForCondition,
    };

    JumpSelector(Index index, Workspace::ClientSelector::SelectionCriterium criterium)
        : m_tag(JumpSelectorTag::OnWorkspaceBySelector),
          m_workspace_selector(std::pair(index, criterium))
    {}

    JumpSelector(SelectionCriterium criterium, std::string&& str_)
        : m_string(str_)
    {
        switch (criterium) {
        case SelectionCriterium::ByNameEquals:       m_tag = JumpSelectorTag::ByNameEquals;       return;
        case SelectionCriterium::ByClassEquals:      m_tag = JumpSelectorTag::ByClassEquals;      return;
        case SelectionCriterium::ByInstanceEquals:   m_tag = JumpSelectorTag::ByInstanceEquals;   return;
        case SelectionCriterium::ByNameContains:     m_tag = JumpSelectorTag::ByNameContains;     return;
        case SelectionCriterium::ByClassContains:    m_tag = JumpSelectorTag::ByClassContains;    return;
        case SelectionCriterium::ByInstanceContains: m_tag = JumpSelectorTag::ByInstanceContains; return;
        default: return;
        }
    }

    JumpSelector(std::function<bool(const Client_ptr)>&& filter)
        : m_tag(JumpSelectorTag::ForCondition),
          m_filter(filter)
    {}

    ~JumpSelector()
    {
        switch (m_tag) {
        case JumpSelectorTag::OnWorkspaceBySelector:
        {
            (&m_workspace_selector)->std::pair<
                    Index,
                    Workspace::ClientSelector::SelectionCriterium
            >::~pair();

            return;
        }
        case JumpSelectorTag::ByNameEquals:       // fallthrough
        case JumpSelectorTag::ByClassEquals:      // fallthrough
        case JumpSelectorTag::ByInstanceEquals:   // fallthrough
        case JumpSelectorTag::ByNameContains:     // fallthrough
        case JumpSelectorTag::ByClassContains:    // fallthrough
        case JumpSelectorTag::ByInstanceContains:
        {
            (&m_string)->std::string::~string();

            return;
        }
        case JumpSelectorTag::ForCondition:
        {
            (&m_filter)->std::function<bool(const Client_ptr)>::~function();

            return;
        }
        default: return;
        }
    }

    SelectionCriterium
    criterium() const
    {
        switch (m_tag) {
        case JumpSelectorTag::OnWorkspaceBySelector: return SelectionCriterium::OnWorkspaceBySelector;
        case JumpSelectorTag::ByNameEquals:          return SelectionCriterium::ByNameEquals;
        case JumpSelectorTag::ByClassEquals:         return SelectionCriterium::ByClassEquals;
        case JumpSelectorTag::ByInstanceEquals:      return SelectionCriterium::ByInstanceEquals;
        case JumpSelectorTag::ByNameContains:        return SelectionCriterium::ByNameContains;
        case JumpSelectorTag::ByClassContains:       return SelectionCriterium::ByClassContains;
        case JumpSelectorTag::ByInstanceContains:    return SelectionCriterium::ByInstanceContains;
        case JumpSelectorTag::ForCondition:          return SelectionCriterium::ForCondition;
        default: Util::die("no associated criterium");
        }

        return {};
    }

    std::pair<Index, Workspace::ClientSelector::SelectionCriterium> const&
    workspace_selector() const
    {
        return m_workspace_selector;
    }

    std::string const&
    string_value() const
    {
        return m_string;
    }

    std::function<bool(const Client_ptr)> const&
    filter() const
    {
        return m_filter;
    }

private:
    enum class JumpSelectorTag
    {
        OnWorkspaceBySelector,
        ByNameEquals,
        ByClassEquals,
        ByInstanceEquals,
        ByNameContains,
        ByClassContains,
        ByInstanceContains,
        ForCondition,
    };

    JumpSelectorTag m_tag;

    union
    {
        std::pair<Index, Workspace::ClientSelector::SelectionCriterium> m_workspace_selector;
        std::string m_string;
        std::function<bool(const Client_ptr)> m_filter;
    };

};

#endif//__JUMP_H_GUARD__
