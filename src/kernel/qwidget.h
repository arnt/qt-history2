/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.h#17 $
**
** Definition of QWidget class
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWIDGET_H
#define QWIDGET_H

#include "qwindefs.h"
#include "qobject.h"
#include "qpaintd.h"
#include "qcolor.h"
#include "qfont.h"
#include "qcursor.h"
#include "qevent.h"


class QWidget : public QObject, public QPaintDevice
{						// base class for UI objects
friend class QApplication;
friend class QPainter;
    Q_OBJECT
public:
    QWidget( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QWidget();

    WId	    id()		const	{ return ident; }

  // GUI style setting

    GUIStyle style() const;			// get widget GUI style
    void    setStyle( GUIStyle );		// set widget GUI style

  // Widget control functions

    void    enable();				// enable events
    void    disable();				// disable events
    bool    isDisabled()	const	{ return testFlag(WState_Disabled); }

  // Widget coordinates (relative to its parent)

    QRect   geometry()		const	{ return ncrect; }
    QRect   clientGeometry()	const	{ return rect; }
    QSize   clientSize()	const	{ return rect.size(); }
    int	    clientWidth()	const	{ return rect.width(); }
    int	    clientHeight()	const	{ return rect.height(); }
    QRect   clientRect()	const	{ return QRect(0,0,rect.width(),
						       rect.height()); }
    void    setMinimumSize( int w, int h );
    void    setMaximumSize( int w, int h );
    void    setSizeIncrement( int w, int h );

  // Widget coordinate mapping

    QPoint  mapToGlobal( const QPoint & )   const;
    QPoint  mapFromGlobal( const QPoint & ) const;
    QPoint  mapToParent( const QPoint & )   const;
    QPoint  mapFromParent( const QPoint & ) const;

  // Widget attribute functions

    bool    setMouseMoveEvents( bool );

    QColor  backgroundColor() const;
    QColor  foregroundColor() const;
    virtual void setBackgroundColor( const QColor & );
    virtual void setForegroundColor( const QColor & );

    QFont   font() const;			// get/set font
    virtual void setFont( const QFont & );

    QCursor cursor() const;			// get/set cursor
    void    setCursor( const QCursor & );

  // Keyboard focus functions

    bool    hasFocus() const { return testFlag(WState_FocusA); }
    void    setFocus();				// set keyboard focus
    static QWidget *widgetInFocus();		// get widget in focus

  // Grab functions

    void    grabKeyboard();
    void    releaseKeyboard();
    void    grabMouse( bool exclusive=TRUE );
    void    releaseMouse();

  // Update/refresh functions

    void    update();				// update widget
    void    update( int x, int y, int w, int h);// update part of widget
    void    repaint( bool erase=TRUE );		// repaint widget
    void    repaint( int x, int y, int w, int h, bool erase=TRUE );
    void    repaint( const QRect &, bool erase=TRUE );

  // Widget management functions

    virtual void show();			// show widget
    virtual void hide();			// hide widget
    virtual bool close( bool forceKill=FALSE );	// close widget
    bool    isVisible()	const { return testFlag(WState_Visible); }
    bool    isActive()	const { return testFlag(WState_Active); }
    void    raise();				// raise widget
    void    lower();				// lower widget
    virtual void move( int x, int y );		// move widget
    void    move( const QPoint & );
    virtual void resize( int w, int h );	// resize widget
    void    resize( const QSize & );
    virtual void changeGeometry( int x, int y, int w, int h );
    void    changeGeometry( const QRect & );	// move and resize

    void    erase();				// erase widget contents
    void    scroll( int dx, int dy );		// scroll widget contents

    void    drawText( int x, int y, const char * );
    void    drawText( const QPoint &, const char * );

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
    virtual bool focusInEvent( QEvent * );
    virtual void focusOutEvent( QEvent * );
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
    bool    testFlag( WFlags n ) const { return (flags & n) != 0; }

#if defined(_WS_X11_)
    GC	    getGC()  const { return gc; }
#endif

protected:
    void    setFlag( WFlags n )	{ flags |= n; }
    void    clearFlag( WFlags n )	{ flags &= ~n; }
    void    setRect( const QRect & );		// set rect, update ncrect
    void    setNCRect( const QRect & );		// set ncrect, update rect
    long    metric( int ) const;		// get metric information
    static  ulong nWidgets();			// get number of widgets

#if defined(_WS_PM_)
    int	    convertYPos( int );
    void    reposChildren();
    WId	    frm_wnd;
#elif defined(_WS_X11_)
    GC	    gc;
#endif

private:
    void    set_id( WId );			// set widget id
    bool    create();				// create widget
    bool    destroy();				// destroy widget
    void    createExtra();			// create extra data
    WId	    ident;				// widget identifier
    WFlags  flags;				// widget flags
    QRect   rect;				// widget geometry
    QRect   ncrect;				// non-client geometry
    QColor  bg_col;				// background color
    QColor  fg_col;				// foreground color
    QFont   fnt;				// widget font
    QCursor curs;				// widget cursor
    QWExtra *extra;				// extra widget data
    static void createMapper();			// create widget mapper
    static void destroyMapper();		// destroy widget mapper
    static QWidgetMapper *mapper;		// maps identifier to widget
    static QWidget *activeWidget;		// widget in keyboard focus
};


inline void QWidget::repaint( bool erase )
{
    repaint( rect, erase );
}

inline void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    repaint( QRect(x,y,w,h), erase );
}

inline void QWidget::move( const QPoint &p )
{
    move( p.x(), p.y() );
}

inline void QWidget::resize( const QSize &s )
{
    resize( s.width(), s.height());
}

inline void QWidget::changeGeometry( const QRect &r )
{
    changeGeometry( r.left(), r.top(), r.width(), r.height() );
}

inline void QWidget::drawText( const QPoint &p, const char *s )
{
    drawText( p.x(), p.y(), s );
}


#endif // QWIDGET_H
