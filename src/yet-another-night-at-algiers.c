/* Customizable desktop links for the Midnight Commander
 *
 * Copyright (C) 1998-1999 The Free Software Foundation
 *
 * Authors: Miguel de Icaza <miguel@nuclecu.unam.mx>
 */

#include "config.h"
#include "doorman.h"
#include "file.h"

#include <unistd.h>

#include <gnome.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>

#include <signal.h>
#include <errno.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#define EXECUTE_INTERNAL   1
#define EXECUTE_TEMPFILE   2
#define EXECUTE_AS_SHELL   4
#define EXECUTE_SETUID     8
#define EXECUTE_WAIT      16

static char *desktop_directory = NULL;
static char *shell = NULL;

static void
desktop_create_url (const char *filename, const char *title, const char *url, const char *icon)
{
	FILE *f;

	f = fopen (filename, "w");
	if (f) {

		fprintf (f, "URL: %s\n", url);
		fclose (f);

		gnome_metadata_set (filename, "desktop-url",
				    strlen (url) + 1, url);
		gnome_metadata_set (filename, "icon-caption",
				    strlen (title) + 1, title);

		gnome_metadata_set (filename, "icon-filename", strlen (icon) + 1, icon);
	}
}

static void
desktop_load_init_from (const char *file)
{
	char *key;
	char *file_and_section;
	char *title, *type;

	void *iterator_handle;
	char *config_path = g_strconcat ("=", file, "=", NULL);

	iterator_handle = gnome_config_init_iterator_sections (config_path);

	iterator_handle = gnome_config_iterator_next (
		iterator_handle, &key, NULL);

	do {
		/* Now access the values in the section */
		file_and_section = g_strconcat (config_path, "/", key, "/", NULL);

		gnome_config_push_prefix (file_and_section);
		title = gnome_config_get_translated_string ("title=None");
		type = gnome_config_get_string ("type=url");

		/*
		 * handle the different link types
		 */
		if (strcmp (type, "url") == 0){
			int  used;
			char *icon, *url;
			char *icon2 = NULL;

			url = gnome_config_get_string ("url");
			icon = gnome_config_get_string_with_default ("icon=", &used);
			if (icon){
				icon2 = gnome_pixmap_file (icon);
				g_free (icon);
			}
			if (!icon2)
				icon2 = g_concat_dir_and_file (ICONDIR, "gnome-http-url.png");
			if (url && *url){
				char *filename = g_concat_dir_and_file (desktop_directory, key);

				desktop_create_url (filename, title, url, icon2);
				g_free (filename);
			}

			if (url)
				g_free (url);
			g_free (icon2);
		}
		g_free (title);
		g_free (file_and_section);
		gnome_config_pop_prefix ();
		
		/* Get next section name */
		iterator_handle = gnome_config_iterator_next (
			iterator_handle, &key, NULL);

	} while (iterator_handle);

	g_free (config_path);
}

static int 
is_exe (mode_t mode)
{
    if ((S_IXUSR & mode) || (S_IXGRP & mode) || (S_IXOTH & mode))
	return 1;
    return 0;
}

static char *
name_quote (const char *s, int quote_percent)
{
    char *ret, *d;
    
    d = ret = g_malloc (strlen (s)*2 + 2 + 1);
    if (*s == '-') {
        *d++ = '.';
        *d++ = '/';
    }

    for (; *s; s++, d++) {
	switch (*s)
	{
	    case '%':
		if (quote_percent)
		    *d++ = '%';
		break;
	    case '\'':
	    case '\\':
	    case '\r':
	    case '\n':
	    case '\t':
	    case '"':
	    case ':':
	    case ';':
	    case ' ':
	    case '?':
	    case '|':
	    case '[':
	    case ']':
	    case '{':
	    case '}':
	    case '<':
	    case '>':
	    case '`':
	    case '~':
	    case '!':
	    case '@':
	    case '#':
	    case '$':
	    case '^':
	    case '&':
	    case '*':
	    case '(':
	    case ')':
		*d++ = '\\';
	}
	*d = *s;
    }
    *d = '\0';
    return ret;
}


