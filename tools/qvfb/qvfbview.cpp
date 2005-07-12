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

#if !defined( Q_WS_QWS ) || defined( QT_NO_QWS_MULTIPROCESS )
#define QLock QWSSemaphore
#undef QT_NO_QWS_MULTIPROCESS
#include "../../src/gui/embedded/qlock.cpp"
#else
#include "qlock_p.h"
#endif

#include "qvfbview.h"
#include "qvfbhdr.h"

#define QTE_PIPE "QtEmbedded-%1"

#include <qapplication.h>
#include <qpainter.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qwmatrix.h>
#include <QPaintEvent>
#include <QScrollArea>
#include <qfile.h>
#include "qanimationwriter.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#ifndef Q_WS_QWS
// Get the name of the directory where Qt/Embedded temporary data should
// live.
static QString qws_dataDir(int qws_display_id)
{
    QByteArray dataDir = QString("/tmp/qtembedded-%1").arg(qws_display_id).toLocal8Bit();
    if (mkdir(dataDir, 0700)) {
        if (errno != EEXIST) {
            qFatal("Cannot create Qt/Embedded data directory: %s", dataDir.constData());
        }
    }

    struct stat buf;
    if (lstat(dataDir, &buf))
        qFatal("stat failed for Qt/Embedded data directory: %s", dataDir.constData());

    if (!S_ISDIR(buf.st_mode))
        qFatal("%s is not a directory", dataDir.constData());
    if (buf.st_uid != getuid())
        qFatal("Qt/Embedded data directory is not owned by user %d", getuid());

    if ((buf.st_mode & 0677) != 0600)
        qFatal("Qt/Embedded data directory has incorrect permissions: %s", dataDir.constData());
    dataDir += "/";

    return QString(dataDir);
}
#endif

static QString displayPipe;
static QString displayPiped;
class DisplayLock
{
public:
    DisplayLock() : qlock(0) {
        if (QFile::exists(displayPiped)) {
            qlock = new QLock(displayPipe, 'd', false);
            qlock->lock(QLock::Read);
        }
    }
    ~DisplayLock() {
        if (qlock) {
            qlock->unlock();
            delete qlock;
            qlock = 0;
        }
    }
private:
    QLock *qlock;
};

