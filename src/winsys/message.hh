#ifndef __WINSYS_MESSAGE_H_GUARD__
#define __WINSYS_MESSAGE_H_GUARD__

#include "common.hh"
#include "geometry.hh"
#include "input.hh"
#include "window.hh"

#include <string>
#include <variant>
#include <deque>

namespace winsys
{

    struct CommandMessage final
    {
        std::deque<std::string> args;
    };

    struct ConfigMessage final
    {
        std::deque<std::string> args;
    };

    struct WindowMessage final
    {
        std::deque<std::string> args;
    };

    struct WorkspaceMessage final
    {
        std::deque<std::string> args;
    };

    struct QueryMessage final
    {
        std::deque<std::string> args;
    };

    typedef std::variant<
        std::monostate,
        CommandMessage,
        ConfigMessage,
        WindowMessage,
        WorkspaceMessage,
        QueryMessage
    > Message;

}

#endif//__WINSYS_MESSAGE_H_GUARD__
