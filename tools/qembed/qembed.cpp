/****************************************************************************
** $Id$
**
** Utility program for embedding binary data into a C/C++ source code.
** It reads a binary file and generates a C array with the binary data.
** The C code is written to standard output.
**
** Author  : Haavard Nord
** Created : 951017
**
** Copyright (C) 1995-2002 Trolltech AS.  All rights reserved.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that this copyright notice appears in all copies.
** No representations are made about the suitability of this software for any
** purpose. It is provided "as is" without express or implied warranty.
**
*****************************************************************************/

#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qptrlist.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <qimage.h>
#include <qdict.h>
#include <ctype.h>
#include <stdlib.h>

void    embedData( const QByteArray &input, QFile *output );
void    embedData( const uchar* input, int size, QFile *output );
void    embedData( const QRgb* input, int size, QFile *output );
QString convertFileNameToCIdentifier( const char * );

char header[] = "/* Generated by qembed */\n";

struct Embed {
    uint size;
    QString name;
    QString cname;
};

struct EmbedImage {
    int width;
    int height;
    int depth;
    int numColors;
    QRgb* colorTable;
    QString name;
    QString cname;
    bool alpha;
};

int main( int argc, char **argv )
{
    if ( argc < 2 ) {
	qWarning( "Usage:\n\t%s [--images] files", argv[0] );
	return 1;
    }

    QFile output;
    bool output_hdr = FALSE;
    bool images = FALSE;
    output.open( IO_WriteOnly, stdout );
    QTextStream out( &output );


    QPtrList<EmbedImage> list_image;
    QPtrList<Embed> list;
    list.setAutoDelete( TRUE );
    list_image.setAutoDelete( TRUE );

    long l = rand();
    out << "#ifndef _QEMBED_" << l << endl;
    out << "#define _QEMBED_" << l << endl;

    for ( int i = 1; i < argc; i++ ) {
	QString arg = argv[i];
	if ( arg == "--images" ) {
	    if ( !images ) {
		out << "#include <qimage.h>\n";
		out << "#include <qdict.h>\n";
		images = TRUE;
	    }
	} else {
	    QFile f( argv[i] );
	    if ( !f.open(IO_ReadOnly) ) {
		qWarning( "Cannot open file %s, ignoring it", argv[i] );
		continue;
	    }
	    QByteArray a( f.size() );
	    if ( f.readBlock(a.data(), f.size()) != (int)f.size() ) {
		qWarning( "Cannot read file %s, ignoring it", argv[i] );
		f.close();
		continue;
	    }
	    if ( images ) {
		QImage img;
		if ( !img.loadFromData(a) ) {
		    qWarning( "Cannot read image from file %s, ignoring it", argv[i] );
		    f.close();
		    continue;
		}
		EmbedImage *e = new EmbedImage;
		e->width = img.width();
		e->height = img.height();
		e->depth = img.depth();
		e->numColors = img.numColors();
		e->colorTable = new QRgb[e->numColors];
		e->alpha = img.hasAlphaBuffer();
		memcpy(e->colorTable, img.colorTable(), e->numColors*sizeof(QRgb));
		QFileInfo fi(argv[i]);
		e->name = fi.baseName();
		e->cname = convertFileNameToCIdentifier( e->name.latin1() );
		list_image.append( e );
		QString s;
		if ( e->depth == 32 ) {
		    out << s.sprintf( "static const QRgb %s_data[] = {",
				   (const char *)e->cname );
		    embedData( (QRgb*)img.bits(), e->width*e->height, &output );
		} else {
		    if ( e->depth == 1 )
			img = img.convertBitOrder(QImage::BigEndian);
		    out << s.sprintf( "static const unsigned char %s_data[] = {",
				   (const char *)e->cname );
		    embedData( img.bits(), img.numBytes(), &output );
		}
		out << "\n};\n\n";
		if ( e->numColors ) {
		    out << s.sprintf( "static const QRgb %s_ctable[] = {",
				   (const char *)e->cname );
		    embedData( e->colorTable, e->numColors, &output );
		    out << "\n};\n\n";
		}
	    } else {
		Embed *e = new Embed;
		e->size = f.size();
		e->name = argv[i];
		e->cname = convertFileNameToCIdentifier( argv[i] );
		list.append( e );
		QString s;
		out << s.sprintf( "static const unsigned int  %s_len = %d;\n",
			       (const char *)e->cname, e->size );
		out << s.sprintf( "static const unsigned char %s_data[] = {",
			       (const char *)e->cname );
		embedData( a, &output );
		out << "\n};\n\n";
	    }

	    if ( !output_hdr ) {
		output_hdr = TRUE;
		out << header;
	    }

	    f.close();
	}
    }

    if ( list.count() > 0 ) {
	out << "#include <qcstring.h>\n";
	if ( !images )
	    out << "#include <qdict.h>\n";

	out << "static struct Embed {\n"
	       "    unsigned int size;\n"
	       "    const unsigned char *data;\n"
	       "    const char *name;\n"
	       "} embed_vec[] = {\n";
	Embed *e = list.first();
	while ( e ) {
	    out << "    { " << e->size << ", " << e->cname << "_data, "
		 << "\"" << e->name << "\" },\n";
	    e = list.next();
	}
	out << "    { 0, 0, 0 }\n};\n";

	out << "\n"
"static const QByteArray& qembed_findData( const char* name )\n"
"{\n"
"    static QDict<QByteArray> dict;\n"
"    QByteArray* ba = dict.find( name );\n"
"    if ( !ba ) {\n"
"	for ( int i = 0; embed_vec[i].data; i++ ) {\n"
"	    if ( strcmp(embed_vec[i].name, name) == 0 ) {\n"
"		ba = new QByteArray;\n"
"		ba->setRawData( (char*)embed_vec[i].data,\n"
"				embed_vec[i].size );\n"
"		dict.insert( name, ba );\n"
"		break;\n"
"	    }\n"
"	}\n"
"	if ( !ba ) {\n"
"	    static QByteArray dummy;\n"
"	    return dummy;\n"
"	}\n"
"    }\n"
"    return *ba;\n"
"}\n\n";
    }

    if ( list_image.count() > 0 ) {
	out << "static struct EmbedImage {\n"
	       "    int width, height, depth;\n"
	       "    const unsigned char *data;\n"
	       "    int numColors;\n"
	       "    const QRgb *colorTable;\n"
	       "    bool alpha;\n"
	       "    const char *name;\n"
	       "} embed_image_vec[] = {\n";
	EmbedImage *e = list_image.first();
	while ( e ) {
	    out << "    { "
		<< e->width << ", "
		<< e->height << ", "
		<< e->depth << ", "
		<< "(const unsigned char*)" << e->cname << "_data, "
		<< e->numColors << ", ";
	    if ( e->numColors )
		out << e->cname << "_ctable, ";
	    else
		out << "0, ";
	    if ( e->alpha )
		out << "TRUE, ";
	    else
		out << "FALSE, ";
	    out << "\"" << e->name << "\" },\n";
	    e = list_image.next();
	}
	out << "    { 0, 0, 0, 0, 0, 0, 0, 0 }\n};\n";

	out << "\n"
"static const QImage& qembed_findImage( const QString& name )\n"
"{\n"
"    static QDict<QImage> dict;\n"
"    QImage* img = dict.find( name );\n"
"    if ( !img ) {\n"
"	for ( int i = 0; embed_image_vec[i].data; i++ ) {\n"
"	    if ( strcmp(embed_image_vec[i].name, name.latin1()) == 0 ) {\n"
"		img = new QImage((uchar*)embed_image_vec[i].data,\n"
"			    embed_image_vec[i].width,\n"
"			    embed_image_vec[i].height,\n"
"			    embed_image_vec[i].depth,\n"
"			    (QRgb*)embed_image_vec[i].colorTable,\n"
"			    embed_image_vec[i].numColors,\n"
"			    QImage::BigEndian );\n"
"		if ( embed_image_vec[i].alpha )\n"
"		    img->setAlphaBuffer( TRUE );\n"
"		dict.insert( name, img );\n"
"		break;\n"
"	    }\n"
"	}\n"
"	if ( !img ) {\n"
"	    static QImage dummy;\n"
"	    return dummy;\n"
"	}\n"
"    }\n"
"    return *img;\n"
"}\n\n";
    }

    out << "#endif" << endl;

    return 0;
}


