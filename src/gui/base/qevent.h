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
#include "qcoreevent.h"
#endif // QT_H

class QAction;

class Q_GUI_EXPORT QInputEvent : public QEvent
{
public:
    QInputEvent(Type type) : QEvent(type), accpt(true){}
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
private:
    bool accpt;
};

class Q_GUI_EXPORT QMouseEvent : public QInputEvent
{
public:
    QMouseEvent( Type type, const QPoint &pos, int button, int state );

    QMouseEvent( Type type, const QPoint &pos, const QPoint&globalPos,
		 int button, int state )
	: QInputEvent(type), p(pos), g(globalPos), b((ushort)button),s((ushort)state) {};

    const QPoint &pos() const	{ return p; }
    const QPoint &globalPos() const { return g; }
    int	   x()		const	{ return p.x(); }
    int	   y()		const	{ return p.y(); }
    int	   globalX()	const	{ return g.x(); }
    int	   globalY()	const	{ return g.y(); }
    ButtonState button() const	{ return (ButtonState) b; }
    ButtonState state()	const	{ return (ButtonState) s; }
    ButtonState stateAfter() const;
protected:
    QPoint p;
    QPoint g;
    ushort b;
    ushort s;
};


#ifndef QT_NO_WHEELEVENT
class Q_GUI_EXPORT QWheelEvent : public QInputEvent
{
public:
    QWheelEvent( const QPoint &pos, int delta, int state, Orientation orient = Vertical );
    QWheelEvent( const QPoint &pos, const QPoint& globalPos, int delta, int state, Orientation orient = Vertical  )
	: QInputEvent(Wheel), p(pos), g(globalPos), d(delta), s((ushort)state), o(orient) {}
    int	   delta()	const	{ return d; }
    const QPoint &pos() const	{ return p; }
    const QPoint &globalPos() const	{ return g; }
    int	   x()		const	{ return p.x(); }
    int	   y()		const	{ return p.y(); }
    int	   globalX()	const	{ return g.x(); }
    int	   globalY()	const	{ return g.y(); }
    ButtonState state()	const	{ return ButtonState(s); }
    Orientation orientation() 	const 	{ return o; }
protected:
    QPoint p;
    QPoint g;
    int d;
    ushort s;
    Orientation o;
};
#endif

class Q_GUI_EXPORT QTabletEvent : public QInputEvent
{
public:
    enum TabletDevice { NoDevice = -1, Puck, Stylus, Eraser };
    QTabletEvent( Type t, const QPoint &pos, const QPoint &globalPos, int device,
		  int pressure, int xTilt, int yTilt, const QPair<int,int> &uId );
    QTabletEvent( const QPoint &pos, const QPoint &globalPos, int device,
		  int pressure, int xTilt, int yTilt, const QPair<int,int> &uId )
	: QInputEvent( TabletMove ), mPos( pos ), mGPos( globalPos ), mDev( device ),
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

class Q_GUI_EXPORT QKeyEvent : public QInputEvent
{
public:
    QKeyEvent( Type type, int key, int state, const QString& text=QString::null,
	       bool autorep=FALSE, ushort count=1 )
	: QInputEvent(type), txt(text), k(key), s((ushort)state), c(count), autor(autorep)
    {
	if ( key >= Key_Back && key <= Key_MediaLast )
	    ignore();
    }
    int	   key()	const	{ return k; }
#ifdef QT_COMPAT
    /*QT_COMPAT*/ QKeyEvent( Type type, int key, int /*ascii*/, int state, const QString& text=QString::null,
			     bool autorep=FALSE, ushort count=1 )
	: QInputEvent(type), txt(text), k(key), s((ushort)state), c(count), autor(autorep)
    {
	if ( key >= Key_Back && key <= Key_MediaLast )
	    ignore();
    }
    QT_COMPAT int	   ascii()	const	{
	return (txt.length() ? txt.unicode()->latin1() : 0);
    }
#endif
    ButtonState state()	const	{ return ButtonState(s); }
    ButtonState stateAfter() const;
    QString text()      const   { return txt; }
    bool   isAutoRepeat() const	{ return autor; }
    int   count() const	{ return int(c); }

protected:
    QString txt;
    int k;
    ushort s;
    ushort c;
    uint   autor:1;
};


class Q_GUI_EXPORT QFocusEvent : public QEvent
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


class Q_GUI_EXPORT QPaintEvent : public QEvent
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
    QPaintEvent( const QRegion &paintRegion, const QRect &paintRect)
	: QEvent(Paint),
	  rec(paintRect),
	  reg(paintRegion){}

