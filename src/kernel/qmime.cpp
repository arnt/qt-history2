/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmime.cpp#7 $
**
** Implementation of MIME support
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

#include "qmime.h"
#include "qmap.h"
#include "qstringlist.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qdragobject.h"

/*!
  \class QMimeSource qmime.h
  \brief An abstract piece of formatted data.

  \link dnd.html Drag-and-drop\endlink and
  \link QClipboard clipboard\endlink use this abstraction.

  \sa <a href="http://www.isi.edu/in-notes/iana/assignments/media-types/">
	  IANA list of MIME media types</a>
*/


/*!
  Provided to ensure subclasses destruct correctly.
*/
QMimeSource::~QMimeSource()
{
}

/*!
  \fn QByteArray QMimeSource::encodedData(const char*) const

  Returns the encoded payload of this object, in the specified
  MIME format.

  Subclasses must override this function.
*/



/*!
  Returns TRUE if the object can provide the data
  in format \a mimeType.  The default implementation
  iterates over format().
*/
bool QMimeSource::provides(const char* mimeType) const
{
    const char* fmt;
    for (int i=0; (fmt = format(i)); i++) {
	if ( !qstricmp(mimeType,fmt) )
	    return TRUE;
    }
    return FALSE;
}


/*!
  \fn const char * QMimeSource::format(int i) const

  Returns the \e ith format, or NULL.
*/



class QMimeSourceFactoryData {
public:
    QMimeSourceFactoryData() :
	last(0)
    {
    }

    ~QMimeSourceFactoryData()
    {
	QMap<QString, QMimeSource*>::Iterator it = stored.begin();
	while ( it != stored.end() ) {
	    delete *it;
	    ++it;
	}
	delete last;
    }

    QMap<QString, QMimeSource*> stored;
    QMap<QString, QString> extensions;
    QStringList path;
    QMimeSource* last;
};


/*!
  \class QMimeSourceFactory qmime.h
  \brief An extensible supply of MIME-typed data.

  A QMimeSourceFactory provides an abstract interface to a collection of
  information.  Each piece of information is represented by a QMimeSource
  object, which can be examined and converted to concrete data types by
  functions like QImageDrag::canDecode() and QImageDrag::decode().

  The base QMimeSourceFactory can be used in two ways: as an abstraction of
  a collection of files, or as specifically stored data.  For it to access
  files, call setFilePath() before accessing data.  For stored data, call
  setData() for each item (there are also convenience functions setText(),
  setImage(), and setPixmap() that simply call setData() with massaged
  parameters).

  The QRichText classes use QMimeSourceFactory.
*/


/*!
  Constructs a QMimeSourceFactory which has no filepath and no stored
  content.
*/
QMimeSourceFactory::QMimeSourceFactory() :
    d(new QMimeSourceFactoryData)
{
}

/*!
  Destructs the QMimeSourceFactory, deleting all stored content.
*/
QMimeSourceFactory::~QMimeSourceFactory()
{
    delete d;
}


/*!
  Returns a reference to the data associated with \a abs_name.  The return
  value only remains valid until a subsequent call to this function for
  the same object, and only if setData() is not called to modify the data.
*/
const QMimeSource* QMimeSourceFactory::data(const QString& abs_name) const
{
    if ( d->stored.contains(abs_name) )
	return d->stored[abs_name];

    QMimeSource* r = 0;
    QStringList::Iterator it;
    for ( it = d->path.begin(); !r && it != d->path.end(); ++it ) {
	QString filename = *it;
	if ( filename[(int)filename.length()-1] != '/' )
	    filename += '/';
	filename += abs_name;
	QFileInfo fi(filename);
	if ( fi.isReadable() ) {
	    QString e = fi.extension(FALSE);
	    if ( d->extensions.contains(e) ) {
		QString mimetype = d->extensions[e];
		QFile f(filename);
		if ( f.open(IO_ReadOnly) ) {
		    QByteArray ba(f.size());
		    f.readBlock(ba.data(), ba.size());
		    QStoredDrag* sr = new QStoredDrag( mimetype.latin1() );
		    sr->setEncodedData( ba );
		    r = sr;
		}
	    }
	}
    }
    delete d->last;
    d->last = r;
    return r;
}

/*!
  Sets a list of directories which will be searched when named data
  is requested.
*/
void QMimeSourceFactory::setFilePath( const QStringList& path )
{
    d->path = path;
}

/*!
  Sets the MIME-type to be associated with a filename extension.  This 
  determines the MIME-type for files found via a path set by setFilePath().
*/
void QMimeSourceFactory::setExtensionType( const QString& ext, const char* mimetype )
{
    d->extensions.replace(ext, mimetype);
}

/*!
  Converts the absolute or relative data item name \a abs_or_rel_name
  to an absolute name, interpretted within the context of the data item
  named \a context (this must be an absolute name).
*/
QString QMimeSourceFactory::makeAbsolute(const QString& abs_or_rel_name, const QString& context) const
{
    QFileInfo c(context);
    QFileInfo r(c.dir(TRUE), abs_or_rel_name);
    return r.absFilePath();
}

/*!
  A convenience function.
*/
const QMimeSource* QMimeSourceFactory::data(const QString& abs_or_rel_name, const QString& context) const
{
    return data(makeAbsolute(abs_or_rel_name,context));
}


/*!
  Sets \a text to be the data item associated with
  the absolute name \a abs_name.

  Equivalent to setData(abs_name, new QTextDrag(text)).
*/
void QMimeSourceFactory::setText( const QString& abs_name, const QString& text )
{
    setData(abs_name, new QTextDrag(text));
}

/*!
  Sets \a image to be the data item associated with
  the absolute name \a abs_name.

  Equivalent to setData(abs_name, new QImageDrag(image)).
*/
void QMimeSourceFactory::setImage( const QString& abs_name, const QImage& image )
{
    setData(abs_name, new QImageDrag(image));
}

/*!
  Sets \a pixmap to be the data item associated with
  the absolute name \a abs_name.
*/
void QMimeSourceFactory::setPixmap( const QString& abs_name, const QPixmap& pixmap )
{
    setData(abs_name, new QImageDrag(pixmap.convertToImage()));
}

/*!
  Sets \a data to be the data item associated with
  the absolute name \a abs_name. Note that the ownership of \a data is
  transferred to the factory - do not delete or access the pointer after
  passing it to this function.
*/
void QMimeSourceFactory::setData( const QString& abs_name, QMimeSource* data )
{
    if ( d->stored.contains(abs_name) )
	delete d->stored[abs_name];
    d->stored.replace(abs_name,data);
}

