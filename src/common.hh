#ifndef __KRANEWM__COMMON__GUARD__
#define __KRANEWM__COMMON__GUARD__

#include <string>
#include <map>
#include <iostream>

extern "C" {
#include <X11/X.h>
}

const int MOVE_BUTTON        = 1;
const int RESIZE_BUTTON      = 3;
const int SCROLL_UP_BUTTON   = 4;
const int SCROLL_DOWN_BUTTON = 5;
const int CENTER_BUTTON      = 2;
const int BACKWARD_BUTTON    = 8;
const int FORWARD_BUTTON     = 9;
#ifndef DEBUG
const int MODMASK = Mod4Mask;
const int SECMASK = Mod1Mask;
#else
const int MODMASK = Mod1Mask;
const int SECMASK = Mod4Mask;
#endif

const int KB_RESIZE_INCREMENT = 5;
const int KB_MOVE_INCREMENT   = 5;

const ::std::map<unsigned, ::std::string> USER_WORKSPACES({
    // nr   name
    { 1,   "1:main" },
    { 2,   "2:web"  },
    { 3,   "3:term" },
    { 4,   "4"      },
    { 5,   "5"      },
    { 6,   "6"      },
    { 7,   "7"      },
    { 8,   "8"      },
    { 9,   "9"      },
});

const ::std::string WMNAME = "kranewm";
const ::std::string FONTNAME = "-*-terminus-*-*-*-*-*-*-*-*-*-*-*-*";

const int MIN_WINDOW_SIZE = 50;
const int BORDER_WIDTH    = 2;
const int SHADED_HEIGHT   = 14;
const int ICON_WIDTH      = 250;
const int ICON_HEIGHT     = 16;
const int ICON_BORDER     = 1;
const int ICON_ROW_LENGTH = 50;
const int SIDEBAR_WIDTH   = 12;

const unsigned long REG_BORDER_COLOR          = 0x404040;
const unsigned long SEL_BORDER_COLOR          = 0xC0C0C0;
const unsigned long URGENT_BORDER_COLOR       = 0xA52A2A;
const unsigned long MRIND_BG_COLOR            = 0x000000;
const unsigned long MRIND_BORDER_COLOR        = 0xffffff;
const unsigned long REG_ROOT_BG_COLOR         = 0x080808;
const unsigned long SCRATCHPAD_ROOT_BG_COLOR  = 0x121212;
const unsigned long REG_MRIND_BG_COLOR        = 0x080808;
const unsigned long SCRATCHPAD_MRIND_BG_COLOR = 0x121212;
const unsigned long REG_SHADED_BG_COLOR       = 0x404040;
const unsigned long SEL_SHADED_BG_COLOR       = 0x808080;

#endif//__KRANEWM__COMMON__GUARD__
