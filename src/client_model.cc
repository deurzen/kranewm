#include "client_model.hh"


client_ptr_t
client_model::win_to_client(x_wrapper::window_t win)
{
    if (m_client_windows.count(win))
        return m_client_windows[win];
    return nullptr;
}

void
client_model::focus(client_ptr_t client)
{

}
