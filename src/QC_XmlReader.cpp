/*
  QC_XmlReader.cpp

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

//! @file XmlReader.qc defines the XmlReader class

#include "qore-xml-module.h"

#include "QC_XmlReader.h"
#include "QC_XmlNode.h"
#include "ql_xml.h"

qore_classid_t CID_XMLREADER;

//! main Qore Programming Language namespace
/** main Qore Programming Language namespace
 */
/**# namespace Qore {
//! namespace for XML parsing and utility classes
namespace Xml {
*/
//! The XmlReader class allows XML strings to be iterated and parsed piecewise
/** 
 */
/**# class XmlReader {
public:
   constructor(XmlDoc $doc);
   constructor(string $xml);
   XmlReader copy();
   bool read();
   bool readSkipWhitespace();
   int nodeType();
   *string nodeTypeName();
   int depth();
   *string name();
   *string value();
   bool hasAttributes();
   bool hasValue();
   bool isDefault();
   bool isEmptyElement();
   bool isNamespaceDecl();
   bool isValid();
   any toQore();
   any toQoreData();
   int attributeCount();
   *string baseUri();
   *string encoding();
   *string localName();
   *string namespaceUri();
   *string prefix();
   *string xmlLang();
   *string xmlVersion();
   *string getAttribute(string $attr);
   *string getAttributeNs(string $attr, string $ns);
   *string getAttributeOffset(softint $offset = 0);
   *string lookupNamespace();
   *string lookupNamespace(string $ns);
   bool moveToAttribute(string $attr);
   bool moveToAttributeOffset(softint $offset);
   bool moveToAttributeNs(string $attr, string $ns);
   bool moveToElement();
   bool moveToFirstAttribute();
   bool moveToNextAttribute();
   bool next();
   *string getInnerXml();
   *string getOuterXml();
   nothing relaxNGValidate(string $relaxng);
   nothing schemaValidate(string $xsd);
};
};
};
 */

//! cannot be called manually; throws an exception
/** @throw XML-READER-ERROR libxml2 returned an error code when creating the XML reader object
    
    @par Example:
    @code my XmlReader $xr($xmldoc); @endcode
*/
//# Qore::Xml::XmlReader::constructor(XmlDoc $doc) {}
static void XMLREADER_constructor_xmldoc(QoreObject *self, const QoreListNode *params, ExceptionSink *xsink) {
   HARD_QORE_OBJ_DATA(doc, QoreXmlDocData, params, 0, CID_XMLDOC, "XmlReader::constructor()", "XmlDoc", xsink);
   if (*xsink)
      return;

   ReferenceHolder<QoreXmlDocData> doc_holder(doc, xsink);

   SimpleRefHolder<QoreXmlReaderData> xr(new QoreXmlReaderData(doc, xsink));
   if (*xsink)
      return;

   self->setPrivate(CID_XMLREADER, xr.release());   
}

//! cannot be called manually; throws an exception
/** @throw XML-READER-ERROR libxml2 returned an error code when creating the XML reader object (ex: XML string could not be parsed)
    
    @par Example:
    @code my XmlReader $xr($xmlstr); @endcode
*/
//# Qore::Xml::XmlReader::constructor(string $xml) {}
static void XMLREADER_constructor_str(QoreObject *self, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *p0 = HARD_QORE_STRING(params, 0);

   if (p0->getEncoding() == QCS_UTF8)
      p0->ref();
   else {
      p0 = p0->convertEncoding(QCS_UTF8, xsink);
      if (!p0)
	 return;
   }

   SimpleRefHolder<QoreXmlReaderData> xr(new QoreXmlReaderData(const_cast<QoreStringNode *>(p0), xsink));
   if (*xsink)
      return;

   self->setPrivate(CID_XMLREADER, xr.release());
}

//! Creates an independent copy of the XmlReader object
/** @return an independent copy of the XmlReader object
    
    @par Example:
    @code my XmlReader $xrcopy = $xr.copy(); @endcode
*/
//# Qore::Xml::XmlReader Qore::Xml::XmlReader::copy() {}
static void XMLREADER_copy(QoreObject *self, QoreObject *old, QoreXmlReaderData *xr, ExceptionSink *xsink) {
   ReferenceHolder<QoreXmlReaderData> doc(xr->copy(xsink), xsink);
   if (!*xsink)
      self->setPrivate(CID_XMLREADER, doc.release());
}

