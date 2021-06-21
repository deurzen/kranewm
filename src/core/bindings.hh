#ifndef __BINDINGS_H_GUARD__
#define __BINDINGS_H_GUARD__

#include "../winsys/input.hh"
#include "client.hh"

#include <functional>
#include <optional>

class Model;

typedef
    std::function<void(Model&)>
    KeyAction;

typedef
    std::function<bool(Model&, Client_ptr)>
    MouseAction;

typedef
    std::unordered_map<winsys::KeyInput, KeyAction>
    KeyBindings;

typedef
    std::unordered_map<winsys::MouseInput, MouseAction>
    MouseBindings;

#endif//__BINDINGS_H_GUARD__
