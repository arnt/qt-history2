/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qvfbview.h"
#include <qvfbhdr_qws.h>
#include <qwscommand_qws.h>	    // for QTE_PIPE

#include <qimage.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qwmatrix.h>
#include <qpainter.h>
#include "qanimationwriter.h"

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>

QVFbView::QVFbView( int display_id, int w, int h, int d, QWidget *parent,
		    const char *name, uint flags )
    : QScrollView( parent, name, flags ), lockId(-1)
{
    displayId = display_id;
    viewport()->setMouseTracking( TRUE );
    viewport()->setFocusPolicy( StrongFocus );
    zoom = 1;
    animation = 0;

    switch ( d ) {
	case 1:
	case 8:
	case 32:
	    break;
	
	default:
	    qFatal( "Unsupported bit depth %d\n", d );
    }

    mousePipe = QString(QT_VFB_MOUSE_PIPE).arg(display_id);
    keyboardPipe = QString(QT_VFB_KEYBOARD_PIPE).arg(display_id);

    unlink( mousePipe.latin1() );
    mknod( mousePipe.latin1(), S_IFIFO | 0666, 0 );
    mouseFd = open( mousePipe.latin1(), O_RDWR | O_NDELAY );
    if ( mouseFd == -1 ) {
	qFatal( "Cannot open mouse pipe" );
    }

    unlink( keyboardPipe );
    mknod( keyboardPipe, S_IFIFO | 0666, 0 );
    keyboardFd = open( keyboardPipe, O_RDWR | O_NDELAY );
    if ( keyboardFd == -1 ) {
	qFatal( "Cannot open keyboard pipe" );
    }

    key_t key = ftok( mousePipe.latin1(), 'b' );

    int bpl;
    if ( d == 1 )
	bpl = (w*d+7)/8;
    else
	bpl = ((w*d+31)/32)*4;
    
    int dataSize = bpl * h + 1024;
    shmId = shmget( key, dataSize, IPC_CREAT|0666);
    if ( shmId != -1 )
	data = (unsigned char *)shmat( shmId, 0, 0 );
    else {
	struct shmid_ds shm;
	shmctl( shmId, IPC_RMID, &shm );
	shmId = shmget( key, dataSize, IPC_CREAT|0666);
	data = (unsigned char *)shmat( shmId, 0, 0 );
    }

    if ( (int)data == -1 )
	qFatal( "Cannot attach to shared memory" );

    hdr = (QVFbHeader *)data;
    hdr->width = w;
    hdr->height = h;
    hdr->depth = d;
    hdr->linestep = bpl;
    hdr->numcols = 0;
    hdr->dataoffset = 1024;
    hdr->update = QRect();

    resizeContents( w, h );

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );

    setRate( 30 );
}

QVFbView::~QVFbView()
{
    stopAnimation();
    sendKeyboardData( 0, 0, 0, TRUE, FALSE ); // magic die key
    struct shmid_ds shm;
    shmdt( (char*)data );
    shmctl( shmId, IPC_RMID, &shm );
    ::close( mouseFd );
    ::close( keyboardFd );
    unlink( mousePipe );
    unlink( keyboardPipe );
}


void QVFbView::setZoom( double z )
{
    if ( zoom != z ) {
	zoom = z;
	setDirty(QRect(0,0,hdr->width,hdr->height));
	resizeContents( int(hdr->width*z), int(hdr->height*z) );
	drawScreen();
    }
}

void QVFbView::setRate( int r )
{
    refreshRate = r;
    timer->start( 1000/r );
}

void QVFbView::initLock()
{
    QString pipe = QString( QTE_PIPE ).arg( displayId );
    int semkey = ftok( pipe.latin1(), 'd' );
    lockId = semget( semkey, 0, 0 );
}

void QVFbView::lock()
{
    if ( lockId == -1 )
	initLock();

    sembuf sops;
    sops.sem_num = 0;
    sops.sem_flg = SEM_UNDO;
    sops.sem_op = -1;
    int rv;
    do {
	rv = semop(lockId,&sops,1);
    } while ( rv == -1 && errno == EINTR );

    if ( rv == -1 )
	lockId = -1;
}

void QVFbView::unlock()
{
    if ( lockId >= 0 ) {
	sembuf sops;
	sops.sem_num = 0;
	sops.sem_op = 1;
	sops.sem_flg = SEM_UNDO;
	int rv;
	do {
	    rv = semop(lockId,&sops,1);
	} while ( rv == -1 && errno == EINTR );
    }
}

void QVFbView::sendMouseData( const QPoint &pos, int buttons )
{
    write( mouseFd, &pos, sizeof( QPoint ) );
    write( mouseFd, &buttons, sizeof( int ) );
}

void QVFbView::sendKeyboardData( int unicode, int keycode, int modifiers,
				 bool press, bool repeat )
{
    QVFbKeyData kd;

    kd.unicode = unicode | (keycode << 16);
    kd.modifiers = modifiers;
    kd.press = press;
    kd.repeat = repeat;
    write( keyboardFd, &kd, sizeof( QVFbKeyData ) );
}

