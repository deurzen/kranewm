#ifndef __KRANEWM__COMMON__GUARD__
#define __KRANEWM__COMMON__GUARD__

#include <iostream>
#include <map>
#include <string>

extern "C" {
#include <X11/X.h>
}


const ::std::string WMNAME = "kranewm";

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

const int KB_RESIZE_INCREMENT = 15;
const int KB_MOVE_INCREMENT   = 15;

const int MAX_GAPSIZE = 250;
const int MAX_NMASTER = 50;

const int MIN_WINDOW_SIZE = 50;
const int SIDEBAR_WIDTH   = 12;
const int SNAP_OFFSET     = 15;

const bool ALLOW_FOCUSSTEAL = true;

#endif//__KRANEWM__COMMON__GUARD__