//! Moves the position of the current instance to the next node in the stream
/** If an error occurs parsing the XML string, an exception is raised

    @return \c True if the read was successful, \c False if there are no more nodes to read

    @throw PARSE-XML-EXCEPTION cannot move to next node due to an error parsing the XML string (exception description string contains details about the error)
    
    @par Example:
    @code if (!$xr.read())
        break; @endcode

    @see XmlReader::readSkipWhitespace()
*/
//# bool Qore::Xml::XmlReader::read() {}
static AbstractQoreNode *XMLREADER_read(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   int rc = xr->read(xsink);
   return rc == -1 ? 0 : get_bool_node(rc);
}

//! the position of the current instance to the next node in the stream, skipping any whitespace nodes
/** If an error occurs parsing the XML string, an exception is raised

    @return \c True if the read was successful, \c False if there are no more nodes to read

    @throw PARSE-XML-EXCEPTION cannot move to next node due to an error parsing the XML string (exception description string contains details about the error)
    
    @par Example:
    @code if (!$xr.readSkipWhitespace())
        break; @endcode

    @see XmlReader::read()
*/
//# bool Qore::Xml::XmlReader::readSkipWhitespace() {}
static AbstractQoreNode *XMLREADER_readSkipWhitespace(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   int rc = xr->readSkipWhitespace(xsink);
   return rc == -1 ? 0 : get_bool_node(rc);
}

//! Returns the node type of the current node
/** @return the node type of the current node; for return values, see @ref XmlNodeTypes

    @par Example:
    @code my int $nt = $xr.nodeType(); @endcode

    @see @ref NodeTypeMap
    @see XmlReader::nodeTypeName()
*/
//# int Qore::Xml::XmlReader::nodeType() {}
static AbstractQoreNode *XMLREADER_nodeType(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xr->nodeType());
}

//! Returns a string giving the node type of the current node or \c NOTHING if no current node is available
/** @return a string giving the node type of the current node or \c NOTHING if no current node is available; for possible return values, see the values of the @ref NodeTypeMap

    @par Example:
    @code my *string $value = $xmlreader.nodeTypeName(); @endcode

    @see XmlReader::nodeType()
*/
//# *string Qore::Xml::XmlReader::nodeTypeName() {}
static AbstractQoreNode *XMLREADER_nodeTypeName(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = get_xml_node_type_name(xr->nodeType());
   return n ? new QoreStringNode(n) : 0;
}

//! Returns the depth of the node in the tree
/** @return the depth of the node in the tree
    
    @par Example:
    @code my int $depth = $xr.depth(); @endcode
*/
//# int Qore::Xml::XmlReader::depth() {}
static AbstractQoreNode *XMLREADER_depth(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xr->depth());
}

//! Returns the qualified name of the node (\c prefix:LocalName) or \c NOTHING if no name is available
/** @return the qualified name of the node (\c prefix:LocalName) or \c NOTHING if no name is available
    
    @par Example:
    @code my *string $value = $xmlreader.name(); @endcode

    @see XmlReader::localName()
*/
//# *string Qore::Xml::XmlReader::name() {}
static AbstractQoreNode *XMLREADER_name(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = xr->constName();
   return n ? new QoreStringNode(n, QCS_UTF8) : 0;
}

//! Returns the text value of the node or \c NOTHING if not available
/** @return the text value of the node or \c NOTHING if not available
    
    @par Example:
    @code my *string $val = $xr.value(); @endcode
 */
//# *string Qore::Xml::XmlReader::value() {}
static AbstractQoreNode *XMLREADER_value(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = xr->constValue();
   return n ? new QoreStringNode(n, QCS_UTF8) : 0;
}

//! Returns \c True if the node has attributes or \c False if not
/** @return \c True if the node has attributes or \c False if not
    
    @par Example:
    @code my bool $b = $xr.hasAttributes(); @endcode
 */
//# bool Qore::Xml::XmlReader::hasAttributes() {}
static AbstractQoreNode *XMLREADER_hasAttributes(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xr->hasAttributes());
}

//! Returns \c True if the node has a text value or \c False if not
/** @return \c True if the node has a text value or \c False if not
    
    @par Example:
    @code my bool $b = $xr.hasValue(); @endcode
 */
//# bool Qore::Xml::XmlReader::hasValue() {}
static AbstractQoreNode *XMLREADER_hasValue(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xr->hasValue());
}

//! Returns \c True if an attribute node was generated from the default value defined in the DTD or schema, \c False if not
/** @return \c True if an attribute node was generated from the default value defined in the DTD or schema, \c False if not
    
    @par Example:
    @code my bool $b = $xr.isDefault(); @endcode
 */
