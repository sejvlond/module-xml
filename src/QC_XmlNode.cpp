/*
 QC_XmlNode.cpp
 
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

//! @file XmlNode.qc defines the XmlNode class

#include "qore-xml-module.h"

qore_classid_t CID_XMLNODE;
QoreClass *QC_XMLNODE;

//! main Qore Programming Language namespace
/** main Qore Programming Language namespace
 */
/**# namespace Qore {
//! namespace for XML parsing and utility classes
namespace Xml {
*/
//! The XmlNode class provides information about the components of an XML document
/** This class currently cannot be constructed manually, but rather can only be returned by the XmlDoc class. It is possible that future versions of the xml module will remove this restriction.
 */
/**# class XmlNode {
public:
   constructor();
   XmlNode copy();
   int childElementCount();
   int getSpacePreserve();
   int getElementType();
   *string getElementTypeName();
   *XmlNode firstElementChild();
   *XmlNode getLastChild();
   *XmlNode lastElementChild();
   *XmlNode nextElementSibling();
   *XmlNode previousElementSibling();
   string getPath();
   *string getNsProp(string $prop, string $ns);
   *string getProp(string $prop);
   *string getContent();
   *string getName();
   *string getLang();
   bool isText();
   bool isBlank();
   *string getXML();
};
};
};
 */

//! cannot be called manually; throws an exception
/** @throw XMLNODE-CONSTRUCTOR-ERROR this class cannot be constructed directly
 */
//# Qore::Xml::XmlNode::constructor() {}
static void XMLNODE_constructor(QoreObject *self, const QoreListNode *params, ExceptionSink *xsink) {
   xsink->raiseException("XMLNODE-CONSTRUCTOR-ERROR", "this class cannot be constructed directly");
}

//! Returns a copy of the current object
/** @return a copy of the current object

    @par Example:
    @code my XmlNode $xncopy = $xn.copy(); @endcode
*/
//# Qore::Xml::XmlNode Qore::Xml::XmlNode::copy() {}
static void XMLNODE_copy(QoreObject *self, QoreObject *old, QoreXmlNodeData *xn, ExceptionSink *xsink) {
   self->setPrivate(CID_XMLNODE, new QoreXmlNodeData(*xn));
}

//! Returns the number of child elements of the XmlNode
/** @return the number of child elements of the XmlNode

    @par Example:
    @code my int $val = $n.childElementCount(); @endcode
*/
//# int Qore::Xml::XmlNode::childElementCount() {}
static AbstractQoreNode *XMLNODE_childElementCount(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xn->childElementCount());
}

//! Searches the space preserving behaviour of the XmlNode object, i.e. the values of the \c xml:space attribute or the one carried by the nearest ancestor
/** @return -1 if \c xml:space is not inherited, 0 if \c "default", 1 if \c "preserve"

    @par Example:
    @code my int $val = $n.getSpacePreserve(); @endcode
*/
//# int Qore::Xml::XmlNode::getSpacePreserve() {}
static AbstractQoreNode *XMLNODE_getSpacePreserve(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xn->getSpacePreserve());
}

// line numbers are not activated by default and no option is provided to activate them, therefore this
// function is a noop for now - not documented
static AbstractQoreNode *XMLNODE_getLineNumber(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xn->getLineNumber());
}

//! Returns the type of the XmlNode object
/** @return the element type as an integer; for possible values see @ref XMLElementTypes

    @par Example:
    @code my int $val = $n.getElementType(); @endcode
*/
//# int Qore::Xml::XmlNode::getElementType() {}
static AbstractQoreNode *XMLNODE_getElementType(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xn->getElementType());
}

//! Returns the name of the type of the XmlNode object
/** @return the name of the type of the XmlNode object; for possible values see the values of the @ref ElementTypeMap; if the type is unknown, then \c NOTHING is returned

    @par Example:
    @code my *string $name = $n.getElementTypeName(); @endcode
*/
//# *string Qore::Xml::XmlNode::getElementTypeName() {}
static AbstractQoreNode *XMLNODE_getElementTypeName(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   const char *nt = get_xml_element_type_name((int)xn->getElementType());
   return nt ? new QoreStringNode(nt) : 0;
}

static QoreObject *doObject(QoreXmlNodeData *data) {
   return data ? new QoreObject(QC_XMLNODE, getProgram(), data) : 0;
}

//! Returns an XmlNode object for the first child of the current XmlNode object that is an XML element, or \c NOTHING if there is none
/** @return an XmlNode object for the first child of the current XmlNode object that is an XML element, or \c NOTHING if there is none

    @par Example:
    @code my *XmlNode $xn = $node.firstElementChild(); @endcode
*/
//# *XmlNode Qore::Xml::XmlNode::firstElementChild() {}
static AbstractQoreNode *XMLNODE_firstElementChild(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->firstElementChild());
}

