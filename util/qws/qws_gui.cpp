/****************************************************************************
** $Id$
**
** Implementation of Qt/FB dummy framebuffer debug GUI
**
** Created : 991214
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
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

class ZoomBox : public QWidget {
    static const int s=5;
    const QImage& image;
    QPoint dragpos;
public:
    ZoomBox(const QImage& i, QWidget* parent) :
	QWidget(parent),
	image(i)
    {
	resize(202,202);
    }

    void mousePressEvent(QMouseEvent* e)
    {
	dragpos = e->pos();
    }

    int zX() const { return (width()-2)*(s-1)/2/s; }
    int zY() const { return (height()-2)*(s-1)/2/s; }

    void mouseMoveEvent(QMouseEvent* e)
    {
	QPoint p = mapToParent(mapFromGlobal(e->globalPos()))-dragpos;
	//if ( p.x()+zX() < 0 ) p.rx() = -zX();
	//if ( p.y()+zY() < 0 ) p.ry() = -zY();
	move(p);
	repaint(FALSE);
    }

    void paintEvent(QPaintEvent*)
    {
	QRect area(x()+zX(),y()+zY(),(width()-2)/s,(height()-2)/s);
	QPixmap pm;
	pm.convertFromImage(image.copy(area));
	QWMatrix scale; scale.scale(s,s);
	pm = pm.xForm(scale);
	QPainter p(this);
	p.setBackgroundMode(OpaqueMode);
	p.drawPixmap(1,1,pm);
	p.setPen(blue);
	p.drawRect(rect());
    }
};

DummyFramebuffer::DummyFramebuffer( QWidget* parent ) :
    QWidget(parent),
    server( 0 )
{
    setFocusPolicy(StrongFocus);
    setFocus();
    setBackgroundMode(NoBackground);
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


void DummyFramebuffer::serve(int depth, int refresh_delay)
{
    if ( !server ) {
	setFixedSize(size()); // Allow -geometry to set it, but then freeze.
	int swidth = width();
	int sheight = height();
	setFixedSize(swidth,sheight);
	server = new QWSServer( swidth, sheight, depth, 0,  this );
	int nc=0;
	if ( depth == 8 || depth == 1 )
	    nc = 1<<depth;
	img = QImage( server->frameBuffer(),
		    swidth, sheight, depth, 0, nc, QImage::BigEndian );
	oldimg = QImage(swidth, sheight, depth, nc, QImage::BigEndian);
	if ( nc ) {
	    for (int i=0; i<nc; i++) {
		qDebug("color %d: #%06x",i,qRgb(i*255/(nc-1),i*255/(nc-1),i*255/(nc-1)));
		img.setColor(i,qRgb(i*255/(nc-1),i*255/(nc-1),i*255/(nc-1)));
		oldimg.setColor(i,qRgb(i*255/(nc-1),i*255/(nc-1),i*255/(nc-1)));
	    }
	}
	startTimer(refresh_delay);
    }
}

void DummyFramebuffer::setRegionDisplay(bool y)
{
    showregions = y;
    repaint(FALSE);
}

void DummyFramebuffer::setZoomBox(bool y)
{
    if ( y )
	(zoombox = new ZoomBox(oldimg,this))->show();
    else
	delete zoombox, zoombox=0;
}

void DummyFramebuffer::timerEvent(QTimerEvent*)
{
    if ( showregions || img.depth() != 32 ) {
	repaint(FALSE);
    } else {
	int y;
	for (y=0; y<img.height()-1; y++) {
	    QRgb* n=(QRgb*)img.scanLine(y);
	    QRgb* o=(QRgb*)oldimg.scanLine(y);
	    for (int x=0; x<img.width(); x++)
		if ( n[x] != o[x] ) goto topfound;
	}
topfound:
	int miny=y;
	for (y=img.height()-1; y>miny; y--) {
	    QRgb* n=(QRgb*)img.scanLine(y);
	    QRgb* o=(QRgb*)oldimg.scanLine(y);
	    for (int x=0; x<img.width(); x++)
		if ( n[x] != o[x] ) goto bottomfound;
	}
bottomfound:
	int maxy=y;
	int x;
	for (x=0; x<img.width()-1; x++) {
	    for (int y=miny; y<=maxy; y++) {
		QRgb* n=(QRgb*)img.scanLine(y);
		QRgb* o=(QRgb*)oldimg.scanLine(y);
		if ( n[x] != o[x] ) goto leftfound;
	    }
	}
leftfound:
	int minx=x;
	for (x=img.width()-1; x>minx; x--) {
	    for (int y=miny; y<=maxy; y++) {
		QRgb* n=(QRgb*)img.scanLine(y);
		QRgb* o=(QRgb*)oldimg.scanLine(y);
		if ( n[x] != o[x] ) goto rightfound;
	    }
	}
rightfound:
	int maxx=x;
	QRect r; r.setCoords(minx,miny,maxx,maxy);
	bitBlt(&oldimg,minx,miny,&img,minx,miny,maxx-minx+1,maxy-miny+1);
	repaint(r);
	if ( zoombox )
	    zoombox->repaint(FALSE); // ### could optimize
    }
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
    // The DummyFramebuffer should simulate a hardware cursor, since
    // that's what it has... "virtually".
    //server->setMouse(e->pos(), e->stateAfter());

    server->sendMouseEvent(e->pos(), e->stateAfter());
}

void DummyFramebuffer::keyPressEvent(QKeyEvent* e)
{
    keyEvent(e);
}
void DummyFramebuffer::keyReleaseEvent(QKeyEvent* e)
{
    keyEvent(e);
}

void DummyFramebuffer::keyEvent(QKeyEvent* e)
{
    QString text = e->text();
    int state = e->stateAfter();
    int c=e->count(); if ( !c ) c=1;
    for ( ; c; c-- ) {
	int uc = text[0].unicode() + ((e->key()&0xffff)<<16);
	int i=0;
	while (uc) {
	    server->sendKeyEvent(uc,state,e->type()==QEvent::KeyPress,e->isAutoRepeat());
	    uc = text.isEmpty() ? 0 : text[++i];
	}
    }
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
	QPtrList<QWSWindow> windows = server->clientWindows();
	int nc=0;
	for (QWSWindow* i = windows.first(); i; i=windows.next()) {
	    p.setPen(QColor(rgb[nc]));
	    //p.setBrush(NoBrush);
	    p.setBrush(QBrush(QColor(rgb[nc]),Dense6Pattern));
	    nc = (nc + 1)%(sizeof(rgb)/sizeof(rgb[0]));
	    QRegion r = i->allocation();
	    QMemArray<QRect> rects = r.rects();
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
    show_zoom_box_id = view->insertItem("Zoom box",
				this, SLOT(toggleZoomBox()));

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

void DebuggingGUI::toggleZoomBox()
{
    view->setItemChecked( show_zoom_box_id, !view->isItemChecked(show_zoom_box_id) );
    fb->setZoomBox( view->isItemChecked(show_zoom_box_id) );
}

bool DebuggingGUI::showRegions() const
{
    return view->isItemChecked( show_client_regions_id );
}
