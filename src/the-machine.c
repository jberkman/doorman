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
			druid_data.config = CONFIG_CURRENT;
		} else if (GTK_TOGGLE_BUTTON (W ("custom config"))->active) {
			newstate = DS_PANEL;
			druid_data.config = CONFIG_CUSTOM;
		} else {
			druid_data.config = CONFIG_DEFAULT;
		}
		break;
			
	case DS_BACKGROUND:
		try_and_apply (&panel_theme_page);
		try_and_apply (&gtk_theme_page);
		try_and_apply (&background_theme_page);
		try_and_apply (&sawfish_theme_page);
		try_and_apply (&desktop_theme_page);
		break;
	case DS_PANEL_ICONS:
		newstate = DS_FINISHED;
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
