#include "docuparser.h"

#include <qxml.h>
#include <qstring.h>

DocuContentParser::DocuContentParser() : QXmlDefaultHandler() 
{ 
} 

bool DocuContentParser::startDocument()
{
    state = StateInit;
    errorProt = "";
    
    nameBuf = "";
    refBuf = "";
    depth = 0;    
    contentList.clear();

    return true;
}

bool DocuContentParser::startElement( const QString &, const QString &,
			       const QString &qname, 
			       const QXmlAttributes &attr )
{
    if( qname == "docu" && state == StateInit )
	state = StateContent;
    else if( qname == "title" && state == StateContent ){
	state = StateDocTitle;
	refBuf = attr.value( "ref" );
	catBuf = attr.value( "category" );
	category = catBuf;
    }  
    else if( qname == "section" && state == StateContent ){
	state = StateSect;
	depth++;    
    }
    else if( qname == "section" && state == StateSect ){
	state = StateSect;
	depth++;
    }
    else if( qname == "title" && state == StateSect ){
	state = StateSectTitle;
	refBuf = attr.value( "ref" );	
    }
    else 
	return FALSE;
    return TRUE;
}

bool DocuContentParser::endElement( const QString &, const QString &,
			     const QString & )
{
    switch( state ){
	case StateInit:
	    break;
	case StateContent:
	    state = StateInit;	    
	    break;
	case StateDocTitle:
	    state = StateContent;
	    break;
	case StateSect:
	    state = StateContent;
	    if( depth ){
		depth--;
		state = StateSect;		
	    }
	    break;
	case StateSectTitle:
	    state = StateSect;
	    break;    	    
    }	
    return TRUE;        
}

bool DocuContentParser::characters( const QString& ch )
{
    nameBuf = ch.simplifyWhiteSpace();
    if ( nameBuf.isEmpty() )
	return TRUE;
        
    switch ( state ) {
	case StateInit:
        case StateContent:        
	case StateDocTitle:
        case StateSect:	    
	case StateSectTitle:	    
            contentList.append( new ContentItem( nameBuf, refBuf, catBuf, depth ));
	    break;
        default:
            return FALSE;
    }    
    return TRUE;
}

QString DocuContentParser::errorProtocol()
{
    return errorProt;
}

bool DocuContentParser::fatalError( const QXmlParseException& exception )
{
    errorProt += QString( "fatal parsing error: %1 in line %2, column %3\n" )
        .arg( exception.message() )
        .arg( exception.lineNumber() )
        .arg( exception.columnNumber() );

    return QXmlDefaultHandler::fatalError( exception );
}

// -----------------------------------------------------------------------


DocuIndexParser::DocuIndexParser() : QXmlDefaultHandler()
{
}

bool DocuIndexParser::startDocument()
{
    state = StateInitial;
    indexlist.clear(); 
    titlelist.clear();
    return true;
}

bool DocuIndexParser::startElement( const QString &, const QString &,
			       const QString &qname, 
			       const QXmlAttributes &attr )
{
    if( qname == "docindex" && state == StateInitial ){
	state = StateIndex;
	category = attr.value( "category" );
    }
    else if( qname == "item" && state == StateIndex ){	
	state = StateItem;
	wordBuf = "\"" + attr.value( "keyword" ) + "\" ";
    }    
    else if( qname == "link" && state == StateItem ){
	state = StateLink;
	descrBuf = attr.value( "descr" );	    
    }
    else 
	return FALSE;
    return TRUE;
}

bool DocuIndexParser::endElement( const QString &, const QString &,
			     const QString & )
{
    switch( state ){
	case StateInitial:
	    break;
	case StateIndex:
	    state = StateInitial;	    
	    break;
	case StateItem:
	    state = StateIndex;
	    break;
	case StateLink:
	    QString Buf = wordBuf + refBuf;
	    indexlist << Buf;
	    int i = refBuf.find("#");
	    refBuf.remove( i, refBuf.length() - i );
	    Buf = refBuf + " | " + descrBuf;
	    titlelist << Buf;	    
	    state = StateItem;
	    break;    	    
    }	
    return TRUE;        
}

bool DocuIndexParser::characters( const QString& ch )
{
    QString buf = ch.simplifyWhiteSpace();
    if ( buf.isEmpty() )
	return TRUE;
        
    switch ( state ) {
	case StateInitial:
        case StateIndex:
        case StateItem:
	    break;
        case StateLink:	    
	    refBuf = buf;
	    break;
	default:
            return FALSE;
    }      
    return TRUE;
}

QString DocuIndexParser::errorProtocol()
{
    return errorProt;
}

bool DocuIndexParser::fatalError( const QXmlParseException& exception )
{
    errorProt += QString( "fatal parsing error: %1 in line %2, column %3\n" )
        .arg( exception.message() )
        .arg( exception.lineNumber() )
        .arg( exception.columnNumber() );

    return QXmlDefaultHandler::fatalError( exception );
}

