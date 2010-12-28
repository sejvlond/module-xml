/*
  QC_XmlDoc.cpp

  Qore Programming Language
  
  Copyright 2003 - 2010 David Nichols

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

//! @file XmlDoc.qc defines the XmlDoc class

#include "qore-xml-module.h"

#include "QC_XmlDoc.h"
#include "QoreXPath.h"
#include "QoreXmlReader.h"
#include "QC_XmlNode.h"
#include "ql_xml.h"

qore_classid_t CID_XMLDOC;

#ifdef HAVE_XMLTEXTREADERRELAXNGSETSCHEMA
int QoreXmlDoc::validateRelaxNG(const char *rng, int size, ExceptionSink *xsink) {
   QoreXmlRelaxNGContext schema(rng, size, xsink);
   if (!schema) {
      if (!*xsink)
	 xsink->raiseException("RELAXNG-SYNTAX-ERROR", "RelaxNG schema passed as argument to XmlDoc::validateRelaxNG() could not be parsed");
      return -1;
   }

   QoreXmlRelaxNGValidContext vcp(schema);
   int rc = vcp.validateDoc(ptr);

   if (!rc)
      return 0;
   if (*xsink)
      return -1;

   if (rc < 0)
      xsink->raiseException("RELAXNG-INTERNAL-ERROR", "an internal error occured validating the document against the RelaxNG schema passed; xmlRelaxNGValidateDoc() returned %d", rc);
   else if (rc)
      xsink->raiseException("RELAXNG-ERROR", "The document failed RelaxNG validation", rc);
   return -1;
}
#endif

#ifdef HAVE_XMLTEXTREADERSETSCHEMA
int QoreXmlDoc::validateSchema(const char *xsd, int size, ExceptionSink *xsink) {
   QoreXmlSchemaContext schema(xsd, size, xsink);
   if (!schema) {
      if (!*xsink)
	 xsink->raiseException("XSD-SYNTAX-ERROR", "XSD schema passed as argument to XmlDoc::validateSchema() could not be parsed");
      return -1;
   }

   QoreXmlSchemaValidContext vcp(schema);
   int rc = vcp.validateDoc(ptr);

   if (!rc)
      return 0;
   if (*xsink)
      return -1;

   if (rc < 0)
      xsink->raiseException("XSD-INTERNAL-ERROR", "an internal error occured validating the document against the XSD schema passed; xmlSchemaValidateDoc() returned %d", rc);
   else if (rc)
      xsink->raiseException("XSD-ERROR", "The document failed XSD validation", rc);
   return -1;
}
#endif

QoreXmlNodeData *QoreXmlDocData::getRootElement() {
   xmlNodePtr n = xmlDocGetRootElement(ptr);
   if (!n) return 0;
   return new QoreXmlNodeData(n, this);
}

QoreStringNode *doString(xmlChar *str) {
   if (!str)
      return 0;
   QoreStringNode *rv = new QoreStringNode((const char *)str);
   xmlFree(str);
   return rv;
}

QoreXmlNodeData *doNode(xmlNodePtr p, QoreXmlDocData *doc) {
   if (!p)
      return 0;
   return new QoreXmlNodeData(p, doc);
}

//! main %Qore Programming Language namespace
/**# namespace Qore {
//! namespace for XML parsing and utility classes
namespace Xml {
*/
//! The XmlDoc class provides access to a parsed XML document by wrapping a \c C \c xmlDocPtr from <a href="http://xmlsoft.org">libxml2</a>
/** Currently this class provides read-only access to XML documents; it is possible that this restriction will be removed in future versions of the xml module.
 */
/**# class XmlDoc {
public:
*/

//! creates a new XmlDoc object from the hash value passed
/** @param $data the must have only one top-level key, as the XML string that will be used for the XmlDoc object will be created directly from the hash

    @par Example:
    @code my XmlDoc $xd($hash); @endcode
 */
//# constructor(hash $data) {}
static void XMLDOC_constructor_hash(QoreObject *self, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreHashNode *h = HARD_QORE_HASH(params, 0);
   SimpleRefHolder<QoreStringNode> xml(makeXMLString(QCS_UTF8, *h, false, xsink));
   if (!xml)
      return;
   SimpleRefHolder<QoreXmlDocData> xd(new QoreXmlDocData(*xml));
   if (!xd->isValid()) {
      xsink->raiseException("XMLDOC-CONSTRUCTOR-ERROR", "error parsing XML string");
      return;
   }

   self->setPrivate(CID_XMLDOC, xd.release());   
}

