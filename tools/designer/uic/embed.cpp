/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
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

#include "uic.h"
#include <qfile.h>
#include <qimage.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// on embedded, we do not compress image data. Rationale: by mapping
// the ready-only data directly into memory we are both faster and
// more memory efficient
#if defined(Q_WS_QWS) && !defined(QT_NO_IMAGE_COLLECTION_COMPRESSION)
#define QT_NO_IMAGE_COLLECTION_COMPRESSION
#endif

struct EmbedImage
{
    int width, height, depth;
    int numColors;
    QRgb* colorTable;
    QString name;
    QString cname;
    bool alpha;
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
    ulong compressed;
#endif
};

static QString convertToCIdentifier( const char *s )
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


static ulong embedData( QTextStream& out, const uchar* input, int nbytes )
{
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
    QByteArray bazip( qCompress( input, nbytes ) );
    ulong len = bazip.size();
#else
    ulong len = nbytes;
#endif
    static const char hexdigits[] = "0123456789abcdef";
    QString s;
    for ( int i=0; i<(int)len; i++ ) {
	if ( (i%14) == 0 ) {
	    s += "\n    ";
	    out << (const char*)s;
	    s.truncate( 0 );
	}
	uint v = (uchar)
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
		 bazip
#else
		 input
#endif
		 [i];
	s += "0x";
	s += hexdigits[(v >> 4) & 15];
	s += hexdigits[v & 15];
	if ( i < (int)len-1 )
	    s += ',';
    }
    if ( s.length() )
	out << (const char*)s;
    return len;
}

static void embedData( QTextStream& out, const QRgb* input, int n )
{
    out << hex;
    const QRgb *v = input;
    for ( int i=0; i<n; i++ ) {
	if ( (i%14) == 0  )
	    out << endl << "    ";
	out << "0x";
	out << hex << *v++;
	if ( i < n-1 )
	    out << ',';
    }
    out << dec; // back to decimal mode
}

