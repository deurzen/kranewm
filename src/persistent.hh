#ifndef __KRANEWM_PERSISTENT_GUARD__
#define __KRANEWM_PERSISTENT_GUARD__

#include "workspace.hh"

// fwd decl
typedef class context_t* context_ptr_t;

class statehandler_t
{
public:
    explicit statehandler_t() = default;

    void load_state(context_ptr_t);
    void save_state(context_ptr_t);

    bool serialize(user_workspace_t::ws_cmp) const;
    bool deserialize(user_workspace_t::ws_cmp) const;

};


#endif//__KRANEWM_PERSISTENT_GUARD__
