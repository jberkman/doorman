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

static gboolean live = TRUE;

static void
add_theme (GtkCList *clist, ThemePage *page, ThemeData *theme_data)
{
	char *text[2] = { NULL };
	int row;
	text[0] = theme_data->label;
	row = gtk_clist_append (clist, text);
	gtk_clist_set_row_data (clist, row, theme_data);
}

static void
row_select (GtkCList *clist, gint row, gint col, GdkEvent *event, gpointer data)
{
	ThemeData *theme_data;
	ThemePage *page = data;
	char *s;

	g_print ("select!!\n");

	if (live)
		druid_set_sensitive (TRUE, TRUE, TRUE);
	page->selected_row = row;

	theme_data = gtk_clist_get_row_data (clist, row);
	
	if (theme_data->desc)
		gtk_label_set_text (GTK_LABEL (W (page->long_desc)), theme_data->desc);

	s = g_strdup_printf (DATADIR"/shots/%s", theme_data->screenshot);
	gnome_pixmap_load_file (GNOME_PIXMAP (W (page->pixmap)), s);
	g_free (s);
}

static void
reset_page (ThemePage *page)
{
	gtk_label_set_text (GTK_LABEL (W (page->long_label)), page->long_desc);

	gnome_pixmap_load_file (GNOME_PIXMAP (W (page->pixmap)), DATADIR"/nat.png");
}	

static void
row_unselect (GtkCList *clist, gint row, gint col, GdkEvent *event, gpointer data)
{
	ThemePage *page = data;
	g_print ("unselect!!\n");
	druid_set_sensitive (TRUE, FALSE, TRUE);
	reset_page (page);
	page->selected_row = -1;
}

static void
existing_toggled (GtkToggleButton *toggle, gpointer data)
{
	ThemePage *page = data;

	gtk_widget_set_sensitive (W (page->clist), !toggle->active);
	gtk_widget_set_sensitive (W (page->pixmap), !toggle->active);
	druid_set_sensitive (TRUE, toggle->active || (page->selected_row > -1), TRUE);
}

void
setup_theme_page (ThemePage *page)
{
	ThemeData *theme_data;
	GtkWidget *w;
	GtkCList *clist;
	int i;

	gtk_label_set_text (GTK_LABEL (W (page->label)), page->description);
	reset_page (page);

	clist = GTK_CLIST (W (page->clist));
	
	gtk_signal_connect (GTK_OBJECT (clist), "select_row", 
			    GTK_SIGNAL_FUNC (row_select), page);

	gtk_signal_connect (GTK_OBJECT (clist), "unselect_row", 
			    GTK_SIGNAL_FUNC (row_unselect), page);

	gtk_signal_connect (GTK_OBJECT (W (page->toggle)), "toggled",
			    GTK_SIGNAL_FUNC (existing_toggled), page);

	gtk_clist_freeze (clist);

	for (i=0; i<6 && page->theme_data[i].label; i++)
		add_theme (clist, page, &page->theme_data[i]);


	live = FALSE;
	gtk_clist_select_row (clist, 0, 0);
	live = TRUE;

	gtk_clist_thaw (clist);
}

void
try_and_apply (ThemePage *page)
{
	if (GTK_TOGGLE_BUTTON (W (page->toggle))->active)
		return;

	g_assert (page->selected_row > -1);
	page->apply_func (page->theme_data[page->selected_row].location);
}