QString convertFileNameToCIdentifier( const char *s )
{
    QString r = s;
    int len = r.length();
    if ( len > 0 && !isalpha( (char)r[0].latin1() ) )
	r[0] = '_';
    for ( int i=1; i<len; i++ ) {
	if ( !isalnum( (char)r[i].latin1() ) )
	    r[i] = '_';
    }
    return r;
}


void embedData( const QByteArray &input, QFile *output )
{
    embedData((uchar*)input.data(), input.size(), output);
}

void embedData( const uchar* input, int nbytes, QFile *output )
{
    static char hexdigits[] = "0123456789abcdef";
    QString s;
    for ( int i=0; i<nbytes; i++ ) {
	if ( (i%14) == 0 ) {
	    s += "\n    ";
	    output->writeBlock( (const char*)s, s.length() );
	    s.truncate( 0 );
	}
	uint v = input[i];
	s += "0x";
	s += hexdigits[(v >> 4) & 15];
	s += hexdigits[v & 15];
	if ( i < nbytes-1 )
	    s += ',';
    }
    if ( s.length() )
	output->writeBlock( (const char*)s, s.length() );
}

void embedData( const QRgb* input, int n, QFile *output )
{
    QString s;
    for ( int i=0; i<n; i++ ) {
	if ( (i%14) == 0 ) {
	    s += "\n    ";
	    output->writeBlock( (const char*)s, s.length() );
	    s.truncate( 0 );
	}
	QRgb v = input[i];
	s += "0x";
	s += QString::number(v,16);
	if ( i < n-1 )
	    s += ',';
    }
    if ( s.length() )
	output->writeBlock( (const char*)s, s.length() );
}
