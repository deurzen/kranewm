#ifndef __KRANEWM__X_MODEL__GUARD__
#define __KRANEWM__X_MODEL__GUARD__

#include "floating.hh"
#include "constraints.hh"
#include "x_wrapper/window.hh"
#include "x_wrapper/hints.hh"


class x_model
{
public:
    x_model()
        : m_moveresize(nullptr)
    {};

    static bool update_hints(client_ptr_t, x_wrapper::sizehints_t);

    void enter_move(client_ptr_t, Pos);
    void enter_resize(client_ptr_t, Pos);

    Pos update_pointer(Pos);

    inline moveresize_ptr_t moveresize() const { return m_moveresize; }

    void exit_move_resize();

    bool is_valid() const;

private:
    moveresize_ptr_t m_moveresize;
    Pos m_pointer;

};

#endif//__KRANEWM__X_MODEL__GUARD__
