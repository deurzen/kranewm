#ifndef __KRANEWM_X_DATA_GRAPHICS_GUARD__
#define __KRANEWM_X_DATA_GRAPHICS_GUARD__

#include "window.hh"

#include <string>


namespace x_data
{
    class graphicscontext_t
    {
    public:
        graphicscontext_t(window_t _win, const ::std::string& fontname, int line_width)
          : win(_win),
            font_dim{12, 6},
            pos{},
            line_width(line_width)
        {
            XGCValues values;
            values.function           = GXcopy;
            values.plane_mask         = AllPlanes;
            values.foreground         = 0xFFFFFF;
            values.background         = 0x000000;
            values.line_width         = line_width;
            values.line_style         = LineSolid;
            values.cap_style          = CapButt;
            values.join_style         = JoinMiter;
            values.fill_style         = FillOpaqueStippled;
            values.fill_rule          = WindingRule;
            values.graphics_exposures = False;
            values.clip_x_origin      = 0;
            values.clip_y_origin      = 0;
            values.clip_mask          = None;
            values.subwindow_mode     = IncludeInferiors;

            gc = XCreateGC(g_dpy, win,
                  GCFunction    | GCPlaneMask         | GCForeground
                | GCBackground  | GCLineWidth         | GCLineStyle
                | GCCapStyle    | GCJoinStyle         | GCFillStyle
                | GCFillRule    | GCGraphicsExposures | GCClipXOrigin
                | GCClipYOrigin | GCClipMask          | GCSubwindowMode,
                &values);

            XFontStruct* font;
            if (!(font = XLoadQueryFont(g_dpy, fontname.c_str())))
                font = XLoadQueryFont(g_dpy, "fixed");

            font_dim.w = font->per_char->width;
            font_dim.h = font->per_char->ascent + font->per_char->descent;

            XSetFont(g_dpy, gc, font->fid);
        }

        ~graphicscontext_t()
        {
            XFree(gc);
        }

        inline dim_t
        get_font_dim() const
        {
            return font_dim;
        }

        void set_foreground(unsigned long);
        void set_background(unsigned long);

        void clear();
        void clear(pos_t);
        void clear_window();
        void draw_string(pos_t, const ::std::string&);

    private:
        window_t win;
        GC gc;
        dim_t font_dim;
        pos_t pos;
        int line_width;

    };
}

#endif//__KRANEWM_X_DATA_GRAPHICS_GUARD__
