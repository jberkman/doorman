/* nautilus-haX0rz.c: various hacks from nautilus to do desktop stuff

   Copyright (C) 1999, 2000, 2001 Eazel, Inc.
   Copyright 2001 Ximian, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: John Sullivan <sullivan@eazel.com>
            jacob berkman  <jacob@ximian.com>
*/

#include "config.h"
#include "doorman.h"

#include <gnome.h>

#include <parser.h>
#include <xmlmemory.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

/* Link types */
typedef enum {
	NAUTILUS_LINK_GENERIC,
	NAUTILUS_LINK_TRASH,
	NAUTILUS_LINK_MOUNT,
	NAUTILUS_LINK_HOME
} NautilusLinkType;

/* Link type XML tags */
#define NAUTILUS_LINK_GENERIC_TAG	"Generic Link"
#define NAUTILUS_LINK_TRASH_TAG 	"Trash Link"
#define NAUTILUS_LINK_MOUNT_TAG 	"Mount Link"
#define NAUTILUS_LINK_HOME_TAG 		"Home Link"

#define NAUTILUS_USER_DIRECTORY_NAME ".nautilus"
#define DEFAULT_NAUTILUS_DIRECTORY_MODE (0755)

#define DESKTOP_DIRECTORY_NAME "desktop"
#define DEFAULT_DESKTOP_DIRECTORY_MODE (0755)

#define NAUTILUS_USER_MAIN_DIRECTORY_NAME "Nautilus"


/* Return a command string containing the path to a terminal on this system. */
static char *
nautilus_gnome_get_terminal_path (void)
{
	const char *term[] = {
		"gnome-terminal",
		"nxterm",
		"color-xterm",
		"rxvt",
		"xterm",
		"dtterm",
		NULL
	};
	static char *terminal_path = NULL;
	int i;

	if (!terminal_path) {
		for (i=0; term[i]; i++) {
			terminal_path = gnome_is_program_in_path (term[i]);
			if (terminal_path)
				break;
		}
	}

	return g_strdup (terminal_path);
}

static const char *
get_tag (NautilusLinkType type)
{
	switch (type) {
	case NAUTILUS_LINK_GENERIC:
		return NAUTILUS_LINK_GENERIC_TAG;
	case NAUTILUS_LINK_TRASH:
		return NAUTILUS_LINK_TRASH_TAG;
	case NAUTILUS_LINK_MOUNT:
		return NAUTILUS_LINK_MOUNT_TAG;
	case NAUTILUS_LINK_HOME:
		return NAUTILUS_LINK_HOME_TAG;
	}

	g_assert_not_reached ();
	return N_("FOR GREAT JUSTICE");
}

/**
 * nautilus_make_path:
 * 
 * Make a path name from a base path and name. The base path
 * can end with or without a separator character.
 *
 * Return value: the combined path name.
 **/
static char * 
nautilus_make_path (const char *path, const char* name)
{
    	gboolean insert_separator;
    	int path_length;
	char *result;
	
	path_length = strlen (path);
    	insert_separator = path_length > 0 && 
    			   name[0] != '\0' && 
    			   path[path_length - 1] != G_DIR_SEPARATOR;

    	if (insert_separator) {
    		result = g_strconcat (path, G_DIR_SEPARATOR_S, name, NULL);
    	} else {
    		result = g_strconcat (path, name, NULL);
    	}

	return result;
}

