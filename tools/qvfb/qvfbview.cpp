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

#include "qglobal.h"
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
#include <qimage.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qwmatrix.h>
#include <qpainter.h>
#include "qanimationwriter.h"
#include <qevent.h>
#include <qfile.h>

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

//#define QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS

#ifndef Q_WS_QWS
// Get the name of the directory where Qt/Embedded temporary data should
// live.
static QString qws_dataDir()
{
    QString username = "unknown";
    const char *logname = getenv("LOGNAME");
    if ( logname )
        username = logname;

    QString dataDir = "/tmp/qtembedded-" + username;
    if ( mkdir( dataDir.latin1(), 0700 ) ) {
        if ( errno != EEXIST ) {
            qFatal( QString("Cannot create Qt/Embedded data directory: %1")
                    .arg( dataDir ) );
        }
    }

    struct stat buf;
    if ( lstat( dataDir.latin1(), &buf ) )
        qFatal( QString( "stat failed for Qt/Embedded data directory: %1" )
                .arg( dataDir ) );

    if ( !S_ISDIR( buf.st_mode ) )
        qFatal( QString( "%1 is not a directory" ).arg( dataDir ) );

    if ( buf.st_uid != getuid() )
        qFatal( QString( "Qt/Embedded data directory is not owned by user %1" )
                .arg( getuid() ) );

    if ( (buf.st_mode & 0677) != 0600 )
        qFatal( QString( "Qt/Embedded data directory has incorrect permissions: %1" )
                .arg( dataDir ) );

    dataDir += "/";

    return dataDir;
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


QVFbView::QVFbView( int display_id, int w, int h, int d, QWidget *parent,
                    Qt::WFlags flags )
    : QWidget( parent, flags ), emulateTouchscreen(false), qwslock(NULL)
{
    displayid = display_id;
    setMouseTracking( true );
    setFocusPolicy( Qt::StrongFocus );
    zm = 1;
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

    mousePipe = QString(QT_VFB_MOUSE_PIPE).arg(display_id);
    keyboardPipe = QString(QT_VFB_KEYBOARD_PIPE).arg(display_id);

    unlink( mousePipe.latin1() );
    mkfifo( mousePipe.latin1(), 0666 );

    mouseFd = open( mousePipe.latin1(), O_RDWR | O_NDELAY );
    if ( mouseFd == -1 ) {
        qFatal( "Cannot open mouse pipe" );
    }

    unlink( keyboardPipe );
    mkfifo( keyboardPipe, 0666 );
    keyboardFd = open( keyboardPipe, O_RDWR | O_NDELAY );
    if ( keyboardFd == -1 ) {
        qFatal( "Cannot open keyboard pipe" );
    }

    key_t key = ftok( mousePipe.latin1(), 'b' );

    int bpl;
    if ( d == 1 )
        bpl = (w*d+7)/8;
    else
        bpl = ((w*actualdepth+31)/32)*4;

    int dataSize = bpl * h + sizeof( QVFbHeader );
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
    viewdepth = d;
    hdr->depth = actualdepth;
    hdr->linestep = bpl;
    hdr->numcols = 0;
    hdr->dataoffset = sizeof( QVFbHeader );
    hdr->update = QRect();

    contentsWidth = w;
    contentsHeight = h;

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );

    gammatable=0;
    setGamma(1.0,1.0,1.0);
    setRate( 30 );
    resize(contentsWidth, contentsHeight);
    displayPipe = qws_dataDir() + QString( QTE_PIPE ).arg( displayid );
    displayPiped = displayPipe + 'd';
}

QVFbView::~QVFbView()
{
    stopAnimation();
    sendKeyboardData( 0, 0, 0, true, false ); // magic die key
    delete qwslock;
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
    int mm = qMax(rmax,qMax(gmax,bmax))+1;
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
    return hdr->width;
}

int QVFbView::displayHeight() const
{
    return hdr->height;
}

int QVFbView::displayDepth() const
{
    return viewdepth;
}

void QVFbView::setZoom( double z )
{
    if ( zm != z ) {
        zm = z;
        setDirty(QRect(0,0,hdr->width,hdr->height));
        contentsWidth = int(hdr->width*z);
        contentsHeight = int(hdr->height*z);
        resize(contentsWidth, contentsHeight);
////        window()->adjustSize();
        drawScreen();
    }
}

void QVFbView::setRate( int r )
{
    refreshRate = r;
    timer->start( 1000/r );
}

void QVFbView::sendMouseData( const QPoint &pos, int buttons, int wheel )
{
    write( mouseFd, &pos, sizeof( QPoint ) );
    write( mouseFd, &buttons, sizeof( int ) );
    if (hdr->serverVersion >= 0x040000) {
        write( mouseFd, &wheel, sizeof( int ) );
    }
}

void QVFbView::sendKeyboardData( int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                 bool press, bool repeat )
{
    QVFbKeyData kd;
    kd.unicode = unicode;
    kd.keycode = keycode;
    kd.modifiers = modifiers;
    kd.press = press;
    kd.repeat = repeat;
    write( keyboardFd, &kd, sizeof( QVFbKeyData ) );
}

