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

#ifndef QXML_H
#define QXML_H

#include <qtextstream.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluestack.h>
#include <qmap.h>


class QXmlNamespaceSupport;
class QXmlAttributes;
class QXmlContentHandler;
class QXmlDefaultHandler;
class QXmlDTDHandler;
class QXmlEntityResolver;
class QXmlErrorHandler;
class QXmlLexicalHandler;
class QXmlDeclHandler;
class QXmlInputSource;
class QXmlLocator;
class QXmlNamespaceSupport;
class QXmlParseException;

class QXmlReader;

class QXmlSimpleReader;
class QXmlSimpleReaderPrivate;


//
// SAX Namespace Support
//

class QXmlNamespaceSupport
{
public:
    QXmlNamespaceSupport();
    ~QXmlNamespaceSupport();

    void setPrefix( const QString&, const QString& );

    QString prefix( const QString& );
    QString uri( const QString& );
    void splitName( const QString&, QString&, QString& );
    void processName( const QString&, bool, QString&, QString& );
    QStringList prefixes();
    QStringList prefixes( const QString& );

    void pushContext();
    void popContext();
    void reset();
private:
    typedef QMap<QString,QString> namespaceMap;
    QValueStack<namespaceMap> nsStack;
    namespaceMap ns;
};


//
// SAX Attributes
//

class QXmlAttributes
{
public:
    QXmlAttributes() {}
    virtual ~QXmlAttributes() {}

    int index( const QString& qName ) const;
    int index( const QString& uri, const QString& localPart ) const;
    int length() const;
    QString localName( int index ) const;
    QString qName( int index ) const;
    QString uri( int index ) const;
    QString type( int index ) const;
    QString type( const QString& qName ) const;
    QString type( const QString& uri, const QString& localName ) const;
    QString value( int index ) const;
    QString value( const QString& qName ) const;
    QString value( const QString& uri, const QString& localName ) const;

private:
    QValueList<QString> qnameList;
    QValueList<QString> uriList;
    QValueList<QString> localnameList;
    QValueList<QString> valueList;

    friend class QXmlSimpleReader;
};

//
// SAX Input Source
//

class QXmlInputSource
{
public:
    QXmlInputSource( );
    QXmlInputSource( QTextStream& stream );
    QXmlInputSource( const QString& systemId );

    inline const QString& data() const
    { return input; }

    void setData( const QString& d );

private:
    QString input;
};

//
// SAX Exception Classes
//

class QXmlParseException
{
public:
    QXmlParseException( const QString& name="", int c=-1, int l=-1, const QString& p="", const QString& s="" )
	: msg( name ), column( c ), line( l ), pub( p ), sys( s )
    { }
    int columnNumber() const
    { return column; }
    int lineNumber() const
    { return line; }
    const QString& publicId() const
    { return pub; }
    const QString& systemId() const
    { return sys; }
    const QString& message() const
    { return msg; }
private:
    QString msg;
    int column;
    int line;
    QString pub;
    QString sys;
};


//
// XML Reader
//

class QXmlReader
{
public:
    virtual bool feature( const QString& name, bool *ok = 0 ) const = 0;
    virtual void setFeature( const QString& name, bool value ) = 0;
    virtual bool hasFeature( const QString& name ) const = 0;
    // getProperty()
    // setProperty()
    virtual void setEntityResolver( QXmlEntityResolver* handler ) = 0;
    virtual QXmlEntityResolver* entityResolver() const = 0;
    virtual void setDTDHandler( QXmlDTDHandler* handler ) = 0;
    virtual QXmlDTDHandler* DTDHandler() const = 0;
    virtual void setContentHandler( QXmlContentHandler* handler ) = 0;
    virtual QXmlContentHandler* contentHandler() const = 0;
    virtual void setErrorHandler( QXmlErrorHandler* handler ) = 0;
    virtual QXmlErrorHandler* errorHandler() const = 0;
    virtual void setLexicalHandler( QXmlLexicalHandler* handler ) = 0;
    virtual QXmlLexicalHandler* lexicalHandler() const = 0;
    virtual void setDeclHandler( QXmlDeclHandler* handler ) = 0;
    virtual QXmlDeclHandler* declHandler() const = 0;
    virtual bool parse( const QXmlInputSource& input ) = 0;
    virtual bool parse( const QString& systemId ) = 0;
};

