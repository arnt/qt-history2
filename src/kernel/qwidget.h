/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.h#4 $
**
** Definition of QWidget class
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
** --------------------------------------------------------------------------
** The QWidget class is the base class of all user interface objects in Qt.
*****************************************************************************/

#ifndef QWIDGET_H
#define QWIDGET_H

#include "qwindefs.h"
#include "qobject.h"
#include "qpaintd.h"
#include "qrect.h"
#include "qcolor.h"
#include "qfont.h"
#include "qcursor.h"
#include "qstring.h"
#include "qevent.h"


class QWidget : public QObject, public QPaintDevice
{						// base class for UI objects
friend class QApplication;
friend class QPainter;
    Q_OBJECT
public:
    QWidget( QView *parent=0, WFlags f=0 );
   ~QWidget();

    WId	     id()		const	{ return ident; }

  // GUI style setting

    GuiStyle guiStyle() const { return gui; }	// get widget GUI style
    void     setGuiStyle( GuiStyle );		// set widget GUI style
    static void setGlobalGuiStyle( GuiStyle );	// set global GUI style

  // Widget control functions

    void     enable();				// enable events
    void     disable();				// disable events
    bool     isDisabled()	const	{ return testFlag(WState_Disabled); }

  // Widget coordinates (relative to its parent)

    QRect    geometry()		const	{ return ncrect; }
    QRect    clientGeometry()	const	{ return rect; }
    QSize    clientSize()	const	{ return rect.size(); }
    QRect    clientRect()	const;

  // Widget coordinate mapping

    QPoint   mapToGlobal( const QPoint & )   const;
    QPoint   mapFromGlobal( const QPoint & ) const;
    QPoint   mapToParent( const QPoint & )   const;
    QPoint   mapFromParent( const QPoint & ) const;

  // Widget attribute functions

    bool     setMouseMoveEvents( bool );

    QColor   backgroundColor() const;
    QColor   foregroundColor() const;
    virtual void setBackgroundColor( const QColor & );
    virtual void setForegroundColor( const QColor & );

    QFont    font() const;			// get/set font
    void     setFont( const QFont & );

    QCursor  cursor() const;			// get/set cursor
    void     setCursor( const QCursor & );

  // Widget management functions

    bool     update();				// update widget
    virtual  bool show();			// show widget
    bool     hide();				// hide widget
    bool     isVisible() const { return testFlag(WState_Visible); }
    bool     isActive()	 const { return testFlag(WState_Active); }
    bool     raise();				// raise widget
    bool     lower();				// lower widget
    virtual bool move( int x, int y );		// move widget
    bool     move( const QPoint & );
    virtual bool resize( int w, int h );	// resize widget
    bool     resize( const QSize & );
    virtual bool changeGeometry( int x, int y, int w, int h );
    bool     changeGeometry( const QRect & );	// move and resize

    bool     erase();				// erase widget contents
    bool     scroll( int dx, int dy );		// scroll widget contents

    bool     drawText( int x, int y, const char * );
    bool     drawText( const QPoint &, const char * );

  // Widget events

protected:
    bool	 event( QEvent * );
    virtual void timerEvent( QTimerEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseDoubleClickEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual bool keyPressEvent( QKeyEvent * );
    virtual bool keyReleaseEvent( QKeyEvent * );
    virtual void paintEvent( QPaintEvent * );
    virtual void moveEvent( QMoveEvent * );
    virtual void resizeEvent( QResizeEvent * );
    virtual bool closeEvent( QEvent * );

#if defined(_WS_MAC_)
    virtual bool macEvent( MSG * );		// Macintosh event
#elif defined(_WS_WIN_)
    virtual bool winEvent( MSG * );		// Windows (+NT) event
#elif defined(_WS_PM_)
    virtual bool pmEvent( QMSG * );		// OS/2 PM event
#elif defined(_WS_X11_)
    virtual bool x11Event( XEvent * );		// X11 event
#endif

  // Misc. functions

public:
    QWidget *parentWidget()	 const	{ return (QWidget*)QObject::parent(); }
    static QWidget *find( WId );		// find widget by identifier
    bool     testFlag( WFlags n ) const { return (flags & n) != 0; }

protected:
    void     setFlag( WFlags n )	{ flags |= n; }
    void     clearFlag( WFlags n )	{ flags &= ~n; }
    void     setRect( const QRect & );		// set rect, update ncrect
    void     setNCRect( const QRect & );	// set ncrect, update rect
    static   ulong nWidgets();			// get number of widgets

#if defined(_WS_PM_)
    int	     convertYPos( int );
    void     reposChildren();
    WId	     frm_wnd;
#elif defined(_WS_X11_)
    GC	     gc;
#endif

private:
    void     set_id( WId );			// set widget id
    bool     create();				// create widget
    bool     destroy();				// destroy widget
    WId	     ident;				// widget identifier
    WFlags   flags;				// widget flags
    QRect    rect;				// widget geometry
    QRect    ncrect;				// non-client geometry
    QColor   bg_col;				// background color
    QColor   fg_col;				// foreground color
    QFont    fnt;				// widget font
    QCursor  curs;				// widget cursor
    GuiStyle gui;				// gui style
    static GuiStyle ggui;			// global GUI style
    static void createMapper();			// create widget mapper
    static void destroyMapper();		// destroy widget mapper
    static QWidgetMapper *mapper;		// maps identifier to widget
};


inline bool QWidget::move( const QPoint &p )
{
    return move( p.x(), p.y() );
}

inline bool QWidget::resize( const QSize &s )
{
    return resize( s.width(), s.height());
}

inline bool QWidget::changeGeometry( const QRect &r )
{
    return changeGeometry( r.left(), r.top(), r.width(), r.height() );
}

inline bool QWidget::drawText( const QPoint &p, const char *s )
{
    return drawText( p.x(), p.y(), s );
}


#endif // QWIDGET_H
