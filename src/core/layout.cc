#include "../winsys/util.hh"
#include "client.hh"
#include "cycle.t.hh"
#include "defaults.hh"
#include "layout.hh"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <fstream>

using namespace winsys;

LayoutHandler::Layout::Layout(LayoutKind kind)
    : kind(kind),
      config(kind_to_config(kind)),
      default_data(kind_to_default_data(kind)),
      data({}, true)
{
    data.insert_at_back(new LayoutData(default_data));
    data.insert_at_back(new LayoutData(default_data));
    data.insert_at_back(new LayoutData(default_data));
}

LayoutHandler::Layout::~Layout()
{
    for (LayoutData_ptr data : data)
        delete data;
}


LayoutHandler::LayoutHandler()
    : m_kind(LayoutKind::Float),
      m_prev_kind(LayoutKind::Float),
      m_layouts({
          { LayoutKind::Float,         new Layout(LayoutKind::Float) },
          { LayoutKind::BLFloat,       new Layout(LayoutKind::BLFloat) },
          { LayoutKind::SingleFloat,   new Layout(LayoutKind::SingleFloat) },
          { LayoutKind::BLSingleFloat, new Layout(LayoutKind::BLSingleFloat) },
          { LayoutKind::Center,        new Layout(LayoutKind::Center) },
          { LayoutKind::Monocle,       new Layout(LayoutKind::Monocle) },
          { LayoutKind::Paper,         new Layout(LayoutKind::Paper) },
          { LayoutKind::SPaper,        new Layout(LayoutKind::SPaper) },
          { LayoutKind::Stack,         new Layout(LayoutKind::Stack) },
          { LayoutKind::SStack,        new Layout(LayoutKind::SStack) },
          { LayoutKind::BStack,        new Layout(LayoutKind::BStack) },
          { LayoutKind::SBStack,       new Layout(LayoutKind::SBStack) },
          { LayoutKind::Horz,          new Layout(LayoutKind::Horz) },
          { LayoutKind::SHorz,         new Layout(LayoutKind::SHorz) },
          { LayoutKind::Vert,          new Layout(LayoutKind::Vert) },
          { LayoutKind::SVert,         new Layout(LayoutKind::SVert) }
      }),
      mp_layout(m_layouts.at(m_kind)),
      mp_prev_layout(m_layouts.at(m_kind))
{}

LayoutHandler::~LayoutHandler()
{
    for (auto [_,layout] : m_layouts)
        delete layout;
}


void
LayoutHandler::arrange(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    if (mp_layout->config.margin) {
        Layout::LayoutData_ptr data = *mp_layout->data.active_element();

        screen_region.pos.x += data->margin.left;
        screen_region.pos.y += data->margin.top;

        screen_region.dim.w -= data->margin.left + data->margin.right;
        screen_region.dim.h -= data->margin.top + data->margin.bottom;
    }

    switch (m_kind) {
        case LayoutKind::Float:         arrange_float(screen_region, placements, begin, end);         break;
        case LayoutKind::BLFloat:       arrange_blfloat(screen_region, placements, begin, end);       break;
        case LayoutKind::SingleFloat:   arrange_singlefloat(screen_region, placements, begin, end);   break;
        case LayoutKind::BLSingleFloat: arrange_blsinglefloat(screen_region, placements, begin, end); break;
        case LayoutKind::Center:        arrange_center(screen_region, placements, begin, end);        break;
        case LayoutKind::Monocle:       arrange_monocle(screen_region, placements, begin, end);       break;
        case LayoutKind::Paper:         arrange_paper(screen_region, placements, begin, end);         break;
        case LayoutKind::SPaper:        arrange_spaper(screen_region, placements, begin, end);        break;
        case LayoutKind::Stack:         arrange_stack(screen_region, placements, begin, end);         break;
        case LayoutKind::SStack:        arrange_sstack(screen_region, placements, begin, end);        break;
        case LayoutKind::BStack:        arrange_bstack(screen_region, placements, begin, end);        break;
        case LayoutKind::SBStack:       arrange_sbstack(screen_region, placements, begin, end);       break;
        case LayoutKind::Horz:          arrange_horz(screen_region, placements, begin, end);          break;
        case LayoutKind::SHorz:         arrange_shorz(screen_region, placements, begin, end);         break;
        case LayoutKind::Vert:          arrange_vert(screen_region, placements, begin, end);          break;
        case LayoutKind::SVert:         arrange_svert(screen_region, placements, begin, end);         break;
    }

    if (mp_layout->config.gap) {
        Layout::LayoutData_ptr data = *mp_layout->data.active_element();

        std::for_each(
            placements.begin(),
            placements.end(),
            [data](Placement& placement) {
                if (placement.region && !Client::is_free(placement.client)) {
                    placement.region->pos.x += data->gap_size;
                    placement.region->pos.y += data->gap_size;
                    placement.region->dim.w -= 2 * data->gap_size;
                    placement.region->dim.h -= 2 * data->gap_size;

                    placement.region->apply_minimum_dim(Client::MIN_CLIENT_DIM);
                }
            }
        );
    }
}


