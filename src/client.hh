//TODO update_child_offset from x_handler

#include "util.hh"
#include "x_wrapper/window.hh"

#include <set>


enum ClientEffect {
    NO_EFFECT = 0,
    MAP       = 1 << 0,
    WITHDRAW  = 1 << 1,
    ICONIFY   = 1 << 2
};


struct SizeConstraints
{
    SizeConstraints(Size _base, Size _inc, Size _max, Size _min, Range<float> _aspect)
      : base(_base),
        inc(_inc),
        max(_max),
        min(_min),
        aspect(_aspect)
    {}

    inline bool operator==(const SizeConstraints& size_constraints) const
    {
        return size_constraints.base == base
            && size_constraints.inc == inc
            && size_constraints.max == max
            && size_constraints.min == min
            && size_constraints.aspect == aspect;
    }

    Size base;
    Size inc;
    Size max;
    Size min;
    Range<float> aspect;
};


typedef struct client_t* client_ptr_t;
typedef struct client_t
{
    client_t(x_wrapper::window_t _win, x_wrapper::window_t _frame,
        Pos _pos, Size _size, const SizeConstraints& _size_constraints,
        bool _floating, bool _fullscreen, bool _shaded)
      : win(_win),
        frame(_frame),
        mr_indicator(None),
        float_indicator(None),
        pos(_pos),
        float_pos(_pos),
        size(_size),
        float_size(_size),
        size_constraints(_size_constraints),
        effect(NO_EFFECT),
        floating(_floating),
        fullscreen(_fullscreen),
        shaded(_shaded),
        iconified(false),
        urgent(false),
        parent(nullptr)
    {}

    x_wrapper::window_t      win;
    x_wrapper::window_t      frame;
    x_wrapper::window_t      mr_indicator;
    x_wrapper::window_t      float_indicator;
    Pos                      pos;
    Pos                      float_pos;
    Size                     size;
    Size                     float_size;
    SizeConstraints          size_constraints;
    ClientEffect             effect;
    bool                     floating;
    bool                     fullscreen;
    bool                     shaded;
    bool                     iconified;
    bool                     urgent;
    client_ptr_t             parent;
    ::std::set<client_ptr_t> children;
}* client_ptr_t;
