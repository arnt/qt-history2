/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt GUI Designer.
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qxml.h"


// NOT REVISED

// Error strings for the XML reader
#define XMLERR_OK                         "no error occured"
#define XMLERR_TAGMISMATCH                "tag mismatch"
#define XMLERR_UNEXPECTEDEOF              "unexpected end of file"
#define XMLERR_FINISHEDPARSINGWHILENOTEOF "parsing is finished but end of file is not reached"
#define XMLERR_LETTEREXPECTED             "letter is expected"
#define XMLERR_ERRORPARSINGELEMENT        "error while parsing element"
#define XMLERR_ERRORPARSINGPROLOG         "error while parsing prolog"
#define XMLERR_ERRORPARSINGMAINELEMENT    "error while parsing main element"
#define XMLERR_ERRORPARSINGCONTENT        "error while parsing content"
#define XMLERR_ERRORPARSINGNAME           "error while parsing name"
#define XMLERR_ERRORPARSINGNMTOKEN        "error while parsing Nmtoken"
#define XMLERR_ERRORPARSINGATTRIBUTE      "error while parsing attribute"
#define XMLERR_ERRORPARSINGMISC           "error while parsing misc"
#define XMLERR_ERRORPARSINGCHOICE         "error while parsing choice or seq"
#define XMLERR_ERRORBYCONSUMER            "error triggered by consumer"
#define XMLERR_UNEXPECTEDCHARACTER        "unexpected character"
#define XMLERR_EQUALSIGNEXPECTED          "expected '=' but not found"
#define XMLERR_QUOTATIONEXPECTED          "expected \" or ' but not found"
#define XMLERR_ERRORPARSINGREFERENCE      "error while parsing reference"
#define XMLERR_ERRORPARSINGPI             "error while parsing processing instruction"
#define XMLERR_ERRORPARSINGATTLISTDECL    "error while parsing attribute list declaration"
#define XMLERR_ERRORPARSINGATTTYPE        "error while parsing attribute type declaration"
#define XMLERR_ERRORPARSINGATTVALUE       "error while parsing attribute value declaration"
#define XMLERR_ERRORPARSINGELEMENTDECL    "error while parsing element declaration"
#define XMLERR_ERRORPARSINGENTITYDECL     "error while parsing entity declaration"
#define XMLERR_ERRORPARSINGNOTATIONDECL   "error while parsing notation declaration"
#define XMLERR_ERRORPARSINGEXTERNALID     "error while parsing external id"
#define XMLERR_ERRORPARSINGCOMMENT        "error while parsing comment"
#define XMLERR_ERRORPARSINGENTITYVALUE    "error while parsing entity value declaration"
#define XMLERR_CDSECTHEADEREXPECTED       "expected the header for a cdata section"
#define XMLERR_MORETHANONEDOCTYPE         "more than one document type definition"
#define XMLERR_ERRORPARSINGDOCTYPE        "error while parsing document type definition"
#define XMLERR_INVALIDNAMEFORPI           "invalid name for processing instruction"
#define XMLERR_VERSIONEXPECTED            "version expected while reading the xml declaration"
#define XMLERR_EDECLORSDDECLEXPECTED      "EDecl or SDDecl expected while reading the xml declaration"
#define XMLERR_SDDECLEXPECTED             "SDDecl expected while reading the xml declaration"
#define XMLERR_WRONGVALUEFORSDECL         "wrong value for standalone declaration"
#define XMLERR_UNPARSEDENTITYREFERENCE    "unparsed entity reference"
#define XMLERR_INTERNALGENERALENTITYINDTD "internal general entity reference not allowed in DTD"
#define XMLERR_EXTERNALGENERALENTITYINDTD "external parsed general entity reference not allowed in DTD"
#define XMLERR_EXTERNALGENERALENTITYINAV  "external parsed general entity reference not allowed in attribute value"


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


/*!
  \class QXmlParseException qxml.h
  \brief QXml

  \ingroup xml

  Snafu...
*/
/*!
  \fn QXmlParseException::QXmlParseException( const QString& name, int c, int l, const QString& p, const QString& s )

  Fnord...
*/
/*!
  \fn const QString& QXmlParseException::message() const

  Fnord...
*/
/*!
  \fn int QXmlParseException::columnNumber() const

  Fnord...
*/
/*!
  \fn int QXmlParseException::lineNumber() const

  Fnord...
*/
/*!
  \fn const QString& QXmlParseException::publicId() const

  Fnord...
*/
/*!
  \fn const QString& QXmlParseException::systemId() const

  Fnord...
*/


/*!
  \class QXmlLocator qxml.h
  \brief QXml

  \ingroup xml

  Snafu...
*/
/*!
    \fn QXmlLocator::QXmlLocator( QXmlSimpleReader* parent )

    Fnord...
*/
/*!
    \fn QXmlLocator::~QXmlLocator()

    Fnord...
*/
/*!
    \fn inline int QXmlLocator::columnNumber()

    Fnord...
*/
/*!
    \fn inline int QXmlLocator::lineNumber()

    Fnord...
*/


/*********************************************
 *
 * QXmlNamespaceSupport
 *
 *********************************************/

/*!
  \class QXmlNamespaceSupport qxml.h
  \brief QXmlNamespaceSupport is designed to help parsers providing XML
  namespace support.

  \ingroup xml

  It provides some functions that make it easy to handle namespaces. Its main
  use is for subclasses of \l QXmlReader which want to provide namespace
  support.
*/

/*!
  Constructs a QXmlNamespaceSupport.
*/
QXmlNamespaceSupport::QXmlNamespaceSupport()
{
    reset();
}

/*!
  Destructs a QXmlNamespaceSupport.
*/
QXmlNamespaceSupport::~QXmlNamespaceSupport()
{
}

/*!
  Declare a Namespace prefix.

  This function declares a prefix in the current namespace context; the prefix
  will remain in force until this context is popped, unless it is shadowed in a
  descendant context.

  Note that there is an asymmetry in this library: while \l prefix() will not
  return the default "" prefix, even if you have declared one; to check for a
  default prefix, you have to look it up explicitly using \l uri(). This
  asymmetry exists to make it easier to look up prefixes for attribute names,
  where the default prefix is not allowed.
*/
void QXmlNamespaceSupport::setPrefix( const QString& pre, const QString& uri )
{
    if( pre.isNull() ) {
	ns.insert( "", uri );
    } else {
	ns.insert( pre, uri );
    }
}

/*!
  Return one of the prefixes mapped to a Namespace URI.

  If more than one prefix is currently mapped to the same URI, this method will
  make an arbitrary selection; if you want all of the prefixes, use the
  \l prefixes() method instead.

  Note: this will never return the empty (default) prefix; to check for a
  default prefix, use the \l uri() method with an argument of "".
*/
QString QXmlNamespaceSupport::prefix( const QString& uri )
{
    namespaceMap::Iterator itc, it = ns.begin();
    while ( (itc=it) != ns.end() ) {
	++it;
	if ( itc.data() == uri && !itc.key().isEmpty() )
	    return itc.key();
    }
    return "";
}

/*!
  Look up a prefix and get the currently-mapped Namespace URI.

  This method looks up the prefix in the current context. Use the empty string
  ("") for the default Namespace.
*/
QString QXmlNamespaceSupport::uri( const QString& prefix )
{
    const QString& returi = ns[ prefix ];
    return returi;
}

/*!
  Split the name at the ':' and return the prefix and the local name.
*/
void QXmlNamespaceSupport::splitName( const QString& qname,
	QString& prefix, QString& localname )
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
  Process a raw XML 1.0 name.

  This method processes a raw XML 1.0 name in the current context by removing
  the prefix and looking it up among the prefixes currently declared.

  First parameter is the raw XML 1.0 name to be processed. The second parameter
  is a flag whether the name is the name of an attribute (TRUE) or not (FALSE).

  The return values will be stored in the last two parameters as follows:
  <ul>
  <li> The Namespace URI, or an empty string if none is in use.
  <li> The local name (without prefix).
  </ul>

  If the raw name has a prefix that has not been declared, then the return
  value will be empty.

  Note that attribute names are processed differently than element names: an
  unprefixed element name will received the default Namespace (if any), while
  an unprefixed element name will not
*/
void QXmlNamespaceSupport::processName( const QString& qname,
	bool isAttribute,
	QString& nsuri, QString& localname )
{
    uint pos;
    // search the ':'
    for( pos=0; pos<qname.length(); pos++ ) {
	if ( qname.at(pos) == ':' )
	    break;
    }
    if ( pos < qname.length() ) {
	// there was a ':'
	nsuri = uri( qname.left( pos ) );
	localname = qname.mid( pos+1 );
    } else {
	// there was no ':'
	if ( isAttribute ) {
	    nsuri = ""; // attributes don't take default namespace
	} else {
	    nsuri = uri( "" ); // get default namespace
	}
	localname = qname;
    }
}

/*!
  Return an enumeration of all prefixes currently declared.

  Note: if there is a default prefix, it will not be returned in this
  enumeration; check for the default prefix using \l uri() with an argument
  of "".
*/
QStringList QXmlNamespaceSupport::prefixes()
{
    QStringList list;

    namespaceMap::Iterator itc, it = ns.begin();
    while ( (itc=it) != ns.end() ) {
	++it;
	if ( !itc.key().isEmpty() )
	    list.append( itc.key() );
    }
    return list;
}

/*!
  Return an enumeration of all prefixes currently declared for a URI.

  This method returns prefixes mapped to a specific Namespace URI. The xml:
  prefix will be included. If you want only one prefix that's mapped to the
  Namespace URI, and you don't care which one you get, use the \l prefix()
  method instead.

  Note: the empty (default) prefix is never included in this enumeration; to
  check for the presence of a default Namespace, use \l uri() with an
  argument of "".
*/
QStringList QXmlNamespaceSupport::prefixes( const QString& uri )
{
    QStringList list;

    namespaceMap::Iterator itc, it = ns.begin();
    while ( (itc=it) != ns.end() ) {
	++it;
	if ( itc.data() == uri && !itc.key().isEmpty() )
	    list.append( itc.key() );
    }
    return list;
}

/*!
  Start a new Namespace context.

  Normally, you should push a new context at the beginning of each XML element:
  the new context will automatically inherit the declarations of its parent
  context, but it will also keep track of which declarations were made within
  this context.
*/
void QXmlNamespaceSupport::pushContext()
{
    nsStack.push( ns );
}

/*!
  Revert to the previous Namespace context.

  Normally, you should pop the context at the end of each XML element.  After
  popping the context, all Namespace prefix mappings that were previously in
  force are restored.
*/
void QXmlNamespaceSupport::popContext()
{
    if( !nsStack.isEmpty() )
	ns = nsStack.pop();
}

/*!
  Reset this Namespace support object for reuse.
*/
void QXmlNamespaceSupport::reset()
{
    nsStack.clear();
    ns.clear();
    ns.insert( "xml", "http://www.w3.org/XML/1998/namespace" ); // the xml namespace
}



