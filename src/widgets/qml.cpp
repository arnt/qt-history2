/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qml.cpp#54 $
**
** Implementation of QML classes
**
** Created : 990101
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include "qml.h"
#include <qapplication.h>
#include <qlayout.h>
#include <qpainter.h>

#include <qstack.h>
#include <stdio.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qimage.h>





//************************************************************************


//************************************************************************


/*!
  \class QMLProvider qml.h
  \brief Provides a collection of QML documents

  Since QML documents link to each other, a view which displays QML will
  often need access to other documents so as to follow links, etc.
  A QMLProvider actually knows nothing about QML - it is just a repository
  of documents and images which QML documents happen to use.

  The default implementation gives no meaning to the names of documents
  it provides, beyond that provided by setPath().  However, the image()
  and document() functions are virtual, so a subclass could interpret
  the names as filenames, URLs, or whatever.

  \sa QMLView::setProvider()
*/

/*!
  Create a provider.  Like any QObject, the created object will be
  deleted when its parent destructs (if the child still exists then).
 */
QMLProvider::QMLProvider(  QObject *parent, const char *name )
    : QObject( parent, name )
{
    images.setAutoDelete( TRUE );
    documents.setAutoDelete( TRUE );
}

/*!
  Destroys the provider.
 */
QMLProvider::~QMLProvider()
{
}


static QMLProvider* defaultprovider = 0;

static void cleanup_provider()
{
    delete defaultprovider;
}

/*!
  Returns the application-wide default provider.
 */
QMLProvider* QMLProvider::defaultProvider()
{
    if (!defaultprovider) {
	defaultprovider = new QMLProvider;
	qAddPostRoutine(cleanup_provider);
    }
    return defaultprovider;
}

/*!
  Sets the default provider, destroying any previously set provider.
 */
void QMLProvider::setDefaultProvider( QMLProvider* provider)
{
    delete defaultprovider;
    defaultprovider = provider;
}


/*!
  Binds the given name to a pixmap. The pixmap can be accessed with
  image().
*/
void QMLProvider::setImage(const QString& name, const QPixmap& pm)
{
    images.insert(name, new QPixmap(pm));
}

/*!
  Returns the image corresponding to \a name.

  \sa setImage()
*/
QPixmap QMLProvider::image(const QString &name)
{
    QPixmap* p = images[name];
    if (p)
	return *p;
    else {
	return QPixmap( absoluteFilename( name ) );
    }
}

/*!
  Binds the \a name to the document contents \a doc.
  The document can then be accessed with document().
*/
void QMLProvider::setDocument(const QString &name, const QString& doc)
{
    documents.insert(name, new QString(doc));
}

/*!
  Returns the document contents corresponding to \a name.
  \sa setDocument()
*/
QString QMLProvider::document(const QString &name)
{
    QString* s = documents[name];
    if (s)
	return *s;
    {
	QFile f ( absoluteFilename( name ) );
	QString d;
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream t( &f );
	    d = t.read();
	    f.close();
	}
	return d;
    }
}


QString QMLProvider::absoluteFilename( const QString& name) const
{
    QString file;
    if ( name.left(6) == "file:/") {
	file = name.right( name.length()-5);
    }
    else if (name[0] == '/')
	file = name;
    else
	file = searchPath + name;
    return file;
}

/*!
  If an item cannot be found in the definitions set by setDocument()
  and setImage(), the default implementations of image() and document()
  will try to load it from the local filesystem in the single directory
  specified with setPath().

  \sa path()
 */
void QMLProvider::setPath( const QString &path )
{
    searchPath = path;
    if ( searchPath.left(6) == "file:/") {
	searchPath = searchPath.right( searchPath.length()-5);
    }
    if (!searchPath.isEmpty() && searchPath[(int)searchPath.length()-1]!='/')
	searchPath += '/';
}


/*!
 */
void QMLProvider::setReferenceDocument( const QString &doc )
{
    QString file = absoluteFilename( doc );
    int slash = file.findRev('/');
    if ( slash != -1 )
	searchPath = file.left( slash + 1);
}

/*!
  Returns the current search path.

  \sa setPath()
 */
QString QMLProvider::path() const
{
    return searchPath;
}



