/****************************************************************************
** $Id: //depot/qt/main/src/xml/qxml.cpp#75 $
**
** Implementation of QXmlSimpleReader and related classes.
**
** Created : 000518
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the XML module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qxml.h"
#include "qtextcodec.h"
#include "qbuffer.h"
#include "qregexp.h"
#include "qstack.h"
#include "qmap.h"
#include "qvaluestack.h"

#ifndef QT_NO_XML
// NOT REVISED

//#define QT_QXML_DEBUG

// Error strings for the XML reader
#define XMLERR_OK                         "no error occurred"
#define XMLERR_ERRORBYCONSUMER            "error triggered by consumer"
#define XMLERR_UNEXPECTEDEOF              "unexpected end of file"
#define XMLERR_MORETHANONEDOCTYPE         "more than one document type definition"
#define XMLERR_ERRORPARSINGELEMENT        "error while parsing element"
#define XMLERR_TAGMISMATCH                "tag mismatch"
#define XMLERR_ERRORPARSINGCONTENT        "error while parsing content"
#define XMLERR_UNEXPECTEDCHARACTER        "unexpected character"
#define XMLERR_INVALIDNAMEFORPI           "invalid name for processing instruction"
#define XMLERR_VERSIONEXPECTED            "version expected while reading the XML declaration"
#define XMLERR_WRONGVALUEFORSDECL         "wrong value for standalone declaration"
#define XMLERR_EDECLORSDDECLEXPECTED      "EDecl or SDDecl expected while reading the XML declaration"
#define XMLERR_SDDECLEXPECTED             "SDDecl expected while reading the XML declaration"
#define XMLERR_ERRORPARSINGDOCTYPE        "error while parsing document type definition"
#define XMLERR_LETTEREXPECTED             "letter is expected"
#define XMLERR_ERRORPARSINGCOMMENT        "error while parsing comment"
#define XMLERR_ERRORPARSINGREFERENCE      "error while parsing reference"
#define XMLERR_INTERNALGENERALENTITYINDTD "internal general entity reference not allowed in DTD"
#define XMLERR_EXTERNALGENERALENTITYINAV  "external parsed general entity reference not allowed in attribute value"
#define XMLERR_EXTERNALGENERALENTITYINDTD "external parsed general entity reference not allowed in DTD"
#define XMLERR_UNPARSEDENTITYREFERENCE    "unparsed entity reference in wrong context"

// the constants for the lookup table
static const signed char cltWS      =  0; // white space
static const signed char cltPer     =  1; // %
static const signed char cltAmp     =  2; // &
static const signed char cltGt      =  3; // >
static const signed char cltLt      =  4; // <
static const signed char cltSlash   =  5; // /
static const signed char cltQm      =  6; // ?
static const signed char cltEm      =  7; // !
static const signed char cltDash    =  8; // -
static const signed char cltCB      =  9; // ]
static const signed char cltOB      = 10; // [
static const signed char cltEq      = 11; // =
static const signed char cltDq      = 12; // "
static const signed char cltSq      = 13; // '
static const signed char cltUnknown = 14;

// character lookup table
static const signed char charLookupTable[256]={
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x00 - 0x07
    cltUnknown, // 0x08
    cltWS,      // 0x09 \t
    cltWS,      // 0x0A \n
    cltUnknown, // 0x0B
    cltUnknown, // 0x0C
    cltWS,      // 0x0D \r
    cltUnknown, // 0x0E
    cltUnknown, // 0x0F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x17 - 0x16
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x18 - 0x1F
    cltWS,      // 0x20 Space
    cltEm,      // 0x21 !
    cltDq,      // 0x22 "
    cltUnknown, // 0x23
    cltUnknown, // 0x24
    cltPer,     // 0x25 %
    cltAmp,     // 0x26 &
    cltSq,      // 0x27 '
    cltUnknown, // 0x28
    cltUnknown, // 0x29
    cltUnknown, // 0x2A
    cltUnknown, // 0x2B
    cltUnknown, // 0x2C
    cltDash,    // 0x2D -
    cltUnknown, // 0x2E
    cltSlash,   // 0x2F /
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x30 - 0x37
    cltUnknown, // 0x38
    cltUnknown, // 0x39
    cltUnknown, // 0x3A
    cltUnknown, // 0x3B
    cltLt,      // 0x3C <
    cltEq,      // 0x3D =
    cltGt,      // 0x3E >
    cltQm,      // 0x3F ?
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x40 - 0x47
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x48 - 0x4F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x50 - 0x57
    cltUnknown, // 0x58
    cltUnknown, // 0x59
    cltUnknown, // 0x5A
    cltOB,      // 0x5B [
    cltUnknown, // 0x5C
    cltCB,      // 0x5D ]
    cltUnknown, // 0x5E
    cltUnknown, // 0x5F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x60 - 0x67
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x68 - 0x6F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x70 - 0x77
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x78 - 0x7F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x80 - 0x87
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x88 - 0x8F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x90 - 0x97
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x98 - 0x9F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xA0 - 0xA7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xA8 - 0xAF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xB0 - 0xB7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xB8 - 0xBF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xC0 - 0xC7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xC8 - 0xCF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xD0 - 0xD7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xD8 - 0xDF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xE0 - 0xE7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xE8 - 0xEF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xF0 - 0xF7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown  // 0xF8 - 0xFF
};


class QXmlAttributesPrivate
{
};
class QXmlInputSourcePrivate
{
};
class QXmlParseExceptionPrivate
{
};
class QXmlLocatorPrivate
{
};
class QXmlDefaultHandlerPrivate
{
};

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
bool operator==( const QMap<QString, QString>, const QMap<QString, QString> )
{
    return FALSE;
}
#endif

/*!
  \class QXmlParseException qxml.h
  \brief The QXmlParseException class is used to report errors with the
  QXmlErrorHandler interface.

  \module XML

  The XML subsystem constructs an instance of this class when it
  detects error, and any interested application can access details
  about the error using lineNumber(), fileName() and so on.

  \sa QXmlErrorHandler QXmlReader
*/
/*!
  \fn QXmlParseException::QXmlParseException( const QString& name, int c, int l, const QString& p, const QString& s )

  Constructs a parse exception with the error string \a name in the column
  \a c and line \a l for the public identifier \a p and the system identifier
  \a s.
*/
/*!
  Returns the error message.
*/
QString QXmlParseException::message() const
{
    return msg;
}
/*!
  Returns the column number the error occurred.
*/
int QXmlParseException::columnNumber() const
{
    return column;
}
/*!
  Returns the line number the error occurred.
*/
int QXmlParseException::lineNumber() const
{
    return line;
}
/*!
  Returns the public identifier the error occurred.
*/
QString QXmlParseException::publicId() const
{
    return pub;
}
/*!
  Returns the system identifier the error occurred.
*/
QString QXmlParseException::systemId() const
{
    return sys;
}


/*!
  \class QXmlLocator qxml.h
  \brief The QXmlLocator class provides the XML handler classes with
  information about the actual parsing position.

  \module XML

  The reader reports a QXmlLocator to the content handler before he starts to
  parse the document. This is done with the
  QXmlContentHandler::setDocumentLocator() function. The handler classes can
  now use this locator to get the actual position the reader is at.
*/
/*!
    \fn QXmlLocator::QXmlLocator( QXmlSimpleReader* parent )

    Constructs an XML locator for the reader \a parent.
*/
QXmlLocator::QXmlLocator( QXmlSimpleReader* parent )
{
    reader = parent;
}

/*!
    \fn QXmlLocator::~QXmlLocator()

    Destructor.
*/
QXmlLocator::~QXmlLocator()
{
}

/*!
    Gets the column number (starting with 1) or -1 if there is no column number
    available.
*/
int QXmlLocator::columnNumber()
{
    return ( reader->columnNr == -1 ? -1 : reader->columnNr + 1 );
}

/*!
    Gets the line number (starting with 1) or -1 if there is no line number
    available.
*/
int QXmlLocator::lineNumber()
{
    return ( reader->lineNr == -1 ? -1 : reader->lineNr + 1 );
}


/*********************************************
 *
 * QXmlNamespaceSupport
 *
 *********************************************/

class QXmlNamespaceSupportPrivate
{
public:
    QXmlNamespaceSupportPrivate()
    {
	ns = new QMap<QString, QString>;
	ns->insert( "xml", "http://www.w3.org/XML/1998/namespace" ); // the XML namespace
    }

    ~QXmlNamespaceSupportPrivate()
    {
	nsStack.setAutoDelete( TRUE );
	nsStack.clear();
	delete ns;
    }

    QStack<QMap<QString, QString> > nsStack;
    QMap<QString, QString> *ns;
};

/*!
  \class QXmlNamespaceSupport qxml.h
  \brief The QXmlNamespaceSupport class is a helper class for XML readers which
  want to include namespace support.

  \module XML

  It provides some functions that makes it easy to handle namespaces. Its main
  use is for subclasses of QXmlReader which want to provide namespace
  support.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.
*/

/*!
  Constructs a QXmlNamespaceSupport.
*/
QXmlNamespaceSupport::QXmlNamespaceSupport()
{
    d = new QXmlNamespaceSupportPrivate;
}

/*!
  Destructs a QXmlNamespaceSupport.
*/
QXmlNamespaceSupport::~QXmlNamespaceSupport()
{
    delete d;
}

/*!
  This function declares a prefix \a pre in the current namespace context to be
  the namespace URI \a uri; the prefix remains in force until this context
  is popped, unless it is shadowed in a descendant context.

  Note that there is an asymmetry in this library: while prefix() does not
  return the default "" prefix, even if you have declared one; to check for a
  default prefix, you have to look it up explicitly using uri(). This
  asymmetry exists to make it easier to look up prefixes for attribute names,
  where the default prefix is not allowed.
*/
void QXmlNamespaceSupport::setPrefix( const QString& pre, const QString& uri )
{
    if( pre.isNull() ) {
	d->ns->insert( "", uri );
    } else {
	d->ns->insert( pre, uri );
    }
}

/*!
  Returns one of the prefixes mapped to the namespace URI \a uri.

  If more than one prefix is currently mapped to the same URI, this function
  makes an arbitrary selection; if you want all of the prefixes, use the
  prefixes() function instead.

  Note: this function never returns the empty (default) prefix; to check for a
  default prefix, use the uri() function with an argument of "".
*/
QString QXmlNamespaceSupport::prefix( const QString& uri ) const
{
    QMap<QString, QString>::ConstIterator itc, it = d->ns->begin();
    while ( (itc=it) != d->ns->end() ) {
	++it;
	if ( itc.data() == uri && !itc.key().isEmpty() )
	    return itc.key();
    }
    return "";
}

/*!
  Looks up the prefix \a prefix in the current context and returns the
  currently-mapped namespace URI. Use the empty string ("") for the default
  namespace.
*/
QString QXmlNamespaceSupport::uri( const QString& prefix ) const
{
    const QString& returi = (*d->ns)[ prefix ];
    return returi;
}

/*!
  Splits the name \a qname at the ':' and returns the prefix in \a prefix and
  the local name in \a localname.
*/
void QXmlNamespaceSupport::splitName( const QString& qname,
	QString& prefix, QString& localname ) const
{
    uint pos;
    // search the ':'
    for( pos=0; pos<qname.length(); pos++ ) {
	if ( qname.at(pos) == ':' )
	    break;
    }
    // and split
    prefix = qname.left( pos );
    localname = qname.mid( pos+1 );
}

/*!
  Processes a raw XML 1.0 name in the current context by removing the prefix
  and looking it up among the prefixes currently declared.

  \a qname is the raw XML 1.0 name to be processed and \a isAttribute is a flag
  that specifies wheter the name is the name of an attribute (TRUE) or not
  (FALSE).

  This function stores the return values the last two parameters as follows:
  It stores the namespace URI in \a nsuri and the local name (without prefix)
  in \a localname.  If no namespace is in use, it stores a null string.

  If the raw name has a prefix that has not been declared, then the return
  value is empty.

  Note that attribute names are processed differently than element names: an
  unprefixed element name gets the default namespace (if any), while
  an unprefixed element name does not.
*/
void QXmlNamespaceSupport::processName( const QString& qname,
	bool isAttribute,
	QString& nsuri, QString& localname ) const
{
    uint pos = qname.find( ':' );
    if ( pos < qname.length() ) {
	// there was a ':'
	nsuri = uri( qname.left( pos ) );
	localname = qname.mid( pos+1 );
    } else {
	// there was no ':'
	if ( isAttribute ) {
	    nsuri = QString::null; // attributes don't take default namespace
	} else {
	    nsuri = uri( "" ); // get default namespace
	}
	localname = qname;
    }
}

/*!
  Returns a list of all prefixes currently declared.

  Note: if there is a default prefix, this function does not return it in the
  list; check for the default prefix using uri() with an argument
  of "".
*/
QStringList QXmlNamespaceSupport::prefixes() const
{
    QStringList list;

    QMap<QString, QString>::ConstIterator itc, it = d->ns->begin();
    while ( (itc=it) != d->ns->end() ) {
	++it;
	if ( !itc.key().isEmpty() )
	    list.append( itc.key() );
    }
    return list;
}

/*!
  \overload
  Returns a list of all prefixes currently declared for the namespace URI \a
  uri.

  The "xml:" prefix is included. If you want only one prefix that is
  mapped to the namespace URI, and you don't care which one you get, use the
  prefix() function instead.

  Note: the empty (default) prefix is never included in this enumeration; to
  check for the presence of a default namespace, use uri() with an
  argument of "".
*/
QStringList QXmlNamespaceSupport::prefixes( const QString& uri ) const
{
    QStringList list;

    QMap<QString, QString>::ConstIterator itc, it = d->ns->begin();
    while ( (itc=it) != d->ns->end() ) {
	++it;
	if ( itc.data() == uri && !itc.key().isEmpty() )
	    list.append( itc.key() );
    }
    return list;
}

/*!
  Starts a new namespace context.

  Normally, you should push a new context at the beginning of each XML element:
  the new context inherits automatically the declarations of its parent
  context, but it also keeps track of which declarations were made within
  this context.
*/
void QXmlNamespaceSupport::pushContext()
{
    d->nsStack.push( new QMap<QString, QString>(*d->ns) );
}

/*!
  Reverts to the previous namespace context.

  Normally, you should pop the context at the end of each XML element.  After
  popping the context, all namespace prefix mappings that were previously in
  force are restored.
*/
void QXmlNamespaceSupport::popContext()
{
    delete d->ns;
    if( !d->nsStack.isEmpty() )
	d->ns = d->nsStack.pop();
}

/*!
  Resets this namespace support object for reuse.
*/
void QXmlNamespaceSupport::reset()
{
    delete d;
    d = new QXmlNamespaceSupportPrivate;
}



/*********************************************
 *
 * QXmlAttributes
 *
 *********************************************/

/*!
  \class QXmlAttributes qxml.h
  \brief The QXmlAttributes class provides XML attributes.

  \module XML

  If attributes are reported by QXmlContentHandler::startElement() this
  class is used to pass the attribute values. It provides you with different
  functions to access the attribute names and values.
*/
/*!
  \fn QXmlAttributes::QXmlAttributes()

  Constructs an empty attribute list.
*/
/*!
  \fn QXmlAttributes::~QXmlAttributes()

  Destructs attributes.
*/

/*!
  Looks up the index of an attribute by the qualified name \a qName.

  Returns the index of the attribute (starting with 0) or -1 if it wasn't
  found.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.
*/
int QXmlAttributes::index( const QString& qName ) const
{
    return qnameList.findIndex( qName );
}

/*!
  \overload
  Looks up the index of an attribute by a namespace name.

  \a uri specifies the namespace URI, or an empty string if the name has no
  namespace URI. \a localPart specifies the attribute's local name.

  Returns the index of the attribute (starting with 0) or -1 if it wasn't
  found.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.
*/
int QXmlAttributes::index( const QString& uri, const QString& localPart ) const
{
    QString uriTmp;
    if ( uri.isEmpty() )
	uriTmp = QString::null;
    else
	uriTmp = uri;
    uint count = uriList.count();
    for ( uint i=0; i<count; i++ ) {
	if ( uriList[i] == uriTmp && localnameList[i] == localPart )
	    return i;
    }
    return -1;
}

/*!
  Returns the number of attributes in the list.
*/
int QXmlAttributes::length() const
{
    return valueList.count();
}

/*!
  Looks up an attribute's local name for the index \a index (starting with 0).
  If no namespace processing is done, the local name is a null string.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.
*/
QString QXmlAttributes::localName( int index ) const
{
    return localnameList[index];
}

/*!
  Looks up an attribute's XML 1.0 qualified name for the index \a index
  (starting with 0).

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.
*/
QString QXmlAttributes::qName( int index ) const
{
    return qnameList[index];
}

/*!
  Looks up an attribute's namespace URI for the index \a index (starting with
  0). If no namespace processing is done or if the attribute has no namespace,
  the namespace URI is a null string.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.
*/
QString QXmlAttributes::uri( int index ) const
{
    return uriList[index];
}

/*!
  Looks up an attribute's type for the index \a index (starting with 0).

  At the moment only 'CDATA' is returned.
*/
QString QXmlAttributes::type( int ) const
{
    return "CDATA";
}

/*!
  \overload
  Looks up an attribute's type for the qualified name \a qName.

  At the moment only 'CDATA' is returned.
*/
QString QXmlAttributes::type( const QString& ) const
{
    return "CDATA";
}

/*!
  \overload
  Looks up an attribute's type by namespace name.

  \a uri specifies the namespace URI and \a localName specifies the local name.
  If the name has no namespace URI, use an empty string for \a uri.

  At the moment only 'CDATA' is returned.
*/
QString QXmlAttributes::type( const QString&, const QString& ) const
{
    return "CDATA";
}

/*!
  Looks up an attribute's value for the index \a index (starting with 0).
*/
QString QXmlAttributes::value( int index ) const
{
    return valueList[index];
}

/*!  \overload
  Looks up an attribute's value for the qualified name \a qName.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.
*/
QString QXmlAttributes::value( const QString& qName ) const
{
    int i = index( qName );
    if ( i == -1 )
	return QString::null;
    return valueList[ i ];
}

/*!  \overload
  Looks up an attribute's value by namespace name.

  \a uri specifies the namespace URI, or an empty string if the name has no
  namespace URI. \a localName specifies the attribute's local name.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.
*/
QString QXmlAttributes::value( const QString& uri, const QString& localName ) const
{
    int i = index( uri, localName );
    if ( i == -1 )
	return QString::null;
    return valueList[ i ];
}


/*********************************************
 *
 * QXmlInputSource
 *
 *********************************************/

/*!
  \class QXmlInputSource qxml.h
  \brief The QXmlInputSource class is the source where XML data is read from.

  \module XML

  All subclasses of QXmlReader read the input XML document from this class.

  On construction, this class reads all data that is available on the source.
  The first call of data() returns this data. Any other calls of data() try
  to read more data from the source and returns only the new data.

  You can also add data with setData(). Then the next call of data() returns
  this data. Any other calls try to read more data from the source. So
  setData() has a higher priority than the polling behavior.

  Usually you either construct a QXmlInputSource that works on a QIODevice* or
  you construct an empty QXmlInputSource and set the data with setData(). There
  are only rare occasions where you want to mix both methods.

  This class recognizes the encoding of the data: it tries to read the encoding
  declaration of the XML file and if it finds it, it reads the data in the
  corresponding encoding. If it does not find an encoding declaration, then it
  assumes that the data is either in UTF-8 or UTF-16, depending if it can find
  a byte-order mark.

  \sa QXmlReader QXmlSimpleReader
*/

