/*
$Id$
*/

#include "structureparser.h"
#include <qapplication.h>
#include <qfile.h>
#include <qxml.h>
#include <qtable.h>
#include <qvbox.h>
#include <qmainwindow.h>
#include <qlabel.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );      

    QFile xmlFile( argc == 2 ? argv[1] : "fnord.xml" );
    QXmlInputSource source( &xmlFile );

    QXmlSimpleReader reader;

    QVBox * container = new QVBox();

    QTable * nameSpace = new QTable( container, "table_namespace" );    
    StructureParser * handlerNamespace = new StructureParser( nameSpace );
    reader.setContentHandler( handlerNamespace );
    reader.parse( source );

    QLabel * namespaceLabel = new QLabel( 
                             "http://xml.org/sax/features/namespaces\t\tTRUE\n"
                             "http://xml.org/sax/features/namespace-prefixes\tFALSE\n"
                             "(default)\n",
                             container );

    QTable * namespacePrefix = new QTable( container, "table_namespace_prefix" );    
    StructureParser * handlerNamespacePrefix = new StructureParser( namespacePrefix );
    reader.setContentHandler( handlerNamespacePrefix );
    reader.setFeature( "http://xml.org/sax/features/namespaces", TRUE );
    reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", TRUE );
    reader.parse( source );

    QLabel * namespacePrefixLabel = new QLabel( 
                             "http://xml.org/sax/features/namespaces\t\tTRUE\n"
                             "http://xml.org/sax/features/namespace-prefixes\tTRUE\n",
                             container );


    QTable * prefix = new QTable( container, "table_prefix");    
    StructureParser * handlerPrefix = new StructureParser( prefix );
    reader.setContentHandler( handlerPrefix );
    reader.setFeature( "http://xml.org/sax/features/namespaces", FALSE );
    reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", TRUE );
    reader.parse( source );

    QLabel * prefixLabel = new QLabel( 
                             "http://xml.org/sax/features/namespaces\t\tFALSE\n"
                             "http://xml.org/sax/features/namespace-prefixes\tTRUE\n",
                             container );


    app.setMainWidget( container );
    container->show();
    return app.exec();      
}
