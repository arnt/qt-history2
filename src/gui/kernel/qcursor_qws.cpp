/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qcursor.h>

#ifndef QT_NO_CURSOR
#include <private/qcursor_p.h>
#include <qbitmap.h>
#include <qwsdisplay_qws.h>

static int nextCursorId = Qt::BitmapCursor;

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

QCursorData::QCursorData(Qt::CursorShape s)
    : cshape(s), bm(0), bmm(0), hx(0), hy(0), id(s)
{
    ref = 1;
}

QCursorData::~QCursorData()
{
    delete bm;
    delete bmm;
}


/*****************************************************************************
  Global cursors
 *****************************************************************************/

extern QCursorData *qt_cursorTable[Qt::LastCursor + 1]; // qcursor.cpp

int QCursor::handle() const
{
    return d->id;
}


void QCursor::setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    if (bitmap.depth() != 1 || mask.depth() != 1 || bitmap.size() != mask.size()) {
        qWarning("QCursor: Cannot create bitmap cursor; invalid bitmap(s)");
        QCursorData *c = qt_cursorTable[0];
        c->ref.ref();
        d = c;
        return;
    }
    d = new QCursorData;
    d->bm  = new QBitmap(bitmap);
    d->bmm = new QBitmap(mask);
    d->cshape = Qt::BitmapCursor;
    d->id = ++nextCursorId;
    d->hx = hotX >= 0 ? hotX : bitmap.width() / 2;
    d->hy = hotY >= 0 ? hotY : bitmap.height() / 2;

    QPaintDevice::qwsDisplay()->defineCursor(d->id, *d->bm, *d->bmm, d->hx, d->hy);
}

void QCursor::update() const
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    if (d->cshape == Qt::BitmapCursor) {
        // XXX
        return;
    }
    if (d->cshape >= Qt::SizeVerCursor && d->cshape < Qt::SizeAllCursor ||
         d->cshape == Qt::BlankCursor) {
        //        int i = (d->cshape - Qt::SizeVerCursor)*2;
        // XXX data: cursor_bits16[i], 16,16
        // XXX mask: cursor_bits16[i+1], 16,16
        return;
    }
    if (d->cshape >= Qt::SplitVCursor && d->cshape <= Qt::PointingHandCursor) {
        //int i = (d->cshape - Qt::SplitVCursor)*2;
        // XXX data: cursor_bits32[i], 32, 32
        // XXX mask: cursor_bits32[i+1], 32, 32
        //int hs = d->cshape != Qt::PointingHandCursor? 16 : 0;
        // XXX ...
        return;
    }

    // XXX standard shapes?
}

#endif //QT_NO_CURSOR

extern int *qt_last_x,*qt_last_y;

QPoint QCursor::pos()
{
    // This doesn't know about hotspots yet so we disable it
    //qt_accel_update_cursor();
    if (qt_last_x)
        return QPoint(*qt_last_x, *qt_last_y);
    else
        return QPoint();
}

void QCursor::setPos(int x, int y)
{
    // Need to check, since some X servers generate null mouse move
    // events, causing looping in applications which call setPos() on
    // every mouse move event.
    //
    if (pos() == QPoint(x, y))
        return;
    QPaintDevice::qwsDisplay()->setCursorPosition(x, y);
}
