#include "commands.hh"

#include "client-model.hh"
#include "sidebar.hh"

#include "x-data/window.hh"

#include <unistd.h>


void
floatingconditionalcommand_t::execute()
{
    if (m_client->floating || m_clients.active_workspace()->in_float_layout()) {
        m_truecommand->execute();
    } else {
        m_falsecommand->execute();
    }
}

void
quitcommand_t::execute()
{
    m_running = false;
}

void
zoomcommand_t::execute()
{
    m_clients.active_workspace()->zoom();
    m_clients.sync_workspace_focus();
}

void
clientfloatcommand_t::execute()
{
    if (m_client->parent || (!m_client->in_window && m_client->fullscreen) || m_client->disowned)
        return;

    if (m_client->floating) {
        if (m_client->above)
            m_clients.set_above(m_client, clientaction_t::remove);
        else if (m_client->below)
            m_clients.set_below(m_client, clientaction_t::remove);
    }

    m_client->set_float(clientaction_t::toggle);
    m_client->resize(m_client->float_dim).move(m_client->float_pos);

    m_clients.active_workspace()->raise_client(m_client);
    m_windowstack.apply(m_clients.active_workspace());
    m_clients.active_workspace()->arrange();
    m_sidebar.draw_clientstate();
}

void
clientfullscreencommand_t::execute()
{
    m_clients.set_fullscreen(m_client, clientaction_t::toggle);
    m_sidebar.draw_clientstate();
}

void
clientinwindowcommand_t::execute()
{
    m_clients.set_inwindow(m_client, clientaction_t::toggle);
    m_clients.refullscreen_clients();
    m_sidebar.draw_clientstate();
}

void
clientstickycommand_t::execute()
{
    if (m_client->parent)
        return;

    m_clients.set_sticky(m_client, clientaction_t::toggle);
}

void
clientabovecommand_t::execute()
{
    m_clients.set_above(m_client, clientaction_t::toggle);
    m_sidebar.draw_clientstate();
}

void
clientbelowcommand_t::execute()
{
    m_clients.set_below(m_client, clientaction_t::toggle);
    m_sidebar.draw_clientstate();
}

void
clientcentercommand_t::execute()
{
    if (!(!m_client->in_window && m_client->fullscreen) && (m_client->floating
        || ((!m_client->sticky && m_clients.client_user_workspace(m_client)->in_float_layout())
        || (m_client->sticky && m_clients.active_workspace()->in_float_layout()))))
    {
        m_client->center();
    }
}

void
clientsnapmovenorthcommand_t::execute()
{
    if (!(!m_client->in_window && m_client->fullscreen) && (m_client->floating
        || ((!m_client->sticky && m_clients.client_user_workspace(m_client)->in_float_layout())
        || (m_client->sticky && m_clients.active_workspace()->in_float_layout()))))
    {
        m_client->snapmove(snapedge_t::north);
    }
}

void
clientsnapmoveeastcommand_t::execute()
{
    if (!(!m_client->in_window && m_client->fullscreen) && (m_client->floating
        || ((!m_client->sticky && m_clients.client_user_workspace(m_client)->in_float_layout())
        || (m_client->sticky && m_clients.active_workspace()->in_float_layout()))))
    {
        m_client->snapmove(snapedge_t::east);
    }
}

void
clientsnapmovesouthcommand_t::execute()
{
    if (!(!m_client->in_window && m_client->fullscreen) && (m_client->floating
        || ((!m_client->sticky && m_clients.client_user_workspace(m_client)->in_float_layout())
        || (m_client->sticky && m_clients.active_workspace()->in_float_layout()))))
    {
        m_client->snapmove(snapedge_t::south);
    }
}

void
clientsnapmovewestcommand_t::execute()
{
    if (!(!m_client->in_window && m_client->fullscreen) && (m_client->floating
        || ((!m_client->sticky && m_clients.client_user_workspace(m_client)->in_float_layout())
        || (m_client->sticky && m_clients.active_workspace()->in_float_layout()))))
    {
        m_client->snapmove(snapedge_t::west);
    }
}

