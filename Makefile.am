AUTOMAKE_OPTIONS = foreign

SUBDIRS = src

DOXYGEN_QORE_API_FILTER=docs/doxygen-api-filter.q
DOXYGEN_OUTPUT=docs/xml

DOXYFILES = docs/doxyfile \
	docs/mainpage.doxygen \
	docs/xml.q \
	docs/XmlRpcClient.qc \
	docs/XmlDoc.qc \
	docs/XmlNode.qc \
	docs/XmlReader.qc

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

if COND_DOXYGEN
DX_CLEANFILES = ${DOXYFILES}
docs/xml/html/index.html: $(DOXYFILES)
	cd docs; $(DOXYGEN_CMD) doxyfile;
	$(DOXYGEN_QORE_API_FILTER) --post $(DOXYGEN_OUTPUT)/html/*html
	$(DOXYGEN_QORE_API_FILTER) --post $(DOXYGEN_OUTPUT)/html/search/*html

doxygen-doc: docs/xml/html/index.html

else
doxygen-doc:
	echo no doxygen support
endif

mostlyclean-local:
	rm -rf ${DOXYGEN_OUTPUT} ${DX_CLEANFILES}

docs/mainpage.doxygen: docs/mainpage.doxygen.tmpl
	$(DOXYGEN_QORE_API_FILTER) -l $<

docs/doxyfile: docs/doxyfile.tmpl
	sed s/__VERSION__/${VERSION}/ $< > $@

docs/xml.q: src/ql_xml.cpp
	$(DOXYGEN_QORE_API_FILTER) $< $@

docs/XmlRpcClient.qc: src/QC_XmlRpcClient.cpp
	$(DOXYGEN_QORE_API_FILTER) $< $@

docs/XmlDoc.qc: src/QC_XmlDoc.cpp
	$(DOXYGEN_QORE_API_FILTER) $< $@

docs/XmlNode.qc: src/QC_XmlNode.cpp
	$(DOXYGEN_QORE_API_FILTER) $< $@

docs/XmlReader.qc: src/QC_XmlReader.cpp
	$(DOXYGEN_QORE_API_FILTER) $< $@

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
