#ifndef __LAYOUT_H_GUARD__
#define __LAYOUT_H_GUARD__

#include "cycle.hh"
#include "placement.hh"
#include "../winsys/decoration.hh"
#include "../winsys/util.hh"

#include <vector>
#include <deque>
#include <unordered_map>

class LayoutHandler final
{
    typedef std::deque<Client_ptr>::const_iterator client_iter;
    typedef std::vector<Placement>& placement_vector;

public:
    enum class LayoutKind
    {
        /// free layouts
        Float,
        BLFloat,
        SingleFloat,
        BLSingleFloat,

        // overlapping tiled layouts
        Center,
        Monocle,

        // non-overlapping tiled layouts
        Paper,
        SPaper,
        Stack,
        SStack,
        BStack,
        SBStack,
        Horz,
        SHorz,
        Vert,
        SVert,
    };

private:
    typedef struct Layout final
    {
        typedef struct LayoutData final
        {
            constexpr static std::size_t MAX_MAIN_COUNT = 16;
            constexpr static std::size_t MAX_GAP_SIZE = 300;
            constexpr static winsys::Extents MAX_MARGIN
                = winsys::Extents { 700, 700, 400, 400 };

            LayoutData(
                winsys::Extents margin,
                std::size_t gap_size,
                std::size_t main_count,
                float main_factor
            )
                : margin(margin),
                  gap_size(gap_size),
                  main_count(main_count),
                  main_factor(main_factor)
            {}

            // generic layout data
            winsys::Extents margin;
            std::size_t gap_size;

            // tiled layout data
            std::size_t main_count;
            float main_factor;
        }* LayoutData_ptr;

    private:
        struct LayoutConfig final
        {
            Placement::PlacementMethod method;
            winsys::Decoration decoration;
            bool margin;
            bool gap;
            bool persistent;
            bool single;
            bool wraps;
        };

    public:
        Layout(LayoutKind);
        ~Layout();

        inline bool
        operator==(const Layout& other) const
        {
            return other.kind == kind;
        }

        const LayoutKind kind;
        const LayoutConfig config;

        const LayoutData default_data;
        Cycle<LayoutData_ptr> data;

        static LayoutConfig kind_to_config(LayoutKind kind);
        static LayoutData kind_to_default_data(LayoutKind kind);

    }* Layout_ptr;

public:
    LayoutHandler();
    ~LayoutHandler();

    void arrange(winsys::Region, placement_vector, client_iter, client_iter) const;

    LayoutKind kind() const;
    void set_kind(LayoutKind);
    void set_prev_kind();

    bool layout_is_free() const;
    bool layout_has_margin() const;
    bool layout_has_gap() const;
    bool layout_is_persistent() const;
    bool layout_is_single() const;
    bool layout_wraps() const;

    std::size_t gap_size() const;
    std::size_t main_count() const;
    float main_factor() const;
    winsys::Extents margin() const;

    void copy_data_from_prev_layout();
    void set_prev_layout_data();

    void change_gap_size(Util::Change<int>);
    void change_main_count(Util::Change<int>);
    void change_main_factor(Util::Change<float>);
    void change_margin(Util::Change<int>);
    void change_margin(winsys::Edge, Util::Change<int>);
    void reset_gap_size();
    void reset_margin();
    void reset_layout_data();
    void cycle_layout_data(winsys::Direction);

    void save_layout(std::size_t) const;
    void load_layout(std::size_t);

private:
    LayoutKind m_kind;
    LayoutKind m_prev_kind;

    std::unordered_map<LayoutKind, Layout_ptr> m_layouts;

    Layout_ptr mp_layout;
    Layout_ptr mp_prev_layout;

    void arrange_float(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_blfloat(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_singlefloat(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_blsinglefloat(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_center(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_monocle(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_paper(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_spaper(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_stack(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_sstack(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_bstack(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_sbstack(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_horz(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_shorz(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_vert(winsys::Region, placement_vector, client_iter, client_iter) const;
    void arrange_svert(winsys::Region, placement_vector, client_iter, client_iter) const;

};

#endif//__LAYOUT_H_GUARD__
