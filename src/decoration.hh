#ifndef __KRANEWM__DECORATION__GUARD__
#define __KRANEWM__DECORATION__GUARD__

#include <string>


const ::std::string FONTNAME = "-*-terminus-*-*-*-*-*-*-*-*-*-*-*-*";

const int BORDER_HEIGHT = 3;

const unsigned long SIDEBAR_FG_COLOR = 0xF8EBE7;
const unsigned long SIDEBAR_BG_COLOR = 0x454745;

const unsigned long REG_COLOR = 0x333333;
const unsigned long SEL_COLOR = 0xED764D;
const unsigned long URG_COLOR = 0x772953;
const unsigned long DIS_COLOR = 0x2C001E;
const unsigned long SELSTICKY_COLOR = 0x77216F;
const unsigned long REGSTICKY_COLOR = 0x444444;

const unsigned long SIDEBAR_LAYOUT_COLOR           = 0x3DD6D0;
const unsigned long SIDEBAR_CONTEXT_COLOR          = 0x71F79F;
const unsigned long SIDEBAR_WORKSPACES_COLOR       = SIDEBAR_FG_COLOR;
const unsigned long SIDEBAR_ICONS_COLOR            = SIDEBAR_FG_COLOR;
const unsigned long SIDEBAR_ICONNUMBERS_COLOR      = SIDEBAR_LAYOUT_COLOR;
const unsigned long SIDEBAR_ACTIVE_WORKSPACE_COLOR = SEL_COLOR;
const unsigned long SIDEBAR_NSTICKY_COLOR          = SELSTICKY_COLOR;
const unsigned long SIDEBAR_NCLIENTS_COLOR         = SIDEBAR_WORKSPACES_COLOR;

const unsigned long MRIND_BG_COLOR     = REG_COLOR;
const unsigned long MRIND_BORDER_COLOR = SIDEBAR_FG_COLOR;
const unsigned long FSIND_BG_COLOR     = REG_COLOR;
const unsigned long FSIND_BORDER_COLOR = URG_COLOR;
const unsigned long FLIND_BG_COLOR     = REG_COLOR;
const unsigned long FLIND_BORDER_COLOR = SIDEBAR_FG_COLOR;

#endif//__KRANEWM__DECORATION__GUARD__
