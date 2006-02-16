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
#include <qwindowsystem_qws.h>
#include <qsocketnotifier.h>
#include <qapplication.h>
#include <qscreen_qws.h>

//===========================================================================

class QVFbMouseHandler : public QObject {
    Q_OBJECT
public:
    QVFbMouseHandler(int display_id);
    ~QVFbMouseHandler();

private:
    int mouseFD;
    int mouseIdx;
    enum {mouseBufSize = 128};
    uchar mouseBuf[mouseBufSize];

private slots:
    void readMouseData();
};

QVFbMouseHandler::QVFbMouseHandler(int display_id)
{
    mouseFD = -1;
    QByteArray mouseDev = QByteArray(QT_VFB_MOUSE_PIPE).replace("%1", QByteArray::number(display_id));

    if ((mouseFD = open(mouseDev, O_RDWR | O_NDELAY)) < 0) {
        qDebug("Cannot open %s (%s)", mouseDev.constData(),
                strerror(errno));
    } else {
        // Clear pending input
        char buf[2];
        while (read(mouseFD, buf, 1) > 0) { }

        mouseIdx = 0;

        QSocketNotifier *mouseNotifier;
        mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read, this);
        connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
    }
}

QVFbMouseHandler::~QVFbMouseHandler()
{
    if (mouseFD >= 0)
        close(mouseFD);
}

void QVFbMouseHandler::readMouseData()
{
    int n;
    do {
        n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx);
        if (n > 0)
            mouseIdx += n;
    } while (n > 0);

    int idx = 0;
    static const int packetsize = sizeof(QPoint) + 2*sizeof(int);
    while (mouseIdx-idx >= packetsize) {
        uchar *mb = mouseBuf+idx;
        QPoint mousePos = *reinterpret_cast<QPoint *>(mb);
        mb += sizeof(QPoint);
        int bstate = *reinterpret_cast<int *>(mb);
        mb += sizeof(int);
        int wheel = *reinterpret_cast<int *>(mb);
//        limitToScreen(mousePos);
        QWSServer::sendMouseEvent(mousePos, bstate, wheel);
        idx += packetsize;
    }

    int surplus = mouseIdx - idx;
    for (int i = 0; i < surplus; i++)
        mouseBuf[i] = mouseBuf[idx+i];
    mouseIdx = surplus;
}
//===========================================================================

class QVFbKeyboardHandler : public QObject
{
    Q_OBJECT
public:
    QVFbKeyboardHandler(int display_id);
    virtual ~QVFbKeyboardHandler();

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int kbdFD;
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
};


QVFbKeyboardHandler::QVFbKeyboardHandler(int display_id)
{
    kbdFD = -1;
    kbdIdx = 0;
    kbdBufferLen = sizeof(QVFbKeyData) * 5;
    kbdBuffer = new unsigned char [kbdBufferLen];

    QByteArray terminalName = QByteArray(QT_VFB_KEYBOARD_PIPE).replace("%1", QByteArray::number(display_id));

    if ((kbdFD = open(terminalName, O_RDWR | O_NDELAY)) < 0) {
        qDebug("Cannot open %s (%s)", terminalName.constData(),
        strerror(errno));
    } else {
        // Clear pending input
        char buf[2];
        while (read(kbdFD, buf, 1) > 0) { }

        notifier = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)),this, SLOT(readKeyboardData()));
    }
}

QVFbKeyboardHandler::~QVFbKeyboardHandler()
{
    if (kbdFD >= 0)
        close(kbdFD);
    delete [] kbdBuffer;
}


void QVFbKeyboardHandler::readKeyboardData()
{
    int n;
    do {
        n  = read(kbdFD, kbdBuffer+kbdIdx, kbdBufferLen - kbdIdx);
        if (n > 0)
            kbdIdx += n;
    } while (n > 0);

    int idx = 0;
    while (kbdIdx - idx >= (int)sizeof(QVFbKeyData)) {
        QVFbKeyData *kd = (QVFbKeyData *)(kbdBuffer + idx);
        if (kd->unicode == 0 && kd->keycode == 0 && kd->modifiers == 0 && kd->press) {
            // magic exit key
            qWarning("Instructed to quit by Virtual Keyboard");
            qApp->quit();
        }
        QWSServer::processKeyEvent(kd->unicode, kd->keycode, kd->modifiers, kd->press, kd->repeat);
        idx += sizeof(QVFbKeyData);
    }

    int surplus = kbdIdx - idx;
    for (int i = 0; i < surplus; i++)
        kbdBuffer[i] = kbdBuffer[idx+i];
    kbdIdx = surplus;
}


//===========================================================================

/*!
    \class QVFbScreen
    \ingroup qws
*/

QVFbScreen::QVFbScreen(int display_id) : QScreen(display_id)
{
    mouseHandler = 0;
    keyboardHandler = 0;
    shmrgn = 0;
    hdr = 0;
    data = 0;
}

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
        // We handle mouse and keyboard here
        QWSServer::setDefaultMouse("None");
        QWSServer::setDefaultKeyboard("None");
        mouseHandler = new QVFbMouseHandler(displayId);
        keyboardHandler = new QVFbKeyboardHandler(displayId);
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
    if (qApp->type() == QApplication::GuiServer) {
        delete mouseHandler;
        mouseHandler = 0;
        delete keyboardHandler;
        keyboardHandler = 0;
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
#include "qscreenvfb_qws.moc"

#endif // QT_NO_QWS_QVFB

