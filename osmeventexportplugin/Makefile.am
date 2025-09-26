AM_CPPFLAGS = -I$(srcdir)/../include \
	      -I$(includedir)/infiniband

lib_LTLIBRARIES = libosmeventexportplugin.la

if DEBUG
DBGFLAGS = -ggdb -D_DEBUG_
else
DBGFLAGS = -g
endif

libosmeventexportplugin_la_CFLAGS = -Wall -Wwrite-strings $(DBGFLAGS) -D_XOPEN_SOURCE=600 -D_GNU_SOURCE=1

if HAVE_LD_VERSION_SCRIPT
    libosmeventexportplugin_version_script = -Wl,--version-script=$(srcdir)/libosmeventexportplugin.map
else
    libosmeventexportplugin_version_script =
endif

osmeventexportplugin_api_version=$(shell grep LIBVERSION= $(srcdir)/libosmeventexportplugin.ver | sed 's/LIBVERSION=//')

libosmeventexportplugin_la_SOURCES = src/osmeventexportplugin.c
libosmeventexportplugin_la_LDFLAGS = -version-info $(osmeventexportplugin_api_version) \
	 -export-dynamic $(libosmeventexportplugin_version_script)
libosmeventexportplugin_la_LIBADD = -L../complib -losmcomp -L../libopensm -lopensm $(OSMV_LDADD)
libosmeventexportplugin_la_DEPENDENCIES = $(srcdir)/libosmeventexportplugin.map

libosmeventexportpluginincludedir = $(includedir)/infiniband/complib

libosmeventexportplugininclude_HEADERS =

# headers are distributed as part of the include dir
EXTRA_DIST = $(srcdir)/libosmeventexportplugin.map $(srcdir)/libosmeventexportplugin.ver