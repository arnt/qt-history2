/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>

#include <qimage.h>
#include <qtimer.h>

#include "qvfbview.h"
#include <qvfbhdr_qws.h>

QVFbView::QVFbView( int w, int h, int d, QWidget *parent, const char *name,
			uint flags )
    : QScrollView( parent, name, flags ), lockId(-1)
{
    viewport()->setMouseTracking( true );
    viewport()->setFocusPolicy( StrongFocus );

    switch ( d ) {
	case 8:
	case 32:
	    break;
	
	default:
	    qFatal( "Unsupported bit depth %d\n", d );
    }

    mknod( QT_VFB_MOUSE_PIPE, S_IFIFO | 0666, 0 );
    mouseFd = open( QT_VFB_MOUSE_PIPE, O_RDWR | O_NDELAY );
    if ( mouseFd == -1 ) {
	qFatal( "Cannot open mouse pipe" );
    }

    mknod( QT_VFB_KEYBOARD_PIPE, S_IFIFO | 0666, 0 );
    keyboardFd = open( QT_VFB_KEYBOARD_PIPE, O_RDWR | O_NDELAY );
    if ( keyboardFd == -1 ) {
	qFatal( "Cannot open keyboard pipe" );
    }

    key_t key = ftok( QT_VFB_MOUSE_PIPE, 'b' );

    int bpl = ((w*d+31)/32)*4;
    
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
    hdr->dataoffset = 1024;
    hdr->update = QRect();

    resizeContents( w, h );

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );

    setRate( 30 );
}

QVFbView::~QVFbView()
{
    struct shmid_ds shm;
    shmdt( data );
    shmctl( shmId, IPC_RMID, &shm );
    sendKeyboardData( 0, 0, 0, TRUE, FALSE ); // magic die key
    ::close( mouseFd );
    ::close( keyboardFd );
    unlink( QT_VFB_MOUSE_PIPE );
    unlink( QT_VFB_KEYBOARD_PIPE );
}

void QVFbView::setRate( int r )
{
    refreshRate = r;
    timer->start( 1000/r );
}

void QVFbView::initLock()
{
    int semkey = ftok( "/dev/fb0", 'd' );
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
	if (rv == -1 && errno != EINTR)
	    qDebug("Semop unlock failure %s",strerror(errno));
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
	    if (rv == -1 && errno != EINTR)
		qDebug("Semop unlock failure %s",strerror(errno));
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
    if ( hdr->dirty ) {
	drawScreen();
    }
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
	QImage img( data + hdr->dataoffset + r.y() * hdr->linestep,
		    hdr->width, r.height(), hdr->depth, hdr->clut,
		    256, QImage::LittleEndian );
	QPixmap pm;
	pm.convertFromImage( img );
	unlock();
	p.drawPixmap( r.x(), r.y(), pm, r.x(), 0, r.width(), r.height() );
    } else
	unlock();
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
    lock();
    QRect r( pe->rect() );
    r.moveBy( contentsX(), contentsY() );
    hdr->update = hdr->update.unite( r );
    hdr->dirty = TRUE;
    unlock();
    drawScreen();
}

void QVFbView::contentsMousePressEvent( QMouseEvent *e )
{
    sendMouseData( e->pos(), e->stateAfter() );
}

void QVFbView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    sendMouseData( e->pos(), e->stateAfter() );
}

void QVFbView::contentsMouseMoveEvent( QMouseEvent *e )
{
    sendMouseData( e->pos(), e->state() );
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

