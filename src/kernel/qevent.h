/****************************************************************************
**
** Definition of event classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QEVENT_H
#define QEVENT_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qobject.h"
#include "qregion.h"
#include "qnamespace.h"
#include "qmime.h"
#include "qpair.h"
#include "qstring.h"
#endif // QT_H

class Q_EXPORT QTimerEvent : public QEvent
{
public:
    QTimerEvent( int timerId )
	: QEvent(Timer), id(timerId) {}
    int	  timerId()	const	{ return id; }
protected:
    int	  id;
};


class Q_EXPORT QMouseEvent : public QEvent
{
public:
    QMouseEvent( Type type, const QPoint &pos, int button, int state );

    QMouseEvent( Type type, const QPoint &pos, const QPoint&globalPos,
		 int button, int state )
	: QEvent(type), p(pos), g(globalPos), b((ushort)button),s((ushort)state),accpt(TRUE) {};

    const QPoint &pos() const	{ return p; }
    const QPoint &globalPos() const { return g; }
    int	   x()		const	{ return p.x(); }
    int	   y()		const	{ return p.y(); }
    int	   globalX()	const	{ return g.x(); }
    int	   globalY()	const	{ return g.y(); }
    ButtonState button() const	{ return (ButtonState) b; }
    ButtonState state()	const	{ return (ButtonState) s; }
    ButtonState stateAfter() const;
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    QPoint p;
    QPoint g;
    ushort b;
    ushort s;
    uint   accpt:1;
};


#ifndef QT_NO_WHEELEVENT
class Q_EXPORT QWheelEvent : public QEvent
{
public:
    QWheelEvent( const QPoint &pos, int delta, int state, Orientation orient = Vertical );
    QWheelEvent( const QPoint &pos, const QPoint& globalPos, int delta, int state, Orientation orient = Vertical  )
	: QEvent(Wheel), p(pos), g(globalPos), d(delta), s((ushort)state),
	  accpt(TRUE), o(orient) {}
    int	   delta()	const	{ return d; }
    const QPoint &pos() const	{ return p; }
    const QPoint &globalPos() const	{ return g; }
    int	   x()		const	{ return p.x(); }
    int	   y()		const	{ return p.y(); }
    int	   globalX()	const	{ return g.x(); }
    int	   globalY()	const	{ return g.y(); }
    ButtonState state()	const	{ return ButtonState(s); }
    Orientation orientation() 	const 	{ return o; }
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    QPoint p;
    QPoint g;
    int d;
    ushort s;
    bool   accpt;
    Orientation o;
};
#endif

class Q_EXPORT QTabletEvent : public QEvent
{
public:
    enum TabletDevice { NoDevice = -1, Puck, Stylus, Eraser };
    QTabletEvent( Type t, const QPoint &pos, const QPoint &globalPos, int device,
		  int pressure, int xTilt, int yTilt, const QPair<int,int> &uId );
    QTabletEvent( const QPoint &pos, const QPoint &globalPos, int device,
		  int pressure, int xTilt, int yTilt, const QPair<int,int> &uId )
	: QEvent( TabletMove ), mPos( pos ), mGPos( globalPos ), mDev( device ),
	  mPress( pressure ), mXT( xTilt ), mYT( yTilt ), mType( uId.first ),
	  mPhy( uId.second ), mbAcc(TRUE)
    {}
    int pressure()	const { return mPress; }
    int xTilt()		const { return mXT; }
    int yTilt()		const { return mYT; }
    const QPoint &pos()	const { return mPos; }
    const QPoint &globalPos() const { return mGPos; }
    int x()		const { return mPos.x(); }
    int y()		const { return mPos.y(); }
    int globalX()	const { return mGPos.x(); }
    int globalY()	const { return mGPos.y(); }
    TabletDevice device() 	const { return TabletDevice(mDev); }
    int isAccepted() const { return mbAcc; }
    void accept() { mbAcc = TRUE; }
    void ignore() { mbAcc = FALSE; }
    QPair<int,int> uniqueId() { return QPair<int,int>( mType, mPhy); }
protected:
    QPoint mPos;
    QPoint mGPos;
    int mDev,
	mPress,
	mXT,
	mYT,
	mType,
	mPhy;
    bool mbAcc;

};

class Q_EXPORT QKeyEvent : public QEvent
{
public:
    QKeyEvent( Type type, int key, int state, const QString& text=QString::null,
	       bool autorep=FALSE, ushort count=1 )
	: QEvent(type), txt(text), k(key), s((ushort)state), c(count),
	  accpt(TRUE), autor(autorep)
    {
	if ( key >= Key_Back && key <= Key_MediaLast )
	    accpt = FALSE;
    }
    int	   key()	const	{ return k; }
#ifndef QT_NO_COMPAT
    QKeyEvent( Type type, int key, int /*ascii*/, int state, const QString& text=QString::null,
	       bool autorep=FALSE, ushort count=1 )
	: QEvent(type), txt(text), k(key), s((ushort)state), c(count),
	  accpt(TRUE), autor(autorep)
    {
	if ( key >= Key_Back && key <= Key_MediaLast )
	    accpt = FALSE;
    }
    int	   ascii()	const	{ return k > 255 ? 0 : k; }
