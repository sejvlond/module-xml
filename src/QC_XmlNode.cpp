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

#include "qore-xml-module.h"

qore_classid_t CID_XMLNODE;
QoreClass *QC_XMLNODE;

static void XMLNODE_constructor(QoreObject *self, const QoreListNode *params, ExceptionSink *xsink) {
   xsink->raiseException("XMLNODE-CONSTRUCTOR-ERROR", "this class cannot be constructed directly");
}

static void XMLNODE_copy(QoreObject *self, QoreObject *old, QoreXmlNodeData *xn, ExceptionSink *xsink) {
   self->setPrivate(CID_XMLNODE, new QoreXmlNodeData(*xn));
}

static AbstractQoreNode *XMLNODE_childElementCount(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xn->childElementCount());
}

static AbstractQoreNode *XMLNODE_getSpacePreserve(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xn->getSpacePreserve());
}

static AbstractQoreNode *XMLNODE_getLineNumber(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xn->getLineNumber());
}

static AbstractQoreNode *XMLNODE_getElementType(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return new QoreBigIntNode(xn->getElementType());
}

static AbstractQoreNode *XMLNODE_getElementTypeName(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   const char *nt = get_xml_element_type_name((int)xn->getElementType());
   return nt ? new QoreStringNode(nt) : 0;
}

static QoreObject *doObject(QoreXmlNodeData *data) {
   return data ? new QoreObject(QC_XMLNODE, getProgram(), data) : 0;
}

static AbstractQoreNode *XMLNODE_firstElementChild(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->firstElementChild());
}

static AbstractQoreNode *XMLNODE_getLastChild(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->getLastChild());
}

static AbstractQoreNode *XMLNODE_lastElementChild(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->lastElementChild());
}

static AbstractQoreNode *XMLNODE_nextElementSibling(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->nextElementSibling());
}

static AbstractQoreNode *XMLNODE_previousElementSibling(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return doObject(xn->previousElementSibling());
}

static AbstractQoreNode *XMLNODE_getPath(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getPath(xsink);
}

static AbstractQoreNode *XMLNODE_getNsProp(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *prop = HARD_QORE_STRING(params, 0);
   const QoreStringNode *ns = HARD_QORE_STRING(params, 1);
   return xn->getNsProp(prop->getBuffer(), ns->getBuffer());
}

static AbstractQoreNode *XMLNODE_getProp(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   const QoreStringNode *prop = HARD_QORE_STRING(params, 0);
   return xn->getProp(prop->getBuffer());
}

/*
static AbstractQoreNode *XMLNODE_getBase(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getBase();
}
*/

static AbstractQoreNode *XMLNODE_getContent(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getContent();
}

static AbstractQoreNode *XMLNODE_getName(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getName();
}

static AbstractQoreNode *XMLNODE_getLang(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return xn->getLang();
}

static AbstractQoreNode *XMLNODE_isText(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xn->isText());
}

static AbstractQoreNode *XMLNODE_isBlank(QoreObject *self, QoreXmlNodeData *xn, const QoreListNode *params, ExceptionSink *xsink) {
   return get_bool_node(xn->isBlank());
}

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