//! a new XmlDoc object from the XML string passed
/** @param $xml the XML string to use as a basis for the XmlDoc object 

    @par Example:
    @code my XmlDoc $xd($xml); @endcode
 */
//# constructor(string $xml) {}
static void XMLDOC_constructor_str(QoreObject *self, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *str = HARD_QORE_STRING(params, 0);
   SimpleRefHolder<QoreXmlDocData> xd(new QoreXmlDocData(str));
   if (!xd->isValid()) {
      xsink->raiseException("XMLDOC-CONSTRUCTOR-ERROR", "error parsing XML string");
      return;
   }

   self->setPrivate(CID_XMLDOC, xd.release());   
}

//! Returns a copy of the current object
/** @return a copy of the current object

    @par Example:
    @code my XmlDoc $xdcopy = $xd.copy(); @endcode
*/
//# Qore::Xml::XmlDoc copy() {}
static void XMLDOC_copy(QoreObject *self, QoreObject *old, QoreXmlDocData *xd, ExceptionSink *xsink) {
   self->setPrivate(CID_XMLDOC, new QoreXmlDocData(*xd));
}

//! Returns the XML version of the contained XML document
/** @return the XML version of the contained XML document (normally \c "1.0")
    
    @par Example:
    @code my string $xmlver = $xd.getVersion(); @endcode
 */
//# string getVersion() {}
static AbstractQoreNode *XMLDOC_getVersion(QoreObject *self, QoreXmlDocData *xd, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreStringNode(xd->getVersion());
}

//! Returns a hash corresponding to the data contained in the XML document with out-of-order keys preserved by appending a suffix to hash keys
/** Returns a hash structure correponding to the XML data contained by the XmlDoc object.  If duplicate, out-of-order XML elements are found in the input string, they are deserialized to hash elements with the same name as the XML element but including a caret \c '^' and a numeric prefix to maintain the same key order in the hash as in the input XML string.

    For a similar method not preserving the order of keys in the XML in the resulting hash by collapsing all elements at the same level with the same name to the same list, see XmlDoc::toQoreData(). 

    @return a hash corresponding to the data contained in the XML document with out-of-order keys preserved by appending a suffix to hash keys

    @throw PARSE-XML-EXCEPTION error parsing XML string

    @par Example:
    @code my hash $h = $xd.toQore(); @endcode

    @see parseXMLAsData() and parseXML()
 */
//# hash toQore() {}
static AbstractQoreNode *XMLDOC_toQore(QoreObject *self, QoreXmlDocData *xd, const QoreListNode *params, ExceptionSink *xsink) {
   QoreXmlReader reader(xd->getDocPtr(), xsink);
   if (*xsink)
      return 0;
   return reader.parseXMLData(QCS_UTF8, false, xsink);
}

//! Returns a Qore hash corresponding to the data contained in the XML document; out-of-order keys are not preserved but are instead collapsed to the same Qore list
/** Returns a Qore hash structure corresponding to the XML data contained by the XmlDoc object; does not preserve hash order with out-of-order duplicate keys: collapses all to the same list.

    Note that data deserialized with this function may not be reserialized to the same input XML string due to the fact that duplicate, out-of-order XML elements are collapsed into lists in the resulting Qore hash, thereby losing the order in the original XML string.

    For a similar method preserving the order of keys in the XML in the resulting Qore hash by generating Qore hash element names with numeric suffixes, see XmlDoc::toQore().

    @return a Qore hash corresponding to the data contained in the XML document; out-of-order keys are not preserved but are instead collapsed to the same Qore list

    @throw PARSE-XML-EXCEPTION error parsing XML string

    @par Example:
    @code my hash $h = $xd.toQoreData(); @endcode

    @see parseXMLAsData() and parseXML()
 */
//# hash toQoreData() {}
static AbstractQoreNode *XMLDOC_toQoreData(QoreObject *self, QoreXmlDocData *xd, const QoreListNode *params, ExceptionSink *xsink) {
   QoreXmlReader reader(xd->getDocPtr(), xsink);
   if (*xsink)
      return 0;
   return reader.parseXMLData(QCS_UTF8, true, xsink);
}