class QXmlSimpleReaderPrivate;

class QXmlSimpleReader : public QXmlReader
{
public:
    QXmlSimpleReader();
    virtual ~QXmlSimpleReader();

    bool feature( const QString& name, bool *ok = 0 ) const;
    void setFeature( const QString& name, bool value );
    bool hasFeature( const QString& name ) const;

    void setEntityResolver( QXmlEntityResolver* handler );
    QXmlEntityResolver* entityResolver() const;
    void setDTDHandler( QXmlDTDHandler* handler );
    QXmlDTDHandler* DTDHandler() const;
    void setContentHandler( QXmlContentHandler* handler );
    QXmlContentHandler* contentHandler() const;
    void setErrorHandler( QXmlErrorHandler* handler );
    QXmlErrorHandler* errorHandler() const;
    void setLexicalHandler( QXmlLexicalHandler* handler );
    QXmlLexicalHandler* lexicalHandler() const;
    void setDeclHandler( QXmlDeclHandler* handler );
    QXmlDeclHandler* declHandler() const;

    bool parse( const QXmlInputSource& input );
    bool parse( const QString& systemId );

private:
    // variables
    QXmlContentHandler* contentHnd;
    QXmlErrorHandler*   errorHnd;
    QXmlDTDHandler*     dtdHnd;
    QXmlEntityResolver* entityRes;
    QXmlLexicalHandler* lexicalHnd;
    QXmlDeclHandler*    declHnd;

    QChar c; // the character at reading position
    int lineNr; // number of line
    int columnNr; // position in line
    int pos; // position in string

    int     namePos;
    QChar   nameArray[256]; // only used for names
    QString nameValue; // only used for names
    int     refPos;
    QChar   refArray[256]; // only used for references
    QString refValue; // only used for references
    int     stringPos;
    QChar   stringArray[256]; // used for any other strings that are parsed
    QString stringValue; // used for any other strings that are parsed

    QString xml;
    int xmlLength;
    QString xmlRef; // used for parsing of entity references

    QValueStack<QString> tags;

    QXmlSimpleReaderPrivate* d;

    static const QChar QEOF;

    // inlines
    virtual bool is_S( const QChar& c );
    virtual bool is_Letter( const QChar& c );
    virtual bool is_NameBeginning( const QChar& c );
    virtual bool is_Digit( const QChar& c );
    virtual bool is_CombiningChar( const QChar& );
    virtual bool is_Extender( const QChar& );
    virtual bool is_NameChar( const QChar& c );

    QString& string();
    void stringClear();
    void stringAddC();
    void stringAddC(const QChar& c);
    QString& name();
    void nameClear();
    void nameAddC();
    void nameAddC(const QChar& c);
    QString& ref();
    void refClear();
    void refAddC();
    void refAddC(const QChar& c);

    // used by parseReference() and parsePEReference()
    enum EntityRecognitionContext { InContent, InAttributeValue, InEntityValue, InDTD };

    // private methods
    void eat_ws();

    void next();
    bool atEnd();

    void init( const QXmlInputSource& i );

    bool parseProlog();
    bool parseElement();
    bool parseMisc();
    bool parseContent();

    bool parsePI(bool xmldecl=FALSE);
    bool parseDoctype();
    bool parseComment();

    bool parseName( bool useRef=FALSE );
    bool parseNmtoken();
    bool parseAttribute();
    bool parseReference( bool &charDataRead, EntityRecognitionContext context );

    bool parseExternalID( bool allowPublicID = FALSE );
    bool parsePEReference( EntityRecognitionContext context );
    bool parseMarkupdecl();
    bool parseAttlistDecl();
    bool parseAttType();
    bool parseAttValue();
    bool parseElementDecl();
    bool parseNotationDecl();
    bool parseChoiceSeq();
    bool parseEntityDecl();
    bool parseEntityValue();

    bool parseString( const QString& s );

    void reportParseError();

    friend class QXmlSimpleReaderPrivate;
    friend class QXmlLocator;
};

//
// SAX Locator
//

class QXmlLocator
{
public:
    QXmlLocator( QXmlSimpleReader* parent )
    { reader = parent; }
    ~QXmlLocator()
    { }

