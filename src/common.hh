#ifndef __KRANEWM__COMMON__GUARD__
#define __KRANEWM__COMMON__GUARD__

#include <string>
#include <map>
#include <iostream>

extern "C" {
#include <X11/X.h>
}


const ::std::string WMNAME = "kranewm";
const ::std::string FONTNAME = "-*-terminus-*-*-*-*-*-*-*-*-*-*-*-*";

const int LEFT_BUTTON        = 1;
const int RIGHT_BUTTON       = 3;
const int SCROLL_UP_BUTTON   = 4;
const int SCROLL_DOWN_BUTTON = 5;
const int MIDDLE_BUTTON      = 2;
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

const int MIN_WINDOW_SIZE = 50;
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
