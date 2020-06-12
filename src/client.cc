#include "client.hh"
#include "common.hh"
#include "decoration.hh"
#include "x-data/event.hh"
#include "x-data/attributes.hh"
#include "x-data/hints.hh"


client_ptr_t
create_client(x_data::window_t win, rule_t& rule)
{
    auto sizehints = x_data::get_sizehints(win);

    dim_t base{}, inc{}, max{}, min{};
    range_t<float> aspect{};

    if (sizehints.flag_set(PBaseSize))
        base = sizehints.base_dim();
    else if (sizehints.flag_set(PMinSize))
        base = sizehints.min_dim();

    if (sizehints.flag_set(PResizeInc))
        inc = sizehints.inc_dim();

    if (sizehints.flag_set(PMaxSize))
        max = sizehints.max_dim();

    if (sizehints.flag_set(PMinSize))
        min = sizehints.min_dim();
    else if (sizehints.flag_set(PBaseSize))
        min = sizehints.base_dim();

    if (sizehints.flag_set(PAspect))
        aspect = {static_cast<float>(sizehints.min_aspect().y) / sizehints.min_aspect().x,
            static_cast<float>(sizehints.max_aspect().y) / sizehints.max_aspect().x};

    sizeconstraints_t sizeconstraints(base, inc, max, min, aspect);

    auto win_attrs = x_data::get_attributes(win);
    pos_t pos = win_attrs;
    dim_t dim = win_attrs;
    sizeconstraints.apply(pos, dim);

    x_data::window_t frame = x_data::create_window();
    x_data::select_input(win, REG_WIN_SELECTION);
    x_data::select_input(frame, REG_FRAME_SELECTION);

    frame.set_background_color(REG_COLOR);
    win.reparent({0, BORDER_HEIGHT}, frame);

    rule.fullscreen |= win.is_of_state("FULLSCREEN");
    rule.floating |= rule.fullscreen
        || sizeconstraints.is_fixed()
        || win.is_of_type("DIALOG")
        || win.is_of_type("UTILITY")
        || win.is_of_type("SPLASH");

    client_ptr_t client = new client_t(win, frame, sizeconstraints, rule);
    client->resize({dim.w, dim.h + BORDER_HEIGHT});
    client->move({pos.x, pos.y - BORDER_HEIGHT});

    client->above = client->win.is_of_state("ABOVE");
    client->below = client->win.is_of_state("BELOW");

    return client;
}


void
update_offset(client_ptr_t client)
{
    auto frame_attrs = x_data::get_attributes(client->frame);
    auto win_attrs = x_data::get_attributes(client->win);

    x_data::event_t event;
    event.get().type = ConfigureNotify;
    event.get().xconfigure.send_event = True;
    event.get().xconfigure.display = x_data::g_dpy;
    event.get().xconfigure.event = client->win;
    event.get().xconfigure.window = client->win;
    event.get().xconfigure.x = frame_attrs.x();
    event.get().xconfigure.y = frame_attrs.y() + BORDER_HEIGHT;
    event.get().xconfigure.width = win_attrs.w();
    event.get().xconfigure.height = win_attrs.h();
    event.get().xconfigure.border_width = 0;
    event.get().xconfigure.above = None;
    event.get().xconfigure.override_redirect = True;

    event.send(StructureNotifyMask);
}

void
client_t::disown_child(client_ptr_t child)
{
    child->parent = nullptr;
    children.erase(child);
}

bool
client_t::consume_expect(clientexpect_t occurred)
{
    if (occurred == expect) {
        switch (expect) {
        case clientexpect_t::map:      win.set_state(NormalState);    break;
        case clientexpect_t::withdraw: win.set_state(WithdrawnState); break;
        case clientexpect_t::iconify:
            {
                win.set_state(IconicState);
                x_data::select_input(frame, NONSUBSTR_FRAME_SELECTION);
                win.unmap();
                x_data::select_input(frame, REG_FRAME_SELECTION);
            }
            break;
        case clientexpect_t::deiconify:
            {
                win.set_state(NormalState);
                x_data::select_input(frame, NONSUBSTR_FRAME_SELECTION);
                win.map();
                x_data::select_input(frame, REG_FRAME_SELECTION);
            }
            break;
        default: break;
        }

        expect = clientexpect_t::noeffect;
        return true;
    }

    return false;
}

void
client_t::must_expect(clientexpect_t to_expect)
{
    expect = to_expect;
}

