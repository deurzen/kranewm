#ifndef __KRANEWM__DECORATION__GUARD__
#define __KRANEWM__DECORATION__GUARD__

#include <string>


const ::std::string FONTNAME = "-*-terminus-*-*-*-*-*-*-*-*-*-*-*-*";

const int BORDER_HEIGHT = 3;

const unsigned long SIDEBAR_FG_COLOR = 0xF8EBE7;
const unsigned long SIDEBAR_BG_COLOR = 0x2D2D2D;

const unsigned long REG_COLOR = 0x333333;
const unsigned long SEL_COLOR = 0xED764D;
const unsigned long URG_COLOR = 0xF2777A;
const unsigned long DIS_COLOR = 0x6699CC;
const unsigned long SELSTICKY_COLOR = 0xCC99CC;
const unsigned long REGSTICKY_COLOR = 0x444444;

const unsigned long SIDEBAR_LAYOUT_COLOR           = 0xFFCC66;
const unsigned long SIDEBAR_CONTEXT_COLOR          = 0x99CC99;
const unsigned long SIDEBAR_WORKSPACES_COLOR       = SIDEBAR_FG_COLOR;
const unsigned long SIDEBAR_ICONS_COLOR            = SIDEBAR_FG_COLOR;
const unsigned long SIDEBAR_ICONNUMBERS_COLOR      = SIDEBAR_LAYOUT_COLOR;
const unsigned long SIDEBAR_ACTIVE_WORKSPACE_COLOR = SEL_COLOR;
const unsigned long SIDEBAR_NSTICKY_COLOR          = SELSTICKY_COLOR;
const unsigned long SIDEBAR_NCLIENTS_COLOR         = SIDEBAR_WORKSPACES_COLOR;
const unsigned long ROOT_WORKSPACE_COLOR           = 0xFFFFFF;

// moveresize
const unsigned long MRIND_BG_COLOR     = REG_COLOR;
const unsigned long MRIND_BORDER_COLOR = SIDEBAR_FG_COLOR;
// fullscreen
const unsigned long FSIND_BG_COLOR     = REG_COLOR;
const unsigned long FSIND_BORDER_COLOR = URG_COLOR;
// floating
const unsigned long FLIND_BG_COLOR     = REG_COLOR;
const unsigned long FLIND_BORDER_COLOR = SIDEBAR_FG_COLOR;
// invincible state
const unsigned long ININD_BG_COLOR     = REG_COLOR;
const unsigned long ININD_BORDER_COLOR = URG_COLOR;
// below state
const unsigned long BSIND_BG_COLOR     = REG_COLOR;
const unsigned long BSIND_BORDER_COLOR = SELSTICKY_COLOR;
// above state
const unsigned long ASIND_BG_COLOR     = REG_COLOR;
const unsigned long ASIND_BORDER_COLOR = DIS_COLOR;
// unmapped sidebar indicator
const unsigned long USIND_BG_COLOR     = 0x000000;
const unsigned long USIND_BORDER_COLOR = 0xFFFFFF;

#endif//__KRANEWM__DECORATION__GUARD__