LayoutHandler::LayoutKind
LayoutHandler::kind() const
{
    return m_kind;
}

void
LayoutHandler::set_kind(LayoutKind kind)
{
    if (kind == m_kind)
        return;

    m_prev_kind = m_kind;
    m_kind = kind;

    mp_prev_layout = mp_layout;
    mp_layout = *Util::const_retrieve(m_layouts, m_kind);
}

void
LayoutHandler::set_prev_kind()
{
    if (m_prev_kind == m_kind)
        return;

    std::swap(m_kind, m_prev_kind);
    std::swap(mp_layout, mp_prev_layout);
}


bool
LayoutHandler::layout_is_free() const
{
    return mp_layout->config.method == Placement::PlacementMethod::Free;
}

bool
LayoutHandler::layout_has_margin() const
{
    return mp_layout->config.margin;
}

bool
LayoutHandler::layout_has_gap() const
{
    return mp_layout->config.gap;
}

bool
LayoutHandler::layout_is_persistent() const
{
    return mp_layout->config.persistent;
}

bool
LayoutHandler::layout_is_single() const
{
    return mp_layout->config.single;
}

bool
LayoutHandler::layout_wraps() const
{
    return mp_layout->config.wraps;
}


std::size_t
LayoutHandler::gap_size() const
{
    return (*mp_layout->data.active_element())->gap_size;
}

std::size_t
LayoutHandler::main_count() const
{
    return (*mp_layout->data.active_element())->main_count;
}

float
LayoutHandler::main_factor() const
{
    return (*mp_layout->data.active_element())->main_factor;
}

Extents
LayoutHandler::margin() const
{
    return (*mp_layout->data.active_element())->margin;
}


void
LayoutHandler::copy_data_from_prev_layout()
{
    *(*mp_layout->data.active_element())
        = *(*mp_prev_layout->data.active_element());
}

void
LayoutHandler::set_prev_layout_data()
{
    std::optional<Layout::LayoutData_ptr> prev_data
        = mp_layout->data.prev_active_element();

    if (prev_data)
        mp_layout->data.activate_element(*prev_data);
}


void
LayoutHandler::change_gap_size(Util::Change<int> change)
{
    Layout::LayoutData_ptr data = *mp_layout->data.active_element();
    int value = static_cast<int>(data->gap_size) + change;

    if (value <= 0)
        data->gap_size = 0;
    else if (static_cast<std::size_t>(value) >= Layout::LayoutData::MAX_GAP_SIZE)
        data->gap_size = Layout::LayoutData::MAX_GAP_SIZE;
    else
        data->gap_size = value;
}

void
LayoutHandler::change_main_count(Util::Change<int> change)
{
    Layout::LayoutData_ptr data = *mp_layout->data.active_element();
    int value = static_cast<int>(data->main_count) + change;

    if (value <= 0)
        data->main_count = 0;
    else if (static_cast<std::size_t>(value) >= Layout::LayoutData::MAX_MAIN_COUNT)
        data->main_count = Layout::LayoutData::MAX_MAIN_COUNT;
    else
        data->main_count = value;
}