client_t&
client_t::set_float(clientaction_t action)
{
    switch (action) {
    case clientaction_t::add:    floating = true;      break;
    case clientaction_t::remove: floating = false;     break;
    case clientaction_t::toggle: floating = !floating; break;
    default: break;
    }

    return *this;
}

client_t&
client_t::move(pos_t new_pos, bool tiled)
{
    frame.move(new_pos);

    if (tiled)
        pos = new_pos;
    else
        float_pos = new_pos;

    update_offset(this);
    return *this;
}

client_t&
client_t::resize(dim_t new_dim, bool tiled)
{
    const static auto do_resize([](auto win, auto w, auto h){ win.resize({w, h}); });
    dim_t old_dim = tiled ? dim : float_dim;

    if (new_dim.w >= old_dim.w) {
        do_resize(win, new_dim.w, old_dim.h - BORDER_HEIGHT);
        do_resize(frame, new_dim.w, old_dim.h);
    } else {
        do_resize(frame, new_dim.w, old_dim.h);
        do_resize(win, new_dim.w, old_dim.h - BORDER_HEIGHT);
    }

    if (new_dim.h >= old_dim.h) {
        if (new_dim.h > BORDER_HEIGHT)
            do_resize(win, new_dim.w, new_dim.h - BORDER_HEIGHT);
        frame.resize(new_dim);
    } else {
        frame.resize(new_dim);
        if (new_dim.h > BORDER_HEIGHT)
            do_resize(win, new_dim.w, new_dim.h - BORDER_HEIGHT);
    }

    if (tiled)
        dim = new_dim;
    else
        float_dim = new_dim;

    update_offset(this);
    return *this;
}

client_t&
client_t::moveresize(pos_t new_pos, dim_t new_dim, bool tiled)
{
    frame.moveresize(new_pos, new_dim);
    if (new_dim.h > BORDER_HEIGHT)
        win.resize({new_dim.w, new_dim.h - BORDER_HEIGHT});

    if (tiled) {
        pos = new_pos;
        dim = new_dim;
    } else {
        float_pos = new_pos;
        float_dim = new_dim;
    }

    update_offset(this);
    return *this;
}

client_t&
client_t::map()
{
    win.map();
    frame.map();
    return *this;
}

client_t&
client_t::unmap()
{
    frame.unmap();
    return *this;
}

client_t&
client_t::map_children()
{
    for (auto& child : children) {
        child->expect = clientexpect_t::map;
        child->map();
    }

    return *this;
}


client_t&
client_t::unmap_children()
{
    for (auto& child : children) {
        child->expect = clientexpect_t::withdraw;
        child->unmap();
    }

    return *this;
}

client_t&
client_t::center()
{
    auto attrs = x_data::get_attributes(frame);
    auto root_attrs = x_data::get_attributes(x_data::g_root);

    move({root_attrs.w() / 2 - attrs.w() / 2, root_attrs.h() / 2 - attrs.h() / 2});
    return *this;
}

client_t&
client_t::snapmove(snapedge_t edge)
{
    auto attrs = x_data::get_attributes(frame);
    auto root_attrs = x_data::get_attributes(x_data::g_root);

    switch (edge) {
    case snapedge_t::north: move({attrs.x(), SNAP_OFFSET});                              break;
    case snapedge_t::east:  move({root_attrs.w() - attrs.w() - SNAP_OFFSET, attrs.y()}); break;
    case snapedge_t::south: move({attrs.x(), root_attrs.h() - attrs.h() - SNAP_OFFSET}); break;
    case snapedge_t::west:  move({SNAP_OFFSET, attrs.y()});                              break;
    default: break;
    }

    return *this;
}

client_t&
client_t::snapresize(snapedge_t edge)
{
    auto attrs = x_data::get_attributes(frame);
    auto root_attrs = x_data::get_attributes(x_data::g_root);

    switch (edge) {
    case snapedge_t::north: move({attrs.x(), SNAP_OFFSET}); resize({attrs.w(), attrs.y() + attrs.h() - SNAP_OFFSET}); break;
    case snapedge_t::east:  resize({root_attrs.w() - SNAP_OFFSET - attrs.x(), attrs.h()});                            break;
    case snapedge_t::south: resize({attrs.w(), root_attrs.h() - SNAP_OFFSET - attrs.y()});                            break;
    case snapedge_t::west:  move({SNAP_OFFSET, attrs.y()}); resize({attrs.x() + attrs.w() - SNAP_OFFSET, attrs.h()}); break;
    default: break;
    }

    return *this;
}
