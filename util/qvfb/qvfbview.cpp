
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <qimage.h>
#include <qtimer.h>

#include "qvfbview.h"
#include "qvfbhdr.h"

QVFbView::QVFbView( int w, int h, int d, QWidget *parent, const char *name,
			uint flags )
    : QScrollView( parent, name, flags )
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

    image = new QImage( data + hdr->dataoffset, w, h, d, hdr->clut, 256,
			QImage::LittleEndian );
    
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
    ::close( mouseFd );
    unlink( QT_VFB_MOUSE_PIPE );
    unlink( QT_VFB_KEYBOARD_PIPE );
}

void QVFbView::setRate( int r )
{
    refreshRate = r;
    timer->start( 1000/r );
}

void QVFbView::sendMouseData( const QPoint &pos, int buttons )
{
    write( mouseFd, &pos, sizeof( QPoint ) );
    write( mouseFd, &buttons, sizeof( int ) );
}

void QVFbView::sendKeyboardData( int unicode, int keycode, bool press,
				 bool repeat )
{
    QVFbKeyData kd;

    kd.unicode = unicode | (keycode << 16);
    kd.press = press;
    kd.repeat = repeat;
    write( keyboardFd, &kd, sizeof( QVFbKeyData ) );
}

void QVFbView::timeout()
{
    if ( hdr->dirty ) {
	hdr->dirty = false;
	repaintContents( 0, 0, hdr->width, hdr->height, false );
    }
}

void QVFbView::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    QPixmap pm;
    pm.convertFromImage( *image );

    p->drawPixmap( cx, cy, pm, cx, cy, cw, ch );
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
    sendKeyboardData(e->text()[0].unicode(), e->key(), TRUE, e->isAutoRepeat());
}

void QVFbView::keyReleaseEvent( QKeyEvent *e )
{
    sendKeyboardData(e->text()[0].unicode(), e->key(), FALSE, e->isAutoRepeat());
}