static struct sigaction previous_sigchld;
static int child_died_notify_handler;

/*
 * A list of childs that were executed with a temporary file
 * We remove the files when they die
 */
static GList *children;

typedef struct {
	pid_t pid;
	char  *temp_file;
} Child;

/*
 * Received when a child dies, notifies the high level routione
 * that new input is available
 */
static void
gnome_sigchld_handler (int sig)
{
	char c;
	
	if (previous_sigchld.sa_handler != SIG_IGN &&
	    previous_sigchld.sa_handler != SIG_DFL){
		(*previous_sigchld.sa_handler)(sig);
	}
	write (child_died_notify_handler, &c, sizeof (c));
}

/*
 * Invoked from the main loop when a child has died
 * deal with it
 */
static void
gnome_child_died (gpointer data, gint source, GdkInputCondition condition)
{
	GList *l;
	char c;
	
	read (source, &c, sizeof (c));
	for (l = children; l; l = l->next){
		int status;
		Child *child = l->data;
		
		if (child->pid == waitpid (child->pid, &status, WUNTRACED | WNOHANG)){
			children = g_list_remove (children, child);
				
			unlink (child->temp_file);
			g_free (child->temp_file);
			g_free (child);
		}
	}
}

static int
max_open_files (void)
{
	static int files;

	if (files)
		return files;

#ifdef HAVE_SYSCONF
	files = sysconf (_SC_OPEN_MAX);
	if (files != -1)
		return files;
#endif
#ifdef OPEN_MAX
	return files = OPEN_MAX;
#else
	return files = 256;
#endif
}

static int 
my_system_get_child_pid (int flags, const char *shell, const char *command, pid_t *pid)
{
	struct sigaction ignore, save_intr, save_quit, save_stop;
	int status = 0, i;
	static int gnome_sigchld_installed;
	
	ignore.sa_handler = SIG_IGN;
	sigemptyset (&ignore.sa_mask);
	ignore.sa_flags = 0;
    
	sigaction (SIGINT, &ignore, &save_intr);    
	sigaction (SIGQUIT, &ignore, &save_quit);

	if (!gnome_sigchld_installed){
		struct sigaction newsig;
		int monitors [2];

		pipe (monitors);
		sigemptyset (&newsig.sa_mask);
		newsig.sa_flags = 0;
		newsig.sa_handler = gnome_sigchld_handler;
		
		sigaction (SIGCHLD, &newsig, &previous_sigchld);
		gnome_sigchld_installed = 1;

		gdk_input_add (monitors [0], GDK_INPUT_READ, gnome_child_died, NULL);
	}
	
	if ((*pid = fork ()) < 0){
		return -1;
	}
	if (*pid == 0){
		const int top = max_open_files ();
		struct sigaction default_pipe;

		sigaction (SIGINT,  &save_intr, NULL);
		sigaction (SIGQUIT, &save_quit, NULL);

		/*
		 * reset sigpipe
		 */
		default_pipe.sa_handler = SIG_DFL;
		sigemptyset (&default_pipe.sa_mask);
		default_pipe.sa_flags = 0;
		
		sigaction (SIGPIPE, &default_pipe, NULL);
		
		for (i = 0; i < top; i++)
			close (i);

		/* Setup the file descriptor for the child */
		   
		/* stdin */
		open ("/dev/null", O_APPEND);

		/* stdout */
		open ("/dev/null", O_RDONLY);

		/* stderr */
		open ("/dev/null", O_RDONLY);
		
		if (!(flags & EXECUTE_WAIT))
			*pid = fork ();
		
		if (*pid == 0){
			if (flags & EXECUTE_AS_SHELL)
				execl (shell, shell, "-c", command, (char *) 0);
			else
				execlp (shell, shell, command, (char *) 0);
			/* See note below for why we use _exit () */
			_exit (127);		/* Exec error */
		} else {
			int status;

			if (flags & EXECUTE_WAIT)
				waitpid (*pid, &status, 0);

			if (flags & EXECUTE_TEMPFILE){
				Child *child;

				child = g_new (Child, 1);
				child->pid = *pid;
				child->temp_file = g_strdup (command);
			}
		}
		/* We need to use _exit instead of exit to avoid
		 * calling the atexit handlers (specifically the gdk atexit
		 * handler
		 */
		_exit (0);
	}
	waitpid (*pid, &status, 0);
	sigaction (SIGINT,  &save_intr, NULL);
	sigaction (SIGQUIT, &save_quit, NULL);
	sigaction (SIGTSTP, &save_stop, NULL);

	return WEXITSTATUS(status);
}

