#ifndef __KRANEWM_ICONIFICATION_GUARD__
#define __KRANEWM_ICONIFICATION_GUARD__

#include "common.hh"
#include "decoration.hh"

#include "x-data/window.hh"


enum class applicationtype_t
{
    other,
    terminal,
    browser,
    filing,
    productivity,
    editor,
    virtualization,
    imagery,
    audio,
    messaging,
    news,
    language,
    utility,
};

static const ::std::map<applicationtype_t, char> ICON_CHARACTERS({
//    type                               name
    { applicationtype_t::other,          'o' },
    { applicationtype_t::terminal,       't' },
    { applicationtype_t::browser,        'b' },
    { applicationtype_t::filing,         'f' },
    { applicationtype_t::productivity,   'p' },
    { applicationtype_t::editor,         'e' },
    { applicationtype_t::virtualization, 'v' },
    { applicationtype_t::imagery,        'i' },
    { applicationtype_t::audio,          'a' },
    { applicationtype_t::messaging,      'm' },
    { applicationtype_t::news,           'n' },
    { applicationtype_t::language,       'l' },
    { applicationtype_t::utility,        'u' },
});

static const ::std::map<::std::string, applicationtype_t> APPLICATION_TYPES({
//    class                   type
    { "URxvt",                applicationtype_t::terminal },
    { "XTerm",                applicationtype_t::terminal },
    { "st-256color",          applicationtype_t::terminal },

    { "Chromium",             applicationtype_t::browser },
    { "firefox",              applicationtype_t::browser },
    { "Firefox",              applicationtype_t::browser },
    { "qutebrowser",          applicationtype_t::browser },

    { "Thunar",               applicationtype_t::filing },
    { "Xfe",                  applicationtype_t::filing },
    { "qBittorrent",          applicationtype_t::filing },

    { "nixnote2",             applicationtype_t::productivity },

    { "Emacs",                applicationtype_t::editor },
    { "Gvim",                 applicationtype_t::editor },
    { "NetBeans IDE 8.2",     applicationtype_t::editor },
    { "calligrawords",        applicationtype_t::editor },
    { "kwrite",               applicationtype_t::editor },

    { "VirtualBox Manager",   applicationtype_t::virtualization },

    { "Gimp",                 applicationtype_t::imagery },
    { "Inkscape",             applicationtype_t::imagery },
    { "Pinta",                applicationtype_t::imagery },
    { "SimpleScreenRecorder", applicationtype_t::imagery },
    { "Zathura",              applicationtype_t::imagery },
    { "calibre",              applicationtype_t::imagery },
    { "mpv",                  applicationtype_t::imagery },
    { "vlc",                  applicationtype_t::imagery },

    { "Audacity",             applicationtype_t::audio },
    { "Rhythmbox",            applicationtype_t::audio },
    { "cantata",              applicationtype_t::audio },

    { "Artha",                applicationtype_t::language },
    { "GoldenDict",           applicationtype_t::language },

    { "Anki",                 applicationtype_t::utility },
    { "Gnucash",              applicationtype_t::utility },
    { "Gnumeric",             applicationtype_t::utility },
    { "Qalculate-gtk",        applicationtype_t::utility },
    { "calligrasheets",       applicationtype_t::utility },
});

#endif//__KRANEWM_ICONIFICATION_GUARD__
