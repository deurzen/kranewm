#include "ipc.hh"

#include "input.hh"

#include <iostream>
#include <sstream>

static const ::std::string NORETURN = "\x18";


void
ipc_t::handle_ipc()
{
    int cli_fd = accept4(m_sock_fd, NULL, 0, SOCK_CLOEXEC);

    char cli_msg[BUFSIZ];
    ssize_t n = read(cli_fd, cli_msg, sizeof(cli_msg) - 1);

    if (cli_fd > 0 && n > 0) {
        cli_msg[n] = '\0';
        ::std::FILE* cli_stream = fdopen(cli_fd, "w");
        ::std::vector<::std::string> msg_tokens;

        for (ssize_t i = 0, j = 0; i < n; ++i) {
            if (!cli_msg[i]) {
                msg_tokens.push_back(cli_msg + j);
                j = i + 1;
            }
        }

        if (cli_stream) {
            process_message(msg_tokens, cli_fd);
            write(cli_fd, NORETURN.c_str(), NORETURN.size());
        } else
            warn("couldn't establish connection with client");

        close(cli_fd);
    }
}

void
ipc_t::process_message(::std::vector<::std::string>& tokens, int cli_fd)
{
    m_input.process_ipc_input(resolve_command(tokens, cli_fd));
}

commandbind_t
ipc_t::resolve_command(::std::vector<::std::string>& tokens, int cli_fd)
{
    if (tokens.size() && m_ipcbinds.count(tokens.front())) {
        ipcbind_t bind = m_ipcbinds.at(tokens.front());

        if (bind.is_bound()) {
            if (tokens.size() > 1)
                return fail_command("this command takes no arguments", cli_fd);

            return bind.get_bind();
        }

        if (tokens.size() < 2)
            return fail_command("no command argument provided", cli_fd);

        if (tokens.size() > 2)
            return fail_command("too many arguments provided", cli_fd);

        try {
            switch (bind.get_argtype()) {
            case argtype_t::d:   return { bind.get_op(), resolve_int(tokens.back()) };
            case argtype_t::f:   return { bind.get_op(), resolve_float(tokens.back()) };
            case argtype_t::i:   return { bind.get_op(), resolve_int_index(tokens.back()) };
            case argtype_t::c:   return { bind.get_op(), resolve_char_index(tokens.back()) };
            case argtype_t::lt:  return { bind.get_op(), resolve_layout(tokens.back()) };
            case argtype_t::dir: return { bind.get_op(), resolve_direction(tokens.back()) };
            default: break;
            }
        } catch (::std::invalid_argument& err) {
            return fail_command("unable to resolve argument (" + ::std::string(err.what()) + ")", cli_fd);
        }

    } else if (tokens.size())
        return fail_command("command `" + tokens.front() + "` not recognized", cli_fd);

    return fail_command("no valid command provided", cli_fd);
}

int
ipc_t::resolve_int(::std::string& arg)
{
    return ::std::stoi(arg);
}

float
ipc_t::resolve_float(::std::string& arg)
{
    return ::std::stof(arg);
}

int
ipc_t::resolve_char_index(::std::string& arg)
{
    if (arg.size() == 1 && range_t<char>::contains('a', 'i', arg[0]))
        return arg[0] - 'a' + 1;

    throw ::std::invalid_argument("received invalid index, expected char between 'a' and 'i'");
}

int
ipc_t::resolve_int_index(::std::string& arg)
{
    if (arg.size() == 1 && range_t<char>::contains('1', '9', arg[0]))
        return arg[0] - '0';

    throw ::std::invalid_argument("received invalid index, expected integer between 1 and 9");
}

layout_t
ipc_t::resolve_layout(::std::string& arg)
{
    if (!arg.compare("toggle"))      return layout_t::toggle;
    if (!arg.compare("floating"))    return layout_t::floating;
    if (!arg.compare("tile"))        return layout_t::tile;
    if (!arg.compare("stick"))       return layout_t::stick;
    if (!arg.compare("deck"))        return layout_t::deck;
    if (!arg.compare("doubledeck"))  return layout_t::doubledeck;
    if (!arg.compare("sdeck"))       return layout_t::sdeck;
    if (!arg.compare("sdoubledeck")) return layout_t::sdoubledeck;
    if (!arg.compare("grid"))        return layout_t::grid;
    if (!arg.compare("pillar"))      return layout_t::pillar;
    if (!arg.compare("column"))      return layout_t::column;
    if (!arg.compare("monocle"))     return layout_t::monocle;
    if (!arg.compare("center"))      return layout_t::center;
    if (!arg.compare("centerstack")) return layout_t::centerstack;

    throw ::std::invalid_argument("received invalid layout name");
}

direction_t
ipc_t::resolve_direction(::std::string& arg)
{
    if (!arg.compare("up"))    return direction_t::up;
    if (!arg.compare("right")) return direction_t::right;
    if (!arg.compare("down"))  return direction_t::down;
    if (!arg.compare("left"))  return direction_t::left;

    throw ::std::invalid_argument("received invalid direction");
}