QVFbView::QVFbView( int display_id, int _w, int _h, int d, Rotation r, QWidget *parent,
		    Qt::WFlags  flags )
    : QWidget( parent, flags ), lockId(-1), emulateTouchscreen(false), emulateLcdScreen(false)
{
    displayid = display_id;
    rotation = r;
    setMouseTracking( TRUE );
    setFocusPolicy( Qt::StrongFocus );
    setBackgroundMode( Qt::NoBackground );
    hzm = 1;
    vzm = 1;
    animation = 0;
    int actualdepth=d;

    switch ( d ) {
	case 12:
	    actualdepth=16;
	    break;
	case 1:
	case 4:
	case 8:
	case 16:
	case 24:
	case 32:
	    break;

	default:
	    qFatal( "Unsupported bit depth %d\n", d );
    }

    int w = ( rotation & 0x1 ) ? _h : _w;
    int h = ( rotation & 0x1 ) ? _w : _h;

    QString username = "unknown";
    const char *logname = getenv("LOGNAME");
    if ( logname )
        username = logname;

    QString oldPipe = "/tmp/qtembedded-" + username + "/" + QString( QTE_PIPE ).arg( displayid );
    int oldPipeSemkey = ftok( oldPipe.latin1(), 'd' );
    int oldPipeLockId = semget( oldPipeSemkey, 0, 0 );
    if (oldPipeLockId >= 0){
        sembuf sops;
        sops.sem_num = 0;
        sops.sem_op = 1;
        sops.sem_flg = SEM_UNDO;
        int rv;
        do {
            rv = semop(lockId,&sops,1);
        } while ( rv == -1 && errno == EINTR );
        qFatal("Cannot create lock file as an old version of QVFb has opened %s. Close other QVFb and try again", oldPipe.latin1());
    }

    mousePipe = QString(QT_VFB_MOUSE_PIPE).arg(display_id);
    keyboardPipe = QString(QT_VFB_KEYBOARD_PIPE).arg(display_id);

    unlink( mousePipe.local8Bit().data() );
    mkfifo( mousePipe.local8Bit().data(), 0666 );

    mouseFd = ::open( mousePipe.local8Bit().data(), O_RDWR | O_NDELAY );
    if ( mouseFd == -1 ) {
	qFatal( "Cannot open mouse pipe %s", mousePipe.latin1());
    }

    unlink( keyboardPipe );
    mkfifo( keyboardPipe, 0666 );
    keyboardFd = ::open( keyboardPipe.local8Bit().data(), O_RDWR | O_NDELAY );
    if ( keyboardFd == -1 ) {
	::close(mouseFd);
	qFatal( "Cannot open keyboard pipe %s", keyboardPipe.latin1());
    }

    key_t key = ftok( mousePipe.latin1(), 'b' );

    int bpl;
    if ( d == 1 )
	bpl = (w*d+7)/8;
    else
	bpl = ((w*actualdepth+31)/32)*4;

    int dataSize = bpl * h + sizeof(QVFbHeader);
    shmId = shmget( key, dataSize, IPC_CREAT|0666);
    if ( shmId != -1 )
	data = (unsigned char *)shmat( shmId, 0, 0 );
    else {
	struct shmid_ds shm;
	shmctl( shmId, IPC_RMID, &shm );
	shmId = shmget( key, dataSize, IPC_CREAT|0666);
	if ( shmId == -1 )
	    qFatal( "Cannot get shared memory 0x%08x", key );
	data = (unsigned char *)shmat( shmId, 0, 0 );
    }

    if ( (long)data == -1 ){
	::close(mouseFd);
	::close(keyboardFd);
	qFatal( "Cannot attach to shared memory %d",shmId );
    }

    hdr = (QVFbHeader *)data;
    hdr->width = w;
    hdr->height = h;
    viewdepth = d;
    hdr->depth = actualdepth;
    hdr->linestep = bpl;
    hdr->numcols = 0;
    hdr->dataoffset = sizeof(QVFbHeader);
    hdr->update = QRect();

    contentsWidth = _w;
    contentsHeight = _h;

    resize( _w, _h );

    t_flush = new QTimer( this );
    connect( t_flush, SIGNAL(timeout()), this, SLOT(flushChanges()) );

    gammatable=0;
    setGamma(1.0,1.0,1.0);
    setRate( 30 );

#ifdef Q_WS_QWS
    displayPipe = qws_dataDir() + QString( QTE_PIPE ).arg( displayid );
#else
    displayPipe = qws_dataDir(displayid) + QString( QTE_PIPE ).arg( displayid );
#endif
    displayPiped = displayPipe + 'd';
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

QSize QVFbView::sizeHint() const
{
    return QSize(contentsWidth, contentsHeight);
}

void QVFbView::setGamma(double gr, double gg, double gb)
{
    if ( viewdepth < 12 )
	return; // not implemented

    gred=gr; ggreen=gg; gblue=gb;

    switch ( viewdepth ) {
      case 12:
	rsh = 12;
	gsh = 7;
	bsh = 1;
	rmax = 15;
	gmax = 15;
	bmax = 15;
	break;
      case 16:
	rsh = 11;
	gsh = 5;
	bsh = 0;
	rmax = 31;
	gmax = 63;
	bmax = 31;
	break;
      case 24:
      case 32:
	rsh = 16;
	gsh = 8;
	bsh = 0;
	rmax = 255;
	gmax = 255;
	bmax = 255;
    }
    int mm = QMAX(rmax,QMAX(gmax,bmax))+1;
    if ( gammatable )
	delete [] gammatable;
    gammatable = new QRgb[mm];
    for (int i=0; i<mm; i++) {
	int r = int(pow(i,gr)*255/rmax);
	int g = int(pow(i,gg)*255/gmax);
	int b = int(pow(i,gb)*255/bmax);
	if ( r > 255 ) r = 255;
	if ( g > 255 ) g = 255;
	if ( b > 255 ) b = 255;
	gammatable[i] = qRgb(r,g,b);
//qDebug("%d: %d,%d,%d",i,r,g,b);
    }

    setDirty(rect());
}

void QVFbView::getGamma(int i, QRgb& rgb)
{
    if ( i > 255 ) i = 255;
    if ( i < 0 ) i = 0;
    rgb = qRgb(qRed(gammatable[i*rmax/255]),
               qGreen(gammatable[i*rmax/255]),
               qBlue(gammatable[i*rmax/255]));
}

int QVFbView::displayId() const
{
    return displayid;
}

int QVFbView::displayWidth() const
{
    return ( (int)rotation & 0x01 ) ? hdr->height : hdr->width;
}

int QVFbView::displayHeight() const
{
    return ( (int)rotation & 0x01 ) ? hdr->width: hdr->height;
}

int QVFbView::displayDepth() const
{
    return viewdepth;
}

QVFbView::Rotation QVFbView::displayRotation() const
{
    return rotation;
}


void QVFbView::setZoom( double hz, double vz )
{
    if ( hzm != hz || vzm != vz ) {
	hzm = hz;
	vzm = vz;
	setDirty(QRect(0,0,displayWidth(),displayHeight()));
        contentsWidth = int(hdr->width*hz);
        contentsHeight = int(hdr->height*vz);
        resize(contentsWidth, contentsHeight);
	updateGeometry();
	qApp->sendPostedEvents();
	topLevelWidget()->adjustSize();
	update();
    }
}

void QVFbView::setRate( int r )
{
    refreshRate = r;
    t_flush->start( 1000/r );
}

static QRect mapToDevice( const QRect &r, const QSize &s, QVFbView::Rotation rotation )
{
    int x1 = r.x(), y1 = r.y(), x2 = r.right(), y2 = r.bottom(), w = s.width(), h = s.height();
    switch ( rotation ) {
	case QVFbView::Rot90:
	    return QRect( QPoint(y1, w - x1 - 1), QPoint(y2, w - x2 - 1) );
	case QVFbView::Rot180:
	    return QRect( QPoint(w - x1 - 1, h - y1 - 1), QPoint(w - x2 - 1, h - y2 - 1) );
	case QVFbView::Rot270:
	    return QRect( QPoint(h - y1 - 1, x1), QPoint(h - y2 - 1, x2) );
	default:
	    break;
    }
    return r;
}

void QVFbView::sendMouseData( const QPoint &pos, int buttons, int wheel )
{
    QPoint p = mapToDevice( QRect(pos,QSize(1,1)), QSize(displayWidth(), displayHeight()), rotation ).topLeft(); 
    write( mouseFd, &p, sizeof( QPoint ) );
    write( mouseFd, &buttons, sizeof( int ) );
    if (hdr->serverVersion >= 0x040000) {
        write( mouseFd, &wheel, sizeof( int ) );
    }
}

void QVFbView::sendKeyboardData( int unicode, int keycode, int modifiers,
				 bool press, bool repeat )
{
    QVFbKeyData kd;
    kd.unicode = unicode;
    kd.keycode = keycode;
    kd.modifiers = static_cast<Qt::KeyboardModifier>(modifiers);
    kd.press = press;
    kd.repeat = repeat;
    write( keyboardFd, &kd, sizeof( QVFbKeyData ) );
}

void QVFbView::flushChanges()
{
    DisplayLock();
    t_flush->start( 1000/refreshRate );
    if ( animation ) {
	    QRect r( hdr->update );
	    r = r.intersect( QRect(0, 0, hdr->width, hdr->height ) );
	    if ( r.isEmpty() ) {
		animation->appendBlankFrame();
	    } else {
		int l;
		QImage img = getBuffer( r, l );
		animation->appendFrame(img,QPoint(r.x(),r.y()));
	    }
    }
    if ( hdr->dirty ) {
	repaint();
    }
}

QImage QVFbView::getBuffer( const QRect &r, int &leading ) const
{
    switch ( viewdepth ) {
      case 12:
      case 16: {
	static unsigned char *imgData = 0;
	if ( !imgData ) {
	    int bpl = ((hdr->width*32+31)/32)*4;
	    imgData = new unsigned char [ bpl * hdr->height ];
	}
	QImage img( imgData, r.width(), r.height(), 32, 0, 0, QImage::IgnoreEndian );
	const int rsh = viewdepth == 12 ? 12 : 11;
	const int gsh = viewdepth == 12 ? 7 : 5;
	const int bsh = viewdepth == 12 ? 1 : 0;
	const int rmax = viewdepth == 12 ? 15 : 31;
	const int gmax = viewdepth == 12 ? 15 : 63;
	const int bmax = viewdepth == 12 ? 15 : 31;
	for ( int row = 0; row < r.height(); row++ ) {
	    QRgb *dptr = (QRgb*)img.scanLine( row );
	    ushort *sptr = (ushort*)(data + hdr->dataoffset + (r.y()+row)*hdr->linestep);
	    sptr += r.x();
	    for ( int col=0; col < r.width(); col++ ) {
		ushort s = *sptr++;
		*dptr++ = qRgb(qRed(gammatable[(s>>rsh)&rmax]),qGreen(gammatable[(s>>gsh)&gmax]),qBlue(gammatable[(s>>bsh)&bmax]));
		//*dptr++ = qRgb(((s>>rsh)&rmax)*255/rmax,((s>>gsh)&gmax)*255/gmax,((s>>bsh)&bmax)*255/bmax);
	    }
	}
	leading = 0;
	return img;
      }
      case 4: {
	static unsigned char *imgData = 0;
	if ( !imgData ) {
	    int bpl = ((hdr->width*8+31)/32)*4;
	    imgData = new unsigned char [ bpl * hdr->height ];
	}
	QImage img( imgData, r.width(), r.height(), 8, hdr->clut, 16,
		    QImage::IgnoreEndian );
	for ( int row = 0; row < r.height(); row++ ) {
	    unsigned char *dptr = img.scanLine( row );
	    unsigned char *sptr = data + hdr->dataoffset + (r.y()+row)*hdr->linestep;
	    sptr += r.x()/2;
	    int col = 0;
	    if ( r.x() & 1 ) {
		*dptr++ = *sptr++ >> 4;
		col++;
	    }
	    for ( ; col < r.width()-1; col+=2 ) {
		unsigned char s = *sptr++;
		*dptr++ = s & 0x0f;
		*dptr++ = s >> 4;
	    }
	    if ( !(r.right() & 1) )
		*dptr = *sptr & 0x0f;
	}
	leading = 0;
	return img;
      }
      case 24: {
        static unsigned char *imgData = 0;
        if (!imgData) {
            int bpl = hdr->width *4;
            imgData = new unsigned char[bpl * hdr->height];
        }
        QImage img(imgData, r.width(), r.height(), 32, 0, 0, QImage::IgnoreEndian);
        for (int row = 0; row < r.height(); ++row) {
            uchar *dptr = img.scanLine(row);
            uchar *sptr = data + hdr->dataoffset + (r.y() + row) * hdr->linestep;
            sptr += r.x() * 3;
            for (int col = 0; col < r.width(); ++col) {
                *dptr++ = *sptr++;
                *dptr++ = *sptr++;
                *dptr++ = *sptr++;
                dptr++;
            }
        }
        leading = 0;
        return img;
      }
      case 32: {
	leading = r.x();
	return QImage( data + hdr->dataoffset + r.y() * hdr->linestep,
		    hdr->width, r.height(), hdr->depth, 0,
		    0, QImage::LittleEndian );
      }
      case 8: {
	leading = r.x();
	return QImage( data + hdr->dataoffset + r.y() * hdr->linestep,
		    hdr->width, r.height(), hdr->depth, hdr->clut,
		    256, QImage::LittleEndian );
      }
      case 1: {
	leading = r.x();
	return QImage( data + hdr->dataoffset + r.y() * hdr->linestep,
		    hdr->width, r.height(), hdr->depth, hdr->clut,
		    0, QImage::LittleEndian );
      }
    }
    return QImage();
}

static int findMultiple(int start, double m, int limit, int step)
{
    int r = start;
    while (r != limit) {
	if ( int(int(r * m)/m) == r )
	    break;
	r += step;
    }
    return r;
}

void QVFbView::drawScreen()
{
    QPainter p( this );

//    p.translate( -contentsX(), -contentsY() );

    DisplayLock();
    QRect r( hdr->update );
    hdr->dirty = FALSE;
    hdr->update = QRect();
    //qDebug( "update %d, %d, %dx%d", r.y(), r.x(), r.width(), r.height() );
    r = r.intersect( QRect(0, 0, hdr->width, hdr->height ) );
    //qDebug( "update %d, %d, %dx%d", r.y(), r.x(), r.width(), r.height() );
    if ( !r.isEmpty() )  {
	if ( int(hzm) != hzm || int(vzm) != vzm ) {
	    r.rLeft() = findMultiple(r.left(),hzm,0,-1);
	    r.rTop() = findMultiple(r.top(),vzm,0,-1);
	    int w = findMultiple(r.width(),hzm,hdr->width,1);
	    int h = findMultiple(r.height(),vzm,hdr->height,1);
	    r.rRight() = r.left()+w-1;
	    r.rBottom() = r.top()+h-1;
	}
	int leading;
	QImage img( getBuffer( r, leading ) );
	QPixmap pm;
	if ( hzm == 1.0 && vzm == 1.0 ) {
	    pm.convertFromImage( img );
	} else if ( emulateLcdScreen && hzm == 3.0 && vzm == 3.0 ) {
	    QImage img2( img.width()*3, img.height(), 32 );
	    for ( int row = 0; row < img2.height(); row++ ) {
		QRgb *dptr = (QRgb*)img2.scanLine( row );
		QRgb *sptr = (QRgb*)img.scanLine( row );
		for ( int col = 0; col < img.width(); col++ ) {
		    QRgb s = *sptr++;
		    *dptr++ = qRgb(qRed(s),0,0);
		    *dptr++ = qRgb(0,qGreen(s),0);
		    *dptr++ = qRgb(0,0,qBlue(s));
		}
	    }
	    QWMatrix m;
	    m.scale(1.0, 3.0);
	    pm.convertFromImage( img2 );
	    pm = pm.xForm(m);
	} else if ( int(hzm) == hzm && int(vzm) == vzm ) {
	    QWMatrix m;
	    m.scale(hzm,vzm);
	    pm.convertFromImage( img );
	    pm = pm.xForm(m);
	} else {
	    pm.convertFromImage( img.smoothScale(int(img.width()*hzm),int(img.height()*vzm)) );
	}

	int x1 = r.x();
	int y1 = r.y();
	int leadingX = leading;
	int leadingY = 0;

	// Do the rotation thing
	int rotX1 = hdr->width - x1 - img.width();
	int rotY1 = hdr->height - y1 - img.height();
	int rotLeadingX = (leading) ? hdr->width - leadingX - img.width() : 0;
	int rotLeadingY = 0;
	switch ( rotation ) {
	    case Rot0:
		break;
	    case Rot90:
		leadingY = leadingX;
		leadingX = rotLeadingY;
		y1 = x1;
		x1 = rotY1;
		break;
	    case Rot180:
		leadingX = rotLeadingX;
		leadingY = leadingY;
		x1 = rotX1;
		y1 = rotY1;
		break;
	    case Rot270:
		leadingX = leadingY;
		leadingY = rotLeadingX;
		x1 = y1;
		y1 = rotX1;
		break;
	    default:
		break;
	}
	x1 = int(x1*hzm);
	y1 = int(y1*vzm);
	leadingX = int(leadingX*hzm);
	leadingY = int(leadingY*vzm);
	if ( rotation != 0 ) {
	    QWMatrix m;
	    m.rotate(rotation * 90.0);
	    pm = pm.xForm(m);
	}
	p.setPen( Qt::black );
	p.setBrush( Qt::white );
	p.drawPixmap( x1, y1, pm, leadingX, leadingY, pm.width(), pm.height() );
    }
}

//bool QVFbView::eventFilter( QObject *obj, QEvent *e )
//{
//    if ( obj == this &&
//	 (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut) )
//	return TRUE;
//
//    return QWidgetView::eventFilter( obj, e );
//}

void QVFbView::paintEvent( QPaintEvent *pe )
{
    QRect r( pe->rect() );
//    r.moveBy( contentsX(), contentsY() );
    r = QRect(int(r.x()/hzm),int(r.y()/vzm),
	    int(r.width()/hzm)+1,int(r.height()/vzm)+1);
    setDirty(r);
    drawScreen();
}

void QVFbView::setDirty( const QRect& r )
{
    DisplayLock();
    hdr->update |= mapToDevice( r, QSize(displayWidth(), displayHeight()), rotation ).normalize();
    hdr->dirty = TRUE;
}

void QVFbView::mousePressEvent( QMouseEvent *e )
{
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->stateAfter(), 0 );
}