void Uic::embed( QTextStream& out, const char* project, const QStringList& images )
{

    QString cProject = convertToCIdentifier( project );

    QStringList::ConstIterator it;
    out << "/****************************************************************************" << endl;
    out << "** Image collection for project '" << project << "'." << endl;
    out << "**" << endl;
    out << "** Generated from reading image files: " << endl;
    for ( it = images.begin(); it != images.end(); ++it )
	out << "**      " << *it << endl;
    out << "**" << endl;
    out << "** Created: " << QDateTime::currentDateTime().toString() << endl;
    out << "**      by:  The User Interface Compiler (uic)" << endl;
    out << "**" << endl;
    out << "** WARNING! All changes made in this file will be lost!" << endl;
    out << "****************************************************************************/" << endl << endl;

    out << "#include <qimage.h>" << endl;
    out << "#include <qdict.h>" << endl;
    out << "#include <qmime.h>" << endl;
    out << "#include <qdragobject.h>" << endl;

    QPtrList<EmbedImage> list_image;
    int image_count = 0;
    for ( it = images.begin(); it != images.end(); ++it ) {
	QImage img;
	if ( !img.load( *it  ) ) {
	    fprintf( stderr, "uic: cannot load image file %s\n", (*it).latin1() );
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
	QFileInfo fi( *it );
	e->name = fi.fileName();
	e->cname = QString("image_%1").arg( image_count++);
	list_image.append( e );
	out << "// " << *it << endl;
	QString s;
	if ( e->depth == 1 )
	    img = img.convertBitOrder(QImage::BigEndian);
	out << s.sprintf( "static const unsigned char %s_data[] = {",
			  (const char *)e->cname );
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
	e->compressed =
#endif
	    embedData( out, img.bits(), img.numBytes() );
	out << "\n};\n\n";
	if ( e->numColors ) {
	    out << s.sprintf( "static const QRgb %s_ctable[] = {",
			      (const char *)e->cname );
	    embedData( out, e->colorTable, e->numColors );
	    out << "\n};\n\n";
	}
    }

    if ( !list_image.isEmpty() ) {
	out << "static struct EmbedImage {\n"
	    "    int width, height, depth;\n"
	    "    const unsigned char *data;\n"
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
	    "    ulong compressed;\n"
#endif
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
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
		<< e->compressed << ", "
#endif
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
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
	out << "    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }\n};\n";
#else
	out << "    { 0, 0, 0, 0, 0, 0, 0, 0 }\n};\n";
#endif

	out << "\n"
	    "static QImage uic_findImage( const QString& name )\n"
	    "{\n"
	    "    for (int i=0; embed_image_vec[i].data; i++) {\n"
	    "	if ( QString::fromUtf8(embed_image_vec[i].name) == name ) {\n"
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
	    "	    QByteArray baunzip;\n"
	    "	    baunzip = qUncompress( embed_image_vec[i].data, \n"
	    "		embed_image_vec[i].compressed );\n"
	    "	    QImage img((uchar*)baunzip.data(),\n"
	    "			embed_image_vec[i].width,\n"
	    "			embed_image_vec[i].height,\n"
	    "			embed_image_vec[i].depth,\n"
	    "			(QRgb*)embed_image_vec[i].colorTable,\n"
	    "			embed_image_vec[i].numColors,\n"
	    "			QImage::BigEndian\n"
	    "		);\n"
	    "	    img = img.copy();\n"
#else
	    "	    QImage img((uchar*)embed_image_vec[i].data,\n"
	    "			embed_image_vec[i].width,\n"
	    "			embed_image_vec[i].height,\n"
	    "			embed_image_vec[i].depth,\n"
	    "			(QRgb*)embed_image_vec[i].colorTable,\n"
	    "			embed_image_vec[i].numColors,\n"
	    "			QImage::BigEndian\n"
	    "		);\n"
#endif
	    "	    if ( embed_image_vec[i].alpha )\n"
	    "		img.setAlphaBuffer(TRUE);\n"
	    "	    return img;\n"
	    "        }\n"
	    "    }\n"
	    "    return QImage();\n"
	    "}\n\n";

	out << "class MimeSourceFactory_" << cProject << " : public QMimeSourceFactory" << endl;
	out << "{" << endl;
	out << "public:" << endl;
	out << "    MimeSourceFactory_" << cProject << "() {}" << endl;
	out << "    ~MimeSourceFactory_" << cProject << "() {}" << endl;
	out << "    const QMimeSource* data( const QString& abs_name ) const {" << endl;
	out << "\tconst QMimeSource* d = QMimeSourceFactory::data( abs_name );" << endl;
	out << "\tif ( d || abs_name.isNull() ) return d;" << endl;
	out << "\tQImage img = uic_findImage( abs_name );" << endl;
	out << "\tif ( !img.isNull() )" << endl;
	out << "\t    ((QMimeSourceFactory*)this)->setImage( abs_name, img );" << endl;
	out << "\treturn QMimeSourceFactory::data( abs_name );" << endl;
	out << "    };" << endl;
	out << "};" << endl;

	out << "static QMimeSourceFactory* factory = 0;" << endl;

	out << "void qInitImages_" << cProject << "()" << endl;
	out << "{" << endl;
	out << "    if ( !factory ) {" << endl;
	out << "\tfactory = new MimeSourceFactory_" << cProject << ";" << endl;
	out << "\tQMimeSourceFactory::defaultFactory()->addFactory( factory );" << endl;
	out << "    }" <<  endl;
	out << "}" << endl;

	out << "void qCleanupImages_" << cProject << "()" << endl;
	out << "{" << endl;
	out << "    if ( factory ) {" << endl;
	out << "\tQMimeSourceFactory::defaultFactory()->removeFactory( factory );" << endl;
	out << "\tdelete factory;" << endl;
	out << "\tfactory = 0;" << endl;
	out << "    }" <<  endl;
	out << "}" << endl;

	out << "class StaticInitImages_" << cProject << endl;
	out << "{" << endl;
	out << "public:" << endl;
	out << "    StaticInitImages_" << cProject << "() { qInitImages_" << cProject << "(); }" << endl;
	out << "    ~StaticInitImages_" << cProject << "() { qCleanupImages_" << cProject << "(); }" << endl;
	out << "};" << endl;

	out << "static StaticInitImages_" << cProject << " staticImages;" << endl;
    }
}
