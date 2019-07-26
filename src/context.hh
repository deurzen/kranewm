#ifndef __KRANEWM_CONTEXT_GUARD__
#define __KRANEWM_CONTEXT_GUARD__

#include "workspace.hh"

#include <vector>


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
          m_activated(nullptr)
    {
        if (initialize)
            context_t::initialize();
    }

    ~context_t()
    {
        for (auto& workspace : m_workspaces)
            delete workspace;
    }

    explicit operator bool() const
    {
        return m_is_initialized;
    }

    void initialize();

    void arrange() const;

    char get_letter() const;
    const ::std::vector<user_workspace_ptr_t>* get_workspaces();

    bool empty() const;
    bool contains(client_ptr_t) const;

    const user_workspace_ptr_t get_previous() const;
    const user_workspace_ptr_t get_activated() const;
    void set_previous(user_workspace_ptr_t);
    void set_activated(user_workspace_ptr_t);

    context_t& add_workspace(user_workspace_ptr_t);
    context_t& remove_workspace(user_workspace_ptr_t);

    context_t& add_all_client(client_ptr_t);
    context_t& remove_all_client(client_ptr_t);

    context_t& set_all_nmaster(unsigned);
    context_t& set_all_gap_size(unsigned);
    context_t& set_all_mfactor(float);
    context_t& set_all_layout(layout_t);

    unsigned get_nsticky() const;
    unsigned get_nnonsticky(unsigned) const;
    void record_sticky();
    void erase_sticky();

private:
    ewmh_t& m_ewmh;

    char m_letter;
    ::std::string m_name;
    bool m_is_initialized;
    unsigned m_nsticky;

    ::std::vector<user_workspace_ptr_t> m_workspaces;
    user_workspace_ptr_t m_previous;
    user_workspace_ptr_t m_activated;

}* context_ptr_t;

#endif//__KRANEWM_CONTEXT_GUARD__
