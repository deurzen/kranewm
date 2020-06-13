#ifndef __KRANEWM__X_MODEL__GUARD__
#define __KRANEWM__X_MODEL__GUARD__

#include "x-data/window.hh"
#include "x-data/hints.hh"

// fwd decls
typedef class client_t* client_ptr_t;
typedef class moveresize_t* moveresize_ptr_t;


class x_model_t final
{
public:
    x_model_t()
      : m_moveresize(nullptr)
    {}

    static bool update_hints(client_ptr_t, x_data::sizehints_t);

    void enter_move(client_ptr_t, pos_t);
    void enter_resize(client_ptr_t, pos_t);

    pos_t update_pointer(pos_t);

    inline moveresize_ptr_t moveresize() const { return m_moveresize; }

    void exit_move_resize();

    bool is_valid() const;

private:
    moveresize_ptr_t m_moveresize;
    pos_t m_pointer;

};

#endif//__KRANEWM__X_MODEL__GUARD__