void
clientsnapresizenorthcommand_t::execute()
{
    if (!(!m_client->in_window && m_client->fullscreen) && (m_client->floating
        || ((!m_client->sticky && m_clients.client_user_workspace(m_client)->in_float_layout())
        || (m_client->sticky && m_clients.active_workspace()->in_float_layout()))))
    {
        m_client->snapresize(snapedge_t::north);
    }
}

void
clientsnapresizeeastcommand_t::execute()
{
    if (!(!m_client->in_window && m_client->fullscreen) && (m_client->floating
        || ((!m_client->sticky && m_clients.client_user_workspace(m_client)->in_float_layout())
        || (m_client->sticky && m_clients.active_workspace()->in_float_layout()))))
    {
        m_client->snapresize(snapedge_t::east);
    }
}

void
clientsnapresizesouthcommand_t::execute()
{
    if (!(!m_client->in_window && m_client->fullscreen) && (m_client->floating
        || ((!m_client->sticky && m_clients.client_user_workspace(m_client)->in_float_layout())
        || (m_client->sticky && m_clients.active_workspace()->in_float_layout()))))
    {
        m_client->snapresize(snapedge_t::south);
    }
}

void
clientsnapresizewestcommand_t::execute()
{
    if (!(!m_client->in_window && m_client->fullscreen) && (m_client->floating
        || ((!m_client->sticky && m_clients.client_user_workspace(m_client)->in_float_layout())
        || (m_client->sticky && m_clients.active_workspace()->in_float_layout()))))
    {
        m_client->snapresize(snapedge_t::west);
    }
}

void
clienticonifycommand_t::execute()
{
    m_clients.set_iconified(m_client, clientaction_t::add);
}

void
clientdisowncommand_t::execute()
{
    m_clients.set_disowned(m_client, clientaction_t::add);
}

void
clientkillcommand_t::execute()
{
    m_client->win.force_close();
}

void
clientmoveforwardcommand_t::execute()
{
    m_clients.active_workspace()->move_forward();
}

void
clientmovebackwardcommand_t::execute()
{
    m_clients.active_workspace()->move_backward();
}

void
clientmarksetcommand_t::execute()
{
    m_clients.set_marked(m_client);
}

void
clientmarkjumpcommand_t::execute()
{
    m_clients.jump_marked();
}

void
clientmasterjumpcommand_t::execute()
{
    auto clients = m_clients.active_workspace()->get_all();

    if (!clients.empty()) {
        m_clients.active_workspace()->set_focused(clients.front());
        m_clients.sync_workspace_focus();
    }
}

void
clientstackjumpcommand_t::execute()
{
    auto clients = m_clients.active_workspace()->get_all();
    auto nmaster = m_clients.active_workspace()->get_nmaster();

    if (!clients.empty() && nmaster < clients.size()) {
        m_clients.active_workspace()->set_focused(clients[nmaster]);
        m_clients.sync_workspace_focus();
    }
}

void
clientlastjumpcommand_t::execute()
{
    auto clients = m_clients.active_workspace()->get_all();

    if (!clients.empty()) {
        m_clients.active_workspace()->set_focused(clients.back());
        m_clients.sync_workspace_focus();
    }
}

void
clientpanejumpcommand_t::execute()
{
    m_clients.active_workspace()->jump_pane();
    m_clients.sync_workspace_focus();
}

void
clientjumpindexcommand_t::execute()
{
    auto clients = m_clients.active_workspace()->get_all();

    if (clients.size() > m_index) {
        m_clients.active_workspace()->set_focused(clients[m_index]);
        m_clients.sync_workspace_focus();
    }
}

void
clientworkspacecommand_t::execute()
{
    m_clients.client_to_workspace(m_client, m_number);
    m_clients.active_workspace()->arrange();
    m_clients.refullscreen_clients();
}

void
clientnextworkspacecommand_t::execute()
{
    auto workspace_nr = m_clients.active_workspace()->get_number();

    workspace_nr %= USER_WORKSPACES.size();
    m_clients.client_to_workspace(m_client, workspace_nr + 1);
    m_clients.active_workspace()->arrange();
    m_clients.refullscreen_clients();
}

