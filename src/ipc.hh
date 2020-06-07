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
const ::std::string DEFAULT_SOCK_PATH_PREFIX = "/tmp/" + WMNAME;

enum class argtype_t
{
    d, i = d, // int
    f,        // float
    lt,       // layout
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
        m_sock_address({}),
        m_sock_path({}),
        m_ipcbinds({
            // command                            operation                             arg
            {  "workspace",                   {   commandop_t::workspaceset,          { argtype_t::i   }                   } },
            {  "layout",                      {   commandop_t::workspacelayout,       { argtype_t::lt  }                   } },
            {  "profilesave",                 {   commandop_t::profilesave,           { argtype_t::i   }                   } },
            {  "profileload",                 {   commandop_t::profileload,           { argtype_t::i   }                   } },
            {  "deiconify",                   {   commandop_t::clientdeiconifyindex,  { argtype_t::i   }                   } },
            {  "workspaceactiveclient",       {   commandop_t::clientworkspace,       { argtype_t::i   }                   } },
            {  "context",                     {   commandop_t::contextset,            { argtype_t::i   }                   } },
            {  "contextactiveclient",         {   commandop_t::clientcontext,         { argtype_t::i   }                   } },
            {  "moveactiveclient",            {   commandop_t::clientmove,            { argtype_t::dir }                   } },
            {  "growactiveclient",            {   commandop_t::clientgrow,            { argtype_t::dir }                   } },
            {  "shrinkactiveclient",          {   commandop_t::clientshrink,          { argtype_t::dir }                   } },
            // command                            bind
            {  "prevworkspace",               { { commandop_t::previousworkspace }                                         } },
            {  "nextworkspace",               { { commandop_t::nextworkspace }                                             } },
            {  "killactiveclient",            { { commandop_t::clientkill }                                                } },
            {  "focusbackward",               { { commandop_t::focusbackward }                                             } },
            {  "focusforward",                { { commandop_t::focusforward }                                              } },
            {  "zoom",                        { { commandop_t::zoom }                                                      } },
            {  "togworkspace",                { { commandop_t::workspaceset, 0 }                                           } },
            {  "masterjump",                  { { commandop_t::clientmasterjump }                                          } },
            {  "decnmaster",                  { { commandop_t::workspacenmaster, -1 }                                      } },
            {  "decmfactor",                  { { commandop_t::workspacemfactor, -.05f }                                   } },
            {  "incnmaster",                  { { commandop_t::workspacenmaster,  1 }                                      } },
            {  "incmfactor",                  { { commandop_t::workspacemfactor,  .05f }                                   } },
            {  "stregion",                    { { "~/bin/stregion" }                                                       } },
            {  "stickyactiveclient",          { { commandop_t::clientsticky }                                              } },
            {  "iconifyactiveclient",         { { commandop_t::clienticonify }                                             } },
            {  "deiconifypop",                { { commandop_t::deiconifypop }                                              } },
            {  "togsidebar",                  { { commandop_t::sidebarshow }                                               } },
            {  "stackjump",                   { { commandop_t::clientstackjump }                                           } },
            {  "lastjump",                    { { commandop_t::clientlastjump }                                            } },
            {  "panejump",                    { { commandop_t::clientpanejump }                                            } },
            {  "workspacemirror",             { { commandop_t::workspacemirror }                                           } },
            {  "dmenu",                       { { "~/bin/dmenu_runner" }                                                   } },
            {  "secbrowser",                  {  { "qutebrowser" }                                                         } },
            {  "toglayout",                   { { commandop_t::workspacelayout, layout_t::toggle }                         } },
            {  "fullscreenactiveclient",      { { commandop_t::clientfullscreen }                                          } },
            {  "skippy-xd",                   { { "skippy-xd --toggle-window-picker" }                                     } },
            {  "calculator",                  { { "qalculate-gtk" }                                                        } },
            {  "decgapsize",                  { { commandop_t::workspacegapsize, -1 }                                      } },
            {  "incgapsize",                  { { commandop_t::workspacegapsize,  1 }                                      } },
            {  "disownactiveclient",          { { commandop_t::clientdisown }                                              } },
            {  "reclaimpop",                  { { commandop_t::reclaimpop }                                                } },
            {  "belowactiveclient",           { { commandop_t::clientbelow }                                               } },
            {  "aboveactiveclient",           { { commandop_t::clientabove }                                               } },
            {  "gapsizereset",                { { commandop_t::workspacegapsize, 0 }                                       } },
            {  "markactiveclient",            { { commandop_t::clientmarkset }                                             } },
            {  "prevworkspaceactiveclient",   { { commandop_t::clientpreviousworkspace }                                   } },
            {  "nextworkspaceactiveclient",   { { commandop_t::clientnextworkspace }                                       } },
            {  "inwindowactiveclient",        { { commandop_t::clientinwindow }                                            } },
            {  "movebackwardactiveclient",    { { commandop_t::clientmovebackward }                                        } },
            {  "moveforwardactiveclient",     { { commandop_t::clientmoveforward }                                         } },
            {  "togsidebarall",               { { commandop_t::sidebarshowall }                                            } },
            {  "floatactiveclient",           { { commandop_t::clientfloat }                                               } },
            {  "dmenupass",                   { { "~/bin/dmenupass" }                                                      } },
            {  "browser",                     { { "firefox" }                                                              } },
            {  "jumpmark",                    { { commandop_t::clientmarkjump }                                            } },
            {  "centeractiveclient",          { { commandop_t::clientcenter }                                              } },
            {  "snapmovenorthactiveclient",   { { commandop_t::clientsnapmovenorth }                                       } },
            {  "snapmoveeastactiveclient",    { { commandop_t::clientsnapmoveeast }                                        } },
            {  "snapmovesouthactiveclient",   { { commandop_t::clientsnapmovesouth }                                       } },
            {  "snapmovewestactiveclient",    { { commandop_t::clientsnapmovewest }                                        } },
            {  "masterforward",               { { commandop_t::masterforward }                                             } },
            {  "masterbackward",              { { commandop_t::masterbackward }                                            } },
            {  "stackforward",                { { commandop_t::stackforward }                                              } },
            {  "stackbackward",               { { commandop_t::stackbackward }                                             } },
            {  "moveleftmasterforward",       { { commandop_t::floatingconditional,
                                                              new commandbind_t{ commandop_t::clientmove, direction_t::left },
                                                              new commandbind_t{ commandop_t::masterforward }
                                              } } },
            {  "movedownstackbackward",       { { commandop_t::floatingconditional,
                                                              new commandbind_t{ commandop_t::clientmove, direction_t::down },
                                                              new commandbind_t{ commandop_t::stackbackward }
                                              } } },
            {  "moveupstackforward",          { { commandop_t::floatingconditional,
                                                              new commandbind_t{ commandop_t::clientmove, direction_t::up },
                                                              new commandbind_t{ commandop_t::stackforward }
                                              } } },
            {  "moverightmasterbackward",     { { commandop_t::floatingconditional,
                                                              new commandbind_t{ commandop_t::clientmove, direction_t::right },
                                                              new commandbind_t{ commandop_t::masterbackward }
                                              } } },
            {  "neomutt",                     { { "st -g 140x42 -e zsh -i -c neomutt" }                                    } },
            {  "sncli",                       { { "st -g 80x42 -e zsh -i -c sncli" }                                       } },
            {  "rtv",                         { { "st -g 80x42 -e zsh -i -c rtv" }                                         } },
            {  "irssi",                       { { "st -g 80x42 -e zsh -i -c irssi" }                                       } },
            {  "dmenupasscopy",               { { "~/bin/dmenupass --copy" }                                               } },
            {  "dmenunotify",                 { { "~/bin/dmenunotify" }                                                    } },
            {  "allbackward",                 { { commandop_t::allbackward }                                               } },
            {  "allforward",                  { { commandop_t::allforward }                                                } },
            {  "prevcontext",                 { { commandop_t::previouscontext }                                           } },
            {  "nextcontext",                 { { commandop_t::nextcontext }                                               } },
            {  "floatingpreservedim",         { { commandop_t::workspacelayoutpreservedim, layout_t::floating }            } },
            {  "snapresizenorthactiveclient", { { commandop_t::clientsnapresizenorth }                                     } },
            {  "snapresizeeastactiveclient",  { { commandop_t::clientsnapresizeeast }                                      } },
            {  "snapresizesouthactiveclient", { { commandop_t::clientsnapresizesouth }                                     } },
            {  "snapresizewestactiveclient",  { { commandop_t::clientsnapresizewest }                                      } },
            {  "quit",                        { { commandop_t::quit }                                                      } },
            {  "incvolaudio",                 { { "amixer -D pulse sset Master 5%+" }                                      } },
            {  "decvolaudio",                 { { "amixer -D pulse sset Master 5%-" }                                      } },
            {  "togmuteaudio",                { { "amixer -D pulse set Master 1+ toggle" }                                 } },
            {  "togplayaudio",                { { "playerctl play-pause" }                                                 } },
            {  "stopaudio",                   { { "playerctl stop" }                                                       } },
            {  "prevaudio",                   { { "playerctl previous" }                                                   } },
            {  "nextaudio",                   { { "playerctl next" }                                                       } },
            {  "seekbackwardaudio",           { { "playerctl position -5" }                                                } },
            {  "seekforwardaudio",            { { "playerctl position +5" }                                                } },
            {  "incbrightness",               { { "xbacklight -inc 10" }                                                   } },
            {  "decbrightness",               { { "xbacklight -dec 10" }                                                   } },
            {  "selscreenshot",               { { "maim -m 1 -s $(date +~/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)" }  } },
            {  "incbrightness5",              { { "xbacklight -inc 5" }                                                    } },
            {  "decbrightness5",              { { "xbacklight -dec 5" }                                                    } },
            {  "screenshot",                  { { "maim -m 1 $(date +~/screenshots/scrots/SS_%Y-%h-%d_%H-%M-%S.png)" }     } },
            {  "spawntiledterm",              { { "st" }                                                                   } },
            {  "spawnterm",                   { { "st -n \"kranewm:cf\"" }                                                 } },
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
    fail_command(::std::string&& msg)
    {
        warn("error resolving command: " + msg);
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
    void process_message(::std::vector<::std::string>&, FILE*);

    commandbind_t resolve_command(::std::vector<::std::string>&);

    int resolve_int(::std::string&);
    float resolve_float(::std::string&);
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
