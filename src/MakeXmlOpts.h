/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
 MakeXmlOpts.h

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

#ifndef MAKE_XML_OPTS_H
#define MAKE_XML_OPTS_H

#include <cassert>
#include <string>
#include <stdexcept>
#include "qore/QoreEncoding.h"
#include "qore/QoreHashNode.h"
#include "qore/QoreStringNode.h"
#include "qore/QoreValue.h"
#include "qore-xml-module.h"


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
    explicit MakeXmlOpts();

    MakeXmlOpts(const MakeXmlOpts&) = default;
    MakeXmlOpts& operator=(const MakeXmlOpts&) = default;

    MakeXmlOpts(MakeXmlOpts &&) = default;
    MakeXmlOpts& operator=(MakeXmlOpts &&) = default;

public:
    static MakeXmlOpts createFromFlags(
            int flags, const QoreEncoding* ccs = nullptr);

    static MakeXmlOpts createFromHash(const QoreHashNode *hash);

private:
    template <typename T>
    static void parseValue(
            T &output, const QoreHashNode *hash,
            const std::string &key, qore_type_t keyType,
            bool mandatory = false);

public:
    std::string m_docVersion;
    const QoreEncoding *m_encoding;
    bool m_formatWithWhitespaces;
    bool m_useNumericRefs;
};

// ------------- impl --------------
template <typename T>
void MakeXmlOpts::parseValue(
        T &output, const QoreHashNode *hash,
        const std::string &key, qore_type_t keyType,
        bool mandatory) {
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

// std::string specialization
template <>
void MakeXmlOpts::parseValue<std::string>(
        std::string &output, const QoreHashNode *hash,
        const std::string &key, qore_type_t keyType,
        bool mandatory);


#endif // !MAKE_XML_OPTS_H
