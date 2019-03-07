#include "workspace.hh"


user_workspace_t&
user_workspace_t::set_layout(LayoutType _layout)
{
    LayoutType current_layout = layout;
    if (_layout == LT_TOGGLE)
        layout = previous_layout;
    else
        layout = _layout;

    previous_layout = current_layout;
    return *this;
}
