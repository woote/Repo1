/* Pango
 * pangox.h:
 *
 * Copyright (C) 1999 Red Hat Software
 * Copyright (C) 2000 SuSE Linux Ltd
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __PANGOXFT_H__
#define __PANGOXFT_H__

#include <pango/pango-context.h>
#include <pango/pango-ot.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#define PANGO_RENDER_TYPE_XFT "PangoRenderXft"

/* Calls for applications
 */
PangoContext * pango_xft_get_context        (Display          *display,
					     int               screen);
void           pango_xft_render           (XftDraw          *draw,
					   XftColor         *color,
					   PangoFont        *font,
					   PangoGlyphString *glyphs,
					   gint              x,
					   gint              y);

#define PANGO_TYPE_XFT_FONT              (pango_xft_font_get_type ())
#define PANGO_XFT_IS_FONT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), PANGO_TYPE_XFT_FONT))

GType      pango_xft_font_get_type (void);

/* For shape engines
 */

XftFont *     pango_xft_font_get_font          (PangoFont *font);
FT_Face       pango_xft_font_get_face          (PangoFont *font);
PangoOTInfo * pango_xft_font_get_ot_info       (PangoFont *font);
Display *     pango_xft_font_get_display       (PangoFont *font);
PangoGlyph    pango_xft_font_get_unknown_glyph (PangoFont *font,
						gunichar   wc);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __PANGOXFT_H__ */