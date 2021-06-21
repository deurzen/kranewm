#include "decoration.hh"

using namespace winsys;

const ColorScheme ColorScheme::DEFAULT_COLOR_SCHEME = ColorScheme {
    .focused   = 0x8181A6,
    .fdisowned = 0xc1c1c1,
    .fsticky   = 0x5F8787,
    .unfocused = 0x333333,
    .udisowned = 0x999999,
    .usticky   = 0x444444,
    .urgent    = 0x87875F
};

const Decoration Decoration::NO_DECORATION = Decoration {
    std::nullopt,
    std::nullopt
};

const Decoration Decoration::FREE_DECORATION = Decoration {
    std::nullopt,
    Frame {
        Extents { 3, 1, 1, 1 },
        ColorScheme::DEFAULT_COLOR_SCHEME
    }
};


const Extents
Decoration::extents() const
{
    Extents extents = Extents { 0, 0, 0, 0 };

    if (border) {
        extents.left += 1;
        extents.right += 1;
        extents.top += 1;
        extents.bottom += 1;
    }

    if (frame) {
        extents.left += frame->extents.left;
        extents.right += frame->extents.right;
        extents.top += frame->extents.top;
        extents.bottom += frame->extents.bottom;
    }

    return extents;
}
