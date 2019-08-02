#ifndef __KRANEWM_ICONIFICATION_GUARD__
#define __KRANEWM_ICONIFICATION_GUARD__

#include "common.hh"
#include "decoration.hh"

#include "x-data/window.hh"

enum class applicationtype_t {
    other,
    browser,
    terminal,
    editor,
    video,
    audio,
};


static const ::std::map<applicationtype_t, char> ICON_CHARACTERS({
//    type                         name
    { applicationtype_t::other,    'o' },
    { applicationtype_t::browser,  'b' },
    { applicationtype_t::terminal, 't' },
    { applicationtype_t::editor,   'e' },
    { applicationtype_t::video,    'v' },
    { applicationtype_t::audio,    'a' },
});


// fwd decls
typedef class client_t* client_ptr_t;


typedef struct moveresize_t
{
    moveresize_t(client_ptr_t _client)
      : client(_client)
    {}

    ~moveresize_t()
    {}

    void process_iconification();
    void process_deiconification();

    client_ptr_t client;

}* moveresize_ptr_t;

#endif//__KRANEWM_ICONIFICATION_GUARD__
