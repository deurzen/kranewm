#ifndef __KRANEWM__X_MODEL__GUARD__
#define __KRANEWM__X_MODEL__GUARD__

#include "floating.hh"
#include "constraints.hh"
#include "x_wrapper/window.hh"


class x_model
{
public:
    x_model()
        : m_moveresize(nullptr)
    {};

    static bool update_hints(client_ptr_t, XSizeHints);
    void apply_hints(Pos&, Size&, SizeConstraints) const;

    void enter_move(client_ptr_t, Pos);
    void enter_resize(client_ptr_t, Pos);

    Pos update_pointer(Pos);

    client_ptr_t get_move_resize_client() const;
    MoveResizeState get_move_resize_state() const;
    Corner get_move_resize_corner() const;

    void exit_move_resize();

    bool is_valid() const;

private:
    moveresize_ptr_t m_moveresize;
    Pos m_pointer;

};

#endif//__KRANEWM__X_MODEL__GUARD__
