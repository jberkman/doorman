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

#define MARK_STRING "# -- THEME AUTO-WRITTEN DO NOT EDIT\n"
gchar gtkrc_tmp[1024];

static void
print_standard_stuff(FILE *fout, gchar *theme, gchar *font)
{
	gchar *homedir;

	homedir = g_strconcat ("include \"",
			       gnome_util_user_home(),
			       "/.gtkrc.mine\"\n\n", NULL);
	fprintf(fout, MARK_STRING);
	fprintf(fout, "include \"%s\"\n\n", theme);
	if (font)
		fprintf(fout, "style \"user-font\"\n{\n  font=\"%s\"\n}\nwidget_class \"*\" style \"user-font\"\n\n", font);
	fprintf(fout, homedir);
	g_free (homedir);
	fprintf(fout, MARK_STRING);
}

static void
edit_file_to_use(gchar *file, gchar *theme, gchar *font)
{
	FILE *fin, *fout;
	gchar tmp[4096], buf[4096];
	gchar nextline = 0, hastheme = 0;
  
	srand(time(NULL));
	g_snprintf(tmp, sizeof(tmp), "/tmp/gtkrc_%i", rand());
	fout = fopen(tmp, "w");
	if (!fout)
		return;
	fin = fopen(file, "r");
	if (!fin)
	{
		print_standard_stuff (fout, theme, font);
		fclose(fout);
		cp(tmp, file);
		return;
	}
	while (fgets(buf, sizeof(buf), fin))
	{ 
		if (!strcmp(MARK_STRING, buf))
			hastheme += 1;
	}
	rewind(fin);
	if (!hastheme)
	{
		print_standard_stuff (fout, theme, font);
		while (fgets(buf, sizeof(buf), fin))
			fprintf(fout, "%s", buf);
	}
	else if (hastheme == 1)
		/* we keep this in for backwards compatability. */
	{
		nextline = 0;
		while (fgets(buf, sizeof(buf), fin))
		{
			if (nextline == 1)
				nextline = 0;
			else if (!strcmp(MARK_STRING, buf))
			{
				print_standard_stuff (fout, theme, font);
				nextline = 1;
			}
			else if (nextline == 0)
				fprintf(fout, "%s", buf);
		}
	}
	else
	{
		nextline = 0;
		while (fgets(buf, sizeof(buf), fin))
		{
			if (!strcmp(MARK_STRING, buf))
			{
				if (nextline == 0)
				{
					nextline = 1;
					print_standard_stuff (fout, theme, font);
				}
				else
				{
					nextline = 0;
				}
			} else if (nextline == 0)
				fprintf(fout, "%s", buf);
		}
	}
	fclose(fin);
	fclose(fout);
	cp(tmp, file);
	rm(tmp);
}


static void
apply_gtk_theme (const char *location)
{
	gchar *gtkrc, *theme;

	gtkrc = g_strdup_printf ("%s/.gtkrc", gnome_util_user_home ());
	theme = g_strdup_printf ("%s/%s/gtk/gtkrc", GTK_THEMEDIR, location);

	edit_file_to_use(gtkrc, theme, NULL);

	g_free (theme);
	g_free (gtkrc);
}

ThemePage gtk_theme_page = {
	N_("Choose a gtk theme."),
	N_("The gtk theme makes your apps look dope"),
	{ 
		{ "Preview 1",   "HeliX",     "gtk/helix.png" },
		{ "Thin Ice",    "ThinIce",   "gtk/thinice.png" },
		{ "Raleigh",     "Raleigh",   "gtk/raleigh.png" },
		{ "Eazel",       "Eazel",     "gtk/eazel.png" }
	},
	apply_gtk_theme,
	"gtk label",
	"gtk desc",
	"gtk clist",
	"gtk pixmap",
	"gtk toggle"
};
