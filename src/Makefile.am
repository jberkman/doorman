## Process this file with automake to produce Makefile.in

doormandir = $(datadir)/doorman

INCLUDES = \
	-I$(top_srcdir)/intl $(EXTRA_GNOME_CFLAGS)	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DDATADIR=\""$(datadir)/doorman"\"		\
	-DICONDIR=\""$(datadir)/pixmaps"\"

bin_PROGRAMS = doorman

doorman_SOURCES = \
	doorman.h	\
	doorman.c	\
	the-machine.c

doorman_LDADD =$(EXTRA_GNOME_LIBS) $(INTLLIBS)

