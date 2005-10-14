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
#define QLock QWSSemaphoreGL
#undef QT_NO_QWS_MULTIPROCESS
#include "../../src/gui/embedded/qlock.cpp"
#else
#include "qlock_p.h"
#endif

#include "qglvfbview.h"
#include "qvfbhdr.h"

#define QTE_PIPE "QtEmbedded-%1"

#include "qanimationwriter.h"
#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QBitmap>
#include <QTimer>
#include <QWMatrix>
#include <QPaintEvent>
#include <QScrollArea>
#include <QFile>

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

QGLVFbView::QGLVFbView(int display_id, int _w, int _h, int d, Rotation r,
                       QWidget *parent)
    : QGLWidget(parent), QVFbViewIface(display_id, _w, _h, d, r),
      lockId(-1)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setBackgroundMode(Qt::NoBackground);
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

    int w = ( m_rotation & 0x1 ) ? _h : _w;
    int h = ( m_rotation & 0x1 ) ? _w : _h;

    QString username = "unknown";
    const char *logname = getenv("LOGNAME");
    if ( logname )
        username = logname;

    QString oldPipe = "/tmp/qtembedded-" + username + "/" + QString(QTE_PIPE).arg(m_displayId);
    int oldPipeSemkey = ftok(oldPipe.latin1(), 'd');
    int oldPipeLockId = semget(oldPipeSemkey, 0, 0);
    if (oldPipeLockId >= 0){
        sembuf sops;
        sops.sem_num = 0;
        sops.sem_op = 1;
        sops.sem_flg = SEM_UNDO;
        int rv;
        do {
            rv = semop(lockId,&sops,1);
        } while ( rv == -1 && errno == EINTR );
        qFatal("Cannot create lock file as an old version of QVFb has opened %s. Close other QVFb and try again",
               oldPipe.latin1());
    }

    int bpl;
    if ( d == 1 )
	bpl = (w*d+7)/8;
    else
	bpl = ((w*actualdepth+31)/32)*4;

    key_t key = ftok(m_mousePipe.latin1(), 'b');
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
	::close(m_mouseFd);
	::close(m_keyboardFd);
	qFatal( "Cannot attach to shared memory %d",shmId );
    }

    m_hdr = (QVFbHeader *)data;
    m_hdr->width = w;
    m_hdr->height = h;
    viewdepth = d;
    m_hdr->depth = actualdepth;
    m_hdr->linestep = bpl;
    m_hdr->numcols = 0;
    m_hdr->dataoffset = sizeof(QVFbHeader);
    m_hdr->update = QRect();

    contentsWidth = _w;
    contentsHeight = _h;

    resize(_w, _h);

    t_flush = new QTimer(this);
    connect(t_flush, SIGNAL(timeout()), this, SLOT(flushChanges()));

    m_gammatable=0;
    setGamma(1.0,1.0,1.0);
    setRate( 30 );

#ifdef Q_WS_QWS
    displayPipe = qws_dataDir() + QString(QTE_PIPE).arg(m_displayId);
#else
    displayPipe = qws_dataDir(m_displayId) + QString(QTE_PIPE).arg(m_displayId);
#endif
    displayPiped = displayPipe + 'd';
}

QGLVFbView::~QGLVFbView()
{
    stopAnimation();
    struct shmid_ds shm;
    shmdt((char*)data);
    shmctl(shmId, IPC_RMID, &shm);
}

QSize QGLVFbView::sizeHint() const
{
    return QSize(contentsWidth, contentsHeight);
}

void QGLVFbView::setGamma(double gr, double gg, double gb)
{
    QVFbViewIface::setGamma(gr, gg, gb);
    setDirty(rect());
}

bool QGLVFbView::setZoom(double hz, double vz)
{
    if (QVFbViewIface::setZoom(hz, vz)) {
	setDirty(QRect(0,0,displayWidth(),displayHeight()));
        contentsWidth = int(m_hdr->width*hz);
        contentsHeight = int(m_hdr->height*vz);
        resize(contentsWidth, contentsHeight);
	updateGeometry();
	qApp->sendPostedEvents();
	topLevelWidget()->adjustSize();
	update();
        return true;
    }
    return false;
}

bool QGLVFbView::setRate(int r)
{
    QVFbViewIface::setRate(r);
    t_flush->start(1000/r);
    return true;
}

void QGLVFbView::flushChanges()
{
    DisplayLock();
    t_flush->start(1000/m_refreshRate);
    if (animation) {
        QRect r(m_hdr->update);
        r = r.intersect(QRect(0, 0, m_hdr->width, m_hdr->height));
        if (r.isEmpty()) {
            animation->appendBlankFrame();
        } else {
            int l;
            QImage img = getBuffer(r, l);
            animation->appendFrame(img,QPoint(r.x(),r.y()));
        }
    }
    if (m_hdr->dirty) {
	repaint();
    }
}

