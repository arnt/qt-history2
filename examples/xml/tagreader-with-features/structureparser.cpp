#include "structureparser.h"

#include <qstring.h>
#include <q3listview.h>

StructureParser::StructureParser( Q3ListView * t )
                : QXmlDefaultHandler()
{
    setListView( t );
}

void StructureParser::setListView( Q3ListView * t )
{
    table = t;
    table->setSorting( -1 );
    table->addColumn( "Qualified name" );
    table->addColumn( "Namespace" );
}

bool StructureParser::startElement( const QString& namespaceURI,
                                    const QString& ,
                                    const QString& qName,
                                    const QXmlAttributes& attributes)
{
    Q3ListViewItem * element;

    if ( ! stack.isEmpty() ){
	Q3ListViewItem *lastChild = stack.top()->firstChild();
	if ( lastChild ) {
	    while ( lastChild->nextSibling() )
		lastChild = lastChild->nextSibling();
	}
	element = new Q3ListViewItem( stack.top(), lastChild, qName, namespaceURI );
    } else {
	element = new Q3ListViewItem( table, qName, namespaceURI );
    }
    stack.push( element );
    element->setOpen( TRUE );

    if ( attributes.length() > 0 ) {
	for ( int i = 0 ; i < attributes.length(); i++ ) {
	    new Q3ListViewItem( element, attributes.qName(i), attributes.uri(i) );
	}
    }
    return TRUE;
}

bool StructureParser::endElement( const QString&, const QString&,
                                  const QString& )
{
    stack.pop();
    return TRUE;
}