//# bool Qore::Xml::XmlReader::isDefault() {}
static AbstractQoreNode *XMLREADER_isDefault(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xr->isDefault());
}

//! Returns \c True if the current node is empty or \c False if not
/** @return \c True if the current node is empty or \c False if not
    
    @par Example:
    @code my bool $b = $xr.isEmptyElement(); @endcode
 */
//# bool Qore::Xml::XmlReader::isEmptyElement() {}
static AbstractQoreNode *XMLREADER_isEmptyElement(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xr->isEmptyElement());
}

//! Returns \c True if the current node is a namespace declaration rather than a regular attribute or \c False if not
/** @return \c True if the current node is a namespace declaration rather than a regular attribute or \c False if not
    
    @par Example:
    @code my bool $b = $xr.isNamespaceDecl(); @endcode
 */
//# bool Qore::Xml::XmlReader::isNamespaceDecl() {}
static AbstractQoreNode *XMLREADER_isNamespaceDecl(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xr->isNamespaceDecl());
}

//! Returns \c True if the current reader parser context is valid, \c False if not
/** @return \c True if the current reader parser context is valid, \c False if not
    
    @par Example:
    @code my bool $b = $xr.isValid(); @endcode
 */
//# bool Qore::Xml::XmlReader::isValid() {}
static AbstractQoreNode *XMLREADER_isValid(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xr->isValid());
}

//! Returns Qore data corresponding to the XML starting at the current node position, maintains element order by mangling node names
/** If duplicate, out-of-order XML elements are found in the input string, they are deserialized to hash elements with the same name as the XML element but including a caret "^" and a numeric prefix to maintain the same key order in the hash as in the input XML string.

    @return If there are sub elements, a hash of the XML is returned, the sub elements representing the current node's children.  If only text is present at the current element, a string is returned. If no information is available, then \c NOTHING is returned.

    @note This method is functionally similar to parseXML()
    
    @par Example:
    @code my any $data = $xr.toQore(); @endcode

    @see XmlReader::toQoreData()
 */
//# any Qore::Xml::XmlReader::toQore() {}
static AbstractQoreNode *XMLREADER_toQore(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return xr->parseXMLData(QCS_UTF8, false, xsink);
}

//! Returns Qore data corresponding to the XML starting at the current node position, collapses duplicate out of order elements to a list
/** Data deserialized with this method may not be reserialized to an identical XML string due to the fact that XML elements with the same name are collapsed into Qore lists in the resulting Qore hash irrespective of the order in the original XML string.

    @return If there are sub elements, a hash of the XML is returned, the sub elements representing the current node's children.  If only text is present at the current element, a string is returned.  If no information is available, then \c NOTHING is returned.
    
    @note This method is functionally similar to parseXMLAsData()

    @par Example:
    @code my any $data = $xr.toQoreData(); @endcode

    @see XmlReader::toQore()
 */
//# any Qore::Xml::XmlReader::toQoreData() {}
static AbstractQoreNode *XMLREADER_toQoreData(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return xr->parseXMLData(QCS_UTF8, true, xsink);
}

//! Returns the number of attributes of the current node
/** @return the number of attributes of the current node
    
    @par Example:
    @code my int $n = $xr.attributeCount(); @endcode
 */
//# int Qore::Xml::XmlReader::attributeCount() {}
static AbstractQoreNode *XMLREADER_attributeCount(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xr->attributeCount());
}

//! Returns the base URI of the node if known, \c NOTHING if not.
/** @return the base URI of the node if known, \c NOTHING if not.
    
    @par Example:
    @code my *string $value = $xmlreader.baseUri(); @endcode
 */
//# *string Qore::Xml::XmlReader::baseUri() {}
static AbstractQoreNode *XMLREADER_baseUri(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = xr->baseUri();
   return n ? new QoreStringNode(n, QCS_UTF8) : 0;
}

//! Returns the encoding string given in the XML string or \c NOTHING if none is given
/** @return the encoding string given in the XML string or \c NOTHING if none is given
    
    @par Example:
    @code my *string $value = $xmlreader.encoding(); @endcode
 */
//# *string Qore::Xml::XmlReader::encoding() {}
static AbstractQoreNode *XMLREADER_encoding(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = xr->encoding();
   return n ? new QoreStringNode(n, QCS_UTF8) : 0;
}

//! Returns the local name of the node or \c NOTHING if no name is available
/** @return the local name of the node or \c NOTHING if no name is available
    
    @par Example:
    @code my *string $value = $xmlreader.localName(); @endcode
 */
