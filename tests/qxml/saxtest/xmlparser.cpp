#include "xmlparser.h"

XMLParser::XMLParser( QListView *protocol, QLabel *err, QListView *t, QTextStream *ts )
{
    loc = 0;

    parseProtocol = protocol;
    parseProtocolTS = ts;
    parseProtocolItem = 0;

    errorProtocol = err;

    tree = t;
    treeItem = 0;
    treeItemAfter = 0;
}


XMLParser::~XMLParser()
{
}


void XMLParser::setDocumentLocator( QXmlLocator* locator )
{
    loc = locator;
}


bool XMLParser::startDocument()
{
    addToProtocol( "startDocument", "" );
    return TRUE;
}


bool XMLParser::endDocument()
{
    addToProtocol( "endDocument", "" );
    return TRUE;
}


bool XMLParser::startPrefixMapping( const QString& prefix, const QString& uri )
{
    addToProtocol( "startPrefixMapping", prefix + " " + uri );
    return TRUE;
}


bool XMLParser::endPrefixMapping( const QString& prefix )
{
    addToProtocol( "endPrefixMapping", prefix );
    return TRUE;
}


bool XMLParser::startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts )
{
    // build tree
    QListViewItem* tmp;
    QString a, attIndicator;
    for ( int i=0; i<atts.length(); i++ ) {
	attIndicator = "-+-ATT-+-";
	a += atts.qName(i) +
	    "(" + atts.uri(i) + ":" + atts.localName(i) + ")" +
	    "=" + atts.value(i) +
	    "\n";
    }
    if ( treeItem == 0 ) {
	tmp = new QListViewItem( tree, treeItemAfter,
		qName, namespaceURI, localName, attIndicator,
		a );
    } else {
	tmp = new QListViewItem( treeItem, treeItemAfter,
		qName, namespaceURI, localName, attIndicator,
		a );
    }
    treeItem = tmp;
    treeItemAfter = 0;

    // build protocol
    addToProtocol( "startElement", namespaceURI + " " + localName + " " + qName );
    return TRUE;
}


bool XMLParser::endElement( const QString& namespaceURI, const QString& localName, const QString& qName )
{
    // build tree
    treeItemAfter = treeItem;
    treeItem = treeItem->parent();

    // build protocol
    addToProtocol( "endElement", namespaceURI + " " + localName + " " + qName );
    return TRUE;
}


bool XMLParser::characters( const QString& ch )
{
    // build tree
    QString cs = ch.stripWhiteSpace();
    if ( !cs.isEmpty() ) {
	QListViewItem *tmp = new QListViewItem( treeItem, treeItemAfter,
		"-+-characters-+-", "", "", "",
		ch );
	treeItemAfter = tmp;
    }

    // build protocol
    addToProtocol( "characters", ch );
    return TRUE;
}


bool XMLParser::ignorableWhitespace( const QString& ch )
{
    addToProtocol( "ignorableWhitespace", ch );
    return TRUE;
}


bool XMLParser::processingInstruction( const QString& target, const QString& data )
{
    addToProtocol( "processingInstruction", target + " " + data );
    return TRUE;
}


bool XMLParser::skippedEntity( const QString& name )
{
    addToProtocol( "skippedEntity", name );
    return TRUE;
}


bool XMLParser::warning( const QXmlParseException& )//exception )
{
    addToProtocol( "warning", "" );
    return TRUE;
}


bool XMLParser::error( const QXmlParseException& )//exception )
{
    addToProtocol( "error", "" );
    return TRUE;
}


bool XMLParser::fatalError( const QXmlParseException& exception )
{
    addToProtocol( "fatalError", "" );

    QString errorString = QString( "fatal parsing error: %1 in line %2, column %3\n" )
	.arg( exception.message() )
	.arg( exception.lineNumber() )
	.arg( exception.columnNumber() );
    errorProtocol->setText( errorProtocol->text() + errorString );

    return QXmlDefaultHandler::fatalError( exception );
}


bool XMLParser::notationDecl( const QString& name, const QString& publicId, const QString& systemId )
{
    addToProtocol( "notationDecl", name + " " + publicId + " " + systemId );
    return TRUE;
}


bool XMLParser::unparsedEntityDecl( const QString& name, const QString& publicId, const QString& systemId, const QString& notationName )
{
    addToProtocol( "unparsedEntityDecl", name + " " + publicId + " " + systemId + " " + notationName );
    return TRUE;
}


bool XMLParser::resolveEntity( const QString& publicId, const QString& systemId, QXmlInputSource* ret )
{
    addToProtocol( "resolveEntity", publicId + " " + systemId );
    ret = 0;
    return TRUE;
}


bool XMLParser::startDTD( const QString& name, const QString& publicId, const QString& systemId )
{
    addToProtocol( "startDTD", name + " " + publicId + " " + systemId );
    return TRUE;
}


bool XMLParser::endDTD()
{
    addToProtocol( "endDTD", "" );
    return TRUE;
}


bool XMLParser::startEntity( const QString& name )
{
    addToProtocol( "startEntity", name );
    return TRUE;
}


bool XMLParser::endEntity( const QString& name )
{
    addToProtocol( "endEntity", name );
    return TRUE;
}


bool XMLParser::startCDATA()
{
    addToProtocol( "startCDATA", "" );
    return TRUE;
}


bool XMLParser::endCDATA()
{
    addToProtocol( "endCDATA", "" );
    return TRUE;
}


bool XMLParser::comment( const QString& ch )
{
    addToProtocol( "comment", ch );
    return TRUE;
}


bool XMLParser::attributeDecl( const QString& eName, const QString& aName, const QString& type, const QString& valueDefault, const QString& value )
{
    addToProtocol( "attributeDecl", eName + " " + aName + " " + type + " " + valueDefault + " " + value );
    return TRUE;
}


bool XMLParser::internalEntityDecl( const QString& name, const QString& value )
{
    addToProtocol( "internalEntityDecl", name + " " + value );
    return TRUE;
}


bool XMLParser::externalEntityDecl( const QString& name, const QString& publicId, const QString& systemId )
{
    addToProtocol( "externalEntityDecl", name + " " + publicId + " " + systemId );
    return TRUE;
}

void XMLParser::addToProtocol( const QString& name, const QString &args )
{
    QString row, column;
    if ( loc != 0 ) {
	row.setNum( loc->lineNumber() );
	column.setNum( loc->columnNumber() );
    } else {
	row = "?";
	column = "?";
    }
    QListViewItem *tmp = new QListViewItem( parseProtocol, parseProtocolItem,
	    name, args, row, column );
    parseProtocolItem = tmp;

    *parseProtocolTS << "(" << row << "," << column << ")"
	<< name << " | " << args << endl;
}