static int 
my_system (int flags, const char *shell, const char *command)
{
	pid_t pid;
	
	return my_system_get_child_pid (flags, shell, command, &pid);
}

static void
desktop_init_at (const char *dir)
{
	DIR *d;
	struct dirent *dent;
	struct stat s;
	const int links_extlen = sizeof (".links")-1;

	d = opendir (dir);
	if (!d)
		return;

	while ((dent = readdir (d)) != NULL){
		int len = strlen (dent->d_name);
		char *fname;

		fname = g_concat_dir_and_file (dir, dent->d_name);

		if (stat (fname, &s) < 0) {
			g_free (fname);
			continue;
		}
		if (S_ISDIR (s.st_mode)) {
			g_free (fname);
			continue;
		}
		if (is_exe (s.st_mode)) {
			/* let's try running it */
			char *desktop_quoted;
			char *command;

			desktop_quoted = name_quote (desktop_directory, 0);
			command = g_strconcat (fname, " --desktop-dir=", desktop_quoted, NULL);
			g_free (desktop_quoted);

			my_system (EXECUTE_WAIT | EXECUTE_AS_SHELL, shell, command);
			g_free (command);
			g_free (fname);
			continue;
		}

		if (len < links_extlen){
			g_free (fname);
			continue;
		}

		if (strcmp (dent->d_name + len - links_extlen, ".links")){
			g_free (fname);
			continue;
		}

		desktop_load_init_from (fname);
		g_free (fname);
	}
	closedir (d);
}

void
gdesktop_links_init (void)
{
	char *link_name;
	char *icon;
	char *dir;

	if (!desktop_directory)
		desktop_directory = gnome_util_prepend_user_home (".gnome-desktop/");

	mkdirs (desktop_directory);

	if (!shell) {
		shell = getenv ("SHELL");
		if (!shell || !*shell)
			shell = g_strdup (getpwuid (geteuid ())->pw_shell);
		if (!shell || !*shell)
			shell = "/bin/sh";
	}

	/* Create the link to the user's home directory so that he will have an icon */
	link_name = g_concat_dir_and_file (desktop_directory, _("Home directory"));
	symlink (gnome_user_home_dir, link_name);
	g_free (link_name);

	/* Create the link to the user's trash directory */
	link_name = g_concat_dir_and_file (desktop_directory, "Trash");
	icon = gnome_pixmap_file ("mc/gnome-trashcan.png");
	mkdir (link_name, S_IRUSR | S_IWUSR | S_IXUSR );
	if (icon){
		gnome_metadata_set (link_name, "icon-filename", strlen (icon) + 1, icon);
		g_free (icon);
	}
	g_free (link_name);

	/* Create custom links */

	desktop_init_at (DESKTOP_INIT_DIR);

	dir = gnome_libdir_file ("desktop-links");
	if (dir) {
		desktop_init_at (dir);
		g_free (dir);
	}

	gnome_config_sync ();
}
