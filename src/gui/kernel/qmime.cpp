/****************************************************************************
**
** Implementation of MIME support.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmime.h"

#ifndef QT_NO_MIME

#include "qmap.h"
#include "qstringlist.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qdragobject.h"
#include "qpixmap.h"
#include "qcleanuphandler.h"

/*!
    \class QMimeSource
    \brief The QMimeSource class is an abstraction of objects which provide formatted data of a certain MIME type.

    \ingroup io
    \ingroup draganddrop
    \ingroup misc

    \link dnd.html Drag-and-drop\endlink and
    \link QClipboard clipboard\endlink use this abstraction.

    \sa \link http://www.isi.edu/in-notes/iana/assignments/media-types/
            IANA list of MIME media types\endlink
*/

static int qt_mime_serial_number = 0;

/*!
    Constructs a mime source and assigns a globally unique serial
    number to it.

    \sa serialNumber()
*/

QMimeSource::QMimeSource()
{
    ser_no = qt_mime_serial_number++;
    cacheType = NoCache;
}

/*!
    \fn int QMimeSource::serialNumber() const

    Returns the mime source's globally unique serial number.
*/


void QMimeSource::clearCache()
{
    if (cacheType == Text) {
        delete cache.txt.str;
        delete cache.txt.subtype;
        cache.txt.str = 0;
        cache.txt.subtype = 0;
    } else if (cacheType == Graphics) {
        delete cache.gfx.img;
        delete cache.gfx.pix;
        cache.gfx.img = 0;
        cache.gfx.pix = 0;
    }
    cacheType = NoCache;
}

/*!
    Provided to ensure that subclasses destroy themselves correctly.
*/
QMimeSource::~QMimeSource()
{
    clearCache();
}

/*!
    \fn QByteArray QMimeSource::encodedData(const char *format) const

    Returns the encoded data of this object in the specified MIME
    \a format.

    Subclasses must reimplement this function.
*/



/*!
    Returns true if the object can provide the data in format \a
    mimeType; otherwise returns false.

    If you inherit from QMimeSource, for consistency reasons it is
    better to implement the more abstract canDecode() functions such
    as QTextDrag::canDecode() and QImageDrag::canDecode().
*/
bool QMimeSource::provides(const char* mimeType) const
{
    const char* fmt;
    for (int i=0; (fmt = format(i)); i++) {
        if (!qstricmp(mimeType,fmt))
            return true;
    }
    return false;
}


/*!
    \fn const char * QMimeSource::format(int i) const

    Returns the \a{i}-th supported MIME format, or 0.
*/


#endif // QT_NO_MIME
