/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.h#41 $
**
** Definition of QWidget class
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWIDGET_H
#define QWIDGET_H

#include "qwindefs.h"
#include "qobject.h"
#include "qpaintd.h"
#include "qpalette.h"
#include "qfontmet.h"
#include "qfontinf.h"
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

  // Widget coordinates

    QRect   frameGeometry()	const	{ return frect; }
    QRect   geometry()		const	{ return crect; }
    int	    x()			const	{ return crect.x(); }
    int	    y()			const	{ return crect.y(); }
    QPoint  pos()		const	{ return crect.topLeft(); }
    QSize   size()		const	{ return crect.size(); }
    int	    width()		const	{ return crect.width(); }
    int	    height()		const	{ return crect.height(); }
    QRect   rect()		const	{ return QRect(0,0,crect.width(),
						       crect.height()); }
    void    setMinimumSize( int w, int h );
    void    setMaximumSize( int w, int h );
    void    setSizeIncrement( int w, int h );

  // Widget coordinate mapping

    QPoint  mapToGlobal( const QPoint & )   const;
    QPoint  mapFromGlobal( const QPoint & ) const;
    QPoint  mapToParent( const QPoint & )   const;
    QPoint  mapFromParent( const QPoint & ) const;

  // Widget attribute functions

    QColor  backgroundColor() const;
    QColor  foregroundColor() const;
    virtual void setBackgroundColor( const QColor & );
    virtual void setBackgroundPixmap( const QPixmap & );

    const QColorGroup &colorGroup() const;
    const QPalette    &palette()    const;
    virtual void       setPalette( const QPalette & );

    QFontMetrics fontMetrics()	const { return QFontMetrics(fnt); }
    QFontInfo	 fontInfo()	const { return QFontInfo(fnt); }

    QFont	&font();			// get/set font
    virtual void setFont( const QFont & );
    QCursor cursor() const;			// get/set cursor
    void    setCursor( const QCursor & );

    bool    setMouseTracking( bool enable );

  // Keyboard input focus functions

    bool    hasFocus() const;
    void    setFocus();				// set keyboard focus

  // Grab functions

    void    grabMouse();
    void    grabMouse( const QCursor & );
    void    releaseMouse();
    void    grabKeyboard();
    void    releaseKeyboard();

  // Update/refresh functions

    bool    enableUpdates( bool enable );	// enable widget update/repaint
    void    update();				// update widget
    void    update( int x, int y, int w, int h);// update part of widget
    void    repaint( bool erase=TRUE );		// repaint widget
    void    repaint( int x, int y, int w, int h, bool erase=TRUE );
    void    repaint( const QRect &, bool erase=TRUE );

  // Widget management functions

    virtual void show();			// show widget
    virtual void hide();			// hide widget
    virtual bool close( bool forceKill=FALSE ); // close widget
    bool    isVisible() const { return testFlag(WState_Visible); }
    bool    isActive()	const { return testFlag(WState_Active); }
    void    raise();				// raise widget
    void    lower();				// lower widget
    virtual void move( int x, int y );		// move widget
    void    move( const QPoint & );
    virtual void resize( int w, int h );	// resize widget
    void    resize( const QSize & );
    virtual void setGeometry( int x, int y, int w, int h );
    void    setGeometry( const QRect & );	// move and resize

    void    recreate( QWidget *parent, WFlags f, const QPoint &p,
		      bool showIt=FALSE );

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
    virtual void keyPressEvent( QKeyEvent * );
    virtual void keyReleaseEvent( QKeyEvent * );
    virtual void focusInEvent( QFocusEvent * );
    virtual void focusOutEvent( QFocusEvent * );
    virtual void paintEvent( QPaintEvent * );
    virtual void moveEvent( QMoveEvent * );
    virtual void resizeEvent( QResizeEvent * );
    virtual void closeEvent( QCloseEvent * );

#if defined(_WS_MAC_)
    virtual bool macEvent( MSG * );		// Macintosh event
#elif defined(_WS_WIN_)
    virtual bool winEvent( MSG * );		// Windows (+NT) event
#elif defined(_WS_PM_)
    virtual bool pmEvent( QMSG * );		// OS/2 PM event
#elif defined(_WS_X11_)
    virtual bool x11Event( XEvent * );		// X11 event
#endif

  // Signals

signals:
    void    destroyed();

  // Misc. functions

public:
    QWidget    *parentWidget() const	{ return (QWidget*)QObject::parent(); }
    bool	testFlag( WFlags n ) const { return (flags & n) != 0; }
    static QWidget	 *find( WId );		// find widget by identifier
    static QWidgetMapper *wmapper()	{ return mapper; }

protected:
    WFlags	getFlags() const	{ return flags; }
    void	setFlag( WFlags n )	{ flags |= n; }
    void	clearFlag( WFlags n )	{ flags &= ~n; }
    void	setFRect( const QRect & );
    void	setCRect( const QRect & );
    bool	acceptFocus() const	{ return testFlag(WState_AcceptFocus);}
    void	setAcceptFocus( bool );
    long	metric( int )	 const;		// get metric information

    virtual bool focusNextChild();
    virtual bool focusPrevChild();

#if defined(_WS_PM_)
    int		convertYPos( int );
    void	reposChildren();
    WId		frm_wnd;
#endif

private:
    void	set_id( WId );			// set widget id
    bool	create();			// create widget
    bool	destroy();			// destroy widget
    void	createExtra();			// create extra data
    WId		ident;				// widget identifier
    WFlags	flags;				// widget flags
    QRect	frect;				// widget frame geometry
    QRect	crect;				// widget client geometry
    QColor	bg_col;				// background color
    QPalette	pal;				// widget palette
    QFont	fnt;				// widget font
    QCursor	curs;				// widget cursor
    QWExtra    *extra;				// extra widget data
    QWidget    *focusChild;			// child widget in focus
    static void createMapper();			// create widget mapper
    static void destroyMapper();		// destroy widget mapper
    static QWidgetMapper *mapper;		// maps identifier to widget
};


inline void QWidget::repaint( bool erase )
{
    repaint( rect(), erase );
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

inline void QWidget::setGeometry( const QRect &r )
{
    setGeometry( r.left(), r.top(), r.width(), r.height() );
}

inline void QWidget::drawText( const QPoint &p, const char *s )
{
    drawText( p.x(), p.y(), s );
}


#endif // QWIDGET_H
