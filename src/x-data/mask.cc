#include "mask.hh"

using namespace x_data;

::std::set<uint8_t> x_data::g_unused_masks({
    0, LockMask, Mod2Mask, Mod3Mask
});

::std::vector<uint8_t> x_data::g_ignored_masks([](){
    ::std::set<uint8_t> to_ignore;
    to_ignore.emplace();

    for (auto&& mask: g_unused_masks) {
        ::std::vector<uint8_t> mask_disjunction;

        for (auto m: to_ignore) {
            uint8_t next_mask = m | mask;
            to_ignore.insert(next_mask);
        }

        to_ignore.insert(begin(mask_disjunction), end(mask_disjunction));
    }

    return ::std::vector<uint8_t>(to_ignore.begin(), to_ignore.end());
}());

uint8_t x_data::g_allowed_mask([](){
    uint8_t to_ignore = 0;
    for (auto mask : g_unused_masks)
        to_ignore |= mask;

    return ~to_ignore;
}());

unsigned
x_data::clean_mask(unsigned mask)
{
    return mask & g_allowed_mask;
}