QImage QGLVFbView::getBuffer(const QRect &r, int &leading) const
{
    switch (viewdepth) {
      case 12:
      case 16: {
	static unsigned char *imgData = 0;
	if (!imgData) {
	    int bpl = ((m_hdr->width*32+31)/32)*4;
	    imgData = new unsigned char [ bpl * m_hdr->height ];
	}
	QImage img(imgData, r.width(), r.height(), 32, 0, 0, QImage::IgnoreEndian);
	const int rsh = viewdepth == 12 ? 12 : 11;
	const int gsh = viewdepth == 12 ? 7 : 5;
	const int bsh = viewdepth == 12 ? 1 : 0;
	const int rmax = viewdepth == 12 ? 15 : 31;
	const int gmax = viewdepth == 12 ? 15 : 63;
	const int bmax = viewdepth == 12 ? 15 : 31;
	for ( int row = 0; row < r.height(); row++ ) {
	    QRgb *dptr = (QRgb*)img.scanLine( row );
	    ushort *sptr = (ushort*)(data + m_hdr->dataoffset + (r.y()+row)*m_hdr->linestep);
	    sptr += r.x();
	    for ( int col=0; col < r.width(); col++ ) {
		ushort s = *sptr++;
		*dptr++ = qRgb(qRed(m_gammatable[(s>>rsh)&rmax]),
                               qGreen(m_gammatable[(s>>gsh)&gmax]),
                               qBlue(m_gammatable[(s>>bsh)&bmax]));
		//*dptr++ = qRgb(((s>>rsh)&rmax)*255/rmax,((s>>gsh)&gmax)*255/gmax,((s>>bsh)&bmax)*255/bmax);
	    }
	}
	leading = 0;
	return img;
      }
      case 4: {
	static unsigned char *imgData = 0;
	if ( !imgData ) {
	    int bpl = ((m_hdr->width*8+31)/32)*4;
	    imgData = new unsigned char [ bpl * m_hdr->height ];
	}
	QImage img( imgData, r.width(), r.height(), 8, m_hdr->clut, 16,
		    QImage::IgnoreEndian );
	for ( int row = 0; row < r.height(); row++ ) {
	    unsigned char *dptr = img.scanLine( row );
	    unsigned char *sptr = data + m_hdr->dataoffset + (r.y()+row)*m_hdr->linestep;
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
            int bpl = m_hdr->width *4;
            imgData = new unsigned char[bpl * m_hdr->height];
        }
        QImage img(imgData, r.width(), r.height(), 32, 0, 0, QImage::IgnoreEndian);
        for (int row = 0; row < r.height(); ++row) {
            uchar *dptr = img.scanLine(row);
            uchar *sptr = data + m_hdr->dataoffset + (r.y() + row) * m_hdr->linestep;
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
	return QImage( data + m_hdr->dataoffset + r.y() * m_hdr->linestep,
		    m_hdr->width, r.height(), m_hdr->depth, 0,
		    0, QImage::LittleEndian );
      }
      case 8: {
	leading = r.x();
	return QImage( data + m_hdr->dataoffset + r.y() * m_hdr->linestep,
		    m_hdr->width, r.height(), m_hdr->depth, m_hdr->clut,
		    256, QImage::LittleEndian );
      }
      case 1: {
	leading = r.x();
	return QImage( data + m_hdr->dataoffset + r.y() * m_hdr->linestep,
		    m_hdr->width, r.height(), m_hdr->depth, m_hdr->clut,
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

void QGLVFbView::drawScreen()
{
    QPainter p( this );

//    p.translate( -contentsX(), -contentsY() );

    DisplayLock();
    QRect r( m_hdr->update );
    m_hdr->dirty = false;
    m_hdr->update = QRect();
    //qDebug( "update %d, %d, %dx%d", r.y(), r.x(), r.width(), r.height() );
    r = r.intersect( QRect(0, 0, m_hdr->width, m_hdr->height ) );
    //qDebug( "update %d, %d, %dx%d", r.y(), r.x(), r.width(), r.height() );
    if ( !r.isEmpty() )  {
	if ( int(m_hzm) != m_hzm || int(m_vzm) != m_vzm ) {
	    r.rLeft() = findMultiple(r.left(),m_hzm,0,-1);
	    r.rTop() = findMultiple(r.top(),m_vzm,0,-1);
	    int w = findMultiple(r.width(), m_hzm,m_hdr->width,1);
	    int h = findMultiple(r.height(), m_vzm,m_hdr->height,1);
	    r.rRight() = r.left()+w-1;
	    r.rBottom() = r.top()+h-1;
	}
	int leading;
	QImage img(getBuffer(r, leading));
	QPixmap pm;
	if (m_hzm == 1.0 && m_vzm == 1.0) {
	    pm.convertFromImage(img);
	} else if (m_emulateLcdScreen && m_hzm == 3.0 && m_vzm == 3.0) {
	    QImage img2(img.width()*3, img.height(), 32);
	    for (int row = 0; row < img2.height(); ++row) {
		QRgb *dptr = (QRgb*)img2.scanLine(row);
		QRgb *sptr = (QRgb*)img.scanLine(row);
		for (int col = 0; col < img.width(); col++) {
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
	} else if ( int(m_hzm) == m_hzm && int(m_vzm) == m_vzm ) {
	    QWMatrix m;
	    m.scale(m_hzm, m_vzm);
	    pm.convertFromImage(img);
	    pm = pm.xForm(m);
	} else {
	    pm.convertFromImage(img.smoothScale(int(img.width()*m_hzm),
                                                int(img.height()*m_vzm)));
	}

	int x1 = r.x();
	int y1 = r.y();
	int leadingX = leading;
	int leadingY = 0;

	// Do the rotation thing
	int rotX1 = m_hdr->width - x1 - img.width();
	int rotY1 = m_hdr->height - y1 - img.height();
	int rotLeadingX = (leading) ? m_hdr->width - leadingX - img.width() : 0;
	int rotLeadingY = 0;
	switch (m_rotation) {
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
	x1 = int(x1*m_hzm);
	y1 = int(y1*m_vzm);
	leadingX = int(leadingX*m_hzm);
	leadingY = int(leadingY*m_vzm);
	if (m_rotation != 0) {
	    QWMatrix m;
	    m.rotate(m_rotation * 90.0);
	    pm = pm.xForm(m);
	}
	p.setPen(Qt::black);
	p.setBrush(Qt::white);
	p.drawPixmap(x1, y1, pm, leadingX, leadingY,
                     pm.width(), pm.height());
    }
}

void QGLVFbView::paintEvent(QPaintEvent *pe)
{
    QRect r(pe->rect());
//    r.moveBy( contentsX(), contentsY() );
    r = QRect(int(r.x()/m_hzm),int(r.y()/m_vzm),
              int(r.width()/m_hzm)+1,int(r.height()/m_vzm)+1);
    setDirty(r);
    drawScreen();
}

void QGLVFbView::setDirty(const QRect &r)
{
    DisplayLock();
    m_hdr->update |= mapToDevice(r, QSize(displayWidth(), displayHeight()),
                               m_rotation).normalize();
    m_hdr->dirty = true;
}

void QGLVFbView::mousePressEvent(QMouseEvent *e)
{
    sendMouseData(QPoint(int(e->x()/m_hzm),int(e->y()/m_vzm)), e->stateAfter(), 0 );
}

void QGLVFbView::contextMenuEvent(QContextMenuEvent *)
{

}

void QGLVFbView::mouseDoubleClickEvent(QMouseEvent *e)
{
    sendMouseData(QPoint(int(e->x()/m_hzm),int(e->y()/m_vzm)),
                  e->stateAfter(), 0);
}

void QGLVFbView::mouseReleaseEvent(QMouseEvent *e)
{
    sendMouseData(QPoint(int(e->x()/m_hzm),int(e->y()/m_vzm)), e->stateAfter(), 0);
}

void QGLVFbView::skinMouseEvent(QMouseEvent *e)
{
    sendMouseData(QPoint(int(e->x()/m_hzm),int(e->y()/m_vzm)), e->stateAfter(), 0);
}

void QGLVFbView::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_emulateTouchscreen || (e->state() & Qt::MouseButtonMask))
	sendMouseData(QPoint(int(e->x()/m_hzm),int(e->y()/m_vzm)), e->state(), 0);
}

void QGLVFbView::wheelEvent(QWheelEvent *e)
{
    if (!e)
        return;
    sendMouseData(QPoint(int(e->x()/m_hzm),int(e->y()/m_vzm)),
                  e->buttons(), e->delta());
}

void QGLVFbView::keyPressEvent( QKeyEvent *e )
{
    sendKeyboardData(e->text()[0].unicode(), e->key(),
		     e->state()&(Qt::ShiftButton|Qt::ControlButton|Qt::AltButton),
		     true, e->isAutoRepeat());
}

void QGLVFbView::keyReleaseEvent( QKeyEvent *e )
{
    sendKeyboardData(e->ascii(), e->key(),
		     e->state()&(Qt::ShiftButton|Qt::ControlButton|Qt::AltButton),
		     false, e->isAutoRepeat());
}


QImage QGLVFbView::image() const
{
    int l;
    DisplayLock();
    QImage r = getBuffer( QRect(0, 0, m_hdr->width, m_hdr->height), l ).copy();
    return r;
}

void QGLVFbView::startAnimation(const QString &filename)
{
    delete animation;
    animation = new QAnimationWriter(filename,"MNG");
    animation->setFrameRate(m_refreshRate);
    animation->appendFrame(QImage(data + m_hdr->dataoffset,
                m_hdr->width, m_hdr->height, m_hdr->depth, m_hdr->clut,
                256, QImage::LittleEndian));
}

void QGLVFbView::stopAnimation()
{
    delete animation;
    animation = 0;
}


void QGLVFbView::skinKeyPressEvent( int code, const QString& text, bool autorep )
{
    QKeyEvent e(QEvent::KeyPress,code,text.isEmpty() ? 0 : text[0].latin1(),0,text,autorep);
    keyPressEvent(&e);
}

void QGLVFbView::skinKeyReleaseEvent( int code, const QString& text, bool autorep )
{
    QKeyEvent e(QEvent::KeyRelease,code,text.isEmpty() ? 0 : text[0].latin1(),0,text,autorep);
    keyReleaseEvent(&e);
}
