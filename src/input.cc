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
        process_command(m_mousebinds[{event.button, event.state, mousetarget_t::client}].first, client);
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
    if (m_processbinds.count(event)) {
        m_clients.jump_process(m_processbinds.at(event));
        return;
    }

    process_command(m_keybinds[event], client);
}

void
inputhandler_t::process_ipc_input(commandbind_t commandbind)
{
    process_command(commandbind, m_clients.focused_client());
}


void
inputhandler_t::process_command(commandbind_t commandbind, client_ptr_t target)
{
    static ::std::unordered_map<commandbind_t, command_ptr_t> interned_commands{};
    command_ptr_t command;

    if (interned_commands.count(commandbind))
        command = interned_commands.at(commandbind);
    else {
        command = create_command(commandbind, target);
        if (command->is_internable()) {
            interned_commands[commandbind] = command;
        }
    }

    command->execute();

    if (!command->is_internable())
        delete command;
}

command_ptr_t
inputhandler_t::create_command(commandbind_t commandbind, client_ptr_t target)
{
    switch (commandbind.get_op()) { // global commands
    case commandop_t::noop:                       break;
    case commandop_t::floatingconditional:        return new floatingconditionalcommand_t(m_clients,
                                                       create_command(*commandbind.get_comp1(), target),
                                                       create_command(*commandbind.get_comp2(), target),
                                                       target);
    case commandop_t::quit:                       return new quitcommand_t(m_running);
    case commandop_t::zoom:                       return new zoomcommand_t(m_clients);
    case commandop_t::clientmoveforward:          return new clientmoveforwardcommand_t(m_clients);
    case commandop_t::clientmovebackward:         return new clientmovebackwardcommand_t(m_clients);
    case commandop_t::clientmarkjump:             return new clientmarkjumpcommand_t(m_clients);
    case commandop_t::clientmasterjump:           return new clientmasterjumpcommand_t(m_clients);
    case commandop_t::clientstackjump:            return new clientstackjumpcommand_t(m_clients);
    case commandop_t::clientlastjump:             return new clientlastjumpcommand_t(m_clients);
    case commandop_t::clientpanejump:             return new clientpanejumpcommand_t(m_clients);
    case commandop_t::clientjumpindex:            return new clientjumpindexcommand_t(m_clients, ::std::get<int>(*commandbind.get_arg()));
    case commandop_t::clientgrow:                 return new clientgrowcommand_t(m_clients, target, ::std::get<direction_t>(*commandbind.get_arg()));
    case commandop_t::clientshrink:               return new clientshrinkcommand_t(m_clients, target, ::std::get<direction_t>(*commandbind.get_arg()));
    case commandop_t::clientmove:                 return new clientmovecommand_t(m_clients, target, ::std::get<direction_t>(*commandbind.get_arg()));
    case commandop_t::clientmovemouse:            return new clientmovemousecommand_t(m_clients, target);
    case commandop_t::masterforward:              return new masterforwardcommand_t(m_clients, m_windowstack);
    case commandop_t::masterbackward:             return new masterbackwardcommand_t(m_clients, m_windowstack);
    case commandop_t::stackforward:               return new stackforwardcommand_t(m_clients, m_windowstack);
    case commandop_t::stackbackward:              return new stackbackwardcommand_t(m_clients, m_windowstack);
    case commandop_t::allforward:                 return new allforwardcommand_t(m_clients, m_windowstack);
    case commandop_t::allbackward:                return new allbackwardcommand_t(m_clients, m_windowstack);
    case commandop_t::clienticonifyindex:         return new clienticonifyindexcommand_t(m_clients, ::std::get<int>(*commandbind.get_arg()));
    case commandop_t::deiconifypop:               return new deiconifypopcommand_t(m_clients);
    case commandop_t::clientdeiconifyindex:       return new clientdeiconifyindexcommand_t(m_clients, ::std::get<int>(*commandbind.get_arg()));
    case commandop_t::reclaimpop:                 return new reclaimpopcommand_t(m_clients);
    case commandop_t::profilesave:                return new profilesavecommand_t(m_clients, ::std::get<int>(*commandbind.get_arg()));
    case commandop_t::profileload:                return new profileloadcommand_t(m_clients, m_sidebar, ::std::get<int>(*commandbind.get_arg()));
    case commandop_t::workspaceset:               return new workspacesetcommand_t(m_clients, m_sidebar, ::std::get<int>(*commandbind.get_arg()));
    case commandop_t::nextworkspace:              return new nextworkspacecommand_t(m_clients, m_sidebar);
    case commandop_t::previousworkspace:          return new previousworkspacecommand_t(m_clients, m_sidebar);
    case commandop_t::workspacemirror:            return new workspacemirrorcommand_t(m_clients);
    case commandop_t::workspacemfactor:           return new workspacemfactorcommand_t(m_clients, ::std::get<float>(*commandbind.get_arg()));
    case commandop_t::workspacenmaster:           return new workspacenmastercommand_t(m_clients, ::std::get<int>(*commandbind.get_arg()));
    case commandop_t::workspacegapsize:           return new workspacegapsizecommand_t(m_clients, ::std::get<int>(*commandbind.get_arg()));
    case commandop_t::workspacelayout:            return new workspacelayoutcommand_t(m_clients,
                                                       m_windowstack, m_sidebar, ::std::get<layout_t>(*commandbind.get_arg()));
    case commandop_t::workspacelayoutpreservedim: return new workspacelayoutpreservedimcommand_t(m_clients,
                                                       m_windowstack, m_sidebar, ::std::get<layout_t>(*commandbind.get_arg()));
    case commandop_t::sidebarshow:                return new sidebarshowcommand_t(m_clients, m_sidebar);
    case commandop_t::sidebarshowall:             return new sidebarshowallcommand_t(m_clients, m_sidebar);
    case commandop_t::contextset:                 return new contextsetcommand_t(m_clients, ::std::get<int>(*commandbind.get_arg()));
    case commandop_t::nextcontext:                return new nextcontextcommand_t(m_clients);
    case commandop_t::previouscontext:            return new previouscontextcommand_t(m_clients);
    case commandop_t::focusforward:               return new focusforwardcommand_t(m_clients);
    case commandop_t::focusbackward:              return new focusbackwardcommand_t(m_clients);
    case commandop_t::workspacekill:              return new workspacekillcommand_t(m_clients.active_workspace());
    case commandop_t::contextkill:                return new contextkillcommand_t(m_clients.active_context());
    case commandop_t::external:                   return new externalcommand_t(::std::get<::std::string>(*commandbind.get_arg()));
    default: break;
    }

    if (target)
        switch (commandbind.get_op()) { // target client commands
        case commandop_t::clientfloat:             return new clientfloatcommand_t(m_clients, m_windowstack, m_sidebar, target);
        case commandop_t::clientfullscreen:        return new clientfullscreencommand_t(m_clients, m_sidebar, target);
        case commandop_t::clientinwindow:          return new clientinwindowcommand_t(m_clients, m_sidebar, target);
        case commandop_t::clientsticky:            return new clientstickycommand_t(m_clients, m_sidebar, target);
        case commandop_t::clientabove:             return new clientabovecommand_t(m_clients, m_sidebar, target);
        case commandop_t::clientbelow:             return new clientbelowcommand_t(m_clients, m_sidebar, target);
        case commandop_t::clientcenter:            return new clientcentercommand_t(m_clients, target);
        case commandop_t::clientsnapmovenorth:     return new clientsnapmovenorthcommand_t(m_clients, target);
        case commandop_t::clientsnapmoveeast:      return new clientsnapmoveeastcommand_t(m_clients, target);
        case commandop_t::clientsnapmovesouth:     return new clientsnapmovesouthcommand_t(m_clients, target);
        case commandop_t::clientsnapmovewest:      return new clientsnapmovewestcommand_t(m_clients, target);
        case commandop_t::clientsnapresizenorth:   return new clientsnapresizenorthcommand_t(m_clients, target);
        case commandop_t::clientsnapresizeeast:    return new clientsnapresizeeastcommand_t(m_clients, target);
        case commandop_t::clientsnapresizesouth:   return new clientsnapresizesouthcommand_t(m_clients, target);
        case commandop_t::clientsnapresizewest:    return new clientsnapresizewestcommand_t(m_clients, target);
        case commandop_t::clientkill:              return new clientkillcommand_t(target);
        case commandop_t::clientmarkset:           return new clientmarksetcommand_t(m_clients, target);
        case commandop_t::clientworkspace:         return new clientworkspacecommand_t(m_clients, target, ::std::get<int>(*commandbind.get_arg()));
        case commandop_t::clientnextworkspace:     return new clientnextworkspacecommand_t(m_clients, target);
        case commandop_t::clientpreviousworkspace: return new clientpreviousworkspacecommand_t(m_clients, target);
        case commandop_t::clientcontext:           return new clientcontextcommand_t(m_clients, target, ::std::get<int>(*commandbind.get_arg()));
        case commandop_t::clientnextcontext:       return new clientnextcontextcommand_t(m_clients, target);
        case commandop_t::clientpreviouscontext:   return new clientpreviouscontextcommand_t(m_clients, target);
        case commandop_t::clientresizemouse:       return new clientresizemousecommand_t(m_clients, target);
        case commandop_t::clienticonify:           return new clienticonifycommand_t(m_clients, target);
        case commandop_t::clientdisown:            return new clientdisowncommand_t(m_clients, target);
        default: break;
        }

    return new noopcommand_t();
}
