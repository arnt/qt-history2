/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp.h#25 $
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


class QApplication				// application class
{
friend class QWidget;
public:
    QApplication( int &argc, char **argv );
    virtual ~QApplication();

    static GUIStyle style();
    static void	    setStyle( GUIStyle );

    static QCursor *cursor();			// get/set application cursor
    static void	    setCursor( const QCursor & );
    static void	    restoreCursor();

    static QPalette *palette();			// get/set application palette
    static void	    setPalette( const QPalette &, bool updateAllWidgets=FALSE);

    static QFont   *font();			// get/set application font
    static void	    setFont( const QFont &, bool updateAllWidgets=FALSE );

    static QWidget *desktop();			// get desktop widget

    int		    exec( QWidget *mainWidget );// start event handing
    int		    enter_loop();
    void	    exit_loop();
    static void	    quit( int retcode = 0 );	// quit application

    QWidget	   *mainWidget() const { return main_widget; }

    static bool	    sendEvent( QObject *receiver, QEvent *event )
	{ return qApp->notify( receiver, event ); }
    static void	    postEvent( QObject *receiver, QEvent *event );

    virtual bool    notify( QObject *, QEvent * ); // send event to object

    static bool	    startingUp();		// is application starting up?
    static bool	    closingDown();		// is application closing down?

    static void	    flushX();			// flush X output buffer
    static void	    syncX();			// syncronize with X server

protected:
    static QWidget *main_widget;		// main application widget

private:
    bool	    quit_now;			// quit flags
    int		    quit_code;
    static GUIStyle appStyle;			// application GUI style
    static QPalette *appPal;			// application palette
    static QFont   *appFont;			// application font
    static QCursor *appCursor;			// application cursor
    static bool	    starting_up;
    static bool	    closing_down;

public:
#if defined(_WS_MAC_)
    virtual bool macEventFilter( MSG * );	// Macintosh event filter
#elif defined(_WS_WIN_)
    virtual bool winEventFilter( MSG * );	// Windows event filter
#elif defined(_WS_PM_)
    virtual bool pmEventFilter( QMSG * );	// OS/2 PM event filter
#elif defined(_WS_X11_)
    virtual bool x11EventFilter( XEvent * );	// X11 event filter
#endif
};


inline GUIStyle QApplication::style()
{
    return appStyle;
}

inline QCursor *QApplication::cursor()
{
    return appCursor;
}

inline QFont *QApplication::font()
{
    return appFont;
}


#endif // QAPP_H