void QVFbView::timeout()
{
    DisplayLock(); // Autolock display;
    if ( animation ) {
#ifndef NO_QVFB_ANIMATION
        QRect r( hdr->update );
        r = r.intersect( QRect(0, 0, hdr->width, hdr->height ) );
        if ( r.isEmpty() ) {
                animation->appendBlankFrame();
            } else {
                int l;
                QImage img = getBuffer( r, l );
                animation->appendFrame(img,QPoint(r.x(),r.y()));
            }
#endif
    }

    if ( hdr->dirty ) {
        QRect r(hdr->update);
        r = QRect(int(r.x()*zm),int(r.y()*zm),
                  int(r.width()*zm)+1,int(r.height()*zm)+1);
        repaint(r);
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
#ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
            for ( int col=0; col < r.width()/2; col++ ) {
#else
            for ( int col=0; col < r.width(); col++ ) {
#endif
                ushort s = *sptr++;
#ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
                ushort s2 = *sptr++;
                *dptr++ = qRgb(qRed(gammatable[(s2>>rsh)&rmax]),qGreen(gammatable[(s2>>gsh)&gmax]),qBlue(gammatable[(s2>>bsh)&bmax]));
#endif
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
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if ( r.x() & 1 ) {
                *dptr++ = *sptr++ & 0x0f;
                col++;
            }
            for ( ; col < r.width()-1; col+=2 ) {
                unsigned char s = *sptr++;
                *dptr++ = s >> 4;
                *dptr++ = s & 0x0f;
            }
            if ( !(r.right() & 1) )
                *dptr = *sptr >> 4;
#else
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
#endif
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
      case 32:{
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
#ifndef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                    0, QImage::LittleEndian );
#else
                    0, QImage::BigEndian );
#endif
      }
    }
    return QImage();
}

void QVFbView::drawScreen()
{
    QPainter p(this);
    DisplayLock(); // Autolock display;
    QRect r( hdr->update );
    hdr->dirty = false;
    hdr->update = QRect();
    // qDebug( "update %d, %d, %dx%d", r.y(), r.x(), r.width(), r.height() );
    r = r.intersect( QRect(0, 0, hdr->width, hdr->height ) );
    if ( !r.isEmpty() )  {
        if ( int(zm) != zm ) {
            r.rLeft() = int(int(r.left()*zm)/zm);
            r.rTop() = int(int(r.top()*zm)/zm);
            r.rRight() = int(int(r.right()*zm+zm+0.0000001)/zm+1.9999);
            r.rBottom() = int(int(r.bottom()*zm+zm+0.0000001)/zm+1.9999);
            r.rRight() = qMin(r.right(),hdr->width-1);
            r.rBottom() = qMin(r.bottom(),hdr->height-1);
        }
        int leading;
        QImage img( getBuffer( r, leading ) );
        QPixmap pm;
        if ( zm == 1 ) {
            pm.convertFromImage( img );
        } else if ( int(zm) == zm ) {
            QWMatrix m;
            m.scale(zm,zm);
            pm.convertFromImage( img );
            pm = pm.xForm(m);
        } else {
            pm.convertFromImage( img.smoothScale(int(img.width()*zm),int(img.height()*zm)) );
        }
        p.setPen( Qt::black );
        p.setBrush( Qt::white );
        p.drawPixmap( int(r.x()*zm), int(r.y()*zm), pm,
                      int(leading*zm), 0, pm.width()-int(leading*zm), pm.height() );
    }
}


void QVFbView::paintEvent( QPaintEvent *pe )
{
    QRect r( pe->rect() );
    r = QRect(int(r.x()/zm),int(r.y()/zm),
            int(r.width()/zm)+1,int(r.height()/zm)+1);
    setDirty(r);
    drawScreen();
}

void QVFbView::setDirty( const QRect& r )
{
    DisplayLock(); // Autolock display;
    hdr->update |= r;
    hdr->dirty = true;
}

void QVFbView::mousePressEvent( QMouseEvent *e )
{
    sendMouseData( e->pos()/zm, e->stateAfter(), 0 );
}

void QVFbView::contextMenuEvent( QContextMenuEvent *e )
{
}

void QVFbView::mouseDoubleClickEvent( QMouseEvent *e )
{
    sendMouseData( e->pos()/zm, e->stateAfter(), 0 );
}

void QVFbView::mouseReleaseEvent( QMouseEvent *e )
{
    sendMouseData( e->pos()/zm, e->stateAfter(), 0 );
}

void QVFbView::mouseMoveEvent( QMouseEvent *e )
{
    if ( !emulateTouchscreen || (e->state() & Qt::MouseButtonMask ) )
        sendMouseData( e->pos()/zm, e->state(), 0 );
}

void QVFbView::wheelEvent( QWheelEvent *e )
{
    if (!e)
        return;
    sendMouseData(e->pos()/zm, e->buttons(), e->delta());
}

void QVFbView::keyPressEvent( QKeyEvent *e )
{
    sendKeyboardData(e->text()[0].unicode(), e->key(),
                     e->modifiers() & (Qt::ShiftModifier|Qt::ControlModifier|Qt::AltModifier),
                     true, e->isAutoRepeat());
}

void QVFbView::keyReleaseEvent( QKeyEvent *e )
{
    sendKeyboardData(/*#### e->ascii()*/0, e->key(),
                     e->modifiers() & (Qt::ShiftModifier|Qt::ControlModifier|Qt::AltModifier),
                     false, e->isAutoRepeat());
}


QImage QVFbView::image() const
{
    int l;
    DisplayLock(); // Autolock display;
    QImage r = getBuffer( QRect(0, 0, hdr->width, hdr->height), l ).copy();
    return r;
}

void QVFbView::startAnimation( const QString& /*filename*/ )
{
#ifndef NO_QVFB_ANIMATION
    delete animation;
    animation = new QAnimationWriter(filename,"MNG");
    animation->setFrameRate(refreshRate);
    animation->appendFrame(QImage(data + hdr->dataoffset,
                hdr->width, hdr->height, hdr->depth, hdr->clut,
                256, QImage::LittleEndian));
#endif
}

void QVFbView::stopAnimation()
{
#ifndef NO_QVFB_ANIMATION
    delete animation;
    animation = 0;
#endif
}

void QVFbView::setTouchscreenEmulation( bool b )
{
    emulateTouchscreen = b;
}
