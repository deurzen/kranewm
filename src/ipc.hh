#ifndef __KRANEWM__IPC__GUARD__
#define __KRANEWM__IPC__GUARD__

#include "commands.hh"
#include "util.hh"

#include "x-data/property.hh"
#include "x-data/display.hh"

#include <limits.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

class inputhandler_t;


const ::std::string IPC_PREFIX = "_" + uppercase(WMNAME) + "_IPC_";
const ::std::string SOCK_PATH_ENV = uppercase(WMNAME) + "_SOCKETPATH";
#ifndef DEBUG
const ::std::string DEFAULT_SOCK_PATH_PREFIX = "/tmp/" + WMNAME;
#else
const ::std::string DEFAULT_SOCK_PATH_PREFIX = "/tmp/D" + WMNAME;
#endif


enum class argtype_t
{
    d,        // int
    f,        // float
    lt,       // layout
    i,        // int index
    c,        // char index
    dir,      // direction
};

class ipcbind_t
{
public:
    ipcbind_t(commandbind_t commandbind = commandop_t::noop)
      : m_commandbind(commandbind),
        m_op_arg(::std::nullopt)
    {}

    ipcbind_t(commandop_t commandop, argtype_t arg)
      : m_commandbind(::std::nullopt),
        m_op_arg({commandop, arg})
    {}

    bool
    is_bound() const
    {
        return !!m_commandbind;
    }

    commandbind_t
    get_bind() const
    {
        return *m_commandbind;
    }

    commandop_t
    get_op() const
    {
        return m_op_arg->first;
    }

    argtype_t
    get_argtype() const
    {
        return m_op_arg->second;
    }

private:
    ::std::optional<commandbind_t> m_commandbind;
    ::std::optional<::std::pair<commandop_t, argtype_t>> m_op_arg;

};

typedef ::std::unordered_map<::std::string, ipcbind_t> ipcbinds_t;


