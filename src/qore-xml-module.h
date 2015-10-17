/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  qore-xml-module.h

  Qore Programming Language

  Copyright (C) 2006 - 2015 Qore Technologies

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

#ifndef _QORE_XML_MODULE_H
#define _QORE_XML_MODULE_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <qore/Qore.h>

// XML generation flags
// no flags
#define XGF_NONE                0
// use unicode numeric character references instead of non-ascii characters
#define XGF_USE_NUMERIC_REFS    CE_NONASCII
// add whitespace formatting
#define XGF_ADD_FORMATTING      (1 << 20)

#define XGF_ENCODE_MASK (XGF_USE_NUMERIC_REFS)

// XML parsing flags
// no flags
#define XPF_NONE                 0
// preserve element order by re-writing hash keys in case of duplicate out-of-order elements
#define XPF_PRESERVE_ORDER       (1 << 20)

#define XPF_DECODE_MASK (XPF_DECODE_NUMERIC_REFS | XPF_DECODE_XHTML_REFS)

#endif
