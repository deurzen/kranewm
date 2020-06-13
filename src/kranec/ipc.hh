#ifndef __KRANEWM__KRANEC__IPC__GUARD__
#define __KRANEWM__KRANEC__IPC__GUARD__

#include "../common.hh"
#include "../util.hh"

#include "../x-data/property.hh"
#include "../x-data/cardinal.hh"

#include <string>

#include <limits.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static const ::std::string NORETURN = "\x18";

const ::std::size_t MAX_EVENTS = 50;

const ::std::string IPC_PREFIX = "_" + uppercase(WMNAME) + "_IPC_";
const ::std::string SOCK_PATH_ENV = uppercase(WMNAME) + "_SOCKETPATH";
#ifndef DEBUG
const ::std::string DEFAULT_SOCK_PATH_PREFIX = "/tmp/" + WMNAME;
#else
const ::std::string DEFAULT_SOCK_PATH_PREFIX = "/tmp/D" + WMNAME;
#endif


class ipc_t final
{
public:
    ipc_t(int argc, char** argv)
      : m_argc(argc),
        m_argv(argv),
        m_sock_fd(-1),
        m_sock_address(),
        m_sock_path()
    {
        x_data::cardinal_t is_enabled = x_data::get_property<x_data::cardinal_t>(x_data::g_root, IPC_PREFIX + "ENABLED");
        if (!is_enabled)
            fail_ipc(WMNAME + " compiled without IPC support");

        if ((m_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
            fail_ipc("couldn't open socket");

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
    }

    ~ipc_t()
    {
        close(m_sock_fd);
    }

    void
    process_message()
    {
        if (connect(m_sock_fd, reinterpret_cast<struct sockaddr*>(&m_sock_address), sizeof(m_sock_address)) == -1)
            fail_ipc("couldn't connect to socket");

        ::std::string cli_msg;
        for (; m_argc > 0; --m_argc, ++m_argv)
            cli_msg += ::std::string(*m_argv) + '\0';

        if (write(m_sock_fd, cli_msg.c_str(), cli_msg.size()) == -1)
            fail_ipc("couldn't send message");

        char ser_msg[BUFSIZ];
        ssize_t n = read(m_sock_fd, ser_msg, sizeof(ser_msg) - 1);
        ser_msg[n] = '\0';

        if (NORETURN.compare(ser_msg))
            ::std::cout << ser_msg << ::std::endl << ::std::flush;
    }

    void
    fail_ipc(::std::string&& msg)
    {
        warn("error contacting server: " + msg);
        exit(1);
    }


private:
    int m_argc;
    char** m_argv;

    int m_sock_fd;
    struct sockaddr_un m_sock_address;
    ::std::string m_sock_path;

};

#endif//__KRANEWM__KRANEC__IPC__GUARD__
