/*  $Id: xbexcept.cpp,v 1.8 1999/03/31 14:26:45 dyp Exp $

    Xbase project source code

    This file contains exceptions for error reporting

    Copyright (C) 1998 Denis Pershin (dyp@inetlab.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; see the file COPYING.LIB.  If
    not, write to the Free Software Foundation, Inc., 59 Temple Place
    - Suite 330, Boston, MA 02111-1307, USA.

*/
#include <xdb/xbconfig.h>

#include <stdlib.h>

#include <xdb/retcodes.h>
#include <xdb/xbexcept.h>
#include <xdb/xtypes.h>

const char *xbStrError(xbShort err)
{
  switch (err) {
  case XB_NO_ERROR:
    return "No error";
  case XB_EOF: 
    return "Xbase EoF";
  case XB_BOF: 
    return "XBase BoF";
  case XB_NO_MEMORY: 
    return "Out of memory";
  case XB_FILE_EXISTS: 
    return "File already exists";
  case XB_OPEN_ERROR: 
    return "Error opening file";
  case XB_WRITE_ERROR: 
    return "Error write to file";
  case XB_UNKNOWN_FIELD_TYPE: 
    return "Unknown field type";
  case XB_ALREADY_OPEN: 
    return "File already opened";
  case XB_NOT_XBASE: 
    return "File is not XBase";
  case XB_INVALID_RECORD: 
    return "Invalid record";
  case XB_INVALID_OPTION: 
    return "Invalid option";
  case XB_NOT_OPEN: 
    return "File not opened";
  case XB_SEEK_ERROR: 
    return "Seek error";
  case XB_READ_ERROR: 
    return "Read error";
  case XB_NOT_FOUND: 
    return "Not found";
  case XB_FOUND: 
    return "Found";
  case XB_INVALID_KEY: 
    return "Invalid key";
  case XB_INVALID_NODELINK: 
    return "Invalid nodelink";
  case XB_KEY_NOT_UNIQUE: 
    return "Key not unique";
  case XB_INVALID_KEY_EXPRESSION: 
    return "Invalid key expression";
  case XB_DBF_FILE_NOT_OPEN: 
    return "DBF file not open";
  case XB_INVALID_KEY_TYPE: 
    return "Invalid key type";
  case XB_INVALID_NODE_NO: 
    return "Invalid node no";
  case XB_NODE_FULL: 
    return "Node full";
  case XB_INVALID_FIELDNO: 
    return "Invalid field no";
  case XB_INVALID_DATA: 
    return "Invalid data";
  case XB_NOT_LEAFNODE: 
    return "Not leafnode";
  case XB_LOCK_FAILED: 
    return "Lock failed";
  case XB_CLOSE_ERROR: 
    return "Close error";
  case XB_INVALID_SCHEMA: 
    return "Invalid schema";
  case XB_INVALID_NAME: 
    return "Invlaid name";
  case XB_INVALID_BLOCK_SIZE: 
    return "Invalid block size";
  case XB_INVALID_BLOCK_NO: 
    return "Invalid block no";
  case XB_NOT_MEMO_FIELD: 
    return "Not memo field";
  case XB_NO_MEMO_DATA: 
    return "No memo data";
  case XB_EXP_SYNTAX_ERROR: 
    return "Expression syntax error";
  case XB_PARSE_ERROR: 
    return "Parse error";
  case XB_NO_DATA: 
    return "No data";
  case XB_UNKNOWN_TOKEN_TYPE: 
    return "Unknown token type";
  case XB_INVALID_FIELD: 
    return "Invalid field";
  case XB_INSUFFICIENT_PARMS: 
    return "Insufficient parameters";
  case XB_INVALID_FUNCTION: 
    return "Invalid function";
  case XB_INVALID_FIELD_LEN: 
    return "Invalid field len";
  default: 
    return "Unknown exception";
  }
}

#ifdef HAVE_EXCEPTIONS

#include <string.h>
#include <errno.h>

xbException::xbException (int err) {
    this->err = err;
}

xbException::~xbException () XB_THROW {
}

const char* xbException::what() const XB_THROW {
  return "xbException"; 
};

int xbException::getErr() {
  return err;
};

const char *xbException::error() {
  return xbStrError(err);
}

xbIOException::xbIOException (int err) : xbException(err) {
	m_errno = errno;
  name = NULL;
}

xbIOException::xbIOException (int err, const char *n) : xbException(err) {
	name = n;
	m_errno = errno;
}

xbIOException::~xbIOException () XB_THROW {
}

const char* xbIOException::what() const XB_THROW {
  return "xbIOException"; 
};

const char *xbIOException::_errno() const {
	return (strerror(m_errno));
}

xbOpenException::xbOpenException () : xbIOException(XB_OPEN_ERROR) {
}

xbOpenException::xbOpenException (const char *n) : xbIOException(XB_OPEN_ERROR, n) {
}

xbOpenException::~xbOpenException () XB_THROW {
}

const char* xbOpenException::what() const XB_THROW {
  return "xbOpenException"; 
};

xbEoFException::xbEoFException () : xbIOException(XB_EOF) {
}

xbEoFException::~xbEoFException () XB_THROW {
}

const char* xbEoFException::what() const XB_THROW {
  return "xbEoFException"; 
};

xbOutOfMemoryException::xbOutOfMemoryException () : xbException(XB_NO_MEMORY) {
}

xbOutOfMemoryException::~xbOutOfMemoryException () XB_THROW {
}

const char* xbOutOfMemoryException::what() const XB_THROW {
  return "xbOutOfMemoryException"; 
};

#endif