//! Returns an XmlNode object for the last child of the current XmlNode object, or \c NOTHING if there is none
/** @return an XmlNode object for the last child of the current XmlNode object, or \c NOTHING if there is none

    @par Example:
    @code my *XmlNode $xn = $node.getLastChild(); @endcode
*/
//# *XmlNode Qore::Xml::XmlNode::getLastChild();
static AbstractQoreNode *XMLNODE_getLastChild(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->getLastChild());
}

//! Returns an XmlNode object for the last child of the current XmlNode object that is an XML element, or \c NOTHING if there is none
/** @return an XmlNode object for the last child of the current XmlNode object that is an XML element, or \c NOTHING if there is none

    @par Example:
    @code my *XmlNode $xn = $node.lastElementChild(); @endcode
*/
//# *XmlNode Qore::Xml::XmlNode::lastElementChild();
static AbstractQoreNode *XMLNODE_lastElementChild(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->lastElementChild());
}

//! Returns an XmlNode object for the next element at the same level of the current XmlNode object, or \c NOTHING if there is none
/** @return an XmlNode object for the next element at the same level of the current XmlNode object, or \c NOTHING if there is none

    @par Example:
    @code my *XmlNode $xn = $node.nextElementSibling(); @endcode
*/
//# *XmlNode Qore::Xml::XmlNode::nextElementSibling();
static AbstractQoreNode *XMLNODE_nextElementSibling(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->nextElementSibling());
}

//! Returns an XmlNode object for the previous element at the same level of the current XmlNode object, or \c NOTHING if there is none
/** @return an XmlNode object for the previous element at the same level of the current XmlNode object, or \c NOTHING if there is none

    @par Example:
    @code my *XmlNode $xn = $node.previousElementSibling(); @endcode
*/
//# *XmlNode Qore::Xml::XmlNode::previousElementSibling();
static AbstractQoreNode *XMLNODE_previousElementSibling(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->previousElementSibling());
}

//! Returns a string representing a structured path for the current node
/** @return a string representing a structured path for the current node

    @par Example:
    @code my string $path = $node.getPath(); @endcode
*/
//# string Qore::Xml::XmlNode::getPath() {}
static AbstractQoreNode *XMLNODE_getPath(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getPath(xsink);
}

//! Returns the value of the given property anchored in the given namespace, or \c NOTHING if no such property exists in the current XmlNode
/** @param $prop the name of the property
    @param $ns the namespace anchoring the property

    @return the value of the given property anchored in the given namespace, or \c NOTHING if no such property exists in the current XmlNode

    @par Example:
    @code my *string $val = $node.getNsProp($prop, $ns); @endcode
*/
//# *string Qore::Xml::XmlNode::getNsProp(string $prop, string $ns) {}
static AbstractQoreNode *XMLNODE_getNsProp(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *prop = HARD_QORE_STRING(params, 0);
   const QoreStringNode *ns = HARD_QORE_STRING(params, 1);
   return xn->getNsProp(prop->getBuffer(), ns->getBuffer());
}

//! Returns the value of the given property, or \c NOTHING if no such property exists in the current XmlNode
/** @param $prop the name of the property

    @return the value of the given property, or \c NOTHING if no such property exists in the current XmlNode

    @par Example:
    @code my *string $val = $node.getProp($prop); @endcode
*/
//# *string Qore::Xml::XmlNode::getProp(string $prop) {}
static AbstractQoreNode *XMLNODE_getProp(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *prop = HARD_QORE_STRING(params, 0);
   return xn->getProp(prop->getBuffer());
}

/*
static AbstractQoreNode *XMLNODE_getBase(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getBase();
}
*/

//! Returns a string of the content of the current node or \c NOTHING if there is none
/** @return a string of the content of the current node or \c NOTHING if there is none

    @par Example:
    @code my *string $value = $xmlnode.getContent(); @endcode
 */
//# *string Qore::Xml::XmlNode::getContent() {}
static AbstractQoreNode *XMLNODE_getContent(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getContent();
}

//! Returns the name of the current node or \c NOTHING if no name is available
/** @return the name of the current node or \c NOTHING if no name is available

    @par Example:
    @code my *string $name = $xmlnode.getName(); @endcode
*/
//# *string Qore::Xml::XmlNode::getName() {}
static AbstractQoreNode *XMLNODE_getName(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getName();
}

//! Returns the language of the current node, determined by the value of the \c xml:lang attribute of this node or of the nearest ancestor; if no such property is available, then \c NOTHING is returned
/** @return the language of the current node, determined by the value of the \c xml:lang attribute of this node or of the nearest ancestor; if no such property is available, then \c NOTHING is returned

    @par Example:
    @code my *string $lang = $xmlnode.getLang(); @endcode
 */
//# *string Qore::Xml::XmlNode::getLang() {}
static AbstractQoreNode *XMLNODE_getLang(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getLang();
}

//! Returns \c True if the node is a text node, \c False if not
/** @return \c True if the node is a text node, \c False if not

    @par Example:
    @code my bool $b = $xmlnode.isText(); @endcode
*/
//# bool Qore::Xml::XmlNode::isText() {}
static AbstractQoreNode *XMLNODE_isText(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xn->isText());
}

