modverdir = $(libdir)/$(mymodverdir)

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

USER_MODULES = qlib/XmlRpcHandler.qm

EXTRA_DIST = COPYING ChangeLog AUTHORS README \
	RELEASE-NOTES \
	src/QC_XmlDoc.qpp \
	src/QC_XmlNode.qpp \
	src/QC_XmlReader.qpp \
	src/QC_XmlRpcClient.qpp \
	src/ql_xml.qpp \
	test/xml-test.q \
	examples/HttpServer.qc \
	examples/SoapClient.qc \
	examples/WSDL.qc \
	examples/MultiPartMessage.qc \
	examples/SoapHandler.qc \
	examples/xml-rpc-client.q \
	examples/XmlRpcServerValidation.q \
	$(USER_MODULES) \
	qore-xml-module.spec

ACLOCAL_AMFLAGS=-I m4

dist_modver_DATA = $(USER_MODULES)

if COND_DOXYGEN
DOX = xml XmlRpcHandler
DOXYGEN_OUTPUT=docs/xml/html docs/XmlRpcHandler/html
DOXYFILES = docs/doxyfile docs/mainpage.doxygen
DOXYFILES_SRC = docs/doxyfile.tmpl docs/qlib/Doxyfile.tmpl docs/footer_template.html
QLIB_TMP_DOXYFILES = docs/qlib/XmlRpcHandler.qm.dox.h docs/qlib/Doxyfile.XmlRpcHandler
QDX = qdx
DX_CLEANFILES = ${DOXYFILES} $(QLIB_TMP_DOXYFILES)

docs/qlib/Doxyfile.XmlRpcHandler: docs/qlib/Doxyfile.tmpl
	$(QDX) -M=qlib/XmlRpcHandler.qm:docs/qlib/XmlRpcHandler.qm.dox.h $< $@

docs/qlib/XmlRpcHandler.qm.dox.h: qlib/XmlRpcHandler.qm
	$(QDX) $< $@

docs/XmlRpcHandler/html: docs/qlib/XmlRpcHandler.qm.dox.h docs/qlib/Doxyfile.XmlRpcHandler
	$(DOXYGEN_CMD) docs/qlib/Doxyfile.XmlRpcHandler
	$(QDX) --post docs/XmlRpcHandler/html/*html
	$(QDX) --post docs/XmlRpcHandler/html/search/*html

docs/xml/html: $(DOXYFILES)
	cd docs; $(DOXYGEN_CMD) doxyfile;
	$(QDX) --post docs/xml/html/*html
	$(QDX) --post docs/xml/search/*html

docs/mainpage.doxygen: docs/mainpage.doxygen.tmpl
	$(QDX) -d $< $@

docs/doxyfile: docs/doxyfile.tmpl
	sed s/__VERSION__/${VERSION}/ $< > $@

doxygen-doc: $(DOXYGEN_OUTPUT)

EXTRA_DIST += $(DOXYGEN_OUTPUT)

html-local: $(DOXYGEN_OUTPUT)

install-html-local:
	for dox in $(DOX); do $(MKDIR_P) $(DESTDIR)$(datadir)/$(PACKAGE)/docs/$$dox/html/search; done
	for dir in $(DOXYGEN_OUTPUT); do \
	   for f in $$dir/*; do if [ -f $$f ]; then $(INSTALL_DATA) $$f $(DESTDIR)$(datadir)/$(PACKAGE)/$$dir; fi; done; \
	   for f in $$dir/search/*; do $(INSTALL_DATA) $$f $(DESTDIR)$(datadir)/$(PACKAGE)/$$dir/search; done; \
	done

clean-local:
	$(RM) -rf $(DOXYGEN_OUTPUT)

uninstall-local:
	$(RM) -rf $(DESTDIR)$(datadir)/$(PACKAGE)/docs

else
doxygen-doc:
	echo no doxygen support
endif

mostlyclean-local:
	rm -rf ${DOXYGEN_OUTPUT} ${DX_CLEANFILES}

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
