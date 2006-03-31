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

#ifndef QT_NO_QWS_QVFB

#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <qvfbhdr.h>
#include <qscreenvfb_qws.h>
#include <qkbdvfb_qws.h>
#include <qmousevfb_qws.h>
#include <qwindowsystem_qws.h>
#include <qsocketnotifier.h>
#include <qapplication.h>
#include <qscreen_qws.h>

/*!
    \class QVFbScreen
    \ingroup qws

    \brief The QVFbScreen class implements a screen driver for the
    virtual framebuffer.

    Note that this class is only available in \l {Qtopia Core}.
    Custom screen drivers can be added by subclassing the
    QScreenDriverPlugin class, using the QScreenDriverFactory class to
    dynamically load the driver into the application, but there should
    only be one screen object per application.

    The Qtopia Core platform provides a \l {The Virtual
    Framebuffer}{virtual framebuffer} for development and debugging;
    the virtual framebuffer allows Qtopia Core programs to be
    developed on a desktop machine, without switching between consoles
    and X11.

    \sa QScreen, QScreenDriverPlugin, {Running Applications}
*/

/*!
    \fn bool QVFbScreen::connect(const QString & displaySpec)
    \reimp
*/

/*!
    \fn void QVFbScreen::disconnect()
    \reimp
*/

/*!
    \fn bool QVFbScreen::initDevice()
    \reimp
*/

/*!
    \fn void QVFbScreen::restore()
    \reimp
*/

/*!
    \fn void QVFbScreen::save()
    \reimp
*/

/*!
    \fn void QVFbScreen::setDirty(const QRect & r)
    \reimp
*/

/*!
    \fn void QVFbScreen::setMode(int nw, int nh, int nd)
    \reimp
*/

/*!
    \fn void QVFbScreen::shutdownDevice()
    \reimp
*/

/*!
    \variable QVFbScreen::success
    \internal
*/

/*!
    \variable QVFbScreen::shmrgn
    \internal
*/

/*!
    \variable QVFbScreen::hdr
    \internal
*/

/*!
    \fn QVFbScreen::QVFbScreen(int displayId)

    Constructs a QVNCScreen object. The \a displayId argument
    identifies the Qtopia Core server to connect to.
*/
QVFbScreen::QVFbScreen(int display_id) : QScreen(display_id)
{
    shmrgn = 0;
    hdr = 0;
    data = 0;
}

/*!
    Destroys this QVFbScreen object.
*/
QVFbScreen::~QVFbScreen()
{
}


static int QVFb_dummy;

bool QVFbScreen::connect(const QString &displaySpec)
{
    screen_optype=&QVFb_dummy;
    screen_lastop=&QVFb_dummy;

    if (displaySpec.indexOf(":Gray") >= 0)
        grayscale = true;

    key_t key = ftok(QByteArray(QT_VFB_MOUSE_PIPE).replace("%1", QByteArray::number(displayId)), 'b');

    if (key == -1)
        return false;

    int shmId = shmget(key, 0, 0);
    if (shmId != -1)
        shmrgn = (unsigned char *)shmat(shmId, 0, 0);
    else
        return false;

    if ((long)shmrgn == -1 || shmrgn == 0) {
        qDebug("No shmrgn %ld", (long)shmrgn);
        return false;
    }

    hdr = (QVFbHeader *) shmrgn;
    data = shmrgn + hdr->dataoffset;

    dw = w = hdr->width;
    dh = h = hdr->height;
    d = hdr->depth;
    lstep = hdr->linestep;

    qDebug("Connected to VFB server: %d x %d x %d", w, h, d);

    size = lstep * h;
    mapsize = size;
    screencols = hdr->numcols;
    memcpy(screenclut, hdr->clut, sizeof(QRgb) * screencols);

    if (qApp->type() == QApplication::GuiServer) {
        QString mouseDev = "QVFbMouse:";
        mouseDev += QT_VFB_MOUSE_PIPE;
        QString keyboardDev = "QVFbKbd:";
        keyboardDev += QT_VFB_KEYBOARD_PIPE;

        static char mDevBuffer[200];
        static char kDevBuffer[200];

        strncpy(mDevBuffer, mouseDev.arg(displayId).toLatin1().constData(), 200);
        strncpy(kDevBuffer, keyboardDev.arg(displayId).toLatin1().constData(), 200);

        qwsServer->setDefaultMouse(mDevBuffer);
        qwsServer->setDefaultKeyboard(kDevBuffer);

        if (hdr->dataoffset >= (int)sizeof(QVFbHeader)) {
            hdr->serverVersion = QT_VERSION;
        }
    }

    return true;
}

void QVFbScreen::disconnect()
{
    if ((long)shmrgn != -1 && shmrgn) {
        if (qApp->type() == QApplication::GuiServer && hdr->dataoffset >= (int)sizeof(QVFbHeader)) {
            hdr->serverVersion = 0;
        }
        shmdt((char*)shmrgn);
    }
}

bool QVFbScreen::initDevice()
{
    if(d==8) {
        screencols=256;
        if (grayscale) {
            // Build grayscale palette
            for(int loopc=0;loopc<256;loopc++) {
                screenclut[loopc]=qRgb(loopc,loopc,loopc);
            }
        } else {
            // 6x6x6 216 color cube
            int idx = 0;
            for(int ir = 0x0; ir <= 0xff; ir+=0x33) {
                for(int ig = 0x0; ig <= 0xff; ig+=0x33) {
                    for(int ib = 0x0; ib <= 0xff; ib+=0x33) {
                        screenclut[idx]=qRgb(ir, ig, ib);
                        idx++;
                    }
                }
            }
            screencols=idx;
        }
        memcpy(hdr->clut, screenclut, sizeof(QRgb) * screencols);
        hdr->numcols = screencols;
    } else if (d == 4) {
        int val = 0;
        for (int idx = 0; idx < 16; idx++, val += 17) {
            screenclut[idx] = qRgb(val, val, val);
        }
        screencols = 16;
        memcpy(hdr->clut, screenclut, sizeof(QRgb) * screencols);
        hdr->numcols = screencols;
    } else if (d == 1) {
        screencols = 2;
        screenclut[1] = qRgb(0xff, 0xff, 0xff);
        screenclut[0] = qRgb(0, 0, 0);
        memcpy(hdr->clut, screenclut, sizeof(QRgb) * screencols);
        hdr->numcols = screencols;
    }

#ifndef QT_NO_QWS_CURSOR
    QScreenCursor::initSoftwareCursor();
#endif
    return true;
}

void QVFbScreen::shutdownDevice()
{
}

void QVFbScreen::setMode(int ,int ,int)
{
}

// save the state of the graphics card
// This is needed so that e.g. we can restore the palette when switching
// between linux virtual consoles.
void QVFbScreen::save()
{
    // nothing to do.
}

// restore the state of the graphics card.
void QVFbScreen::restore()
{
}

#endif // QT_NO_QWS_QVFB

