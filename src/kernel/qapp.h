/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp.h#50 $
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


extern QApplication *qApp;			// global application object


class QApplication : public QObject		// application class
{
    Q_OBJECT
public:
    QApplication( int &argc, char **argv );
    virtual ~QApplication();

    int		    argc()	const;
    char	  **argv()	const;

    static GUIStyle style();
    static void	    setStyle( GUIStyle );

    static QCursor *overrideCursor();
    static void	    setOverrideCursor( const QCursor &, bool replace=FALSE );
    static void	    restoreOverrideCursor();

    static QPalette *palette();
    static void	    setPalette( const QPalette &, bool updateAllWidgets=FALSE);

    static QFont   *font();
    static void	    setFont( const QFont &, bool updateAllWidgets=FALSE );
    static QFontMetrics fontMetrics();

    QWidget	   *mainWidget()  const;
    void	    setMainWidget( QWidget * );

    static QWidget *desktop();
    static QClipboard *clipboard();
    QWidget	   *focusWidget() const;

    static QWidget *widgetAt( int x, int y, bool child=FALSE );
    static QWidget *widgetAt( const QPoint &, bool child=FALSE );

    int		    exec();
    int		    enter_loop();
    void	    exit_loop();
    static void	    exit( int retcode=0 );

    static bool	    sendEvent( QObject *receiver, QEvent *event )
	{ return qApp->notify( receiver, event ); }
    static void	    postEvent( QObject *receiver, QEvent *event );

    virtual bool    notify( QObject *, QEvent * );

    static bool	    startingUp();
    static bool	    closingDown();

    static void	    flushX();
    static void	    syncX();

    static void	    beep();

#if defined(_WS_MAC_)
    virtual bool    macEventFilter( MSG * );
#elif defined(_WS_WIN_)
    virtual bool    winEventFilter( MSG * );
#elif defined(_WS_PM_)
    virtual bool    pmEventFilter( QMSG * );
#elif defined(_WS_X11_)
    virtual bool    x11EventFilter( XEvent * );
#endif

#if defined(_WS_WIN_)
    void	    winFocus( QWidget *, bool );
#endif

public slots:
    void	    quit();

private:
    int		    app_argc;
    char	  **app_argv;
    bool	    quit_now;
    int		    quit_code;
    static GUIStyle app_style;
    static QPalette *app_pal;
    static QFont   *app_font;
    static QCursor *app_cursor;
    static bool	    starting_up;
    static bool	    closing_down;
    static int	    loop_level;
    static QWidget *main_widget;
    static QWidget *focus_widget;

    friend class QWidget;

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