// the following two are guaranteed not to be a character
const QChar QXmlInputSource::EndOfData = QChar((ushort)0xfffe);
const QChar QXmlInputSource::EndOfDocument = QChar((ushort)0xffff);

/*!
  Common part of the constructors.
*/
void QXmlInputSource::init()
{
    inputDevice = 0;
    inputStream = 0;

    str = QString::null;
    pos = 0;
    length = str.length();
    nextReturnedEndOfData = FALSE;
    encMapper = 0;
}

/*!
  Constructs an input source which contains no data.

  \sa setData()
*/
QXmlInputSource::QXmlInputSource()
{
    init();
}

/*!  Constructs an input source and gets the data from \a dev. If \a
  dev is not open, it is opened in read-only mode. If \a dev is a null
  pointer or it is not possible to read from the device, the input
  source will contain no data.

  \sa setData() QIODevice
*/
QXmlInputSource::QXmlInputSource( QIODevice *dev )
{
    init();
    inputDevice = dev;
    fetchData();
}

/*! \obsolete
  Constructs an input source and gets the data from the text stream \a stream.
*/
QXmlInputSource::QXmlInputSource( QTextStream& stream )
{
    init();
    inputStream = &stream;
    fetchData();
}

/*! \obsolete
  Constructs an input source and gets the data from the file \a file. If the
  file cannot be read the input source is empty.
*/
QXmlInputSource::QXmlInputSource( QFile& file )
{
    init();
    inputDevice = &file;
    fetchData();
}

/*!
  Destructor.
*/
QXmlInputSource::~QXmlInputSource()
{
    delete encMapper;
}

/*!
  Returns the next character of the input source. If this funciton reaches the
  end of available data, it returns QXmlInputSource::EndOfData. If you call next()
  after that, it tries to fetch more data by calling fetchData(). If this call
  results in new data, this function returns the first character of that data;
  otherwise it returns QXmlInputSource::EndOfDocument.

  This function can have the two special return values QXmlInputSource::EndOfData and
  QXmlInputSource::EndOfDocument. This destinction is especially useful for incremental
  parsing: QXmlInputSource::EndOfData means that the input source ran out of data,
  but may be able to deliver more data at a later point. QXmlInputSource::EndOfDocument
  on the other hand means, that the input source really run out of data and
  can't provide more data at a later point. The non-incremental parsing mode
  does not distinguish between the two values - they mean that the end of the
  data was reached.

  \sa fetchData() QXmlSimpleReader::prarse() QXmlSimpleReader::parseContinue()
*/
QChar QXmlInputSource::next()
{
    if ( pos >= length ) {
	if ( nextReturnedEndOfData ) {
	    nextReturnedEndOfData = FALSE;
	    fetchData();
	    if ( pos >= length ) {
		return EndOfDocument;
	    }
	    return next();
	}
	nextReturnedEndOfData = TRUE;
	return EndOfData;
    }
    return str[pos++];
}

/*!
  Returns the data the input source contains or QString::null if the input
  source does not contain any data.

  On construction, this class reads all data that is available on the source.
  You can also set data with setData().

  If the class was able to read data on construction, the first call of this
  function returns it. Otherwise the first call of this function returns the
  data that was set with setData() or QString::null, if no data is set.

  Any other calls of data() return either the data that was set with setData()
  or try to read more data from the source and return it. This function never
  returns data that was returned by a previous call.

  This class tries to find out the correct encoding for the raw data.

  \sa setData() QXmlInputSource()
*/
QString QXmlInputSource::data()
{
    return str;
}

/*!
  Sets the data of the input source to \a dat.

  If the input source already contains data from a previous call of setData()
  this function deletes that data first.

  \sa data()
*/
void QXmlInputSource::setData( const QString& dat )
{
    str = dat;
    pos = 0;
    length = str.length();
    nextReturnedEndOfData = FALSE;
}

/*! \overload
  This function sets the raw data of the input source to \a dat. If you get the
  data with data(), it is sent through the right text-codec.
*/
void QXmlInputSource::setData( const QByteArray& dat )
{
    str = fromRawData( dat );
    pos = 0;
    length = str.length();
    nextReturnedEndOfData = FALSE;
}

/*!
  This private function reads the data from inputDevice (if it is not 0) or
  from inputStream (if it is not 0) and stores it in rawData. If rawData is 0,
  it allocates a new QByteArray, otherwise it replaces the new data.
*/
void QXmlInputSource::fetchData()
{
    QByteArray rawData;

    if ( inputDevice != 0 ) {
	if ( inputDevice->isOpen() || inputDevice->open( IO_ReadOnly )  )
	    rawData = inputDevice->readAll();
    } else if ( inputStream != 0 ) {
	if ( inputStream->device()->isDirectAccess() ) {
	    rawData = inputStream->device()->readAll();
	} else {
	    int nread = 0;
	    const int bufsize = 512;
	    while ( !inputStream->device()->atEnd() ) {
		rawData.resize( nread + bufsize );
		nread += inputStream->device()->readBlock( rawData.data()+nread, bufsize );
	    }
	    rawData.resize( nread );
	}
    }
    str = fromRawData( rawData );
    pos = 0;
    length = str.length();
    nextReturnedEndOfData = FALSE;
}

/*!
  This function reads the XML file from \a data and tries to recoginize the
  encoding. It converts the raw data \a data to a QString and returns it. It
  tries the best to get the right encoding for the XML file.

  If \a beginning is TRUE, this function assumes the beginning of a new XML
  document and looks for a new encoding declaration. If \a beginning is FALSE,
  it converts the raw data with the guess from prior calls. Specifying FALSE is
  useful if you do incremental parsing, i.e., when one XML document is parsed
  in chunks.
*/
QString QXmlInputSource::fromRawData( const QByteArray &data, bool beginning )
{
    if ( beginning ) {
	delete encMapper;
	encMapper = 0;
    }
    if ( encMapper == 0 ) {
	QTextCodec *codec = 0;
	// look for byte order mark and read the first 5 characters
	if ( data.size() >= 2 &&
		( ((uchar)data.at(0)==(uchar)0xfe &&
		   (uchar)data.at(1)==(uchar)0xff ) ||
		  ((uchar)data.at(0)==(uchar)0xff &&
		   (uchar)data.at(1)==(uchar)0xfe ) )) {
	    codec = QTextCodec::codecForMib( 1000 ); // UTF-16
	} else {
	    codec = QTextCodec::codecForMib( 106 ); // UTF-8
	}
	if ( !codec )
	    return QString::null;

	encMapper = codec->makeDecoder();
	QString input = encMapper->toUnicode( data.data(), data.size() );
	// ### unexpected EOF? (for incremental parsing)
	// starts the document with an XML declaration?
	if ( input.find("<?xml") == 0 ) {
	    // try to find out if there is an encoding
	    int endPos = input.find( ">" );
	    int pos = input.find( "encoding" );
	    if ( pos < endPos && pos != -1 ) {
		QString encoding;
		do {
		    pos++;
		    if ( pos > endPos ) {
			return input;
		    }
		} while( input[pos] != '"' && input[pos] != '\'' );
		pos++;
		while( input[pos] != '"' && input[pos] != '\'' ) {
		    encoding += input[pos];
		    pos++;
		    if ( pos > endPos ) {
			return input;
		    }
		}

		codec = QTextCodec::codecForName( encoding );
		if ( codec == 0 ) {
		    return input;
		}
		delete encMapper;
		encMapper = codec->makeDecoder();
		return encMapper->toUnicode( data.data(), data.size() );
	    }
	}
	return input;
    }
    return encMapper->toUnicode( data.data(), data.size() );
}


/*********************************************
 *
 * QXmlDefaultHandler
 *
 *********************************************/

/*!
  \class QXmlContentHandler qxml.h
  \brief The QXmlContentHandler class provides an interface to report logical
  content of XML data.

  \module XML

  If the application needs to be informed of basic parsing events, it
  implements this interface and sets it with QXmlReader::setContentHandler().
  The reader reports basic document-related events like the start and end of
  elements and character data through this interface.

  The order of events in this interface is very important, and mirrors the
  order of information in the document itself. For example, all of an element's
  content (character data, processing instructions, and/or subelements)
  appear, in order, between the startElement() event and the corresponding
  endElement() event.

  The class QXmlDefaultHandler gives a default implementation for this
  interface; subclassing from this class is very convenient if you want only be
  informed of some parsing events.

  See also the <a href="xml-sax.html#introSAX2">Introduction to SAX2</a>.

  \sa QXmlDTDHandler QXmlDeclHandler QXmlEntityResolver QXmlErrorHandler
  QXmlLexicalHandler
*/
/*!
  \fn void QXmlContentHandler::setDocumentLocator( QXmlLocator* locator )

  The reader calls this function before he starts parsing the document. The
  argument \a locator is a pointer to a QXmlLocator which allows the
  application to get the actual position of the parsing in the document.

  Do not destroy the \a locator; it is destroyed when the reader is destroyed
  (do not use the \a locator after the reader got destroyed).
*/
/*!
  \fn bool QXmlContentHandler::startDocument()

  The reader calls this function when he starts parsing the document.
  The reader calls this function only once before any other functions in
  this class or in the QXmlDTDHandler class are called (except
  QXmlContentHandler::setDocumentLocator()).

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  \sa endDocument()
*/
/*!
  \fn bool QXmlContentHandler::endDocument()

  The reader calls this function after he has finished the parsing. It
  is only called once. It is the last function of all handler functions that is
  called. It is called after the reader has read all input or has abandoned
  parsing because of a fatal error.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  \sa startDocument()
*/
/*!
  \fn bool QXmlContentHandler::startPrefixMapping( const QString& prefix, const QString& uri )

  The reader calls this function to signal the begin of a prefix-URI
  namespace mapping scope. This information is not necessary for normal
  namespace processing since the reader automatically replaces prefixes for
  element and attribute names.

  Note that startPrefixMapping and endPrefixMapping calls are not guaranteed to
  be properly nested relative to each-other: all startPrefixMapping events
  occur before the corresponding startElement event, and all endPrefixMapping
  events occur after the corresponding endElement event, but their order
  is not otherwise guaranteed.

  The argument \a prefix is the namespace prefix being declared and the
  argument \a uri is the namespace URI the prefix is mapped to.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.

  \sa endPrefixMapping()
*/
/*!
  \fn bool QXmlContentHandler::endPrefixMapping( const QString& prefix )

  The reader calls this function to signal the end of a prefix mapping for the
  prefix \a prefix.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.

  \sa startPrefixMapping()
*/
/*!
  \fn bool QXmlContentHandler::startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts )

  The reader calls this function when he has parsed a start element tag.

  There is a corresponding endElement() call when the corresponding end
  element tag was read. The startElement() and endElement() calls are always
  nested correctly. Empty element tags (e.g. &lt;a/&gt;) are reported by
  startElement() directly followed by a call to endElement().

  The attribute list provided contains only attributes with explicit
  values. The attribute list contains attributes used for namespace
  declaration (i.e. attributes starting with xmlns) only if the
  namespace-prefix property of the reader is TRUE.

  The argument \a namespaceURI is the namespace URI, or a null string if the
  element has no namespace URI or if no namespace processing is done, \a
  localName is the local name (without prefix), or a null string if no
  namespace processing is done, \a qName is the qualified name (with prefix)
  and \a atts are the attributes attached to the element. If there are no
  attributes, \a atts is an empty attributes object

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.

  \sa endElement()
*/
/*!
  \fn bool QXmlContentHandler::endElement( const QString& namespaceURI, const QString& localName, const QString& qName )

  The reader calls this function when he has parsed an end element tag with the
  qualified name \a qName, the local name \a localName and the namespace URI \a
  namespaceURI.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  See also the <a href="xml-sax.html#namespaces">namespace description</a>.

  \sa startElement()
*/
/*!
  \fn bool QXmlContentHandler::characters( const QString& ch )

  The reader calls this function when he has parsed a chunk of character
  data (either normal character data or character data inside a CDATA section;
  if you have to distinguish between those two types you have to use
  QXmlLexicalHandler::startCDATA() and QXmlLexicalHandler::endCDATA() in
  addition). The character data is reported in \a ch.

  Some readers report whitespace in element content using the
  ignorableWhitespace() function rather than this one (QXmlSimpleReader does
  not do it, though).

  A reader is allowed to report the character data of an element in more than
  one chunk; e.g. a reader might want to report "a &amp;lt; b" in three
  characters() events ("a ", "<" and " b").

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn bool QXmlContentHandler::ignorableWhitespace( const QString& ch )

  Some readers may use this function to report each chunk of whitespace in
  element content (QXmlSimpleReader does not though). The whitespace reported
  in \a ch.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn bool QXmlContentHandler::processingInstruction( const QString& target, const QString& data )

  The reader calls this function when he has parsed a processing
  instruction.

  \a target is the target name of the processing instruction and \a data is the
  data of the processing instruction.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn bool QXmlContentHandler::skippedEntity( const QString& name )

  Some readers may skip entities if they have not seen the declarations (e.g.
  because they are in an external DTD). If they do so they report that they
  skipped the entity with the name \a name by calling this function.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn QString QXmlContentHandler::errorString()

  The reader calls this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlErrorHandler qxml.h
  \brief The QXmlErrorHandler class provides an interface to report errors in
  XML data.

  \module XML

  If the application is interested in reporting errors to the user or any other
  customized error handling, you should subclass this class.

  You can set the error handler with QXmlReader::setErrorHandler().

  See also the <a href="xml-sax.html#introSAX2">Introduction to SAX2</a>.

  \sa QXmlDTDHandler QXmlDeclHandler QXmlContentHandler QXmlEntityResolver
  QXmlLexicalHandler
*/
/*!
  \fn bool QXmlErrorHandler::warning( const QXmlParseException& exception )

  A reader might use this function to report a warning. Warnings are conditions
  that are not errors or fatal errors as defined by the XML 1.0 specification.
  Details of the warning are stored in \a exception.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn bool QXmlErrorHandler::error( const QXmlParseException& exception )

  A reader might use this function to report a recoverable error. A recoverable
  error corresponds to the definiton of "error" in section 1.2 of the XML 1.0
  specification. Details of the error are stored in \a exception.

  The reader must continue to provide normal parsing events after invoking this
  function.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn bool QXmlErrorHandler::fatalError( const QXmlParseException& exception )

  A reader must use this function to report a non-recoverable error. Details
  of the error are stored in \a exception.

  If this function returns TRUE the reader might try to go on parsing and
  reporting further errors; but no regular parsing events are reported.
*/
/*!
  \fn QString QXmlErrorHandler::errorString()

  The reader calls this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlDTDHandler qxml.h
  \brief The QXmlDTDHandler class provides an interface to report DTD content
  of XML data.

  \module XML

  If an application needs information about notations and unparsed entities,
  then the application implements this interface and registers an instance with
  QXmlReader::setDTDHandler().

  Note that this interface includes only those DTD events that the XML
  recommendation requires processors to report: notation and unparsed entity
  declarations.

  See also the <a href="xml-sax.html#introSAX2">Introduction to SAX2</a>.

  \sa QXmlDeclHandler QXmlContentHandler QXmlEntityResolver QXmlErrorHandler
  QXmlLexicalHandler
*/
/*!
  \fn bool QXmlDTDHandler::notationDecl( const QString& name, const QString& publicId, const QString& systemId )

  The reader calls this function when he has parsed a notation declaration.

  The argument \a name is the notation name, \a publicId is the notations's
  public identifier and \a systemId is the notations's system identifier.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn bool QXmlDTDHandler::unparsedEntityDecl( const QString& name, const QString& publicId, const QString& systemId, const QString& notationName )

  The reader calls this function when he finds an unparsed entity declaration.

  The argument \a name is the unparsed entity's name, \a publicId is the
  entity's public identifier, \a systemId is the entity's system identifier and
  \a notationName is the name of the associated notation.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn QString QXmlDTDHandler::errorString()

  The reader calls this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlEntityResolver qxml.h
  \brief The QXmlEntityResolver class provides an interface to resolve extern
  entities contained in XML data.

  \module XML

  If an application needs to implement customized handling for external
  entities, it must implement this interface and register it with
  QXmlReader::setEntityResolver().

  See also the <a href="xml-sax.html#introSAX2">Introduction to SAX2</a>.

  \sa QXmlDTDHandler QXmlDeclHandler QXmlContentHandler QXmlErrorHandler
  QXmlLexicalHandler
*/
/*!
  \fn bool QXmlEntityResolver::resolveEntity( const QString& publicId, const QString& systemId, QXmlInputSource*& ret )

  The reader calls this function before he opens any external entity,
  except the top-level document entity. The application may request the reader
  to resolve the entity itself (\a ret is 0) or to use an entirely different
  input source (\a ret points to the input source).

  The reader deletes the input source \a ret when he no longer needs it. So
  you should allocate it on the heap with \c new.

  The argument \a publicId is the public identifier of the external entity, \a
  systemId is the system identifier of the external entity and \a ret is the
  return value of this function: if it is 0 the reader should resolve the
  entity itself, if it is non-zero it must point to an input source which the
  reader uses instead.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn QString QXmlEntityResolver::errorString()

  The reader calls this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlLexicalHandler qxml.h
  \brief The QXmlLexicalHandler class provides an interface to report lexical
  content of XML data.

  \module XML

  The events in the lexical handler apply to the entire document, not just to
  the document element, and all lexical handler events appear between the
  content handler's startDocument and endDocument events.

  You can set the lexical handler with QXmlReader::setLexicalHandler().

  This interface is designed after the SAX2 extension LexicalHandler. The
  functions startEntity() and endEntity() are not included though.

  See also the <a href="xml-sax.html#introSAX2">Introduction to SAX2</a>.

  \sa QXmlDTDHandler QXmlDeclHandler QXmlContentHandler QXmlEntityResolver
  QXmlErrorHandler
*/
/*!
  \fn bool QXmlLexicalHandler::startDTD( const QString& name, const QString& publicId, const QString& systemId )

  The reader calls this function to report the start of a DTD declaration, if
  any. It reports the name of the document type in \a name, the public
  identifier in \a publicId and the system identifier in \a systemId.

  If the public identifier resp. the system identifier is missing, the reader
  sets \a publicId resp. \a systemId to QString::null.

  All declarations reported through QXmlDTDHandler or QXmlDeclHandler appear
  between the startDTD() and endDTD() calls.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  \sa endDTD()
*/
/*!
  \fn bool QXmlLexicalHandler::endDTD()

  The reader calls this function to report the end of a DTD declaration, if
  any.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  \sa startDTD()
*/
/*!
  \fn bool QXmlLexicalHandler::startCDATA()

  The reader calls this function to report the start of a CDATA section. The
  content of the CDATA section is reported through the regular
  QXmlContentHandler::characters(). This function is intended only to report
  the boundary.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  \sa endCDATA()
*/
/*!
  \fn bool QXmlLexicalHandler::endCDATA()

  The reader calls this function to report the end of a CDATA section.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.

  \sa startCDATA()
*/
/*!
  \fn bool QXmlLexicalHandler::comment( const QString& ch )

  The reader calls this function to report an XML comment anywhere in the
  document. It reports the text of the comment in \a ch.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn QString QXmlLexicalHandler::errorString()

  The reader calls this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlDeclHandler qxml.h
  \brief The QXmlDeclHandler class provides an interface to report declaration
  content of XML data.

  \module XML

  You can set the declaration handler with QXmlReader::setDeclHandler().

  This interface is designed after the SAX2 extension DeclHandler.

  See also the <a href="xml-sax.html#introSAX2">Introduction to SAX2</a>.

  \sa QXmlDTDHandler QXmlContentHandler QXmlEntityResolver QXmlErrorHandler
  QXmlLexicalHandler
*/
/*!
  \fn bool QXmlDeclHandler::attributeDecl( const QString& eName, const QString& aName, const QString& type, const QString& valueDefault, const QString& value )

  The reader calls this function to report an attribute type declaration. Only
  the effective (first) declaration for an attribute is reported.

  The reader passes the name of the associated element in \a eName, the name of
  the attribute in \a aName. It passes a string that represents the attribute
  type in \a type and a string that represents the attribute default in \a
  valueDefault. This string is either "#IMPLIED", "#REQUIRED", "#FIXED" or null
  (if none of the other applies). Finally, the reader passes the attribute's
  default value in \a value. If no default value is specified in the XML file,
  \a value is QString::null.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn bool QXmlDeclHandler::internalEntityDecl( const QString& name, const QString& value )

  The reader calls this function to report an internal entity declaration. Only
  the effective (first) declaration is reported.

  The reader passes the name of the entity in \a name and the value of the
  entity in \a value.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn bool QXmlDeclHandler::externalEntityDecl( const QString& name, const QString& publicId, const QString& systemId )

  The reader calls this function to report a parsed external entity
  declaration. Only the effective (first) declaration for each entity is
  reported.

  The reader passes the name of the entity in \a name, the public identifier in
  \a publicId and the system identifier in \a systemId. If there is no public
  identifier specified, it passes QString::null in \a publicId.

  If this function returns FALSE the reader stops parsing and reports
  an error. The reader uses the function errorString() to get the error
  message that is used for reporting the error.
*/
/*!
  \fn QString QXmlDeclHandler::errorString()

  The reader calls this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlDefaultHandler qxml.h
  \brief The QXmlDefaultHandler class provides a default implementation of all
  XML handler classes.

  \module XML

  Very often you are only interested in parts of the things that that the
  reader reports to you. This class simply implements a default behaviour of
  the handler classes (most of the time: do nothing). Normally this is the
  class you subclass for implementing your customized handler.

  See also the <a href="xml-sax.html#introSAX2">Introduction to SAX2</a>.

  \sa QXmlDTDHandler QXmlDeclHandler QXmlContentHandler QXmlEntityResolver
  QXmlErrorHandler QXmlLexicalHandler
*/
/*!
  \fn QXmlDefaultHandler::QXmlDefaultHandler()

  Constructor.
*/
/*!
  \fn QXmlDefaultHandler::~QXmlDefaultHandler()

  Destructor.
*/

