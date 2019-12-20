#include "persistent.hh"

#include "context.hh"


void
statehandler_t::load_state(context_ptr_t context)
{
    auto workspaces = context->get_workspaces();
    for (auto& workspace : *workspaces)
        ;
}

void
statehandler_t::save_state(context_ptr_t context)
{
    auto workspaces = context->get_workspaces();
    for (auto& workspace : *workspaces)
        ;
}

bool
statehandler_t::serialize(user_workspace_t::ws_cmp) const
{
    // TODO
    return true;
}

bool
statehandler_t::deserialize(user_workspace_t::ws_cmp) const
{
    // TODO
    return true;
}
