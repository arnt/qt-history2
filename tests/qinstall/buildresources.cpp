/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qstring.h>
#include <qfile.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qdom.h>
#include <qdict.h>

#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>

struct Embed {
	unsigned long uncompressedSize;
	unsigned long compressedSize;
	QString name;
	QString cname;
};


void usage( const char *argv0 )
{
    qWarning( "Usage:\n\t%s [-basepath path] xmlSetupFile", argv0 );
    exit( 1 );
}


class EmbeddedDataFiles {
    public:
	EmbeddedDataFiles() {
	    list.setAutoDelete( TRUE );
	}
	void addFile( const QString& filename );
	void addComponents( const QDomElement& parentElement );
	void outputData( const QString& basePath );
    private:
	QPtrList<Embed> list;
};


void EmbeddedDataFiles::addFile( const QString& filename )
{
    // Only add if not in list
    for ( int i = 0; i < list.count(); i++ )
	if ( list.at(i)->name == filename )
	    return;
    // Add new entry
    Embed *e = new Embed;
    e->name = filename;
    list.append( e );
}


void EmbeddedDataFiles::addComponents( const QDomElement& parentElement )
{
    // resursively iterate over all the components in the
    // setup xml file and add all the referenced files that can be 
    // installed.
    QDomNode node = parentElement.firstChild();
    while ( !node.isNull() ) {
	if ( node.isElement() ) {
	    QDomElement e = node.toElement();
	    if ( node.nodeName() == "Component" ) {
		QStringList files( QStringList::split( ";", e.attribute( "files" ) ) );
		for ( int i = 0; i < files.count(); i++ )
		    addFile( files[i] );
		// recursively build the tree
		addComponents( node.toElement() );
	    }
	}
	node = node.nextSibling();
    }
}


void EmbeddedDataFiles::outputData( const QString& basePath )
{
    for ( int i = 0; i < list.count(); i++ ) {
	Embed *e = list.at(i);

	QFile f( basePath + "/" + e->name );
	if ( !f.open(IO_ReadOnly) ) {
	    qWarning( "Cannot open file %s, ignoring it", e->name.latin1() );
	    continue;
	}
	QByteArray a( f.size() );
	if ( f.readBlock(a.data(), f.size()) != (int)f.size() ) {
	    qWarning( "Cannot read file %s, ignoring it", e->name.latin1() );
	    f.close();
	    continue;
	}
	
	e->uncompressedSize = f.size();
	e->cname = e->name;
	int len = e->cname.length();
	if ( len > 0 && !isalpha( (char)e->cname[0].latin1() ) )
	    e->cname[0] = '_';
	for ( int i=1; i<len; i++ ) 
	    if ( !isalnum( (char)e->cname[i].latin1() ) )
		e->cname[i] = '_';
	
	//unsigned char *compressedData = (uchar*)malloc( (unsigned int)(a.size() * 1.1 + 64) ); 
	QByteArray c( (unsigned int)(a.size() * 1.1 + 64) ); 
	uchar *bytes = (uchar*)c.data();
	compress2( bytes, &(e->compressedSize), (unsigned char*)a.data(), a.size(), Z_BEST_COMPRESSION );        
	
	printf( "static const unsigned char %s_data[] = {", e->cname.latin1() );
	for ( int i = 0; i < e->compressedSize; i++ ) {
	    if ( (i % 14) == 0 )
		printf("\n    ");
	    printf( "0x%02x", bytes[i] );
	    if ( i < e->compressedSize - 1 )
		printf(",");
	}
	printf( "\n};\n\n" );

	f.close();
    }

    // Generate summery
    if ( list.count() > 0 ) {
	printf( "struct Embed {\n"
		   "    unsigned long       compressedSize;\n"
		   "    unsigned long       uncompressedSize;\n"
		   "    const unsigned char *data;\n"
		   "    const char          *name;\n"
		   "};\n"
		   "\n" 
		   "Embed embed_vec[] = {\n" );
	for ( Embed *e = list.first(); e; e = list.next() ) 
	    printf( "    { %i, %i, %s_data,\"%s\" },\n", e->compressedSize, e->uncompressedSize, e->cname.latin1(), e->name.latin1() );
	printf( "    { 0, 0, 0, 0 }\n};\n\n" );
    }
}


int main( int argc, char **argv )
{
    if ( argc < 2 )
	usage( argv[0] );

    EmbeddedDataFiles list;
    QString basePath;
    QString setupFile;

    // Embed data for all input files
    for ( int i = 1; i < argc; i++ ) {
	QString arg = argv[i];
	if ( arg == "-basepath" ) {
	    i++;
	    if ( i >= argc )
		usage( argv[0] );
	    basePath = argv[i];
	} else {
	    if ( !setupFile.isNull() )
		usage( argv[0] );
	    setupFile = arg;
	}
    }

    QFile f( basePath + "/" + setupFile );
    QDomDocument setupData;
    setupData.setContent( &f );

    list.addFile( setupFile );

#define addSetupFile( SECTION, ATTR ) \
    list.addFile( setupData.elementsByTagName( SECTION ).item(0).toElement().attribute( ATTR ) )

    addSetupFile( "InstallationWizard", "setupIcon" );
    addSetupFile( "InstallationWizard", "splashScreen" );
    addSetupFile( "Welcome", "image" );
    addSetupFile( "License", "image" );
    addSetupFile( "License", "file" );
    addSetupFile( "Destination", "image" );
    addSetupFile( "Configuration", "image" );
    addSetupFile( "Customize", "image" );
    addSetupFile( "Review", "image" );
    addSetupFile( "Install", "image" );

    list.addComponents( setupData.elementsByTagName("ComponentTree").item(0).toElement() );

    list.outputData( basePath );

    return 0;
}

