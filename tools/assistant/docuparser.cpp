#include "docuparser.h"

#include <qxml.h>
#include <qstring.h>


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
    if( qname == "DOC" && state == StateInit ) {
	state = StateContent;
	contentRef = attr.value( "ref" );
	title = attr.value( "title" );
	category = attr.value( "category" );	
	contentList.append( new ContentItem( title, contentRef, depth ) );
    }
    else if( qname == "section" && ( state == StateContent || state == StateSect ) ) {
	state = StateSect;
	contentRef = attr.value( "ref" );
	title = attr.value( "title" );
	depth++;    
	contentList.append( new ContentItem( title, contentRef, depth ) );
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

QString DocuParser::errorProtocol()
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

QString DocuParser::getCategory()
{
    return category;
}

QPtrList<ContentItem> DocuParser::getContentItems()
{
    return contentList;
}

QPtrList<IndexItem> DocuParser::getIndexItems()
{
    return indexList;
}
