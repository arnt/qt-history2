/*****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Implementation of QGfxvnc (remote frame buffer driver)
** Proof of concept driver only.
** 
** Created : 20000703
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QGFXVNC_QWS_H
#define QGFXVNC_QWS_H

#include <qwssocket_qws.h>

#ifndef QT_NO_QWS_VNC

class QTimer;

class QRfbRect
{
public:
    QRfbRect() {}
    QRfbRect( Q_UINT16 _x, Q_UINT16 _y, Q_UINT16 _w, Q_UINT16 _h ) {
	x = _x; y = _y; w = _w; h = _h;
    }

    void read( QWSSocket *s );
    void write( QWSSocket *s );

    Q_UINT16 x;
    Q_UINT16 y;
    Q_UINT16 w;
    Q_UINT16 h;
};

class QRfbPixelFormat
{
public:
    static int size() { return 16; }

    void read( QWSSocket *s );
    void write( QWSSocket *s );

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
    ~QRfbServerInit() { delete name; }

    int size() const { return QRfbPixelFormat::size() + 8 + strlen( name ); }
    void setName( const char *n );

    void read( QWSSocket *s );
    void write( QWSSocket *s );

    Q_UINT16 width;
    Q_UINT16 height;
    QRfbPixelFormat format;
    char *name;
};

class QRfbSetEncodings
{
public:
    bool read( QWSSocket *s );

    Q_UINT16 count;
};

class QRfbFrameBufferUpdateRequest
{
public:
    bool read( QWSSocket *s );

    char incremental;
    QRfbRect rect;
};

class QRfbKeyEvent
{
public:
    bool read( QWSSocket *s );

    char down;
    int  keycode;
    int  unicode;
};

class QRfbPointerEvent
{
public:
    bool read( QWSSocket *s );

    uint buttons;
    Q_UINT16 x;
    Q_UINT16 y;
};

class QRfbClientCutText
{
public:
    bool read( QWSSocket *s );

    Q_UINT32 length;
};


class QVNCServer : public QWSServerSocket
{
    Q_OBJECT
public:
    QVNCServer();
    ~QVNCServer();

    virtual void newConnection( int socket );

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
    bool checkFill( const uchar *data, int numPixels );
    int getPixel( uchar ** );
    void sendHextile();
    void sendRaw();

private slots:
    void readClient();
    void checkUpdate();
    void discardClient();

private:
    enum ClientState { Protocol, Init, Connected };
    QTimer *timer;
    QWSSocket *client;
    ClientState state;
    Q_UINT8 msgType;
    bool handleMsg;
    QRfbPixelFormat pixelFormat;
    int keymod;
    int encodingsPending;
    int cutTextPending;
    bool supportHextile;
    bool wantUpdate;
};

#endif // QT_NO_QWS_VNC

#endif // QGFXVNC_QWS_H

