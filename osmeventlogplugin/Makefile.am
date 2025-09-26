AM_CPPFLAGS = -I$(srcdir)/../include \
	      -I$(includedir)/infiniband

lib_LTLIBRARIES = libosmeventlogplugin.la

if DEBUG
DBGFLAGS = -ggdb -D_DEBUG_
else
DBGFLAGS = -g
endif

libosmeventlogplugin_la_CFLAGS = -Wall -Wwrite-strings $(DBGFLAGS) -D_XOPEN_SOURCE=600 -D_GNU_SOURCE=1

if HAVE_LD_VERSION_SCRIPT
    libosmeventlogplugin_version_script = -Wl,--version-script=$(srcdir)/libosmeventlogplugin.map
else
    libosmeventlogplugin_version_script =
endif

osmeventlogplugin_api_version=$(shell grep LIBVERSION= $(srcdir)/libosmeventlogplugin.ver | sed 's/LIBVERSION=//')

libosmeventlogplugin_la_SOURCES = src/osmeventlogplugin.c
libosmeventlogplugin_la_LDFLAGS = -version-info $(osmeventlogplugin_api_version) \
	 -export-dynamic $(libosmeventlogplugin_version_script)
libosmeventlogplugin_la_LIBADD = -L../complib -losmcomp -L../libopensm -lopensm $(OSMV_LDADD)
libosmeventlogplugin_la_DEPENDENCIES = $(srcdir)/libosmeventlogplugin.map

libosmeventlogpluginincludedir = $(includedir)/infiniband/complib

libosmeventlogplugininclude_HEADERS =

# headers are distributed as part of the include dir
EXTRA_DIST = $(srcdir)/libosmeventlogplugin.map $(srcdir)/libosmeventlogplugin.ver