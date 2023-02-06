# LIBRARIES
##################################################
if USE_OPENRDM
lib_LTLIBRARIES += plugins/openrdm/libolaopenrdm.la

# Plugin description is generated from README.md
built_sources += plugins/openrdm/OpenRDMPluginDescription.h
nodist_plugins_openrdm_libolaopenrdm_la_SOURCES = \
    plugins/openrdm/OpenRDMPluginDescription.h
plugins/openrdm/OpenRDMPluginDescription.h: plugins/openrdm/README.md plugins/openrdm/Makefile.mk plugins/convert_README_to_header.sh
	sh $(top_srcdir)/plugins/convert_README_to_header.sh $(top_srcdir)/plugins/openrdm $(top_builddir)/plugins/openrdm/OpenRDMPluginDescription.h

plugins_openrdm_libolaopenrdm_la_SOURCES = \
    plugins/openrdm/OpenRDMPlugin.cpp \
    plugins/openrdm/OpenRDMPlugin.h \
    plugins/openrdm/OpenRDMDevice.cpp \
    plugins/openrdm/OpenRDMDevice.h \
    plugins/openrdm/OpenRDMThread.cpp \
    plugins/openrdm/OpenRDMThread.h \
    plugins/openrdm/OpenRDMWidget.cpp \
    plugins/openrdm/OpenRDMWidget.h \
    plugins/openrdm/OpenRDMDriver.c \
    plugins/openrdm/OpenRDMDriver.h \
    plugins/openrdm/dmx.h \
    plugins/openrdm/rdm.cpp \
    plugins/openrdm/rdm.h \
    plugins/openrdm/Semaphore.cpp \
    plugins/openrdm/Semaphore.h
plugins_openrdm_libolaopenrdm_la_LIBADD = \
    common/libolacommon.la \
    olad/plugin_api/libolaserverplugininterface.la

if HAVE_LIBFTDI1
plugins_openrdm_libolaopenrdm_la_LIBADD += $(libftdi1_LIBS)
else
plugins_openrdm_libolaopenrdm_la_LIBADD += $(libftdi0_LIBS)
endif

endif

EXTRA_DIST += plugins/openrdm/README.md