void
LayoutHandler::change_main_factor(Util::Change<float> change)
{
    Layout::LayoutData_ptr data = *mp_layout->data.active_element();
    float value = data->main_factor + change;

    if (value <= 0.05f)
        data->main_factor = 0.05f;
    else if (value >= 0.95f)
        data->main_factor = 0.95f;
    else
        data->main_factor = value;
}

void
LayoutHandler::change_margin(Util::Change<int> change)
{
    change_margin(Edge::Left, change);
    change_margin(Edge::Top, change);
    change_margin(Edge::Right, change);
    change_margin(Edge::Bottom, change);
}

void
LayoutHandler::change_margin(Edge edge, Util::Change<int> change)
{
    Layout::LayoutData_ptr data = *mp_layout->data.active_element();
    int* margin;
    const int* max_value;

    switch (edge) {
    case Edge::Left:
    {
        margin = &data->margin.left;
        max_value = &Layout::LayoutData::MAX_MARGIN.left;
        break;
    }
    case Edge::Top:
    {
        margin = &data->margin.top;
        max_value = &Layout::LayoutData::MAX_MARGIN.top;
        break;
    }
    case Edge::Right:
    {
        margin = &data->margin.right;
        max_value = &Layout::LayoutData::MAX_MARGIN.right;
        break;
    }
    case Edge::Bottom:
    {
        margin = &data->margin.bottom;
        max_value = &Layout::LayoutData::MAX_MARGIN.bottom;
        break;
    }
    }

    int value = *margin + change;

    if (value <= 0)
        *margin = 0;
    else if (value >= *max_value)
        *margin = *max_value;
    else
        *margin = value;
}

void
LayoutHandler::reset_gap_size()
{
    (*mp_layout->data.active_element())->gap_size
        = mp_layout->default_data.gap_size;
}

void
LayoutHandler::reset_margin()
{
    (*mp_layout->data.active_element())->margin
        = mp_layout->default_data.margin;
}

void
LayoutHandler::reset_layout_data()
{
    *(*mp_layout->data.active_element())
        = mp_layout->default_data;
}

void
LayoutHandler::cycle_layout_data(Direction direction)
{
    mp_layout->data.cycle_active(direction);
}


void
LayoutHandler::save_layout(std::size_t number) const
{
    std::stringstream datadir_ss;
    std::string home_path = std::getenv("HOME");

    if (const char* env_xdgdata = std::getenv("XDG_DATA_HOME"))
        datadir_ss << env_xdgdata << "/" << WM_NAME << "/";
    else
        datadir_ss << home_path << "/.local/share/" << WM_NAME << "/"
                   << "layout_" << number;

    std::string file_path = datadir_ss.str();

    std::vector<Layout::LayoutData> data;
    data.reserve(mp_layout->data.size());
    for (Layout::LayoutData_ptr data_ptr : mp_layout->data.as_deque())
        data.push_back(*data_ptr);

    typename std::vector<Layout::LayoutData>::size_type size
        = data.size();

    std::ofstream out(file_path, std::ios::out | std::ios::binary);
    out.write(reinterpret_cast<const char*>(&m_kind), sizeof(LayoutKind));
    out.write(reinterpret_cast<const char*>(&size), sizeof(size));
    out.write(reinterpret_cast<const char*>(&data[0]),
        data.size() * sizeof(Layout::LayoutData));
    out.close();
}

void
LayoutHandler::load_layout(std::size_t number)
{
    std::stringstream datadir_ss;
    std::string home_path = std::getenv("HOME");

    if (const char* env_xdgdata = std::getenv("XDG_DATA_HOME"))
        datadir_ss << env_xdgdata << "/" << WM_NAME << "/";
    else
        datadir_ss << home_path << "/.local/share/" << WM_NAME << "/"
                   << "layout_" << number;

    std::string file_path = datadir_ss.str();

    LayoutKind kind;
    std::vector<Layout::LayoutData> data;
    typename std::vector<Layout::LayoutData>::size_type size = 0;

    std::ifstream in(file_path, std::ios::in | std::ios::binary);
    if (in.good()) {
        in.read(reinterpret_cast<char*>(&kind), sizeof(LayoutKind));
        in.read(reinterpret_cast<char*>(&size), sizeof(size));

        data.resize(size, mp_layout->default_data);
        in.read(reinterpret_cast<char*>(&data[0]),
            data.size() * sizeof(Layout::LayoutData));

        set_kind(kind);
        for (Layout::LayoutData_ptr data : mp_layout->data)
            delete data;

        mp_layout->data.clear();
        for (auto data_ : data)
            mp_layout->data.insert_at_back(new Layout::LayoutData(data_));
    }
    in.close();
}