//# *string Qore::Xml::XmlReader::localName() {}
static AbstractQoreNode *XMLREADER_localName(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = xr->localName();
   return n ? new QoreStringNode(n, QCS_UTF8) : 0;
}

//! Returns the URI defining the namespace associated with the node, or \c NOTHING if not available
/** @return the URI defining the namespace associated with the node, or \c NOTHING if not available
    
    @par Example:
    @code my *string $value = $xmlreader.namespaceUri(); @endcode
 */
//# *string Qore::Xml::XmlReader::namespaceUri() {}
static AbstractQoreNode *XMLREADER_namespaceUri(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = xr->namespaceUri();
   return n ? new QoreStringNode(n, QCS_UTF8) : 0;
}

//! Returns the shorthand reference to the namespace associated with the node, or \c NOTHING if not available
/** @return the shorthand reference to the namespace associated with the node, or \c NOTHING if not available
    
    @par Example:
    @code my *string $value = $xmlreader.prefix(); @endcode
 */
//# *string Qore::Xml::XmlReader::prefix() {}
static AbstractQoreNode *XMLREADER_prefix(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = xr->prefix();
   return n ? new QoreStringNode(n, QCS_UTF8) : 0;
}

//! Returns the \c xml:lang scope within which the node resides or \c NOTHING if there is none
/** @return the \c xml:lang scope within which the node resides or \c NOTHING if there is none
    
    @par Example:
    @code my *string $value = $xmlreader.xmlLang(); @endcode
 */
//# *string Qore::Xml::XmlReader::xmlLang() {}
static AbstractQoreNode *XMLREADER_xmlLang(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = xr->xmlLang();
   return n ? new QoreStringNode(n, QCS_UTF8) : 0;
}

//! Returns a string giving the XML version of the source document (normally \c "1.0") or \c NOTHING if none is present
/** @return a string giving the XML version of the source document (normally \c "1.0") or \c NOTHING if none is present
    
    @par Example:
    @code my *string $value = $xmlreader.xmlVersion(); @endcode
 */
//# *string Qore::Xml::XmlReader::xmlVersion() {}
static AbstractQoreNode *XMLREADER_xmlVersion(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const char *n = xr->xmlVersion();
   return n ? new QoreStringNode(n, QCS_UTF8) : 0;
}

//! Returns the value of the attribute matching the qualified name passed, or \c NOTHING if no such attribute exists in the current XmlReader
/** @param $attr the name of the attribute to retrieve

    @return the value of the attribute matching the qualified name passed, or \c NOTHING if no such attribute exists in the current XmlReader
    
    @par Example:
    @code my *string $value = $xmlreader.getAttribute($name); @endcode

    @see XmlReader::getAttributeNs()
 */
//# *string Qore::Xml::XmlReader::getAttribute(string $attr) {}
static AbstractQoreNode *XMLREADER_getAttribute(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *attr = HARD_QORE_STRING(params, 0);
   return xr->getAttribute(attr->getBuffer());
}

//! Returns the value of the attribute with the specified index relative to the containing element, or \c NOTHING if no such attribute exists in the current XmlReader
/** @param $offset the attribute number of the attribute to retrieve relative to the containing element starting from 0

    @return the value of the attribute with the specified index relative to the containing element, or \c NOTHING if no such attribute exists in the current XmlReader
    
    @par Example:
    @code my *string $value = $xmlreader.getAttributeOffset($offset); @endcode

    @see XmlReader::getAttribute()
 */
//# *string Qore::Xml::XmlReader::getAttributeOffset(softint $offset = 0) {}
static AbstractQoreNode *XMLREADER_getAttributeOffset(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return xr->getAttributeOffset((int)HARD_QORE_INT(params, 0));
}

//! Returns the value of the given attribute anchored in the given namespace, or \c NOTHING if no such attribute exists in the current XmlReader
/** @param $attr the name of the attribute to retrieve
    @param $ns the namespace URI of the attribute
    
    @return the value of the given attribute anchored in the given namespace, or \c NOTHING if no such attribute exists in the current XmlReader
    
    @par Example:
    @code my *string $value = $xmlreader.getAttributeNs($localname, $namespaceuri); @endcode

    @see XmlReader::getAttribute()
 */
//# *string Qore::Xml::XmlReader::getAttributeNs(string $attr, string $ns) {}
static AbstractQoreNode *XMLREADER_getAttributeNs(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *lname = HARD_QORE_STRING(params, 0);
   const QoreStringNode *ns = HARD_QORE_STRING(params, 1);
   return xr->getAttributeNs(lname->getBuffer(), ns->getBuffer());
}