void QVFbView::timeout()
{
    lock();
    if ( animation ) {
	    // ### Could use img from drawScreen, with offset, but we do it
	    // ### this way for now to exercise QAnimationWriter.
	    QRect r( hdr->update );
	    r = r.intersect( QRect(0, 0, hdr->width, hdr->height ) );
	    if ( r.isEmpty() ) {
		animation->appendBlankFrame();
	    } else {
		QImage img( data + hdr->dataoffset + r.y() * hdr->linestep,
			hdr->width, r.height(), hdr->depth, hdr->clut,
			hdr->depth == 1 ? 0 : 256, QImage::LittleEndian );
		animation->appendFrame(img,QPoint(0,r.y()));
	    }
    }
    if ( hdr->dirty ) {
	drawScreen();
    }
    unlock();
}

void QVFbView::drawScreen()
{
    QPainter p( viewport() );

    p.translate( -contentsX(), -contentsY() );

    lock();
    QRect r( hdr->update );
    hdr->dirty = FALSE;
    hdr->update = QRect();
//    qDebug( "update %d, %d, %dx%d", r.y(), r.x(), r.width(), r.height() );
    r = r.intersect( QRect(0, 0, hdr->width, hdr->height ) );
    if ( !r.isEmpty() )  {
	if ( int(zoom) != zoom ) {
	    r.rLeft() = int(int(r.left()*zoom)/zoom);
	    r.rTop() = int(int(r.top()*zoom)/zoom);
	    r.rRight() = int(int(r.right()*zoom+zoom+0.0000001)/zoom+1.9999);
	    r.rBottom() = int(int(r.bottom()*zoom+zoom+0.0000001)/zoom+1.9999);
	    r.rRight() = QMIN(r.right(),hdr->width-1);
	    r.rBottom() = QMIN(r.bottom(),hdr->height-1);
	}
	QImage img( data + hdr->dataoffset + r.y() * hdr->linestep,
		    hdr->width, r.height(), hdr->depth, hdr->clut,
		    hdr->depth == 1 ? 0 : 256, QImage::LittleEndian );
	QPixmap pm;
	if ( zoom == 1 ) {
	    pm.convertFromImage( img );
	} else if ( int(zoom) == zoom ) {
	    QWMatrix m;
	    m.scale(zoom,zoom);
	    pm.convertFromImage( img );
	    pm = pm.xForm(m);
	} else {
	    pm.convertFromImage( img.smoothScale(int(img.width()*zoom),int(img.height()*zoom)) );
	}
	unlock();
	p.setPen( black );
	p.setBrush( white );
	p.drawPixmap( int(r.x()*zoom), int(r.y()*zoom), pm,
			int(r.x()*zoom), 0, pm.width(), pm.height() );
    } else {
	unlock();
    }
}

bool QVFbView::eventFilter( QObject *obj, QEvent *e )
{
    if ( obj == viewport() &&
	 (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut) )
	return TRUE;

    return QScrollView::eventFilter( obj, e );
}

void QVFbView::viewportPaintEvent( QPaintEvent *pe )
{
    QRect r( pe->rect() );
    r.moveBy( contentsX(), contentsY() );
    r = QRect(int(r.x()/zoom),int(r.y()/zoom),
	    int(r.width()/zoom)+1,int(r.height()/zoom)+1);
    setDirty(r);
    drawScreen();
}

void QVFbView::setDirty( const QRect& r )
{
    lock();
    hdr->update |= r;
    hdr->dirty = TRUE;
    unlock();
}

void QVFbView::contentsMousePressEvent( QMouseEvent *e )
{
    sendMouseData( e->pos()/zoom, e->stateAfter() );
}

void QVFbView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
    sendMouseData( e->pos()/zoom, e->stateAfter() );
}

void QVFbView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    sendMouseData( e->pos()/zoom, e->stateAfter() );
}

void QVFbView::contentsMouseMoveEvent( QMouseEvent *e )
{
    sendMouseData( e->pos()/zoom, e->state() );
}



void QVFbView::keyPressEvent( QKeyEvent *e )
{
    sendKeyboardData(e->text()[0].unicode(), e->key(), 
		     e->state()&(ShiftButton|ControlButton|AltButton),
		     TRUE, e->isAutoRepeat());
}

void QVFbView::keyReleaseEvent( QKeyEvent *e )
{
    sendKeyboardData(e->text()[0].unicode(), e->key(), 
		     e->state()&(ShiftButton|ControlButton|AltButton),
		     FALSE, e->isAutoRepeat());
}


void QVFbView::saveAs( const QString& filename )
{
    QImage img( data + hdr->dataoffset,
		hdr->width, hdr->height, hdr->depth, hdr->clut,
		256, QImage::LittleEndian );
    img.save(filename,"PNG");
}

void QVFbView::startAnimation( const QString& filename )
{
    delete animation;
    animation = new QAnimationWriter(filename,"MNG");
    animation->setFrameRate(refreshRate);
    animation->appendFrame(QImage(data + hdr->dataoffset,
                hdr->width, hdr->height, hdr->depth, hdr->clut,
                256, QImage::LittleEndian));
}

void QVFbView::stopAnimation()
{
    delete animation;
    animation = 0;
}

