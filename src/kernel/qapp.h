/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp.h#68 $
**
** Definition of QApplication class
**
** Created : 931107
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QAPP_H
#define QAPP_H

#include "qwidget.h"


#if defined(TrueColor)
// X11/X.h #defines TrueColor, so it can't be used in any class or enum
#error "TrueColor already #defined by X11/X.h. Include X11/X.h after qapp.h"
#endif


extern QApplication *qApp;			// global application object


class QApplication : public QObject		// application class
{
    Q_OBJECT
public:
    QApplication( int &argc, char **argv );
#if defined(_WS_X11_)
    QApplication( Display* dpy );
#endif
    virtual ~QApplication();

    int		    argc()	const;
    char	  **argv()	const;

    static GUIStyle style();
    static void	    setStyle( GUIStyle );

#if 1	/* OBSOLETE */
    enum ColorMode { NormalColors, CustomColors };
    static QApplication::ColorMode colorMode();
    static void      setColorMode( QApplication::ColorMode );
#endif

    enum ColorSpec { NormalColor=0, CustomColor=1, PrivateColor=4,
		     ManyColor=4, TrueColor=4 };
    static int	     colorSpec();
    static void      setColorSpec( int );

    static QCursor  *overrideCursor();
    static void	     setOverrideCursor( const QCursor &, bool replace=FALSE );
    static void	     restoreOverrideCursor();

    static bool	     hasGlobalMouseTracking();
    static void	     setGlobalMouseTracking( bool enable );

    static QPalette *palette();
    static void	     setPalette( const QPalette &,bool updateAllWidgets=FALSE);

    static QFont    *font();
    static void	     setFont( const QFont &, bool updateAllWidgets=FALSE );
    static QFontMetrics fontMetrics();

    QWidget	    *mainWidget()  const;
    void	     setMainWidget( QWidget * );

    static QWidgetList *topLevelWidgets();
    static QWidget  *desktop();
    static QWidget  *activePopupWidget();
    static QWidget  *activeModalWidget();
    static QClipboard *clipboard();
    QWidget	    *focusWidget() const;

    static QWidget  *widgetAt( int x, int y, bool child=FALSE );
    static QWidget  *widgetAt( const QPoint &, bool child=FALSE );

    int		     exec();
    void	     processEvents();
    void	     processEvents( int maxtime );
    void	     processOneEvent();
    int		     enter_loop();
    void	     exit_loop();
    static void	     exit( int retcode=0 );

    static bool	     sendEvent( QObject *receiver, QEvent *event )
	{ return qApp->notify( receiver, event ); }
    static void	     postEvent( QObject *receiver, QEvent *event );

    virtual bool     notify( QObject *, QEvent * );

    static bool	     startingUp();
    static bool	     closingDown();

    static void	     flushX();
    static void	     syncX();

    static void	     beep();

#if defined(_WS_MAC_)
    virtual bool     macEventFilter( MSG * );
#elif defined(_WS_WIN_)
    virtual bool     winEventFilter( MSG * );
#elif defined(_WS_PM_)
    virtual bool     pmEventFilter( QMSG * );
#elif defined(_WS_X11_)
    virtual bool     x11EventFilter( XEvent * );
    int              x11ProcessEvent( XEvent* );
#endif

#if defined(_WS_WIN_)
    void	     winFocus( QWidget *, bool );
#endif

signals:
    void	     lastWindowClosed();

public slots:
    void	     quit();

private:
    bool	     processNextEvent( bool );
    void	     initialize( int, char ** );

    int		     app_argc;
    char	   **app_argv;
    bool	     quit_now;
    int		     quit_code;
    static GUIStyle  app_style;
    static int	     app_cspec;
    static QPalette *app_pal;
    static QFont    *app_font;
    static QCursor  *app_cursor;
    static int	     app_tracking;
    static bool	     is_app_running;
    static bool	     is_app_closing;
    static int	     loop_level;
    static QWidget  *main_widget;
    static QWidget  *focus_widget;

    friend class QWidget;
    friend class QETWidget;

private:	// Disabled copy constructor and operator=
    QApplication( const QApplication & ) {}
    QApplication &operator=( const QApplication & ) { return *this; }
};


inline int QApplication::argc() const
{
    return app_argc;
}

inline char **QApplication::argv() const
{
    return app_argv;
}

inline GUIStyle QApplication::style()
{
    return app_style;
}

inline QCursor *QApplication::overrideCursor()
{
    return app_cursor;
}

inline bool QApplication::hasGlobalMouseTracking()
{
    return app_tracking > 0;
}

inline QFont *QApplication::font()
{
    return app_font;
}

inline QWidget *QApplication::mainWidget() const
{
    return main_widget;
}

inline QWidget *QApplication::focusWidget() const
{
    return focus_widget;
}

inline QWidget *QApplication::widgetAt( const QPoint &p, bool child )
{
    return widgetAt( p.x(), p.y(), child );
}


#endif // QAPP_H
