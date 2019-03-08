#include "floating.hh"

void
moveresize_t::process_move_increment(pos_t pos, dim_t size, pos_t delta)
{
    pos = {pos.x + delta.x, pos.y + delta.y};
    client->sizeconstraints.apply(pos, size);
    client->move(pos);
}

void
moveresize_t::process_resize_increment(pos_t pos, dim_t size, pos_t delta)
{
    switch (grabbed_at) {
    case TOP_LEFT:
        {
            int resize_width  = ::std::max(size.w - delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(size.h - delta.y, MIN_WINDOW_SIZE);

            pos_t new_pos = {pos.x + (size.w - resize_width), pos.y + (size.h - resize_height)};
            dim_t new_size = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_size);
            client->resize({new_size.w, new_size.h + BORDER_HEIGHT});

            if (!(new_size == dim_t{size.w, size.h})) {
                new_pos = {pos.x + (size.w - new_size.w), pos.y + (size.h - new_size.h)};
                client->move(new_pos);
            }
        }
        break;
    case TOP_RIGHT:
        {
            int resize_width  = ::std::max(size.w + delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(size.h - delta.y, MIN_WINDOW_SIZE);

            pos_t new_pos = {pos.x, pos.y + (size.h - resize_height)};
            dim_t new_size = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_size);
            client->resize({new_size.w, new_size.h + BORDER_HEIGHT});

            if (!(new_size == dim_t{size.w, size.h})) {
                new_pos = {pos.x, pos.y + (size.h - new_size.h)};
                client->move(new_pos);
            }
        }
        break;
    case BOTTOM_LEFT:
        {
            int resize_width  = ::std::max(size.w - delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(size.h + delta.y, MIN_WINDOW_SIZE);

            pos_t new_pos = {pos.x + (size.w - resize_width), pos.y};
            dim_t new_size = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_size);
            client->resize({new_size.w, new_size.h + BORDER_HEIGHT});

            if (!(new_size == dim_t{size.w, size.h})) {
                new_pos = {pos.x + (size.w - new_size.w), pos.y};
                client->move(new_pos);
            }
        }
        break;
    case BOTTOM_RIGHT:
        {
            int resize_width  = ::std::max(size.w + delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(size.h + delta.y, MIN_WINDOW_SIZE);

            pos_t new_pos = pos;
            dim_t new_size = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_size);
            client->resize({new_size.w, new_size.h + BORDER_HEIGHT});
        }
        break;
    default: break;
    }
}
