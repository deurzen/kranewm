#include "workspace.hh"


userworkspace_t&
userworkspace_t::set_layout(LayoutType _layout)
{
    LayoutType current_layout = layout;
    if (_layout == LT_TOGGLE)
        layout = previous_layout;
    else
        layout = _layout;

    previous_layout = current_layout;
    return *this;
}

void
userworkspace_t::arrange() const
{

}
