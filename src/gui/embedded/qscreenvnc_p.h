/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCREENVNC_P_H
#define QSCREENVNC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include "qscreenvnc_qws.h"

#ifndef QT_NO_QWS_VNC

#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qtcpserver.h>
#include <private/qsharedmemory_p.h>

#ifndef QT_NO_QWS_CURSOR
class QVNCCursor : public QScreenCursor
{
public:
    QVNCCursor(QVNCScreen *s) : screen(s) { hwaccel = true; }
    ~QVNCCursor() {}

    void hide();
    void show();
    void set(const QImage &image, int hotx, int hoty);
    void move(int x, int y);

private:
    QVNCScreen *screen;
};
#endif // QT_NO_QWS_CURSOR

#define MAP_TILE_SIZE 16
#define MAP_WIDTH 1280 / MAP_TILE_SIZE
#define MAP_HEIGHT 1024 / MAP_TILE_SIZE

struct QVNCHeader
{
    bool dirty;
    uchar map[MAP_HEIGHT][MAP_WIDTH];
};

class QRfbRect
{
public:
    QRfbRect() {}
    QRfbRect(quint16 _x, quint16 _y, quint16 _w, quint16 _h) {
        x = _x; y = _y; w = _w; h = _h;
    }

    void read(QTcpSocket *s);
    void write(QTcpSocket *s);

    quint16 x;
    quint16 y;
    quint16 w;
    quint16 h;
};

class QRfbPixelFormat
{
public:
    static int size() { return 16; }

    void read(QTcpSocket *s);
    void write(QTcpSocket *s);

    int bitsPerPixel;
    int depth;
    bool bigEndian;
    bool trueColor;
    int redBits;
    int greenBits;
    int blueBits;
    int redShift;
    int greenShift;
    int blueShift;
};

class QRfbServerInit
{
public:
    QRfbServerInit() { name = 0; }
    ~QRfbServerInit() { delete[] name; }

    int size() const { return QRfbPixelFormat::size() + 8 + strlen(name); }
    void setName(const char *n);

    void read(QTcpSocket *s);
    void write(QTcpSocket *s);

    quint16 width;
    quint16 height;
    QRfbPixelFormat format;
    char *name;
};

class QRfbSetEncodings
{
public:
    bool read(QTcpSocket *s);

    quint16 count;
};

class QRfbFrameBufferUpdateRequest
{
public:
    bool read(QTcpSocket *s);

    char incremental;
    QRfbRect rect;
};

class QRfbKeyEvent
{
public:
    bool read(QTcpSocket *s);

    char down;
    int  keycode;
    int  unicode;
};

class QRfbPointerEvent
{
public:
    bool read(QTcpSocket *s);

    uint buttons;
    quint16 x;
    quint16 y;
};

class QRfbClientCutText
{
public:
    bool read(QTcpSocket *s);

    quint32 length;
};

class QVNCServer;

class QVNCScreenPrivate : public QObject
{
public:
    QVNCScreenPrivate(QVNCScreen *parent);
    ~QVNCScreenPrivate();

    void configure();

    bool doOnScreenSurface;
    int refreshRate;
    QVNCHeader hdr;
    QVNCServer *vncServer;
    QScreen *subscreen;

#ifndef QT_NO_QWS_MULTIPROCESS
    QSharedMemory *shm;
#endif

    QVNCScreen *q_ptr;
};

class QVNCServer : public QObject
{
    Q_OBJECT
public:
    QVNCServer(QVNCScreen *screen);
    QVNCServer(QVNCScreen *screen, int id);
    ~QVNCServer();

    void setRefreshRate(int rate) { refreshRate = rate; }

    enum ClientMsg { SetPixelFormat = 0,
                     FixColourMapEntries = 1,
                     SetEncodings = 2,
                     FramebufferUpdateRequest = 3,
                     KeyEvent = 4,
                     PointerEvent = 5,
                     ClientCutText = 6 };

    enum ServerMsg { FramebufferUpdate = 0,
                     SetColourMapEntries = 1 };

private:
    void setPixelFormat();
    void setEncodings();
    void frameBufferUpdateRequest();
    void pointerEvent();
    void keyEvent();
    void clientCutText();
    bool checkFill(const uchar *data, int width, int height, int stride);
    void convertPixels(char *dst, const char *src, int count);
    void sendHextile();
    void sendRaw();
    inline QVNCHeader* header() { return &qvnc_screen->d_ptr->hdr; }

private slots:
    void newConnection();
    void readClient();
    void checkUpdate();
    void discardClient();

private:
    void init(uint port);
    enum ClientState { Protocol, Init, Connected };
    QTimer *timer;
    QTcpServer *serverSocket;
    QTcpSocket *client;
    ClientState state;
    quint8 msgType;
    bool handleMsg;
    QRfbPixelFormat pixelFormat;
    Qt::KeyboardModifiers keymod;
    int encodingsPending;
    int cutTextPending;
    bool supportHextile;
    bool wantUpdate;
    int nibble;
    bool sameEndian;
    int refreshRate;
    QVNCScreen *qvnc_screen;
};

#endif // QT_NO_QWS_VNC
#endif // QSCREENVNC_P_H
