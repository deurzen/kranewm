#ifndef __KRANEWM__X_DATA__MASK__GUARD__
#define __KRANEWM__X_DATA__MASK__GUARD__

#include <set>
#include <vector>

extern "C" {
#include <X11/Xutil.h>
}

namespace x_data
{
    extern ::std::set<uint8_t> g_unused_masks;
    extern ::std::vector<uint8_t> g_ignored_masks;
    extern uint8_t g_allowed_mask;

    extern unsigned clean_mask(unsigned);
}

#endif//__KRANEWM__X_DATA__MASK__GUARD__
