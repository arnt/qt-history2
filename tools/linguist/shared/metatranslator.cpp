/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   metatranslator.cpp
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include <qapplication.h>
#include <qcstring.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qxml.h>

#include "metatranslator.h"

class TsHandler : public QXmlDefaultHandler
{
public:
    TsHandler( MetaTranslator *translator )
	: tor( translator ), type( MetaTranslatorMessage::Finished ),
	  inMessage( FALSE ), ferrorCount( 0 ) { }

    virtual bool startElement( const QString& namespaceURI,
			       const QString& localName, const QString& qName,
			       const QXmlAttributes& atts );
    virtual bool endElement( const QString& namespaceURI,
			     const QString& localName, const QString& qName );
    virtual bool characters( const QString& ch );
    virtual bool fatalError( const QXmlParseException& exception );

private:
    MetaTranslator *tor;
    MetaTranslatorMessage::Type type;
    bool inMessage;
    QCString codec;
    QCString context;
    QCString source;
    QCString comment;
    QString translation;

    QString accum;
    int ferrorCount;
};

bool TsHandler::startElement( const QString& /* namespaceURI */,
			      const QString& /* localName */,
			      const QString& qName,
			      const QXmlAttributes& atts )
{
    if ( qName == QString("context") ) {
	context.truncate( 0 );
	source.truncate( 0 );
	comment.truncate( 0 );
	translation.truncate( 0 );
    } else if ( qName == QString("message") ) {
	inMessage = TRUE;
	type = MetaTranslatorMessage::Finished;
	source.truncate( 0 );
	comment.truncate( 0 );
	translation.truncate( 0 );
    } else if ( qName == QString("translation") ) {
	for ( int i = 0; i < atts.length(); i++ ) {
	    if ( atts.qName(i) == QString("type") ) {
		if ( atts.value(i) == QString("unfinished") )
		    type = MetaTranslatorMessage::Unfinished;
		else if ( atts.value(i) == QString("obsolete") )
		    type = MetaTranslatorMessage::Obsolete;
		else
		    type = MetaTranslatorMessage::Finished;
	    }
	}
    }
    accum.truncate( 0 );
    return TRUE;
}

bool TsHandler::endElement( const QString& /* namespaceURI */,
			    const QString& /* localName */,
			    const QString& qName )
{
    if ( qName == QString("codec") ) {
	tor->setCodec( accum );
    } else if ( qName == QString("name") ) {
	context = accum;
    } else if ( qName == QString("source") ) {
	source = accum;
    } else if ( qName == QString("comment") ) {
	if ( inMessage )
	    comment = accum;
	else
	    tor->insert( MetaTranslatorMessage(context, "", accum,
			 QString::null, MetaTranslatorMessage::Unfinished) );
    } else if ( qName == QString("translation") ) {
	translation = accum;
    } else if ( qName == QString("message") ) {
	tor->insert( MetaTranslatorMessage(context, source, comment,
					   translation, type) );
	inMessage = FALSE;
    }
    return TRUE;
}

bool TsHandler::characters( const QString& ch )
{
    accum += ch;
    return TRUE;
}

bool TsHandler::fatalError( const QXmlParseException& exception )
{
    if ( ferrorCount++ == 0 ) {
	QString msg;
	msg.sprintf( "Parse error at line %d, column %d (%s).",
		     exception.lineNumber(), exception.columnNumber(),
		     exception.message().latin1() );
	if ( qApp == 0 )
	    qWarning( "XML error: %s", msg.latin1() );
	else
	    QMessageBox::information( qApp->mainWidget(),
				      QObject::tr("Qt Linguist"), msg );
    }
    return FALSE;
}

static QCString protect( const QCString& str )
{
    // evil conversion back and forth... still likely to be faster because
    // of all the appending going on.
    int k = 0;
    int l = (int) str.length();
    QString result;
    while( k < l ) {
	switch( str[k] ) {
	case '\\':
	    result += QString::fromLatin1( "&quot;" );
	    break;
	case '&':
	    result += QString::fromLatin1( "&amp;" );
	    break;
	case '>':
	    result += QString::fromLatin1( "&gt;" );
	    break;
	case '<':
	    result += QString::fromLatin1( "&lt;" );
	    break;
	case '\'':
	    result += QString::fromLatin1( "&apos;" );
	    break;
	default:
	    result += str[k];
	    break;
	}
	k++;
    }
    return result.ascii();
}

static QCString evilBytes( const QCString& str )
{
    // ditto
    int k = 0;
    QString result;
    QCString tmp = protect( str );
    int l = (int) tmp.length();
    while( k < l ) {
	if ( (uchar)(tmp[k]) > 127 )
	    result += ( QString::fromLatin1( "&#x" ) +
		        QString::number( (uint)(uchar)(tmp[k]), 16 ) +
			QString::fromLatin1( ";" ) );
	else
	    result += QChar( tmp[k] );
	k++;
    }
    return result.ascii();
}

MetaTranslatorMessage& MetaTranslatorMessage::operator=(
	const MetaTranslatorMessage& m )
{
    QTranslatorMessage::operator=( m );
    ty = m.ty;
    return *this;
}

