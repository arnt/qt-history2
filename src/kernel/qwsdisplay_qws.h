/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwsdisplay_qws.h#151 $
**
** QWS display
**
** Created : 20000616
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWSDISPLAY_H
#define QWSDISPLAY_H

#include "qregion.h"
#include "qlock_qws.h"
#include "qwindowdefs.h"

// Class forward definitions
class QWSDisplayData;
class QWSRegionManager;

class QWSDisplay
{
public:
    QWSDisplay();

    bool eventPending() const;
    QWSEvent *getEvent();
    QGfx * screenGfx();
    QWSRegionManager *regionManager();

    uchar* frameBuffer() const;
    int width() const;
    int height() const;
    int depth() const;
    int greenDepth() const;

    void addProperty( int winId, int property );
    void setProperty( int winId, int property, int mode, const QByteArray &data );
    void removeProperty( int winId, int property );
    bool getProperty( int winId, int property, char *&data, int &len );

    void requestRegion( int winId, QRegion );
    void moveRegion( int winId, int dx, int dy );
    void destroyRegion( int winId );
    void requestFocus(int winId, bool get);
    void setAltitude( int winId, int altitude, bool fixed = FALSE );
    int takeId();
    void setSelectionOwner( int winId, const QTime &time );
    void convertSelection( int winId, int selectionProperty, const QString &mimeTypes );
    void defineCursor(int id, const QBitmap &curs, const QBitmap &mask,
			int hotX, int hotY);
    void selectCursor( QWidget *w, unsigned int id );
    void grabMouse( QWidget *w, bool grab );

    // Lock display for access only by this process
    static bool initLock( const QString &filename, bool create = FALSE );
    static bool grabbed() { return lock->locked(); }
    static void grab() { lock->lock( QLock::Write ); }
    static void ungrab() { lock->unlock(); }

private:
    friend class QApplication;
    QWSDisplayData *d;

    int getPropertyLen;
    char *getPropertyData;
    static QLock *lock;
};

#endif // QWSDISPLAY_H