void
LayoutHandler::arrange_float(
    Region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    std::transform(
        begin,
        end,
        std::back_inserter(placements),
        [this](Client_ptr client) -> Placement {
            return Placement {
                mp_layout->config.method,
                client,
                mp_layout->config.decoration,
                client->free_region
            };
        }
    );
}

void
LayoutHandler::arrange_blfloat(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    arrange_float(screen_region, placements, begin, end);
}

void
LayoutHandler::arrange_singlefloat(
    Region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    std::transform(
        begin,
        end,
        std::back_inserter(placements),
        [this](Client_ptr client) -> Placement {
            return Placement {
                mp_layout->config.method,
                client,
                mp_layout->config.decoration,
                client->focused ? std::optional(client->free_region) : std::nullopt
            };
        }
    );
}

void
LayoutHandler::arrange_blsinglefloat(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    arrange_singlefloat(screen_region, placements, begin, end);
}

void
LayoutHandler::arrange_center(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    const Layout::LayoutData_ptr data = *mp_layout->data.active_element();

    std::size_t h_comp = Layout::LayoutData::MAX_MAIN_COUNT;
    float w_ratio = data->main_factor / 0.95;
    float h_ratio = static_cast<float>((h_comp - data->main_count))
        / static_cast<float>(h_comp);

    std::transform(
        begin,
        end,
        std::back_inserter(placements),
        [=,this](Client_ptr client) -> Placement {
            Region region = screen_region;

            int w = region.dim.w * w_ratio;
            int h = region.dim.h * h_ratio;

            if (w <= region.dim.w)
                region.pos.x += (region.dim.w - w) / 2.f;

            if (h <= region.dim.h)
                region.pos.y += (region.dim.h - h) / 2.f;

            region.dim = { w, h };

            return Placement {
                mp_layout->config.method,
                client,
                mp_layout->config.decoration,
                region
            };
        }
    );
}

void
LayoutHandler::arrange_monocle(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    std::transform(
        begin,
        end,
        std::back_inserter(placements),
        [=,this](Client_ptr client) {
            return Placement {
                mp_layout->config.method,
                client,
                mp_layout->config.decoration,
                screen_region
            };
        }
    );
}

void
LayoutHandler::arrange_paper(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    static const float MIN_W_RATIO = 0.5;

    const Layout::LayoutData_ptr data = *mp_layout->data.active_element();
    std::size_t n = end - begin;

    if (n == 1) {
        placements.emplace_back(Placement {
            mp_layout->config.method,
            *begin,
            Decoration::NO_DECORATION,
            screen_region
        });

        return;
    }

    int cw;
    if (data->main_factor > MIN_W_RATIO) {
        cw = screen_region.dim.w * data->main_factor;
    } else {
        cw = screen_region.dim.w * MIN_W_RATIO;
    }

    int w = static_cast<float>(screen_region.dim.w - cw)
        / static_cast<float>(n - 1);

    bool contains_active = false;
    const auto last_active = std::max_element(
        begin,
        end,
        [&contains_active](const Client_ptr lhs, const Client_ptr rhs) {
            if (lhs->focused) {
                contains_active = true;
                return false;
            } else if (rhs->focused) {
                contains_active = true;
                return true;
            }

            return lhs->last_focused < rhs->last_focused;
        }
    );

    bool after_active = false;
    std::size_t i = 0;

    std::transform(
        begin,
        end,
        std::back_inserter(placements),
        [=,this,&after_active,&i](Client_ptr client) -> Placement {
            int x = screen_region.pos.x + static_cast<int>(i++ * w);

            if ((!contains_active && *last_active == client) || client->focused) {
                after_active = true;

                return Placement {
                    mp_layout->config.method,
                    client,
                    mp_layout->config.decoration,
                    Region {
                        Pos {
                            x,
                            screen_region.pos.y
                        },
                        Dim {
                            cw,
                            screen_region.dim.h
                        }
                    }
                };
            } else {
                if (after_active)
                    x += cw - w;

                return Placement {
                    mp_layout->config.method,
                    client,
                    mp_layout->config.decoration,
                    Region {
                        Pos {
                            x,
                            screen_region.pos.y
                        },
                        Dim {
                            w,
                            screen_region.dim.h
                        }
                    }
                };
            }
        }
    );
}

