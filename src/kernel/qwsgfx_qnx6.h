/****************************************************************************
** $Id$
**
** Definition of Qt/Embedded Qnx keyboard drivers
**
** Copyright (C) 1999-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QWSQNXFB_H
#define QWSQNXFB_H

#ifdef Q_OS_QNX6

#include <display.h>
#include <disputil.h>
#include <qgfxraster_qws.h>
#include <qgfx_qws.h>
#include <qpolygonscanner.h>
#include <qpen.h>
#include <qstring.h>

// QnxFb Gfx class
template <const int depth, const int type>
class QQnxFbGfx : public QGfxRaster<depth, type> {
    public:
	QQnxFbGfx();
	~QQnxFbGfx();

	int bitDepth(){ return DISP_BITS_PER_PIXEL ( ctx.dsurf->pixel_format );};

	void sync ();
	void fillRect (int,int,int,int);
	void hlineUnclipped ( int, int, int);

    private:
	disp_draw_context_t ctx;
};

// Screen class
class QQnxScreen : public QScreen {
    public:
	QQnxScreen(int display_id):QScreen(display_id){};
	~QQnxScreen(){};

	bool connect(const QString & spec);
	void disconnect();

	bool initDevice();
	void shutdownDevice();
	void setMode(int, int, int);

	QRgb *clut();

	QGfx* createGfx (unsigned char*, int, int, int, int);

	int initCursor(void *, bool);
};

#ifndef QT_NO_QWS_CURSOR
class QQnxCursor : public QScreenCursor
{
public:
    QQnxCursor(){};
    ~QQnxCursor(){};

    virtual void init(SWCursorData *,bool=FALSE);

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
    virtual void show();
    virtual void hide();

    virtual bool restoreUnder( const QRect &, QGfxRasterBase * = 0 )
                { return FALSE; }
    virtual void saveUnder() {}
    virtual void drawCursor() {}
    virtual void draw() {}
    virtual bool supportsAlphaCursor() { return false; }

    static bool enabled() { return false; }

private:
    int hotx;
    int hoty;
    QBitArray cursor,mask;
    QColor colour0,colour1;
};
#endif // QT_NO_QWS_CURSOR

#endif // Q_OS_QNX6

#endif // QWSQNXFB_H
