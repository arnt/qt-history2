/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.h#7 $
**
** Definition of QApplication class
**
** Author  : Haavard Nord
** Created : 931107
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QAPP_H
#define QAPP_H

#include "qwidget.h"


extern int qMain( int, char ** );		// user-supplied main function
extern QApplication *qApp;			// global application object


class QApplication				// application class
{
friend class QWidget;
public:
    QApplication();
    virtual ~QApplication();

    static GUIStyle style()	{ return appStyle; }
    static void	    setStyle( GUIStyle );

    int		 exec( QWidget *mainWidget );	// start event handing
    static void	 quit( int retcode = 0 );	// quit application

    static bool	 sendEvent( QObject *object, QEvent *event )
	{ return qApp->notify( object, event ); }
    static bool	 postEvent( QObject *object, QEvent *event );

    static QWidget *desktop();			// get desktop widget
    QWidget	*mainWidget() const { return main_widget; }

    virtual bool notify( QObject *, QEvent * ); // send event to object

    static void	 cleanup();			// cleanup application

protected:
    static QWidget *main_widget;		// main application widget

private:
    bool     quit_now;				// quit flags
    int	     quit_code;
    static GUIStyle appStyle;			// application GUI style

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


#endif // QAPP_H