/*********************************************
 *
 * QXmlAttributes
 *
 *********************************************/

/*!
  \class QXmlAttributes qxml.h
  \brief QXmlAttributes are used in the XML interface to pass attributes.

  \ingroup xml

  If attributes are reported by \l QXmlContentHandler::startElement() this
  class is used to pass the attribute values.
*/
/*!
  \fn QXmlAttributes::QXmlAttributes()

  Constructs a empty attribute list.
*/
/*!
  \fn QXmlAttributes::~QXmlAttributes()

  Desturcts attributes.
*/

/*!
  Look up the index of an attribute by XML 1.0 qualified name.

  Returns the index of the attribute (starting with 0) or -1 if it wasn't
  found.
*/
int QXmlAttributes::index( const QString& ) const
{
    return 0;
}

/*!
  Look up the index of an attribute by Namespace name.

  The first parameter specifies the namespace URI, or the empty string if
  the name has no Namespace URI. The second parameter specifies the
  attribute's local name.

  Returns the index of the attribute (starting with 0) or -1 if it wasn't
  found.
*/
int QXmlAttributes::index( const QString&, const QString& ) const
{
    return 0;
}

/*!
  Return the number of attributes in the list.
*/
int QXmlAttributes::length() const
{
    return valueList.count();
}

/*!
  Look up an attribute's local name by index (starting with 0).
*/
QString QXmlAttributes::localName( int index ) const
{
    return localnameList[index];
}

/*!
  Look up an attribute's XML 1.0 qualified name by index (starting with 0).
*/
QString QXmlAttributes::qName( int index ) const
{
    return qnameList[index];
}

/*!
  Look up an attribute's Namespace URI by index (starting with 0).
*/
QString QXmlAttributes::uri( int index ) const
{
    return uriList[index];
}

/*!
  Look up an attribute's type by index (starting with 0).
  At the moment only 'CDATA' is returned.
*/
QString QXmlAttributes::type( int ) const
{
    return "CDATA";
}

/*!
  Look up an attribute's type by XML 1.0 qualified name.
  At the moment only 'CDATA' is returned.
*/
QString QXmlAttributes::type( const QString& ) const
{
    return "CDATA";
}

/*!
  Look up an attribute's type by Namespace name.
  At the moment only 'CDATA' is returned.

  The first parameter specifies the namespace URI, or the empty string if
  the name has no Namespace URI. The second parameter specifies the
  attribute's local name.
*/
QString QXmlAttributes::type( const QString&, const QString& ) const
{
    return "CDATA";
}

/*!
  Look up an attribute's value by index (starting with 0).
*/
QString QXmlAttributes::value( int index ) const
{
    return valueList[index];
}

/*!
  Look up an attribute's value by XML 1.0 qualified name.
*/
QString QXmlAttributes::value( const QString& ) const
{
    return "";
}

/*!
  Look up an attribute's value by Namespace name.

  The first parameter specifies the namespace URI, or the empty string if
  the name has no Namespace URI. The second parameter specifies the
  attribute's local name.
*/
QString QXmlAttributes::value( const QString&, const QString& ) const
{
    return "";
}


/*********************************************
 *
 * QXmlInputSource
 *
 *********************************************/

/*!
  \class QXmlInputSource qxml.h
  \brief QXmlInputSource is used in the XML interface as a source for reading
  the XML file.

  \ingroup xml

  All subclasses of \l QXmlReader read the input from this class.
*/
/*!
  \fn inline const QString& QXmlInputSource::data() const

  Return all the data this input source contains.
*/

/*!
  Construct a input source which contains no data.
*/
QXmlInputSource::QXmlInputSource( )
{
    input = "";
}

/*!
  Construct a input source and get the data from the text stream.
*/
QXmlInputSource::QXmlInputSource( QTextStream& stream )
{
    input = stream.read();
}

/*!
  Construct a input source and get the data from the system Id (not implemented
  yet).
*/
QXmlInputSource::QXmlInputSource( const QString& systemId )
{
    input = systemId; // TODO: make something useful!
}

/*!
  Set the data of the input source.
*/
void QXmlInputSource::setData( const QString& d )
{
    input = d;
}


/*********************************************
 *
 * QXmlDefaultHandler
 *
 *********************************************/

/*!
  \class QXmlContentHandler qxml.h
  \brief QXmlContentHandler is an interface used to report logical content
  of an XML file to the user. It is used with a \l QXmlReader.

  \ingroup xml

  If the application needs to be informed of basic parsing events, it
  implements this interface and set it with QXmlReader::setContentHandler().
  The parser uses the instance to report basic document-related events like the
  start and end of elements and character data.

  The order of events in this interface is very important, and mirrors the
  order of information in the document itself. For example, all of an element's
  content (character data, processing instructions, and/or subelements) will
  appear, in order, between the startElement event and the corresponding
  endElement event.

  The class \l QXmlDefaultHandler gives a default implementation for this
  interface; subclassing from this class is very convenient if you want only be
  informed of some parsing events.

  \sa QXmlDTDHandler, QXmlDeclHandler, QXmlEntityResolver, QXmlErrorHandler and
  QXmlLexicalHandler
*/
/*!
  \fn void QXmlContentHandler::setDocumentLocator( QXmlLocator* locator )

  This function gets called before the reader starts parsing the document. The
  argument \e locator is a pointer to a \l QXmlLocator which allows the
  application to get the actual position of the parsing in the document.

  Do not destroy the \e locator; it is destroyed when the reader is destroyed
  (do not use the \e locator after the reader got destroyed).
*/
/*!
  \fn bool QXmlContentHandler::startDocument()

  This function is called by the reader when he starts parsing the document.
  The reader will call this function only once before any other functions in
  this class or in the \l QXmlDTDHandler class are called (except \l
  QXmlContentHandler::setDocumentLocator()).

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.

  \sa endDocument()
*/
/*!
  \fn bool QXmlContentHandler::endDocument()

  This function is called by the reader after he has finished the parsing. It
  is only called once. It is the last function of all handler methods that is
  called. It is called after the reader has read all input or has abandoned
  parsing because of a fatal error.

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.

  \sa startDocument()
*/
/*!
  \fn bool QXmlContentHandler::startPrefixMapping( const QString& prefix, const QString& uri )

  This function is called by the reader to signal the begin of a prefix-URI
  namespace mapping scope. This information is not necessary for normal
  namespace processing since the reader automatically replaces prefixes for
  element and attribute names.

  Note that startPrefixMapping and endPrefixMapping calls are not guaranteed to
  be properly nested relative to each-other: all startPrefixMapping events will
  occur before the corresponding startElement event, and all endPrefixMapping
  events will occur after the corresponding endElement event, but their order
  is not otherwise guaranteed.

  \arg \a prefix is the namespace prefix being declared
  \arg \a uri ist the namespace URI the prefix is mapped to

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.

  \sa endPrefixMapping()
*/
/*!
  \fn bool QXmlContentHandler::endPrefixMapping( const QString& prefix )

  This function is called by the reader to signal the end of a prefix mapping.

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.

  \sa startPrefixMapping()
*/
/*!
  \fn bool QXmlContentHandler::startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts )

  This function is called by the reader when he has parsed a start element tag.

  There will be a corresponding \l endElement() call when the corresponding end
  element tag was read. The startElement() and endElement() calls are always
  nested correctly. Empty element tags (e.g. &lt;a/&gt;) are reported by
  startElement() directly followed by a call to endElement().

  The attribute list provided will contain only attributes with explicit
  values. The attribute list will contain attributes used for namespace
  declaration (xmlns* attributes) only if the namespace-prefix property of the
  reader is TRUE.

  \arg \a uri is the Namespace URI, or the empty string if the element has no
  Namespace URI or if Namespace processing is not being performed
  \arg \a localName is the local name (without prefix), or the empty string if
  Namespace processing is not being performed
  \arg \a qName is the qualified name (with prefix), or the empty string if
  qualified names are not available
  \arg \a atts is the attributes attached to the element. If there are no
  attributes, it shall be an empty Attributes object

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.

  \sa endElement()
*/
/*!
  \fn bool QXmlContentHandler::endElement( const QString& namespaceURI, const QString& localName, const QString& qName )

  This function is called by the reader when he has parsed a end element tag.

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.

  \sa startElement()
*/
/*!
  \fn bool QXmlContentHandler::characters( const QString& ch )

  This function is called by the reader when he has parsed a chunk of character
  data (either normal character data or character data inside a CDATA section;
  if you have to distinguish between those two types you have to use
  additionally \l QXmlLexicalHandler::startCDATA() and \l
  QXmlLexicalHandler::endCDATA()).

  Some readers will report whitespace in element content using the \l
  ignorableWhitespace() function rather than this one (\l QXmlSimpleReader will
  do it not though).

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlContentHandler::ignorableWhitespace( const QString& ch )

  Some readers may use this function to report each chunk of whitespace in
  element content (\l QXmlSimpleReader does not though).

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlContentHandler::processingInstruction( const QString& target, const QString& data )

  The reader will use this function to report a processing instruction.

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlContentHandler::skippedEntity( const QString& name )

  Some readers may skip entities if they have not seen the declarations (e.g.
  because they are in an external DTD). If they do so they will report it by
  calling this function.

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn QString QXmlContentHandler::errorString()

  The reader uses this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlErrorHandler qxml.h
  \brief QXmlErrorHandler is an interface used to report errors to the user
  while parsing an XML file.

  \ingroup xml

  If the application is interested in reporting errors to the user, you should
  subclass... TODO
*/
/*!
  \fn bool QXmlErrorHandler::warning( const QXmlParseException& exception )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlErrorHandler::error( const QXmlParseException& exception )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlErrorHandler::fatalError( const QXmlParseException& exception )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn QString QXmlErrorHandler::errorString()

  The reader uses this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlDTDHandler qxml.h
  \brief QXmlDTDHandler is an interface used to report DTD content of an XML
  file to the user.

  \ingroup xml

  Snafu...
*/
/*!
  \fn bool QXmlDTDHandler::notationDecl( const QString& name, const QString& publicId, const QString& systemId )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlDTDHandler::unparsedEntityDecl( const QString& name, const QString& publicId, const QString& systemId, const QString& notationName )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn QString QXmlDTDHandler::errorString()

  The reader uses this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlEntityResolver qxml.h
  \brief QXmlEntityResolver is an interface used to resolve external entities
  by the user.

  \ingroup xml

  Snafu...
*/
/*!
  \fn bool QXmlEntityResolver::resolveEntity( const QString& publicId, const QString& systemId, QXmlInputSource* ret )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn QString QXmlEntityResolver::errorString()

  The reader uses this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlLexicalHandler qxml.h
  \brief QXmlLexicalHandler is an interface used to report lexical content
  of an XML file to the user.

  \ingroup xml

  Snafu...
*/
/*!
  \fn bool QXmlLexicalHandler::startDTD( const QString& name, const QString& publicId, const QString& systemId )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlLexicalHandler::endDTD()

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlLexicalHandler::startCDATA()

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlLexicalHandler::endCDATA()

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlLexicalHandler::comment( const QString& ch )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn QString QXmlLexicalHandler::errorString()

  The reader uses this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlDeclHandler qxml.h
  \brief QXmlDeclHandler is an interface used to report declaration content
  of an XML file to the user.

  \ingroup xml

  Snafu...
*/
/*!
  \fn bool QXmlDeclHandler::attributeDecl( const QString& eName, const QString& aName, const QString& type, const QString& valueDefault, const QString& value )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlDeclHandler::internalEntityDecl( const QString& name, const QString& value )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn bool QXmlDeclHandler::externalEntityDecl( const QString& name, const QString& publicId, const QString& systemId )

  Fnord...

  If this function returns FALSE the reader will stop parsing and will report
  an error. The reader will use the function \l errorString() to get the error
  message that will be used for reporting the error.
*/
/*!
  \fn QString QXmlDeclHandler::errorString()

  The reader uses this function to get an error string if any of the handler
  functions returns FALSE to him.
*/


