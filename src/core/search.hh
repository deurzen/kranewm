#ifndef __SEARCH_H_GUARD__
#define __SEARCH_H_GUARD__

#include "../winsys/common.hh"
#include "client.hh"
#include "workspace.hh"

#include <functional>
#include <string>
#include <utility>

class SearchSelector final
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

    SearchSelector(Index index, Workspace::ClientSelector::SelectionCriterium criterium)
        : m_tag(SearchSelectorTag::OnWorkspaceBySelector),
          m_workspace_selector(std::pair(index, criterium))
    {}

    SearchSelector(SelectionCriterium criterium, std::string&& str_)
        : m_string(str_)
    {
        switch (criterium) {
        case SelectionCriterium::ByNameEquals:       m_tag = SearchSelectorTag::ByNameEquals;       return;
        case SelectionCriterium::ByClassEquals:      m_tag = SearchSelectorTag::ByClassEquals;      return;
        case SelectionCriterium::ByInstanceEquals:   m_tag = SearchSelectorTag::ByInstanceEquals;   return;
        case SelectionCriterium::ByNameContains:     m_tag = SearchSelectorTag::ByNameContains;     return;
        case SelectionCriterium::ByClassContains:    m_tag = SearchSelectorTag::ByClassContains;    return;
        case SelectionCriterium::ByInstanceContains: m_tag = SearchSelectorTag::ByInstanceContains; return;
        default: return;
        }
    }

    SearchSelector(std::function<bool(const Client_ptr)>&& filter)
        : m_tag(SearchSelectorTag::ForCondition),
          m_filter(filter)
    {}

    ~SearchSelector()
    {
        switch (m_tag) {
        case SearchSelectorTag::OnWorkspaceBySelector:
        {
            (&m_workspace_selector)->std::pair<
                    Index,
                    Workspace::ClientSelector::SelectionCriterium
            >::~pair();

            return;
        }
        case SearchSelectorTag::ByNameEquals:       // fallthrough
        case SearchSelectorTag::ByClassEquals:      // fallthrough
        case SearchSelectorTag::ByInstanceEquals:   // fallthrough
        case SearchSelectorTag::ByNameContains:     // fallthrough
        case SearchSelectorTag::ByClassContains:    // fallthrough
        case SearchSelectorTag::ByInstanceContains:
        {
            (&m_string)->std::string::~string();

            return;
        }
        case SearchSelectorTag::ForCondition:
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
        case SearchSelectorTag::OnWorkspaceBySelector: return SelectionCriterium::OnWorkspaceBySelector;
        case SearchSelectorTag::ByNameEquals:          return SelectionCriterium::ByNameEquals;
        case SearchSelectorTag::ByClassEquals:         return SelectionCriterium::ByClassEquals;
        case SearchSelectorTag::ByInstanceEquals:      return SelectionCriterium::ByInstanceEquals;
        case SearchSelectorTag::ByNameContains:        return SelectionCriterium::ByNameContains;
        case SearchSelectorTag::ByClassContains:       return SelectionCriterium::ByClassContains;
        case SearchSelectorTag::ByInstanceContains:    return SelectionCriterium::ByInstanceContains;
        case SearchSelectorTag::ForCondition:          return SelectionCriterium::ForCondition;
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
    enum class SearchSelectorTag
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

    SearchSelectorTag m_tag;

    union
    {
        std::pair<Index, Workspace::ClientSelector::SelectionCriterium> m_workspace_selector;
        std::string m_string;
        std::function<bool(const Client_ptr)> m_filter;
    };

};

#endif//__SEARCH_H_GUARD__
