/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qevent.h#5 $
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
#define Event_FocusIn		    8		// keyboard focus received
#define Event_FocusOut		    9		// keyboard focus lost
#define Event_Enter		   10		// mouse enters widget
#define Event_Leave		   11		// mouse leaves widget
#define Event_Paint		   12		// paint widget
#define Event_Move		   13		// move widget
#define Event_Resize		   14		// resize widget
#define Event_Create		   15		// after object creation
#define Event_Destroy		   16		// during object destruction
#define Event_Show		   17		// before widget is displayed
#define Event_Hide		   18		// before widget is hidden
#define Event_Close		   19		// request to close widget
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


class QTimerEvent : public QEvent		// timer event
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

class QMouseEvent : public QEvent		// mouse event
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


class QKeyEvent : public QEvent			// keyboard event
{
public:
    QKeyEvent( int type, int kc, char ac, int state )
	: QEvent(type)		{ k=(ushort)kc; a=ac; st=(ushort)state;
				  accpt=TRUE; }
    int	   key()	const	{ return k; }	// key code (Key_Code)
    char   ascii()	const	{ return a; }	// ascii value
    int	   state()	const	{ return st; }	// keyboard status
    bool   isAccepted()	const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    ushort k, st;
    char   a, accpt;
};

#define Q_KEY_EVENT(x)		((QKeyEvent*)x)


class QFocusEvent : public QEvent		// widget focus event
{
public:
    QFocusEvent( int type )
	: QEvent(type)		{}
};

#define Q_FOCUS_EVENT(x)	((QFocusEvent*)x)


class QPaintEvent : public QEvent		// widget paint event
{
public:
    QPaintEvent( const QRect &paintRect )
	: QEvent(Event_Paint)	{ r=paintRect; }
    QRect &rect()		{ return r; }	// rectangle to be painted
protected:
    QRect r;
};

#define Q_PAINT_EVENT(x)	((QPaintEvent*)x)


class QMoveEvent : public QEvent		// widget move event
{
public:
    QMoveEvent( const QPoint &pos )
	: QEvent(Event_Move)	{ p=pos; }
    QPoint &pos()		{ return p; }	// widget position
protected:
    QPoint p;
};

#define Q_MOVE_EVENT(x)		((QMoveEvent*)x)


class QResizeEvent : public QEvent		// widget resize event
{
public:
    QResizeEvent( const QSize &size )
	: QEvent(Event_Resize)	{ s=size; }
    QSize &size()		{ return s; }	// widget size
protected:
    QSize s;
};

#define Q_RESIZE_EVENT(x)	((QResizeEvent*)x)


class QShowEvent : public QEvent		// widget show event
{
public:
    QShowEvent() : QEvent(Event_Show)	{}
};

#define Q_SHOW_EVENT(x)		((QShowEvent*)x)


class QHideEvent : public QEvent		// widget hide event
{
public:
    QHideEvent() : QEvent(Event_Hide)	{}
};

#define Q_HIDE_EVENT(x)		((QHideEvent*)x)


class QCloseEvent : public QEvent		// widget close event
{
public:
    QCloseEvent()
	: QEvent(Event_Close)	{ accpt = TRUE; }
    bool   isAccepted()	const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    bool   accpt;
};

#define Q_CLOSE_EVENT(x)	((QCloseEvent*)x)


#endif // QEVENT_H