//! Returns the default namespace in the scope of the current element, or \c NOTHING if none exists
/** @return the default namespace in the scope of the current element, or \c NOTHING if none exists

    @par Example:
    @code my *string $ns = $xr.lookupNamespace(); @endcode
 */
//# *string Qore::Xml::XmlReader::lookupNamespace() {}
static AbstractQoreNode *XMLREADER_lookupNamespace(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return xr->lookupNamespace(0);
}

//! Returns the namespace corresponding to the given prefix in the scope of the current element, or \c NOTHING if none exists
/** @param $prefix the namespace prefix to resolve

    @return the namespace corresponding to the given prefix in the scope of the current element, or \c NOTHING if none exists

    @par Example:
    @code my *string $value = $xmlreader.lookupNamespace($prefix); @endcode
 */
//# *string Qore::Xml::XmlReader::lookupNamespace(string $prefix) {}
static AbstractQoreNode *XMLREADER_lookupNamespace_str(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *prefix = HARD_QORE_STRING(params, 0);
   return xr->lookupNamespace(prefix->getBuffer());
}

//! Moves the position of the current instance to the attribute with the specified qualified name
/** If an XML parsing error occurs, an exception is thrown

    @param $attr the qualified name of the attribute to move to

    @return \c True in case of success, \c False if not found

    @throw PARSE-XML-EXCEPTION error parsing XML
    
    @par Example:
    @code my bool $b = $xmlreader.moveToAttribute($name); @endcode

    @see XmlReader::moveToAttributeNs()
 */
//# bool Qore::Xml::XmlReader::moveToAttribute(string $attr) {}
static AbstractQoreNode *XMLREADER_moveToAttribute(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *attr = HARD_QORE_STRING(params, 0);
   int rc = xr->moveToAttribute(attr->getBuffer(), xsink);
   return rc == -1 ? 0 : get_bool_node(rc);
}

//! Moves the position of the current instance to the attribute with the specified index relative to the containing element
/** If an XML parsing error occurs, an exception is thrown

    @param $offset the index of the attribute relative to the containing element to move to starting with 0

    @return \c True in case of success, \c False if not found

    @throw PARSE-XML-EXCEPTION error parsing XML
    
    @par Example:
    @code my bool $b = $xmlreader.moveToAttributeOffset($offset); @endcode

    @see XmlReader::moveToAttribute()
 */
//# bool Qore::Xml::XmlReader::moveToAttributeOffset(softint $offset) {}
static AbstractQoreNode *XMLREADER_moveToAttributeOffset(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   int rc = xr->moveToAttributeOffset((int)HARD_QORE_INT(params, 0), xsink);
   return rc == -1 ? 0 : get_bool_node(rc);
}

//! Moves the position of the current instance to the attribute with the specified local name and namespace URI
/** If an XML parsing error occurs, an exception is thrown

    @param $attr the qualified name of the attribute to move to
    @param $ns the namespace URI of the attribute
    
    @return \c True in case of success, \c False if not found

    @throw PARSE-XML-EXCEPTION error parsing XML
    
    @par Example:
    @code my bool $b = $xmlreader.moveToAttributeNs($localname, $namespaceuri); @endcode

    @see XmlReader::moveToAttribute()
 */
//# bool Qore::Xml::XmlReader::moveToAttributeNs(string $attr, string $ns) {}
static AbstractQoreNode *XMLREADER_moveToAttributeNs(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *lname = HARD_QORE_STRING(params, 0);
   const QoreStringNode *ns = HARD_QORE_STRING(params, 1);

   int rc = xr->moveToAttributeNs(lname->getBuffer(), ns->getBuffer(), xsink);
   return rc == -1 ? 0 : get_bool_node(rc);
}

//! Moves the position of the current instance to the element node containing the current attribute node
/** If an XML parsing error occurs, an exception is thrown

    @return \c True in case of success, \c False if not found
    
    @throw PARSE-XML-EXCEPTION error parsing XML
    
    @par Example:
    @code my bool $b = $xmlreader.moveToElement(); @endcode
 */
//# bool Qore::Xml::XmlReader::moveToElement() {}
static AbstractQoreNode *XMLREADER_moveToElement(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   int rc = xr->moveToElement(xsink);
   return rc == -1 ? 0 : get_bool_node(rc);
}

//! Moves the position of the current instance to the first attribute of the current node
/** If an XML parsing error occurs, an exception is thrown
    
    @return \c True in case of success, \c False if not found

    @throw PARSE-XML-EXCEPTION error parsing XML
    
    @par Example:
    @code my bool $b = $xmlreader.moveToFirstAttribute(); @endcode
 */
