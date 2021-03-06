#ifndef __KRANEWM_CONTEXT_GUARD__
#define __KRANEWM_CONTEXT_GUARD__

#include "workspace.hh"

#include <vector>
#include <optional>


static const ::std::map<char, ::std::string> CONTEXTS({
//    lr    name
    { 'a',  "default" },
    { 'b',  "work"    },
    { 'c',  ""        },
    { 'd',  ""        },
    { 'e',  ""        },
    { 'f',  ""        },
    { 'g',  ""        },
    { 'h',  ""        },
    { 'i',  ""        },
});


typedef class context_t* context_ptr_t;

typedef class context_t
{
public:
    context_t(char letter, const ::std::string name, ewmh_t& ewmh, bool initialize = false)
      : m_ewmh(ewmh),
        m_letter(letter),
        m_name(name),
        m_is_initialized(initialize),
        m_nsticky(0),
        m_previous(nullptr),
        m_activated(nullptr),
        m_marked(nullptr),
        m_jumped_from(nullptr)
    {
        if (initialize)
            context_t::initialize();
    }

    ~context_t()
    {
        for (auto& workspace : m_workspaces)
            delete workspace;
    }

    inline explicit
    operator bool() const
    {
        return is_initialized();
    }

    inline bool
    is_initialized() const
    {
        return m_is_initialized;
    }

    void initialize();

    void arrange() const;

    char get_letter() const;
    ::std::size_t get_number() const;
    ::std::size_t get_index() const;

    const ::std::vector<user_workspace_ptr_t>* get_workspaces();

    bool empty() const;
    bool contains(client_ptr_t) const;

    const user_workspace_ptr_t get_previous() const;
    const user_workspace_ptr_t get_activated() const;
    void set_previous(user_workspace_ptr_t);
    void set_activated(user_workspace_ptr_t);

    const client_ptr_t get_marked() const;
    const client_ptr_t get_jumped_from() const;
    void set_marked(client_ptr_t = nullptr);
    void set_jumped_from(client_ptr_t);

    context_t& add_workspace(user_workspace_ptr_t);
    context_t& remove_workspace(user_workspace_ptr_t);

    context_t& add_all_client(client_ptr_t);
    context_t& remove_all_client(client_ptr_t);

    context_t& set_all_nmaster(::std::size_t);
    context_t& set_all_gapsize(::std::size_t);
    context_t& set_all_mfactor(float);
    context_t& set_all_layout(layout_t);

    ::std::size_t get_nsticky() const;
    ::std::size_t get_nnonsticky(::std::size_t) const;
    void record_sticky(::std::optional<unsigned> = ::std::nullopt);
    void erase_sticky(::std::optional<unsigned> = ::std::nullopt);

private:
    ewmh_t& m_ewmh;

    char m_letter;
    ::std::string m_name;
    bool m_is_initialized;
    ::std::size_t m_nsticky;

    ::std::vector<user_workspace_ptr_t> m_workspaces;
    user_workspace_ptr_t m_previous;
    user_workspace_ptr_t m_activated;

    client_ptr_t m_marked;
    client_ptr_t m_jumped_from;

}* context_ptr_t;

#endif//__KRANEWM_CONTEXT_GUARD__
