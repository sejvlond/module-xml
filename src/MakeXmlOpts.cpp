/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
 MakeXmlOpts.cpp

 Qore Programming Language

 Copyright (C) 2017 Qore Technologies, s.r.o.

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

#include "MakeXmlOpts.h"

MakeXmlOpts::MakeXmlOpts() :
    m_docVersion("1.0"),
    m_encoding(QCS_DEFAULT),
    m_formatWithWhitespaces(false),
    m_useNumericRefs(false),
    m_dateFormat("YYYYMMDDHHmmSS")
{}


MakeXmlOpts MakeXmlOpts::createFromFlags(int flags, const QoreEncoding* ccs) {
    MakeXmlOpts opts;
    opts.m_encoding = ccs ? ccs : opts.m_encoding;
    opts.m_formatWithWhitespaces = XGF_ADD_FORMATTING & flags;
    opts.m_useNumericRefs = XGF_USE_NUMERIC_REFS & flags;
    return opts;
}


MakeXmlOpts MakeXmlOpts::createFromHash(const QoreHashNode *hash) {
    MakeXmlOpts opts;

    if (!hash)
        return opts;

    // docVersion
    parseValue(opts.m_docVersion , hash, "docVersion", NT_STRING);
    // encoding
    QoreStringNode *encoding = nullptr;
    parseValue(encoding, hash, "encoding", NT_STRING);
    if (encoding)
        opts.m_encoding = QEM.findCreate(encoding);
    // formatWithWhitespaces
    parseValue(opts.m_formatWithWhitespaces, hash, "formatWithWhitespaces", NT_BOOLEAN);
    // useNumericRefs
    parseValue(opts.m_useNumericRefs, hash, "useNumericRefs", NT_BOOLEAN);
    // dateFormat
    parseValue(opts.m_dateFormat , hash, "dateFormat", NT_STRING);

    return opts;
}


template <>
void MakeXmlOpts::parseValue<std::string>(
        std::string &output, const QoreHashNode *hash,
        const std::string &key, qore_type_t keyType,
        bool mandatory) {
    QoreStringNode *value = nullptr;
    parseValue(value, hash, key, keyType);
    if (value)
        output = value->c_str();
}
