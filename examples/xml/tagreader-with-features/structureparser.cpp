/*
$Id$
*/

#include "structureparser.h"

#include <qstring.h>
#include <qtable.h>
 
StructureParser::StructureParser( QTable * t )
                : QXmlDefaultHandler() 
{
    table = t;
    table->setNumCols(4);

    table->horizontalHeader()->setLabel( 0, "Element" );
    table->horizontalHeader()->setLabel( 1, "Element Namespace" );
    table->horizontalHeader()->setLabel( 2, "Attribute" );
    table->horizontalHeader()->setLabel( 3, "Attribute Namespace" );
}

bool StructureParser::startDocument()
{
    generation = 0;
    row = -1;
    return TRUE;
}

bool StructureParser::startElement( const QString& namespaceURI, const QString& , 
                                    const QString& qName, 
                                    const QXmlAttributes& attributes)
{
    row++;
    table->setNumRows(row + 1);
    table->verticalHeader()->setLabel(row, QString::number( generation ) );
    table->setText( row, 0, qName); 
    table->setText( row, 1, namespaceURI); 

    if ( attributes.length() > 0 ){
	for ( int i = 0 ; i < attributes.length(); i++ ){
	    row++;
	    table->setNumRows( row + 1 );
	    table->verticalHeader()->setLabel(row, QString::number( generation ) );
	    table->setText( row, 2, attributes.qName(i) ); 
	    table->setText( row, 3, attributes.uri(i) ); 
	}      
    } 
    generation ++;

    return TRUE;
}

bool StructureParser::endElement( const QString&, const QString&, const QString& )
{
    generation --;
    return TRUE;
}

bool StructureParser::endDocument()
{
    for ( int x = 0; x < table->numCols(); x++ ){
	table->adjustColumn( x );
    }
    table->setReadOnly( TRUE );
    table->adjustSize();

    return TRUE;
}

