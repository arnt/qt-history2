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
#include <qmime.h>
#include <qdir.h>
#include <qfileinfo.h>

PixmapCollection::PixmapCollection( Project *pro )
    : project( pro )
{
    mimeSourceFactory = new QMimeSourceFactory();
}

void PixmapCollection::addPixmap( const Pixmap &pix )
{
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

void PixmapCollection::setActive()
{
    QMimeSourceFactory::takeDefaultFactory();
    QMimeSourceFactory::setDefaultFactory( mimeSourceFactory );
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
    }
}