//! Returns the XML string for the XmlDoc object
/** @return the XML string for the XmlDoc object
    @throw XML-DOC-TOSTRING-ERROR libxml2 reported an error while attempting to export the XmlDoc object's contents as an XML string

    @par Example:
    @code my string $xml = $xd.toString(); @endcode
*/
//# string toString() {}
static AbstractQoreNode *XMLDOC_toString(QoreObject *self, QoreXmlDocData *xd, const QoreListNode *params, ExceptionSink *xsink) {
   return xd->toString(xsink);
}

//! Evaluates an <a href="http://www.w3.org/TR/xpath">XPath</a> expression and returns a list of matching XmlNode objects.
/** @param $xpath the <a href="http://www.w3.org/TR/xpath">XPath</a> expression to evaluate against the XmlDoc object

    @return a list of XmlNode object matching the <a href="http://www.w3.org/TR/xpath">XPath</a> expression passed as an argument

    @throw XPATH-CONSTRUCTOR-ERROR cannot create XPath context from the XmlDoc object (ex: syntax error in xpath string)
    @throw XPATH-ERROR an error occured evaluating the XPath expression

    @par Example:
    @code my list $list = $xd.evalXPath("//list[2]"); @endcode
 */
//# list evalXPath(string $xpath) {}
static AbstractQoreNode *XMLDOC_evalXPath(QoreObject *self, QoreXmlDocData *xd, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *expr = HARD_QORE_STRING(params, 0);
   QoreXPath xp(xd, xsink);
   if (!xp)
      return 0;

   return xp.eval(expr->getBuffer(), xsink);
}

//! Returns an XmlNode object representing the root element of the document, if any exists, otherwise returns \c NOTHING
/** @return an XmlNode object representing the root element of the document, if any exists, otherwise returns \c NOTHING
    
    @par Example:
    @code my *XmlNode $xn = $xd.getRootElement(); @endcode
 */
//# *XmlNode getRootElement() {}
static AbstractQoreNode *XMLDOC_getRootElement(QoreObject *self, QoreXmlDocData *xd, const QoreListNode *params, ExceptionSink *xsink) {
   QoreXmlNodeData *n = xd->getRootElement();
   if (!n) return 0;
   return new QoreObject(QC_XMLNODE, getProgram(), n);
}

//! Validates the XML document against a RelaxNG schema; if any errors occur, exceptions are thrown
/** The availability of this function depends on the presence of libxml2's \c xmlTextReaderRelaxNGSetSchema() function when this module was compiled; for maximum portability check the constant @ref optionconstants "HAVE_PARSEXMLWITHRELAXNG" before running this method.
    @param $relaxng the RelaxNG schema to use to validate the XmlDoc object

    @throw MISSING-FEATURE-ERROR this exception is thrown when the function is not available; for maximum portability, check the constant @ref optionconstants "HAVE_PARSEXMLWITHRELAXNG" before calling this function
    @throw RELAXNG-SYNTAX-ERROR invalid RelaxNG string
    @throw RELAXNG-INTERNAL-ERROR libxml2 returned an internal error code while validating the document against the RelaxNG schema
    @throw RELAXNG-ERROR The document failed RelaxNG validation

    @par Example:
    @code $xd.validateRelaxNG($relaxng); @endcode    
 */
//# nothing validateRelaxNG(string $relaxng) {}
static AbstractQoreNode *XMLDOC_validateRelaxNG(QoreObject *self, QoreXmlDocData *xd, const QoreListNode *params, ExceptionSink *xsink) {
#ifdef HAVE_XMLTEXTREADERRELAXNGSETSCHEMA
   const QoreStringNode *rng = HARD_QORE_STRING(params, 0);
   // convert to UTF-8 
   TempEncodingHelper nrng(rng, QCS_UTF8, xsink);
   if (!nrng)
      return 0;

   xd->validateRelaxNG(nrng->getBuffer(), nrng->strlen(), xsink);
#else
   xsink->raiseException("MISSING-FEATURE-ERROR", "the libxml2 version used to compile the xml module did not support the xmlTextReaderRelaxNGValidate() function, therefore XmlDoc::validateRelaxNG() is not available; for maximum portability, use the constant Option::HAVE_PARSEXMLWITHRELAXNG to check if this method is implemented before calling");
#endif
   return 0;
}