void
clientpreviousworkspacecommand_t::execute()
{
    auto workspace_nr = m_clients.active_workspace()->get_number() - 1;

    workspace_nr = (workspace_nr == 0) ? USER_WORKSPACES.size() : workspace_nr;
    m_clients.client_to_workspace(m_client, workspace_nr);
    m_clients.active_workspace()->arrange();
    m_clients.refullscreen_clients();
}

void
clientcontextcommand_t::execute()
{
    m_clients.client_to_context(m_client, m_number);
    m_clients.active_workspace()->arrange();
    m_clients.refullscreen_clients();
}

void
clientnextcontextcommand_t::execute()
{
    auto context_nr = m_clients.active_context()->get_letter() - 'a' + 1;

    context_nr %= CONTEXTS.size();
    m_clients.client_to_context(m_client, context_nr + 1);
    m_clients.active_context()->arrange();
    m_clients.refullscreen_clients();
}

void
clientpreviouscontextcommand_t::execute()
{
    auto context_nr = m_clients.active_context()->get_letter() - 'a';

    context_nr = (context_nr == 0) ? CONTEXTS.size() : context_nr;
    m_clients.client_to_context(m_client, context_nr);
    m_clients.active_context()->arrange();
    m_clients.refullscreen_clients();
}

void
clientgrowcommand_t::execute()
{
    if (!(m_client->floating || m_clients.active_workspace()->in_float_layout()))
        return;

    auto dim = m_client->float_dim;
    auto pos = m_client->float_pos;

    switch (m_direction) {
    case direction_t::up:    pos.y -= m_increment; dim.h += m_increment; break;
    case direction_t::right: dim.w += m_increment;                       break;
    case direction_t::down:  dim.h += m_increment;                       break;
    case direction_t::left:  pos.x -= m_increment; dim.w += m_increment; break;
    default: return;
    }

    m_client->resize(dim).move(pos);
}

void
clientshrinkcommand_t::execute()
{
    if (!(m_client->floating || m_clients.active_workspace()->in_float_layout()))
        return;

    auto dim = m_client->float_dim;
    auto pos = m_client->float_pos;

    switch (m_direction) {
    case direction_t::up:    pos.y += m_increment; dim.h -= m_increment; break;
    case direction_t::right: dim.w -= m_increment;                       break;
    case direction_t::down:  dim.h -= m_increment;                       break;
    case direction_t::left:  pos.x += m_increment; dim.w -= m_increment; break;
    default: return;
    }

    m_client->resize(dim).move(pos);
}

void
clientmovecommand_t::execute()
{
    auto pos = m_client->float_pos;

    switch (m_direction) {
    case direction_t::up:    pos.y -= m_increment; break;
    case direction_t::right: pos.x += m_increment; break;
    case direction_t::down:  pos.y += m_increment; break;
    case direction_t::left:  pos.x -= m_increment; break;
    default: return;
    }

    m_client->move(pos);
}

void
clientmovemousecommand_t::execute()
{
    m_clients.start_moving(m_client);
}

void
clientresizemousecommand_t::execute()
{
    m_clients.start_resizing(m_client);
}

void
masterforwardcommand_t::execute()
{
    m_clients.active_workspace()->rotate_master_forward();
    auto clients = m_clients.active_workspace()->get_all();

    if (clients.size() && m_clients.active_workspace()->stack_focused()) {
        m_clients.active_workspace()->raise_client(clients.front());
        m_windowstack.apply(m_clients.active_workspace());
    }

    m_clients.focus(m_clients.active_workspace()->get_focused());
    m_clients.sync_workspace_focus();
}

void
masterbackwardcommand_t::execute()
{
    m_clients.active_workspace()->rotate_master_backward();
    auto clients = m_clients.active_workspace()->get_all();

    if (clients.size() && m_clients.active_workspace()->stack_focused()) {
        m_clients.active_workspace()->raise_client(clients.front());
        m_windowstack.apply(m_clients.active_workspace());
    }

    m_clients.focus(m_clients.active_workspace()->get_focused());
    m_clients.sync_workspace_focus();
}