class ipc_t
{
public:
    ipc_t(inputhandler_t& input)
      : m_enabled(true),
        m_input(input),
        m_sock_fd(-1),
        m_sock_address(),
        m_sock_path(),
        m_ipcbinds({
            // command                            operation                             arg
            {  "context",                     {   commandop_t::contextset,            { argtype_t::c   }                   } },
            {  "contextactiveclient",         {   commandop_t::clientcontext,         { argtype_t::c   }                   } },
            {  "deiconify",                   {   commandop_t::clientdeiconifyindex,  { argtype_t::d   }                   } },
            {  "growactiveclient",            {   commandop_t::clientgrow,            { argtype_t::dir }                   } },
            {  "layout",                      {   commandop_t::workspacelayout,       { argtype_t::lt  }                   } },
            {  "moveactiveclient",            {   commandop_t::clientmove,            { argtype_t::dir }                   } },
            {  "profileload",                 {   commandop_t::profileload,           { argtype_t::d   }                   } },
            {  "profilesave",                 {   commandop_t::profilesave,           { argtype_t::d   }                   } },
            {  "shrinkactiveclient",          {   commandop_t::clientshrink,          { argtype_t::dir }                   } },
            {  "workspace",                   {   commandop_t::workspaceset,          { argtype_t::i   }                   } },
            {  "workspaceactiveclient",       {   commandop_t::clientworkspace,       { argtype_t::i   }                   } },
            // command                            bind
            {  "aboveactiveclient",           { { commandop_t::clientabove }                                               } },
            {  "allbackward",                 { { commandop_t::allbackward }                                               } },
            {  "allforward",                  { { commandop_t::allforward }                                                } },
            {  "belowactiveclient",           { { commandop_t::clientbelow }                                               } },
            {  "centeractiveclient",          { { commandop_t::clientcenter }                                              } },
            {  "decgapsize",                  { { commandop_t::workspacegapsize, -1 }                                      } },
            {  "decmfactor",                  { { commandop_t::workspacemfactor, -.05f }                                   } },
            {  "decnmaster",                  { { commandop_t::workspacenmaster, -1 }                                      } },
            {  "deiconifypop",                { { commandop_t::deiconifypop }                                              } },
            {  "disownactiveclient",          { { commandop_t::clientdisown }                                              } },
            {  "floatactiveclient",           { { commandop_t::clientfloat }                                               } },
            {  "floatingpreservedim",         { { commandop_t::workspacelayoutpreservedim, layout_t::floating }            } },
            {  "focusbackward",               { { commandop_t::focusbackward }                                             } },
            {  "focusforward",                { { commandop_t::focusforward }                                              } },
            {  "fullscreenactiveclient",      { { commandop_t::clientfullscreen }                                          } },
            {  "gapsizereset",                { { commandop_t::workspacegapsize, 0 }                                       } },
            {  "iconifyactiveclient",         { { commandop_t::clienticonify }                                             } },
            {  "incgapsize",                  { { commandop_t::workspacegapsize,  1 }                                      } },
            {  "incmfactor",                  { { commandop_t::workspacemfactor,  .05f }                                   } },
            {  "incnmaster",                  { { commandop_t::workspacenmaster,  1 }                                      } },
            {  "inwindowactiveclient",        { { commandop_t::clientinwindow }                                            } },
            {  "jumpmark",                    { { commandop_t::clientmarkjump }                                            } },
            {  "killactiveclient",            { { commandop_t::clientkill }                                                } },
            {  "lastjump",                    { { commandop_t::clientlastjump }                                            } },
            {  "markactiveclient",            { { commandop_t::clientmarkset }                                             } },
            {  "masterbackward",              { { commandop_t::masterbackward }                                            } },
            {  "masterforward",               { { commandop_t::masterforward }                                             } },
            {  "masterjump",                  { { commandop_t::clientmasterjump }                                          } },
            {  "movebackwardactiveclient",    { { commandop_t::clientmovebackward }                                        } },
            {  "movedownstackbackward",       { { commandop_t::floatingconditional,
                                                    new commandbind_t{ commandop_t::clientmove, direction_t::down },
                                                    new commandbind_t{ commandop_t::stackbackward }
                                              } } },
            {  "moveforwardactiveclient",     { { commandop_t::clientmoveforward }                                         } },
            {  "moveleftmasterforward",       { { commandop_t::floatingconditional,
                                                    new commandbind_t{ commandop_t::clientmove, direction_t::left },
                                                    new commandbind_t{ commandop_t::masterforward }
                                              } } },
            {  "moverightmasterbackward",     { { commandop_t::floatingconditional,
                                                    new commandbind_t{ commandop_t::clientmove, direction_t::right },
                                                    new commandbind_t{ commandop_t::masterbackward }
                                              } } },
            {  "moveupstackforward",          { { commandop_t::floatingconditional,
                                                    new commandbind_t{ commandop_t::clientmove, direction_t::up },
                                                    new commandbind_t{ commandop_t::stackforward }
                                              } } },
            {  "nextcontext",                 { { commandop_t::nextcontext }                                               } },
            {  "nextworkspace",               { { commandop_t::nextworkspace }                                             } },
            {  "nextworkspaceactiveclient",   { { commandop_t::clientnextworkspace }                                       } },
            {  "panejump",                    { { commandop_t::clientpanejump }                                            } },
            {  "prevcontext",                 { { commandop_t::previouscontext }                                           } },
            {  "prevworkspace",               { { commandop_t::previousworkspace }                                         } },
            {  "prevworkspaceactiveclient",   { { commandop_t::clientpreviousworkspace }                                   } },
            {  "quit",                        { { commandop_t::quit }                                                      } },
            {  "reclaimpop",                  { { commandop_t::reclaimpop }                                                } },
            {  "snapmoveeastactiveclient",    { { commandop_t::clientsnapmoveeast }                                        } },
            {  "snapmovenorthactiveclient",   { { commandop_t::clientsnapmovenorth }                                       } },
            {  "snapmovesouthactiveclient",   { { commandop_t::clientsnapmovesouth }                                       } },
            {  "snapmovewestactiveclient",    { { commandop_t::clientsnapmovewest }                                        } },
            {  "snapresizeeastactiveclient",  { { commandop_t::clientsnapresizeeast }                                      } },
            {  "snapresizenorthactiveclient", { { commandop_t::clientsnapresizenorth }                                     } },
            {  "snapresizesouthactiveclient", { { commandop_t::clientsnapresizesouth }                                     } },
            {  "snapresizewestactiveclient",  { { commandop_t::clientsnapresizewest }                                      } },
            {  "stackbackward",               { { commandop_t::stackbackward }                                             } },
            {  "stackforward",                { { commandop_t::stackforward }                                              } },
            {  "stackjump",                   { { commandop_t::clientstackjump }                                           } },
            {  "stickyactiveclient",          { { commandop_t::clientsticky }                                              } },
            {  "toglayout",                   { { commandop_t::workspacelayout, layout_t::toggle }                         } },
            {  "togsidebar",                  { { commandop_t::sidebarshow }                                               } },
            {  "togsidebarall",               { { commandop_t::sidebarshowall }                                            } },
            {  "togworkspace",                { { commandop_t::workspaceset, 0 }                                           } },
            {  "workspacemirror",             { { commandop_t::workspacemirror }                                           } },
            {  "zoom",                        { { commandop_t::zoom }                                                      } },
            // command                            external command
            {  "browser",                     { { "firefox" }                                                              } },
            {  "calculator",                  { { "qalculate-gtk" }                                                        } },
            {  "decbrightness",               { { "xbacklight -dec 10" }                                                   } },
            {  "decbrightness5",              { { "xbacklight -dec 5" }                                                    } },
            {  "decvolaudio",                 { { "amixer -D pulse sset Master 5%-" }                                      } },
            {  "dmenu",                       { { "~/bin/dmenu_runner" }                                                   } },
            {  "dmenunotify",                 { { "~/bin/dmenunotify" }                                                    } },
            {  "dmenupass",                   { { "~/bin/dmenupass" }                                                      } },
            {  "dmenupasscopy",               { { "~/bin/dmenupass --copy" }                                               } },
            {  "incbrightness",               { { "xbacklight -inc 10" }                                                   } },
            {  "incbrightness5",              { { "xbacklight -inc 5" }                                                    } },
            {  "incvolaudio",                 { { "amixer -D pulse sset Master 5%+" }                                      } },
            {  "irssi",                       { { "st -g 80x42 -e zsh -i -c irssi" }                                       } },
            {  "neomutt",                     { { "st -g 140x42 -e zsh -i -c neomutt" }                                    } },
            {  "nextaudio",                   { { "playerctl next" }                                                       } },
            {  "prevaudio",                   { { "playerctl previous" }                                                   } },
            {  "rtv",                         { { "st -g 80x42 -e zsh -i -c rtv" }                                         } },
            {  "screenshot",                  { { "maim -m 1 $(date +~/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)" }     } },
            {  "secbrowser",                  { { "qutebrowser" }                                                          } },
            {  "seekbackwardaudio",           { { "playerctl position -5" }                                                } },
            {  "seekforwardaudio",            { { "playerctl position +5" }                                                } },
            {  "selscreenshot",               { { "maim -m 1 -s $(date +~/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)" }  } },
            {  "skippy-xd",                   { { "skippy-xd --toggle-window-picker" }                                     } },
            {  "sncli",                       { { "st -g 80x42 -e zsh -i -c sncli" }                                       } },
            {  "spawnterm",                   { { "st -n \"kranewm:cf\"" }                                                 } },
            {  "spawntiledterm",              { { "st" }                                                                   } },
            {  "stopaudio",                   { { "playerctl stop" }                                                       } },
            {  "stregion",                    { { "~/bin/stregion" }                                                       } },
            {  "togmuteaudio",                { { "amixer -D pulse set Master 1+ toggle" }                                 } },
            {  "togplayaudio",                { { "playerctl play-pause" }                                                 } },
            {  "togskippy",                   { { "skippy-xd --toggle-window-picker" }                                     } },
        })
    {
        x_data::replace_property<x_data::string_t>(x_data::g_root, {"_IPC_PREFIX", IPC_PREFIX});
        x_data::replace_property<x_data::cardinal_t>(x_data::g_root, {IPC_PREFIX + "ENABLED", m_enabled});

        if (!m_enabled)
            return;

        if(const char* env_sock_path = ::std::getenv(SOCK_PATH_ENV.c_str()))
            m_sock_path = ::std::string(env_sock_path);
        else {
            char hostname[HOST_NAME_MAX];
            hostname[0] = '\0';
            gethostname(hostname, HOST_NAME_MAX);

            m_sock_path = ::std::string(DEFAULT_SOCK_PATH_PREFIX);
            if (hostname[0] != '\0')
                m_sock_path += "-" + ::std::string(hostname);
        }

        m_sock_address.sun_family = AF_UNIX;
        memcpy(m_sock_address.sun_path, m_sock_path.c_str(), m_sock_path.size());

        if ((m_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            fail_ipc("couldn't open socket");
            return;
        }

        unlink(m_sock_path.c_str());

        if (bind(m_sock_fd, reinterpret_cast<struct sockaddr*>(&m_sock_address), sizeof(m_sock_address)) == -1) {
            fail_ipc("couldn't bind name to socket");
            return;
        }

        if (listen(m_sock_fd, SOMAXCONN) == -1) {
            fail_ipc("couldn't listen to socket");
            return;
        }

        x_data::replace_property<x_data::string_t>(x_data::g_root, {IPC_PREFIX + "SOCKET_PATH", m_sock_path});
        x_data::replace_property<x_data::cardinal_t>(x_data::g_root, {IPC_PREFIX + "SOCKET_FD", (CARD32)m_sock_fd});
    }

    ~ipc_t()
    {
        unlink(m_sock_path.c_str());
        close(m_sock_fd);
    }

    int
    get_sock_fd() const
    {
        if (m_enabled)
            return m_sock_fd;

        return -1;
    }

    commandbind_t
    fail_command(::std::string&& msg, int cli_fd)
    {
        warn("error resolving command: " + msg);
        write(cli_fd, msg.c_str(), msg.size());
        return commandop_t::noop;
    }

    void
    fail_ipc(::std::string&& msg)
    {
        warn("error setting up IPC server: " + msg);
        x_data::replace_property<x_data::cardinal_t>(x_data::g_root, {IPC_PREFIX + "ENABLED", false});
        m_enabled = false;
    }

    bool
    is_enabled() const
    {
        return m_enabled;
    }

    void handle_ipc();
    void process_message(::std::vector<::std::string>&, int);

    commandbind_t resolve_command(::std::vector<::std::string>&, int);

    int resolve_int(::std::string&);
    float resolve_float(::std::string&);
    int resolve_char_index(::std::string&);
    int resolve_int_index(::std::string&);
    layout_t resolve_layout(::std::string&);
    direction_t resolve_direction(::std::string&);

private:
    bool m_enabled;
    inputhandler_t& m_input;

    int m_sock_fd;
    struct sockaddr_un m_sock_address;
    ::std::string m_sock_path;

    ipcbinds_t m_ipcbinds;

};


#endif//__KRANEWM__IPC__GUARD__
