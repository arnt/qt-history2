/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.cpp#23 $
**
** Implementation of Qt/FB dummy framebuffer debug GUI
**
** Created : 991214
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qws_gui.h"
#include "qws.h"
#include "qmenubar.h"
#include "qpopupmenu.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qfiledialog.h"

DummyFramebuffer::DummyFramebuffer( QWidget* parent ) :
    QWidget(parent),
    server( 0 )
{
    setMouseTracking(TRUE);
    showregions = FALSE;
}

QSize DummyFramebuffer::sizeHint() const
{
    return QSize(640,480);
}

QSizePolicy DummyFramebuffer::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}


void DummyFramebuffer::serve(int refresh_delay)
{
    if ( !server ) {
	setFixedSize(size()); // Allow -geometry to set it, but then freeze.
	int swidth = width();
	int sheight = height();
	setFixedSize(swidth,sheight);
	server = new QWSServer( swidth, sheight, this );
	img = QImage( server->frameBuffer(),
		    swidth, sheight, 32, 0, 0, QImage::BigEndian );
	startTimer(refresh_delay);
    }
}

void DummyFramebuffer::setRegionDisplay(bool y)
{
    showregions = y;
    repaint(FALSE);
}

void DummyFramebuffer::timerEvent(QTimerEvent*)
{
    repaint(FALSE);
}

void DummyFramebuffer::mousePressEvent(QMouseEvent* e)
{
    sendMouseEvent(e);
}
void DummyFramebuffer::mouseReleaseEvent(QMouseEvent* e)
{
    sendMouseEvent(e);
}
void DummyFramebuffer::mouseMoveEvent(QMouseEvent* e)
{
    sendMouseEvent(e);
}

void DummyFramebuffer::sendMouseEvent(QMouseEvent* e)
{
    server->sendMouseEvent(e->pos(), e->stateAfter());
}

void DummyFramebuffer::paintEvent(QPaintEvent* e)
{
    static QRgb rgb[] = {
	0x0000ff,
	0x00ff00,
	0x00ffff,
	0xff0000,
	0xff00ff,
	0xffff00,
	0xffffff,
	0x000000,
    };

    QRect r = e->rect();

    QPixmap pm;
    pm.convertFromImage(img,OrderedDither);
    if ( showregions ) {
	QPainter p(&pm);
	QList<QWSWindow> windows = server->clientWindows();
	int nc=0;
	for (QWSWindow* i = windows.first(); i; i=windows.next()) {
	    p.setPen(QColor(rgb[nc]));
	    //p.setBrush(NoBrush);
	    p.setBrush(QBrush(QColor(rgb[nc]),Dense6Pattern));
	    nc = (nc + 1)%(sizeof(rgb)/sizeof(rgb[0]));
	    QRegion r = i->allocation();
	    QArray<QRect> rects = r.rects();
	    for (int j=0; j<(int)rects.count(); j++) {
		p.drawRect(rects[j]);
	    }
	}
    }
    bitBlt(this, r.x(), r.y(), &pm, r.x(), r.y(), r.width(), r.height());
}

DebuggingGUI::DebuggingGUI()
{
    QPopupMenu * file = new QPopupMenu( this );
    file->insertItem("Screendump...", this, SLOT(screendump()));
    file->insertSeparator();
    file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ) );
    menuBar()->insertItem("&File", file);

    view = new QPopupMenu( this );

    show_client_regions_id = view->insertItem("Show client regions",
				this, SLOT(toggleRegions()));
    view->setItemChecked( show_client_regions_id, FALSE );

    menuBar()->insertItem("&View", view);

    fb = new DummyFramebuffer(this);
    setCentralWidget(fb);
}

void DebuggingGUI::screendump()
{
    QString filename = QFileDialog::getSaveFileName("screen.png","*.png",this);
    if ( !!filename ) {
	fb->image().save(filename,"PNG");
    }
}

void DebuggingGUI::toggleRegions()
{
    view->setItemChecked( show_client_regions_id, !showRegions() );
    fb->setRegionDisplay(showRegions());
}

bool DebuggingGUI::showRegions() const
{
    return view->isItemChecked( show_client_regions_id );
}
