/* bug-buddy bug submitting program
 *
 * Copyright (C) 2000 Jacob Berkman
 * Copyright 2001 Ximian, Inc.
 *
 * Author:  Jacob Berkman  <jacob@bug-buddy.org>
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
	DS_PANEL_ICONS,
	DS_PANEL,
	DS_DESKTOP,
	DS_ICONS,
	DS_SAWFISH,
	DS_GTK,
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

typedef void (*ThemeApplyFunc) (const char *location);k

typedef struct {
	const char *label;
	const char *location;
	const char *screenshot;
	ThemeApplyFunc apply_func;
} ThemeData;

typedef struct {
	const char *description;
	ThemeData *theme_data;
	const char *long_desc;
} ThemePage;


enum {
	TP_PANEL,
	TP_DESKTOP,
	TP_SAWFISH,
	TP_GTK,
	TP_BACKGROUND,
	TP_LAST
};
extern ThemePage theme_page[TP_LAST];


#define GET_WIDGET(name) (glade_xml_get_widget (druid_data.xml, (name)))
#define W(name)          (glade_xml_get_widget (druid_data.xml, (name)))

#define d(x) x



void druid_set_sensitive (gboolean prev, gboolean next, gboolean cancel);
void druid_set_state     (DruidState state);

#endif /* DOORMAN_H */
