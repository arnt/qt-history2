/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Assistant.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "docuparser.h"

#include <qxml.h>
#include <qstring.h>

QDataStream &operator>>( QDataStream &s, ContentItem &ci )
{
    s >> ci.title;
    s >> ci.reference;
    s >> ci.depth;
    return s;
}

QDataStream &operator<<( QDataStream &s, const ContentItem &ci )
{
    s << ci.title;
    s << ci.reference;
    s << ci.depth;
    return s;
}

const QString DocuParser::DocumentKey = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";

DocuParser::DocuParser() : QXmlDefaultHandler()
{
}

bool DocuParser::startDocument()
{
    state = StateInit;
    errorProt = "";

    contentRef = "";
    indexRef = "";
    depth = 0;
    contentList.clear();
    indexList.clear();
    category = "undefined";

    return TRUE;
}

bool DocuParser::startElement( const QString &, const QString &,
			       const QString &qname,
			       const QXmlAttributes &attr )
{
    if( qname == "DCF" && state == StateInit ) {
	state = StateContent;
	contentRef = attr.value( "ref" );
	docTitle = attr.value( "title" );
	title = docTitle;
	category = attr.value( "category" );
	contentList.append( ContentItem( title, contentRef, depth ) );
    }
    else if( qname == "section" && ( state == StateContent || state == StateSect ) ) {
	state = StateSect;
	contentRef = attr.value( "ref" );
	title = attr.value( "title" );
	depth++;
	contentList.append( ContentItem( title, contentRef, depth ) );
    }
    else if ( qname == "keyword" && state == StateSect ) {
	state = StateKeyword;
	indexRef = attr.value( "ref" );
    }
    else
	return FALSE;
    return TRUE;
}

bool DocuParser::endElement( const QString &, const QString &,
			     const QString & )
{
    switch( state ){
	case StateInit:
	    break;
	case StateContent:
	    state = StateInit;
	    break;
	case StateSect:
	    state = StateContent;
	    if( depth ){
		depth--;
		state = StateSect;
	    }
	    break;
	case StateKeyword:
	    state = StateSect;
	    break;
    }
    return TRUE;
}

bool DocuParser::characters( const QString& ch )
{
    QString str = ch.simplifyWhiteSpace();
    if ( str.isEmpty() )
	return TRUE;

    switch ( state ) {
	case StateInit:
        case StateContent:
        case StateSect:
            return FALSE;
	    break;
        case StateKeyword:
	    indexList.append( new IndexItem( str, indexRef ) );
	    break;
	default:
            return FALSE;
    }
    return TRUE;
}

QString DocuParser::errorProtocol() const
{
    return errorProt;
}

bool DocuParser::fatalError( const QXmlParseException& exception )
{
    errorProt += QString( "fatal parsing error: %1 in line %2, column %3\n" )
        .arg( exception.message() )
        .arg( exception.lineNumber() )
        .arg( exception.columnNumber() );

    return QXmlDefaultHandler::fatalError( exception );
}

QString DocuParser::getCategory() const
{
    return category;
}

QString DocuParser::getDocumentationTitle() const
{
    return docTitle;
}

QValueList<ContentItem> DocuParser::getContentItems()
{
    return contentList;
}

QPtrList<IndexItem> DocuParser::getIndexItems()
{
    return indexList;
}