/*!
  \class QXmlDefaultHandler qxml.h
  \brief QXmlDefaultHandler is used in the Xml interface to easily create
  customized handler.

  \ingroup xml

  Snafu...
*/
/*!
  \fn QXmlDefaultHandler::QXmlDefaultHandler()

  Fnord...
*/
/*!
  \fn QXmlDefaultHandler::~QXmlDefaultHandler()

  Fnord...
*/

/*!
  Do nothing.
*/
void QXmlDefaultHandler::setDocumentLocator( QXmlLocator* )
{
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::startDocument()
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::endDocument()
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::startPrefixMapping( const QString&, const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::endPrefixMapping( const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::startElement( const QString&, const QString&,
	const QString&, const QXmlAttributes& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::endElement( const QString&, const QString&,
	const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::characters( const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::ignorableWhitespace( const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::processingInstruction( const QString&,
	const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::skippedEntity( const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::warning( const QXmlParseException& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::error( const QXmlParseException& )
{
    return TRUE;
}

/*!
  Blubber...
*/
bool QXmlDefaultHandler::fatalError( const QXmlParseException& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::notationDecl( const QString&, const QString&,
	const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::unparsedEntityDecl( const QString&, const QString&,
	const QString&, const QString& )
{
    return TRUE;
}

/*!
  Always return 0, so that the parser will use the system identifier provided
  in the XML document.
*/
bool QXmlDefaultHandler::resolveEntity( const QString&, const QString&,
	QXmlInputSource* ret )
{
    ret = 0;
    return TRUE;
}

/*!
  Blubber...
*/
QString QXmlDefaultHandler::errorString()
{
    return QString( XMLERR_ERRORBYCONSUMER );
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::startDTD( const QString&, const QString&, const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::endDTD()
{
    return TRUE;
}

#if 0
/*!
  Do nothing.
*/
bool QXmlDefaultHandler::startEntity( const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::endEntity( const QString& )
{
    return TRUE;
}
#endif

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::startCDATA()
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::endCDATA()
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::comment( const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::attributeDecl( const QString&, const QString&, const QString&, const QString&, const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
*/
bool QXmlDefaultHandler::internalEntityDecl( const QString&, const QString& )
{
    return TRUE;
}

/*!
  Do nothing.
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
    // constructor
    QXmlSimpleReaderPrivate()
    { }


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
  \brief QXmlReader is an interface for XML parsers.

  \ingroup xml

  Snafu...
  SAX, etc.

  The following code fragment realizes a parser that outputs the name of all
  start tags. They are indented corresponding to how they are nested.
  \code
    class StructureParser : public QXmlDefaultHandler
    {
    public:
	bool startDocument()
	{
	    indent = "";
	    return TRUE;
	}
	bool startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& )
	{
	    cout << indent << qName << endl;
	    indent += " ";
	    return TRUE;
	}
	bool endElement( const QString&, const QString&, const QString& )
	{
	    indent.remove( 0, 1 );
	    return TRUE;
	}
    private:
	QString indent;
    };

    void parse( QTextStream& ts )
    {
	StructureParser hnd;
	QXmlInputSource source( ts );

	QXmlSimpleReader parser;
	parser.setContentHandler( &hnd );
	parser.parse( source );
    }
  \endcode
*/
/*!
  \fn bool QXmlReader::feature( const QString& name, bool *ok ) const

  Fnord...
*/
/*!
  \fn bool QXmlReader::setFeature( const QString& name, bool value )

  Fnord...
*/
/*!
  \fn void QXmlReader::setEntityResolver( QXmlEntityResolver* handler )

  Fnord...
*/
/*!
  \fn QXmlEntityResolver* QXmlReader::entityResolver()

  Fnord...
*/
/*!
  \fn void QXmlReader::setDTDHandler( QXmlDTDHandler* handler )

  Fnord...
*/
/*!
  \fn QXmlDTDHandler* QXmlReader::DTDHandler()

  Fnord...
*/
/*!
  \fn void QXmlReader::setContentHandler( QXmlContentHandler* handler )

  Fnord...
*/
/*!
  \fn QXmlContentHandler* QXmlReader::contentHandler()

  Fnord...
*/
/*!
  \fn void QXmlReader::setErrorHandler( QXmlErrorHandler* handler )

  Fnord...
*/
/*!
  \fn QXmlErrorHandler* QXmlReader::errorHandler()

  Fnord...
*/
/*!
  \fn void QXmlReader::setLexicalHandler( QXmlLexicalHandler* handler )

  Fnord...
*/
/*!
  \fn QXmlLexicalHandler* QXmlReader::lexicalHandler()

  Fnord...
*/
/*!
  \fn void QXmlReader::setDeclHandler( QXmlDeclHandler* handler )

  Fnord...
*/
/*!
  \fn QXmlDeclHandler* QXmlReader::declHandler()

  Fnord...
*/
/*!
  \fn bool QXmlReader::parse( const QXmlInputSource& input )

  Fnord...
*/
/*!
  \fn bool QXmlReader::parse( const QString& systemId )

  Fnord...
*/


/*!
  \class QXmlSimpleReader qxml.h
  \brief QXmlSimpleReader is a simple XML parser.

  \ingroup xml

  Snafu...
*/

//guaranteed not to be a characater
const QChar QXmlSimpleReader::QEOF = QChar((ushort)0xffff);

/*!
  Constructs a simple xml reader.
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
  Destroys a simple xml reader.
*/
QXmlSimpleReader::~QXmlSimpleReader()
{
    delete d->locator;
    delete d;
}

/*!
  Get the state of a feature.

  Supported features:
  <ul>
  <li> http://xml.org/sax/features/namespaces
  <li> http://xml.org/sax/features/namespace-prefixes
  <li> http://trolltech.com/xml/features/report-whitespace-only-CharData
  </ul>
*/
bool QXmlSimpleReader::feature( const QString& feature, bool *ok ) const
{
    if ( ok != 0 )
	*ok = TRUE;
    if        ( feature == "http://xml.org/sax/features/namespaces" ) {
	return d->useNamespaces;
    } else if ( feature == "http://xml.org/sax/features/namespace-prefixes" ) {
	return d->useNamespacePrefixes;
    } else if ( feature == "http://trolltech.com/xml/features/report-whitespace-only-CharData" ) {
	return d->reportWhitespaceCharData;
    } else {
	qWarning( "Unknown feature " + feature );
	if ( ok != 0 )
	    *ok = FALSE;
    }
    return FALSE;
}

/*!
  Set the state of a feature.

  Supported features:
  <ul>
  <li> http://xml.org/sax/features/namespaces
  <li> http://xml.org/sax/features/namespace-prefixes
  <li> http://trolltech.com/xml/features/report-whitespace-only-CharData
  </ul>
*/
void QXmlSimpleReader::setFeature( const QString& feature, bool value )
{
    if        ( feature == "http://xml.org/sax/features/namespaces" ) {
	d->useNamespaces = value;
    } else if ( feature == "http://xml.org/sax/features/namespace-prefixes" ) {
	d->useNamespacePrefixes = value;
    } else if ( feature == "http://trolltech.com/xml/features/report-whitespace-only-CharData" ) {
	d->reportWhitespaceCharData = value;
    } else {
	qWarning( "Unknown feature " + feature );
    }
}

/*!
  Supported features:
  <ul>
  <li> http://xml.org/sax/features/namespaces
  <li> http://xml.org/sax/features/namespace-prefixes
  <li> http://trolltech.com/xml/features/report-whitespace-only-CharData
  </ul>
*/
bool QXmlSimpleReader::hasFeature( const QString& feature ) const
{
    if (    feature == "http://xml.org/sax/features/namespaces" ||
	    feature == "http://xml.org/sax/features/namespace-prefixes" ||
	    feature == "http://trolltech.com/xml/features/report-whitespace-only-CharData" ) {
	return TRUE;
    } else {
	return FALSE;
    }
}

/*!
  Sets a entity resolver.
*/
void QXmlSimpleReader::setEntityResolver( QXmlEntityResolver* handler )
{ entityRes = handler; }

/*!
  Returns the actual used entity resolver.
*/
QXmlEntityResolver* QXmlSimpleReader::entityResolver() const
{ return entityRes; }

/*!
  Sets a DTD handler.
*/
void QXmlSimpleReader::setDTDHandler( QXmlDTDHandler* handler )
{ dtdHnd = handler; }

/*!
  Returns the actual used DTD handler.
*/
QXmlDTDHandler* QXmlSimpleReader::DTDHandler() const
{ return dtdHnd; }

/*!
  Sets a content handler.
*/
void QXmlSimpleReader::setContentHandler( QXmlContentHandler* handler )
{ contentHnd = handler; }

/*!
  Returns the actual used content handler.
*/
QXmlContentHandler* QXmlSimpleReader::contentHandler() const
{ return contentHnd; }

/*!
  Sets a error handler.
*/
void QXmlSimpleReader::setErrorHandler( QXmlErrorHandler* handler )
{ errorHnd = handler; }

/*!
  Returns the actual used error handler.
*/
QXmlErrorHandler* QXmlSimpleReader::errorHandler() const
{ return errorHnd; }

/*!
  Sets a lexical handler.
*/
void QXmlSimpleReader::setLexicalHandler( QXmlLexicalHandler* handler )
{ lexicalHnd = handler; }

/*!
  Returns the actual used lexical handler.
*/
QXmlLexicalHandler* QXmlSimpleReader::lexicalHandler() const
{ return lexicalHnd; }

/*!
  Sets a handler for DTD declaration events.
*/
void QXmlSimpleReader::setDeclHandler( QXmlDeclHandler* handler )
{ declHnd = handler; }

/*!
  Returns the actual used handler for DTD declaration events.
*/
QXmlDeclHandler* QXmlSimpleReader::declHandler() const
{ return declHnd; }



/*!
  Parse an XML document [1] from the given system id (uri).

  Returns TRUE if everything went fine or FALSE.
*/
bool QXmlSimpleReader::parse( const QString& systemId )
{
    QXmlInputSource is( systemId );
    return parse( is );
}


/*!
  Parse an XML document [1] from the given input source id (uri).

  Returns TRUE if everything went fine or FALSE.
*/
bool QXmlSimpleReader::parse( const QXmlInputSource& input )
{
    init( input );
    // call the handler
    if ( contentHnd ) {
	contentHnd->setDocumentLocator( d->locator );
	if ( !contentHnd->startDocument() ) {
	    d->error = contentHnd->errorString();
	    goto parseError;
	}
    }
    // parse prolog
    if ( !parseProlog() ) {
	d->error = XMLERR_ERRORPARSINGPROLOG;
	goto parseError;
    }
    // parse element
    if ( !parseElement() ) {
	d->error = XMLERR_ERRORPARSINGMAINELEMENT;
	goto parseError;
    }
    // parse Misc*
    while ( !atEnd() ) {
	if ( !parseMisc() ) {
	    d->error = XMLERR_ERRORPARSINGMISC;
	    goto parseError;
	}
    }
    // is stack empty?
    if ( !tags.isEmpty() ) {
	d->error = XMLERR_UNEXPECTEDEOF;
	goto parseError;
    }
    // call the handler
    if ( contentHnd ) {
	if ( !contentHnd->endDocument() ) {
	    d->error = contentHnd->errorString();
	    goto parseError;
	}
    }

    return TRUE;

    // error handling

parseError:
    reportParseError();
    tags.clear();
    return FALSE;
}

/*!
  Parse the prolog [22].
*/
bool QXmlSimpleReader::parseProlog()
{
    bool xmldecl_possible = TRUE;
    bool doctype_read = FALSE;

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

    // use some kind of state machine for parsing
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// read input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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
	// get new state
	state = table[state][input];

	// in some cases do special actions depending on state
	switch ( state ) {
	    case EatWS:
		// xml declaration only on first position possible
		xmldecl_possible = FALSE;
		// eat white spaces
		eat_ws();
		break;
	    case Lt:
		// next character
		next();
		break;
	    case Em:
		// xml declaration only on first position possible
		xmldecl_possible = FALSE;
		// next character
		next();
		break;
	    case DocType:
		if ( !parseDoctype() ) {
		    d->error = XMLERR_ERRORPARSINGPROLOG;
		    goto parseError;
		}
		if ( doctype_read ) {
		    d->error = XMLERR_MORETHANONEDOCTYPE;
		} else {
		    doctype_read = FALSE;
		}
		/*
		// call the consumer
		if ( consumer ) {
		    if ( !consumer->doctype(d->doctype) ) {
			d->error = XMLERR_ERRORBYCONSUMER;
			goto parseError;
		    }
		}
		*/
		break;
	    case Comment:
		if ( !parseComment() ) {
		    d->error = XMLERR_ERRORPARSINGPROLOG;
		    goto parseError;
		}
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			d->error = lexicalHnd->errorString();
			goto parseError;
		    }
		}
		break;
	    case PI:
		if ( !parsePI(xmldecl_possible) ) {
		    d->error = XMLERR_ERRORPARSINGPROLOG;
		    goto parseError;
		}
		// call the handler
		if ( contentHnd ) {
		    if ( xmldecl_possible && !d->xmlVersion.isEmpty() ) {
			/*
			if ( !consumer->xmlDecleration(d->xmlVersion,d->encoding,standalone) ) {
			    d->error = XMLERR_ERRORBYCONSUMER;
			    goto parseError;
			}
			*/
		    } else {
			if ( !contentHnd->processingInstruction(name(),string()) ) {
			    d->error = contentHnd->errorString();
			    goto parseError;
			}
		    }
		}
		// xml declaration only on first position possible
		xmldecl_possible = FALSE;
		break;
	    case Done:
		return TRUE;
		break;
	    case -1:
		d->error = XMLERR_ERRORPARSINGELEMENT;
		goto parseError;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse an element [39].

  Precondition: the opening '<' is already read.
*/
bool QXmlSimpleReader::parseElement()
{
    static QString uri, lname, prefix;
    static bool t;

    const signed char Init             = 0;
    const signed char ReadName         = 1;
    const signed char STagEnd          = 2;
    const signed char ETagBegin        = 3;
    const signed char EmptyTag         = 4;
    const signed char Attribute        = 5;
    const signed char Done             = 6;

    const signed char InpNameBe        = 0; // is_NameBeginning()
    const signed char InpGt            = 1; // >
    const signed char InpSlash         = 2; // /
    const signed char InpUnknown       = 3;

    // use some kind of state machine for parsing
    static signed char table[6][4] = {
     /*  InpNameBe   InpGt     InpSlash    InpUnknown */
	{ ReadName,   -1,       -1,         -1    }, // Init
	{ Attribute,  STagEnd,  EmptyTag,   -1    }, // ReadName
	{ -1,         -1,       ETagBegin,  -1    }, // STagEnd
	{ -1,         Done,     -1,         -1    }, // ETagBegin
	{ -1,         Done,     -1,         -1    }, // EmptyTag
	{ Attribute,  STagEnd,  EmptyTag,   -1    }  // Attribute
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// read input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( is_NameBeginning(c) ) {
	    input = InpNameBe;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else if ( c == '/' ) {
	    input = InpSlash;
	} else {
	    input = InpUnknown;
	}
	// get new state
	state = table[state][input];

	// in some cases do special actions depending on state
	switch ( state ) {
	    case ReadName:
		// get the name of the tag
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		// store it on the stack
		tags.push( name() );
		// empty the attributes
		d->attList.qnameList.clear();
		d->attList.uriList.clear();
		d->attList.localnameList.clear();
		d->attList.valueList.clear();
		// namespace support?
		if ( d->useNamespaces ) {
		    d->namespaceSupport.pushContext();
		}
		// eliminate white spaces
		eat_ws();
		break;
	    case STagEnd:
		// call the handler
		if ( contentHnd ) {
		    if ( d->useNamespaces ) {
			d->namespaceSupport.processName( tags.top(), FALSE, uri, lname );
			t = contentHnd->startElement( uri, lname, tags.top(), d->attList );
		    } else {
			t = contentHnd->startElement( "", "", tags.top(), d->attList );
		    }
		    if ( !t ) {
			d->error = contentHnd->errorString();
			goto parseError;
		    }
		}
		next();
		if ( !parseContent() ) {
		    d->error = XMLERR_ERRORPARSINGCONTENT;
		    goto parseError;
		}
		break;
	    case ETagBegin:
		// get the name of the tag
		next();
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		// pop the stack and compare it with the name
		if ( tags.pop() != name() ) {
		    d->error = XMLERR_TAGMISMATCH;
		    goto parseError;
		}
		// call the handler
		if ( contentHnd ) {
		    if ( !contentHnd->endElement("","",name()) ) {
			d->error = contentHnd->errorString();
			goto parseError;
		    }
		}
		// namespace support?
		if ( d->useNamespaces ) {
		    d->namespaceSupport.popContext();
		}
		// eliminate white spaces
		eat_ws();
		break;
	    case EmptyTag:
		if  ( tags.isEmpty() ) {
		    d->error = XMLERR_TAGMISMATCH;
		    goto parseError;
		}
		// pop the stack and call the handler
		if ( contentHnd ) {
		    // report startElement first...
		    if ( d->useNamespaces ) {
			d->namespaceSupport.processName( tags.top(), FALSE, uri, lname );
			t = contentHnd->startElement( uri, lname, tags.top(), d->attList );
		    } else {
			t = contentHnd->startElement( "", "", tags.top(), d->attList );
		    }
		    if ( !t ) {
			d->error = contentHnd->errorString();
			goto parseError;
		    }
		    // ... followed by endElement
		    if ( !contentHnd->endElement( "","",tags.pop() ) ) {
			d->error = contentHnd->errorString();
			goto parseError;
		    }
		} else {
		    tags.pop();
		}
		// next character
		next();
		break;
	    case Attribute:
		// get name and value of attribute
		if ( !parseAttribute() ) {
		    d->error = XMLERR_ERRORPARSINGATTRIBUTE;
		    goto parseError;
		}
		// add the attribute to the list
		if ( d->useNamespaces ) {
		    // is it a namespace declaration?
		    d->namespaceSupport.splitName( name(), prefix, lname );
		    if ( prefix == "xmlns" ) {
			// namespace declaration
			d->namespaceSupport.setPrefix( lname, string() );
			if ( d->useNamespacePrefixes ) {
			    d->attList.qnameList.append( name() );
			    d->attList.uriList.append( "" );
			    d->attList.localnameList.append( "" );
			    d->attList.valueList.append( string() );
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
		    d->attList.uriList.append( "" );
		    d->attList.localnameList.append( "" );
		    d->attList.valueList.append( string() );
		}
		// eliminate white spaces
		eat_ws();
		break;
	    case Done:
		next();
		return TRUE;
		break;
	    case -1:
		d->error = XMLERR_ERRORPARSINGELEMENT;
		goto parseError;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse a content [43].

  A content is only used between tags. If a end tag is found the < is already
  read and the head stand on the '/' of the end tag '</name>'.
*/
bool QXmlSimpleReader::parseContent()
{
    bool charDataRead = FALSE;

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

    // use some kind of state machine for parsing
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input (use lookup-table instead of nested ifs for performance
	// reasons)
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if ( c.row() ) {
	    input = InpUnknown;
	} else {
	    input = mapCLT2FSMChar[ charLookupTable[ c.cell() ] ];
	}

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Init:
		// next character
		next();
		break;
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
		// reference may be CharData; so clear string to be safe
		if ( !charDataRead) {
		    charDataRead = TRUE;
		    stringClear();
		}
		// parse reference
		if ( !parseReference( charDataRead, InContent ) ) {
		    d->error = XMLERR_ERRORPARSINGREFERENCE;
		    goto parseError;
		}
		break;
	    case Lt:
		// call the handler for CharData
		if ( contentHnd ) {
		    if ( charDataRead ) {
			if ( d->reportWhitespaceCharData || !string().simplifyWhiteSpace().isEmpty() ) {
			    if ( !contentHnd->characters( string() ) ) {
				d->error = contentHnd->errorString();
				goto parseError;
			    }
			}
		    }
		}
		charDataRead = FALSE;
		// next character
		next();
		break;
	    case PI:
		// parse processing instruction
		if ( !parsePI() ) {
		    d->error = XMLERR_ERRORPARSINGPI;
		    goto parseError;
		}
		// call the handler
		if ( contentHnd ) {
		    if ( !contentHnd->processingInstruction(name(),string()) ) {
			d->error = contentHnd->errorString();
			goto parseError;
		    }
		}
		break;
	    case Elem:
		// parse element
		if ( !parseElement() ) {
		    d->error = XMLERR_ERRORPARSINGELEMENT;
		    goto parseError;
		}
		break;
	    case Em:
		// next character
		next();
		break;
	    case Com:
		// parse comment
		if ( !parseComment() ) {
		    d->error = XMLERR_ERRORPARSINGCOMMENT;
		    goto parseError;
		}
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			d->error = lexicalHnd->errorString();
			goto parseError;
		    }
		}
		break;
	    case CDS:
		// process the CDSect header ('<![' is already
		// read; now read 'CDATA[')
		next();
		if( !parseString( "CDATA[" ) ) {
		    d->error = XMLERR_CDSECTHEADEREXPECTED;
		    goto parseError;
		}
		// empty string
		stringClear();
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
		// ...and test if this skipping was legal
		if        ( c == '>' ) {
		    // the end of the CDSect
		    if ( lexicalHnd ) {
			if ( !lexicalHnd->startCDATA() ) {
			    d->error = lexicalHnd->errorString();
			    goto parseError;
			}
		    }
		    if ( contentHnd ) {
			if ( d->reportWhitespaceCharData || !string().simplifyWhiteSpace().isEmpty() ) {
			    if ( !contentHnd->characters( string() ) ) {
				d->error = contentHnd->errorString();
				goto parseError;
			    }
			}
		    }
		    if ( lexicalHnd ) {
			if ( !lexicalHnd->endCDATA() ) {
			    d->error = lexicalHnd->errorString();
			    goto parseError;
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
				d->error = contentHnd->errorString();
				goto parseError;
			    }
			}
		    }
		}
		// Done
		return TRUE;
		break;
	    case -1:
		// Error
		d->error = XMLERR_ERRORPARSINGCONTENT;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse Misc [27].
*/
bool QXmlSimpleReader::parseMisc()
{
    const signed char Init             = 0;
    const signed char Lt               = 1; // '<' was read
    const signed char eatWS            = 2; // eat whitespaces
    const signed char PI               = 3; // read PI
    const signed char Comment          = 4; // read comment

    const signed char InpWS            = 0; // S
    const signed char InpLt            = 1; // <
    const signed char InpQm            = 2; // ?
    const signed char InpEm            = 3; // !
    const signed char InpUnknown       = 4;

    // use some kind of state machine for parsing
    static signed char table[2][5] = {
     /*  InpWS   InpLt  InpQm  InpEm     InpUnknown */
	{ eatWS,  Lt,    -1,    -1,       -1    }, // Init
	{ -1,     -1,    PI,    Comment,  -1    }  // Lt
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( is_S(c) ) {
	    input = InpWS;
	} else if ( c == '<' ) {
	    input = InpLt;
	} else if ( c == '?' ) {
	    input = InpQm;
	} else if ( c == '!' ) {
	    input = InpEm;
	} else {
	    input = InpUnknown;
	}

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case eatWS:
		eat_ws();
		return TRUE;
	    case Lt:
		next();
		break;
	    case PI:
		if ( !parsePI() ) {
		    d->error = XMLERR_ERRORPARSINGPI;
		    goto parseError;
		}
		eat_ws();
		return TRUE;
	    case Comment:
		next();
		if ( !parseComment() ) {
		    d->error = XMLERR_ERRORPARSINGCOMMENT;
		    goto parseError;
		}
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			d->error = lexicalHnd->errorString();
			goto parseError;
		    }
		}
		eat_ws();
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_UNEXPECTEDCHARACTER;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse a processing instruction [16].

  If xmldec is TRUE, it tries to parse a PI or a xml declaration [23].

  Precondition: the beginning '<' of the PI is already read and the head stand
  on the '?' of '<?'.

  If this funktion was successful, the head-position is on the first
  character after the PI.
*/
bool QXmlSimpleReader::parsePI(bool xmldecl)
{
    const signed char Init             =  0;
    const signed char Name             =  1; // read Name
    const signed char XMLDecl          =  2; // read XMLDecl
    const signed char PI               =  3; // read PI
    const signed char Version          =  4; // read versionInfo
    const signed char EorSD            =  5; // read EDecl or SDDecl
    const signed char SD               =  6; // read SDDecl
    const signed char ADone            =  7; // almost done
    const signed char Char             =  8; // Char was read
    const signed char Qm               =  9; // Qm was read
    const signed char Done             = 10; // finished reading content

    const signed char InpNameBe        = 0; // is_nameBeginning()
    const signed char InpGt            = 1; // >
    const signed char InpQm            = 2; // ?
    const signed char InpUnknown       = 3;

    // use some kind of state machine for parsing
    static signed char table[11][4] = {
     /*  InpNameBe  InpGt  InpQm   InpUnknown  */
	{ -1,        -1,    Name,   -1     }, // Init
	{ -1,        -1,    -1,     -1     }, // Name (this state is left not through input)
	{ Version,   -1,    -1,     -1     }, // XMLDecl
	{ Char,      Char,  Qm,     Char   }, // PI
	{ EorSD,     -1,    ADone,  -1     }, // Version
	{ SD,        -1,    ADone,  -1     }, // EorSD
	{ -1,        -1,    ADone,  -1     }, // SD
	{ -1,        Done,  -1,     -1     }, // ADone
	{ Char,      Char,  Qm,     Char   }, // Char
	{ Char,      Done,  Qm,     Char   }, // Qm
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( is_NameBeginning(c) ) {
	    input = InpNameBe;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else if ( c == '?' ) {
	    input = InpQm;
	} else {
	    input = InpUnknown;
	}

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Name:
		// get name
		next();
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		// test what name was read and determine the next state
		// (not very beautiful, I admit)
		if ( name().lower() == "xml" ) {
		    if ( xmldecl && name()=="xml" ) {
			state = XMLDecl;
		    } else {
			d->error = XMLERR_INVALIDNAMEFORPI;
			goto parseError;
		    }
		} else {
		    state = PI;
		    stringClear();
		}
		// eat WS
		eat_ws();
		break;
	    case Version:
		// get version (syntax like an attribute)
		if ( !parseAttribute() ) {
		    d->error = XMLERR_VERSIONEXPECTED;
		    goto parseError;
		}
		if ( name() != "version" ) {
		    d->error = XMLERR_VERSIONEXPECTED;
		    goto parseError;
		}
		d->xmlVersion = string();
		// eat WS
		eat_ws();
		break;
	    case EorSD:
		// get the EDecl or SDDecl (syntax like an attribute)
		if ( !parseAttribute() ) {
		    d->error = XMLERR_EDECLORSDDECLEXPECTED;
		    goto parseError;
		}
		if        ( name() == "standalone" ) {
		    if ( string()=="yes" ) {
			d->standalone = QXmlSimpleReaderPrivate::Yes;
		    } else if ( string()=="no" ) {
			d->standalone = QXmlSimpleReaderPrivate::No;
		    } else {
			d->error = XMLERR_WRONGVALUEFORSDECL;
			goto parseError;
		    }
		} else if ( name() == "encoding" ) {
		    d->encoding = string();
		} else {
		    d->error = XMLERR_EDECLORSDDECLEXPECTED;
		    goto parseError;
		}
		// eat WS
		eat_ws();
		break;
	    case SD:
		// get the SDDecl (syntax like an attribute)
		if ( d->standalone != QXmlSimpleReaderPrivate::Unknown ) {
		    // already parsed the standalone declaration
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		if ( !parseAttribute() ) {
		    d->error = XMLERR_SDDECLEXPECTED;
		    goto parseError;
		}
		if ( name() != "standalone" ) {
		    d->error = XMLERR_SDDECLEXPECTED;
		    goto parseError;
		}
		if ( string()=="yes" ) {
		    d->standalone = QXmlSimpleReaderPrivate::Yes;
		} else if ( string()=="no" ) {
		    d->standalone = QXmlSimpleReaderPrivate::No;
		} else {
		    d->error = XMLERR_WRONGVALUEFORSDECL;
		    goto parseError;
		}
		// eat WS
		eat_ws();
		break;
	    case ADone:
		next();
		break;
	    case Char:
		stringAddC();
		next();
		break;
	    case Qm:
		// skip the '?'...
		next();
		// and test if the skipping was legal
		if ( c != '>' ) {
		    stringAddC( '?' );
		}
		break;
	    case Done:
		next();
		return TRUE;
		break;
	    case -1:
		// Error
		d->error = XMLERR_UNEXPECTEDCHARACTER;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse a document type definition (doctypedecl [28]).

  Precondition: the beginning '<!' of the doctype is already read the head
  stands on the 'D' of '<!DOCTYPE'.

  If this funktion was successful, the head-position is on the first
  character after the document type definition.
*/
bool QXmlSimpleReader::parseDoctype()
{
    // some init-stuff
    d->systemId = "";
    d->publicId = "";

    const signed char Init             = 0;
    const signed char Doctype          = 1; // read the doctype
    const signed char Sys              = 2; // read SYSTEM
    const signed char MP               = 3; // markupdecl or PEReference
    const signed char PER              = 4; // PERReference
    const signed char Mup              = 5; // markupdecl
    const signed char MPE              = 6; // end of markupdecl or PEReference
    const signed char Done             = 7;

    const signed char InpD             = 0; // 'D'
    const signed char InpS             = 1; // 'S' or 'P'
    const signed char InpOB            = 2; // [
    const signed char InpCB            = 3; // ]
    const signed char InpPer           = 4; // %
    const signed char InpGt            = 5; // >
    const signed char InpUnknown       = 6;

    // use some kind of state machine for parsing
    static signed char table[7][7] = {
     /*  InpD      InpS  InpOB  InpCB  InpPer InpGt  InpUnknown */
	{ Doctype,  -1,   -1,    -1,    -1,    -1,    -1      }, // Init
	{ -1,       Sys,  MP,    -1,    -1,    Done,  -1      }, // Doctype
	{ -1,       -1,   MP,    -1,    -1,    Done,  -1      }, // Sys
	{ -1,       -1,   -1,    MPE,   PER,   -1,    Mup     }, // MP
	{ -1,       -1,   -1,    MPE,   PER,   -1,    Mup     }, // PER
	{ -1,       -1,   -1,    MPE,   PER,   -1,    Mup     }, // Mup
	{ -1,       -1,   -1,    -1,    -1,    Done,  -1      }  // MPE
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( c == 'D' ) {
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Doctype:
		next();
		if ( !parseString( "OCTYPE" ) ) {
		    d->error = XMLERR_ERRORPARSINGDOCTYPE;
		    goto parseError;
		}
		if ( !is_S(c) ) {
		    d->error = XMLERR_ERRORPARSINGDOCTYPE;
		    goto parseError;
		}
		eat_ws();
		parseName();
		d->doctype = name();
		eat_ws();
		break;
	    case Sys:
		if ( !parseExternalID() ) {
		    d->error = XMLERR_ERRORPARSINGDOCTYPE;
		    goto parseError;
		}
		eat_ws();
		break;
	    case MP:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->startDTD( d->doctype, d->publicId, d->systemId ) ) {
			d->error = lexicalHnd->errorString();
			goto parseError;
		    }
		}
		next();
		eat_ws();
		break;
	    case PER:
		if ( !parsePEReference( InDTD ) ) {
		    d->error = XMLERR_ERRORPARSINGDOCTYPE;
		    goto parseError;
		}
		eat_ws();
		break;
	    case Mup:
		if ( !parseMarkupdecl() ) {
		    d->error = XMLERR_ERRORPARSINGDOCTYPE;
		    goto parseError;
		}
		eat_ws();
		break;
	    case MPE:
		next();
		eat_ws();
		break;
	    case Done:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->endDTD() ) {
			d->error = lexicalHnd->errorString();
			goto parseError;
		    }
		}
		// next character
		next();
		// Done
		return TRUE;
		break;
	    case -1:
		// Error
		d->error = XMLERR_ERRORPARSINGDOCTYPE;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse a ExternalID [75].

  If allowPublicID is TRUE parse ExternalID [75] or PublicID [83].
*/
bool QXmlSimpleReader::parseExternalID( bool allowPublicID )
{
    // some init-stuff
    d->systemId = "";
    d->publicId = "";

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
    const signed char InpWS            = 4; // white space
    const signed char InpUnknown       = 5;

    // use some kind of state machine for parsing
    static signed char table[15][6] = {
     /*  InpSQ    InpDQ    InpS     InpP     InpWS     InpUnknown */
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( is_S(c) ) {
	    input = InpWS;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Sys:
		next();
		if( !parseString( "YSTEM" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case SysWS:
		eat_ws();
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
		next();
		if( !parseString( "UBLIC" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case PubWS:
		eat_ws();
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
		eat_ws();
		break;
	    case PDone:
		if ( allowPublicID ) {
		    d->publicId = string();
		    return TRUE;
		} else {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Done:
		d->systemId = string();
		next();
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_UNEXPECTEDCHARACTER;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
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

    // use some kind of state machine for parsing
    static signed char table[4][9] = {
     /*  InpLt  InpQm  InpEm  InpDash  InpA   InpE   InpL   InpN   InpUnknown */
	{ Lt,    -1,    -1,    -1,      -1,    -1,    -1,    -1,    -1     }, // Init
	{ -1,    Qm,    Em,    -1,      -1,    -1,    -1,    -1,    -1     }, // Lt
	{ -1,    -1,    -1,    Dash,    CA,    CE,    -1,    CN,    -1     }, // Em
	{ -1,    -1,    -1,    -1,      -1,    -1,    CEL,   CEN,   -1     }  // CE
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
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
		if ( !parsePI() ) {
		    d->error = XMLERR_ERRORPARSINGPI;
		    goto parseError;
		}
		return TRUE;
	    case Dash:
		if ( !parseComment() ) {
		    d->error = XMLERR_ERRORPARSINGCOMMENT;
		    goto parseError;
		}
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			d->error = lexicalHnd->errorString();
			goto parseError;
		    }
		}
		return TRUE;
	    case CA:
		if ( !parseAttlistDecl() ) {
		    d->error = XMLERR_ERRORPARSINGATTLISTDECL;
		    goto parseError;
		}
		return TRUE;
	    case CEL:
		if ( !parseElementDecl() ) {
		    d->error = XMLERR_ERRORPARSINGELEMENTDECL;
		    goto parseError;
		}
		return TRUE;
	    case CEN:
		if ( !parseEntityDecl() ) {
		    d->error = XMLERR_ERRORPARSINGENTITYDECL;
		    goto parseError;
		}
		return TRUE;
	    case CN:
		if ( !parseNotationDecl() ) {
		    d->error = XMLERR_ERRORPARSINGNOTATIONDECL;
		    goto parseError;
		}
		return TRUE;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_LETTEREXPECTED;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse a PEReference [69]
*/
bool QXmlSimpleReader::parsePEReference( EntityRecognitionContext context )
{
    const signed char Init             = 0;
    const signed char Next             = 1;
    const signed char Name             = 2;
    const signed char Done             = 3;

    const signed char InpSemi          = 0; // ;
    const signed char InpPer           = 1; // %
    const signed char InpUnknown       = 2;

    // use some kind of state machine for parsing
    static signed char table[3][3] = {
     /*  InpSemi  InpPer  InpUnknown */
	{ -1,      Next,   -1    }, // Init
	{ -1,      -1,     Name  }, // Next
	{ Done,    -1,     -1    }  // Name
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( c == ';' ) {
	    input = InpSemi;
	} else if ( c == '%' ) {
	    input = InpPer;
	} else {
	    input = InpUnknown;
	}

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Next:
		next();
		break;
	    case Name:
		if ( !parseName( TRUE ) ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		if ( d->parameterEntities.find( ref() ) == d->parameterEntities.end() ) {
		    // ### skip it???
		    if ( contentHnd ) {
			if ( !contentHnd->skippedEntity( QString("%") + ref() ) ) {
			    d->error = contentHnd->errorString();
			    goto parseError;
			}
		    }
		} else {
		    if        ( context == InEntityValue ) {
			// Included in literal
			xmlRef = d->parameterEntities.find( ref() )
			    .data().replace( QRegExp("\""), "&quot;" ).replace( QRegExp("'"), "&apos;" )
			    + xmlRef;
		    } else if ( context == InDTD ) {
			// Included as PE ### correct???
			xmlRef = QString(" ") +
			    d->parameterEntities.find( ref() ).data() +
			    QString(" ") + xmlRef;
		    }
		}
		break;
	    case Done:
		next();
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_LETTEREXPECTED;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
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
    const signed char Attdef           =  4; // parse the AttDef
    const signed char Ws2              =  5; // whitespace read
    const signed char Atttype          =  6; // parse the AttType
    const signed char Ws3              =  7; // whitespace read
    const signed char DDecH            =  8; // DefaultDecl with #
    const signed char DefReq           =  9; // parse the string "REQUIRED"
    const signed char DefImp           = 10; // parse the string "IMPLIED"
    const signed char DefFix           = 11; // parse the string "FIXED"
    const signed char Attval           = 12; // parse the AttValue
    const signed char Ws4              = 13; // whitespace read
    const signed char Done             = 14;

    const signed char InpWs            = 0; // white space
    const signed char InpGt            = 1; // >
    const signed char InpHash          = 2; // #
    const signed char InpA             = 3; // A
    const signed char InpI             = 4; // I
    const signed char InpF             = 5; // F
    const signed char InpR             = 6; // R
    const signed char InpUnknown       = 7;

    // use some kind of state machine for parsing
    static signed char table[14][8] = {
     /*  InpWs    InpGt    InpHash  InpA      InpI     InpF     InpR     InpUnknown */
	{ -1,      -1,      -1,      Attlist,  -1,      -1,      -1,      -1      }, // Init
	{ Ws,      -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attlist
	{ -1,      -1,      -1,      Name,     Name,    Name,    Name,    Name    }, // Ws
	{ -1,      Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }, // Name
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Attlist:
		next();
		if( !parseString( "TTLIST" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Ws:
		eat_ws();
		break;
	    case Name:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		d->attDeclEName = name();
		eat_ws();
		break;
	    case Attdef:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		d->attDeclAName = name();
		break;
	    case Ws2:
		eat_ws();
		break;
	    case Atttype:
		if ( !parseAttType() ) {
		    d->error = XMLERR_ERRORPARSINGATTTYPE;
		    goto parseError;
		}
		break;
	    case Ws3:
		eat_ws();
		break;
	    case DDecH:
		next();
		break;
	    case DefReq:
		next();
		if( !parseString( "EQUIRED" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case DefImp:
		next();
		if( !parseString( "MPLIED" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case DefFix:
		next();
		if( !parseString( "IXED" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Attval:
		if ( !parseAttValue() ) {
		    d->error = XMLERR_ERRORPARSINGATTVALUE;
		    goto parseError;
		}
		break;
	    case Ws4:
		if ( declHnd ) {
		    // TODO: not all values are computed yet...
		    if ( !declHnd->attributeDecl( d->attDeclEName, d->attDeclAName, "", "", "" ) ) {
			d->error = declHnd->errorString();
			goto parseError;
		    }
		}
		eat_ws();
		break;
	    case Done:
		next();
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_LETTEREXPECTED;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
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

    // use some kind of state machine for parsing
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case ST:
		next(); // C
		if( !parseString( "DATA" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case TTI:
		next(); // I
		if( !parseString( "D" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case TTI2:
		next(); // R
		if( !parseString( "EF" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case TTI3:
		next(); // S
		break;
	    case TTE:
		next(); // E
		if( !parseString( "NTIT" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case TTEY:
		next(); // Y
		break;
	    case TTEI:
		next(); // I
		if( !parseString( "ES" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case N:
		next(); // N
		break;
	    case TTNM:
		next(); // M
		if( !parseString( "TOKEN" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case TTNM2:
		next(); // S
		break;
	    case NO:
		next(); // O
		if( !parseString( "TATION" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case NO2:
		eat_ws();
		break;
	    case NO3:
		next();
		eat_ws();
		break;
	    case NOName:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		break;
	    case NO4:
		eat_ws();
		break;
	    case EN:
		next();
		eat_ws();
		break;
	    case ENNmt:
		if ( !parseNmtoken() ) {
		    d->error = XMLERR_ERRORPARSINGNMTOKEN;
		    goto parseError;
		}
		break;
	    case EN2:
		eat_ws();
		break;
	    case ADone:
		next();
		return TRUE;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_LETTEREXPECTED;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse a AttValue [10]

  Precondition: the head stands on the beginning " or '

  If this function was successful, the head stands on the first
  character after the closing " or ' and the value of the attribute
  is in string().
*/
bool QXmlSimpleReader::parseAttValue()
{
    bool tmp;

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

    // use some kind of state machine for parsing
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Dq:
	    case Sq:
		stringClear();
		next();
		break;
	    case DqRef:
	    case SqRef:
		if ( !parseReference( tmp, InAttributeValue ) ) {
		    d->error = XMLERR_ERRORPARSINGREFERENCE;
		    goto parseError;
		}
		break;
	    case DqC:
	    case SqC:
		// read one character and add it
		stringAddC();
		next();
		break;
	    case Done:
		// next character
		next();
		// Done
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_UNEXPECTEDCHARACTER;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
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

    // use some kind of state machine for parsing
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// read input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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
	// get new state
	state = table[state][input];

	// in some cases do special actions depending on state
	switch ( state ) {
	    case Elem:
		next(); // L
		if( !parseString( "EMENT" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Ws1:
		eat_ws();
		break;
	    case Nam:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		break;
	    case Ws2:
		eat_ws();
		break;
	    case Empty:
		next(); // E
		if( !parseString( "MPTY" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Any:
		next(); // A
		if( !parseString( "NY" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Cont:
		next();
		eat_ws();
		break;
	    case Mix:
		next(); // #
		if( !parseString( "PCDATA" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Mix2:
		eat_ws();
		break;
	    case Mix3:
		next();
		break;
	    case MixN1:
		next();
		eat_ws();
		break;
	    case MixN2:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		break;
	    case MixN3:
		eat_ws();
		break;
	    case MixN4:
		next();
		break;
	    case Cp:
		if ( !parseChoiceSeq() ) {
		    d->error = XMLERR_ERRORPARSINGCHOICE;
		    goto parseError;
		}
		break;
	    case Cp2:
		next();
		break;
	    case WsD:
		next();
		eat_ws();
		break;
	    case Done:
		next();
		return TRUE;
	    case -1:
		d->error = XMLERR_UNEXPECTEDCHARACTER;
		goto parseError;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
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

    const signed char InpWS            = 0;
    const signed char InpGt            = 1; // >
    const signed char InpN             = 2; // N 
    const signed char InpUnknown       = 3;

    // use some kind of state machine for parsing
    static signed char table[7][4] = {
     /*  InpWS   InpGt  InpN    InpUnknown */
	{ -1,     -1,    Not,    -1     }, // Init
	{ Ws1,    -1,    -1,     -1     }, // Not
	{ -1,     -1,    Nam,    Nam    }, // Ws1
	{ Ws2,    Done,  -1,     -1     }, // Nam
	{ -1,     Done,  ExtID,  ExtID  }, // Ws2
	{ Ws3,    Done,  -1,     -1     }, // ExtID
	{ -1,     Done,  -1,     -1     }  // Ws3
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( is_S(c) ) {
	    input = InpWS;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else if ( c == 'N' ) {
	    input = InpN;
	} else {
	    input = InpUnknown;
	}

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Not:
		next(); // N
		if( !parseString( "OTATION" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Ws1:
		eat_ws();
		break;
	    case Nam:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		break;
	    case Ws2:
		eat_ws();
		break;
	    case ExtID:
		if( !parseExternalID( TRUE ) ) {
		    d->error = XMLERR_ERRORPARSINGEXTERNALID;
		    goto parseError;
		}
		// call the handler
		if ( dtdHnd ) {
		    if ( !dtdHnd->notationDecl( name(), d->publicId, d->systemId ) ) {
			d->error = dtdHnd->errorString();
			goto parseError;
		    }
		}
		break;
	    case Ws3:
		eat_ws();
		break;
	    case Done:
		next();
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_UNEXPECTEDCHARACTER;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
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

    const signed char InpWS            = 0; // S
    const signed char InpOp            = 1; // (
    const signed char InpCp            = 2; // )
    const signed char InpQm            = 3; // ?
    const signed char InpAst           = 4; // *
    const signed char InpPlus          = 5; // +
    const signed char InpPipe          = 6; // |
    const signed char InpComm          = 7; // ,
    const signed char InpUnknown       = 8;

    // use some kind of state machine for parsing
    static signed char table[6][9] = {
     /*  InpWS   InpOp  InpCp  InpQm  InpAst  InpPlus  InpPipe  InpComm  InpUnknown */
	{ -1,     Ws1,   -1,    -1,    -1,     -1,      -1,      -1,      Name  }, // Init
	{ -1,     CS,    -1,    -1,    -1,     -1,      -1,      -1,      CS    }, // Ws1
	{ Ws2,    -1,    Done,  Ws2,   Ws2,    Ws2,     More,    More,    -1    }, // CS
	{ -1,     -1,    Done,  -1,    -1,     -1,      More,    More,    -1    }, // Ws2
	{ -1,     Ws1,   -1,    -1,    -1,     -1,      -1,      -1,      Name  }, // More (same as Init)
	{ Ws2,    -1,    Done,  Ws2,   Ws2,    Ws2,     More,    More,    -1    }  // Name (same as CS)
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( is_S(c) ) {
	    input = InpWS;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Ws1:
		next();
		eat_ws();
		break;
	    case CS:
		if ( !parseChoiceSeq() ) {
		    d->error = XMLERR_ERRORPARSINGCHOICE;
		    goto parseError;
		}
		break;
	    case Ws2:
		next();
		eat_ws();
		break;
	    case More:
		next();
		eat_ws();
		break;
	    case Name:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		break;
	    case Done:
		next();
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_UNEXPECTEDCHARACTER;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
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
    const signed char Done             = 18;

    const signed char InpWs            = 0; // white space
    const signed char InpPer           = 1; // %
    const signed char InpQuot          = 2; // " or '
    const signed char InpGt            = 3; // >
    const signed char InpN             = 4; // N
    const signed char InpUnknown       = 5;

    // use some kind of state machine for parsing
    static signed char table[18][6] = {
     /*  InpWs  InpPer  InpQuot  InpGt  InpN    InpUnknown */
	{ -1,    -1,     -1,      -1,    Ent,    -1      }, // Init
	{ Ws1,   -1,     -1,      -1,    -1,     -1      }, // Ent
	{ -1,    PEDec,  -1,      -1,    Name,   Name    }, // Ws1
	{ Ws2,   -1,     -1,      -1,    -1,     -1      }, // Name
	{ -1,    -1,     EValue,  -1,    -1,     ExtID   }, // Ws2
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // EValue
	{ Ws3,   -1,     -1,      Done,  -1,     -1      }, // ExtID
	{ -1,    -1,     -1,      Done,  Ndata,  -1      }, // Ws3
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Ent:
		next();
		if( !parseString( "TITY" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Ws1:
		eat_ws();
		break;
	    case Name:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		break;
	    case Ws2:
		eat_ws();
		break;
	    case EValue:
		if( !parseEntityValue() ) {
		    d->error = XMLERR_ERRORPARSINGENTITYVALUE;
		    goto parseError;
		}
		if(  d->entities.find( name() ) == d->entities.end() &&
			d->externEntities.find( name() ) == d->externEntities.end() ) {
		    d->entities.insert( name(), string() );
		    if ( declHnd ) {
			if ( !declHnd->internalEntityDecl( name(), string() ) ) {
			    d->error = declHnd->errorString();
			    goto parseError;
			}
		    }
		}
		break;
	    case ExtID:
		if( !parseExternalID() ) {
		    d->error = XMLERR_ERRORPARSINGEXTERNALID;
		    goto parseError;
		}
		if(  d->entities.find( name() ) == d->entities.end() &&
			d->externEntities.find( name() ) == d->externEntities.end() ) {
		    d->externEntities.insert( name(), QXmlSimpleReaderPrivate::ExternEntity( d->publicId, d->systemId, "" ) );
		    if ( declHnd ) {
			if ( !declHnd->externalEntityDecl( name(), d->publicId, d->systemId ) ) {
			    d->error = declHnd->errorString();
			    goto parseError;
			}
		    }
		}
		break;
	    case Ws3:
		eat_ws();
		break;
	    case Ndata:
		next(); // N
		if( !parseString( "DATA" ) ) {
		    d->error = XMLERR_UNEXPECTEDCHARACTER;
		    goto parseError;
		}
		break;
	    case Ws4:
		eat_ws();
		break;
	    case NNam:
		if ( !parseName( TRUE ) ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		if(  d->entities.find( name() ) == d->entities.end() &&
			d->externEntities.find( name() ) == d->externEntities.end() ) {
		    d->externEntities.insert( name(), QXmlSimpleReaderPrivate::ExternEntity( d->publicId, d->systemId, ref() ) );
		    if ( declHnd ) {
			if ( !declHnd->externalEntityDecl( name(), d->publicId, d->systemId ) ) {
			    d->error = declHnd->errorString();
			    goto parseError;
			}
		    }
		}
		break;
	    case PEDec:
		next();
		break;
	    case Ws6:
		eat_ws();
		break;
	    case PENam:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		break;
	    case Ws7:
		eat_ws();
		break;
	    case PEVal:
		if( !parseEntityValue() ) {
		    d->error = XMLERR_ERRORPARSINGENTITYVALUE;
		    goto parseError;
		}
		if(  d->parameterEntities.find( name() ) == d->parameterEntities.end() &&
			d->externParameterEntities.find( name() ) == d->externParameterEntities.end() ) {
		    d->parameterEntities.insert( name(), string() );
		    if ( declHnd ) {
			if ( !declHnd->internalEntityDecl( QString("%")+name(), string() ) ) {
			    d->error = declHnd->errorString();
			    goto parseError;
			}
		    }
		}
		break;
	    case PEEID:
		if( !parseExternalID() ) {
		    d->error = XMLERR_ERRORPARSINGEXTERNALID;
		    goto parseError;
		}
		if(  d->parameterEntities.find( name() ) == d->parameterEntities.end() &&
			d->externParameterEntities.find( name() ) == d->externParameterEntities.end() ) {
		    d->externParameterEntities.insert( name(), QXmlSimpleReaderPrivate::ExternParameterEntity( d->publicId, d->systemId ) );
		    if ( declHnd ) {
			if ( !declHnd->externalEntityDecl( QString("%")+name(), d->publicId, d->systemId ) ) {
			    d->error = declHnd->errorString();
			    goto parseError;
			}
		    }
		}
		break;
	    case WsE:
		eat_ws();
		break;
	    case Done:
		next();
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_LETTEREXPECTED;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse a EntityValue [9]
*/
bool QXmlSimpleReader::parseEntityValue()
{
    bool tmp;

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

    // use some kind of state machine for parsing
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
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
		if ( !parsePEReference( InEntityValue ) ) {
		    d->error = XMLERR_ERRORPARSINGDOCTYPE;
		    goto parseError;
		}
		break;
	    case DqRef:
	    case SqRef:
		if ( !parseReference( tmp, InEntityValue ) ) {
		    d->error = XMLERR_ERRORPARSINGREFERENCE;
		    goto parseError;
		}
		break;
	    case Done:
		next();
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_LETTEREXPECTED;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
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

    // use some kind of state machine for parsing
    static signed char table[6][3] = {
     /*  InpDash  InpGt  InpUnknown */
	{ Dash1,   -1,    -1  }, // Init
	{ Dash2,   -1,    -1  }, // Dash1
	{ Com2,    Com,   Com }, // Dash2
	{ Com2,    Com,   Com }, // Com
	{ ComE,    Com,   Com }, // Com2
	{ -1,      Done,  -1  }  // ComE
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( c == '-' ) {
	    input = InpDash;
	} else if ( c == '>' ) {
	    input = InpGt;
	} else {
	    input = InpUnknown;
	}

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Dash1:
		// next character
		next();
		break;
	    case Dash2:
		// next character
		next();
		// empty string
		stringClear();
		break;
	    case Com:
		// read one character and add it
		stringAddC();
		next();
		break;
	    case Com2:
		// next character
		next();
		// if next character is not a dash than don't skip it
		if ( c != '-' ) {
		    stringAddC( '-' );
		}
		break;
	    case ComE:
		// next character
		next();
		break;
	    case Done:
		// next character
		next();
		// Done
		return TRUE;
		break;
	    case -1:
		// Error
		d->error = XMLERR_ERRORPARSINGCOMMENT;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
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
    const signed char Eq               = 2; // the '=' was read
    const signed char Quotes           = 3; // " or ' were read

    const signed char InpNameBe        = 0;
    const signed char InpEq            = 1; // =
    const signed char InpDq            = 2; // "
    const signed char InpSq            = 3; // '
    const signed char InpUnknown       = 4;

    // use some kind of state machine for parsing
    static signed char table[3][5] = {
     /*  InpNameBe  InpEq  InpDq    InpSq    InpUnknown */
	{ PName,     -1,    -1,      -1,      -1    }, // Init
	{ -1,        Eq,    -1,      -1,      -1    }, // PName
	{ -1,        -1,    Quotes,  Quotes,  -1    }  // Eq
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case PName:
		if ( !parseName() ) {
		    d->error = XMLERR_ERRORPARSINGNAME;
		    goto parseError;
		}
		eat_ws();
		break;
	    case Eq:
		next();
		eat_ws();
		break;
	    case Quotes:
		if ( !parseAttValue() ) {
		    d->error = XMLERR_ERRORPARSINGATTVALUE;
		    goto parseError;
		}
		// Done
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_UNEXPECTEDCHARACTER;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse a Name [5] and store the name in name or ref (if useRef ist TRUE).
*/
bool QXmlSimpleReader::parseName( bool useRef )
{
    const signed char Init             = 0;
    const signed char Name1            = 1; // parse first signed character of the name
    const signed char Name             = 2; // parse name
    const signed char Done             = 3;

    const signed char InpNameBe        = 0; // name beginning signed characters
    const signed char InpNameCh        = 1; // NameChar without InpNameBe
    const signed char InpUnknown       = 2;

    // use some kind of state machine for parsing
    static signed char table[3][3] = {
     /*  InpNameBe  InpNameCh  InpUnknown */
	{ Name1,     -1,        -1    }, // Init
	{ Name,      Name,      Done  }, // Name1
	{ Name,      Name,      Done  }  // Name
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if        ( is_NameBeginning(c) ) {
	    input = InpNameBe;
	} else if ( is_NameChar(c) ) {
	    input = InpNameCh;
	} else {
	    input = InpUnknown;
	}

	// set state according to input
	state = table[state][input];

	// do some actions according to state
	switch ( state ) {
	    case Name1:
		if ( useRef ) {
		    refClear();
		    refAddC();
		} else {
		    nameClear();
		    nameAddC();
		}
		next();
		break;
	    case Name:
		if ( useRef ) {
		    refAddC();
		} else {
		    nameAddC();
		}
		next();
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_LETTEREXPECTED;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
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

    // use some kind of state machine for parsing
    static signed char table[3][2] = {
     /*  InpNameCh  InpUnknown */
	{ NameF,     -1    }, // Init
	{ Name,      Done  }, // NameF
	{ Name,      Done  }  // Name
    };
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if ( is_NameChar(c) ) {
	    input = InpNameCh;
	} else {
	    input = InpUnknown;
	}

	// set state according to input
	state = table[state][input];

	// do some actions according to state
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
	    case Done:
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_LETTEREXPECTED;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse a Reference [67].

  charDataRead is set to TRUE if the reference must not be parsed. The
  character(s) which the reference mapped to are appended to string. The
  head stands on the first character after the reference.

  charDataRead is set to FALSE if the reference must be parsed. The
  charachter(s) which the reference mapped to are inserted at the reference
  position. The head stands on the first character of the replacement).
*/
bool QXmlSimpleReader::parseReference( bool &charDataRead, EntityRecognitionContext context )
{
    // temporary variables
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

    // use some kind of state machine for parsing
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
    signed char state = Init;
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
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

	// set state according to input
	state = table[state][input];

	// do some actions according to state
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
		parseName( TRUE );
		break;
	    case DoneD:
		tmp = ref().toUInt( &ok, 10 );
		if ( ok ) {
		    stringAddC( QChar(tmp) );
		} else {
		    d->error = XMLERR_ERRORPARSINGREFERENCE;
		    goto parseError;
		}
		charDataRead = TRUE;
		next();
		return TRUE;
	    case DoneH:
		tmp = ref().toUInt( &ok, 16 );
		if ( ok ) {
		    stringAddC( QChar(tmp) );
		} else {
		    d->error = XMLERR_ERRORPARSINGREFERENCE;
		    goto parseError;
		}
		charDataRead = TRUE;
		next();
		return TRUE;
	    case DoneN:
		if        ( ref() == "amp" ) {
		    if ( context == InEntityValue ) {
			// Bypassed
			stringAddC( '&' ); stringAddC( 'a' ); stringAddC( 'm' ); stringAddC( 'p' ); stringAddC( ';' );
		    } else {
			// Included or Included in literal
			stringAddC( '&' );
		    }
		    charDataRead = TRUE;
		} else if ( ref() == "lt" ) {
		    if ( context == InEntityValue ) {
			// Bypassed
			stringAddC( '&' ); stringAddC( 'l' ); stringAddC( 't' ); stringAddC( ';' );
		    } else {
			// Included or Included in literal
			stringAddC( '<' );
		    }
		    charDataRead = TRUE;
		} else if ( ref() == "gt" ) {
		    if ( context == InEntityValue ) {
			// Bypassed
			stringAddC( '&' ); stringAddC( 'g' ); stringAddC( 't' ); stringAddC( ';' );
		    } else {
			// Included or Included in literal
			stringAddC( '>' );
		    }
		    charDataRead = TRUE;
		} else if ( ref() == "apos" ) {
		    if ( context == InEntityValue ) {
			// Bypassed
			stringAddC( '&' ); stringAddC( 'a' ); stringAddC( 'p' ); stringAddC( 'o' ); stringAddC( 's' ); stringAddC( ';' );
		    } else {
			// Included or Included in literal
			stringAddC( '\'' );
		    }
		    charDataRead = TRUE;
		} else if ( ref() == "quot" ) {
		    if ( context == InEntityValue ) {
			// Bypassed
			stringAddC( '&' ); stringAddC( 'q' ); stringAddC( 'u' ); stringAddC( 'o' ); stringAddC( 't' ); stringAddC( ';' );
		    } else {
			// Included or Included in literal
			stringAddC( '"' );
		    }
		    charDataRead = TRUE;
		} else {
		    QMap<QString,QString>::Iterator it;
		    it = d->entities.find( ref() );
		    if ( it != d->entities.end() ) {
			// "Internal General"
			switch ( context ) {
			    case InContent:
				// Included
				xmlRef = it.data() + xmlRef;
				charDataRead = FALSE;
				break;
			    case InAttributeValue:
				// Included in literal
				xmlRef = it.data().replace( QRegExp("\""), "&quot;" ).replace( QRegExp("'"), "&apos;" )
					+ xmlRef;
				charDataRead = FALSE;
				break;
			    case InEntityValue:
				{
				    // Bypassed
				    stringAddC( '&' );
				    for ( int i=0; i<(int)ref().length(); i++ ) {
					stringAddC( ref()[i] );
				    }
				    stringAddC( ';');
				    charDataRead = TRUE;
				}
				break;
			    case InDTD:
				// Forbidden
				d->error = XMLERR_INTERNALGENERALENTITYINDTD;
				charDataRead = FALSE;
				break;
			}
		    } else {
			QMap<QString,QXmlSimpleReaderPrivate::ExternEntity>::Iterator itExtern;
			itExtern = d->externEntities.find( ref() );
			if ( itExtern != d->externEntities.end() ) {
			    // "External Parsed General"
			    switch ( context ) {
				case InContent:
				    // Included if validating
				    if ( contentHnd ) {
					if ( !contentHnd->skippedEntity( ref() ) ) {
					    d->error = contentHnd->errorString();
					    goto parseError;
					}
				    }
				    charDataRead = FALSE;
				    break;
				case InAttributeValue:
				    // Forbidden
				    d->error = XMLERR_EXTERNALGENERALENTITYINAV;
				    charDataRead = FALSE;
				    break;
				case InEntityValue:
				    {
					// Bypassed
					stringAddC( '&' );
					for ( int i=0; i<(int)ref().length(); i++ ) {
					    stringAddC( ref()[i] );
					}
					stringAddC( ';');
					charDataRead = TRUE;
				    }
				    break;
				case InDTD:
				    // Forbidden
				    d->error = XMLERR_EXTERNALGENERALENTITYINDTD;
				    charDataRead = FALSE;
				    break;
			    }
			} else {
			    // "Unparsed" ### or is the definition of unparsed entities different?
			    if ( context == InEntityValue ) {
				// Bypassed
				// (this does not conform with the table 4.4 of the XML specification;
				// on the other hand: in this case it is not really an unparsed entity)
				stringAddC( '&' );
				for ( int i=0; i<(int)ref().length(); i++ ) {
				    stringAddC( ref()[i] );
				}
				stringAddC( ';');
				charDataRead = TRUE;
			    } else {
#if 0
				// Forbidden
				d->error = XMLERR_UNPARSEDENTITYREFERENCE;
				goto parseError;
				charDataRead = FALSE;
#else
				// ### skip it???
				if ( contentHnd ) {
				    if ( !contentHnd->skippedEntity( ref() ) ) {
					d->error = contentHnd->errorString();
					goto parseError;
				    }
				}
#endif
			    }
			}
		    }
		}
		next();
		return TRUE;
	    case -1:
		// Error
		d->error = XMLERR_ERRORPARSINGREFERENCE;
		goto parseError;
		break;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}

/*!
  Parse over a simple string.

  After the string was successfully parsed, the head is on the first
  character after the string.
*/
bool QXmlSimpleReader::parseString( const QString& s )
{
    signed char Done                   = s.length();

    const signed char InpCharExpected  = 0; // the character that was expected
    const signed char InpUnknown       = 1;

    signed char state = 0; // state in this function is the position in the string s
    signed char input;

    while ( TRUE ) {

	// get input
	if ( atEnd() ) {
	    d->error = XMLERR_UNEXPECTEDEOF;
	    goto parseError;
	}
	if ( c == s[(int)state] ) {
	    input = InpCharExpected;
	} else {
	    input = InpUnknown;
	}

	// set state according to input
	if ( input == InpCharExpected ) {
	    state++;
	} else {
	    // Error
	    d->error = XMLERR_UNEXPECTEDCHARACTER;
	    goto parseError;
	}

	// do some actions according to state
	next();
	if ( state == Done ) {
	    return TRUE;
	}

    }

    return TRUE;

parseError:
    reportParseError();
    return FALSE;
}


/*
  Init the data values.
*/
void QXmlSimpleReader::init( const QXmlInputSource& i )
{
    xml = i.data();
    xmlLength = xml.length();
    xmlRef = "";

    d->externParameterEntities.clear();
    d->parameterEntities.clear();
    d->externEntities.clear();
    d->entities.clear();

    tags.clear();

    d->doctype = "";
    d->xmlVersion = "";
    d->encoding = "";
    d->standalone = QXmlSimpleReaderPrivate::Unknown;

    lineNr = 0;
    columnNr = -1;
    pos = 0;
    next();
    d->error = XMLERR_OK;
}

void QXmlSimpleReader::reportParseError()
{
    if ( errorHnd )
	errorHnd->fatalError( QXmlParseException( d->error, columnNr, lineNr ) );
}
