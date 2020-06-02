#ifndef __KRANEWM__IPC__GUARD__
#define __KRANEWM__IPC__GUARD__

#include "commands.hh"

#include "x-data/property.hh"
#include "x-data/display.hh"

#include <limits.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


const ::std::string IPC_PREFIX = "_KRANEWM_IPC_";
const ::std::string SOCK_PATH_ENV = "KRANEWM_SOCKETPATH";
const ::std::string DEFAULT_SOCK_PATH_PREFIX = "/tmp/kranewm";


class ipc_t
{
public:
    ipc_t()
      : m_enabled(true),
        m_sock_fd(-1),
        m_sock_address({}),
        m_sock_path({})
    {
        x_data::replace_property<x_data::string_t>(x_data::g_root, {"_IPC_PREFIX", IPC_PREFIX});
        x_data::replace_property<x_data::cardinal_t>(x_data::g_root, {IPC_PREFIX + "ENABLED", m_enabled});

        if (!m_enabled)
            return;

        if(const char* env_sock_path = ::std::getenv(SOCK_PATH_ENV.c_str()))
            m_sock_path = ::std::string(env_sock_path);
        else {
            char hostname[HOST_NAME_MAX];
            gethostname(hostname, HOST_NAME_MAX);

            m_sock_path = ::std::string(DEFAULT_SOCK_PATH_PREFIX);
            m_sock_path += "-" + ::std::string(hostname);
        }

        m_sock_address.sun_family = AF_UNIX;
        memcpy(m_sock_address.sun_path, m_sock_path.c_str(), m_sock_path.size());

        if ((m_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            fail_ipc("couldn't open socket");
            return;
        }

        unlink(m_sock_path.c_str());

        if (bind(m_sock_fd, (struct sockaddr*)&m_sock_address, sizeof(m_sock_address)) == -1) {
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

    void
    fail_ipc(::std::string&& msg)
    {
        ::std::cerr << "error setting up IPC server: " << msg << ::std::endl;
        m_enabled = false;
        x_data::replace_property<x_data::cardinal_t>(x_data::g_root, {IPC_PREFIX + "ENABLED", false});
    }

    bool
    is_enabled() const
    {
        return m_enabled;
    }

    ~ipc_t()
    {
        //TODO unbind socket
    }

private:
    bool m_enabled;
    int m_sock_fd;
    struct sockaddr_un m_sock_address;
    ::std::string m_sock_path;

};


#endif//__KRANEWM__IPC__GUARD__
