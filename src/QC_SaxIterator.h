/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  QC_SaxIterator.h
 
  Qore Programming Language
 
  Copyright 2003 - 2014 David Nichols
 
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

#ifndef _QORE_QC_SAXITERATOR_H

#define _QORE_QC_SAXITERATOR_H

#include "QC_XmlReader.h"

#include <string>

DLLEXPORT extern qore_classid_t CID_SAXITERATOR;
DLLLOCAL QoreClass *initSaxIteratorClass(QoreNamespace& ns);

class QoreSaxIterator : public QoreXmlReaderData, public QoreAbstractIteratorBase {
protected:
   std::string element_name;
   bool val;

public:
   DLLLOCAL QoreSaxIterator(QoreStringNode* xml, const char* ename, ExceptionSink* xsink) : QoreXmlReaderData(xml, xsink), element_name(ename), val(false) {
   }

   DLLLOCAL QoreSaxIterator(QoreXmlDocData* doc, const char* ename, ExceptionSink* xsink) : QoreXmlReaderData(doc, xsink), element_name(ename), val(false) {
   }

   DLLLOCAL QoreSaxIterator(const QoreSaxIterator& old, ExceptionSink* xsink) : QoreXmlReaderData(old, xsink), element_name(old.element_name), val(false) {
   }

   DLLLOCAL AbstractQoreNode* getReferencedValue(ExceptionSink* xsink) {
      SimpleRefHolder<QoreStringNode> holder(getInnerXml());
      if (!holder)
         return 0;
      TempEncodingHelper str(*holder, QCS_UTF8, xsink);
      if (*xsink)
         return 0;
      str.makeTemp();
      ((QoreString*)(*str))->prepend("<a>");
      ((QoreString*)(*str))->concat("</a>");
      
      QoreXmlReader reader(*str, QORE_XML_PARSER_OPTIONS, xsink);
      if (!reader)
         return 0;

      ReferenceHolder<QoreHashNode> h(reader.parseXMLData(QCS_UTF8, true, xsink), xsink);
      AbstractQoreNode* n = h->getKeyValue("a");
      return n ? n->refSelf() : 0;
   }

   DLLLOCAL bool next(ExceptionSink* xsink) {
      if (!val) {
         if (!isValid())
            reset(xsink);
      }

      while (true) {
         if (!readSkipWhitespace(xsink)) {
            val = false;
            break;
         }
         if (nodeType() == XML_READER_TYPE_ELEMENT) {
            const char* n = localName();
            if (n && element_name == n) {
               if (!val)
                  val = true;
               break;
            }
         }
      }

      return val;
   }

   DLLLOCAL bool valid() const {
      return val;
   }

   DLLLOCAL virtual const char* getName() const { return "SaxIterator"; }
};

#endif
