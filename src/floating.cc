#include "floating.hh"


void
moveresize_t::process_move_increment(pos_t pos, dim_t dim, pos_t delta)
{
    pos = {pos.x + delta.x, pos.y + delta.y};
    client->sizeconstraints.apply(pos, dim);
    client->move(pos);
}

void
moveresize_t::process_resize_increment(pos_t pos, dim_t dim, pos_t delta)
{
    switch (grabbed_at) {
    case corner_t::top_left:
        {
            int resize_width  = ::std::max(dim.w - delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(dim.h - delta.y, MIN_WINDOW_SIZE);

            pos_t new_pos = {pos.x + (dim.w - resize_width), pos.y + (dim.h - resize_height)};
            dim_t new_dim = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_dim);
            client->resize({new_dim.w, new_dim.h + BORDER_HEIGHT});

            if (!(new_dim == dim_t{dim.w, dim.h})) {
                new_pos = {pos.x + (dim.w - new_dim.w), pos.y + (dim.h - new_dim.h)};
                client->move(new_pos);
            }
        }
        break;
    case corner_t::top_right:
        {
            int resize_width  = ::std::max(dim.w + delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(dim.h - delta.y, MIN_WINDOW_SIZE);

            pos_t new_pos = {pos.x, pos.y + (dim.h - resize_height)};
            dim_t new_dim = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_dim);
            client->resize({new_dim.w, new_dim.h + BORDER_HEIGHT});

            if (!(new_dim == dim_t{dim.w, dim.h})) {
                new_pos = {pos.x, pos.y + (dim.h - new_dim.h)};
                client->move(new_pos);
            }
        }
        break;
    case corner_t::bottom_left:
        {
            int resize_width  = ::std::max(dim.w - delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(dim.h + delta.y, MIN_WINDOW_SIZE);

            pos_t new_pos = {pos.x + (dim.w - resize_width), pos.y};
            dim_t new_dim = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_dim);
            client->resize({new_dim.w, new_dim.h + BORDER_HEIGHT});

            if (!(new_dim == dim_t{dim.w, dim.h})) {
                new_pos = {pos.x + (dim.w - new_dim.w), pos.y};
                client->move(new_pos);
            }
        }
        break;
    case corner_t::bottom_right:
        {
            int resize_width  = ::std::max(dim.w + delta.x, MIN_WINDOW_SIZE);
            int resize_height = ::std::max(dim.h + delta.y, MIN_WINDOW_SIZE);

            pos_t new_pos = pos;
            dim_t new_dim = {resize_width, resize_height};

            client->sizeconstraints.apply(new_pos, new_dim);
            client->resize({new_dim.w, new_dim.h + BORDER_HEIGHT});
        }
        break;
    default: break;
    }
}
