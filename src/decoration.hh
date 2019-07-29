#ifndef __KRANEWM__DECORATION__GUARD__
#define __KRANEWM__DECORATION__GUARD__

#include <string>


const ::std::string FONTNAME = "-*-terminus-*-*-*-*-*-*-*-*-*-*-*-*";

const int BORDER_HEIGHT   = 3;
const int ICON_WIDTH      = 250;
const int ICON_HEIGHT     = 16;
const int ICON_BORDER     = 1;
const int ICON_ROW_LENGTH = 50;

const unsigned long SIDEBAR_FG_COLOR = 0xF5E2BD;
const unsigned long SIDEBAR_BG_COLOR = 0x080808;

const unsigned long REG_COLOR = 0x333333;
const unsigned long SEL_COLOR = 0xDEB477;
const unsigned long URG_COLOR = 0xDD614A;
const unsigned long SELSTICKY_COLOR = 0xD3A26D;
const unsigned long REGSTICKY_COLOR = 0x444444;

const unsigned long SIDEBAR_LAYOUT_COLOR           = 0x7E99A8;
const unsigned long SIDEBAR_CONTEXT_COLOR          = 0xB0BC98;
const unsigned long SIDEBAR_WORKSPACES_COLOR       = SIDEBAR_FG_COLOR;
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
