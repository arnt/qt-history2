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

#include "pixmapcollection.h"
#include "project.h"
#include "mainwindow.h"
#include <qmime.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qimage.h>

PixmapCollection::PixmapCollection( Project *pro )
    : project( pro ), iface( new DesignerPixmapCollectionImpl( this ) )
{
    mimeSourceFactory = new QMimeSourceFactory();
}

PixmapCollection::~PixmapCollection()
{
    delete iface;
}

void PixmapCollection::addPixmap( const Pixmap &pix, bool force )
{
    if ( !force ) {
	for ( QValueList<Pixmap>::Iterator it = pixList.begin(); it != pixList.end(); ++it ) {
	    if ( (*it).name == pix.name )
		return;
	}
    }

    Pixmap pixmap = pix;
    pixmap.name = unifyName( pixmap.name );
    pixList.append( pixmap );
    mimeSourceFactory->setPixmap( pixmap.name, pixmap.pix );
    savePixmap( pixmap );
}

void PixmapCollection::removePixmap( const QString &name )
{
    removePixmapFile( name );
    for ( QValueList<Pixmap>::Iterator it = pixList.begin(); it != pixList.end(); ++it ) {
	if ( (*it).name == name ) {
	    pixList.remove( it );
	    break;
	}
    }
}

QValueList<PixmapCollection::Pixmap> PixmapCollection::pixmaps() const
{
    return pixList;
}

QString PixmapCollection::unifyName( const QString &n )
{
    QString name = n;
    bool restart = FALSE;
    int added = 1;

    for ( QValueList<Pixmap>::Iterator it = pixList.begin(); it != pixList.end(); ++it ) {
	if ( restart )
	    it = pixList.begin();
	restart = FALSE;
	if ( name == (*it).name ) {
	    name = n;
	    name += "_" + QString::number( added );
	    ++added;
	    restart = TRUE;
	}
    }
	
    return name;
}

void PixmapCollection::setActive( bool b )
{
    if ( b )
	QMimeSourceFactory::defaultFactory()->addFactory( mimeSourceFactory );
    else
	QMimeSourceFactory::defaultFactory()->removeFactory( mimeSourceFactory );
}

QPixmap PixmapCollection::pixmap( const QString &name )
{
    for ( QValueList<Pixmap>::Iterator it = pixList.begin(); it != pixList.end(); ++it ) {
	if ( (*it).name == name )
	    return (*it).pix;
    }
    return QPixmap();
}

void PixmapCollection::savePixmap( const Pixmap &pix )
{
    mkdir();
    QString f = project->fileName();
    pix.pix.save( QFileInfo( f ).dirPath( TRUE ) + "/images/" + pix.name, "PNG" );
}

void PixmapCollection::mkdir()
{
    QString f = project->fileName();
    QDir d( QFileInfo( f ).dirPath( TRUE ) );
    d.mkdir( "images" );
}

void PixmapCollection::removePixmapFile( const QString &name )
{
    QString f = project->fileName();
    QDir d( QFileInfo( f ).dirPath( TRUE ) + "/images" );
    d.remove( name );
}

void PixmapCollection::load()
{
    QString f = project->fileName();
    QDir d( QFileInfo( f ).dirPath( TRUE ) + "/images" );
    QStringList l = d.entryList( QDir::Files );
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
	Pixmap pix;
	pix.name = *it;
	pix.pix = QPixmap( d.path() + "/" + *it, "PNG" );
	pixList.append( pix );
	mimeSourceFactory->setPixmap( pix.name, pix.pix );
    }
}

struct EmbedImage
{
    int width, height, depth;
    int numColors;
    QRgb* colorTable;
    QString name;
    QString cname;
    bool alpha;
};

static void embedData( const uchar* input, int nbytes, QFile *output )
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

static void embedData( const QRgb* input, int n, QFile *output )
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

static bool block_create = FALSE;

