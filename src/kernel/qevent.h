/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qevent.h#3 $
**
** Definition of QEvent classes
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QEVENT_H
#define QEVENT_H

#include "qwindefs.h"
#include "qrect.h"


#define Event_None		    0		// invalid event
#define Event_Timer		    1		// timer event
#define Event_MouseButtonPress	    2		// mouse button pressed
#define Event_MouseButtonRelease    3		// mouse button released
#define Event_MouseButtonDblClick   4		// mouse button double click
#define Event_MouseMove		    5		// mouse move
#define Event_KeyPress		    6		// key pressed
#define Event_KeyRelease	    7		// key released
#define Event_Paint		    8		// paint widget
#define Event_Move		    9		// move widget
#define Event_Resize		   10		// resize widget
#define Event_Create		   11		// before widget is created
#define Event_Destroy		   12		// after widget was destroyed
#define Event_Show		   13		// before widget is displayed
#define Event_Hide		   14		// before widget is hidden
#define Event_Close		   15		// request to close widget
#define Event_FocusIn		   16		// keyboard focus received
#define Event_FocusOut		   17		// keyboard focus lost
#define Event_Enter		   18		// mouse enters widget
#define Event_Leave		   19		// mouse leaves widget
#define Event_Quit		   20		// request to quit application
#define Event_User		 1000		// first user event id


class QEvent					// event base class
{
public:
    QEvent( int	 type )		{ t = type; }
    int	  type()	const	{ return t; }	// event type
protected:
    int	  t;
};


class QTimerEvent : public QEvent		// timer event class
{
public:
    QTimerEvent( int timerId )
	: QEvent(Event_Timer)	{ id=timerId; }
    int	  timerId()	const	{ return id; }	// timer identifier
protected:
    int	  id;
};

#define Q_TIMER_EVENT(x)	((QTimerEvent*)x)


enum ButtonState {				// mouse/keyboard state values
    NoButton	   = 0x00,
    LeftButton	   = 0x01,
    RightButton	   = 0x02,
    MidButton	   = 0x04,
    ShiftButton	   = 0x08,
    ControlButton  = 0x10,
    AltButton	   = 0x20
};

class QMouseEvent : public QEvent		// mouse event class
{
public:
    QMouseEvent( int type, const QPoint &pos, int button, int state )
	: QEvent(type)		{ p=pos; b=button; st=(ushort)state; }
    QPoint &pos()		{ return p; }	// mouse position
    int	   button()	const	{ return b; }	// button which caused event
    int	   state()	const	{ return st; }	// button state (OR'ed)
protected:
    QPoint p;
    int	   b;
    ushort st;
};

#define Q_MOUSE_EVENT(x)	((QMouseEvent*)x)


class QKeyEvent : public QEvent			// keyboard event class
{
public:
    QKeyEvent( int type, int kc, char ac, int state )
	: QEvent(type)		{ k=(ushort)kc, a=ac, st=(ushort)state; }
    int	   key()	const	{ return k; }	// key code (Key_Code)
    char   ascii()	const	{ return a; }	// ascii value
    int	   state()	const	{ return st; }	// keyboard status
protected:
    ushort k, st;
    char   a;
};

#define Q_KEY_EVENT(x)		((QKeyEvent*)x)


class QPaintEvent : public QEvent		// widget paint event class
{
public:
    QPaintEvent( const QRect &paintRect )
	: QEvent(Event_Paint) { r=paintRect; }
    QRect &rect()		{ return r; }	// rectangle to be painted
protected:
    QRect r;
};

#define Q_PAINT_EVENT(x)	((QPaintEvent*)x)


class QMoveEvent : public QEvent		// widget move event class
{
public:
    QMoveEvent( const QPoint &pos )
	: QEvent(Event_Move)	{ p=pos; }
    QPoint &pos()		{ return p; }	// widget position
protected:
    QPoint p;
};

#define Q_MOVE_EVENT(x)		((QMoveEvent*)x)


class QResizeEvent : public QEvent		// widget resize event class
{
public:
    QResizeEvent( const QSize &size )
	: QEvent(Event_Resize)	{ s=size; }
    QSize &size()		{ return s; }	// widget size
protected:
    QSize s;
};

#define Q_RESIZE_EVENT(x)	((QResizeEvent*)x)


#endif // QEVENT_H