void
stackforwardcommand_t::execute()
{
    m_clients.active_workspace()->rotate_stack_forward();
    auto nmaster = m_clients.active_workspace()->get_nmaster();
    auto clients = m_clients.active_workspace()->get_all();

    if (clients.size() && nmaster < clients.size()  && m_clients.active_workspace()->master_focused()) {
        m_clients.active_workspace()->raise_client(clients[nmaster]);
        m_windowstack.apply(m_clients.active_workspace());
    }

    m_clients.focus(m_clients.active_workspace()->get_focused());
    m_clients.sync_workspace_focus();
}

void
stackbackwardcommand_t::execute()
{
    m_clients.active_workspace()->rotate_stack_backward();
    auto nmaster = m_clients.active_workspace()->get_nmaster();
    auto clients = m_clients.active_workspace()->get_all();

    if (clients.size() && nmaster < clients.size() && m_clients.active_workspace()->master_focused()) {
        m_clients.active_workspace()->raise_client(clients[nmaster]);
        m_windowstack.apply(m_clients.active_workspace());
    }

    m_clients.focus(m_clients.active_workspace()->get_focused());
    m_clients.sync_workspace_focus();
}

void
allforwardcommand_t::execute()
{
    m_clients.active_workspace()->rotate_clients_forward();
    auto nmaster = m_clients.active_workspace()->get_nmaster();
    auto clients = m_clients.active_workspace()->get_all();

    if (clients.size() && m_clients.active_workspace()->stack_focused()) {
        m_clients.active_workspace()->raise_client(clients.front());
        m_windowstack.apply(m_clients.active_workspace());
    } else if (clients.size() && nmaster < clients.size()  && m_clients.active_workspace()->master_focused()) {
        m_clients.active_workspace()->raise_client(clients[nmaster]);
        m_windowstack.apply(m_clients.active_workspace());
    }

    m_clients.focus(m_clients.active_workspace()->get_focused());
    m_clients.sync_workspace_focus();
}

void
allbackwardcommand_t::execute()
{
    m_clients.active_workspace()->rotate_clients_backward();
    auto nmaster = m_clients.active_workspace()->get_nmaster();
    auto clients = m_clients.active_workspace()->get_all();

    if (clients.size() && m_clients.active_workspace()->stack_focused()) {
        m_clients.active_workspace()->raise_client(clients.front());
        m_windowstack.apply(m_clients.active_workspace());
    } else if (clients.size() && nmaster < clients.size()  && m_clients.active_workspace()->master_focused()) {
        m_clients.active_workspace()->raise_client(clients[nmaster]);
        m_windowstack.apply(m_clients.active_workspace());
    }

    m_clients.focus(m_clients.active_workspace()->get_focused());
    m_clients.sync_workspace_focus();
}

void
clienticonifyindexcommand_t::execute()
{
    auto clients = m_clients.active_workspace()->get_all();

    if (clients.size() > m_index)
        m_clients.set_iconified(clients[m_index], clientaction_t::add);
}

void
deiconifypopcommand_t::execute()
{
    auto icons = m_clients.active_workspace()->get_icons();

    if (!icons.empty())
        m_clients.set_iconified(icons.back(), clientaction_t::remove);
}

void
clientdeiconifyindexcommand_t::execute()
{
    auto icons = m_clients.active_workspace()->get_icons();

    if (icons.size() > m_index)
        m_clients.set_iconified(icons[m_index], clientaction_t::remove);
}

void
reclaimpopcommand_t::execute()
{
    auto disowned = m_clients.active_workspace()->get_disowned();

    if (!disowned.empty())
        m_clients.set_disowned(disowned.back(), clientaction_t::remove);
}

void
profilesavecommand_t::execute()
{
    m_clients.save_profile(m_number);
}

void
profileloadcommand_t::execute()
{
    m_clients.apply_profile(m_number);
    m_sidebar.draw();
}