//! Returns \c True if the node is empty or whitespace only, \c False if not
/** @return \c True if the node is empty or whitespace only, \c False if not

    @par Example:
    @code my bool $b = $xmlnode.isBlank(); @endcode
*/
//# bool Qore::Xml::XmlNode::isBlank() {}
static AbstractQoreNode *XMLNODE_isBlank(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xn->isBlank());
}

//! Returns a string containing XML corresponding to the current node and all its children or \c NOTHING if no information is available
/** @return a string containing XML corresponding to the current node and all its children or \c NOTHING if no information is available

    @par Example:
    @code my *string $value = $xmlnode.getXML(); @endcode
 */
//# *string Qore::Xml::XmlNode::getXML() {}
static AbstractQoreNode *XMLNODE_getXML(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getXML();
}

QoreClass *initXmlNodeClass() {
   QORE_TRACE("initXmlNodeClass()");

   QC_XMLNODE = new QoreClass("XmlNode");
   CID_XMLNODE = QC_XMLNODE->getID();
   QC_XMLNODE->setConstructorExtended(XMLNODE_constructor);
   QC_XMLNODE->setCopy((q_copy_t)XMLNODE_copy);

   QC_XMLNODE->addMethodExtended("childElementCount",      (q_method_t)XMLNODE_childElementCount, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, bigIntTypeInfo);
   QC_XMLNODE->addMethodExtended("getSpacePreserve",       (q_method_t)XMLNODE_getSpacePreserve, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, bigIntTypeInfo);
   QC_XMLNODE->addMethodExtended("getLineNumber",          (q_method_t)XMLNODE_getLineNumber, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, bigIntTypeInfo);
   QC_XMLNODE->addMethodExtended("getElementType",         (q_method_t)XMLNODE_getElementType, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, bigIntTypeInfo);

   // *string XmlNode::getElementTypeName()  
   QC_XMLNODE->addMethodExtended("getElementTypeName",     (q_method_t)XMLNODE_getElementTypeName, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *XmlNode XmlNode::firstElementChild()  
   QC_XMLNODE->addMethodExtended("firstElementChild",      (q_method_t)XMLNODE_firstElementChild, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, QC_XMLNODE->getOrNothingTypeInfo());

   // *XmlNode XmlNode::getLastChild()  
   QC_XMLNODE->addMethodExtended("getLastChild",           (q_method_t)XMLNODE_getLastChild, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, QC_XMLNODE->getOrNothingTypeInfo());

   // *XmlNode XmlNode::lastElementChild()  
   QC_XMLNODE->addMethodExtended("lastElementChild",       (q_method_t)XMLNODE_lastElementChild, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, QC_XMLNODE->getOrNothingTypeInfo());

   // *XmlNode XmlNode::nextElementSibling()  
   QC_XMLNODE->addMethodExtended("nextElementSibling",     (q_method_t)XMLNODE_nextElementSibling, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, QC_XMLNODE->getOrNothingTypeInfo());

   // *XmlNode XmlNode::previousElementSibling()  
   QC_XMLNODE->addMethodExtended("previousElementSibling", (q_method_t)XMLNODE_previousElementSibling, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, QC_XMLNODE->getOrNothingTypeInfo());

   QC_XMLNODE->addMethodExtended("getPath",                (q_method_t)XMLNODE_getPath, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringTypeInfo);

   // *string XmlNode::getNsProp(string $prop, string $ns)  
   QC_XMLNODE->addMethodExtended("getNsProp",              (q_method_t)XMLNODE_getNsProp, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo, 2, stringTypeInfo, QORE_PARAM_NO_ARG, stringTypeInfo, QORE_PARAM_NO_ARG);

   // *string XmlNode::getProp(string $prop)  
   QC_XMLNODE->addMethodExtended("getProp",                (q_method_t)XMLNODE_getProp, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   //QC_XMLNODE->addMethod("getBase",                (q_method_t)XMLNODE_getBase);

   // *string XmlNode::getContent()  
   QC_XMLNODE->addMethodExtended("getContent",             (q_method_t)XMLNODE_getContent, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlNode::getLang()  
   QC_XMLNODE->addMethodExtended("getLang",                (q_method_t)XMLNODE_getLang, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   // *string XmlNode::getName()  
   QC_XMLNODE->addMethodExtended("getName",                (q_method_t)XMLNODE_getName, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   QC_XMLNODE->addMethodExtended("isText",                 (q_method_t)XMLNODE_isText, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, boolTypeInfo);

   QC_XMLNODE->addMethodExtended("isBlank",                (q_method_t)XMLNODE_isBlank, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, boolTypeInfo);

   // *string XmlNode::getXML()  
   QC_XMLNODE->addMethodExtended("getXML",                 (q_method_t)XMLNODE_getXML, false, QC_RET_VALUE_ONLY, QDOM_DEFAULT, stringOrNothingTypeInfo);

   return QC_XMLNODE;   
}
