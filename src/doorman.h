/* 
 * Copyright 2001 Ximian, Inc.
 *
 * Author:  Jacob Berkman  <jacob@ximian.com>
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


#ifndef DOORMAN_H
#define DOORMAN_H

#include <glade/glade.h>
#include <libgnomeui/gnome-canvas.h>

typedef enum {
	DS_WELCOME,
	DS_CONFIGURATION,
	DS_RESET_PANEL,
	DS_PANEL,
	DS_DESKTOP,
	DS_SAWFISH,
	DS_GTK,
	DS_BACKGROUND,
	DS_FINISHED,
	DS_LAST
} DruidState;

typedef enum {
	CONFIG_DEFAULT,
	CONFIG_CUSTOM,
	CONFIG_CURRENT
} ConfigType;

typedef struct {
	GladeXML *xml;
	DruidState state;
	ConfigType config;

	GnomeCanvasItem *title_box;
	GnomeCanvasItem *banner;
	GnomeCanvasItem *logo;
	GnomeCanvasItem *top_image;

	GnomeCanvasItem *side_box;
	GnomeCanvasItem *side_image;
} DruidData;

extern DruidData druid_data;

typedef void (*ThemeApplyFunc) (const char *location);

typedef struct {
	const char *label;
	const char *location;
	const char *screenshot;
	const char *desc;
} ThemeData;

typedef struct {
	const char *description;
	const char *long_desc;
	ThemeData theme_data[6];
	ThemeApplyFunc apply_func;
	const char *label;
	const char *long_label;
	const char *clist;
	const char *pixmap;
	const char *toggle;
	int selected_row;
} ThemePage;


extern ThemePage desktop_theme_page;
extern ThemePage panel_theme_page;
extern ThemePage background_theme_page;
extern ThemePage sawfish_theme_page;
extern ThemePage gtk_theme_page;


#define GET_WIDGET(name) (glade_xml_get_widget (druid_data.xml, (name)))
#define W(name)          (glade_xml_get_widget (druid_data.xml, (name)))

#define d(x) x

void try_and_apply (ThemePage *page);
void setup_theme_page (ThemePage *page);
void druid_set_sensitive (gboolean prev, gboolean next, gboolean cancel);
void druid_set_state     (DruidState state);

void convert_gmc_desktop_icons (void);
char *nautilus_get_user_directory (void);
void druid_set_first_time_file_flag (void);

void gdesktop_links_init (void);

#endif /* DOORMAN_H */
