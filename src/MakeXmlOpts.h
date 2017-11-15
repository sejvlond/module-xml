/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
 MakeXmlOpts.h

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

#ifndef MAKE_XML_OPTS_H
#define MAKE_XML_OPTS_H

#include <cassert>
#include <string>
#include <stdexcept>

#include <iostream>

class MakeXmlOpts final {
public:
    class InvalidHash final : public std::invalid_argument {
    public:
        explicit InvalidHash(const std::string &what_arg) :
            std::invalid_argument(what_arg)
        {}

        explicit InvalidHash(const char *what_arg) :
            std::invalid_argument(what_arg)
        {}

        InvalidHash(InvalidHash &&) = default;
    };

public:
    MakeXmlOpts() :
        m_docVersion("1.0"),
        m_encoding(QCS_DEFAULT),
        m_formatWithWhitespaces(false),
        m_useNumericRefs(false)
    {}

    MakeXmlOpts(const MakeXmlOpts&) = default;
    MakeXmlOpts& operator=(const MakeXmlOpts&) = default;

    MakeXmlOpts(MakeXmlOpts &&) = default;
    MakeXmlOpts& operator=(MakeXmlOpts &&) = default;

public:
    static MakeXmlOpts createFromFlags(
            int flags, const QoreEncoding* ccs = nullptr) {
        MakeXmlOpts opts;
        opts.m_encoding = ccs ? ccs : opts.m_encoding;
        opts.m_formatWithWhitespaces = XGF_ADD_FORMATTING & flags;
        opts.m_useNumericRefs = XGF_USE_NUMERIC_REFS & flags;
        return opts;
    }

    static MakeXmlOpts createFromHash(const QoreHashNode *hash) {
        assert(hash);

        MakeXmlOpts opts;
        // docVersion
        QoreStringNode *docVersion = nullptr;
        parseValue(docVersion, hash, "docVersion", NT_STRING);
        if (docVersion)
            opts.m_docVersion = docVersion->c_str();
        // encoding
        QoreStringNode *encoding = nullptr;
        parseValue(encoding, hash, "encoding", NT_STRING);
        if (encoding)
            opts.m_encoding = QEM.findCreate(encoding);
        // formatWithWhitespaces
        parseValue(opts.m_formatWithWhitespaces, hash, "formatWithWhitespaces", NT_BOOLEAN);
        // useNumericRefs
        parseValue(opts.m_useNumericRefs, hash, "useNumericRefs", NT_BOOLEAN);

        return opts;
    }

private:
    template <typename T>
    static void parseValue(
            T &output, const QoreHashNode *hash,
            const std::string &key, qore_type_t keyType,
            bool mandatory = false) {
        assert(hash);
        bool exists = false;
        auto value = hash->getValueKeyValueExistence(key.c_str(), exists);
        if (!exists) {
            if (mandatory)
                throw InvalidHash(key);
            return;
        }
        if (value.getType() != keyType)
            throw InvalidHash(key);
        output = value.get<typename std::remove_pointer<T>::type>();
    }

public:
    std::string m_docVersion;
    const QoreEncoding *m_encoding;
    bool m_formatWithWhitespaces;
    bool m_useNumericRefs;
};

#endif // !MAKE_XML_OPTS_H
