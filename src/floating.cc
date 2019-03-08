#include "floating.hh"

void
moveresize_t::process_move_increment(Pos pos, Size size, Pos delta)
{
    pos = {pos.x + delta.x, pos.y + delta.y};
    client->sizeconstraints.apply(pos, size);
    client->move(pos);
}

void
moveresize_t::process_resize_increment(Pos pos, Size size, Pos delta)
{
    switch (grabbed_at) {
    case TOP_LEFT:
        {
            int resize_width  = ::std::max(size.w - delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(size.h - delta.y, MIN_WINDOW_SIZE);

            Pos new_pos = {pos.x + (size.w - resize_width), pos.y + (size.h - resize_height)};
            Size new_size = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_size);
            client->resize({new_size.w, new_size.h + BORDER_HEIGHT});

            if (!(new_size == Size{size.w, size.h})) {
                new_pos = {pos.x + (size.w - new_size.w), pos.y + (size.h - new_size.h)};
                client->move(new_pos);
            }
        }
        break;
    case TOP_RIGHT:
        {
            int resize_width  = ::std::max(size.w + delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(size.h - delta.y, MIN_WINDOW_SIZE);

            Pos new_pos = {pos.x, pos.y + (size.h - resize_height)};
            Size new_size = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_size);
            client->resize({new_size.w, new_size.h + BORDER_HEIGHT});

            if (!(new_size == Size{size.w, size.h})) {
                new_pos = {pos.x, pos.y + (size.h - new_size.h)};
                client->move(new_pos);
            }
        }
        break;
    case BOTTOM_LEFT:
        {
            int resize_width  = ::std::max(size.w - delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(size.h + delta.y, MIN_WINDOW_SIZE);

            Pos new_pos = {pos.x + (size.w - resize_width), pos.y};
            Size new_size = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_size);
            client->resize({new_size.w, new_size.h + BORDER_HEIGHT});

            if (!(new_size == Size{size.w, size.h})) {
                new_pos = {pos.x + (size.w - new_size.w), pos.y};
                client->move(new_pos);
            }
        }
        break;
    case BOTTOM_RIGHT:
        {
            int resize_width  = ::std::max(size.w + delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(size.h + delta.y, MIN_WINDOW_SIZE);

            Pos new_pos = pos;
            Size new_size = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_size);
            client->resize({new_size.w, new_size.h + BORDER_HEIGHT});
        }
        break;
    }
}
