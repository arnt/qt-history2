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
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <qregexp.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

struct EmbedImage
{
    int width, height, depth;
    int numColors;
    QRgb* colorTable;
    QString name;
    QString cname;
    bool alpha;
};

QString convertToCIdentifier( const char *s )
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


static void embedData( QTextStream& out, const uchar* input, int nbytes )
{
    static const char hexdigits[] = "0123456789abcdef";
    QString s;
    for ( int i=0; i<nbytes; i++ ) {
	if ( (i%14) == 0 ) {
	    s += "\n    ";
	    out << (const char*)s;
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
	out << (const char*)s;
}

static void embedData( QTextStream& out, const QRgb* input, int n )
{
    QString s;
    for ( int i=0; i<n; i++ ) {
	if ( (i%14) == 0 ) {
	    s += "\n    ";
	    out << (const char*)s;
	    s.truncate( 0 );
	}
	QRgb v = input[i];
	s += "0x";
	s += QString::number(v,16);
	if ( i < n-1 )
	    s += ',';
    }
    if ( s.length() )
	out << (const char*)s;
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
	if ( e->depth == 32 ) {
	    out << s.sprintf( "static const QRgb %s_data[] = {",
			      (const char *)e->cname );
	    embedData( out, (QRgb*)img.bits(), e->width*e->height );
	} else {
	    if ( e->depth == 1 )
		img = img.convertBitOrder(QImage::BigEndian);
	    out << s.sprintf( "static const unsigned char %s_data[] = {",
			      (const char *)e->cname );
	    embedData( out, img.bits(), img.numBytes() );
	}
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
	    "static QDict<QImage> *" << cProject << "image_dict = 0;\n"
	    "static QImage& uic_findImage_" << cProject << "( const QString& name )\n"
	    "{\n"
	    "    if ( !" << cProject << "image_dict ) {\n"
	    "        " << cProject << "image_dict = new QDict<QImage>;\n"
	    "        " << cProject << "image_dict->setAutoDelete( TRUE );\n"
	    "    }\n"
	    "    QImage* img = " << cProject << "image_dict->find(name);\n"
	    "    if ( !img ) {\n"
	    "        for (int i=0; embed_image_vec[i].data; i++) {\n"
	    "	if ( QString::fromUtf8(embed_image_vec[i].name) == name ) {\n"
	    "	    img = new QImage((uchar*)embed_image_vec[i].data,\n"
	    "			embed_image_vec[i].width,\n"
	    "			embed_image_vec[i].height,\n"
	    "			embed_image_vec[i].depth,\n"
	    "			(QRgb*)embed_image_vec[i].colorTable,\n"
	    "			embed_image_vec[i].numColors,\n"
	    "			QImage::BigEndian\n"
	    "		);\n"
	    "	    if ( embed_image_vec[i].alpha )\n"
	    "	        img->setAlphaBuffer(TRUE);\n"
	    "	    break;\n"
	    "	}\n"
	    "        }\n"
	    "        if ( !img ) {\n"
	    "            static QImage dummy;\n"
	    "            return dummy;\n"
	    "        } else {\n"
	    "            " << cProject << "image_dict->insert( name, img );\n"
	    "        }\n"
	    "    }\n"
	    "    return *img;\n"
	    "}\n\n";

	out << "class DesignerMimeSourceFactory_" << cProject << " : public QMimeSourceFactory" << endl;
	out << "{" << endl;
	out << "public:" << endl;
	out << "    DesignerMimeSourceFactory_" << cProject << "() {}" << endl;

	out << "    const QMimeSource* data( const QString& abs_name ) const {" << endl;
	out << "\tQImage img;" << endl;
	out << "\tif ( !!abs_name )" << endl;
	out << "\t    img = uic_findImage_" << cProject << "( abs_name );" << endl;
	out << "\tif ( !img.isNull() ) {" << endl;
	out << "\t    QMimeSourceFactory::defaultFactory()->setImage( abs_name, img );" << endl;
	out << "\t    return QMimeSourceFactory::defaultFactory()->data( abs_name );" << endl;
	out << "\t} else {" << endl;
	out << "\t    QMimeSourceFactory::removeFactory( (QMimeSourceFactory*)this );" << endl;
	out << "\t    const QMimeSource *s = QMimeSourceFactory::defaultFactory()->data( abs_name );" << endl;
	out << "\t    QMimeSourceFactory::addFactory( (QMimeSourceFactory*)this );" << endl;
	out << "\t    return s;" << endl;
	out << "\t}" << endl;
	out << "\treturn 0;" << endl;
	out << "    };" << endl;
	out << "};" << endl;

	out << "static DesignerMimeSourceFactory_" << cProject <<"  *designerMimeSourceFactory = 0;" << endl;

	out << "static void qInitImages_" << cProject << "()" << endl;
	out << "{" << endl;
	out << "    if ( designerMimeSourceFactory )" << endl;
	out << "	return;" << endl;
	out << "    designerMimeSourceFactory = new DesignerMimeSourceFactory_" << cProject << ";" << endl;
	out << "    QMimeSourceFactory::defaultFactory()->addFactory( designerMimeSourceFactory );" << endl;
	out << "}" << endl;

	out << "static void qCleanupImages_" << cProject << "()" << endl;
	out << "{" << endl;
	out << "    delete " << cProject << "image_dict;" << endl;
	out << "    " << cProject << "image_dict = 0;" << endl;
	out << "    if ( !designerMimeSourceFactory )" << endl;
	out << "	return;" << endl;
	out << "    QMimeSourceFactory::defaultFactory()->removeFactory( designerMimeSourceFactory );" << endl;
	out << "    delete designerMimeSourceFactory;" << endl;
	out << "    designerMimeSourceFactory = 0;" << endl;
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