//# bool Qore::Xml::XmlReader::moveToFirstAttribute() {}
static AbstractQoreNode *XMLREADER_moveToFirstAttribute(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   int rc = xr->moveToFirstAttribute(xsink);
   return rc == -1 ? 0 : get_bool_node(rc);
}

//! Moves the position of the current instance to the next attribute of the current node
/** If an XML parsing error occurs, an exception is thrown
    
    @return \c True in case of success, \c False if not found

    @throw PARSE-XML-EXCEPTION error parsing XML
    
    @par Example:
    @code my bool $b = $xmlreader.moveToNextAttribute(); @endcode
 */
//# bool Qore::Xml::XmlReader::moveToNextAttribute() {}
static AbstractQoreNode *XMLREADER_moveToNextAttribute(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   int rc = xr->moveToNextAttribute(xsink);
   return rc == -1 ? 0 : get_bool_node(rc);
}

//! Moves the position of the current instance to the next node in the tree at the same level, skipping any subtree
/** If an XML parsing error occurs, an exception is thrown
    
    @return \c True in case of success, \c False if not found

    @throw PARSE-XML-EXCEPTION error parsing XML
    
    @par Example:
    @code my bool $b = $xmlreader.next(); @endcode
 */
//# bool Qore::Xml::XmlReader::next() {}
static AbstractQoreNode *XMLREADER_next(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   int rc = xr->next(xsink);
   return rc == -1 ? 0 : get_bool_node(rc);
}

//! Returns an XML string of the contents of the all current node's child nodes and markup, or \c NOTHING if the current node is neither an element nor an attribute or has no child nodes
/** @return an XML string of the contents of the all current node's child nodes and markup, or \c NOTHING if the current node is neither an element nor an attribute or has no child nodes
    
    @par Example:
    @code my *string $value = $xmlreader.getInnerXml(); @endcode

    @see XmlReader::getOuterXml()
 */
//# *string Qore::Xml::XmlReader::getInnerXml() {}
static AbstractQoreNode *XMLREADER_getInnerXml(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return xr->getInnerXml();
}

//! Return an XML string of the contents of the current node and all child nodes and markup, or \c NOTHING if the current node is neither an element nor an attribute or has no child nodes 
/** @return an XML string of the contents of the current node and all child nodes and markup, or \c NOTHING if the current node is neither an element nor an attribute or has no child nodes
    
    @par Example:
    @code my *string $value = $xmlreader.getOuterXml(); @endcode

    @see XmlReader::getInnerXml()
 */
//# *string Qore::Xml::XmlReader::getOuterXml() {}
static AbstractQoreNode *XMLREADER_getOuterXml(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
   return xr->getOuterXml();
}

//! Set a RelaxNG schema for schema validation while parsing the XML document
/** This method must be called before the first call to XmlReader::read()

    The availability of this method depends on the presence of libxml2's \c xmlTextReaderRelaxNGValidate() function when the xml module was compiled; for maximum portability check the constant @ref optionconstants "HAVE_PARSEXMLWITHRELAXNG" before calling this method.

    @param $relaxng the RelaxNG schema string to use to validate the XML document

    @throw RELAXNG-SYNTAX-ERROR invalid RelaxNG string
    @throw MISSING-FEATURE-ERROR this exception is thrown when the function is not available; for maximum portability, check the constant @ref optionconstants "HAVE_PARSEXMLWITHRELAXNG" before calling this function

    @par Example:
    @code $xr.relaxNGValidate($relaxng); @endcode
 */
//# nothing Qore::Xml::XmlReader::relaxNGValidate(string $relaxng) {}
static AbstractQoreNode *XMLREADER_relaxNGValidate(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
#ifdef HAVE_XMLTEXTREADERRELAXNGSETSCHEMA
   const QoreStringNode *rng = HARD_QORE_STRING(params, 0);
   xr->relaxNGValidate(rng->getBuffer(), xsink);
#else
   xsink->raiseException("MISSING-FEATURE-ERROR", "the libxml2 version used to compile the qore library did not support the xmlTextReaderRelaxNGValidate() function, therefore XmlReader::relaxNGValidate() is not available in Qore; for maximum portability, use the constant Option::HAVE_PARSEXMLWITHRELAXNG to check if this method is implemented before calling");
#endif
   return 0;
}

