/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp.h#30 $
**
** Definition of QApplication class
**
** Author  : Haavard Nord
** Created : 931107
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
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

    static GUIStyle style();			// get/set GUI style
    static void	    setStyle( GUIStyle );

    static QCursor *cursor();			// get/set application cursor
    static void	    setCursor( const QCursor & );
    static void	    restoreCursor();

    static QPalette *palette();			// get/set application palette
    static void	    setPalette( const QPalette &, bool updateAllWidgets=FALSE);

    static QFont   *font();			// get/set application font
    static void	    setFont( const QFont &, bool updateAllWidgets=FALSE );

    QWidget	   *mainWidget()  const;
    void	    setMainWidget( QWidget * );

    static QWidget *desktop();			// get desktop widget
    QWidget	   *focusWidget() const;

    int		    exec( QWidget *mainWidget );// start event handing
    int		    exec();
    int		    enter_loop();
    void	    exit_loop();
    static void	    quit( int retcode );	// quit application

    static bool	    sendEvent( QObject *receiver, QEvent *event )
	{ return qApp->notify( receiver, event ); }
    static void	    postEvent( QObject *receiver, QEvent *event );

    virtual bool    notify( QObject *, QEvent * ); // send event to object

    static bool	    startingUp();		// is application starting up?
    static bool	    closingDown();		// is application closing down?

    static void	    flushX();			// flush X output buffer
    static void	    syncX();			// syncronize with X server

#if defined(_WS_MAC_)
    virtual bool    macEventFilter( MSG * );	// Macintosh event filter
#elif defined(_WS_WIN_)
    virtual bool    winEventFilter( MSG * );	// Windows event filter
#elif defined(_WS_PM_)
    virtual bool    pmEventFilter( QMSG * );	// OS/2 PM event filter
#elif defined(_WS_X11_)
    virtual bool    x11EventFilter( XEvent * ); // X11 event filter
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
    static QWidget *main_widget;
    static QWidget *focus_widget;

    friend class QWidget;
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

inline QCursor *QApplication::cursor()
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


#endif // QAPP_H
