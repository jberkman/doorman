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


#include "config.h"
#include "doorman.h"

#include <gnome.h>

static void
apply_desktop_theme (const char *location)
{
	gnome_config_set_string ("/session-options/Options/CurrentSession", location);
	gnome_config_sync ();

	if (!strcmp (location, "None"))
		return;

	/* 
	 * this adds the new icons for gmc, which then get added to
	 * nautlius if the druid hasn't been run yet.
	 *
	 * otherwise, we have to manually add some here.
	 *
	 * sigh.  this all sucks a lot and it isn't my fault.
	 *
	 */

	gdesktop_links_init ();

	if (!strcmp (location, "Nautilus")) {
		char *ndir;
		char *first_time;

		ndir = nautilus_get_user_directory ();

		first_time = g_concat_dir_and_file (ndir, "first-time-flag");

		if (!g_file_exists (first_time)) {
			convert_gmc_desktop_icons ();
			druid_set_first_time_file_flag ();
		} else {
			/* add new dinguses for nautilus here */
		}
	}
}

ThemePage desktop_theme_page = {
	N_("Choose a desktop manager for your desktop."),
	N_("The desktop manager can place icons on your desktop, and is also a file manager"),
	{ 
		{ "Nautilus", "Nautilus", "desktop/nautilus.png" },
		{ "GMC",      "GMC",      "desktop/gmc.png" },
		{ "None",     "None",     "desktop/none.png" }
	},
	apply_desktop_theme,
	"desktop label",
	"desktop desc",
	"desktop clist",
	"desktop pixmap",
	"desktop toggle"
};
