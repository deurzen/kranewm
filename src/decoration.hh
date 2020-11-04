#ifndef __KRANEWM__DECORATION__GUARD__
#define __KRANEWM__DECORATION__GUARD__

#include <string>



const int BORDER_HEIGHT = 3;

const unsigned long SIDEBAR_FG_COLOR = 0xAEA79F;
const unsigned long SIDEBAR_BG_COLOR = 0x1D1D1D;

const unsigned long REG_COLOR = 0x191A2A;
const unsigned long SEL_COLOR = 0xE95420;
const unsigned long URG_COLOR = 0x31182F;
const unsigned long DIS_COLOR = 0x636363;
const unsigned long SELSTICKY_COLOR = 0x3802CA;
const unsigned long REGSTICKY_COLOR = 0x2C001E;

const unsigned long SIDEBAR_LAYOUT_COLOR           = 0xF7F7F7;
const unsigned long SIDEBAR_CONTEXT_COLOR          = 0xAEA79F;
const unsigned long SIDEBAR_WORKSPACES_COLOR       = SIDEBAR_FG_COLOR;
const unsigned long SIDEBAR_ICONS_COLOR            = SIDEBAR_FG_COLOR;
const unsigned long SIDEBAR_ICONNUMBERS_COLOR      = SIDEBAR_LAYOUT_COLOR;
const unsigned long SIDEBAR_ACTIVE_WORKSPACE_COLOR = 0xE95825;
const unsigned long SIDEBAR_NSTICKY_COLOR          = SELSTICKY_COLOR;
const unsigned long SIDEBAR_NCLIENTS_COLOR         = SIDEBAR_WORKSPACES_COLOR;
const unsigned long ROOT_WORKSPACE_COLOR           = 0x1D1D1D;

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
const unsigned long USIND_BG_COLOR     = 0x1D1D1D;
const unsigned long USIND_BORDER_COLOR = 0xAEA79F;

#endif//__KRANEWM__DECORATION__GUARD__