    inline int columnNumber()
    { return ( reader->columnNr == -1 ? -1 : reader->columnNr + 1 ); }
    inline int lineNumber()
    { return ( reader->lineNr == -1 ? -1 : reader->columnNr + 1 ); }
//    inline QString getPublicId()
//    inline QString getSystemId()

private:
    QXmlSimpleReader* reader;
};

//
// SAX handler classes
//

class QXmlContentHandler
{
public:
    virtual void setDocumentLocator( QXmlLocator* locator ) = 0;
    virtual bool startDocument() = 0;
    virtual bool endDocument() = 0;
    virtual bool startPrefixMapping( const QString& prefix, const QString& uri ) = 0;
    virtual bool endPrefixMapping( const QString& prefix ) = 0;
    virtual bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts ) = 0;
    virtual bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName ) = 0;
    virtual bool characters( const QString& ch ) = 0;
    virtual bool ignorableWhitespace( const QString& ch ) = 0;
    virtual bool processingInstruction( const QString& target, const QString& data ) = 0;
    virtual bool skippedEntity( const QString& name ) = 0;
    virtual QString errorString() = 0;
};

class QXmlErrorHandler
{
public:
    virtual bool warning( const QXmlParseException& exception ) = 0;
    virtual bool error( const QXmlParseException& exception ) = 0;
    virtual bool fatalError( const QXmlParseException& exception ) = 0;
    virtual QString errorString() = 0;
};

class QXmlDTDHandler
{
public:
    virtual bool notationDecl( const QString& name, const QString& publicId, const QString& systemId ) = 0;
    virtual bool unparsedEntityDecl( const QString& name, const QString& publicId, const QString& systemId, const QString& notationName ) = 0;
    virtual QString errorString() = 0;
};

class QXmlEntityResolver
{
public:
    virtual bool resolveEntity( const QString& publicId, const QString& systemId, QXmlInputSource* ret ) = 0;
    virtual QString errorString() = 0;
};

class QXmlLexicalHandler
{
public:
    virtual bool startDTD( const QString& name, const QString& publicId, const QString& systemId ) = 0;
    virtual bool endDTD() = 0;
//    virtual bool startEntity( const QString& name ) = 0;
//    virtual bool endEntity( const QString& name ) = 0;
    virtual bool startCDATA() = 0;
    virtual bool endCDATA() = 0;
    virtual bool comment( const QString& ch ) = 0;
    virtual QString errorString() = 0;
};

class QXmlDeclHandler
{
public:
    virtual bool attributeDecl( const QString& eName, const QString& aName, const QString& type, const QString& valueDefault, const QString& value ) = 0;
    virtual bool internalEntityDecl( const QString& name, const QString& value ) = 0;
    virtual bool externalEntityDecl( const QString& name, const QString& publicId, const QString& systemId ) = 0;
    virtual QString errorString() = 0;
};

class QXmlDefaultHandler : public QXmlContentHandler, public QXmlErrorHandler, public QXmlDTDHandler, public QXmlEntityResolver, public QXmlLexicalHandler, public QXmlDeclHandler
{
public:
    QXmlDefaultHandler() { }
    virtual ~QXmlDefaultHandler() { }

    void setDocumentLocator( QXmlLocator* locator );
    bool startDocument();
    bool endDocument();
    bool startPrefixMapping( const QString& prefix, const QString& uri );
    bool endPrefixMapping( const QString& prefix );
    bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
    bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
    bool characters( const QString& ch );
    bool ignorableWhitespace( const QString& ch );
    bool processingInstruction( const QString& target, const QString& data );
    bool skippedEntity( const QString& name );

    bool warning( const QXmlParseException& exception );
    bool error( const QXmlParseException& exception );
    bool fatalError( const QXmlParseException& exception );

    bool notationDecl( const QString& name, const QString& publicId, const QString& systemId );
    bool unparsedEntityDecl( const QString& name, const QString& publicId, const QString& systemId, const QString& notationName );

    bool resolveEntity( const QString& publicId, const QString& systemId, QXmlInputSource* ret );

    bool startDTD( const QString& name, const QString& publicId, const QString& systemId );
    bool endDTD();
//    bool startEntity( const QString& name );
//    bool endEntity( const QString& name );
    bool startCDATA();
    bool endCDATA();
    bool comment( const QString& ch );

