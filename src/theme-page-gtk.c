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
apply_gtk_theme (const char *location)
{

}

ThemePage gtk_theme_page = {
	N_("Choose a gtk theme."),
	N_("The gtk theme makes your apps look dope"),
	{ { "Ximian GNOME 1.4 default",
	    "ximian-default",
	    "gtkl/ximian-default.png" }
	},
	apply_gtk_theme,
	"gtk label",
	"gtk desc",
	"gtk clist",
	"gtk pixmap",
	"gtk toggle"
};