static gboolean
nautilus_link_local_create (const char *directory_path,
			    const char *name,
			    const char *image,
			    const char *target_uri,
			    const GdkPoint *point,
			    NautilusLinkType type)
{
	xmlDocPtr output_document;
	xmlNodePtr root_node;
	char *path;
	int result;
#if 0
	char *uri;
	GList dummy_list;
#endif	
	g_return_val_if_fail (directory_path != NULL, FALSE);
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (image != NULL, FALSE);
	g_return_val_if_fail (target_uri != NULL, FALSE);
	
	/* create a new xml document */
	output_document = xmlNewDoc ("1.0");
	
	/* add the root node to the output document */
	root_node = xmlNewDocNode (output_document, NULL, "nautilus_object", NULL);
	xmlDocSetRootElement (output_document, root_node);

	/* Add mime magic string so that the mime sniffer can recognize us.
	 * Note: The value of the tag identfies what type of link this.  */
	xmlSetProp (root_node, "nautilus_link", get_tag (type));
	
	/* Add link and custom icon tags */
	xmlSetProp (root_node, "custom_icon", image);
	xmlSetProp (root_node, "link", target_uri);
	
	/* all done, so save the xml document as a link file */
	path = nautilus_make_path (directory_path, name);
	result = xmlSaveFile (path, output_document);
	
	xmlFreeDoc (output_document);

	if (result <= 0) {
		g_free (path);
		return FALSE;
	}
	
#if 0
	/* Notify that this new file has been created. */
	uri = gnome_vfs_get_uri_from_local_path (path);
	dummy_list.data = uri;
	dummy_list.next = NULL;
	dummy_list.prev = NULL;
	nautilus_directory_notify_files_added (&dummy_list);
	nautilus_directory_schedule_metadata_remove (&dummy_list);

	if (point != NULL) {
		item.uri = uri;
		item.set = TRUE;
		item.point.x = point->x;
		item.point.y = point->y;
		
		dummy_list.data = &item;
		dummy_list.next = NULL;
		dummy_list.prev = NULL;
	
		nautilus_directory_schedule_position_set (&dummy_list);
	}
	g_free (uri);
#endif

	g_free (path);

	return TRUE;
}


static void
nautilus_link_local_create_from_gnome_entry (GnomeDesktopEntry *entry, const char *dest_path, const GdkPoint *position)
{
	const char *icon_name;
	char *launch_string, *terminal_path;
	char *arguments, *temp_str;
	gboolean create_link;
	int index;

	if (entry == NULL || dest_path == NULL) {
		return;
	}
	
	terminal_path = nautilus_gnome_get_terminal_path ();
	if (terminal_path == NULL) {
		return;
	}

	create_link = TRUE;
	
	/* Extract arguments from exec array */			
	for (index = 0, arguments = NULL; index < entry->exec_length; ++index) {
		if (arguments == NULL) {
			arguments = g_strdup (entry->exec[index]);
		} else {
			temp_str = arguments;
			arguments = g_strdup_printf ("%s %s", temp_str, entry->exec[index]);
			g_free (temp_str);
		}		
	}
		
	if (strcmp (entry->type, "Application") == 0) {
		if (entry->terminal) {		
			if (strstr (terminal_path, "gnome-terminal") != NULL) {
				/* gnome-terminal takes different arguments */
				launch_string = g_strdup_printf ("command:%s '-x %s'", terminal_path, arguments);			
			} else {
				launch_string = g_strdup_printf ("command:%s '-e %s'", terminal_path, arguments);
			}			
		} else {
			launch_string = g_strdup_printf ("command:%s", arguments);
		}		
	} else if (strcmp (entry->type, "URL") == 0) {
		launch_string = g_strdup_printf ("command:%s", arguments);
	} else {
		/* Unknown .desktop file type */
		launch_string = NULL;
		create_link = TRUE;		
	}
	
	if (entry->icon != NULL) {
		icon_name = entry->icon;
	} else {
		icon_name = "gnome-unknown.png";
	}
	
	if (create_link) {
		nautilus_link_local_create (dest_path, entry->name, icon_name, 
			    	    	    launch_string, position, NAUTILUS_LINK_GENERIC);
	}
				
	g_free (launch_string);
	g_free (arguments);
	g_free (terminal_path);
}

/**
 * nautilus_get_user_directory:
 * 
 * Get the path for the directory containing nautilus settings.
 *
 * Return value: the directory path.
 **/
