#include "client_model.hh"


client_ptr_t
client_model::win_to_client(x_wrapper::window_t win)
{
    for (auto&& [_win,_client] : m_client_windows)
        if (_win.get() == win.get())
            return _client;

    return nullptr;
}

void
client_model::focus(client_ptr_t client)
{

}
