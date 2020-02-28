#include "input.hh"

#include "sidebar.hh"
#include "client-model.hh"
#include "ipc.hh"

#include "x-data/window.hh"

#include <unistd.h>


bool
inputhandler_t::moves_focus(XButtonEvent event) const
{
    if (m_mousebinds.count({event.button, event.state, mousetarget_t::client}))
        return !m_mousebinds.at({event.button, event.state, mousetarget_t::client}).second;

    return true;
}

void
inputhandler_t::process_mouse_input_global(XButtonEvent event)
{
    if (m_mousebinds.count({event.button, event.state, mousetarget_t::root}))
        process_command(m_mousebinds[{event.button, event.state, mousetarget_t::root}].first);
}

void
inputhandler_t::process_mouse_input_sidebar(XButtonEvent event)
{
    if (m_mousebinds.count({event.button, event.state, mousetarget_t::sidebar}))
        process_command(m_mousebinds[{event.button, event.state, mousetarget_t::sidebar}].first);
}

void
inputhandler_t::process_mouse_input_client(client_ptr_t client, XButtonEvent event)
{
    if (m_mousebinds.count({event.button, event.state, mousetarget_t::client})) {
        m_target = client;
        process_command(m_mousebinds[{event.button, event.state, mousetarget_t::client}].first);
        m_target = nullptr;
    }
}

void
inputhandler_t::process_key_input_global(XKeyEvent event)
{
    if (m_processbinds.count(event)) {
        m_clients.jump_process(m_processbinds[event]);
        return;
    }

    process_command(m_keybinds[event]);
}

void
inputhandler_t::process_key_input_client(client_ptr_t client, XKeyEvent event)
{
    m_target = client;
    process_command(m_keybinds[event]);
    m_target = nullptr;
}

void
inputhandler_t::process_ipc_global(ipccommand_t command)
{
    auto [data,op] = command;

    process_command(op);
}

void
inputhandler_t::process_command(commandbind_t commandbind)
{
    command_ptr_t command;

    switch (commandbind.get_op()) {
    case commandop_t::noop:                    command = new noopcommand_t();          break;
    case commandop_t::floatingconditional:     break;
    case commandop_t::quit:                    command = new quitcommand_t(m_running); break;
    case commandop_t::zoom:                    command = new zoomcommand_t(m_clients); break;
    case commandop_t::clientfloat:             command = new clientfloatcommand_t(m_clients, m_windowstack, m_sidebar, m_target); break;
    case commandop_t::clientfullscreen:         break;
    case commandop_t::clientsticky:             break;
    case commandop_t::clientabove:              break;
    case commandop_t::clientbelow:             break;
    case commandop_t::clientcenter:             break;
    case commandop_t::clientsnapnorth:          break;
    case commandop_t::clientsnapeast:           break;
    case commandop_t::clientsnapsouth:          break;
    case commandop_t::clientsnapwest:          break;
    case commandop_t::clientkill:              break;
    case commandop_t::clientmoveforward:        break;
    case commandop_t::clientmovebackward:      break;
    case commandop_t::clientmarkset:            break;
    case commandop_t::clientmarkjump:          break;
    case commandop_t::clientmasterjump:         break;
    case commandop_t::clientstackjump:          break;
    case commandop_t::clientlastjump:           break;
    case commandop_t::clientpanejump:           break;
    case commandop_t::clientjumpindex:         break;
    case commandop_t::clientworkspace:          break;
    case commandop_t::clientnextworkspace:      break;
    case commandop_t::clientpreviousworkspace: break;
    case commandop_t::clientcontext:            break;
    case commandop_t::clientnextcontext:        break;
    case commandop_t::clientpreviouscontext:   break;
    case commandop_t::clientgrow:               break;
    case commandop_t::clientshrink:             break;
    case commandop_t::clientmove:               break;
    case commandop_t::clientmovemouse:          break;
    case commandop_t::clientresizemouse:       break;
    case commandop_t::masterforward:            break;
    case commandop_t::masterbackward:           break;
    case commandop_t::stackforward:             break;
    case commandop_t::stackbackward:            break;
    case commandop_t::allforward:               break;
    case commandop_t::allbackward:             break;
    case commandop_t::clienticonify:            break;
    case commandop_t::clienticonifyindex:       break;
    case commandop_t::deiconifypop:             break;
    case commandop_t::clientdeiconifyindex:    break;
    case commandop_t::clientdisown:             break;
    case commandop_t::reclaimpop:              break;
    case commandop_t::profilesave:              break;
    case commandop_t::profileload:             break;
    case commandop_t::workspaceset:             break;
    case commandop_t::nextworkspace:            break;
    case commandop_t::previousworkspace:       break;
    case commandop_t::workspacemirror:          break;
    case commandop_t::workspacemfactor:         break;
    case commandop_t::workspacenmaster:         break;
    case commandop_t::workspacegapsize:         break;
    case commandop_t::workspacelayout:         break;
    case commandop_t::sidebarshow:             break;
    case commandop_t::contextset:              command = new contextsetcommand_t(m_clients, ::std::get<int>(*commandbind.get_arg())); break;
    case commandop_t::nextcontext:             command = new nextcontextcommand_t(m_clients); break;
    case commandop_t::previouscontext:         command = new previouscontextcommand_t(m_clients); break;
    case commandop_t::focusforward:            command = new focusforwardcommand_t(m_clients); break;
    case commandop_t::focusbackward:           command = new focusbackwardcommand_t(m_clients); break;
    case commandop_t::external:                command = new externalcommand_t(::std::get<::std::string>(*commandbind.get_arg())); break;
    default: break;
    }

    command->execute();
    delete command;
}
