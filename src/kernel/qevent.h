/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qevent.h#54 $
**
** Definition of event classes
**
** Created : 931029
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QEVENT_H
#define QEVENT_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qrect.h"
#include "qstring.h"
#endif // QT_H


#define Event_None		    0		// invalid event
#define Event_Timer		    1		// timer event
#define Event_MouseButtonPress	    2		// mouse button pressed
#define Event_MouseButtonRelease    3		// mouse button released
#define Event_MouseButtonDblClick   4		// mouse button double click
#define Event_MouseMove		    5		// mouse move
#define Event_KeyPress		    6		// key pressed
#define Event_KeyRelease	    7		// key released
#define Event_FocusIn		    8		// keyboard focus received
#define Event_FocusOut		    9		// keyboard focus lost
#define Event_Enter		   10		// mouse enters widget
#define Event_Leave		   11		// mouse leaves widget
#define Event_Paint		   12		// paint widget
#define Event_Move		   13		// move widget
#define Event_Resize		   14		// resize widget
#define Event_Create		   15		// after object creation
#define Event_Destroy		   16		// during object destruction
#define Event_Show		   17		// widget is shown
#define Event_Hide		   18		// widget is hidden
#define Event_Close		   19		// request to close widget
#define Event_Quit		   20		// request to quit application
#define Event_Accel		   30		// accelerator event
#define Event_Clipboard		   40		// internal clipboard event
#define Event_SockAct		   50		// socket activation
#define Event_DragEnter		   60		// drag moves into widget
#define Event_DragMove		   61		// drag moves in widget
#define Event_DragLeave		   62		// drag leaves or is cancelled
#define	Event_Drop		   63		// actual drop
#define	Event_DragResponse	   64		// drag accepted/rejected
#define Event_ChildInserted	   70		// new child widget
#define Event_ChildRemoved	   71		// deleted child widget
#define Event_LayoutHint	   72		// child min/max size changed
#define Event_User		 1000		// first user event id


class QEvent					// event base class
{
public:
    QEvent( int type )
	: t(type), posted(FALSE) {}
   ~QEvent()			{ if ( posted ) peErrMsg(); }
    int	  type()	const	{ return t; }
protected:
    int	  t;
    bool  posted;
private:
    void  peErrMsg();
};


class QTimerEvent : public QEvent		// timer event
{
public:
    QTimerEvent( int timerId )
	: QEvent(Event_Timer), id(timerId) {}
    int	  timerId()	const	{ return id; }
protected:
    int	  id;
};

#define Q_TIMER_EVENT(x)	((QTimerEvent*)x)


enum ButtonState {				// mouse/keyboard state values
    NoButton	    = 0x00,
    LeftButton	    = 0x01,
    RightButton	    = 0x02,
    MidButton	    = 0x04,
    MouseButtonMask = 0x07,
    ShiftButton	    = 0x08,
    ControlButton   = 0x10,
    AltButton	    = 0x20,
    KeyButtonMask   = 0x38
};

class QMouseEvent : public QEvent		// mouse event
{
public:
    QMouseEvent( int type, const QPoint &pos, int button, int state )
	: QEvent(type), p(pos), b(button),s((ushort)state) {}
    const QPoint &pos() const	{ return p; }
    int	   x()		const	{ return p.x(); }
    int	   y()		const	{ return p.y(); }
    int	   button()	const	{ return b; }
    int	   state()	const	{ return s; }
protected:
    QPoint p;
    int	   b;
    ushort s;
};

#define Q_MOUSE_EVENT(x)	((QMouseEvent*)x)


class QKeyEvent : public QEvent			// keyboard event
{
public:
    QKeyEvent( int type, int key, int ascii, int state )
	: QEvent(type), k((ushort)key), s((ushort)state), a((uchar)ascii),
	  accpt(TRUE) {}
    int	   key()	const	{ return k; }
    int	   ascii()	const	{ return a; }
    int	   state()	const	{ return s; }
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    ushort k, s;
    uchar  a;
    char   accpt;				// ### Qt 2.0: bool
};

#define Q_KEY_EVENT(x)		((QKeyEvent*)x)


class QFocusEvent : public QEvent		// widget focus event
{
public:
    QFocusEvent( int type )
	: QEvent(type) {}
    bool   gotFocus()	const { return type() == Event_FocusIn; }
    bool   lostFocus()	const { return type() == Event_FocusOut; }
};

#define Q_FOCUS_EVENT(x)	((QFocusEvent*)x)


class QPaintEvent : public QEvent		// widget paint event
{
public:
    QPaintEvent( const QRect &paintRect )
	: QEvent(Event_Paint), r(paintRect) {}
    const QRect &rect() const	{ return r; }
protected:
    QRect r;
};