bool MetaTranslatorMessage::operator==( const MetaTranslatorMessage& m ) const
{
    return strcmp( context(), m.context() ) == 0 &&
	   strcmp( sourceText(), m.sourceText() ) == 0 &&
	   strcmp( comment(), m.comment() ) == 0;
}

bool MetaTranslatorMessage::operator<( const MetaTranslatorMessage& m ) const
{
    int delta = strcmp( context(), m.context() );
    if ( delta == 0 )
	delta = strcmp( sourceText(), m.sourceText() );
    if ( delta == 0 )
	delta = strcmp( comment(), m.comment() );
    return delta < 0;
}

MetaTranslator::MetaTranslator()
    : codec( "iso8859-1" )
{
}

MetaTranslator& MetaTranslator::operator=( const MetaTranslator& tor )
{
    mm = tor.mm;
    codec = tor.codec;
    return *this;
}

bool MetaTranslator::load( const QString& filename )
{
    mm.clear();

    QFile f( filename );
    if ( !f.open(IO_ReadOnly) )
	return FALSE;

    QTextStream t( &f );
    QXmlInputSource in( t );
    QXmlSimpleReader reader;
    // don't click on these!
    reader.setFeature( "http://xml.org/sax/features/namespaces", FALSE );
    reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", TRUE );
    reader.setFeature( "http://trolltech.com/xml/features/report-whitespace"
		       "-only-CharData", FALSE );
    QXmlDefaultHandler *hand = new TsHandler( this );
    reader.setContentHandler( hand );
    reader.setErrorHandler( hand );

    bool ok = reader.parse( in );
    reader.setContentHandler( 0 );
    reader.setErrorHandler( 0 );
    delete hand;
    f.close();
    if ( !ok )
	mm.clear();
    return ok;
}

bool MetaTranslator::save( const QString& filename ) const
{
    QFile f( filename );
    if ( !f.open(IO_WriteOnly) )
	return FALSE;

    QTextStream t( &f );
    t.setCodec( QTextCodec::codecForName( "ISO 8859-1" ) );

    t << "<!DOCTYPE TS><TS>\n";
    t << "<codec>" << codec << "</codec>\n";
    TMM::ConstIterator m = mm.begin();
    while ( m != mm.end() ) {
	TMMInv inv;
	TMMInv::Iterator i;
	QCString context = m.key().context();
	QCString comment = "";

	do {
	    if ( QCString(m.key().sourceText()).isEmpty() )
		comment += QCString( m.key().comment() );
	    else
		inv.insert( *m, m.key() );
	} while ( ++m != mm.end() && QCString(m.key().context()) == context );

	t << "<context>\n";
	t << "    <name>" << evilBytes( context ) << "</name>\n";
	if ( !comment.isEmpty() )
	    t << "    <comment>" << evilBytes( comment ) << "</comment>\n";

	for ( i = inv.begin(); i != inv.end(); ++i ) {
	    t << "    <message>\n"
	      << "        <source>" << evilBytes( (*i).sourceText() )
	      << "</source>\n";
	    if ( !QCString((*i).comment()).isEmpty() )
		t << "        <comment>" << evilBytes( (*i).comment() )
		  << "</comment>\n";
	    t << "        <translation";
	    if ( (*i).type() == MetaTranslatorMessage::Unfinished )
		t << " type='unfinished'";
	    else if ( (*i).type() == MetaTranslatorMessage::Obsolete )
		t << " type='obsolete'";
	    t << ">" << protect( (*i).translation().utf8() )
	      << "</translation>\n";
	    t << "    </message>\n";
	}
	t << "</context>\n";
    }
    t << "</TS>\n";
    f.close();
    return TRUE;
}

bool MetaTranslator::release( const QString& filename ) const
{
    QTranslator tor( 0 );
    TMM::ConstIterator m;
    for ( m = mm.begin(); m != mm.end(); ++m )
	tor.insert( m.key() );
    return tor.save( filename, QTranslator::Stripped );
}

bool MetaTranslator::contains( const char *context, const char *sourceText,
			       const char *comment ) const
{
    return mm.find( MetaTranslatorMessage(context, sourceText, comment) ) !=
	   mm.end();
}

void MetaTranslator::insert( const MetaTranslatorMessage& m )
{
    int pos = mm.count();
    TMM::Iterator n = mm.find( m );
    if ( n != mm.end() )
	pos = *n;
    mm.replace( m, pos );
}

QString MetaTranslator::toUnicode( const char *str ) const
{
    QTextCodec *c = QTextCodec::codecForName( codec );
    if ( c == 0 )
	return QString::fromLatin1( str );
    else
	return c->toUnicode( str );
}

QValueList<MetaTranslatorMessage> MetaTranslator::messages() const
{
    int n = mm.count();
    TMM::ConstIterator *t = new TMM::ConstIterator[n + 1];
    TMM::ConstIterator m;
    for ( m = mm.begin(); m != mm.end(); ++m )
	t[*m] = m;

    QValueList<MetaTranslatorMessage> val;
    for ( int i = 0; i < n; i++ )
	val.append( t[i].key() );

    delete[] t;
    return val;
}