    const QRect &rect() const	  { return rec; }
    const QRegion &region() const { return reg; }

#ifdef QT_COMPAT
    QT_COMPAT bool erased() const { return true; }
#endif
protected:
    friend class QApplication;
    friend class QCoreApplication;
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

class Q_GUI_EXPORT QMoveEvent : public QEvent
{
public:
    QMoveEvent( const QPoint &pos, const QPoint &oldPos )
	: QEvent(Move), p(pos), oldp(oldPos) {}
    const QPoint &pos()	  const { return p; }
    const QPoint &oldPos()const { return oldp;}
protected:
    QPoint p, oldp;
    friend class QApplication;
    friend class QCoreApplication;
};


class Q_GUI_EXPORT QResizeEvent : public QEvent
{
public:
    QResizeEvent( const QSize &size, const QSize &oldSize )
	: QEvent(Resize), s(size), olds(oldSize) {}
    const QSize &size()	  const { return s; }
    const QSize &oldSize()const { return olds;}
protected:
    QSize s, olds;
    friend class QApplication;
    friend class QCoreApplication;
};


class Q_GUI_EXPORT QCloseEvent : public QInputEvent
{
public:
    QCloseEvent()
	: QInputEvent(Close) {}
};

class Q_GUI_EXPORT QIconDragEvent : public QEvent
{
public:
    QIconDragEvent()
	: QEvent(IconDrag), accpt(FALSE) {}

    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    bool   accpt;
};

class Q_GUI_EXPORT QShowEvent : public QEvent
{
public:
    QShowEvent()
	: QEvent(Show) {}
};


class Q_GUI_EXPORT QHideEvent : public QEvent
{
public:
    QHideEvent()
	: QEvent(Hide) {}
};

class Q_GUI_EXPORT QContextMenuEvent : public QInputEvent
{
public:
    enum Reason { Mouse, Keyboard, Other };
    QContextMenuEvent( Reason reason, const QPoint &pos, const QPoint &globalPos, int state )
	: QInputEvent( ContextMenu ), p( pos ), gp( globalPos ),
	reas( reason ), s((ushort)state) {}
    QContextMenuEvent( Reason reason, const QPoint &pos, int state );

    int	    x() const { return p.x(); }
    int	    y() const { return p.y(); }
    int	    globalX() const { return gp.x(); }
    int	    globalY() const { return gp.y(); }

    const QPoint&   pos() const { return p; }
    const QPoint&   globalPos() const { return gp; }

    ButtonState state()	const	{ return (ButtonState) s; }
    Reason  reason() const { return Reason( reas ); }

protected:
    QPoint  p;
    QPoint  gp;
    uint    reas:8;
    ushort s;
};


class Q_GUI_EXPORT QIMEvent : public QInputEvent
{
public:
    QIMEvent( Type type, const QString &text, int cursorPosition, int selLength = 0 )
	: QInputEvent(type), txt(text), cpos(cursorPosition), selLen(selLength) {}
    const QString &text() const { return txt; }
    int cursorPos() const { return cpos; }
    int selectionLength() const { return selLen; }

private:
    QString txt;
    int cpos;
    int selLen;
};

#ifndef QT_NO_DRAGANDDROP

// This class is rather closed at the moment.  If you need to create your
// own DND event objects, write to qt-bugs@trolltech.com and we'll try to
// find a way to extend it so it covers your needs.

class Q_GUI_EXPORT QDropEvent : public QEvent, public QMimeSource
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



class Q_GUI_EXPORT QDragMoveEvent : public QDropEvent
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


class Q_GUI_EXPORT QDragEnterEvent : public QDragMoveEvent
{
public:
    QDragEnterEvent( const QPoint& pos ) :
	QDragMoveEvent(pos, DragEnter) { }
};


/* An internal class */
class Q_GUI_EXPORT QDragResponseEvent : public QEvent
{
public:
    QDragResponseEvent( bool accepted )
	: QEvent(DragResponse), a(accepted) {}
    bool   dragAccepted() const	{ return a; }
protected:
    bool a;
};


class Q_GUI_EXPORT QDragLeaveEvent : public QEvent
{
public:
    QDragLeaveEvent()
	: QEvent(DragLeave) {}
};

#endif // QT_NO_DRAGANDDROP


class Q_GUI_EXPORT QHelpEvent : public QEvent
{
public:
    inline QHelpEvent(Type type, const QPoint &pos, const QPoint &globalPos)
	: QEvent(type), p(pos), gp(globalPos) {}

    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return gp.x(); }
    inline int globalY() const { return gp.y(); }

    inline const QPoint& pos() const { return p; }
    inline const QPoint& globalPos() const { return gp; }

private:
    QPoint  p;
    QPoint  gp;
};

class Q_GUI_EXPORT QStatusTipEvent : public QEvent
{
public:
    QStatusTipEvent(const QString &tip):QEvent(StatusTip), s(tip){}
    QString tip() const { return s; }
private:
    QString s;
};

class Q_GUI_EXPORT QActionEvent : public QEvent
{
    QAction *act, *bef;
public:
    QActionEvent(int type, QAction *action, QAction *before=0) : QEvent((QEvent::Type)type), act(action), bef(before) { }

    QAction *action() const { return act; }
    QAction *before() const { return bef; }
};

class Q_GUI_EXPORT QFileOpenEvent : public QEvent
{
public:
    QFileOpenEvent(const QString &file) : QEvent(FileOpen), f(file) { }
    QString file() const { return f; }
private:
    QString f;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug, const QEvent *);
#endif

#endif // QEVENT_H