/*!  \reimp
  Does nothing.
*/
void QXmlDefaultHandler::setDocumentLocator( QXmlLocator* )
{
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::startDocument()
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::endDocument()
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::startPrefixMapping( const QString&, const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::endPrefixMapping( const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::startElement( const QString&, const QString&,
	const QString&, const QXmlAttributes& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::endElement( const QString&, const QString&,
	const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::characters( const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::ignorableWhitespace( const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::processingInstruction( const QString&,
	const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::skippedEntity( const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::warning( const QXmlParseException& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::error( const QXmlParseException& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::fatalError( const QXmlParseException& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::notationDecl( const QString&, const QString&,
	const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::unparsedEntityDecl( const QString&, const QString&,
	const QString&, const QString& )
{
    return TRUE;
}

/*!  \reimp
  Sets \a ret always to 0, so that the reader uses the system identifier
  provided in the XML document.
*/
bool QXmlDefaultHandler::resolveEntity( const QString&, const QString&,
	QXmlInputSource*& ret )
{
    ret = 0;
    return TRUE;
}

/*!  \reimp
  Returns the default error string.
*/
QString QXmlDefaultHandler::errorString()
{
    return QString( XMLERR_ERRORBYCONSUMER );
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::startDTD( const QString&, const QString&, const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::endDTD()
{
    return TRUE;
}

#if 0
/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::startEntity( const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::endEntity( const QString& )
{
    return TRUE;
}
#endif

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::startCDATA()
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::endCDATA()
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::comment( const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::attributeDecl( const QString&, const QString&, const QString&, const QString&, const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::internalEntityDecl( const QString&, const QString& )
{
    return TRUE;
}

/*!  \reimp
  Does nothing.
*/
bool QXmlDefaultHandler::externalEntityDecl( const QString&, const QString&, const QString& )
{
    return TRUE;
}


/*********************************************
 *
 * QXmlSimpleReaderPrivate
 *
 *********************************************/

class QXmlSimpleReaderPrivate
{
private:
    // functions
    QXmlSimpleReaderPrivate()
    {
	parseStack = 0;
    }

    ~QXmlSimpleReaderPrivate()
    {
	delete parseStack;
    }

    void initIncrementalParsing()
    {
	delete parseStack;
	parseStack = new QStack<ParseState>;
	parseStack->setAutoDelete( TRUE );
    }

    // used to determine if elements are correctly nested
    QValueStack<QString> tags;

    // used for entity declarations
    struct ExternParameterEntity
    {
	ExternParameterEntity( ) {}
	ExternParameterEntity( const QString &p, const QString &s )
	    : publicId(p), systemId(s) {}
	QString publicId;
	QString systemId;
    };
    struct ExternEntity
    {
	ExternEntity( ) {}
	ExternEntity( const QString &p, const QString &s, const QString &n )
	    : publicId(p), systemId(s), notation(n) {}
	QString publicId;
	QString systemId;
	QString notation;
    };
    QMap<QString,ExternParameterEntity> externParameterEntities;
    QMap<QString,QString> parameterEntities;
    QMap<QString,ExternEntity> externEntities;
    QMap<QString,QString> entities;

    // used for standalone declaration
    enum Standalone { Yes, No, Unknown };

    QString doctype; // only used for the doctype
    QString xmlVersion; // only used to store the version information
    QString encoding; // only used to store the encoding
    Standalone standalone; // used to store the value of the standalone declaration

    QString publicId; // used by parseExternalID() to store the public ID
    QString systemId; // used by parseExternalID() to store the system ID
    QString attDeclEName; // use by parseAttlistDecl()
    QString attDeclAName; // use by parseAttlistDecl()

    // flags for some features support
    bool useNamespaces;
    bool useNamespacePrefixes;
    bool reportWhitespaceCharData;

    // used to build the attribute list
    QXmlAttributes attList;

    // helper classes
    QXmlLocator *locator;
    QXmlNamespaceSupport namespaceSupport;

    // error string
    QString error;

    // arguments for parse functions (this is needed to allow incremental
    // parsing)
    bool parsePI_xmldecl;
    bool parseName_useRef;
    bool parseReference_charDataRead;
    QXmlSimpleReader::EntityRecognitionContext parseReference_context;
    bool parseExternalID_allowPublicID;
    QXmlSimpleReader::EntityRecognitionContext parsePEReference_context;
    QString parseString_s;

    // for incremental parsing
    struct ParseState {
	typedef bool (QXmlSimpleReader::*ParseFunction) ();
	ParseFunction function;
	int state;
    };
    QStack<ParseState> *parseStack;

    // friend declarations
    friend class QXmlSimpleReader;
};


/*********************************************
 *
 * QXmlSimpleReader
 *
 *********************************************/

/*!
  \class QXmlReader qxml.h
  \brief The QXmlReader class provides an interface for XML readers (i.e.
  parsers).

  \module XML

  This abstract class describes an interface for all XML readers in Qt. At the
  moment there is only one implementation of a reader included in the XML
  module of Qt (QXmlSimpleReader). In future releases there might be more
  readers with different properties available (e.g. a validating parser).

  The design of the XML classes follow the
  <a href="http://www.megginson.com/SAX/">SAX2 java interface</a>.
  It was adopted to fit into the Qt naming conventions; so it should be very
  easy for anybody who has worked with SAX2 to get started with the Qt XML
  classes.

  All readers use the class QXmlInputSource to read the input document from.
  Since you are normally interested in certain contents of the XML document,
  the reader reports those contents through special handler classes
  (QXmlDTDHandler, QXmlDeclHandler, QXmlContentHandler, QXmlEntityResolver,
  QXmlErrorHandler and QXmlLexicalHandler).

  You have to subclass these classes. Since the handler classes describe only
  interfaces you must implement all functions; there is a class
  (QXmlDefaultHandler) to make this easier; it implements a default behaviour
  (do nothing) for all functions.

  For getting started see also the
  \link xml-sax-walkthrough.html tiny SAX2 parser walkthrough. \endlink

  \sa QXmlSimpleReader
*/
/*!
  \fn bool QXmlReader::feature( const QString& name, bool *ok ) const

  If the reader has the feature \a name, this function returns the value of the
  feature.

  If the reader has not the feature \a name, the return value may be anything.

  If \a ok is not 0, then \a ok  is set to TRUE if the reader has the feature
  \a name, otherwise \a ok is set to FALSE.

  \sa setFeature() hasFeature()
*/
/*!
  \fn void QXmlReader::setFeature( const QString& name, bool value )

  Sets the feature \a name to \a value. If the reader has not the feature \a
  name, this value is ignored.

  \sa feature() hasFeature()
*/
/*!
  \fn bool QXmlReader::hasFeature( const QString& name ) const

  Returns \c TRUE if the reader has the feature \a name, otherwise FALSE.

  \sa feature() setFeature()
*/
/*!
  \fn void* QXmlReader::property( const QString& name, bool *ok ) const

  If the reader has the property \a name, this function returns the value of
  the property.

  If the reader has not the property \a name, the return value is 0.

  If \a ok is not 0, then \a ok  is set to TRUE if the reader has the property
  \a name, otherwise \a ok is set to FALSE.

  \sa setProperty() hasProperty()
*/
/*!
  \fn void QXmlReader::setProperty( const QString& name, void* value )

  Sets the property \a name to \a value. If the reader has not the property \a
  name, this value is ignored.

  \sa property() hasProperty()
*/
/*!
  \fn bool QXmlReader::hasProperty( const QString& name ) const

  Returns TRUE if the reader has the property \a name, otherwise FALSE.

  \sa property() setProperty()
*/
/*!
  \fn void QXmlReader::setEntityResolver( QXmlEntityResolver* handler )

  Sets the entity resolver to \a handler.

  \sa entityResolver()
*/
/*!
  \fn QXmlEntityResolver* QXmlReader::entityResolver() const

  Returns the entity resolver or 0 if none was set.

  \sa setEntityResolver()
*/
/*!
  \fn void QXmlReader::setDTDHandler( QXmlDTDHandler* handler )

  Sets the DTD handler to \a handler.

  \sa DTDHandler()
*/
/*!
  \fn QXmlDTDHandler* QXmlReader::DTDHandler() const

  Returns the DTD handler or 0 if none was set.

  \sa setDTDHandler()
*/
/*!
  \fn void QXmlReader::setContentHandler( QXmlContentHandler* handler )

  Sets the content handler to \a handler.

  \sa contentHandler()
*/
/*!
  \fn QXmlContentHandler* QXmlReader::contentHandler() const

  Returns the content handler or 0 if none was set.

  \sa setContentHandler()
*/
/*!
  \fn void QXmlReader::setErrorHandler( QXmlErrorHandler* handler )

  Sets the error handler to \a handler. Clears the error handler if \a
  handler is null.

  \sa errorHandler()
*/
/*!
  \fn QXmlErrorHandler* QXmlReader::errorHandler() const

  Returns the error handler or 0 if none was set

  \sa setErrorHandler()
*/
/*!
  \fn void QXmlReader::setLexicalHandler( QXmlLexicalHandler* handler )

  Sets the lexical handler to \a handler.

  \sa lexicalHandler()
*/
/*!
  \fn QXmlLexicalHandler* QXmlReader::lexicalHandler() const

  Returns the lexical handler or 0 if none was set.

  \sa setLexicalHandler()
*/
/*!
  \fn void QXmlReader::setDeclHandler( QXmlDeclHandler* handler )

  Sets the declaration handler to \a handler.

  \sa declHandler()
*/
/*!
  \fn QXmlDeclHandler* QXmlReader::declHandler() const

  Returns the declaration handler or 0 if none was set.

  \sa setDeclHandler()
*/
/*!
  \fn bool QXmlReader::parse( const QXmlInputSource& input )

  Parses the XML document \a input. Returns TRUE if the parsing was successful,
  otherwise FALSE.
*/


/*!
  \class QXmlSimpleReader qxml.h
  \brief The QXmlSimpleReader class provides an implementation of a simple XML
  reader (i.e. parser).

  \module XML

  This XML reader is sufficient for simple parsing tasks. Here is a short list
  of the properties of this reader:
  <ul>
  <li> well-formed parser
  <li> does not parse any external entities
  <li> can do namespace processing
  </ul>

  To get started see also the
  \link xml-sax-walkthrough.html tiny SAX2 parser walkthrough. \endlink
*/


/*!
  Constructs a simple XML reader with the following feature settings:
  <ul>
  <li> \e http://xml.org/sax/features/namespaces TRUE
  <li> \e http://xml.org/sax/features/namespace-prefixes FALSE
  <li> \e http://trolltech.com/xml/features/report-whitespace-only-CharData TRUE
  </ul>

  More information about features can be found in the \link xml-sax.html#features
  Qt SAX2 overview. \endlink

  \sa setFeature()
*/
QXmlSimpleReader::QXmlSimpleReader()
{
    d = new QXmlSimpleReaderPrivate();
    d->locator = new QXmlLocator( this );

    entityRes  = 0;
    dtdHnd     = 0;
    contentHnd = 0;
    errorHnd   = 0;
    lexicalHnd = 0;
    declHnd    = 0;

    // default feature settings
    d->useNamespaces = TRUE;
    d->useNamespacePrefixes = FALSE;
    d->reportWhitespaceCharData = TRUE;
}

/*!
  Destroys a simple XML reader.
*/
QXmlSimpleReader::~QXmlSimpleReader()
{
    delete d->locator;
    delete d;
}

/*!  \reimp
*/
bool QXmlSimpleReader::feature( const QString& name, bool *ok ) const
{
    if ( ok != 0 )
	*ok = TRUE;
    if        ( name == "http://xml.org/sax/features/namespaces" ) {
	return d->useNamespaces;
    } else if ( name == "http://xml.org/sax/features/namespace-prefixes" ) {
	return d->useNamespacePrefixes;
    } else if ( name == "http://trolltech.com/xml/features/report-whitespace-only-CharData" ) {
	return d->reportWhitespaceCharData;
    } else {
	qWarning( "Unknown feature %s", name.latin1() );
	if ( ok != 0 )
	    *ok = FALSE;
    }
    return FALSE;
}

/*!  \reimp
  Sets the state of the feature \a name to \a value:

  \walkthrough xml/tagreader-with-features/tagreader.cpp
  \skipto reader
  \printline reader
  \skipto setFeature
  \printline setFeature

  (Code taken from xml/tagreader-with-features/tagreader.cpp)

  If the feature is not recognized, it is ignored.

  The following features are supported:
  <ul>
  <li> \e http://xml.org/sax/features/namespaces:
       if this feature is TRUE, namespace processing is performed
  <li> \e http://xml.org/sax/features/namespace-prefixes:
       if this feature is TRUE, the the original prefixed names and attributes
       used for namespace declarations are reported
  <li> \e http://trolltech.com/xml/features/report-whitespace-only-CharData:
       if this feature is TRUE, CharData that consist of whitespace only (and
       no other characters) are not reported via
       QXmlContentHandler::characters()
  </ul>

  \sa feature() hasFeature()
*/
void QXmlSimpleReader::setFeature( const QString& name, bool value )
{
    if        ( name == "http://xml.org/sax/features/namespaces" ) {
	d->useNamespaces = value;
    } else if ( name == "http://xml.org/sax/features/namespace-prefixes" ) {
	d->useNamespacePrefixes = value;
    } else if ( name == "http://trolltech.com/xml/features/report-whitespace-only-CharData" ) {
	d->reportWhitespaceCharData = value;
    } else {
	qWarning( "Unknown feature %s", name.latin1() );
    }
}

/*!  \reimp
  Returns TRUE if the class has a feature named \a name, otherwise FALSE.

  \sa setFeature() feature()
*/
bool QXmlSimpleReader::hasFeature( const QString& name ) const
{
    if (    name == "http://xml.org/sax/features/namespaces" ||
	    name == "http://xml.org/sax/features/namespace-prefixes" ||
	    name == "http://trolltech.com/xml/features/report-whitespace-only-CharData" ) {
	return TRUE;
    } else {
	return FALSE;
    }
}

/*!  \reimp
  Returns 0 since this class does not support any properties.
*/
void* QXmlSimpleReader::property( const QString&, bool *ok ) const
{
    if ( ok != 0 )
	*ok = FALSE;
    return 0;
}

/*!  \reimp
  Does nothing since this class does not support any properties.
*/
void QXmlSimpleReader::setProperty( const QString&, void* )
{
}

/*!  \reimp
  Returns FALSE since this class does not support any properties.
*/
bool QXmlSimpleReader::hasProperty( const QString& ) const
{
    return FALSE;
}

/*! \reimp */
void QXmlSimpleReader::setEntityResolver( QXmlEntityResolver* handler )
{ entityRes = handler; }

/*! \reimp */
QXmlEntityResolver* QXmlSimpleReader::entityResolver() const
{ return entityRes; }

/*! \reimp */
void QXmlSimpleReader::setDTDHandler( QXmlDTDHandler* handler )
{ dtdHnd = handler; }

/*! \reimp */
QXmlDTDHandler* QXmlSimpleReader::DTDHandler() const
{ return dtdHnd; }

/*! \reimp */
void QXmlSimpleReader::setContentHandler( QXmlContentHandler* handler )
{ contentHnd = handler; }

/*! \reimp */
QXmlContentHandler* QXmlSimpleReader::contentHandler() const
{ return contentHnd; }

/*! \reimp */
void QXmlSimpleReader::setErrorHandler( QXmlErrorHandler* handler )
{ errorHnd = handler; }

/*! \reimp */
QXmlErrorHandler* QXmlSimpleReader::errorHandler() const
{ return errorHnd; }

/*! \reimp */
void QXmlSimpleReader::setLexicalHandler( QXmlLexicalHandler* handler )
{ lexicalHnd = handler; }

/*! \reimp */
QXmlLexicalHandler* QXmlSimpleReader::lexicalHandler() const
{ return lexicalHnd; }

/*! \reimp */
void QXmlSimpleReader::setDeclHandler( QXmlDeclHandler* handler )
{ declHnd = handler; }

/*! \reimp */
QXmlDeclHandler* QXmlSimpleReader::declHandler() const
{ return declHnd; }



/*! \reimp */
bool QXmlSimpleReader::parse( const QXmlInputSource& input )
{
    return parse( input, FALSE );
}

/*! \overload
  Parses the XML document input. Returns FALSE if the parsing detects an
  error.

  If \a incremental is TRUE, the parser does not return FALSE when it reaches
  the end of the \a input without reaching the end of the XML file. It rather
  stores the state of the parser so that parsing can be continued at a later
  state when more data is available. You can use the function parseContinue()
  to continue with parsing. If \a incremental is TRUE and the class was used
  before to do incremental parsing, the state of that parsing session is lost:
  this function will always start a new parsing session.

  If \a incremental is FALSE, this function behaves like the normal parse
  function, i.e. it returns FALSE when the end of input is reached without
  reaching the end of the XML file and the parsing can't be continued.

  \sa parseContinue() QSocket
*/
// ### How to detect when end is reached? (If incremental is TRUE, the
// returned TRUE doesn't mean that parsing is finished!)
bool QXmlSimpleReader::parse( const QXmlInputSource& input, bool incremental )
{
    init( input );
    if ( incremental ) {
	d->initIncrementalParsing();
    } else {
	delete d->parseStack;
	d->parseStack = 0;
    }
    // call the handler
    if ( contentHnd ) {
	contentHnd->setDocumentLocator( d->locator );
	if ( !contentHnd->startDocument() ) {
	    reportParseError( contentHnd->errorString() );
	    d->tags.clear();
	    return FALSE;
	}
    }
    return parseBeginOrContinue( 0, incremental );
}

/*!
  Continues incremental parsing; this function reads the input from the
  QXmlInputSource that was specified with the last parse() command.  To use
  this function, you must have called parse() with the incremental argument set
  to TRUE (otherwise this function returns FALSE).

  If the input source returns an empty string for the function
  QXmlInputSource::data(), then this means that the end of the XML file is
  reached; this is quite important, especially if you want to use the reader to
  parse more than one XML file.

  This function returns FALSE in the case of a parsing error. The case that the
  end of the XML file is reached without having finished the parsing is also an
  error. Otherwise this function returns TRUE. A return value of TRUE does not
  mean that the parsing is finished. Use ### instead to determine if the
  parsing is really finished.

  \sa parse()
*/
bool QXmlSimpleReader::parseContinue()
{
    if ( d->parseStack == 0 ) {
	return FALSE;
    }
    if ( !d->parseStack->isEmpty() ) {
	initData();
	int state = state = d->parseStack->top()->state;
	d->parseStack->pop();
	return parseBeginOrContinue( state, TRUE );
    }
    return FALSE; // this should never happen
}

/*
  Common part of parse() and parseContinue()
*/
bool QXmlSimpleReader::parseBeginOrContinue( int state, bool incremental )
{
    if ( state==0 ) {
	if ( !parseProlog() ) {
	    if ( incremental && d->error.isNull() ) {
		pushParseState( 0, 0 );
		return TRUE;
	    } else {
		d->tags.clear();
		return FALSE;
	    }
	}
	state = 1;
    }
    if ( state==1 ) {
	if ( !parseElement() ) {
	    if ( incremental && d->error.isNull() ) {
		pushParseState( 0, 1 );
		return TRUE;
	    } else {
		d->tags.clear();
		return FALSE;
	    }
	}
	state = 2;
    }
    // parse Misc*
    while ( !atEnd() ) {
	if ( !parseMisc() ) {
	    if ( incremental && d->error.isNull() ) {
		pushParseState( 0, 2 );
		return TRUE;
	    } else {
		d->tags.clear();
		return FALSE;
	    }
	}
    }
    // is stack empty?
    if ( !d->tags.isEmpty() && !d->error.isNull() ) {
	// ### can this case happen at all?
	reportParseError( XMLERR_UNEXPECTEDEOF );
	d->tags.clear();
	return FALSE;
    }
    // call the handler
    if ( contentHnd ) {
	if ( !contentHnd->endDocument() ) {
	    reportParseError( contentHnd->errorString() );
	    return FALSE;
	}
    }
    return TRUE;
}

//
// The following private parse functions have another semantics for the return
// value: They return TRUE iff parsing has finished successfully (i.e. the end
// of the XML file must be reached!). If one of these functions return FALSE,
// there is only an error when d->error.isNULL() is also FALSE.
//

/*
  For the incremental parsing, it is very important that the parse...()
  functions have a certain structure. Since it might be hard to understand how
  they work, here is a description of the layout of these functions:

    bool QXmlSimpleReader::parse...()
    {
(1)	const signed char Init             = 0;
	...

(2)	const signed char Inp...           = 0;
	...

(3)	static signed char table[3][2] = {
	...
	};
	signed char state;
	signed char input;

(4)	if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
(4a)	...
	} else {
(4b)	...
	}

	while ( TRUE ) {
(5)	    switch ( state ) {
	    ...
	    }

(6)
(6a)	    if ( atEnd() ) {
		unexpectedEof( &QXmlSimpleReader::parseNmtoken, state );
		return FALSE;
	    }
(6b)	    if ( is_NameChar(c) ) {
	    ...
	    }
(7)	    state = table[state][input];

(8)	    switch ( state ) {
	    ...
	    }
	}
    }

  Explanation:
  ad 1: constants for the states (used in the transition table)
  ad 2: constants for the input (used in the transition table)
  ad 3: the transition table for the state machine
  ad 4: test if we are in a parseContinue() step
	a) if no, do inititalizations
	b) if yes, restore the state and call parse functions recursively
  ad 5: Do some actions according to the state; from the logical execution
	order, this code belongs after 8 (see there for an explanation)
  ad 6: Check the character that is at the actual "cursor" position:
	a) If we reached the EOF, report either error or push the state (in the
	   case of incremental parsing).
	b) Otherwise, set the input character constant for the transition
	   table.
  ad 7: Get the new state according to the input that was read.
  ad 8: Do some actions according to the state. The last line in every case
	statement reads new data (i.e. it move the cursor). This can also be
	done by calling another parse...() funtion. If you need processing for
	this state after that, you have to put it into the switch statement 5.
	This ensures that you have a well defined re-entry point, when you ran
	out of data.
*/

/*
  Parses the prolog [22].
*/
bool QXmlSimpleReader::parseProlog()
{
    static bool xmldecl_possible;
    static bool doctype_read;

    const signed char Init             = 0;
    const signed char EatWS            = 1; // eat white spaces
    const signed char Lt               = 2; // '<' read
    const signed char Em               = 3; // '!' read
    const signed char DocType          = 4; // read doctype
    const signed char Comment          = 5; // read comment
    const signed char PI               = 6; // read PI
    const signed char Done             = 7;

    const signed char InpWs            = 0;
    const signed char InpLt            = 1; // <
    const signed char InpQm            = 2; // ?
    const signed char InpEm            = 3; // !
    const signed char InpD             = 4; // D
    const signed char InpDash          = 5; // -
    const signed char InpUnknown       = 6;

    static signed char table[7][7] = {
     /*  InpWs   InpLt  InpQm  InpEm  InpD      InpDash  InpUnknown */
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // Init
	{ -1,     Lt,    -1,    -1,    -1,       -1,       -1      }, // EatWS
	{ -1,     -1,    PI,    Em,    Done,     -1,       Done    }, // Lt
	{ -1,     -1,    -1,    -1,    DocType,  Comment,  -1      }, // Em
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // DocType
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // Comment
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }  // PI
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	xmldecl_possible = TRUE;
	doctype_read = FALSE;
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseProlog (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseProlog, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case DocType:
		if ( doctype_read ) {
		    reportParseError( XMLERR_MORETHANONEDOCTYPE );
		    return FALSE;
		} else {
		    doctype_read = FALSE;
		}
		break;
	    case Comment:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		break;
	    case PI:
		// call the handler
		if ( contentHnd ) {
		    if ( xmldecl_possible && !d->xmlVersion.isEmpty() ) {
			QString value( "version = '" );
			value += d->xmlVersion;
			value += "'";
			if ( !d->encoding.isEmpty() ) {
			    value += " encoding = '";
			    value += d->encoding;
			    value += "'";
			}
			if ( d->standalone == QXmlSimpleReaderPrivate::Yes ) {
			    value += " standalone = 'yes'";
			} else if ( d->standalone == QXmlSimpleReaderPrivate::No ) {
			    value += " standalone = 'no'";
			}
			if ( !contentHnd->processingInstruction( "xml", value ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    } else {
			if ( !contentHnd->processingInstruction( name(), string() ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		// XML declaration only on first position possible
		xmldecl_possible = FALSE;
		break;
	    case Done:
		return TRUE;
	    case -1:
		reportParseError( XMLERR_ERRORPARSINGELEMENT );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseProlog, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == '<' ) {
	    input = InpLt;
	} else if ( c == '?' ) {
	    input = InpQm;
	} else if ( c == '!' ) {
	    input = InpEm;
	} else if ( c == 'D' ) {
	    input = InpD;
	} else if ( c == '-' ) {
	    input = InpDash;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case EatWS:
		// XML declaration only on first position possible
		xmldecl_possible = FALSE;
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseProlog, state );
		    return FALSE;
		}
		break;
	    case Lt:
		next();
		break;
	    case Em:
		// XML declaration only on first position possible
		xmldecl_possible = FALSE;
		next();
		break;
	    case DocType:
		if ( !parseDoctype() ) {
		    parseFailed( &QXmlSimpleReader::parseProlog, state );
		    return FALSE;
		}
		break;
	    case Comment:
		if ( !parseComment() ) {
		    parseFailed( &QXmlSimpleReader::parseProlog, state );
		    return FALSE;
		}
		break;
	    case PI:
		d->parsePI_xmldecl = xmldecl_possible;
		if ( !parsePI() ) {
		    parseFailed( &QXmlSimpleReader::parseProlog, state );
		    return FALSE;
		}
		break;
	}
    }
}

/*
  Parse an element [39].

  Precondition: the opening '<' is already read.
*/
bool QXmlSimpleReader::parseElement()
{
    const signed char Init             =  0;
    const signed char ReadName         =  1;
    const signed char Ws1              =  2;
    const signed char STagEnd          =  3;
    const signed char STagEnd2         =  4;
    const signed char ETagBegin        =  5;
    const signed char ETagBegin2       =  6;
    const signed char Ws2              =  7;
    const signed char EmptyTag         =  8;
    const signed char Attribute        =  9;
    const signed char Ws3              = 10;
    const signed char Done             = 11;

    const signed char InpWs            = 0; // whitespace
    const signed char InpNameBe        = 1; // is_NameBeginning()
    const signed char InpGt            = 2; // >
    const signed char InpSlash         = 3; // /
    const signed char InpUnknown       = 4;

    static signed char table[11][5] = {
     /*  InpWs      InpNameBe    InpGt        InpSlash     InpUnknown */
	{ -1,        ReadName,    -1,          -1,          -1        }, // Init
	{ Ws1,       Attribute,   STagEnd,     EmptyTag,    -1        }, // ReadName
	{ -1,        Attribute,   STagEnd,     EmptyTag,    -1        }, // Ws1
	{ STagEnd2,  STagEnd2,    STagEnd2,    STagEnd2,    STagEnd2  }, // STagEnd
	{ -1,        -1,          -1,          ETagBegin,   -1        }, // STagEnd2
	{ -1,        ETagBegin2,  -1,          -1,          -1        }, // ETagBegin
	{ Ws2,       -1,          Done,        -1,          -1        }, // ETagBegin2
	{ -1,        -1,          Done,        -1,          -1        }, // Ws2
	{ -1,        -1,          Done,        -1,          -1        }, // EmptyTag
	{ Ws3,       Attribute,   STagEnd,     EmptyTag,    -1        }, // Attribute
	{ -1,        Attribute,   STagEnd,     EmptyTag,    -1        }  // Ws3
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseElement (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseElement, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case ReadName:
		// store it on the stack
		d->tags.push( name() );
		// empty the attributes
		d->attList.qnameList.clear();
		d->attList.uriList.clear();
		d->attList.localnameList.clear();
		d->attList.valueList.clear();
		if ( d->useNamespaces ) {
		    d->namespaceSupport.pushContext();
		}
		break;
	    case ETagBegin2:
		if ( !processElementETagBegin2() )
		    return FALSE;
		break;
	    case Attribute:
		if ( !processElementAttribute() )
		    return FALSE;
		break;
	    case Done:
		return TRUE;
	    case -1:
		reportParseError( XMLERR_ERRORPARSINGELEMENT );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseElement, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( is_NameBeginning(c) ) {
	    input = InpNameBe;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else if ( c == '/' ) {
	    input = InpSlash;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case ReadName:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case Ws1:
	    case Ws2:
	    case Ws3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case STagEnd:
		// call the handler
		if ( contentHnd ) {
		    if ( d->useNamespaces ) {
			QString uri, lname;
			d->namespaceSupport.processName( d->tags.top(), FALSE, uri, lname );
			if ( !contentHnd->startElement( uri, lname, d->tags.top(), d->attList ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    } else {
			if ( !contentHnd->startElement( QString::null, QString::null, d->tags.top(), d->attList ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		next();
		break;
	    case STagEnd2:
		if ( !parseContent() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case ETagBegin:
		next();
		break;
	    case ETagBegin2:
		// get the name of the tag
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case EmptyTag:
		if  ( d->tags.isEmpty() ) {
		    reportParseError( XMLERR_TAGMISMATCH );
		    return FALSE;
		}
		if ( !processElementEmptyTag() )
		    return FALSE;
		next();
		break;
	    case Attribute:
		// get name and value of attribute
		if ( !parseAttribute() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}
/*
  Helper to break down the size of the code in the case statement.
  Return FALSE on error, otherwise TRUE.
*/
bool QXmlSimpleReader::processElementEmptyTag()
{
    QString uri, lname;
    // pop the stack and call the handler
    if ( contentHnd ) {
	if ( d->useNamespaces ) {
	    // report startElement first...
	    d->namespaceSupport.processName( d->tags.top(), FALSE, uri, lname );
	    if ( !contentHnd->startElement( uri, lname, d->tags.top(), d->attList ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	    // ... followed by endElement...
	    if ( !contentHnd->endElement( uri, lname, d->tags.pop() ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	    // ... followed by endPrefixMapping
	    QStringList prefixesBefore, prefixesAfter;
	    if ( contentHnd ) {
		prefixesBefore = d->namespaceSupport.prefixes();
	    }
	    d->namespaceSupport.popContext();
	    // call the handler for prefix mapping
	    prefixesAfter = d->namespaceSupport.prefixes();
	    for ( QStringList::Iterator it = prefixesBefore.begin(); it != prefixesBefore.end(); ++it ) {
		if ( prefixesAfter.contains(*it) == 0 ) {
		    if ( !contentHnd->endPrefixMapping( *it ) ) {
			reportParseError( contentHnd->errorString() );
			return FALSE;
		    }
		}
	    }
	} else {
	    // report startElement first...
	    if ( !contentHnd->startElement( QString::null, QString::null, d->tags.top(), d->attList ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	    // ... followed by endElement
	    if ( !contentHnd->endElement( QString::null, QString::null, d->tags.pop() ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	}
    } else {
	d->tags.pop();
	d->namespaceSupport.popContext();
    }
    return TRUE;
}
/*
  Helper to break down the size of the code in the case statement.
  Return FALSE on error, otherwise TRUE.
*/
bool QXmlSimpleReader::processElementETagBegin2()
{
    // pop the stack and compare it with the name
    if ( d->tags.pop() != name() ) {
	reportParseError( XMLERR_TAGMISMATCH );
	return FALSE;
    }
    // call the handler
    if ( contentHnd ) {
	if ( d->useNamespaces ) {
	    QString uri, lname;
	    d->namespaceSupport.processName( name(), FALSE, uri, lname );
	    if ( !contentHnd->endElement( uri, lname, name() ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	} else {
	    if ( !contentHnd->endElement( QString::null, QString::null, name() ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	}
    }
    if ( d->useNamespaces ) {
	QStringList prefixesBefore, prefixesAfter;
	if ( contentHnd ) {
	    prefixesBefore = d->namespaceSupport.prefixes();
	}
	d->namespaceSupport.popContext();
	// call the handler for prefix mapping
	if ( contentHnd ) {
	    prefixesAfter = d->namespaceSupport.prefixes();
	    for ( QStringList::Iterator it = prefixesBefore.begin(); it != prefixesBefore.end(); ++it ) {
		if ( prefixesAfter.contains(*it) == 0 ) {
		    if ( !contentHnd->endPrefixMapping( *it ) ) {
			reportParseError( contentHnd->errorString() );
			return FALSE;
		    }
		}
	    }
	}
    }
    return TRUE;
}
/*
  Helper to break down the size of the code in the case statement.
  Return FALSE on error, otherwise TRUE.
*/
bool QXmlSimpleReader::processElementAttribute()
{
    QString uri, lname, prefix;
    // add the attribute to the list
    if ( d->useNamespaces ) {
	// is it a namespace declaration?
	d->namespaceSupport.splitName( name(), prefix, lname );
	if ( prefix == "xmlns" ) {
	    // namespace declaration
	    d->namespaceSupport.setPrefix( lname, string() );
	    if ( d->useNamespacePrefixes ) {
		d->attList.qnameList.append( name() );
		d->attList.uriList.append( QString::null );
		d->attList.localnameList.append( QString::null );
		d->attList.valueList.append( string() );
	    }
	    // call the handler for prefix mapping
	    if ( contentHnd ) {
		if ( !contentHnd->startPrefixMapping( lname, string() ) ) {
		    reportParseError( contentHnd->errorString() );
		    return FALSE;
		}
	    }
	} else {
	    // no namespace delcaration
	    d->namespaceSupport.processName( name(), TRUE, uri, lname );
	    d->attList.qnameList.append( name() );
	    d->attList.uriList.append( uri );
	    d->attList.localnameList.append( lname );
	    d->attList.valueList.append( string() );
	}
    } else {
	// no namespace support
	d->attList.qnameList.append( name() );
	d->attList.uriList.append( QString::null );
	d->attList.localnameList.append( QString::null );
	d->attList.valueList.append( string() );
    }
    return TRUE;
}

/*
  Parse a content [43].

  A content is only used between tags. If a end tag is found the < is already
  read and the head stand on the '/' of the end tag '</name>'.
*/
bool QXmlSimpleReader::parseContent()
{
    static bool charDataRead;

    const signed char Init             =  0;
    const signed char ChD              =  1; // CharData
    const signed char ChD1             =  2; // CharData help state
    const signed char ChD2             =  3; // CharData help state
    const signed char Ref              =  4; // Reference
    const signed char Lt               =  5; // '<' read
    const signed char PI               =  6; // PI
    const signed char Elem             =  7; // Element
    const signed char Em               =  8; // '!' read
    const signed char Com              =  9; // Comment
    const signed char CDS              = 10; // CDSect
    const signed char CDS1             = 11; // read a CDSect
    const signed char CDS2             = 12; // read a CDSect (help state)
    const signed char CDS3             = 13; // read a CDSect (help state)
    const signed char Done             = 14; // finished reading content

    const signed char InpLt            = 0; // <
    const signed char InpGt            = 1; // >
    const signed char InpSlash         = 2; // /
    const signed char InpQMark         = 3; // ?
    const signed char InpEMark         = 4; // !
    const signed char InpAmp           = 5; // &
    const signed char InpDash          = 6; // -
    const signed char InpOpenB         = 7; // [
    const signed char InpCloseB        = 8; // ]
    const signed char InpUnknown       = 9;

    static signed char mapCLT2FSMChar[] = {
	InpUnknown, // white space
	InpUnknown, // %
	InpAmp,     // &
	InpGt,      // >
	InpLt,      // <
	InpSlash,   // /
	InpQMark,   // ?
	InpEMark,   // !
	InpDash,    // -
	InpCloseB,  // ]
	InpOpenB,   // [
	InpUnknown, // =
	InpUnknown, // "
	InpUnknown, // '
	InpUnknown  // unknown
    };

    static signed char const table[14][10] = {
     /*  InpLt  InpGt  InpSlash  InpQMark  InpEMark  InpAmp  InpDash  InpOpenB  InpCloseB  InpUnknown */
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD1,      ChD  }, // Init
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD1,      ChD  }, // ChD
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD2,      ChD  }, // ChD1
	{ Lt,    -1,    ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD2,      ChD  }, // ChD2
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // Ref (same as Init)
	{ -1,    -1,    Done,     PI,       Em,       -1,     -1,      -1,       -1,        Elem }, // Lt
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // PI (same as Init)
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // Elem (same as Init)
	{ -1,    -1,    -1,       -1,       -1,       -1,     Com,     CDS,      -1,        -1   }, // Em
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // Com (same as Init)
	{ CDS1,  CDS1,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS2,      CDS1 }, // CDS
	{ CDS1,  CDS1,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS2,      CDS1 }, // CDS1
	{ CDS1,  CDS1,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS3,      CDS1 }, // CDS2
	{ CDS1,  Init,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS3,      CDS1 }  // CDS3
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	charDataRead = FALSE;
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseContent (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseContent, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case PI:
		if ( contentHnd ) {
		    if ( !contentHnd->processingInstruction(name(),string()) ) {
			reportParseError( contentHnd->errorString() );
			return FALSE;
		    }
		}
		break;
	    case Com:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		break;
	    case CDS:
		// empty string
		stringClear();
		break;
	    case CDS2:
		if (c != ']') {
		    stringAddC( ']' );
		}
		break;
	    case CDS3:
		// test if this skipping was legal
		if        ( c == '>' ) {
		    // the end of the CDSect
		    if ( lexicalHnd ) {
			if ( !lexicalHnd->startCDATA() ) {
			    reportParseError( lexicalHnd->errorString() );
			    return FALSE;
			}
		    }
		    if ( contentHnd ) {
			if ( !contentHnd->characters( string() ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    }
		    if ( lexicalHnd ) {
			if ( !lexicalHnd->endCDATA() ) {
			    reportParseError( lexicalHnd->errorString() );
			    return FALSE;
			}
		    }
		} else if (c == ']') {
		    // three or more ']'
		    stringAddC( ']' );
		} else {
		    // after ']]' comes another character
		    stringAddC( ']' );
		    stringAddC( ']' );
		}
		break;
	    case Done:
		// call the handler for CharData
		if ( contentHnd ) {
		    if ( charDataRead ) {
			if ( d->reportWhitespaceCharData || !string().simplifyWhiteSpace().isEmpty() ) {
			    if ( !contentHnd->characters( string() ) ) {
				reportParseError( contentHnd->errorString() );
				return FALSE;
			    }
			}
		    }
		}
		// Done
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_ERRORPARSINGCONTENT );
		return FALSE;
	}

	// get input (use lookup-table instead of nested ifs for performance
	// reasons)
	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseContent, state );
	    return FALSE;
	}
	if ( c.row() ) {
	    input = InpUnknown;
	} else {
	    input = mapCLT2FSMChar[ charLookupTable[ c.cell() ] ];
	}
	state = table[state][input];

	switch ( state ) {
	    case ChD:
		// on first call: clear string
		if ( !charDataRead ) {
		    charDataRead = TRUE;
		    stringClear();
		}
		stringAddC();
		next();
		break;
	    case ChD1:
		// on first call: clear string
		if ( !charDataRead ) {
		    charDataRead = TRUE;
		    stringClear();
		}
		stringAddC();
		next();
		break;
	    case ChD2:
		stringAddC();
		next();
		break;
	    case Ref:
		if ( !charDataRead) {
		    // reference may be CharData; so clear string to be safe
		    stringClear();
		    d->parseReference_context = InContent;
		    if ( !parseReference() ) {
			parseFailed( &QXmlSimpleReader::parseContent, state );
			return FALSE;
		    }
		    charDataRead = d->parseReference_charDataRead;
		} else {
		    d->parseReference_context = InContent;
		    if ( !parseReference() ) {
			parseFailed( &QXmlSimpleReader::parseContent, state );
			return FALSE;
		    }
		}
		break;
	    case Lt:
		// call the handler for CharData
		if ( contentHnd ) {
		    if ( charDataRead ) {
			if ( d->reportWhitespaceCharData || !string().simplifyWhiteSpace().isEmpty() ) {
			    if ( !contentHnd->characters( string() ) ) {
				reportParseError( contentHnd->errorString() );
				return FALSE;
			    }
			}
		    }
		}
		charDataRead = FALSE;
		next();
		break;
	    case PI:
		d->parsePI_xmldecl = FALSE;
		if ( !parsePI() ) {
		    parseFailed( &QXmlSimpleReader::parseContent, state );
		    return FALSE;
		}
		break;
	    case Elem:
		if ( !parseElement() ) {
		    parseFailed( &QXmlSimpleReader::parseContent, state );
		    return FALSE;
		}
		break;
	    case Em:
		next();
		break;
	    case Com:
		if ( !parseComment() ) {
		    parseFailed( &QXmlSimpleReader::parseContent, state );
		    return FALSE;
		}
		break;
	    case CDS:
		d->parseString_s = "[CDATA[";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseContent, state );
		    return FALSE;
		}
		break;
	    case CDS1:
		// read one character and add it
		stringAddC();
		next();
		break;
	    case CDS2:
		// skip ']'
		next();
		break;
	    case CDS3:
		// skip ']'...
		next();
		break;
	}
    }
}

/*
  Parse Misc [27].
*/
bool QXmlSimpleReader::parseMisc()
{
    const signed char Init             = 0;
    const signed char Lt               = 1; // '<' was read
    const signed char Comment          = 2; // read comment
    const signed char eatWS            = 3; // eat whitespaces
    const signed char PI               = 4; // read PI
    const signed char Comment2         = 5; // read comment

    const signed char InpWs            = 0; // S
    const signed char InpLt            = 1; // <
    const signed char InpQm            = 2; // ?
    const signed char InpEm            = 3; // !
    const signed char InpUnknown       = 4;

    static signed char table[3][5] = {
     /*  InpWs   InpLt  InpQm  InpEm     InpUnknown */
	{ eatWS,  Lt,    -1,    -1,       -1        }, // Init
	{ -1,     -1,    PI,    Comment,  -1        }, // Lt
	{ -1,     -1,    -1,    -1,       Comment2  }  // Comment
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseMisc (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseMisc, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case eatWS:
		return TRUE;
	    case PI:
		if ( contentHnd ) {
		    if ( !contentHnd->processingInstruction(name(),string()) ) {
			reportParseError( contentHnd->errorString() );
			return FALSE;
		    }
		}
		return TRUE;
	    case Comment2:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseMisc, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == '<' ) {
	    input = InpLt;
	} else if ( c == '?' ) {
	    input = InpQm;
	} else if ( c == '!' ) {
	    input = InpEm;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case eatWS:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseMisc, state );
		    return FALSE;
		}
		break;
	    case Lt:
		next();
		break;
	    case PI:
		d->parsePI_xmldecl = FALSE;
		if ( !parsePI() ) {
		    parseFailed( &QXmlSimpleReader::parseMisc, state );
		    return FALSE;
		}
		break;
	    case Comment:
		next();
		break;
	    case Comment2:
		if ( !parseComment() ) {
		    parseFailed( &QXmlSimpleReader::parseMisc, state );
		    return FALSE;
		}
		break;
	}
    }
}

/*
  Parse a processing instruction [16].

  If xmldec is TRUE, it tries to parse a PI or a XML declaration [23].

  Precondition: the beginning '<' of the PI is already read and the head stand
  on the '?' of '<?'.

  If this funktion was successful, the head-position is on the first
  character after the PI.
*/
bool QXmlSimpleReader::parsePI()
{
    const signed char Init             =  0;
    const signed char QmI              =  1; // ? was read
    const signed char Name             =  2; // read Name
    const signed char XMLDecl          =  3; // read XMLDecl
    const signed char Ws1              =  4; // eat ws after "xml" of XMLDecl
    const signed char PI               =  5; // read PI
    const signed char Ws2              =  6; // eat ws after Name of PI
    const signed char Version          =  7; // read versionInfo
    const signed char Ws3              =  8; // eat ws after versionInfo
    const signed char EorSD            =  9; // read EDecl or SDDecl
    const signed char Ws4              = 10; // eat ws after EDecl or SDDecl
    const signed char SD               = 11; // read SDDecl
    const signed char Ws5              = 12; // eat ws after SDDecl
    const signed char ADone            = 13; // almost done
    const signed char Char             = 14; // Char was read
    const signed char Qm               = 15; // Qm was read
    const signed char Done             = 16; // finished reading content

    const signed char InpWs            = 0; // whitespace
    const signed char InpNameBe        = 1; // is_nameBeginning()
    const signed char InpGt            = 2; // >
    const signed char InpQm            = 3; // ?
    const signed char InpUnknown       = 4;

    static signed char table[16][5] = {
     /*  InpWs,  InpNameBe  InpGt  InpQm   InpUnknown  */
	{ -1,     -1,        -1,    QmI,    -1     }, // Init
	{ -1,     Name,      -1,    -1,     -1     }, // QmI
	{ -1,     -1,        -1,    -1,     -1     }, // Name (this state is left not through input)
	{ Ws1,    -1,        -1,    -1,     -1     }, // XMLDecl
	{ -1,     Version,   -1,    -1,     -1     }, // Ws1
	{ Ws2,    -1,        -1,    Qm,     -1     }, // PI
	{ Char,   Char,      Char,  Qm,     Char   }, // Ws2
	{ Ws3,    -1,        -1,    ADone,  -1     }, // Version
	{ -1,     EorSD,     -1,    ADone,  -1     }, // Ws3
	{ Ws4,    -1,        -1,    ADone,  -1     }, // EorSD
	{ -1,     SD,        -1,    ADone,  -1     }, // Ws4
	{ Ws5,    -1,        -1,    ADone,  -1     }, // SD
	{ -1,     -1,        -1,    ADone,  -1     }, // Ws5
	{ -1,     -1,        Done,  -1,     -1     }, // ADone
	{ Char,   Char,      Char,  Qm,     Char   }, // Char
	{ Char,   Char,      Done,  Qm,     Char   }, // Qm
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parsePI (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parsePI, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Name:
		// test what name was read and determine the next state
		// (not very beautiful, I admit)
		if ( name().lower() == "xml" ) {
		    if ( d->parsePI_xmldecl && name()=="xml" ) {
			state = XMLDecl;
		    } else {
			reportParseError( XMLERR_INVALIDNAMEFORPI );
			return FALSE;
		    }
		} else {
		    state = PI;
		    stringClear();
		}
		break;
	    case Version:
		// get version (syntax like an attribute)
		if ( name() != "version" ) {
		    reportParseError( XMLERR_VERSIONEXPECTED );
		    return FALSE;
		}
		d->xmlVersion = string();
		break;
	    case EorSD:
		// get the EDecl or SDDecl (syntax like an attribute)
		if        ( name() == "standalone" ) {
		    if ( string()=="yes" ) {
			d->standalone = QXmlSimpleReaderPrivate::Yes;
		    } else if ( string()=="no" ) {
			d->standalone = QXmlSimpleReaderPrivate::No;
		    } else {
			reportParseError( XMLERR_WRONGVALUEFORSDECL );
			return FALSE;
		    }
		} else if ( name() == "encoding" ) {
		    d->encoding = string();
		} else {
		    reportParseError( XMLERR_EDECLORSDDECLEXPECTED );
		    return FALSE;
		}
		break;
	    case SD:
		if ( name() != "standalone" ) {
		    reportParseError( XMLERR_SDDECLEXPECTED );
		    return FALSE;
		}
		if ( string()=="yes" ) {
		    d->standalone = QXmlSimpleReaderPrivate::Yes;
		} else if ( string()=="no" ) {
		    d->standalone = QXmlSimpleReaderPrivate::No;
		} else {
		    reportParseError( XMLERR_WRONGVALUEFORSDECL );
		    return FALSE;
		}
		break;
	    case Qm:
		// test if the skipping was legal
		if ( c != '>' ) {
		    stringAddC( '?' );
		}
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parsePI, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( is_NameBeginning(c) ) {
	    input = InpNameBe;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else if ( c == '?' ) {
	    input = InpQm;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case QmI:
		next();
		break;
	    case Name:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case Ws1:
	    case Ws2:
	    case Ws3:
	    case Ws4:
	    case Ws5:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case Version:
		if ( !parseAttribute() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case EorSD:
		if ( !parseAttribute() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case SD:
		// get the SDDecl (syntax like an attribute)
		if ( d->standalone != QXmlSimpleReaderPrivate::Unknown ) {
		    // already parsed the standalone declaration
		    reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		    return FALSE;
		}
		if ( !parseAttribute() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case ADone:
		next();
		break;
	    case Char:
		stringAddC();
		next();
		break;
	    case Qm:
		// skip the '?'
		next();
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a document type definition (doctypedecl [28]).

  Precondition: the beginning '<!' of the doctype is already read the head
  stands on the 'D' of '<!DOCTYPE'.

  If this funktion was successful, the head-position is on the first
  character after the document type definition.
*/
bool QXmlSimpleReader::parseDoctype()
{
    static bool startDTDwasReported;

    const signed char Init             =  0;
    const signed char Doctype          =  1; // read the doctype
    const signed char Ws1              =  2; // eat_ws
    const signed char Doctype2         =  3; // read the doctype, part 2
    const signed char Ws2              =  4; // eat_ws
    const signed char Sys              =  5; // read SYSTEM or PUBLIC
    const signed char Ws3              =  6; // eat_ws
    const signed char MP               =  7; // markupdecl or PEReference
    const signed char PER              =  8; // PERReference
    const signed char Mup              =  9; // markupdecl
    const signed char Ws4              = 10; // eat_ws
    const signed char MPE              = 11; // end of markupdecl or PEReference
    const signed char Done             = 12;

    const signed char InpWs            = 0;
    const signed char InpD             = 1; // 'D'
    const signed char InpS             = 2; // 'S' or 'P'
    const signed char InpOB            = 3; // [
    const signed char InpCB            = 4; // ]
    const signed char InpPer           = 5; // %
    const signed char InpGt            = 6; // >
    const signed char InpUnknown       = 7;

    static signed char table[12][8] = {
     /*  InpWs,  InpD       InpS       InpOB  InpCB  InpPer InpGt  InpUnknown */
	{ -1,     Doctype,   -1,        -1,    -1,    -1,    -1,    -1        }, // Init
	{ Ws1,    Doctype2,  Doctype2,  -1,    -1,    -1,    -1,    Doctype2  }, // Doctype
	{ -1,     Doctype2,  Doctype2,  -1,    -1,    -1,    -1,    Doctype2  }, // Ws1
	{ Ws2,    -1,        Sys,       MP,    -1,    -1,    Done,  -1        }, // Doctype2
	{ -1,     -1,        Sys,       MP,    -1,    -1,    Done,  -1        }, // Ws2
	{ Ws3,    -1,        -1,        MP,    -1,    -1,    Done,  -1        }, // Sys
	{ -1,     -1,        -1,        MP,    -1,    -1,    Done,  -1        }, // Ws3
	{ -1,     -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // MP
	{ Ws4,    -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // PER
	{ Ws4,    -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // Mup
	{ -1,     -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // Ws4
	{ -1,     -1,        -1,        -1,    -1,    -1,    Done,  -1        }  // MPE
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	startDTDwasReported = FALSE;
	d->systemId = QString::null;
	d->publicId = QString::null;
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseDoctype (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseDoctype, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Doctype:
		if ( !is_S(c) ) {
		    reportParseError( XMLERR_ERRORPARSINGDOCTYPE );
		    return FALSE;
		}
		break;
	    case Doctype2:
		d->doctype = name();
		break;
	    case MP:
		if ( !startDTDwasReported && lexicalHnd  ) {
		    startDTDwasReported = TRUE;
		    if ( !lexicalHnd->startDTD( d->doctype, d->publicId, d->systemId ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_ERRORPARSINGDOCTYPE );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseDoctype, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == 'D' ) {
	    input = InpD;
	} else if ( c == 'S' ) {
	    input = InpS;
	} else if ( c == 'P' ) {
	    input = InpS;
	} else if ( c == '[' ) {
	    input = InpOB;
	} else if ( c == ']' ) {
	    input = InpCB;
	} else if ( c == '%' ) {
	    input = InpPer;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Doctype:
		d->parseString_s = "DOCTYPE";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Ws1:
	    case Ws2:
	    case Ws3:
	    case Ws4:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Doctype2:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Sys:
		d->parseExternalID_allowPublicID = FALSE;
		if ( !parseExternalID() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case MP:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case PER:
		d->parsePEReference_context = InDTD;
		if ( !parsePEReference() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Mup:
		if ( !parseMarkupdecl() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case MPE:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Done:
		if ( lexicalHnd ) {
		    if ( !startDTDwasReported ) {
			startDTDwasReported = TRUE;
			if ( !lexicalHnd->startDTD( d->doctype, d->publicId, d->systemId ) ) {
			    reportParseError( lexicalHnd->errorString() );
			    return FALSE;
			}
		    }
		    if ( !lexicalHnd->endDTD() ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		next();
		break;
	}
    }
}

/*
  Parse a ExternalID [75].

  If allowPublicID is TRUE parse ExternalID [75] or PublicID [83].
*/
bool QXmlSimpleReader::parseExternalID()
{
    const signed char Init             =  0;
    const signed char Sys              =  1; // parse 'SYSTEM'
    const signed char SysWS            =  2; // parse the whitespace after 'SYSTEM'
    const signed char SysSQ            =  3; // parse SystemLiteral with '
    const signed char SysSQ2           =  4; // parse SystemLiteral with '
    const signed char SysDQ            =  5; // parse SystemLiteral with "
    const signed char SysDQ2           =  6; // parse SystemLiteral with "
    const signed char Pub              =  7; // parse 'PUBLIC'
    const signed char PubWS            =  8; // parse the whitespace after 'PUBLIC'
    const signed char PubSQ            =  9; // parse PubidLiteral with '
    const signed char PubSQ2           = 10; // parse PubidLiteral with '
    const signed char PubDQ            = 11; // parse PubidLiteral with "
    const signed char PubDQ2           = 12; // parse PubidLiteral with "
    const signed char PubE             = 13; // finished parsing the PubidLiteral
    const signed char PubWS2           = 14; // parse the whitespace after the PubidLiteral
    const signed char PDone            = 15; // done if allowPublicID is TRUE
    const signed char Done             = 16;

    const signed char InpSQ            = 0; // '
    const signed char InpDQ            = 1; // "
    const signed char InpS             = 2; // S
    const signed char InpP             = 3; // P
    const signed char InpWs            = 4; // white space
    const signed char InpUnknown       = 5;

    static signed char table[15][6] = {
     /*  InpSQ    InpDQ    InpS     InpP     InpWs     InpUnknown */
	{ -1,      -1,      Sys,     Pub,     -1,       -1      }, // Init
	{ -1,      -1,      -1,      -1,      SysWS,    -1      }, // Sys
	{ SysSQ,   SysDQ,   -1,      -1,      -1,       -1      }, // SysWS
	{ Done,    SysSQ2,  SysSQ2,  SysSQ2,  SysSQ2,   SysSQ2  }, // SysSQ
	{ Done,    SysSQ2,  SysSQ2,  SysSQ2,  SysSQ2,   SysSQ2  }, // SysSQ2
	{ SysDQ2,  Done,    SysDQ2,  SysDQ2,  SysDQ2,   SysDQ2  }, // SysDQ
	{ SysDQ2,  Done,    SysDQ2,  SysDQ2,  SysDQ2,   SysDQ2  }, // SysDQ2
	{ -1,      -1,      -1,      -1,      PubWS,    -1      }, // Pub
	{ PubSQ,   PubDQ,   -1,      -1,      -1,       -1      }, // PubWS
	{ PubE,    -1,      PubSQ2,  PubSQ2,  PubSQ2,   PubSQ2  }, // PubSQ
	{ PubE,    -1,      PubSQ2,  PubSQ2,  PubSQ2,   PubSQ2  }, // PubSQ2
	{ -1,      PubE,    PubDQ2,  PubDQ2,  PubDQ2,   PubDQ2  }, // PubDQ
	{ -1,      PubE,    PubDQ2,  PubDQ2,  PubDQ2,   PubDQ2  }, // PubDQ2
	{ PDone,   PDone,   PDone,   PDone,   PubWS2,   PDone   }, // PubE
	{ SysSQ,   SysDQ,   PDone,   PDone,   PDone,    PDone   }  // PubWS2
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	d->systemId = QString::null;
	d->publicId = QString::null;
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseExternalID (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseExternalID, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case PDone:
		if ( d->parseExternalID_allowPublicID ) {
		    d->publicId = string();
		    return TRUE;
		} else {
		    reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		    return FALSE;
		}
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseExternalID, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == '\'' ) {
	    input = InpSQ;
	} else if ( c == '"' ) {
	    input = InpDQ;
	} else if ( c == 'S' ) {
	    input = InpS;
	} else if ( c == 'P' ) {
	    input = InpP;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Sys:
		d->parseString_s = "SYSTEM";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case SysWS:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case SysSQ:
	    case SysDQ:
		stringClear();
		next();
		break;
	    case SysSQ2:
	    case SysDQ2:
		stringAddC();
		next();
		break;
	    case Pub:
		d->parseString_s = "PUBLIC";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case PubWS:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case PubSQ:
	    case PubDQ:
		stringClear();
		next();
		break;
	    case PubSQ2:
	    case PubDQ2:
		stringAddC();
		next();
		break;
	    case PubE:
		next();
		break;
	    case PubWS2:
		d->publicId = string();
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case Done:
		d->systemId = string();
		next();
		break;
	}
    }
}

/*
  Parse a markupdecl [29].
*/
bool QXmlSimpleReader::parseMarkupdecl()
{
    const signed char Init             = 0;
    const signed char Lt               = 1; // < was read
    const signed char Em               = 2; // ! was read
    const signed char CE               = 3; // E was read
    const signed char Qm               = 4; // ? was read
    const signed char Dash             = 5; // - was read
    const signed char CA               = 6; // A was read
    const signed char CEL              = 7; // EL was read
    const signed char CEN              = 8; // EN was read
    const signed char CN               = 9; // N was read
    const signed char Done             = 10;

    const signed char InpLt            = 0; // <
    const signed char InpQm            = 1; // ?
    const signed char InpEm            = 2; // !
    const signed char InpDash          = 3; // -
    const signed char InpA             = 4; // A
    const signed char InpE             = 5; // E
    const signed char InpL             = 6; // L
    const signed char InpN             = 7; // N
    const signed char InpUnknown       = 8;

    static signed char table[4][9] = {
     /*  InpLt  InpQm  InpEm  InpDash  InpA   InpE   InpL   InpN   InpUnknown */
	{ Lt,    -1,    -1,    -1,      -1,    -1,    -1,    -1,    -1     }, // Init
	{ -1,    Qm,    Em,    -1,      -1,    -1,    -1,    -1,    -1     }, // Lt
	{ -1,    -1,    -1,    Dash,    CA,    CE,    -1,    CN,    -1     }, // Em
	{ -1,    -1,    -1,    -1,      -1,    -1,    CEL,   CEN,   -1     }  // CE
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseMarkupdecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Qm:
		if ( contentHnd ) {
		    if ( !contentHnd->processingInstruction(name(),string()) ) {
			reportParseError( contentHnd->errorString() );
			return FALSE;
		    }
		}
		return TRUE;
	    case Dash:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		return TRUE;
	    case CA:
		return TRUE;
	    case CEL:
		return TRUE;
	    case CEN:
		return TRUE;
	    case CN:
		return TRUE;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseMarkupdecl, state );
	    return FALSE;
	}
	if        ( c == '<' ) {
	    input = InpLt;
	} else if ( c == '?' ) {
	    input = InpQm;
	} else if ( c == '!' ) {
	    input = InpEm;
	} else if ( c == '-' ) {
	    input = InpDash;
	} else if ( c == 'A' ) {
	    input = InpA;
	} else if ( c == 'E' ) {
	    input = InpE;
	} else if ( c == 'L' ) {
	    input = InpL;
	} else if ( c == 'N' ) {
	    input = InpN;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Lt:
		next();
		break;
	    case Em:
		next();
		break;
	    case CE:
		next();
		break;
	    case Qm:
		d->parsePI_xmldecl = FALSE;
		if ( !parsePI() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case Dash:
		if ( !parseComment() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case CA:
		if ( !parseAttlistDecl() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case CEL:
		if ( !parseElementDecl() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case CEN:
		if ( !parseEntityDecl() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case CN:
		if ( !parseNotationDecl() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	}
    }
}

/*
  Parse a PEReference [69]
*/
bool QXmlSimpleReader::parsePEReference()
{
    const signed char Init             = 0;
    const signed char Next             = 1;
    const signed char Name             = 2;
    const signed char Done             = 3;

    const signed char InpSemi          = 0; // ;
    const signed char InpPer           = 1; // %
    const signed char InpUnknown       = 2;

    static signed char table[3][3] = {
     /*  InpSemi  InpPer  InpUnknown */
	{ -1,      Next,   -1    }, // Init
	{ -1,      -1,     Name  }, // Next
	{ Done,    -1,     -1    }  // Name
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parsePEReference (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parsePEReference, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Name:
		if ( d->parameterEntities.find( ref() ) == d->parameterEntities.end() ) {
		    // ### skip it???
		    if ( contentHnd ) {
			if ( !contentHnd->skippedEntity( QString("%") + ref() ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    }
		} else {
		    if ( d->parsePEReference_context == InEntityValue ) {
			// Included in literal
			xmlRef = d->parameterEntities.find( ref() )
			    .data().replace( QRegExp("\""), "&quot;" ).replace( QRegExp("'"), "&apos;" )
			    + xmlRef;
		    } else if ( d->parsePEReference_context == InDTD ) {
			// Included as PE
			xmlRef = QString(" ") +
			    d->parameterEntities.find( ref() ).data() +
			    QString(" ") + xmlRef;
		    }
		}
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parsePEReference, state );
	    return FALSE;
	}
	if        ( c == ';' ) {
	    input = InpSemi;
	} else if ( c == '%' ) {
	    input = InpPer;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Next:
		next();
		break;
	    case Name:
		d->parseName_useRef = TRUE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parsePEReference, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a AttlistDecl [52].

  Precondition: the beginning '<!' is already read and the head
  stands on the 'A' of '<!ATTLIST'
*/
bool QXmlSimpleReader::parseAttlistDecl()
{
    const signed char Init             =  0;
    const signed char Attlist          =  1; // parse the string "ATTLIST"
    const signed char Ws               =  2; // whitespace read
    const signed char Name             =  3; // parse name
    const signed char Ws1              =  4; // whitespace read
    const signed char Attdef           =  5; // parse the AttDef
    const signed char Ws2              =  6; // whitespace read
    const signed char Atttype          =  7; // parse the AttType
    const signed char Ws3              =  8; // whitespace read
    const signed char DDecH            =  9; // DefaultDecl with #
    const signed char DefReq           = 10; // parse the string "REQUIRED"
    const signed char DefImp           = 11; // parse the string "IMPLIED"
    const signed char DefFix           = 12; // parse the string "FIXED"
    const signed char Attval           = 13; // parse the AttValue
    const signed char Ws4              = 14; // whitespace read
    const signed char Done             = 15;

    const signed char InpWs            = 0; // white space
    const signed char InpGt            = 1; // >
    const signed char InpHash          = 2; // #
    const signed char InpA             = 3; // A
    const signed char InpI             = 4; // I
    const signed char InpF             = 5; // F
    const signed char InpR             = 6; // R
    const signed char InpUnknown       = 7;

    static signed char table[15][8] = {
     /*  InpWs    InpGt    InpHash  InpA      InpI     InpF     InpR     InpUnknown */
	{ -1,      -1,      -1,      Attlist,  -1,      -1,      -1,      -1      }, // Init
	{ Ws,      -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attlist
	{ -1,      -1,      -1,      Name,     Name,    Name,    Name,    Name    }, // Ws
	{ Ws1,     Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }, // Name
	{ -1,      Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }, // Ws1
	{ Ws2,     -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attdef
	{ -1,      Atttype, Atttype, Atttype,  Atttype, Atttype, Atttype, Atttype }, // Ws2
	{ Ws3,     -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attype
	{ -1,      Attval,  DDecH,   Attval,   Attval,  Attval,  Attval,  Attval  }, // Ws3
	{ -1,      -1,      -1,      -1,       DefImp,  DefFix,  DefReq,  -1      }, // DDecH
	{ Ws4,     Ws4,     -1,      -1,       -1,      -1,      -1,      -1      }, // DefReq
	{ Ws4,     Ws4,     -1,      -1,       -1,      -1,      -1,      -1      }, // DefImp
	{ Ws3,     -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // DefFix
	{ Ws4,     Ws4,     -1,      -1,       -1,      -1,      -1,      -1      }, // Attval
	{ -1,      Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }  // Ws4
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseAttlistDecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Name:
		d->attDeclEName = name();
		break;
	    case Attdef:
		d->attDeclAName = name();
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseAttlistDecl, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else if ( c == '#' ) {
	    input = InpHash;
	} else if ( c == 'A' ) {
	    input = InpA;
	} else if ( c == 'I' ) {
	    input = InpI;
	} else if ( c == 'F' ) {
	    input = InpF;
	} else if ( c == 'R' ) {
	    input = InpR;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Attlist:
		d->parseString_s = "ATTLIST";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Ws:
	    case Ws1:
	    case Ws2:
	    case Ws3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Name:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Attdef:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Atttype:
		if ( !parseAttType() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case DDecH:
		next();
		break;
	    case DefReq:
		d->parseString_s = "REQUIRED";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case DefImp:
		d->parseString_s = "IMPLIED";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case DefFix:
		d->parseString_s = "FIXED";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Attval:
		if ( !parseAttValue() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Ws4:
		if ( declHnd ) {
		    // ### not all values are computed yet...
		    if ( !declHnd->attributeDecl( d->attDeclEName, d->attDeclAName, "", "", "" ) ) {
			reportParseError( declHnd->errorString() );
			return FALSE;
		    }
		}
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a AttType [54]
*/
bool QXmlSimpleReader::parseAttType()
{
    const signed char Init             =  0;
    const signed char ST               =  1; // StringType
    const signed char TTI              =  2; // TokenizedType starting with 'I'
    const signed char TTI2             =  3; // TokenizedType helpstate
    const signed char TTI3             =  4; // TokenizedType helpstate
    const signed char TTE              =  5; // TokenizedType starting with 'E'
    const signed char TTEY             =  6; // TokenizedType starting with 'ENTITY'
    const signed char TTEI             =  7; // TokenizedType starting with 'ENTITI'
    const signed char N                =  8; // N read (TokenizedType or Notation)
    const signed char TTNM             =  9; // TokenizedType starting with 'NM'
    const signed char TTNM2            = 10; // TokenizedType helpstate
    const signed char NO               = 11; // Notation
    const signed char NO2              = 12; // Notation helpstate
    const signed char NO3              = 13; // Notation helpstate
    const signed char NOName           = 14; // Notation, read name
    const signed char NO4              = 15; // Notation helpstate
    const signed char EN               = 16; // Enumeration
    const signed char ENNmt            = 17; // Enumeration, read Nmtoken
    const signed char EN2              = 18; // Enumeration helpstate
    const signed char ADone            = 19; // almost done (make next and accept)
    const signed char Done             = 20;

    const signed char InpWs            =  0; // whitespace
    const signed char InpOp            =  1; // (
    const signed char InpCp            =  2; // )
    const signed char InpPipe          =  3; // |
    const signed char InpC             =  4; // C
    const signed char InpE             =  5; // E
    const signed char InpI             =  6; // I
    const signed char InpM             =  7; // M
    const signed char InpN             =  8; // N
    const signed char InpO             =  9; // O
    const signed char InpR             = 10; // R
    const signed char InpS             = 11; // S
    const signed char InpY             = 12; // Y
    const signed char InpUnknown       = 13;

    static signed char table[19][14] = {
     /*  InpWs    InpOp    InpCp    InpPipe  InpC     InpE     InpI     InpM     InpN     InpO     InpR     InpS     InpY     InpUnknown */
	{ -1,      EN,      -1,      -1,      ST,      TTE,     TTI,     -1,      N,       -1,      -1,      -1,      -1,      -1     }, // Init
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // ST
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    TTI2,    Done,    Done,    Done   }, // TTI
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    TTI3,    Done,    Done   }, // TTI2
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTI3
	{ -1,      -1,      -1,      -1,      -1,      -1,      TTEI,    -1,      -1,      -1,      -1,      -1,      TTEY,    -1     }, // TTE
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTEY
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTEI
	{ -1,      -1,      -1,      -1,      -1,      -1,      -1,      TTNM,    -1,      NO,      -1,      -1,      -1,      -1     }, // N
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    TTNM2,   Done,    Done   }, // TTNM
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTNM2
	{ NO2,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NO
	{ -1,      NO3,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NO2
	{ NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName }, // NO3
	{ NO4,     -1,      ADone,   NO3,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NOName
	{ -1,      -1,      ADone,   NO3,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NO4
	{ -1,      -1,      ENNmt,   -1,      ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt  }, // EN
	{ EN2,     -1,      ADone,   EN,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // ENNmt
	{ -1,      -1,      ADone,   EN,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }  // EN2
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseAttType (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseAttType, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case ADone:
		return TRUE;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseAttType, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == '(' ) {
	    input = InpOp;
	} else if ( c == ')' ) {
	    input = InpCp;
	} else if ( c == '|' ) {
	    input = InpPipe;
	} else if ( c == 'C' ) {
	    input = InpC;
	} else if ( c == 'E' ) {
	    input = InpE;
	} else if ( c == 'I' ) {
	    input = InpI;
	} else if ( c == 'M' ) {
	    input = InpM;
	} else if ( c == 'N' ) {
	    input = InpN;
	} else if ( c == 'O' ) {
	    input = InpO;
	} else if ( c == 'R' ) {
	    input = InpR;
	} else if ( c == 'S' ) {
	    input = InpS;
	} else if ( c == 'Y' ) {
	    input = InpY;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case ST:
		d->parseString_s = "CDATA";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTI:
		d->parseString_s = "ID";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTI2:
		d->parseString_s = "REF";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTI3:
		next(); // S
		break;
	    case TTE:
		d->parseString_s = "ENTIT";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTEY:
		next(); // Y
		break;
	    case TTEI:
		d->parseString_s = "IES";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case N:
		next(); // N
		break;
	    case TTNM:
		d->parseString_s = "MTOKEN";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTNM2:
		next(); // S
		break;
	    case NO:
		d->parseString_s = "OTATION";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case NO2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case NO3:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case NOName:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case NO4:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case EN:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case ENNmt:
		if ( !parseNmtoken() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case EN2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case ADone:
		next();
		break;
	}
    }
}

/*
  Parse a AttValue [10]

  Precondition: the head stands on the beginning " or '

  If this function was successful, the head stands on the first
  character after the closing " or ' and the value of the attribute
  is in string().
*/
bool QXmlSimpleReader::parseAttValue()
{
    const signed char Init             = 0;
    const signed char Dq               = 1; // double quotes were read
    const signed char DqRef            = 2; // read references in double quotes
    const signed char DqC              = 3; // signed character read in double quotes
    const signed char Sq               = 4; // single quotes were read
    const signed char SqRef            = 5; // read references in single quotes
    const signed char SqC              = 6; // signed character read in single quotes
    const signed char Done             = 7;

    const signed char InpDq            = 0; // "
    const signed char InpSq            = 1; // '
    const signed char InpAmp           = 2; // &
    const signed char InpLt            = 3; // <
    const signed char InpUnknown       = 4;

    static signed char table[7][5] = {
     /*  InpDq  InpSq  InpAmp  InpLt InpUnknown */
	{ Dq,    Sq,    -1,     -1,   -1    }, // Init
	{ Done,  DqC,   DqRef,  -1,   DqC   }, // Dq
	{ Done,  DqC,   DqRef,  -1,   DqC   }, // DqRef
	{ Done,  DqC,   DqRef,  -1,   DqC   }, // DqC
	{ SqC,   Done,  SqRef,  -1,   SqC   }, // Sq
	{ SqC,   Done,  SqRef,  -1,   SqC   }, // SqRef
	{ SqC,   Done,  SqRef,  -1,   SqC   }  // SqRef
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseAttValue (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseAttValue, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseAttValue, state );
	    return FALSE;
	}
	if        ( c == '"' ) {
	    input = InpDq;
	} else if ( c == '\'' ) {
	    input = InpSq;
	} else if ( c == '&' ) {
	    input = InpAmp;
	} else if ( c == '<' ) {
	    input = InpLt;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Dq:
	    case Sq:
		stringClear();
		next();
		break;
	    case DqRef:
	    case SqRef:
		d->parseReference_context = InAttributeValue;
		if ( !parseReference() ) {
		    parseFailed( &QXmlSimpleReader::parseAttValue, state );
		    return FALSE;
		}
		break;
	    case DqC:
	    case SqC:
		stringAddC();
		next();
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a elementdecl [45].

  Precondition: the beginning '<!E' is already read and the head
  stands on the 'L' of '<!ELEMENT'
*/
bool QXmlSimpleReader::parseElementDecl()
{
    const signed char Init             =  0;
    const signed char Elem             =  1; // parse the beginning string
    const signed char Ws1              =  2; // whitespace required
    const signed char Nam              =  3; // parse Name
    const signed char Ws2              =  4; // whitespace required
    const signed char Empty            =  5; // read EMPTY
    const signed char Any              =  6; // read ANY
    const signed char Cont             =  7; // read contentspec (except ANY or EMPTY)
    const signed char Mix              =  8; // read Mixed
    const signed char Mix2             =  9; //
    const signed char Mix3             = 10; //
    const signed char MixN1            = 11; //
    const signed char MixN2            = 12; //
    const signed char MixN3            = 13; //
    const signed char MixN4            = 14; //
    const signed char Cp               = 15; // parse cp
    const signed char Cp2              = 16; //
    const signed char WsD              = 17; // eat whitespace before Done
    const signed char Done             = 18;

    const signed char InpWs            =  0;
    const signed char InpGt            =  1; // >
    const signed char InpPipe          =  2; // |
    const signed char InpOp            =  3; // (
    const signed char InpCp            =  4; // )
    const signed char InpHash          =  5; // #
    const signed char InpQm            =  6; // ?
    const signed char InpAst           =  7; // *
    const signed char InpPlus          =  8; // +
    const signed char InpA             =  9; // A
    const signed char InpE             = 10; // E
    const signed char InpL             = 11; // L
    const signed char InpUnknown       = 12;

    static signed char table[18][13] = {
     /*  InpWs   InpGt  InpPipe  InpOp  InpCp   InpHash  InpQm  InpAst  InpPlus  InpA    InpE    InpL    InpUnknown */
	{ -1,     -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     Elem,   -1     }, // Init
	{ Ws1,    -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Elem
	{ -1,     -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      Nam,    Nam,    Nam,    Nam    }, // Ws1
	{ Ws2,    -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Nam
	{ -1,     -1,    -1,      Cont,  -1,     -1,      -1,    -1,     -1,      Any,    Empty,  -1,     -1     }, // Ws2
	{ WsD,    Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Empty
	{ WsD,    Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Any
	{ -1,     -1,    -1,      Cp,    Cp,     Mix,     -1,    -1,     -1,      Cp,     Cp,     Cp,     Cp     }, // Cont
	{ Mix2,   -1,    MixN1,   -1,    Mix3,   -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Mix
	{ -1,     -1,    MixN1,   -1,    Mix3,   -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Mix2
	{ WsD,    Done,  -1,      -1,    -1,     -1,      -1,    WsD,    -1,      -1,     -1,     -1,     -1     }, // Mix3
	{ -1,     -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      MixN2,  MixN2,  MixN2,  MixN2  }, // MixN1
	{ MixN3,  -1,    MixN1,   -1,    MixN4,  -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // MixN2
	{ -1,     -1,    MixN1,   -1,    MixN4,  -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // MixN3
	{ -1,     -1,    -1,      -1,    -1,     -1,      -1,    WsD,    -1,      -1,     -1,     -1,     -1     }, // MixN4
	{ WsD,    Done,  -1,      -1,    -1,     -1,      Cp2,   Cp2,    Cp2,     -1,     -1,     -1,     -1     }, // Cp
	{ WsD,    Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Cp2
	{ -1,     Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }  // WsD
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseElementDecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseElementDecl, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else if ( c == '|' ) {
	    input = InpPipe;
	} else if ( c == '(' ) {
	    input = InpOp;
	} else if ( c == ')' ) {
	    input = InpCp;
	} else if ( c == '#' ) {
	    input = InpHash;
	} else if ( c == '?' ) {
	    input = InpQm;
	} else if ( c == '*' ) {
	    input = InpAst;
	} else if ( c == '+' ) {
	    input = InpPlus;
	} else if ( c == 'A' ) {
	    input = InpA;
	} else if ( c == 'E' ) {
	    input = InpE;
	} else if ( c == 'L' ) {
	    input = InpL;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Elem:
		d->parseString_s = "LEMENT";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Ws1:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Nam:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Ws2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Empty:
		d->parseString_s = "EMPTY";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Any:
		d->parseString_s = "ANY";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Cont:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Mix:
		d->parseString_s = "#PCDATA";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Mix2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Mix3:
		next();
		break;
	    case MixN1:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case MixN2:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case MixN3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case MixN4:
		next();
		break;
	    case Cp:
		if ( !parseChoiceSeq() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Cp2:
		next();
		break;
	    case WsD:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a NotationDecl [82].

  Precondition: the beginning '<!' is already read and the head
  stands on the 'N' of '<!NOTATION'
*/
bool QXmlSimpleReader::parseNotationDecl()
{
    const signed char Init             = 0;
    const signed char Not              = 1; // read NOTATION
    const signed char Ws1              = 2; // eat whitespaces
    const signed char Nam              = 3; // read Name
    const signed char Ws2              = 4; // eat whitespaces
    const signed char ExtID            = 5; // parse ExternalID
    const signed char Ws3              = 6; // eat whitespaces
    const signed char Done             = 7;

    const signed char InpWs            = 0;
    const signed char InpGt            = 1; // >
    const signed char InpN             = 2; // N
    const signed char InpUnknown       = 3;

    static signed char table[7][4] = {
     /*  InpWs   InpGt  InpN    InpUnknown */
	{ -1,     -1,    Not,    -1     }, // Init
	{ Ws1,    -1,    -1,     -1     }, // Not
	{ -1,     -1,    Nam,    Nam    }, // Ws1
	{ Ws2,    Done,  -1,     -1     }, // Nam
	{ -1,     Done,  ExtID,  ExtID  }, // Ws2
	{ Ws3,    Done,  -1,     -1     }, // ExtID
	{ -1,     Done,  -1,     -1     }  // Ws3
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseNotationDecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case ExtID:
		// call the handler
		if ( dtdHnd ) {
		    if ( !dtdHnd->notationDecl( name(), d->publicId, d->systemId ) ) {
			reportParseError( dtdHnd->errorString() );
			return FALSE;
		    }
		}
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseNotationDecl, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else if ( c == 'N' ) {
	    input = InpN;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Not:
		d->parseString_s = "NOTATION";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Ws1:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Nam:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Ws2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case ExtID:
		d->parseExternalID_allowPublicID = TRUE;
		if ( !parseExternalID() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Ws3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse choice [49] or seq [50].

  Precondition: the beginning '('S? is already read and the head
  stands on the first non-whitespace character after it.
*/
bool QXmlSimpleReader::parseChoiceSeq()
{
    const signed char Init             = 0;
    const signed char Ws1              = 1; // eat whitespace
    const signed char CS               = 2; // choice or set
    const signed char Ws2              = 3; // eat whitespace
    const signed char More             = 4; // more cp to read
    const signed char Name             = 5; // read name
    const signed char Done             = 6; //

    const signed char InpWs            = 0; // S
    const signed char InpOp            = 1; // (
    const signed char InpCp            = 2; // )
    const signed char InpQm            = 3; // ?
    const signed char InpAst           = 4; // *
    const signed char InpPlus          = 5; // +
    const signed char InpPipe          = 6; // |
    const signed char InpComm          = 7; // ,
    const signed char InpUnknown       = 8;

    static signed char table[6][9] = {
     /*  InpWs   InpOp  InpCp  InpQm  InpAst  InpPlus  InpPipe  InpComm  InpUnknown */
	{ -1,     Ws1,   -1,    -1,    -1,     -1,      -1,      -1,      Name  }, // Init
	{ -1,     CS,    -1,    -1,    -1,     -1,      -1,      -1,      CS    }, // Ws1
	{ Ws2,    -1,    Done,  Ws2,   Ws2,    Ws2,     More,    More,    -1    }, // CS
	{ -1,     -1,    Done,  -1,    -1,     -1,      More,    More,    -1    }, // Ws2
	{ -1,     Ws1,   -1,    -1,    -1,     -1,      -1,      -1,      Name  }, // More (same as Init)
	{ Ws2,    -1,    Done,  Ws2,   Ws2,    Ws2,     More,    More,    -1    }  // Name (same as CS)
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseChoiceSeq (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseChoiceSeq, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == '(' ) {
	    input = InpOp;
	} else if ( c == ')' ) {
	    input = InpCp;
	} else if ( c == '?' ) {
	    input = InpQm;
	} else if ( c == '*' ) {
	    input = InpAst;
	} else if ( c == '+' ) {
	    input = InpPlus;
	} else if ( c == '|' ) {
	    input = InpPipe;
	} else if ( c == ',' ) {
	    input = InpComm;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Ws1:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case CS:
		if ( !parseChoiceSeq() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case Ws2:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case More:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case Name:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a EntityDecl [70].

  Precondition: the beginning '<!E' is already read and the head
  stand on the 'N' of '<!ENTITY'
*/
bool QXmlSimpleReader::parseEntityDecl()
{
    const signed char Init             =  0;
    const signed char Ent              =  1; // parse "ENTITY"
    const signed char Ws1              =  2; // white space read
    const signed char Name             =  3; // parse name
    const signed char Ws2              =  4; // white space read
    const signed char EValue           =  5; // parse entity value
    const signed char ExtID            =  6; // parse ExternalID
    const signed char Ws3              =  7; // white space read
    const signed char Ndata            =  8; // parse "NDATA"
    const signed char Ws4              =  9; // white space read
    const signed char NNam             = 10; // parse name
    const signed char PEDec            = 11; // parse PEDecl
    const signed char Ws6              = 12; // white space read
    const signed char PENam            = 13; // parse name
    const signed char Ws7              = 14; // white space read
    const signed char PEVal            = 15; // parse entity value
    const signed char PEEID            = 16; // parse ExternalID
    const signed char WsE              = 17; // white space read
    const signed char EDDone           = 19; // done, but also report an external, unparsed entity decl
    const signed char Done             = 18;

    const signed char InpWs            = 0; // white space
    const signed char InpPer           = 1; // %
    const signed char InpQuot          = 2; // " or '
    const signed char InpGt            = 3; // >
    const signed char InpN             = 4; // N
    const signed char InpUnknown       = 5;

    static signed char table[18][6] = {
     /*  InpWs  InpPer  InpQuot  InpGt  InpN    InpUnknown */
	{ -1,    -1,     -1,      -1,    Ent,    -1      }, // Init
	{ Ws1,   -1,     -1,      -1,    -1,     -1      }, // Ent
	{ -1,    PEDec,  -1,      -1,    Name,   Name    }, // Ws1
	{ Ws2,   -1,     -1,      -1,    -1,     -1      }, // Name
	{ -1,    -1,     EValue,  -1,    -1,     ExtID   }, // Ws2
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // EValue
	{ Ws3,   -1,     -1,      EDDone,-1,     -1      }, // ExtID
	{ -1,    -1,     -1,      EDDone,Ndata,  -1      }, // Ws3
	{ Ws4,   -1,     -1,      -1,    -1,     -1      }, // Ndata
	{ -1,    -1,     -1,      -1,    NNam,   NNam    }, // Ws4
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // NNam
	{ Ws6,   -1,     -1,      -1,    -1,     -1      }, // PEDec
	{ -1,    -1,     -1,      -1,    PENam,  PENam   }, // Ws6
	{ Ws7,   -1,     -1,      -1,    -1,     -1      }, // PENam
	{ -1,    -1,     PEVal,   -1,    -1,     PEEID   }, // Ws7
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // PEVal
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // PEEID
	{ -1,    -1,     -1,      Done,  -1,     -1      }  // WsE
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseEntityDecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case EValue:
		if (  !entityExist( name() ) ) {
		    d->entities.insert( name(), string() );
		    if ( declHnd ) {
			if ( !declHnd->internalEntityDecl( name(), string() ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		break;
	    case NNam:
		if (  !entityExist( name() ) ) {
		    d->externEntities.insert( name(), QXmlSimpleReaderPrivate::ExternEntity( d->publicId, d->systemId, ref() ) );
		    if ( dtdHnd ) {
			if ( !dtdHnd->unparsedEntityDecl( name(), d->publicId, d->systemId, ref() ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		break;
	    case PEVal:
		if (  !entityExist( name() ) ) {
		    d->parameterEntities.insert( name(), string() );
		    if ( declHnd ) {
			if ( !declHnd->internalEntityDecl( QString("%")+name(), string() ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		break;
	    case PEEID:
		if (  !entityExist( name() ) ) {
		    d->externParameterEntities.insert( name(), QXmlSimpleReaderPrivate::ExternParameterEntity( d->publicId, d->systemId ) );
		    if ( declHnd ) {
			if ( !declHnd->externalEntityDecl( QString("%")+name(), d->publicId, d->systemId ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		break;
	    case EDDone:
		if (  !entityExist( name() ) ) {
		    d->externEntities.insert( name(), QXmlSimpleReaderPrivate::ExternEntity( d->publicId, d->systemId, QString::null ) );
		    if ( declHnd ) {
			if ( !declHnd->externalEntityDecl( name(), d->publicId, d->systemId ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		return TRUE;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseEntityDecl, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c == '%' ) {
	    input = InpPer;
	} else if ( c == '"' || c == '\'' ) {
	    input = InpQuot;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else if ( c == 'N' ) {
	    input = InpN;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Ent:
		d->parseString_s = "NTITY";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws1:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Name:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case EValue:
		if ( !parseEntityValue() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case ExtID:
		d->parseExternalID_allowPublicID = FALSE;
		if ( !parseExternalID() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ndata:
		d->parseString_s = "NDATA";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws4:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case NNam:
		d->parseName_useRef = TRUE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case PEDec:
		next();
		break;
	    case Ws6:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case PENam:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws7:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case PEVal:
		if ( !parseEntityValue() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case PEEID:
		d->parseExternalID_allowPublicID = FALSE;
		if ( !parseExternalID() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case WsE:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case EDDone:
		next();
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a EntityValue [9]
*/
bool QXmlSimpleReader::parseEntityValue()
{
    const signed char Init             = 0;
    const signed char Dq               = 1; // EntityValue is double quoted
    const signed char DqC              = 2; // signed character
    const signed char DqPER            = 3; // PERefence
    const signed char DqRef            = 4; // Reference
    const signed char Sq               = 5; // EntityValue is double quoted
    const signed char SqC              = 6; // signed character
    const signed char SqPER            = 7; // PERefence
    const signed char SqRef            = 8; // Reference
    const signed char Done             = 9;

    const signed char InpDq            = 0; // "
    const signed char InpSq            = 1; // '
    const signed char InpAmp           = 2; // &
    const signed char InpPer           = 3; // %
    const signed char InpUnknown       = 4;

    static signed char table[9][5] = {
     /*  InpDq  InpSq  InpAmp  InpPer  InpUnknown */
	{ Dq,    Sq,    -1,     -1,     -1    }, // Init
	{ Done,  DqC,   DqRef,  DqPER,  DqC   }, // Dq
	{ Done,  DqC,   DqRef,  DqPER,  DqC   }, // DqC
	{ Done,  DqC,   DqRef,  DqPER,  DqC   }, // DqPER
	{ Done,  DqC,   DqRef,  DqPER,  DqC   }, // DqRef
	{ SqC,   Done,  SqRef,  SqPER,  SqC   }, // Sq
	{ SqC,   Done,  SqRef,  SqPER,  SqC   }, // SqC
	{ SqC,   Done,  SqRef,  SqPER,  SqC   }, // SqPER
	{ SqC,   Done,  SqRef,  SqPER,  SqC   }  // SqRef
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseEntityValue (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseEntityValue, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseEntityValue, state );
	    return FALSE;
	}
	if        ( c == '"' ) {
	    input = InpDq;
	} else if ( c == '\'' ) {
	    input = InpSq;
	} else if ( c == '&' ) {
	    input = InpAmp;
	} else if ( c == '%' ) {
	    input = InpPer;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Dq:
	    case Sq:
		stringClear();
		next();
		break;
	    case DqC:
	    case SqC:
		stringAddC();
		next();
		break;
	    case DqPER:
	    case SqPER:
		d->parsePEReference_context = InEntityValue;
		if ( !parsePEReference() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityValue, state );
		    return FALSE;
		}
		break;
	    case DqRef:
	    case SqRef:
		d->parseReference_context = InEntityValue;
		if ( !parseReference() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityValue, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a comment [15].

  Precondition: the beginning '<!' of the comment is already read and the head
  stands on the first '-' of '<!--'.

  If this funktion was successful, the head-position is on the first
  character after the comment.
*/
bool QXmlSimpleReader::parseComment()
{
    const signed char Init             = 0;
    const signed char Dash1            = 1; // the first dash was read
    const signed char Dash2            = 2; // the second dash was read
    const signed char Com              = 3; // read comment
    const signed char Com2             = 4; // read comment (help state)
    const signed char ComE             = 5; // finished reading comment
    const signed char Done             = 6;

    const signed char InpDash          = 0; // -
    const signed char InpGt            = 1; // >
    const signed char InpUnknown       = 2;

    static signed char table[6][3] = {
     /*  InpDash  InpGt  InpUnknown */
	{ Dash1,   -1,    -1  }, // Init
	{ Dash2,   -1,    -1  }, // Dash1
	{ Com2,    Com,   Com }, // Dash2
	{ Com2,    Com,   Com }, // Com
	{ ComE,    Com,   Com }, // Com2
	{ -1,      Done,  -1  }  // ComE
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseComment (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseComment, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Dash2:
		stringClear();
		break;
	    case Com2:
		// if next character is not a dash than don't skip it
		if ( c != '-' ) {
		    stringAddC( '-' );
		}
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_ERRORPARSINGCOMMENT );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseComment, state );
	    return FALSE;
	}
	if        ( c == '-' ) {
	    input = InpDash;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Dash1:
		next();
		break;
	    case Dash2:
		next();
		break;
	    case Com:
		stringAddC();
		next();
		break;
	    case Com2:
		next();
		break;
	    case ComE:
		next();
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*!
  Parse a Attribute [41].

  Precondition: the head stands on the first character of the name of the
  attribute (i.e. all whitespaces are already parsed).

  The head stand on the next character after the end quotes. The variable name
  contains the name of the attribute and the variable string contains the value
  of the attribute.
*/
bool QXmlSimpleReader::parseAttribute()
{
    const signed char Init             = 0;
    const signed char PName            = 1; // parse name
    const signed char Ws               = 2; // eat ws
    const signed char Eq               = 3; // the '=' was read
    const signed char Quotes           = 4; // " or ' were read

    const signed char InpNameBe        = 0;
    const signed char InpEq            = 1; // =
    const signed char InpDq            = 2; // "
    const signed char InpSq            = 3; // '
    const signed char InpUnknown       = 4;

    static signed char table[4][5] = {
     /*  InpNameBe  InpEq  InpDq    InpSq    InpUnknown */
	{ PName,     -1,    -1,      -1,      -1    }, // Init
	{ -1,        Eq,    -1,      -1,      Ws    }, // PName
	{ -1,        Eq,    -1,      -1,      -1    }, // Ws
	{ -1,        -1,    Quotes,  Quotes,  -1    }  // Eq
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseAttribute (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseAttribute, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Quotes:
		// Done
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseAttribute, state );
	    return FALSE;
	}
	if        ( is_NameBeginning(c) ) {
	    input = InpNameBe;
	} else if ( c == '=' ) {
	    input = InpEq;
	} else if ( c == '"' ) {
	    input = InpDq;
	} else if ( c == '\'' ) {
	    input = InpSq;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case PName:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseAttribute, state );
		    return FALSE;
		}
		break;
	    case Ws:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttribute, state );
		    return FALSE;
		}
		break;
	    case Eq:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttribute, state );
		    return FALSE;
		}
		break;
	    case Quotes:
		if ( !parseAttValue() ) {
		    parseFailed( &QXmlSimpleReader::parseAttribute, state );
		    return FALSE;
		}
		break;
	}
    }
}

/*
  Parse a Name [5] and store the name in name or ref (if useRef is TRUE).
*/
bool QXmlSimpleReader::parseName()
{
    const signed char Init             = 0;
    const signed char Name1            = 1; // parse first signed character of the name
    const signed char Name             = 2; // parse name
    const signed char Done             = 3;

    const signed char InpNameBe        = 0; // name beginning signed characters
    const signed char InpNameCh        = 1; // NameChar without InpNameBe
    const signed char InpUnknown       = 2;

    static signed char table[3][3] = {
     /*  InpNameBe  InpNameCh  InpUnknown */
	{ Name1,     -1,        -1    }, // Init
	{ Name,      Name,      Done  }, // Name1
	{ Name,      Name,      Done  }  // Name
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseName (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseName, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseName, state );
	    return FALSE;
	}
	if        ( is_NameBeginning(c) ) {
	    input = InpNameBe;
	} else if ( is_NameChar(c) ) {
	    input = InpNameCh;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Name1:
		if ( d->parseName_useRef ) {
		    refClear();
		    refAddC();
		} else {
		    nameClear();
		    nameAddC();
		}
		next();
		break;
	    case Name:
		if ( d->parseName_useRef ) {
		    refAddC();
		} else {
		    nameAddC();
		}
		next();
		break;
	}
    }
}

/*
  Parse a Nmtoken [7] and store the name in name.
*/
bool QXmlSimpleReader::parseNmtoken()
{
    const signed char Init             = 0;
    const signed char NameF            = 1;
    const signed char Name             = 2;
    const signed char Done             = 3;

    const signed char InpNameCh        = 0; // NameChar without InpNameBe
    const signed char InpUnknown       = 1;

    static signed char table[3][2] = {
     /*  InpNameCh  InpUnknown */
	{ NameF,     -1    }, // Init
	{ Name,      Done  }, // NameF
	{ Name,      Done  }  // Name
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseNmtoken (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseNmtoken, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseNmtoken, state );
	    return FALSE;
	}
	if ( is_NameChar(c) ) {
	    input = InpNameCh;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case NameF:
		nameClear();
		nameAddC();
		next();
		break;
	    case Name:
		nameAddC();
		next();
		break;
	}
    }
}

/*
  Parse a Reference [67].

  charDataRead is set to TRUE if the reference must not be parsed. The
  character(s) which the reference mapped to are appended to string. The
  head stands on the first character after the reference.

  charDataRead is set to FALSE if the reference must be parsed. The
  charachter(s) which the reference mapped to are inserted at the reference
  position. The head stands on the first character of the replacement).
*/
bool QXmlSimpleReader::parseReference()
{
    // temporary variables (only used in very local context, so they don't
    // interfere with incremental parsing)
    uint tmp;
    bool ok;

    const signed char Init             =  0;
    const signed char SRef             =  1; // start of a reference
    const signed char ChRef            =  2; // parse CharRef
    const signed char ChDec            =  3; // parse CharRef decimal
    const signed char ChHexS           =  4; // start CharRef hexadecimal
    const signed char ChHex            =  5; // parse CharRef hexadecimal
    const signed char Name             =  6; // parse name
    const signed char DoneD            =  7; // done CharRef decimal
    const signed char DoneH            =  8; // done CharRef hexadecimal
    const signed char DoneN            =  9; // done EntityRef

    const signed char InpAmp           = 0; // &
    const signed char InpSemi          = 1; // ;
    const signed char InpHash          = 2; // #
    const signed char InpX             = 3; // x
    const signed char InpNum           = 4; // 0-9
    const signed char InpHex           = 5; // a-f A-F
    const signed char InpUnknown       = 6;

    static signed char table[8][7] = {
     /*  InpAmp  InpSemi  InpHash  InpX     InpNum  InpHex  InpUnknown */
	{ SRef,   -1,      -1,      -1,      -1,     -1,     -1    }, // Init
	{ -1,     -1,      ChRef,   Name,    Name,   Name,   Name  }, // SRef
	{ -1,     -1,      -1,      ChHexS,  ChDec,  -1,     -1    }, // ChRef
	{ -1,     DoneD,   -1,      -1,      ChDec,  -1,     -1    }, // ChDec
	{ -1,     -1,      -1,      -1,      ChHex,  ChHex,  -1    }, // ChHexS
	{ -1,     DoneH,   -1,      -1,      ChHex,  ChHex,  -1    }, // ChHex
	{ -1,     DoneN,   -1,      -1,      -1,     -1,     -1    }  // Name
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseReference (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseReference, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	switch ( state ) {
	    case DoneD:
		return TRUE;
	    case DoneH:
		return TRUE;
	    case DoneN:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_ERRORPARSINGREFERENCE );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseReference, state );
	    return FALSE;
	}
	if        ( c.row() ) {
	    input = InpUnknown;
	} else if ( c.cell() == '&' ) {
	    input = InpAmp;
	} else if ( c.cell() == ';' ) {
	    input = InpSemi;
	} else if ( c.cell() == '#' ) {
	    input = InpHash;
	} else if ( c.cell() == 'x' ) {
	    input = InpX;
	} else if ( '0' <= c.cell() && c.cell() <= '9' ) {
	    input = InpNum;
	} else if ( 'a' <= c.cell() && c.cell() <= 'f' ) {
	    input = InpHex;
	} else if ( 'A' <= c.cell() && c.cell() <= 'F' ) {
	    input = InpHex;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case SRef:
		refClear();
		next();
		break;
	    case ChRef:
		next();
		break;
	    case ChDec:
		refAddC();
		next();
		break;
	    case ChHexS:
		next();
		break;
	    case ChHex:
		refAddC();
		next();
		break;
	    case Name:
		// read the name into the ref
		d->parseName_useRef = TRUE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseReference, state );
		    return FALSE;
		}
		break;
	    case DoneD:
		tmp = ref().toUInt( &ok, 10 );
		if ( ok ) {
		    stringAddC( QChar(tmp) );
		} else {
		    reportParseError( XMLERR_ERRORPARSINGREFERENCE );
		    return FALSE;
		}
		d->parseReference_charDataRead = TRUE;
		next();
		break;
	    case DoneH:
		tmp = ref().toUInt( &ok, 16 );
		if ( ok ) {
		    stringAddC( QChar(tmp) );
		} else {
		    reportParseError( XMLERR_ERRORPARSINGREFERENCE );
		    return FALSE;
		}
		d->parseReference_charDataRead = TRUE;
		next();
		break;
	    case DoneN:
		if ( !processReference() )
		    return FALSE;
		next();
		break;
	}
    }
}

/*
  Helper function for parseReference()
*/
bool QXmlSimpleReader::processReference()
{
    QString reference = ref();
    if ( reference == "amp" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'a' ); stringAddC( 'm' ); stringAddC( 'p' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '&' );
	}
	d->parseReference_charDataRead = TRUE;
    } else if ( reference == "lt" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'l' ); stringAddC( 't' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '<' );
	}
	d->parseReference_charDataRead = TRUE;
    } else if ( reference == "gt" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'g' ); stringAddC( 't' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '>' );
	}
	d->parseReference_charDataRead = TRUE;
    } else if ( reference == "apos" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'a' ); stringAddC( 'p' ); stringAddC( 'o' ); stringAddC( 's' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '\'' );
	}
	d->parseReference_charDataRead = TRUE;
    } else if ( reference == "quot" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'q' ); stringAddC( 'u' ); stringAddC( 'o' ); stringAddC( 't' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '"' );
	}
	d->parseReference_charDataRead = TRUE;
    } else {
	QMap<QString,QString>::Iterator it;
	it = d->entities.find( reference );
	if ( it != d->entities.end() ) {
	    // "Internal General"
	    switch ( d->parseReference_context ) {
		case InContent:
		    // Included
		    xmlRef = it.data() + xmlRef;
		    d->parseReference_charDataRead = FALSE;
		    break;
		case InAttributeValue:
		    // Included in literal
		    xmlRef = it.data().replace( QRegExp("\""), "&quot;" ).replace( QRegExp("'"), "&apos;" )
			+ xmlRef;
		    d->parseReference_charDataRead = FALSE;
		    break;
		case InEntityValue:
		    {
			// Bypassed
			stringAddC( '&' );
			for ( int i=0; i<(int)reference.length(); i++ ) {
			    stringAddC( reference[i] );
			}
			stringAddC( ';');
			d->parseReference_charDataRead = TRUE;
		    }
		    break;
		case InDTD:
		    // Forbidden
		    d->parseReference_charDataRead = FALSE;
		    reportParseError( XMLERR_INTERNALGENERALENTITYINDTD );
		    return FALSE;
	    }
	} else {
	    QMap<QString,QXmlSimpleReaderPrivate::ExternEntity>::Iterator itExtern;
	    itExtern = d->externEntities.find( reference );
	    if ( itExtern == d->externEntities.end() ) {
		// entity not declared
		// ### check this case for conformance
		if ( d->parseReference_context == InEntityValue ) {
		    // Bypassed
		    stringAddC( '&' );
		    for ( int i=0; i<(int)reference.length(); i++ ) {
			stringAddC( reference[i] );
		    }
		    stringAddC( ';');
		    d->parseReference_charDataRead = TRUE;
		} else {
		    if ( contentHnd ) {
			if ( !contentHnd->skippedEntity( reference ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE; // error
			}
		    }
		}
	    } else if ( (*itExtern).notation.isNull() ) {
		// "External Parsed General"
		switch ( d->parseReference_context ) {
		    case InContent:
			// Included if validating
			if ( contentHnd ) {
			    if ( !contentHnd->skippedEntity( reference ) ) {
				reportParseError( contentHnd->errorString() );
				return FALSE; // error
			    }
			}
			d->parseReference_charDataRead = FALSE;
			break;
		    case InAttributeValue:
			// Forbidden
			d->parseReference_charDataRead = FALSE;
			reportParseError( XMLERR_EXTERNALGENERALENTITYINAV );
			return FALSE;
		    case InEntityValue:
			{
			    // Bypassed
			    stringAddC( '&' );
			    for ( int i=0; i<(int)reference.length(); i++ ) {
				stringAddC( reference[i] );
			    }
			    stringAddC( ';');
			    d->parseReference_charDataRead = TRUE;
			}
			break;
		    case InDTD:
			// Forbidden
			d->parseReference_charDataRead = FALSE;
			reportParseError( XMLERR_EXTERNALGENERALENTITYINDTD );
			return FALSE;
		}
	    } else {
		// "Unparsed"
		// ### notify for "Occurs as Attribute Value" missing (but this is no refence, anyway)
		// Forbidden
		d->parseReference_charDataRead = FALSE;
		reportParseError( XMLERR_UNPARSEDENTITYREFERENCE );
		return FALSE; // error
	    }
	}
    }
    return TRUE; // no error
}


/*
  Parses over a simple string.

  After the string was successfully parsed, the head is on the first
  character after the string.
*/
bool QXmlSimpleReader::parseString()
{
    static signed char Done;

    const signed char InpCharExpected  = 0; // the character that was expected
    const signed char InpUnknown       = 1;

    signed char state; // state in this function is the position in the string s
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	Done = d->parseString_s.length();
	state = 0;
    } else {
	state = d->parseStack->top()->state;
	d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseString (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top()->function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseString, state );
		return FALSE;
	    }
	}
    }

    while ( TRUE ) {
	if ( state == Done ) {
	    return TRUE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseString, state );
	    return FALSE;
	}
	if ( c == d->parseString_s[(int)state] ) {
	    input = InpCharExpected;
	} else {
	    input = InpUnknown;
	}
	if ( input == InpCharExpected ) {
	    state++;
	} else {
	    // Error
	    reportParseError( XMLERR_UNEXPECTEDCHARACTER );
	    return FALSE;
	}

	next();
    }
}

/*
  This private function moves the cursor to the next character.
*/
void QXmlSimpleReader::next()
{
    if ( !xmlRef.isEmpty() ) {
	c = xmlRef[0];
	xmlRef.remove( 0, 1 );
    } else {
	if ( c=='\n' || c=='\r' ) {
	    lineNr++;
	    columnNr = -1;
	}
	c = inputSource->next();
	columnNr++;
    }
}

/*
  This private function moves the cursor to the next non-whitespace character.
  This function does not move the cursor if the actual cursor position is a
  non-whitespace charcter.

  Returns FALSE when you use incremental parsing and this function reaches EOF
  with reading only whitespace characters. In this case it also poplulates the
  parseStack with useful information. In all other cases, this function returns
  TRUE.
*/
bool QXmlSimpleReader::eat_ws()
{
    while ( !atEnd() ) {
	if ( !is_S(c) ) {
	    return TRUE;
	}
	next();
    }
    if ( d->parseStack != 0 ) {
	unexpectedEof( &QXmlSimpleReader::eat_ws, 0 );
	return FALSE;
    }
    return TRUE;
}

bool QXmlSimpleReader::next_eat_ws()
{
    next();
    return eat_ws();
}


/*
  This private function initializes the reader. \a i is the input source to
  read the data from.
*/
void QXmlSimpleReader::init( const QXmlInputSource& i )
{
    lineNr = 0;
    columnNr = -1;
    inputSource = (QXmlInputSource *)&i; // ### pointer stuff is not nice
    initData();

    d->externParameterEntities.clear();
    d->parameterEntities.clear();
    d->externEntities.clear();
    d->entities.clear();

    d->tags.clear();

    d->doctype = "";
    d->xmlVersion = "";
    d->encoding = "";
    d->standalone = QXmlSimpleReaderPrivate::Unknown;
    d->error = QString::null;
}

/*
  This private function initializes the XML data related variables. Especially,
  it reads the data from the input source.
*/
void QXmlSimpleReader::initData()
{
    c = QXmlInputSource::EndOfData;
    xmlRef = "";
    next();
}

/*
  Returns TRUE if a entity with the name \a e exists,
  otherwise returns FALSE.
*/
bool QXmlSimpleReader::entityExist( const QString& e ) const
{
    if (  d->parameterEntities.find(e) == d->parameterEntities.end() &&
	    d->externParameterEntities.find(e) == d->externParameterEntities.end() ) {
	return FALSE;
    } else {
	return TRUE;
    }
}

void QXmlSimpleReader::reportParseError( const QString& error )
{
    d->error = error;
    if ( errorHnd ) {
	if ( d->error.isNull() ) {
	    errorHnd->fatalError( QXmlParseException( XMLERR_OK, columnNr+1, lineNr+1 ) );
	} else {
	    errorHnd->fatalError( QXmlParseException( d->error, columnNr+1, lineNr+1 ) );
	}
    }
}

/*
  This private function is called when a parsing function encounters an
  unexpected EOF. It decides what to do (depending on incremental parsing or
  not). \a where is a pointer to the function where the error occured and \a
  state is the parsing state in this function.
*/
void QXmlSimpleReader::unexpectedEof( ParseFunction where, int state )
{
    if ( d->parseStack == 0 ) {
	reportParseError( XMLERR_UNEXPECTEDEOF );
    } else {
	if ( c == QXmlInputSource::EndOfDocument ) {
	    reportParseError( XMLERR_UNEXPECTEDEOF );
	} else {
	    pushParseState( where, state );
	}
    }
}

/*
  This private function is called when a parse...() function returned FALSE. It
  determines if there was an error or if incremental parsing simply went out of
  data and does the right thing for the case. \a where is a pointer to the
  function where the error occured and \a state is the parsing state in this
  function.
*/
void QXmlSimpleReader::parseFailed( ParseFunction where, int state )
{
    if ( d->parseStack!=0 && d->error.isNull() ) {
	pushParseState( where, state );
    }
}

/*
  This private function pushes the function pointer \a function and state \a
  state to the parse stack. This is used when you are doing an incremental
  parsing and reach the end of file too early.

  Only call this function when d->parseStack!=0.
*/
void QXmlSimpleReader::pushParseState( ParseFunction function, int state )
{
    QXmlSimpleReaderPrivate::ParseState *ps = new QXmlSimpleReaderPrivate::ParseState;
    ps->function = function;
    ps->state = state;
    d->parseStack->push( ps );
}


// use buffers instead of QString::operator+= when single characters are read
QString& QXmlSimpleReader::string()
{
    stringValue += QString( stringArray, stringPos );
    stringPos = 0;
    return stringValue;
}
QString& QXmlSimpleReader::name()
{
    nameValue += QString( nameArray, namePos );
    namePos = 0;
    return nameValue;
}
QString& QXmlSimpleReader::ref()
{
    refValue += QString( refArray, refPos );
    refPos = 0;
    return refValue;
}

void QXmlSimpleReader::stringAddC()
{
    if ( stringPos >= 256 ) {
	stringValue += QString( stringArray, stringPos );
	stringPos = 0;
    }
    stringArray[stringPos++] = c;
}
void QXmlSimpleReader::nameAddC()
{
    if ( namePos >= 256 ) {
	nameValue += QString( nameArray, namePos );
	namePos = 0;
    }
    nameArray[namePos++] = c;
}
void QXmlSimpleReader::refAddC()
{
    if ( refPos >= 256 ) {
	refValue += QString( refArray, refPos );
	refPos = 0;
    }
    refArray[refPos++] = c;
}

void QXmlSimpleReader::stringAddC(const QChar& ch)
{
    if ( stringPos >= 256 ) {
	stringValue += QString( stringArray, stringPos );
	stringPos = 0;
    }
    stringArray[stringPos++] = ch;
}
void QXmlSimpleReader::nameAddC(const QChar& ch)
{
    if ( namePos >= 256 ) {
	nameValue += QString( nameArray, namePos );
	namePos = 0;
    }
    nameArray[namePos++] = ch;
}
void QXmlSimpleReader::refAddC(const QChar& ch)
{
    if ( refPos >= 256 ) {
	refValue += QString( refArray, refPos );
	refPos = 0;
    }
    refArray[refPos++] = ch;
}

#endif //QT_NO_XML