#endif
    ButtonState state()	const	{ return ButtonState(s); }
    ButtonState stateAfter() const;
    bool   isAccepted() const	{ return accpt; }
    QString text()      const   { return txt; }
    bool   isAutoRepeat() const	{ return autor; }
    int   count() const	{ return int(c); }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }

protected:
    QString txt;
    int k;
    ushort s;
    ushort c;
    uint   accpt:1;
    uint   autor:1;
};


class Q_EXPORT QFocusEvent : public QEvent
{
public:

    QFocusEvent( Type type )
	: QEvent(type) {}

    bool   gotFocus()	const { return type() == FocusIn; }
    bool   lostFocus()	const { return type() == FocusOut; }

    enum Reason { Mouse, Tab, Backtab, ActiveWindow, Popup, Shortcut, Other };
    static Reason reason();
    static void setReason( Reason reason );
    static void resetReason();

private:
    static Reason m_reason;
    static Reason prev_reason;
};


class Q_EXPORT QPaintEvent : public QEvent
{
public:
    QPaintEvent( const QRegion& paintRegion )
	: QEvent(Paint),
	  rec(paintRegion.boundingRect()),
	  reg(paintRegion){}
    QPaintEvent( const QRect &paintRect )
	: QEvent(Paint),
	  rec(paintRect),
	  reg(paintRect){}
    const QRect &rect() const	  { return rec; }
    const QRegion &region() const { return reg; }

#ifndef QT_NO_COMPAT
    bool erased() const { return true; }
#endif
protected:
    friend class QApplication;
    friend class QKernelApplication;
    QRect rec;
    QRegion reg;
};

#ifdef Q_WS_QWS
class QWSUpdateEvent : public QPaintEvent
{
public:
    QWSUpdateEvent( const QRegion& paintRegion )
	: QPaintEvent( paintRegion)
	{ t = QWSUpdate; }
    QWSUpdateEvent( const QRect &paintRect )
	: QPaintEvent( paintRect)
	{ t = QWSUpdate; }
};
#endif

class Q_EXPORT QMoveEvent : public QEvent
{
public:
    QMoveEvent( const QPoint &pos, const QPoint &oldPos )
	: QEvent(Move), p(pos), oldp(oldPos) {}
    const QPoint &pos()	  const { return p; }
    const QPoint &oldPos()const { return oldp;}
protected:
    QPoint p, oldp;
    friend class QApplication;
    friend class QKernelApplication;
};


class Q_EXPORT QResizeEvent : public QEvent
{
public:
    QResizeEvent( const QSize &size, const QSize &oldSize )
	: QEvent(Resize), s(size), olds(oldSize) {}
    const QSize &size()	  const { return s; }
    const QSize &oldSize()const { return olds;}
protected:
    QSize s, olds;
    friend class QApplication;
    friend class QKernelApplication;
};


class Q_EXPORT QCloseEvent : public QEvent
{
public:
    QCloseEvent()
	: QEvent(Close), accpt(FALSE) {}
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    bool   accpt;
};


class Q_EXPORT QShowEvent : public QEvent
{
public:
    QShowEvent()
	: QEvent(Show) {}
};


class Q_EXPORT QHideEvent : public QEvent
{
public:
    QHideEvent()
	: QEvent(Hide) {}
};

class Q_EXPORT QContextMenuEvent : public QEvent
{
public:
    enum Reason { Mouse, Keyboard, Other };
    QContextMenuEvent( Reason reason, const QPoint &pos, const QPoint &globalPos, int state )
	: QEvent( ContextMenu ), p( pos ), gp( globalPos ), accpt( TRUE ), consum( TRUE ),
	reas( reason ), s((ushort)state) {}
    QContextMenuEvent( Reason reason, const QPoint &pos, int state );

    int	    x() const { return p.x(); }
    int	    y() const { return p.y(); }
    int	    globalX() const { return gp.x(); }
    int	    globalY() const { return gp.y(); }

    const QPoint&   pos() const { return p; }
    const QPoint&   globalPos() const { return gp; }

