/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qscrollview.h>

class QImage;
class QTimer;
struct QVFbHeader;

class QVFbView : public QScrollView
{
    Q_OBJECT
public:
    QVFbView( int w, int h, int d, QWidget *parent = 0,
		const char *name = 0, uint wflags = 0 );
    ~QVFbView();

    int rate() { return refreshRate; }

public slots:
    void setRate( int );

protected slots:
    void timeout();

protected:
    void initLock();
    void lock();
    void unlock();
    void drawScreen();
    void sendMouseData( const QPoint &pos, int buttons );
    void sendKeyboardData( int unicode, int keycode, int modifiers,
			   bool press, bool repeat );
    virtual bool eventFilter( QObject *obj, QEvent *e );
    virtual void viewportPaintEvent( QPaintEvent *pe );
    virtual void contentsMousePressEvent( QMouseEvent *e );
    virtual void contentsMouseReleaseEvent( QMouseEvent *e );
    virtual void contentsMouseMoveEvent( QMouseEvent *e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void keyReleaseEvent( QKeyEvent *e );

private:
    int shmId;
    unsigned char *data;
    QVFbHeader *hdr;
    int lockId;
    QTimer *timer;
    int mouseFd;
    int keyboardFd;
    int refreshRate;
};