//! Validates the XML document against an XSD schema; if any errors occur, exceptions are thrown
/** The availability of this function depends on the presence of libxml2's \c xmlTextReaderSetSchema() function when this module was compiled; for maximum portability check the constant @ref optionconstants HAVE_PARSEXMLWITHSCHEMA before running this function

    @param $xsd the XSD schema to use to validate the XmlDoc object

    @throw MISSING-FEATURE-ERROR this exception is thrown when the function is not available; for maximum portability, check the constant @ref optionconstants "HAVE_PARSEXMLWITHSCHEMA" before calling this function
    @throw XSD-SYNTAX-ERROR the RelaxNG schema string could not be parsed
    @throw XSD-INTERNAL-ERROR libxml2 returned an internal error code while validating the document against the XSD schema
    @throw XSD-ERROR The document failed XSD validation

    @par Example:
    @code $xd.validateSchema($xsd); @endcode    
 */
//# nothing validateSchema(string $xsd) {}
static AbstractQoreNode *XMLDOC_validateSchema(QoreObject *self, QoreXmlDocData *xd, const QoreListNode *params, ExceptionSink *xsink) {
#ifdef HAVE_XMLTEXTREADERSETSCHEMA
   const QoreStringNode *xsd = HARD_QORE_STRING(params, 0);
   // convert to UTF-8 
   TempEncodingHelper nxsd(xsd, QCS_UTF8, xsink);
   if (!nxsd)
      return 0;

   xd->validateSchema(nxsd->getBuffer(), nxsd->strlen(), xsink);
#else
   xsink->raiseException("MISSING-FEATURE-ERROR", "the libxml2 version used to compile the xml module did not support the xmlTextReaderSchemaValidate() function, therefore XmlDoc::validateSchema() is not available; for maximum portability, use the constant Option::HAVE_PARSEXMLWITHSCHEMA to check if this method is implemented before calling");
#endif
   return 0;
}

/**#
};
};
};
 */

QoreClass *initXmlDocClass(const QoreClass *QC_XmlNode) {
   QORE_TRACE("initXmlDocClass()");

   QoreClass *QC_XMLDOC = new QoreClass("XmlDoc");
   CID_XMLDOC = QC_XMLDOC->getID();

   QC_XMLDOC->setConstructorExtended(XMLDOC_constructor_hash, false, QC_NO_FLAGS, QDOM_DEFAULT, 1, hashTypeInfo, QORE_PARAM_NO_ARG);
   QC_XMLDOC->setConstructorExtended(XMLDOC_constructor_str, false, QC_NO_FLAGS, QDOM_DEFAULT, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   QC_XMLDOC->setCopy((q_copy_t)XMLDOC_copy);

   QC_XMLDOC->addMethodExtended("getVersion",      (q_method_t)XMLDOC_getVersion, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringTypeInfo);

   QC_XMLDOC->addMethodExtended("toQore",          (q_method_t)XMLDOC_toQore, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, hashTypeInfo);

   QC_XMLDOC->addMethodExtended("toQoreData",      (q_method_t)XMLDOC_toQoreData, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, hashTypeInfo);

   QC_XMLDOC->addMethodExtended("toString",        (q_method_t)XMLDOC_toString, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringTypeInfo);
   QC_XMLDOC->addMethodExtended("evalXPath",       (q_method_t)XMLDOC_evalXPath, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, listTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   // *XmlNode XmlDoc::getRootElement()  
   QC_XMLDOC->addMethodExtended("getRootElement",  (q_method_t)XMLDOC_getRootElement, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, QC_XmlNode->getOrNothingTypeInfo());

   QC_XMLDOC->addMethodExtended("validateRelaxNG", (q_method_t)XMLDOC_validateRelaxNG, false, QC_NO_FLAGS, QDOM_DEFAULT, nothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);
   QC_XMLDOC->addMethodExtended("validateSchema",  (q_method_t)XMLDOC_validateSchema, false, QC_NO_FLAGS, QDOM_DEFAULT, nothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   return QC_XMLDOC;   
}

