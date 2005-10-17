#include "qvfbviewiface.h"

#include "qvfbhdr.h"

#include <QImage>

#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

QVFbViewIface::QVFbViewIface(int display_id, int w, int h, int d, Rotation r)
    : m_displayId(display_id), m_width(w), m_height(h), m_viewDepth(d),
      m_rotation(r), m_emulateTouchscreen(false), m_emulateLcdScreen(false)
{
    m_hzm = 1;
    m_vzm = 1;
    m_hdr = 0;

    m_mousePipe = QString(QT_VFB_MOUSE_PIPE).arg(display_id);
    m_keyboardPipe = QString(QT_VFB_KEYBOARD_PIPE).arg(display_id);

    unlink(m_mousePipe.local8Bit().data());
    mkfifo(m_mousePipe.local8Bit().data(), 0666);

    m_mouseFd = ::open(m_mousePipe.local8Bit().data(),
                       O_RDWR | O_NDELAY);
    if (m_mouseFd == -1) {
	qFatal("Cannot open mouse pipe %s", m_mousePipe.latin1());
    }

    unlink(m_keyboardPipe);
    mkfifo(m_keyboardPipe, 0666);
    m_keyboardFd = ::open(m_keyboardPipe.local8Bit().data(),
                          O_RDWR | O_NDELAY);
    if (m_keyboardFd == -1) {
	::close(m_mouseFd);
	qFatal("Cannot open keyboard pipe %s",
               m_keyboardPipe.latin1());
    }

}

QVFbViewIface::~QVFbViewIface()
{
    sendKeyboardData(0, 0, 0, true, false); // magic die key
    ::close(m_mouseFd);
    ::close(m_keyboardFd);
    unlink(m_mousePipe);
    unlink(m_keyboardPipe);
}

bool QVFbViewIface::setTouchscreenEmulation(bool b)
{
    m_emulateTouchscreen = b;
    return true;
}


bool QVFbViewIface::setLcdScreenEmulation(bool b)
{
    m_emulateLcdScreen = b;
    return false;
}


bool QVFbViewIface::setRate(int r)
{
    m_refreshRate = r;
    return true;
}


bool QVFbViewIface::setZoom(double hz, double vz)
{
    if (m_hzm != hz || m_vzm != vz) {
	m_hzm = hz;
	m_vzm = vz;
        return true;
    }
    return false;
}

void QVFbViewIface::setGamma(double gr, double gg, double gb)
{
    if (m_viewDepth < 12)
	return; // not implemented

    int rsh, gsh, bsh, gmax, bmax;

    m_gred=gr; m_ggreen=gg; m_gblue=gb;

    switch (m_viewDepth) {
    case 12:
	rsh = 12;
	gsh = 7;
	bsh = 1;
	m_rmax = 15;
	gmax = 15;
	bmax = 15;
	break;
    case 16:
	rsh = 11;
	gsh = 5;
	bsh = 0;
	m_rmax = 31;
	gmax = 63;
	bmax = 31;
	break;
    case 24:
    case 32:
	rsh = 16;
	gsh = 8;
	bsh = 0;
	m_rmax = 255;
	gmax = 255;
	bmax = 255;
    }
    int mm = qMax(m_rmax,qMax(gmax,bmax))+1;
    if (m_gammatable)
	delete [] m_gammatable;
    m_gammatable = new QRgb[mm];
    for (int i=0; i<mm; i++) {
	int r = int(pow(i,gr)*255/m_rmax);
	int g = int(pow(i,gg)*255/gmax);
	int b = int(pow(i,gb)*255/bmax);
	if ( r > 255 ) r = 255;
	if ( g > 255 ) g = 255;
	if ( b > 255 ) b = 255;
	m_gammatable[i] = qRgb(r,g,b);
        //qDebug("%d: %d,%d,%d",i,r,g,b);
    }

}

void QVFbViewIface::getGamma(int i, QRgb& rgb)
{
    if ( i > 255 ) i = 255;
    if ( i < 0 ) i = 0;
    rgb = qRgb(qRed(m_gammatable[i*m_rmax/255]),
               qGreen(m_gammatable[i*m_rmax/255]),
               qBlue(m_gammatable[i*m_rmax/255]));
}

void QVFbViewIface::sendMouseData(const QPoint &pos, int buttons, int wheel)
{
    QPoint p = mapToDevice(QRect(pos,QSize(1,1)),
                           QSize(displayWidth(), displayHeight()), m_rotation).topLeft();
    write(m_mouseFd, &p, sizeof(QPoint));
    write(m_mouseFd, &buttons, sizeof(int));
    if (m_hdr && m_hdr->serverVersion >= 0x040000) {
        write(m_mouseFd, &wheel, sizeof(int));
    }
}

void QVFbViewIface::sendKeyboardData(int unicode, int keycode, int modifiers,
                                     bool press, bool repeat)
{
    QVFbKeyData kd;
    kd.unicode = unicode;
    kd.keycode = keycode;
    kd.modifiers = static_cast<Qt::KeyboardModifier>(modifiers);
    kd.press = press;
    kd.repeat = repeat;
    write(m_keyboardFd, &kd, sizeof(QVFbKeyData));
}

QRect QVFbViewIface::mapToDevice(const QRect &r, const QSize &s, QVFbViewIface::Rotation rotation)
{
    int x1 = r.x(), y1 = r.y(), x2 = r.right(), y2 = r.bottom(), w = s.width(), h = s.height();
    switch (rotation) {
	case QVFbViewIface::Rot90:
	    return QRect(QPoint(y1, w - x1 - 1), QPoint(y2, w - x2 - 1));
	case QVFbViewIface::Rot180:
	    return QRect(QPoint(w - x1 - 1, h - y1 - 1), QPoint(w - x2 - 1, h - y2 - 1));
	case QVFbViewIface::Rot270:
	    return QRect(QPoint(h - y1 - 1, x1), QPoint(h - y2 - 1, x2));
	default:
	    break;
    }
    return r;
}
