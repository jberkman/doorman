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


#define G_LOG_DOMAIN "background"

#include "config.h"
#include "doorman.h"

#include <gnome.h>

static void
apply_background_theme (const char *location)
{
	char *s;
	g_message ("location: %s", location);

	s = g_strdup_printf (ICONDIR"/Wallpapers/%s-1024x768.png", location);

	gnome_config_push_prefix ("/Background/Default/");
	
	gnome_config_set_bool ("Enabled", TRUE);
	gnome_config_set_string ("wallpaper", s);
	gnome_config_set_int ("wallpaperAlign", 2);

	gnome_config_pop_prefix ();
	gnome_config_sync ();

	g_free (s);
}

ThemePage background_theme_page = {
	N_("Choose a background configuration for your desktop"),
	N_("The background does some stuff."),
	{ { "Ximian GNOME 1.4 default",
	    "ximian-default",
	    "background/ximian-default.png" }
	},	
	apply_background_theme,
	"background label",
	"background desc",
	"background clist",
	"background pixmap",
	"background toggle"
};