void
LayoutHandler::arrange_spaper(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    arrange_paper(screen_region, placements, begin, end);
}

void
LayoutHandler::arrange_stack(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    const Layout::LayoutData_ptr data = *mp_layout->data.active_element();
    std::size_t n = end - begin;

    if (n == 1) {
        placements.emplace_back(Placement {
            mp_layout->config.method,
            *begin,
            Decoration::NO_DECORATION,
            screen_region
        });

        return;
    }

    std::size_t n_main;
    std::size_t n_stack;

    if (n <= data->main_count) {
        n_main = n;
        n_stack = 0;
    } else {
        n_main = data->main_count;
        n_stack = n - n_main;
    }

    int w_main
        = data->main_count > 0
        ? static_cast<float>(screen_region.dim.w) * data->main_factor
        : 0;

    int x_stack = screen_region.pos.x + w_main;
    int w_stack = screen_region.dim.w - w_main;
    int h_main = n_main > 0 ? screen_region.dim.h / n_main : 0;
    int h_stack = n_stack > 0 ? screen_region.dim.h / n_stack : 0;

    std::size_t i = 0;

    std::transform(
        begin,
        end,
        std::back_inserter(placements),
        [=,this,&i](Client_ptr client) -> Placement {
            if (i < data->main_count) {
                return Placement {
                    mp_layout->config.method,
                    client,
                    mp_layout->config.decoration,
                    Region {
                        Pos {
                            screen_region.pos.x,
                            screen_region.pos.y
                                + static_cast<int>(i++) * h_main
                        },
                        Dim {
                            n_stack == 0 ? screen_region.dim.w : w_main,
                            h_main
                        }
                    }
                };
            } else {
                return Placement {
                    mp_layout->config.method,
                    client,
                    mp_layout->config.decoration,
                    Region {
                        Pos {
                            x_stack,
                            screen_region.pos.y
                                + static_cast<int>((i++ - data->main_count) * h_stack)
                        },
                        Dim {
                            w_stack,
                            h_stack
                        }
                    }
                };
            }
        }
    );
}

void
LayoutHandler::arrange_sstack(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    arrange_stack(screen_region, placements, begin, end);
}

void
LayoutHandler::arrange_bstack(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    arrange_stack(screen_region, placements, begin, end);
}

void
LayoutHandler::arrange_sbstack(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    arrange_bstack(screen_region, placements, begin, end);
}

void
LayoutHandler::arrange_horz(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    std::size_t n = end - begin;

    if (n == 1) {
        placements.emplace_back(Placement {
            mp_layout->config.method,
            *begin,
            Decoration::NO_DECORATION,
            screen_region
        });

        return;
    }

    int w = std::lround(static_cast<float>(screen_region.dim.w) / n);
    std::size_t i = 0;

    std::transform(
        begin,
        end,
        std::back_inserter(placements),
        [=,this,&i](Client_ptr client) -> Placement {
            return Placement {
                mp_layout->config.method,
                client,
                mp_layout->config.decoration,
                Region {
                    Pos {
                        screen_region.pos.x + static_cast<int>(i++ * w),
                        screen_region.pos.y
                    },
                    Dim {
                        w,
                        screen_region.dim.h
                    }
                }
            };
        }
    );
}

void
LayoutHandler::arrange_shorz(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    arrange_horz(screen_region, placements, begin, end);
}

