/* 
 * Copyright (C) 2000 Jacob Berkman
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

static char *state_title[] = {
	N_("Welcome to GNOME 1.4"),
	N_("Configuration"),
	N_("Panels and Icons"),
	N_("Panel Configuration"),
	N_("Desktop Manager"),
	N_("Desktop Icons"),
	N_("Sawfish Theme"),
	N_("GTK+ Theme"),
	N_("Background Image"),
	N_("Finished!"),
	NULL
};

void
druid_set_sensitive (gboolean prev, gboolean next, gboolean cancel)
{
	gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), prev);
	gtk_widget_set_sensitive (GET_WIDGET ("druid-next"), next);
	gtk_widget_set_sensitive (GET_WIDGET ("druid-cancel"), cancel);
}

static char *finish_msg;

void
druid_set_state (DruidState state)
{
	static gboolean been_here = FALSE;
	DruidState oldstate;
	GtkWidget *w;
	char *s;
	int pos = 0;

	g_return_if_fail (state >= 0);
	g_return_if_fail (state < DS_LAST);

	if (druid_data.state == state && been_here)
		return;

	been_here = TRUE;

	oldstate = druid_data.state;
	druid_data.state = state;

	gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"),
				  (state > 0));

	gtk_widget_set_sensitive (GET_WIDGET ("druid-next"),
				  (state < DS_FINISHED));

	gnome_canvas_item_set (druid_data.banner,
			       "text", _(state_title[state]),
			       NULL);

	gtk_notebook_set_page (GTK_NOTEBOOK (GET_WIDGET ("druid-notebook")),
			       state);	

	switch (druid_data.state) {
	case DS_FINISHED:
		gtk_widget_hide (GET_WIDGET ("druid-prev"));
		gtk_widget_hide (GET_WIDGET ("druid-next"));
		gtk_widget_show (GET_WIDGET ("druid-finish"));
		druid_set_sensitive (FALSE, FALSE, FALSE);

		gtk_label_set_text (GTK_LABEL (W ("done label")), finish_msg);

		break;
	default:
		break;
	}
}

void
on_druid_prev_clicked (GtkWidget *w, gpointer data)
{
	DruidState newstate;

	newstate = druid_data.state - 1;

	switch (druid_data.state) {
	case DS_PANEL:
		newstate = DS_CONFIGURATION;
		break;
	default:
		break;
	}
	druid_set_state (newstate);
}

static gboolean 
data_loss (void)
{
	GnomeDialog *d;
	d = gnome_question_dialog_modal (_("Do you want to overwrite your old configuration now?"), NULL, W ("druid-window"));
	return (GNOME_YES == gnome_dialog_run_and_close (d));

}

void
on_druid_next_clicked (GtkWidget *w, gpointer data)
{
	DruidState newstate;
	GtkToggleButton *b;
	char *s;

	newstate = druid_data.state + 1;

	switch (druid_data.state) {
	case DS_CONFIGURATION:
		if (GTK_TOGGLE_BUTTON (W ("current config"))->active) {
			newstate = DS_FINISHED;
			finish_msg = _("Your configuration has not been changed.\n\n"
				       "Click close to finish logging in to GNOME 1.4.");
		
		} else if (GTK_TOGGLE_BUTTON (W ("custom config"))->active)
			newstate = DS_PANEL;
		break;
			
	case DS_BACKGROUND:
		if (!data_loss ())
			return;
		
		try_and_apply (&panel_theme_page);
		try_and_apply (&gtk_theme_page);
		try_and_apply (&background_theme_page);
		try_and_apply (&sawfish_theme_page);
		try_and_apply (&desktop_theme_page);

		finish_msg = _("Your configuration has been changed according\n"
			       "to your selections.\n\n"
			       "Click close to finish logging in to GNOME 1.4.");

		break;
	case DS_PANEL_ICONS:
		if (!data_loss ())
			return;

		newstate = DS_FINISHED;
		panel_theme_page.apply_func ("ximian-default");
		gtk_theme_page.apply_func ("ximian-default");
		background_theme_page.apply_func ("ximian-default");
		sawfish_theme_page.apply_func ("ximian-default");
		desktop_theme_page.apply_func ("Nautilus");

		finish_msg = _("Your configuration has been upgraded to the new defaults.\n\n"
			       "Click close to finish logging in to GNOME 1.4.");
		break;
	default:
		break;
	}

	druid_set_state (newstate);
}

void
on_druid_cancel_clicked (GtkWidget *w, gpointer data)
{
	GtkWidget *d;

	d = gnome_question_dialog (
		_("Are you sure you want to cancel\n"
		  "setting up your desktop?"), NULL, NULL);
	if (gnome_dialog_run_and_close (GNOME_DIALOG (d)))
		return;

	gtk_main_quit ();
}
