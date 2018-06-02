lib_LTLIBRARIES = relativeBZDBValues.la

relativeBZDBValues_la_SOURCES = relativeBZDBValues.cpp
relativeBZDBValues_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
relativeBZDBValues_la_LDFLAGS = -module -avoid-version -shared -lyaml-cpp
relativeBZDBValues_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	LICENSE.md \
	README.md \
	relativeBZDBValues.yml \
	relativeBZDBValues.def \
	relativeBZDBValues.sln \
	relativeBZDBValues.vcxproj \
	relativeBZDBValues.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