    ButtonState state()	const	{ return (ButtonState) s; }
    bool    isAccepted() const	{ return accpt; }
    bool    isConsumed() const	{ return consum; }
    void    consume()		{ accpt = FALSE; consum = TRUE; }
    void    accept()		{ accpt = TRUE; consum = TRUE; }
    void    ignore()		{ accpt = FALSE; consum = FALSE; }

    Reason  reason() const { return Reason( reas ); }

protected:
    QPoint  p;
    QPoint  gp;
    bool    accpt;
    bool    consum;
    uint    reas:8;
    ushort s;
};


class Q_EXPORT QIMEvent : public QEvent
{
public:
    QIMEvent( Type type, const QString &text, int cursorPosition, int selLength = 0 )
	: QEvent(type), txt(text), a(TRUE), cpos(cursorPosition), selLen(selLength) {}
    const QString &text() const { return txt; }
    int cursorPos() const { return cpos; }
    bool isAccepted() const { return a; }
    void accept() { a = TRUE; }
    void ignore() { a = FALSE; }
    int selectionLength() const { return selLen; }

private:
    QString txt;
    bool a  : 1;
    int cpos;
    int selLen;
};

#ifndef QT_NO_DRAGANDDROP

// This class is rather closed at the moment.  If you need to create your
// own DND event objects, write to qt-bugs@trolltech.com and we'll try to
// find a way to extend it so it covers your needs.

class Q_EXPORT QDropEvent : public QEvent, public QMimeSource
{
public:
    QDropEvent( const QPoint& pos, Type typ=Drop )
	: QEvent(typ), p(pos),
	  act(0), accpt(0), accptact(0), resv(0),
	  d(0)
	{}
    const QPoint &pos() const	{ return p; }
    bool isAccepted() const	{ return accpt || accptact; }
    void accept(bool y=TRUE)	{ accpt = y; }
    void ignore()		{ accpt = FALSE; }

    bool isActionAccepted() const { return accptact; }
    void acceptAction(bool y=TRUE) { accptact = y; }
    enum Action { Copy, Link, Move, Private, UserAction=100 };
    void setAction( Action a ) { act = (uint)a; }
    Action action() const { return Action(act); }

    QWidget* source() const;
    const char* format( int n = 0 ) const;
    QByteArray encodedData( const char* ) const;
    bool provides( const char* ) const;

    QByteArray data(const char* f) const { return encodedData(f); }

    void setPoint( const QPoint& np ) { p = np; }

protected:
    QPoint p;
    uint act:8;
    uint accpt:1;
    uint accptact:1;
    uint resv:5;
    void * d;
};



class Q_EXPORT QDragMoveEvent : public QDropEvent
{
public:
    QDragMoveEvent( const QPoint& pos, Type typ=DragMove )
	: QDropEvent(pos,typ),
	  rect( pos, QSize( 1, 1 ) ) {}
    QRect answerRect() const { return rect; }
    void accept( bool y=TRUE ) { QDropEvent::accept(y); }
    void accept( const QRect & r) { accpt = TRUE; rect = r; }
    void ignore( const QRect & r) { accpt =FALSE; rect = r; }
    void ignore()		{ QDropEvent::ignore(); }

protected:
    QRect rect;
};


class Q_EXPORT QDragEnterEvent : public QDragMoveEvent
{
public:
    QDragEnterEvent( const QPoint& pos ) :
	QDragMoveEvent(pos, DragEnter) { }
};


/* An internal class */
class Q_EXPORT QDragResponseEvent : public QEvent
{
public:
    QDragResponseEvent( bool accepted )
	: QEvent(DragResponse), a(accepted) {}
    bool   dragAccepted() const	{ return a; }
protected:
    bool a;
};


class Q_EXPORT QDragLeaveEvent : public QEvent
{
public:
    QDragLeaveEvent()
	: QEvent(DragLeave) {}
};

#endif // QT_NO_DRAGANDDROP

class Q_EXPORT QChildEvent : public QEvent
{
public:
    QChildEvent( Type type, QObject *child )
	: QEvent(type), c(child) {}
    QObject *child() const { return c; }
    QWidget *childWidget() const;
    bool added() const { return type() == ChildAdded; }
#ifndef QT_NO_COMPAT
    bool inserted() const { return type() == ChildInserted; }
#endif
    bool polished() const { return type() == ChildPolished; }
    bool removed() const { return type() == ChildRemoved; }
protected:
    QObject *c;
};


class Q_EXPORT QCustomEvent : public QEvent
{
public:
    QCustomEvent( int type );
    QCustomEvent( Type type, void *data )
	: QEvent(type), d(data) {};
    void       *data()	const	{ return d; }
    void	setData( void* data )	{ d = data; }
private:
    void       *d;
};

#endif // QEVENT_H
