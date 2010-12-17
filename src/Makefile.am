
if COND_SINGLE_COMPILATION_UNIT
XML_SOURCES = single-compilation-unit.cpp
else
XML_SOURCES = xml-module.cpp \
	QC_XmlDoc.cpp \
	QC_XmlNode.cpp \
	QC_XmlReader.cpp \
	QC_XmlRpcClient.cpp \
	ql_xml.cpp

endif

lib_LTLIBRARIES = xml.la
xml_la_SOURCES = $(XML_SOURCES)
xml_la_LDFLAGS = -module -avoid-version ${LIBXML2_LDFLAGS}

INCLUDES = -I$(top_srcdir)/include

AM_CPPFLAGS=$(QORE_CPPFLAGS) ${LIBXML2_CPPFLAGS}

if COND_PROFILE
AM_CXXFLAGS=-g -pg
else
if COND_DEBUG
AM_CXXFLAGS=-g
endif
endif

install-exec-hook:
	mv $(DESTDIR)$(libdir)/xml.$(MODULE_SUFFIX) $(DESTDIR)$(libdir)/xml-api-$(MODULE_API).qmod
	rm -f $(DESTDIR)$(libdir)/xml.la