//! Set an XSD schema for schema validation while parsing the XML document
/** This method must be called before the first call to XmlReader::read()

    The availability of this method depends on the presence of libxml2's \c xmlTextReaderSetSchema() function when the xml module was compiled; for maximum portability check the constant @ref optionconstants "HAVE_PARSEXMLWITHSCHEMA" before calling this method.

    @param $xsd the XSD schema string to use to validate the XML document

    @throw XSD-SYNTAX-ERROR invalid XSD string
    @throw MISSING-FEATURE-ERROR this exception is thrown when the function is not available; for maximum portability, check the constant @ref optionconstants "HAVE_PARSEXMLWITHSCHEMA" before calling this function

    @par Example:
    @code $xr.schemaValidate($xsd); @endcode
 */
//# nothing Qore::Xml::XmlReader::schemaValidate(string $xsd) {}
static AbstractQoreNode *XMLREADER_schemaValidate(QoreObject *self, QoreXmlReaderData *xr, const QoreListNode *params, ExceptionSink *xsink) {
#ifdef HAVE_XMLTEXTREADERSETSCHEMA
   const QoreStringNode *xsd = HARD_QORE_STRING(params, 0);
   xr->schemaValidate(xsd->getBuffer(), xsink);
#else
   xsink->raiseException("MISSING-FEATURE-ERROR", "the libxml2 version used to compile the qore library did not support the xmlTextReaderSchemaValidate() function, therefore XmlReader::schemaValidate() is not available in Qore; for maximum portability, use the constant Option::HAVE_PARSEXMLWITHSCHEMA to check if this method is implemented before calling");
#endif
   return 0;
}