void
LayoutHandler::arrange_vert(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    std::size_t n = end - begin;

    if (n == 1) {
        placements.emplace_back(Placement {
            mp_layout->config.method,
            *begin,
            Decoration::NO_DECORATION,
            screen_region
        });

        return;
    }

    int h = std::lround(static_cast<float>(screen_region.dim.h) / n);
    std::size_t i = 0;

    std::transform(
        begin,
        end,
        std::back_inserter(placements),
        [=,this,&i](Client_ptr client) -> Placement {
            return Placement {
                mp_layout->config.method,
                client,
                mp_layout->config.decoration,
                Region {
                    Pos {
                        screen_region.pos.x,
                        screen_region.pos.y + static_cast<int>(i++ * h)
                    },
                    Dim {
                        screen_region.dim.w,
                        h
                    }
                }
            };
        }
    );
}

void
LayoutHandler::arrange_svert(
    Region screen_region,
    placement_vector placements,
    client_iter begin,
    client_iter end
) const
{
    arrange_vert(screen_region, placements, begin, end);
}


LayoutHandler::Layout::LayoutConfig
LayoutHandler::Layout::kind_to_config(LayoutKind kind)
{
    switch (kind) {
    case LayoutKind::Float:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Free,
            Decoration::FREE_DECORATION,
            false,
            false,
            false,
            false,
            true
        };
    }
    case LayoutKind::BLFloat:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Free,
            Decoration::NO_DECORATION,
            false,
            false,
            false,
            false,
            true
        };
    }
    case LayoutKind::SingleFloat:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Free,
            Decoration::FREE_DECORATION,
            false,
            false,
            true,
            true,
            true
        };
    }
    case LayoutKind::BLSingleFloat:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Free,
            Decoration::NO_DECORATION,
            false,
            false,
            true,
            true,
            true
        };
    }
    case LayoutKind::Center:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration::NO_DECORATION,
            true,
            true,
            false,
            false,
            true
        };
    }
    case LayoutKind::Monocle:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration::NO_DECORATION,
            true,
            true,
            false,
            false,
            true
        };
    }
    case LayoutKind::Paper:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 1, 1, 0, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            true,
            true,
            false,
            false
        };
    }
    case LayoutKind::SPaper:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 1, 1, 0, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            false,
            true,
            false,
            false
        };
    }
    case LayoutKind::Stack:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 0, 0, 3, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            true,
            false,
            false,
            true
        };
    }
    case LayoutKind::SStack:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 0, 0, 3, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            false,
            false,
            false,
            true
        };
    }
    case LayoutKind::BStack:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 0, 0, 3, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            true,
            false,
            false,
            true
        };
    }
    case LayoutKind::SBStack:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 0, 0, 3, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            false,
            false,
            false,
            true
        };
    }
    case LayoutKind::Horz:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 0, 0, 3, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            true,
            false,
            false,
            true
        };
    }
    case LayoutKind::SHorz:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 0, 0, 3, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            false,
            false,
            false,
            true
        };
    }
    case LayoutKind::Vert:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 3, 0, 0, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            true,
            false,
            false,
            true
        };
    }
    case LayoutKind::SVert:
    {
        return LayoutConfig {
            Placement::PlacementMethod::Tile,
            Decoration {
                std::nullopt,
                Frame {
                    Extents { 3, 0, 0, 0 },
                    ColorScheme::DEFAULT_COLOR_SCHEME
                }
            },
            true,
            false,
            false,
            false,
            true
        };
    }
    default: Util::die("no associated configuration defined");
    }

    return kind_to_config(LayoutKind::Float);
}

LayoutHandler::Layout::LayoutData
LayoutHandler::Layout::kind_to_default_data(LayoutKind kind)
{
    switch (kind) {
    case LayoutKind::Float:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::BLFloat:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::SingleFloat:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::BLSingleFloat:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::Center:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            5,
            .40f
        };
    }
    case LayoutKind::Monocle:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::Paper:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::SPaper:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::Stack:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::SStack:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::BStack:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::SBStack:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::Horz:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::SHorz:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::Vert:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    case LayoutKind::SVert:
    {
        return Layout::LayoutData {
            Extents { 0, 0, 0, 0 },
            0,
            1,
            .50f
        };
    }
    default: Util::die("no associated default data defined");
    }

    return kind_to_default_data(LayoutKind::Float);
}