#define Q_PAINT_EVENT(x)	((QPaintEvent*)x)


class QMoveEvent : public QEvent		// widget move event
{
public:
    QMoveEvent( const QPoint &pos, const QPoint &oldPos )
	: QEvent(Event_Move), p(pos), oldp(oldPos) {}
    const QPoint &pos()	  const { return p; }
    const QPoint &oldPos()const { return oldp;}
protected:
    QPoint p, oldp;
};

#define Q_MOVE_EVENT(x)		((QMoveEvent*)x)


class QResizeEvent : public QEvent		// widget resize event
{
public:
    QResizeEvent( const QSize &size, const QSize &oldSize )
	: QEvent(Event_Resize), s(size), olds(oldSize) {}
    const QSize &size()	  const { return s; }
    const QSize &oldSize()const { return olds;}
protected:
    QSize s, olds;
};

#define Q_RESIZE_EVENT(x)	((QResizeEvent*)x)


class QCloseEvent : public QEvent		// widget close event
{
public:
    QCloseEvent()
	: QEvent(Event_Close), accpt(FALSE) {}
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    bool   accpt;
};

#define Q_CLOSE_EVENT(x)	((QCloseEvent*)x)


class QShowEvent : public QEvent		// widget show event
{
public:
    QShowEvent(bool spontaneous)
	: QEvent(Event_Show), spont(spontaneous) {}
    bool spontaneous() const { return spont; }
protected:
    bool spont;
};

#define Q_SHOW_EVENT(x)		((QShowEvent*)x)


class QHideEvent : public QEvent		// widget hide event
{
public:
    QHideEvent(bool spontaneous)
	: QEvent(Event_Hide), spont(spontaneous) {}
    bool spontaneous() const { return spont; }
protected:
    bool spont;
};

#define Q_HIDE_EVENT(x)		((QHideEvent*)x)


// this class is rather closed at the moment.  if you need to create
// your own QDragMoveEvent objects, write to qt-bugs@troll.no and
// we'll try to find a way to extend it so it covers your needs.

class QDragMoveEvent : public QEvent
{
public:
    QDragMoveEvent( const QPoint& pos )
	: QEvent(Event_DragMove), p(pos), accpt(FALSE), d(0),
	  rect( p,QSize( 1, 1 ) ) {}
    const QPoint& pos() const   { return p; }
    bool   isAccepted() const   { return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
    void   accept( const QRect & r) { accpt = TRUE; rect = r; }
    void   ignore( const QRect & r) { accpt =FALSE; rect = r; }
    QRect  answerRect() const { return rect; }
    const char * format( int n = 0 );
    bool provides( const char * );
    QByteArray data( const char * );
protected:
    QDragMoveEvent( const QPoint& pos, int type )
	: QEvent(type), p(pos), accpt(FALSE), d(0),
	  rect( p,QSize( 1, 1 ) ) {}
    QPoint p;
    bool   accpt;
    void * d;
    QRect rect;
};

class QDragEnterEvent : public QDragMoveEvent
{
public:
    QDragEnterEvent( const QPoint& pos ) :
	QDragMoveEvent(pos, Event_DragEnter) { }
};


class QDragResponseEvent : public QEvent
{
public:
    QDragResponseEvent( bool accepted )
	: QEvent(Event_DragResponse), a(accepted) {}
    bool   dragAccepted() const	{ return a; }
protected:
    bool a;
};


class QDragLeaveEvent : public QEvent
{
public:
    QDragLeaveEvent()
	: QEvent(Event_DragLeave) {}
};


class QDropEvent : public QEvent
{
public:
    QDropEvent( const QPoint& pos )
	: QEvent(Event_Drop), p(pos), accpt(FALSE) {}
    const QPoint &pos() const	{ return p; }
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
    QByteArray data( const char * );
protected:
    QPoint p;
    bool   accpt;
};


class QChildEvent : public QEvent
{
public:
    QChildEvent( int type, QWidget *child )
	: QEvent(type), c(child) {}
    QWidget *child() const	{ return c; }
    bool inserted() const { return t == Event_ChildInserted; }
    bool removed() const { return t == Event_ChildRemoved; }
protected:
    QWidget *c;
};


class QCustomEvent : public QEvent		// user-defined event
{
public:
    QCustomEvent( int type, void *data )
	: QEvent(type), d(data) {}
    void       *data()	const	{ return d; }
private:
    void       *d;
};


#define Q_CUSTOM_EVENT(x)	((QCustomEvent*)x)


#endif // QEVENT_H