QoreClass *initXmlReaderClass(QoreClass *XmlDoc) {
   QORE_TRACE("initXmlReaderClass()");

   QoreClass *QC_XMLREADER = new QoreClass("XmlReader");
   CID_XMLREADER = QC_XMLREADER->getID();

   QC_XMLREADER->setConstructorExtended(XMLREADER_constructor_xmldoc, false, QC_NO_FLAGS, QDOM_DEFAULT, 1, XmlDoc->getTypeInfo(), QORE_PARAM_NO_ARG);
   QC_XMLREADER->setConstructorExtended(XMLREADER_constructor_str, false, QC_NO_FLAGS, QDOM_DEFAULT, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   QC_XMLREADER->setCopy((q_copy_t)XMLREADER_copy);

   QC_XMLREADER->addMethodExtended("read",                      (q_method_t)XMLREADER_read, false, QC_NO_FLAGS, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("readSkipWhitespace",        (q_method_t)XMLREADER_readSkipWhitespace, false, QC_NO_FLAGS, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("nodeType",                  (q_method_t)XMLREADER_nodeType, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, bigIntTypeInfo);

   // *string XmlReader::nodeTypeName()  
   QC_XMLREADER->addMethodExtended("nodeTypeName",              (q_method_t)XMLREADER_nodeTypeName, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   QC_XMLREADER->addMethodExtended("depth",                     (q_method_t)XMLREADER_depth, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, bigIntTypeInfo);

   // *string XmlReader::name()  
   QC_XMLREADER->addMethodExtended("name",                      (q_method_t)XMLREADER_name, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlReader::value()  
   QC_XMLREADER->addMethodExtended("value",                     (q_method_t)XMLREADER_value, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   QC_XMLREADER->addMethodExtended("hasAttributes",             (q_method_t)XMLREADER_hasAttributes, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("hasValue",                  (q_method_t)XMLREADER_hasValue, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("isDefault",                 (q_method_t)XMLREADER_isDefault, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("isEmptyElement",            (q_method_t)XMLREADER_isEmptyElement, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("isNamespaceDecl",           (q_method_t)XMLREADER_isNamespaceDecl, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("isValid",                   (q_method_t)XMLREADER_isValid, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, boolTypeInfo);

   // hash XmlReader::toQore()  |string|nothing
   QC_XMLREADER->addMethodExtended("toQore",                    (q_method_t)XMLREADER_toQore, false, QC_RET_VALUE_ONLY);

   // hash XmlReader::toQoreData()  |string|nothing
   QC_XMLREADER->addMethodExtended("toQoreData",                (q_method_t)XMLREADER_toQoreData, false, QC_RET_VALUE_ONLY);

   QC_XMLREADER->addMethodExtended("attributeCount",            (q_method_t)XMLREADER_attributeCount, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, bigIntTypeInfo);

   // *string XmlReader::baseUri()  
   QC_XMLREADER->addMethodExtended("baseUri",                   (q_method_t)XMLREADER_baseUri, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlReader::encoding()  
   QC_XMLREADER->addMethodExtended("encoding",                  (q_method_t)XMLREADER_encoding, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlReader::localName()  
   QC_XMLREADER->addMethodExtended("localName",                 (q_method_t)XMLREADER_localName, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlReader::namespaceUri()  
   QC_XMLREADER->addMethodExtended("namespaceUri",              (q_method_t)XMLREADER_namespaceUri, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlReader::prefix()  
   QC_XMLREADER->addMethodExtended("prefix",                    (q_method_t)XMLREADER_prefix, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlReader::xmlLang()  
   QC_XMLREADER->addMethodExtended("xmlLang",                   (q_method_t)XMLREADER_xmlLang, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlReader::xmlVersion()  
   QC_XMLREADER->addMethodExtended("xmlVersion",                (q_method_t)XMLREADER_xmlVersion, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlReader::getAttribute(string $attr)  
   QC_XMLREADER->addMethodExtended("getAttribute",              (q_method_t)XMLREADER_getAttribute, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   // *string XmlReader::getAttribute(softint $offset = 0)  
   QC_XMLREADER->addMethodExtended("getAttributeOffset",        (q_method_t)XMLREADER_getAttributeOffset, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo, 1, softBigIntTypeInfo, zero());

   // *string XmlReader::getAttributeNs(string $attr, string $ns)  
   QC_XMLREADER->addMethodExtended("getAttributeNs",            (q_method_t)XMLREADER_getAttributeNs, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo, 2, stringTypeInfo, QORE_PARAM_NO_ARG, stringTypeInfo, QORE_PARAM_NO_ARG);

   // *string XmlReader::lookupNamespace()  
   QC_XMLREADER->addMethodExtended("lookupNamespace",           (q_method_t)XMLREADER_lookupNamespace, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);
   // *string XmlReader::lookupNamespace(string $ns)  
   QC_XMLREADER->addMethodExtended("lookupNamespace",           (q_method_t)XMLREADER_lookupNamespace_str, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   // bool XmlReader::moveToAttribute(string $attr)  
   QC_XMLREADER->addMethodExtended("moveToAttribute",           (q_method_t)XMLREADER_moveToAttribute, false, QC_NO_FLAGS, QDOM_DEFAULT, boolTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   // bool XmlReader::moveToAttributeOffset(softint $offset = 0)  
   QC_XMLREADER->addMethodExtended("moveToAttributeOffset",     (q_method_t)XMLREADER_moveToAttributeOffset, false, QC_NO_FLAGS, QDOM_DEFAULT, boolTypeInfo, 1, softBigIntTypeInfo, zero());

   // bool XmlReader::moveToAttributeNs(string $attr, string $ns)  
   QC_XMLREADER->addMethodExtended("moveToAttributeNs",         (q_method_t)XMLREADER_moveToAttributeNs, false, QC_NO_FLAGS, QDOM_DEFAULT, boolTypeInfo, 2, stringTypeInfo, QORE_PARAM_NO_ARG, stringTypeInfo, QORE_PARAM_NO_ARG);

   QC_XMLREADER->addMethodExtended("moveToElement",             (q_method_t)XMLREADER_moveToElement, false, QC_NO_FLAGS, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("moveToFirstAttribute",      (q_method_t)XMLREADER_moveToFirstAttribute, false, QC_NO_FLAGS, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("moveToNextAttribute",       (q_method_t)XMLREADER_moveToNextAttribute, false, QC_NO_FLAGS, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLREADER->addMethodExtended("next",                      (q_method_t)XMLREADER_next, false, QC_NO_FLAGS, QDOM_DEFAULT, boolTypeInfo);

   // *string XmlReader::getInnerXml()  
   QC_XMLREADER->addMethodExtended("getInnerXml",               (q_method_t)XMLREADER_getInnerXml, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlReader::getOuterXml()  
   QC_XMLREADER->addMethodExtended("getOuterXml",               (q_method_t)XMLREADER_getOuterXml, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // nothing XmlReader::relaxNGValidate()  
   QC_XMLREADER->addMethodExtended("relaxNGValidate",           (q_method_t)XMLREADER_relaxNGValidate, false, QC_NO_FLAGS, QDOM_DEFAULT, nothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   // nothing XmlReader::schemaValidate()  
   QC_XMLREADER->addMethodExtended("schemaValidate",            (q_method_t)XMLREADER_schemaValidate, false, QC_NO_FLAGS, QDOM_DEFAULT, nothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   return QC_XMLREADER;   
}