void
workspacesetcommand_t::execute()
{
    m_clients.change_active_workspace(m_number);
    m_sidebar.draw();
}

void
nextworkspacecommand_t::execute()
{
    auto workspace = m_clients.active_workspace()->get_number();

    workspace %= USER_WORKSPACES.size();
    m_clients.change_active_workspace(workspace + 1, false);
    m_sidebar.draw();
}

void
previousworkspacecommand_t::execute()
{
    auto workspace_nr = m_clients.active_workspace()->get_number() - 1;

    workspace_nr = (workspace_nr == 0) ? USER_WORKSPACES.size() : workspace_nr;
    m_clients.change_active_workspace(workspace_nr, false);
    m_sidebar.draw();
}

void
workspacemirrorcommand_t::execute()
{
    m_clients.active_workspace()->mirror().arrange();
}

void
workspacemfactorcommand_t::execute()
{
    auto mfactor = m_clients.active_workspace()->get_mfactor();

    if ((m_delta < 0 && mfactor >= .15f) || (m_delta > 0 && mfactor <= 0.85f))
        m_clients.active_workspace()->set_mfactor(mfactor + m_delta).arrange();
}

void
workspacenmastercommand_t::execute()
{
    auto nmaster = m_clients.active_workspace()->get_nmaster();

    if ((m_delta < 0 && nmaster > 0) || (m_delta > 0 && nmaster < MAX_NMASTER))
        m_clients.active_workspace()->set_nmaster(nmaster + m_delta).arrange();
}

void
workspacegapsizecommand_t::execute()
{
    auto gapsize = m_clients.active_workspace()->get_gapsize();

    if ((m_delta < 0 && gapsize > 0) || (m_delta > 0 && gapsize < MAX_GAPSIZE))
        m_clients.active_workspace()->set_gapsize(gapsize + m_delta).arrange();
    else if (m_delta == 0)
        m_clients.active_workspace()->set_gapsize(0).arrange();
}

void
sidebarshowcommand_t::execute()
{
    m_sidebar.toggle();
    m_clients.active_workspace()->arrange();
    m_clients.refullscreen_clients();
}

void
sidebarshowallcommand_t::execute()
{
    m_sidebar.toggle_all();
    m_clients.active_workspace()->arrange();
    m_clients.refullscreen_clients();
}

void
contextsetcommand_t::execute()
{
    m_clients.change_active_context(m_number);
}

void
nextcontextcommand_t::execute()
{
    auto context = m_clients.active_context()->get_letter() - 'a' + 1;

    context %= CONTEXTS.size();
    m_clients.change_active_context(context + 1);
}

void
previouscontextcommand_t::execute()
{
    auto context = m_clients.active_context()->get_letter() - 'a';

    context = (context == 0) ? CONTEXTS.size() : context;
    m_clients.change_active_context(context);
}

void
workspacelayoutcommand_t::execute()
{
    m_clients.active_workspace()->set_layout(m_layout);
    m_windowstack.apply(m_clients.active_workspace());
    m_clients.active_workspace()->arrange();
    m_sidebar.draw_layoutsymbol();
}

void
workspacelayoutpreservedimcommand_t::execute()
{
    m_clients.active_workspace()->set_layout(m_layout);
    m_windowstack.apply(m_clients.active_workspace());

    for (auto& client : m_clients.active_workspace()->get_all())
        client->resize(client->dim).move(client->pos);

    m_clients.active_workspace()->arrange();
    m_sidebar.draw_layoutsymbol();
}

void
focusforwardcommand_t::execute()
{
    m_clients.cycle_focus_forward();
}

void
focusbackwardcommand_t::execute()
{
    m_clients.cycle_focus_backward();
}

void
externalcommand_t::execute()
{
    if (!fork()) {
        if (x_data::g_dpy)
            close(x_data::connection_number());

        setsid();
        execl("/bin/sh", "/bin/sh", "-c", ("exec " + m_command).c_str(), NULL);
        exit(EXIT_SUCCESS);
    }
}
