## Process this file with automake to produce Makefile.in

doormandir = $(datadir)/doorman

INCLUDES = \
	-I$(top_srcdir)/intl $(EXTRA_GNOME_CFLAGS)	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DDATADIR=\""$(datadir)/doorman"\"		\
	-DICONDIR=\""$(datadir)/pixmaps"\"		\
	-DGTK_THEMEDIR=\""$(GTK_PREFIX)/share/themes"\"

bin_PROGRAMS = doorman

doorman_SOURCES = \
	doorman.h		\
	doorman.c		\
	file.c			\
	file.h			\
	the-machine.c		\
	theme-page.c		\
	theme-page-background.c	\
	theme-page-desktop.c	\
	theme-page-gtk.c	\
	theme-page-panel.c	\
	theme-page-sawfish.c

doorman_LDADD = $(EXTRA_GNOME_LIBS) $(INTLLIBS)

