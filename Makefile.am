AUTOMAKE_OPTIONS = foreign

SUBDIRS = src

noinst_HEADERS = src/QC_XmlDoc.h \
	src/QC_XmlNode.h \
	src/QC_XmlReader.h \
	src/QC_XmlRpcClient.h \
	src/QoreXPath.h \
	src/QoreXmlDoc.h \
	src/QoreXmlReader.h \
	src/ql_xml.h \
	src/qore-xml-module.h

EXTRA_DIST = COPYING ChangeLog AUTHORS README \
	RELEASE-NOTES \
	test/xml-test.q \
	qore-xml-module.spec

ACLOCAL_AMFLAGS=-I m4

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
