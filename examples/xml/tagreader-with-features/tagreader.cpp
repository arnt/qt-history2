/*
$Id$
*/

#include "structureparser.h"
#include <qapplication.h>
#include <qfile.h>
#include <qxml.h>
#include <qlistview.h>
#include <qgrid.h>
#include <qmainwindow.h>
#include <qlabel.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );      

    QFile xmlFile( argc == 2 ? argv[1] : "fnord.xml" );
    QXmlInputSource source( &xmlFile );

    QXmlSimpleReader reader;

    QGrid * container = new QGrid( 3 );

    QListView * nameSpace = new QListView( container, "table_namespace" );    
    StructureParser * handler = new StructureParser( nameSpace );
    reader.setContentHandler( handler );
    reader.parse( source );

    QListView * namespacePrefix = new QListView( container, 
                                                 "table_namespace_prefix" );    
    handler->setListView( namespacePrefix );
    reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", 
                       TRUE );
    reader.parse( source );

    QListView * prefix = new QListView( container, "table_prefix");    
    handler->setListView( prefix );
    reader.setFeature( "http://xml.org/sax/features/namespaces", FALSE );
    reader.parse( source );

    QLabel * namespaceLabel = new QLabel( 
             "Default:\n"
             "http://xml.org/sax/features/namespaces: TRUE\n"
             "http://xml.org/sax/features/namespace-prefixes: FALSE\n",
             container );

    QLabel * namespacePrefixLabel = new QLabel( 
             "\n"
             "http://xml.org/sax/features/namespaces: TRUE\n"
             "http://xml.org/sax/features/namespace-prefixes: TRUE\n",
             container );

    QLabel * prefixLabel = new QLabel( 
             "\n"
             "http://xml.org/sax/features/namespaces: FALSE\n"
             "http://xml.org/sax/features/namespace-prefixes: TRUE\n",
             container );


    app.setMainWidget( container );
    container->show();
    return app.exec();      
}