void QVFbView::contextMenuEvent( QContextMenuEvent* )
{

}

void QVFbView::mouseDoubleClickEvent( QMouseEvent *e )
{
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->stateAfter(), 0 );
}

void QVFbView::mouseReleaseEvent( QMouseEvent *e )
{
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->stateAfter(), 0 );
}

void QVFbView::skinMouseEvent( QMouseEvent *e )
{
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->stateAfter(), 0 );
}

void QVFbView::mouseMoveEvent( QMouseEvent *e )
{
    if ( !emulateTouchscreen || (e->state() & Qt::MouseButtonMask ) )
	sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->state(), 0 );
}

void QVFbView::wheelEvent( QWheelEvent *e )
{
    if (!e)
        return;
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->buttons(), e->delta());
}

void QVFbView::setTouchscreenEmulation( bool b )
{
    emulateTouchscreen = b;
}

void QVFbView::setLcdScreenEmulation( bool b )
{
    emulateLcdScreen = b;
}

void QVFbView::keyPressEvent( QKeyEvent *e )
{
    sendKeyboardData(e->text()[0].unicode(), e->key(),
		     e->state()&(Qt::ShiftButton|Qt::ControlButton|Qt::AltButton),
		     TRUE, e->isAutoRepeat());
}

void QVFbView::keyReleaseEvent( QKeyEvent *e )
{
    sendKeyboardData(e->ascii(), e->key(),
		     e->state()&(Qt::ShiftButton|Qt::ControlButton|Qt::AltButton),
		     FALSE, e->isAutoRepeat());
}


QImage QVFbView::image() const
{
    int l;
    DisplayLock();
    QImage r = getBuffer( QRect(0, 0, hdr->width, hdr->height), l ).copy();
    return r;
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


void QVFbView::skinKeyPressEvent( int code, const QString& text, bool autorep )
{
    QKeyEvent e(QEvent::KeyPress,code,text.isEmpty() ? 0 : text[0].latin1(),0,text,autorep);
    keyPressEvent(&e);
}

void QVFbView::skinKeyReleaseEvent( int code, const QString& text, bool autorep )
{
    QKeyEvent e(QEvent::KeyRelease,code,text.isEmpty() ? 0 : text[0].latin1(),0,text,autorep);
    keyReleaseEvent(&e);
}