    bool attributeDecl( const QString& eName, const QString& aName, const QString& type, const QString& valueDefault, const QString& value );
    bool internalEntityDecl( const QString& name, const QString& value );
    bool externalEntityDecl( const QString& name, const QString& publicId, const QString& systemId );

    QString errorString();
};


//
// inlines
//

inline bool QXmlSimpleReader::is_S(const QChar& c)
{ return c==' ' || c=='\t' || c=='\n' || c=='\r'; }

inline bool QXmlSimpleReader::is_Letter( const QChar& c )
{ return c.isLetter(); }

inline bool QXmlSimpleReader::is_NameBeginning( const QChar& c )
{ return c=='_' || c==':' || c.isLetter(); }

inline bool QXmlSimpleReader::is_Digit( const QChar& c )
{ return c.isDigit(); }

inline bool QXmlSimpleReader::is_CombiningChar( const QChar& )
{ return FALSE; }

inline bool QXmlSimpleReader::is_Extender( const QChar& )
{ return FALSE; }

inline bool QXmlSimpleReader::is_NameChar( const QChar& c )
{
    return c=='.' || c=='-' || c=='_' || c==':' ||
	is_Letter(c) || is_Digit(c) ||
	is_CombiningChar(c) || is_Extender(c);
}

inline void QXmlSimpleReader::next()
{
    if ( !xmlRef.isEmpty() ) {
	c = xmlRef[0];
	xmlRef.remove( 0, 1 );
    } else {
	if ( c=='\n' || c=='\r' ) {
	    lineNr++;
	    columnNr = -1;
	}
	if ( pos >= xmlLength ) {
	    c = QEOF;
	} else {
	    c = xml[pos];
	    columnNr++;
	    pos++;
	}
    }
}

inline bool QXmlSimpleReader::atEnd()
{ return c == QEOF; }

inline void QXmlSimpleReader::eat_ws()
{ while ( !atEnd() && is_S(c) ) next(); }


// use buffers instead of QString::operator+= when single characters are read
inline QString& QXmlSimpleReader::string()
{
    stringValue += QString( stringArray, stringPos );
    stringPos = 0;
    return stringValue;
}
inline QString& QXmlSimpleReader::name()
{
    nameValue += QString( nameArray, namePos );
    namePos = 0;
    return nameValue;
}
inline QString& QXmlSimpleReader::ref()
{
    refValue += QString( refArray, refPos );
    refPos = 0;
    return refValue;
}

inline void QXmlSimpleReader::stringClear()
{ stringValue = ""; stringPos = 0; }
inline void QXmlSimpleReader::nameClear()
{ nameValue = ""; namePos = 0; }
inline void QXmlSimpleReader::refClear()
{ refValue = ""; refPos = 0; }

inline void QXmlSimpleReader::stringAddC()
{
    if ( stringPos >= 256 ) {
	stringValue += QString( stringArray, stringPos );
	stringPos = 0;
    }
    stringArray[stringPos++] = c;
}
inline void QXmlSimpleReader::nameAddC()
{
    if ( namePos >= 256 ) {
	nameValue += QString( nameArray, namePos );
	namePos = 0;
    }
    nameArray[namePos++] = c;
}
inline void QXmlSimpleReader::refAddC()
{
    if ( refPos >= 256 ) {
	refValue += QString( refArray, refPos );
	refPos = 0;
    }
    refArray[refPos++] = c;
}

inline void QXmlSimpleReader::stringAddC(const QChar& c)
{
    if ( stringPos >= 256 ) {
	stringValue += QString( stringArray, stringPos );
	stringPos = 0;
    }
    stringArray[stringPos++] = c;
}
inline void QXmlSimpleReader::nameAddC(const QChar& c)
{
    if ( namePos >= 256 ) {
	nameValue += QString( nameArray, namePos );
	namePos = 0;
    }
    nameArray[namePos++] = c;
}
inline void QXmlSimpleReader::refAddC(const QChar& c)
{
    if ( refPos >= 256 ) {
	refValue += QString( refArray, refPos );
	refPos = 0;
    }
    refArray[refPos++] = c;
}

#endif
