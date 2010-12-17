/*
  Qore xml module

  Copyright (C) 2010 Qore Technologies

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "qore-xml-module.h"

#include "QC_XmlRpcClient.h"
#include "QC_XmlNode.h"
#include "QC_XmlDoc.h"
#include "QC_XmlNode.h"
#include "QC_XmlReader.h"

#include "ql_xml.h"

#include <libxml/xmlversion.h>

#include <stdarg.h>

QoreStringNode *xml_module_init();
void xml_module_ns_init(QoreNamespace *rns, QoreNamespace *qns);
void xml_module_delete();

// qore module symbols
DLLEXPORT char qore_module_name[] = "xml";
DLLEXPORT char qore_module_version[] = PACKAGE_VERSION;
DLLEXPORT char qore_module_description[] = "xml module";
DLLEXPORT char qore_module_author[] = "David Nichols";
DLLEXPORT char qore_module_url[] = "http://qore.org";
DLLEXPORT int qore_module_api_major = QORE_MODULE_API_MAJOR;
DLLEXPORT int qore_module_api_minor = QORE_MODULE_API_MINOR;
DLLEXPORT qore_module_init_t qore_module_init = xml_module_init;
DLLEXPORT qore_module_ns_init_t qore_module_ns_init = xml_module_ns_init;
DLLEXPORT qore_module_delete_t qore_module_delete = xml_module_delete;
DLLEXPORT qore_license_t qore_module_license = QL_LGPL;

QoreNamespace XNS("Xml");

static void qoreXmlGenericErrorFunc(QoreString *err, const char *msg, ...) {
   va_list args;
   va_start(args, msg);
   err->clear();
   err->vsprintf(msg, args);
   va_end(args);
}

QoreStringNode *xml_module_init() {
   QoreString err;

   // set our generic error handler to catch initialization errors
   xmlSetGenericErrorFunc((void*)&err,  (xmlGenericErrorFunc)qoreXmlGenericErrorFunc);

   // initialize libxml2 library
   LIBXML_TEST_VERSION

   if (err.strlen())
      return new QoreStringNode(err);

   // reset the generic error handler back to the default
   initGenericErrorDefaultFunc(0);

   XNS.addConstant("XML_ELEMENT_NODE",         new QoreBigIntNode(XML_ELEMENT_NODE));
   XNS.addConstant("XML_ATTRIBUTE_NODE",       new QoreBigIntNode(XML_ATTRIBUTE_NODE));
   XNS.addConstant("XML_TEXT_NODE",            new QoreBigIntNode(XML_TEXT_NODE));
   XNS.addConstant("XML_CDATA_SECTION_NODE",   new QoreBigIntNode(XML_CDATA_SECTION_NODE));
   XNS.addConstant("XML_ENTITY_REF_NODE",      new QoreBigIntNode(XML_ENTITY_REF_NODE));
   XNS.addConstant("XML_ENTITY_NODE",          new QoreBigIntNode(XML_ENTITY_NODE));
   XNS.addConstant("XML_PI_NODE",              new QoreBigIntNode(XML_PI_NODE));
   XNS.addConstant("XML_COMMENT_NODE",         new QoreBigIntNode(XML_COMMENT_NODE));
   XNS.addConstant("XML_DOCUMENT_NODE",        new QoreBigIntNode(XML_DOCUMENT_NODE));
   XNS.addConstant("XML_DOCUMENT_TYPE_NODE",   new QoreBigIntNode(XML_DOCUMENT_TYPE_NODE));
   XNS.addConstant("XML_DOCUMENT_FRAG_NODE",   new QoreBigIntNode(XML_DOCUMENT_FRAG_NODE));
   XNS.addConstant("XML_NOTATION_NODE",        new QoreBigIntNode(XML_NOTATION_NODE));
   XNS.addConstant("XML_HTML_DOCUMENT_NODE",   new QoreBigIntNode(XML_HTML_DOCUMENT_NODE));
   XNS.addConstant("XML_DTD_NODE",             new QoreBigIntNode(XML_DTD_NODE));
   XNS.addConstant("XML_ELEMENT_DECL",         new QoreBigIntNode(XML_ELEMENT_DECL));
   XNS.addConstant("XML_ATTRIBUTE_DECL",       new QoreBigIntNode(XML_ATTRIBUTE_DECL));
   XNS.addConstant("XML_ENTITY_DECL",          new QoreBigIntNode(XML_ENTITY_DECL));
   XNS.addConstant("XML_NAMESPACE_DECL",       new QoreBigIntNode(XML_NAMESPACE_DECL));
   XNS.addConstant("XML_XINCLUDE_START",       new QoreBigIntNode(XML_XINCLUDE_START));
   XNS.addConstant("XML_XINCLUDE_END",         new QoreBigIntNode(XML_XINCLUDE_END));
   XNS.addConstant("XML_DOCB_DOCUMENT_NODE",   new QoreBigIntNode(XML_DOCB_DOCUMENT_NODE));

   XNS.addConstant("XML_NODE_TYPE_NONE",                    new QoreBigIntNode(XML_READER_TYPE_NONE));
   XNS.addConstant("XML_NODE_TYPE_ELEMENT",                 new QoreBigIntNode(XML_READER_TYPE_ELEMENT));
   XNS.addConstant("XML_NODE_TYPE_ATTRIBUTE",               new QoreBigIntNode(XML_READER_TYPE_ATTRIBUTE));
   XNS.addConstant("XML_NODE_TYPE_TEXT",                    new QoreBigIntNode(XML_READER_TYPE_TEXT));
   XNS.addConstant("XML_NODE_TYPE_CDATA",                   new QoreBigIntNode(XML_READER_TYPE_CDATA));
   XNS.addConstant("XML_NODE_TYPE_ENTITY_REFERENCE",        new QoreBigIntNode(XML_READER_TYPE_ENTITY_REFERENCE));
   XNS.addConstant("XML_NODE_TYPE_ENTITY",                  new QoreBigIntNode(XML_READER_TYPE_ENTITY));
   XNS.addConstant("XML_NODE_TYPE_PROCESSING_INSTRUCTION",  new QoreBigIntNode(XML_READER_TYPE_PROCESSING_INSTRUCTION));
   XNS.addConstant("XML_NODE_TYPE_COMMENT",                 new QoreBigIntNode(XML_READER_TYPE_COMMENT));
   XNS.addConstant("XML_NODE_TYPE_DOCUMENT",                new QoreBigIntNode(XML_READER_TYPE_DOCUMENT));
   XNS.addConstant("XML_NODE_TYPE_DOCUMENT_TYPE",           new QoreBigIntNode(XML_READER_TYPE_DOCUMENT_TYPE));
   XNS.addConstant("XML_NODE_TYPE_DOCUMENT_FRAGMENT",       new QoreBigIntNode(XML_READER_TYPE_DOCUMENT_FRAGMENT));
   XNS.addConstant("XML_NODE_TYPE_NOTATION",                new QoreBigIntNode(XML_READER_TYPE_NOTATION));
   XNS.addConstant("XML_NODE_TYPE_WHITESPACE",              new QoreBigIntNode(XML_READER_TYPE_WHITESPACE));
   XNS.addConstant("XML_NODE_TYPE_SIGNIFICANT_WHITESPACE",  new QoreBigIntNode(XML_READER_TYPE_SIGNIFICANT_WHITESPACE));
   XNS.addConstant("XML_NODE_TYPE_END_ELEMENT",             new QoreBigIntNode(XML_READER_TYPE_END_ELEMENT));
   XNS.addConstant("XML_NODE_TYPE_END_ENTITY",              new QoreBigIntNode(XML_READER_TYPE_END_ENTITY));
   XNS.addConstant("XML_NODE_TYPE_XML_DECLARATION",         new QoreBigIntNode(XML_READER_TYPE_XML_DECLARATION));

   // set up element map
   QoreHashNode *xm = new QoreHashNode;
   xm->setKeyValue("1",  new QoreStringNode("XML_ELEMENT_NODE"), 0);
   xm->setKeyValue("2",  new QoreStringNode("XML_ATTRIBUTE_NODE"), 0);
   xm->setKeyValue("3",  new QoreStringNode("XML_TEXT_NODE"), 0);
   xm->setKeyValue("4",  new QoreStringNode("XML_CDATA_SECTION_NODE"), 0);
   xm->setKeyValue("5",  new QoreStringNode("XML_ENTITY_REF_NODE"), 0);
   xm->setKeyValue("6",  new QoreStringNode("XML_ENTITY_NODE"), 0);
   xm->setKeyValue("7",  new QoreStringNode("XML_PI_NODE"), 0);
   xm->setKeyValue("8",  new QoreStringNode("XML_COMMENT_NODE"), 0);
   xm->setKeyValue("9",  new QoreStringNode("XML_DOCUMENT_NODE"), 0);
   xm->setKeyValue("10", new QoreStringNode("XML_DOCUMENT_TYPE_NODE"), 0);
   xm->setKeyValue("11", new QoreStringNode("XML_DOCUMENT_FRAG_NODE"), 0);
   xm->setKeyValue("12", new QoreStringNode("XML_NOTATION_NODE"), 0);
   xm->setKeyValue("13", new QoreStringNode("XML_HTML_DOCUMENT_NODE"), 0);
   xm->setKeyValue("14", new QoreStringNode("XML_DTD_NODE"), 0);
   xm->setKeyValue("15", new QoreStringNode("XML_ELEMENT_DECL"), 0);
   xm->setKeyValue("16", new QoreStringNode("XML_ATTRIBUTE_DECL"), 0);
   xm->setKeyValue("17", new QoreStringNode("XML_ENTITY_DECL"), 0);
   xm->setKeyValue("18", new QoreStringNode("XML_NAMESPACE_DECL"), 0);
   xm->setKeyValue("19", new QoreStringNode("XML_XINCLUDE_START"), 0);
   xm->setKeyValue("20", new QoreStringNode("XML_XINCLUDE_END"), 0);
   xm->setKeyValue("21", new QoreStringNode("XML_DOCB_DOCUMENT_NODE"), 0);

   XNS.addConstant("ElementTypeMap", xm);

   // now set up node map
   xm = new QoreHashNode;
   xm->setKeyValue("0",  new QoreStringNode("XML_NODE_TYPE_NONE"), 0);
   xm->setKeyValue("1",  new QoreStringNode("XML_NODE_TYPE_ELEMENT"), 0);
   xm->setKeyValue("2",  new QoreStringNode("XML_NODE_TYPE_ATTRIBUTE"), 0);
   xm->setKeyValue("3",  new QoreStringNode("XML_NODE_TYPE_TEXT"), 0);
   xm->setKeyValue("4",  new QoreStringNode("XML_NODE_TYPE_CDATA"), 0);
   xm->setKeyValue("5",  new QoreStringNode("XML_NODE_TYPE_ENTITY_REFERENCE"), 0);
   xm->setKeyValue("6",  new QoreStringNode("XML_NODE_TYPE_ENTITY"), 0);
   xm->setKeyValue("7",  new QoreStringNode("XML_NODE_TYPE_PROCESSING_INSTRUCTION"), 0);
   xm->setKeyValue("8",  new QoreStringNode("XML_NODE_TYPE_COMMENT"), 0);
   xm->setKeyValue("9",  new QoreStringNode("XML_NODE_TYPE_DOCUMENT"), 0);
   xm->setKeyValue("10", new QoreStringNode("XML_NODE_TYPE_DOCUMENT_TYPE"), 0);
   xm->setKeyValue("11", new QoreStringNode("XML_NODE_TYPE_DOCUMENT_FRAGMENT"), 0);
   xm->setKeyValue("12", new QoreStringNode("XML_NODE_TYPE_NOTATION"), 0);
   xm->setKeyValue("13", new QoreStringNode("XML_NODE_TYPE_WHITESPACE"), 0);
   xm->setKeyValue("14", new QoreStringNode("XML_NODE_TYPE_SIGNIFICANT_WHITESPACE"), 0);
   xm->setKeyValue("15", new QoreStringNode("XML_NODE_TYPE_END_ELEMENT"), 0);
   xm->setKeyValue("16", new QoreStringNode("XML_NODE_TYPE_END_ENTITY"), 0);
   xm->setKeyValue("17", new QoreStringNode("XML_NODE_TYPE_XML_DECLARATION"), 0);

   XNS.addConstant("NodeTypeMap", xm);

   QoreClass *XmlNode, *XmlDoc;
   XNS.addSystemClass(XmlNode = initXmlNodeClass());
   XNS.addSystemClass(XmlDoc = initXmlDocClass(XmlNode));
   XNS.addSystemClass(initXmlReaderClass(XmlDoc));

   XNS.addSystemClass(initXmlRpcClientClass(QC_HTTPCLIENT));

   // set up Option namespace for XML options
   QoreNamespace *option = new QoreNamespace("Option");

#ifdef HAVE_XMLTEXTREADERSETSCHEMA
   option->addConstant("HAVE_PARSEXMLWITHSCHEMA",  &True);
#else
   option->addConstant("HAVE_PARSEXMLWITHSCHEMA",  &False);
#endif

#ifdef HAVE_XMLTEXTREADERRELAXNGSETSCHEMA
   option->addConstant("HAVE_PARSEXMLWITHRELAXNG",  &True);
#else
   option->addConstant("HAVE_PARSEXMLWITHRELAXNG",  &False);
#endif

   XNS.addInitialNamespace(option);

   init_xml_functions();

   return 0;
}

void xml_module_ns_init(QoreNamespace *rns, QoreNamespace *qns) {
   qns->addNamespace(XNS.copy());
}

void xml_module_delete() {
   // cleanup libxml2 library
   xmlCleanupParser();
}