void PixmapCollection::createCppFile()
{
    MainWindow::self->updateFormList();
    if ( project->projectName() == "<No Project>" )
	return;
    if ( block_create )
	return;
    block_create = TRUE;
    if ( project->imageFile().isEmpty() )
	project->setImageFile( "images.cpp" );
    block_create = FALSE;

    QFile f( project->makeAbsolute( project->imageFile() ) );
    if ( !f.open( IO_WriteOnly ) )
	return;
    QTextStream out( &f );

    out << "#ifndef DESIGNER_IMAGES_H" << endl;
    out << "#define DESIGNER_IMAGES_H" << endl;
    out << "#include <qimage.h>" << endl;
    out << "#include <qdict.h>" << endl;
    out << "#include <qmime.h>" << endl;
    out << "#include <qdragobject.h>" << endl;
    out << "#include <qpixmap.h>" << endl << endl;


    QList<EmbedImage> list_image;
    for ( QValueList<Pixmap>::Iterator it = pixList.begin(); it != pixList.end(); ++it ) {
	QImage img = (*it).pix.convertToImage();
	EmbedImage *e = new EmbedImage;
	e->width = img.width();
	e->height = img.height();
	e->depth = img.depth();
	e->numColors = img.numColors();
	e->colorTable = new QRgb[e->numColors];
	e->alpha = img.hasAlphaBuffer();
	memcpy(e->colorTable, img.colorTable(), e->numColors*sizeof(QRgb));
	e->name = (*it).name;
	e->cname = e->name;
	list_image.append( e );
	QString s;
	if ( e->depth == 32 ) {
	    out << s.sprintf( "static const QRgb %s_data[] = {",
			      (const char *)e->cname );
	    embedData( (QRgb*)img.bits(), e->width*e->height, &f );
	} else {
	    if ( e->depth == 1 )
		img = img.convertBitOrder(QImage::BigEndian);
	    out << s.sprintf( "static const unsigned char %s_data[] = {",
			      (const char *)e->cname );
	    embedData( img.bits(), img.numBytes(), &f );
	}
	out << "\n};\n\n";
	if ( e->numColors ) {
	    out << s.sprintf( "static const QRgb %s_ctable[] = {",
			      (const char *)e->cname );
	    embedData( e->colorTable, e->numColors, &f );
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
	    "static QImage& uic_findImage_" + project->fixedProjectName() + "( const char *name )\n"
	    "{\n"
	    "    static QDict<QImage> dict;\n"
	    "    QImage* img = dict.find(name);\n"
	    "    if ( !img ) {\n"
	    "        for (int i=0; embed_image_vec[i].data; i++) {\n"
	    "    	if ( 0==strcmp(embed_image_vec[i].name, name) ) {\n"
	    "    	    img = new QImage((uchar*)embed_image_vec[i].data,\n"
	    "    			embed_image_vec[i].width,\n"
	    "    			embed_image_vec[i].height,\n"
	    "    			embed_image_vec[i].depth,\n"
	    "    			(QRgb*)embed_image_vec[i].colorTable,\n"
	    "    			embed_image_vec[i].numColors,\n"
	    "    			QImage::BigEndian\n"
	    "    		);\n"
	    "    	    if ( embed_image_vec[i].alpha )\n"
	    "    	        img->setAlphaBuffer(TRUE);\n"
	    "    	    break;\n"
	    "    	}\n"
	    "        }\n"
	    "        if ( !img ) {\n"
	    "            static QImage dummy;\n"
	    "            return dummy;\n"
	    "        }\n"
	    "    }\n"
	    "    return *img;\n"
	    "}\n\n";
	
	out << "class DesignerMimeSourceFactoty : public QMimeSourceFactory" << endl;
	out << "{" << endl;
	out << "public:" << endl;
	out << "    DesignerMimeSourceFactoty() {}" << endl;

	out << "    const QMimeSource* data( const QString& abs_name ) const {" << endl;
	out << "	QImage img = uic_findImage_" << project->fixedProjectName() << "( abs_name );" << endl;
	out << "	QPixmap pix;" << endl;
	out << "	pix.convertFromImage( img );" << endl;
	out << "	QMimeSourceFactory::defaultFactory()->setPixmap( abs_name, pix );" << endl;
	out << "	return QMimeSourceFactory::defaultFactory()->data( abs_name );" << endl;
	out << "    };" << endl;
	out << "};" << endl;

	out << "static DesignerMimeSourceFactoty *designerMimeSourceFactory = 0;" << endl;

	out << "static void qInitImages()" << endl;
	out << "{" << endl;
	out << "    if ( designerMimeSourceFactory )" << endl;
	out << "	return;" << endl;
	out << "    designerMimeSourceFactory = new DesignerMimeSourceFactoty;" << endl;
	out << "    QMimeSourceFactory::defaultFactory()->addFactory( designerMimeSourceFactory );" << endl;
	out << "}" << endl;

	out << "static void qCleanupImages()" << endl;
	out << "{" << endl;
	out << "    if ( !designerMimeSourceFactory )" << endl;
	out << "	return;" << endl;
	out << "    QMimeSourceFactory::defaultFactory()->removeFactory( designerMimeSourceFactory );" << endl;
	out << "    delete designerMimeSourceFactory;" << endl;
	out << "    designerMimeSourceFactory = 0;" << endl;
	out << "}" << endl;

	out << "class StaticInitImages" << endl;
	out << "{" << endl;
	out << "public:" << endl;
	out << "    StaticInitImages() { qInitImages(); }" << endl;
	out << "    ~StaticInitImages() { qCleanupImages(); }" << endl;
	out << "};" << endl;

	out << "static StaticInitImages staticImages;" << endl;
    }

    out << "#endif" << endl;
    f.close();
}

DesignerPixmapCollection *PixmapCollection::iFace()
{
    return iface;
}
