
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
    timer->start( 30 );
}

QVFbView::~QVFbView()
{
    struct shmid_ds shm;
    shmdt( data );
    shmctl( shmId, IPC_RMID, &shm );
    ::close( mouseFd );
    unlink( QT_VFB_MOUSE_PIPE );
    qDebug( "destructed QVFbView" );
}

void QVFbView::sendMouseData( const QPoint &pos, int buttons )
{
    write( mouseFd, &pos, sizeof( QPoint ) );
    write( mouseFd, &buttons, sizeof( int ) );
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

