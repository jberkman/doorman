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

#include "file.h"

static char *to, *from;

static void
cp_file (const char *file)
{
	char *s, *ss;

	s = g_concat_dir_and_file (from, file);
	ss = g_concat_dir_and_file (to, file);

	g_message ("cp: %s => %s\n", s, ss);
	cp (s, ss);

	g_free (ss);
	g_free (s);
}

static void
cp_applet (int num)
{
	char *applet;
	char *s, *id;

	applet = g_strdup_printf ("Applet_%d", num);
	
	s = g_strdup_printf ("Applet_Config=/%s/id=sigh", applet);
	
	id = gnome_config_get_string (s);
	g_free (s);

	if (!strcmp (id, "Empty")) {
		goto freebird;
		return;
	} else if (!strcmp (id, "Extern")) {
		char *file = g_strdup_printf ("%s_Extern", applet);
		cp_file (file);
		g_free (file);
	} else
		g_error ("unknown type: %s\n", id);
	
 freebird:
	g_free (applet);
	g_free (id);
}

static void
apply_panel_theme (const char *location)
{
	char *s, *ss;
	int applets, i;

	from = g_strdup_printf ("%s/panel/%s/", DATADIR, location);
	to = gnome_util_home_file ("panel.d/default/");

	mkdirs (to);

	s = g_strdup_printf ("=%s", from);
	gnome_config_push_prefix (s);
	g_free (s);

	cp_file ("panel");
	cp_file ("Applet_Config");

	applets = gnome_config_get_int ("panel=/Config/applet_count");
	g_message ("need to copy %d applets", applets);

	for (i=1; i<=applets; i++)
		cp_applet (i);

	gnome_config_pop_prefix ();
}

ThemePage panel_theme_page = {
	N_("Choose a panel configuration for your desktop"),
	N_("The panel does some stuff."),
	{ { "Ximian GNOME 1.4 default",
	    "ximian-default",
	    "panel/ximian-default.png" }
	},
	apply_panel_theme,
	"panel label",
	"panel desc",
	"panel clist",
	"panel pixmap",
	"panel toggle"
};