char *
nautilus_get_user_directory (void)
{
	char *user_directory = NULL;

	user_directory = nautilus_make_path (g_get_home_dir (),
					     NAUTILUS_USER_DIRECTORY_NAME);

	if (!g_file_exists (user_directory)) {
		mkdir (user_directory, DEFAULT_NAUTILUS_DIRECTORY_MODE);
		/* FIXME bugzilla.eazel.com 1286: 
		 * How should we handle the case where this mkdir fails? 
		 * Note that nautilus_application_startup will refuse to launch if this 
		 * directory doesn't get created, so that case is OK. But the directory 
		 * could be deleted after Nautilus was launched, and perhaps
		 * there is some bad side-effect of not handling that case.
		 */
	}

	return user_directory;
}


/**
 * nautilus_get_desktop_directory:
 * 
 * Get the path for the directory containing files on the desktop.
 *
 * Return value: the directory path.
 **/
static char *
nautilus_get_desktop_directory (void)
{
	char *desktop_directory, *user_directory;

	user_directory = nautilus_get_user_directory ();
	desktop_directory = nautilus_make_path (user_directory, DESKTOP_DIRECTORY_NAME);
	g_free (user_directory);

	if (!g_file_exists (desktop_directory)) {
		mkdir (desktop_directory, DEFAULT_DESKTOP_DIRECTORY_MODE);
		/* FIXME bugzilla.eazel.com 1286: 
		 * How should we handle the case where this mkdir fails? 
		 * Note that nautilus_application_startup will refuse to launch if this 
		 * directory doesn't get created, so that case is OK. But the directory 
		 * could be deleted after Nautilus was launched, and perhaps
		 * there is some bad side-effect of not handling that case.
		 */
	}

	return desktop_directory;
}

/* handle the final page finishing  */

void
druid_set_first_time_file_flag (void)
{
	FILE *stream;
	char *user_directory, *druid_flag_file_name;
	
	user_directory = nautilus_get_user_directory ();
	druid_flag_file_name = g_strdup_printf ("%s/%s",
						user_directory,
						"first-time-flag");
	g_free (user_directory);

		
	stream = fopen (druid_flag_file_name, "w");
	if (stream) {
		const char *blurb =
			_("Existence of this file indicates that the Nautilus configuration druid\n"
				"has been presented.\n\n"
				"You can manually erase this file to present the druid again.\n\n");
			
			fwrite (blurb, sizeof (char), strlen (blurb), stream);
			fclose (stream);
	}
	
	g_free (druid_flag_file_name);
}

void
convert_gmc_desktop_icons (void)
{
	const char *home_dir;
	char *gmc_desktop_dir,*nautilus_desktop_dir, *link_path;
	struct stat st;
	DIR *dir;
	struct dirent *dirent;
	GnomeDesktopEntry *gmc_link;
	
	gmc_desktop_dir = gnome_util_prepend_user_home (".gnome-desktop/");
	
	if (stat (gmc_desktop_dir, &st) != 0) {
		g_free (gmc_desktop_dir);
		return;
	}
	
	if (!S_ISDIR (st.st_mode)) {
		g_free (gmc_desktop_dir);
		g_message ("Not a dir");
		return;
	}
	
	dir = opendir (gmc_desktop_dir);
	if (dir == NULL) {
		g_free (gmc_desktop_dir);
		return;
	}

	nautilus_desktop_dir = nautilus_get_desktop_directory ();

	/* Iterate all the files here and indentify the GMC links. */
	for (dirent = readdir (dir); dirent != NULL; dirent = readdir (dir)) {
		if (strcmp (dirent->d_name, ".") == 0 || strcmp (dirent->d_name, "..") == 0) {
			continue;
		}
		
		link_path = g_strdup_printf ("%s/%s", gmc_desktop_dir, dirent->d_name);

		gmc_link = gnome_desktop_entry_load (link_path);
		gmc_link = gnome_desktop_entry_load_unconditional (link_path);
		g_free (link_path);
		
		if (gmc_link != NULL) {
			nautilus_link_local_create_from_gnome_entry (gmc_link, nautilus_desktop_dir, NULL);
			gnome_desktop_entry_free (gmc_link);
		}
	}
				
	g_free (gmc_desktop_dir);
	g_free (nautilus_desktop_dir);	

}
