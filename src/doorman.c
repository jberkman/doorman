/* 
 * Copyright 2001 Ximian, Inc.
 *
 * Author:  jacob berkman  <jacob@ximian.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "doorman.h"

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gnome-canvas-pixbuf.h>

DruidData druid_data;

static void
init_canvi (void)
{
	GdkPixbuf *pb;
	GnomeCanvasItem *root;
	GnomeCanvas *canvas;

	canvas = GNOME_CANVAS (GET_WIDGET ("title-canvas"));
	root = GNOME_CANVAS_ITEM (gnome_canvas_root (GNOME_CANVAS (canvas)));

	pb = gdk_pixbuf_new_from_file (DATADIR "/top.png");
	druid_data.title_box = 
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       GNOME_TYPE_CANVAS_PIXBUF,
				       "pixbuf", pb, "x", 0.0, "y", 0.0, NULL);

	druid_data.banner =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       gnome_canvas_text_get_type (),
				       "fill_color", "white",
				       "font", "-adobe-helvetica-bold-r-normal-*-18-*-*-*-p-*-iso8859-1",
				       "fontset", "-adobe-helvetica-bold-r-normal-*-18-*-*-*-p-*-iso8859-1,*-r-*",
				       "anchor", GTK_ANCHOR_WEST,
				       "x", 113.0,
				       "y", 24.0,
				       NULL);	


	pb = gdk_pixbuf_new_from_file (DATADIR "/puzzle.png");
	druid_data.logo =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       GNOME_TYPE_CANVAS_PIXBUF,
				       "pixbuf", pb, NULL);


	/*******************************************************/
	canvas = GNOME_CANVAS (GET_WIDGET ("side-canvas"));
	root = GNOME_CANVAS_ITEM (gnome_canvas_root (GNOME_CANVAS (canvas)));
#if 1
	druid_data.side_box =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       gnome_canvas_rect_get_type (),
				       "x1", 0.0,
				       "y1", 0.0,
				       "fill_color", "#314a82",
				       "outline_color", "#314a82", NULL);
#endif
	pb = gdk_pixbuf_new_from_file (DATADIR "/left.png");

	druid_data.side_image =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       GNOME_TYPE_CANVAS_PIXBUF,
				       "pixbuf", pb, 
				       "x", 0.0, "y", 0.0, NULL);
}

static void
init_ui (void)
{
	GtkWidget *w;
	glade_xml_signal_autoconnect (druid_data.xml);

	init_canvi ();

	w = W ("druid-notebook");
	gtk_notebook_set_show_border (GTK_NOTEBOOK (w), FALSE);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (w), FALSE);

	setup_theme_page (&panel_theme_page);
	setup_theme_page (&gtk_theme_page);
	setup_theme_page (&background_theme_page);
	setup_theme_page (&sawfish_theme_page);
	setup_theme_page (&desktop_theme_page);
}


void
title_configure_size (GtkWidget *w, GtkAllocation *alloc, gpointer data)
{
	gnome_canvas_set_scroll_region (GNOME_CANVAS (w), 0.0, 0.0,
					alloc->width, alloc->height);
#if 0
	gnome_canvas_item_set (druid_data.title_box,
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", (gfloat)alloc->width,
			       "y2", (gfloat)alloc->height,
			       "width_units", 1.0, NULL);

	gnome_canvas_item_set (druid_data.banner,
			       "anchor", GTK_ANCHOR_WEST, NULL);

#endif
	gnome_canvas_item_set (druid_data.logo,
			       "x", (gfloat)(alloc->width - 49),
			       "y", 1.0,
			       NULL);
}

void
side_configure_size (GtkWidget *w, GtkAllocation *alloc, gpointer data)
{
	gnome_canvas_set_scroll_region (GNOME_CANVAS (w), 0.0, 0.0,
					alloc->width, alloc->height);

	gnome_canvas_item_set (druid_data.side_box,
			       "x2", (gfloat)alloc->width,
			       "y2", (gfloat)alloc->height,
			       "width_units", 1.0, NULL);
#if 0
	gnome_canvas_item_set (druid_data.side_image,
			       "x", 0.0,
			       "y", (gfloat)(alloc->height - 179),
			       "width", 68.0,
			       "height", 179.0, NULL);
#endif
}


int
main (int argc, char *argv[])
{
	GtkWidget *w;
	char *s;
	
	memset (&druid_data, 0, sizeof (druid_data));
	druid_data.state = -1;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init (PACKAGE, VERSION, argc, argv);		
	gnome_window_icon_set_default_from_file (ICONDIR"/doorman.png");

	glade_gnome_init ();

	s = "doorman.glade";
	if (!g_file_exists (s))
		s = DATADIR "/doorman.glade";

	druid_data.xml = glade_xml_new (s, NULL);

	if (!druid_data.xml) {
		/* we should complain, but just bail */
		return 1;
	}

	init_ui ();

	gtk_widget_show (GET_WIDGET ("druid-window"));

	druid_set_state (DS_WELCOME);

	gtk_main ();

	return 0;
}
