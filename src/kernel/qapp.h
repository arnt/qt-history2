/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp.h#2 $
**
** Definition of QApplication class
**
** Author  : Haavard Nord
** Created : 931107
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QAPP_H
#define QAPP_H

#include "qwidget.h"


#define USE_APPNOTIFY				// undefine for faster events


class QApplication				// application class
{
friend class QWidget;
public:
    QApplication();
    virtual ~QApplication();

    int	     exec( QWidget *mainWidget );	// start event handing
    static void quit( int retcode = 0 );	// quit application

    QWidget *mainWidget() const { return main_widget; }

    virtual bool notify( QObject *, QEvent * ); // send event to object

    static void cleanup();			// cleanup application

protected:
    static QWidget *main_widget;		// main application widget

private:
    bool     quit_now;				// quit flags
    int	     quit_code;

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


extern int qMain( int, char ** );		// user-supplied main function

extern QApplication *qApp;			// global application object


#if defined(USE_APPNOTIFY)

inline bool SEND_EVENT( QObject *obj, QEvent *evt )
{						// send event to object
    return qApp->notify(obj,evt);		// via qApp
}

#else

inline bool SEND_EVENT( QObject *obj, QEvent *evt )
{						// send event to object
    return obj->event( evt );			// directly
}

#endif // USE_APPNOTIFY


#endif // QAPP_H
