/****************************************************************************
** $Id: $
**
** Implementation of Qt extension classes for Netscape Plugin support.
**
** Created : 970601
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


// Remaining Q_WS_X11 considerations:
//   - What if !piApp upon NPP_NewStream?  Are we safe?
//      - Yes, but users need to know of this:  that no GUI can be
//         done until after setWindow is called.
//   - Use NPN_GetValue in Communicator4.0 to get the display earlier!
//   - For ClientMessage events, trap them, and if they are not for us,
//	untrap them and retransmit them and set a timer to retrap them
//	after N seconds.

// Remaining Q_WS_WIN considerations:
//   - we need to activateZeroTimers() at some time.
//   - we need to call winEventFilter on events
//   - timers:
//    if ( msg.message == WM_TIMER ) {            // timer message received
//        activateTimer( msg.wParam );
//        return TRUE;
//    }
//    if ( msg.message == WM_KEYDOWN || msg.message == WM_KEYUP ) {
//        if ( translateKeyCode(msg.wParam) == 0 ) {
//            TranslateMessage( &msg );           // translate to WM_CHAR
//            return TRUE;
//        }
//    }
//   - qWinProcessConfigRequests?

// Remaining general stuff:
//   - Provide the "reason" parameter to streamDestroyed

// Qt stuff
#include <qapplication.h>
#include <qeventloop.h>
#include <qwidget.h>
#include <qobjectlist.h>
#include <qcursor.h>
#include <qprinter.h>
#include <qfile.h>
#include <qpainter.h>

#include "qnp.h"

#include <stdlib.h>		// Must be here for Borland C++
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#ifdef Q_WS_X11
#include "qxt.h"
#define	 GC GC_QQQ
#endif

extern "C" {
//
// Netscape plugin API
//
#ifdef Q_WS_WIN
#ifndef _WINDOWS
#define _WINDOWS
#endif
#endif
#ifdef Q_WS_X11
#define XP_UNIX
#endif

#include "npapi.h"

#ifdef Q_WS_X11
#undef XP_UNIX
#include "npunix.c"
#endif

//
// Stuff for the NPP_SetWindow function:
//
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h> // for XtCreateWindow
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
//#include <dlfcn.h>
#endif
#ifdef Q_WS_WIN
#include <windows.h>
#endif
}

#ifdef Q_WS_WIN
#include "npwin.cpp"
#endif

static QEventLoop* event_loop = 0;
static QApplication* application = 0;

struct _NPInstance
{
    uint16            fMode;

#ifdef Q_WS_WIN
    HWND            window;
    WNDPROC            fDefaultWindowProc;
#endif

    NPP npp;

#ifdef Q_WS_X11
    Window window;
    Display *display;
#endif

    uint32 x, y;
    uint32 width, height;

    QNPWidget* widget;
    QNPInstance* instance;

    int16 argc;
    QString *argn;
    QString *argv;
};



// The single global plugin
static QNPlugin *qNP=0;
static int instance_count=0;

// Temporary parameter passed `around the side' of calls to user functions
static _NPInstance* next_pi=0;

// To avoid looping when browser OR plugin can delete streams
static int qnps_no_call_back = 0;

#ifdef Q_WS_WIN
// defined in qapplication_win.cpp
Q_EXPORT extern bool qt_win_use_simple_timers;
static HHOOK hhook = 0;

LRESULT CALLBACK FilterProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if ( qApp )
	qApp->sendPostedEvents();

    return CallNextHookEx( hhook, nCode, wParam, lParam );
}

#endif

#ifdef Q_WS_X11
static int (*original_x_errhandler)( Display *dpy, XErrorEvent * ) = 0;
static int dummy_x_errhandler( Display *, XErrorEvent * )
{
    return 0;
}
#endif

/******************************************************************************
 * Plug-in Calls - these are called by Netscape
 *****************************************************************************/


// Instance state information about the plugin.

#ifdef Q_WS_X11

extern "C" char*
NPP_GetMIMEDescription(void)
{
    if (!qNP) qNP = QNPlugin::create();
    return (char*)qNP->getMIMEDescription();
}



extern "C" NPError
NPP_GetValue(void * /*future*/, NPPVariable variable, void *value)
{
    if (!qNP) qNP = QNPlugin::create();
    NPError err = NPERR_NO_ERROR;
    if (variable == NPPVpluginNameString)
	*((const char **)value) = qNP->getPluginNameString();
    else if (variable == NPPVpluginDescriptionString)
	*((const char **)value) = qNP->getPluginDescriptionString();
    else
	err = NPERR_GENERIC_ERROR;

    return err;
}

#endif

/*
** NPP_Initialize is called when your DLL is being loaded to do any
** DLL-specific initialization.
*/
extern "C" NPError
NPP_Initialize(void)
{
#ifdef Q_WS_WIN
    qt_win_use_simple_timers = TRUE;
    // Nothing more - we do it in DLLMain
#endif

    if (!qNP) qNP = QNPlugin::create();
    return NPERR_NO_ERROR;
}

static jref plugin_java_class = 0;

/*
** NPP_GetJavaClass is called during initialization to ask your plugin
** what its associated Java class is. If you don't have one, just return
** NULL. Otherwise, use the javah-generated "use_" function to both
** initialize your class and return it. If you can't find your class, an
** error will be signalled by "use_" and will cause the Navigator to
** complain to the user.
*/
extern "C" jref
NPP_GetJavaClass(void)
{
    if (!qNP) qNP = QNPlugin::create();
    plugin_java_class = (jref)qNP->getJavaClass();
    return plugin_java_class;
}

/*
** NPP_Shutdown is called when your DLL is being unloaded to do any
** DLL-specific shut-down. You should be a good citizen and declare that
** you're not using your java class any more. This allows java to unload
** it, freeing up memory.
*/
extern "C" void
NPP_Shutdown(void)
{
    if (qNP) {
	if (plugin_java_class)
	    qNP->unuseJavaClass();
	delete qNP;
	qNP = 0;
    }

#ifdef Q_WS_X11
    if ( original_x_errhandler )
    	XSetErrorHandler( original_x_errhandler );
#endif
    if ( qApp) {
#ifdef Q_WS_WIN32
	if ( hhook )
	    UnhookWindowsHookEx( hhook );
	hhook = 0;
#endif
	delete application;
	delete event_loop;
    }

}


struct NS_Private {
    uchar* a;
    uchar* b;
};

/*
** NPP_New is called when your plugin is instantiated (i.e. when an EMBED
** tag appears on a page).
*/
extern "C" NPError
NPP_New(NPMIMEType /*pluginType*/,
    NPP instance,
    uint16 mode,
    int16 argc,
    char* argn[],
    char* argv[],
    NPSavedData* /*saved*/)
{
    NPError result = NPERR_NO_ERROR;
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    instance->pdata = new _NPInstance;

    This = (_NPInstance*) instance->pdata;

    if (This == NULL)
	return NPERR_OUT_OF_MEMORY_ERROR;

    This->npp = instance;

    /* mode is NP_EMBED, NP_FULL, or NP_BACKGROUND (see npapi.h) */
    This->fMode = mode;

    This->window = 0;

#ifdef Q_WS_WIN
    This->fDefaultWindowProc = NULL;
#endif

    This->widget = 0;

    This->argc = argc;
    This->argn = new QString[argc+1];
    This->argv = new QString[argc+1];
    for (int i=0; i<This->argc; i++) {
	This->argn[i] = argn[i];
	This->argv[i] = argv[i];
    }

    // Everything is set up - we can let QNPInstance be created now.
    next_pi = This;
    qNP->newInstance();
    instance_count++;

    return result;
}

extern "C" NPError
NPP_Destroy(NPP instance, NPSavedData** /*save*/)
{
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    This = (_NPInstance*) instance->pdata;

    if (This != NULL) {
#ifdef Q_WS_WIN
	SetWindowLong( This->window, GWL_WNDPROC,
		(LONG)This->fDefaultWindowProc );
#endif

	delete This->widget;
	delete This->instance;
	delete [] This->argn;
	delete [] This->argv;

	delete This;
	instance->pdata = NULL;

	instance_count--;
    }

    return NPERR_NO_ERROR;
}


extern "C" NPError
NPP_SetWindow(NPP instance, NPWindow* window)
{
    if (!qNP) qNP = QNPlugin::create();
    NPError result = NPERR_NO_ERROR;
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    This = (_NPInstance*) instance->pdata;

    delete This->widget;

#ifdef Q_WS_WIN
    if (This->window)
	SetWindowLong( This->window, GWL_WNDPROC,
		       (LONG)This->fDefaultWindowProc );
#endif
    if ( !window )
	return result;

#ifdef Q_WS_X11
    This->window = (Window) window->window;
    This->display =
	((NPSetWindowCallbackStruct *)window->ws_info)->display;
#endif
#ifdef Q_WS_WIN
    This->window = (HWND) window->window;
    This->fDefaultWindowProc =
	(WNDPROC)GetWindowLong( This->window, GWL_WNDPROC);
#endif

    This->x = window->x;
    This->y = window->y;
    This->width = window->width;
    This->height = window->height;


    if (!qApp) {
#ifdef Q_WS_X11
	// We are the first Qt-based plugin to arrive
	event_loop = new QXt( "qnp", XtDisplayToApplicationContext(This->display) );
	application = new QApplication(This->display);
#endif
#ifdef Q_WS_WIN
	static int argc=0;
	static char **argv={ 0 };
	application = new QApplication( argc, argv );
#ifdef UNICODE
	if ( qWinVersion() & Qt::WV_NT_based )
	    hhook = SetWindowsHookExW( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
	else
#endif
	    hhook = SetWindowsHookExA( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
#endif
    }

#ifdef Q_WS_X11
    if ( !original_x_errhandler )
    	original_x_errhandler = XSetErrorHandler( dummy_x_errhandler );
#endif

    // New widget on this new window.
    next_pi = This;
    /* This->widget = */ // (happens sooner - in QNPWidget constructor)
    This->instance->newWindow();

#ifdef Q_WS_X11
    This->widget->resize( This->width, This->height );
    XReparentWindow( This->widget->x11Display(), This->widget->winId(), This->window, 0, 0 );
    XSync( This->widget->x11Display(), False );
#endif
#ifdef Q_WS_WIN
#endif
    This->widget->show();
    return result;
}


extern "C" NPError
NPP_NewStream(NPP instance,
	  NPMIMEType type,
	  NPStream *stream,
	  NPBool seekable,
	  uint16 *stype)
{
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    This = (_NPInstance*) instance->pdata;

    if ( This ) {
	QNPStream* qnps = new QNPStream(This->instance,type,stream,seekable);
	stream->pdata = qnps;
	QNPInstance::StreamMode sm = (QNPInstance::StreamMode)*stype;
	if (!This->instance->newStreamCreated(qnps, sm)) {
	    return NPERR_GENERIC_ERROR;
	}
	*stype = sm;
    }

    return NPERR_NO_ERROR;
}


int32 STREAMBUFSIZE = 0X0FFFFFFF; /* If we are reading from a file in NPAsFile
				   * mode so we can take any size stream in our
				   * write call (since we ignore it) */

extern "C" int32
NPP_WriteReady(NPP instance, NPStream *stream)
{
    _NPInstance* This;
    if (instance != NULL) {
	This = (_NPInstance*) instance->pdata;
    } else {
	// Yikes, that's unusual!
	return 0;
    }

    if (This) {
	return This->instance->writeReady((QNPStream*)stream->pdata);
    }

    /* Number of bytes ready to accept in NPP_Write() */
    return STREAMBUFSIZE;
}


extern "C" int32
NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
    if (instance != NULL)
    {
	_NPInstance* This = (_NPInstance*) instance->pdata;

	if (This) {
	    return This->instance->write((QNPStream*)stream->pdata,
		offset, len, buffer);
	}
    }

    return len;        /* The number of bytes accepted */
}


extern "C" NPError
NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    if (!qnps_no_call_back) {
	This = (_NPInstance*) instance->pdata;

	QNPStream* qnps = (QNPStream*)stream->pdata;
	if ( qnps )
	    switch (reason) {
		case NPRES_DONE:
		    qnps->setComplete(TRUE);
		    break;
		case NPRES_USER_BREAK:
		    break;
		case NPRES_NETWORK_ERR:
		    qnps->setOkay(FALSE);
		    break;
	    }

	if (This) {
	    // Give the instance a chance to do something
	    This->instance->streamDestroyed(qnps);
	}

	qnps_no_call_back++;
	delete qnps;
	qnps_no_call_back--;
    }

    return NPERR_NO_ERROR;
}


extern "C" void
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
    _NPInstance* This;

    if (instance == NULL) return;

    This = (_NPInstance*) instance->pdata;

    if ( This ) {
	QNPStream* qnps = (QNPStream*)stream->pdata;
	This->instance->streamAsFile(qnps, fname);
    }
}

typedef struct
{
    int32    type;
    FILE*    fp;
} NPPrintCallbackStruct;

#ifdef Q_WS_X11

class QNPPrinter : public QPrinter {
    QFile file;
public:
    QNPPrinter(FILE* fp)
    {
	file.open(IO_WriteOnly, fp);
	QPDevCmdParam param;
	param.device = &file;
	cmd(PdcSetdev, 0, &param);
    }
    void end()
    {
	QPDevCmdParam param;
	param.device = 0;
	cmd(PdcSetdev, 0, &param);
    }
};
#endif

extern "C" void
NPP_Print(NPP instance, NPPrint* printInfo)
{
    if(printInfo == NULL)
	return;

    if (instance != NULL) {
	_NPInstance* This = (_NPInstance*) instance->pdata;

	if (printInfo->mode == NP_FULL) {
	    printInfo->print.fullPrint.pluginPrinted =
		This->instance->printFullPage();
	} else if (printInfo->mode == NP_EMBED) {
#ifdef Q_WS_X11
	    void* platformPrint =
		printInfo->print.embedPrint.platformPrint;
	    FILE* outfile = ((NPPrintCallbackStruct*)platformPrint)->fp;
	    if (ftell(outfile)) {
// 		NPWindow* w =
// 		    &(printInfo->print.embedPrint.window);
		QNPPrinter prn(outfile);
		QPainter painter(&prn);
		// #### config viewport with w->{x,y,width,height}
		This->instance->print(&painter);
		prn.end();
	    } else {
		// Why does the browser make spurious NPP_Print calls?
	    }
#endif
#ifdef Q_WS_WIN
	    NPWindow* printWindow =
		&(printInfo->print.embedPrint.window);
	    void* platformPrint =
		printInfo->print.embedPrint.platformPrint;
	    // #### Nothing yet.
#endif
	}
    }
}

extern "C" void
NPP_URLNotify(NPP instance,
	      const char* url,
	      NPReason reason,
	      void* notifyData)
{
    if (instance != NULL) {
	QNPInstance::Reason r;
	switch (reason) {
	case NPRES_DONE:
	    r = QNPInstance::ReasonDone;
	    break;
	case NPRES_USER_BREAK:
	    r = QNPInstance::ReasonBreak;
	    break;
	case NPRES_NETWORK_ERR:
	    r = QNPInstance::ReasonError;
	    break;
	default:
	    r = QNPInstance::ReasonUnknown;
	    break;
	}
	_NPInstance* This = (_NPInstance*) instance->pdata;
	This->instance->notifyURL(url, r, notifyData);
    }
}



#ifdef Q_WS_WIN

BOOL   WINAPI   DllMain (HANDLE hInst,
			ULONG ul_reason_for_call,
			LPVOID lpReserved)
{
    return TRUE;
}

#endif



/*!
    \class QNPWidget qnp.h
    \brief The QNPWidget class provides a QWidget that is a Web-browser plugin window.

    \extension NSPlugin

    Derive from QNPWidget to create a widget that can be used as a
    Browser plugin window, or create one and add child widgets.
    Instances of QNPWidget may only be created when
    QNPInstance::newWindow() is called by the browser.

    A common way to develop a plugin widget is to develop it as a
    stand-alone application window, then make it a \e child of a
    plugin widget to use it as a browser plugin. The technique is:

\code
class MyPluginWindow : public QNPWidget
{
    QWidget* child;
public:
    MyPluginWindow()
    {
	// Some widget that is normally used as a top-level widget
	child = new MyIndependentlyDevelopedWidget();

	// Use the background color of the web page
	child->setBackgroundColor( backgroundColor() );

	// Fill the plugin widget
	child->setGeometry( 0, 0, width(), height() );
    }

    void resizeEvent(QResizeEvent*)
    {
	// Fill the plugin widget
	child->resize(size());
    }
};
\endcode

    The default implementation is an empty window.
*/

/*!
    Creates a QNPWidget.
*/
QNPWidget::QNPWidget() :
     pi(next_pi)
{
    if (!next_pi) {
	qFatal("QNPWidget must only be created within call to newWindow");
    }
    next_pi->widget = this;
    next_pi = 0;
}

/*!
    Destroys the window. This will be called by the plugin binding
    code when the window is no longer required. The Web-browser will
    delete windows when they leave the page. The bindings will change
    the QWidget::winId() of the window when the window is resized, but
    this should not affect normal widget behavior.
*/
QNPWidget::~QNPWidget()
{
#ifdef Q_WS_X11
    destroy( FALSE, FALSE ); // X has destroyed all windows
#endif
}


/*!\internal */
void QNPWidget::enterEvent(QEvent*)
{
    enterInstance();
}

/*!\internal */
void QNPWidget:: leaveEvent(QEvent*)
{
    if ( !QApplication::activePopupWidget() )
	leaveInstance();
}

/*!
    Called when the mouse enters the plugin window. Does nothing by
    default.
*/
void QNPWidget::enterInstance()
{
}

/*!
    Called when the mouse leaves the plugin window. Does nothing by
    default.
*/
void QNPWidget::leaveInstance()
{
}

/*!
    Returns the instance for which this widget is the window.
*/
QNPInstance* QNPWidget::instance()
{
    return pi->instance;
}





/*!
    \class QNPInstance qnp.h
    \brief The QNPInstance class provides a QObject that is a Web-browser plugin.

    \extension NSPlugin

    Deriving from QNPInstance creates an object that represents a
    single \c{<EMBED>} tag in an HTML document.

    The QNPInstance is responsible for creating an appropriate window
    if required (not all plugins have windows), and for interacting
    with the input/output facilities intrinsic to plugins.

    Note that there is \e{absolutely no guarantee} regarding the order
    in which functions are called. Sometimes the browser will call
    newWindow() first, at other times, newStreamCreated() will be
    called first (assuming the \c{<EMBED>} tag has a SRC parameter).

    \e{None of Qt's GUI functionality} may be used until after the
    first call to newWindow(). This includes any use of QPaintDevice
    (ie. QPixmap, QWidget, and all subclasses), QApplication, anything
    related to QPainter (QBrush, etc.), fonts, QMovie, QToolTip, etc.
    Useful classes which specifically \e can be used are QImage,
    QFile, and QBuffer.

    This restriction can easily be accommodated by structuring your
    plugin so that the task of the QNPInstance is to gather data,
    while the task of the QNPWidget is to provide a graphical
    interface to that data,
*/

/*!
    \enum QNPInstance::InstanceMode

    This enum type provides Qt-style names for three #defines in
    \c npapi.h:

    \value Embed - corresponds to NP_EMBED
    \value Full - corresponds to NP_FULL
    \value Background - corresponds to NP_BACKGROUND

*/

/*!
    \enum QNPInstance::Reason

    \value ReasonDone
    \value ReasonBreak
    \value ReasonError
    \value ReasonUnknown
*/

/*!
    \enum QNPInstance::StreamMode

    \value Normal
    \value Seek
    \value AsFile
    \value AsFileOnly
*/

/*!
    Creates a QNPInstance.

    Can only be called from within a derived class created within
    QNPlugin::newInstance().
*/
QNPInstance::QNPInstance() :
    pi(next_pi)
{
    if (!next_pi) {
	qFatal("QNPInstance must only be created within call to newInstance");
    }
    next_pi->instance = this;
    next_pi = 0;
}

/*!
    Called when the plugin instance is about to disappear.
*/
QNPInstance::~QNPInstance()
{
}

/*!
    Called at most once, at some time after the QNPInstance is
    created. If the plugin requires a window, this function should
    return a derived class of QNPWidget that provides the required
    interface.
*/
QNPWidget* QNPInstance::newWindow()
{
    // No window by default
    next_pi = 0;
    return 0;
}

/*!
    Returns the plugin window created by newWindow(), if any.
*/
QNPWidget* QNPInstance::widget()
{
    return pi->widget;
}

/*!
    \fn bool QNPInstance::newStreamCreated(QNPStream*, StreamMode& smode)

    This function is called when a new stream has been created. The
    instance should return TRUE if it accepts the processing of the
    stream. If the instance requires the stream as a file, it should
    set \a smode to \c AsFileOnly, in which case the data will be
    delivered some time later to the streamAsFile() function.
    Otherwise, the data will be delivered in chunks to the write()
    function which must consume at least as much data as was returned
    by the most recent call to writeReady().

    Note that the \c AsFileOnly method is not supported by Netscape
    2.0 and MSIE 3.0.
*/
bool QNPInstance::newStreamCreated(QNPStream*, StreamMode&)
{
    return FALSE;
}

/*!
    Called when a stream is delivered as a single file called \a fname
    rather than as chunks. This may be simpler for a plugin to deal
    with, but precludes any incremental behavior.

    Note that the \c AsFileOnly method is not supported by Netscape
    2.0 and MSIE 3.0.

    \sa newStreamCreated(), newStream()
*/
void QNPInstance::streamAsFile(QNPStream*, const char*)
{
}

/*!
    Called when a stream is destroyed. At this point, the stream may
    be complete() and okay(). If it is not okay(), then an error has
    occurred. If it is okay(), but not complete(), then the user has
    cancelled the transmission: do not give an error message in this
    case.
*/
void QNPInstance::streamDestroyed(QNPStream*)
{
}

/*!
    Returns the minimum amount of data the instance is willing to
    receive from the given stream.

    The default returns a very large value.
*/
int QNPInstance::writeReady(QNPStream*)
{
    // Yes, we can handle any amount of data at once.
    return 0X0FFFFFFF;
}

/*!
    \fn int QNPInstance::write(QNPStream*, int offset, int len, void* buffer)

    Called when incoming data is available for processing by the
    instance. The instance \e must consume at least the amount that it
    returned in the most recent call to writeReady(), but it may
    consume up to the amount given by \a len. \a buffer is the data
    available for consumption. The \a offset argument is merely an
    informational value indicating the total amount of data that has
    been consumed in prior calls.

    This function should return the amount of data actually consumed.
*/
int QNPInstance::write(QNPStream*, int, int len, void*)
{
    // Yes, we processed it all... into the bit bucket.
    return len;
}

/*!
    Requests that the \a url be retrieved and sent to the named \a
    window. See Netscape's JavaScript documentation for an explanation
    of window names.
*/
void QNPInstance::getURL(const char* url, const char* window)
{
    NPN_GetURL( pi->npp, url, window );
}

/*!
    \preliminary

    This function is \e{not tested}.

    It is an interface to the NPN_PostURL function of the Netscape
    Plugin API.

    Passes \a url, \a window, \a buf, \a len, and \a file to
    NPN_PostURL.
*/
void QNPInstance::postURL(const char* url, const char* window,
	     uint len, const char* buf, bool file)
{
    NPN_PostURL( pi->npp, url, window, len, buf, file );
}

/*!
    Print the instance full-page. By default, this returns FALSE,
    causing the browser to call the (embedded) print() function
    instead. Requests that the given \a url be retrieved and sent to
    the named \a window. See Netscape's JavaScript documentation for
    an explanation of window names. Passes the arguments including \a
    data to NPN_GetURLNotify.

    \sa
    \link http://developer.netscape.com/docs/manuals/communicator/plugin/refpgur.htm#npngeturlnotify
    Netscape: NPN_GetURLNotify method\endlink
*/
void QNPInstance::getURLNotify(const char* url, const char* window, void*data)
{
#ifdef Q_WS_WIN // Only on Windows?
    NPN_GetURLNotify( pi->npp, url, window, data );
#endif
}

/*!
    \preliminary

    This function is \e{not tested}.

    It is an encapsulation of the NPP_Print function of the Netscape
    Plugin API.
*/
bool QNPInstance::printFullPage()
{
    return FALSE;
}

/*!
    \preliminary

    This function is \e{not tested}.

    Print the instance embedded in a page.

    It is an encapsulation of the NPP_Print function of the Netscape
    Plugin API.
*/
void QNPInstance::print(QPainter*)
{
    // ### default could redirected-print the window.
}

/*!
    Returns the number of arguments to the instance. Note that you
    should not normally rely on the ordering of arguments, and also
    note that the SGML specification does not permit multiple
    arguments with the same name.

    \sa arg()
*/
int QNPInstance::argc() const
{
    return pi->argc;
}

/*!
    Returns the name of the \a{i}-th argument. See argc().
*/
const char* QNPInstance::argn(int i) const
{
    return pi->argv[i];
}

/*!
    \preliminary

    This function is \e{not tested}.

    Called whenever a \a url is notified after a call to
    NPN_GetURLNotify with \a notifyData. The reason is given in \a r.

    It is an encapsulation of the NPP_URLNotify function of the
    Netscape Plugin API.

    See also:
    \link http://developer.netscape.com/docs/manuals/communicator/plugin/refpgur.htm#nppurlnotify
    Netscape: NPP_URLNotify method\endlink
*/
void QNPInstance::notifyURL(const char*, Reason, void*)
{
}

/*!
    Returns the value of the \a{i}-th argument. See argc().
*/
const char* QNPInstance::argv(int i) const
{
    return pi->argv[i];
}

/*!
    Returns the mode of the plugin.
*/
QNPInstance::InstanceMode QNPInstance::mode() const
{
    return (QNPInstance::InstanceMode)pi->fMode;
}

/*!
    Returns the value of the named arguments, or 0 if no argument
    called \a name appears in the \c{<EMBED>} tag of this instance.
    If the argument appears, but has no value assigned, the empty
    string is returned. In summary:

    \table
    \header \i Tag \i Result
    \row \i \c{<EMBED ...>} \i arg("FOO") == 0
    \row \i \c{<EMBED FOO ...>} \i arg("FOO") == ""
    \row \i \c{<EMBED FOO=BAR ...>} \i arg("FOO") == "BAR"
    \endtable
*/
const char* QNPInstance::arg(const char* name) const
{
    for (int i=0; i<pi->argc; i++) {
	// SGML: names are case insensitive
	if ( qstricmp( name, pi->argn[i] ) == 0 ) {
	    if (pi->argv[i].isEmpty())
		return "";
	    else
		return pi->argv[i];
	}
    }
    return 0;
}

/*!
    Returns the user agent (browser name) containing this instance.
*/
const char* QNPInstance::userAgent() const
{
    return NPN_UserAgent(pi->npp);
}

/*!
    \preliminary

    This function is \e{not tested}.

    Requests the creation of a new data stream \e from the plug-in.
    The mime type and window are passed in \a mimetype and \a window.
    \a as_file holds the \c AsFileOnly flag. It is an interface to the
    NPN_NewStream function of the Netscape Plugin API.
*/
QNPStream* QNPInstance::newStream(const char* mimetype, const char* window,
    bool as_file)
{
    NPStream* s=0;
    NPError err = NPN_NewStream(pi->npp, (char*)mimetype, window, &s);
    if (err != NPERR_NO_ERROR) return 0;
    return s ? new QNPStream(this, mimetype, s, as_file) : 0;
}

/*!
    Sets the status message in the browser containing this instance to
    \a msg.
*/
void QNPInstance::status(const char* msg)
{
    NPN_Status(pi->npp, msg);
}


/*!
    Returns the Java object associated with the plug-in instance, an
    object of the \link QNPlugin::getJavaClass() plug-in's Java
    class\endlink, or 0 if the plug-in does not have a Java class,
    Java is disabled, or an error occurred.

    The return value is actually a \c{jref} we use \c{void*} so as to
    avoid burdening plugins which do not require Java.

    \sa QNPlugin::getJavaClass(), QNPlugin::getJavaEnv(), getJavaPeer()
*/
void* QNPInstance::getJavaPeer() const
{
    return NPN_GetJavaPeer(pi->npp);
}


/*!
    \class QNPStream qnp.h
    \brief The QNPStream class provides a stream of data provided to a QNPInstance by the browser.

    \extension NSPlugin

    Note that this is neither a QTextStream nor a QDataStream.

    \sa QNPInstance::write(), QNPInstance::newStreamCreated()
*/

/*!
    Creates a stream. Plugins should not call this, but rather
    QNPInstance::newStream() if a stream is required.

    Takes a QNPInstance \a in, mime type \a mt, a pointer to an
    _NPStream \a st and a seekable flag \a se.
*/
QNPStream::QNPStream(QNPInstance* in,const char* mt, _NPStream* st, bool se) :
    inst(in),
    stream(st),
    mtype(mt),
    seek(se)
{
    isokay = TRUE;
    iscomplete = FALSE;
}

/*!
    Destroys the stream.
*/
QNPStream::~QNPStream()
{
    if (!qnps_no_call_back) {
	qnps_no_call_back++;
	NPN_DestroyStream(inst->pi->npp, stream, NPRES_USER_BREAK);
	qnps_no_call_back--;
    }
}

/*!
    \fn QNPInstance* QNPStream::instance()

    Returns the QNPInstance for which this stream was created.
*/

/*!
    Returns the URL from which the stream was created.
*/
const char* QNPStream::url() const
{
    return stream->url;
}

/*!
    Returns the length of the stream in bytes. Can be 0 for streams of
    unknown length.
*/
uint QNPStream::end() const
{
    return stream->end;
}

/*!
    Returns the time when the source of the stream was last modified.
*/
uint QNPStream::lastModified() const
{
    return stream->lastmodified;
}

/*!
    Returns the MIME type of the stream.
*/
const char* QNPStream::type() const
{
    return mtype;
}

/*!
    Returns TRUE if the stream is seekable; otherwise returns FALSE.
*/
bool QNPStream::seekable() const
{
    return seek;
}

/*!
  \internal
*/
void QNPStream::setOkay(bool y)
{
    isokay = y;
}

/*!
  \internal
*/
void QNPStream::setComplete(bool y)
{
    iscomplete = y;
}

/*!
    Returns TRUE if no errors have occurred on the stream; otherwise
    returns FALSE.
*/
bool QNPStream::okay() const
{
    return isokay;
}

/*!
    Returns TRUE if the stream has received all the data from the
    source; otherwise returns FALSE.
*/
bool QNPStream::complete() const
{
    return iscomplete;
}

/*!
    Requests the section of the stream, of \a length bytes from \a
    offset, be sent to the QNPInstance::write() function of the
    instance() of this stream.
*/
void QNPStream::requestRead(int offset, uint length)
{
    NPByteRange range;
    range.offset = offset;
    range.length = length;
    range.next = 0; // ### Only one supported at this time
    NPN_RequestRead(stream, &range);
}

/*!
    Writes \a len bytes from \a buffer \e to the stream.
*/
int QNPStream::write( int len, void* buffer )
{
    return NPN_Write(inst->pi->npp, stream, len, buffer);
}



/******************************************************************************
 * The plugin itself - only one ever exists, created by QNPlugin::create()
 *****************************************************************************/


/*!
    \class QNPlugin qnp.h
    \brief The QNPlugin class provides the plugin central factory.

    \extension NSPlugin

    This class is the heart of the plugin. One instance of this object
    is created when the plugin is \e first needed, by calling
    QNPlugin::create(), which must be implemented in your plugin code
    to return some derived class of QNPlugin. The one QNPlugin object
    creates all instances for a single running Web-browser process.

    Additionally, if Qt is linked to the plugin as a dynamic library,
    only one instance of QApplication will exist \e{across all plugins
    that have been made with Qt}. So, your plugin should tread lightly
    on global settings - do not, for example, use
    QApplication::setFont() - that will change the font in every
    widget of every Qt-based plugin currently loaded!
*/

/*!
    \fn QNPlugin* QNPlugin::create()

    This must be implemented by your plugin code. It should return a
    derived class of QNPlugin.
*/

/*!
    Returns the plugin most recently returned by QNPlugin::create().
*/
QNPlugin* QNPlugin::actual()
{
    return qNP;
}

/*!
    Creates a QNPlugin. This may only be used by the constructor
    derived class returned by the plugin's implementation of the
    QNPlugin::create() function.
*/
QNPlugin::QNPlugin()
{
    // Encourage linker to include stuff.
    static void* a;
    a = (void*)NP_Initialize;
    a = (void*)NP_Shutdown;
}

/*!
    Destroys the QNPlugin. This is called by the plugin binding code
    just before the plugin is about to be unloaded from memory. If
    newWindow() has been called, a QApplication will still exist at
    this time, but will be deleted shortly after before the plugin is
    deleted.
*/
QNPlugin::~QNPlugin()
{
}

/*!
    Populates \e *\a plugin_major and \e *\a plugin_minor with the
    version of the plugin API and populates \e *\a browser_major and
    \e *\a browser_minor with the version of the browser.
*/
void QNPlugin::getVersionInfo(int& plugin_major, int& plugin_minor,
	     int& browser_major, int& browser_minor)
{
    NPN_Version(&plugin_major, &plugin_minor, &browser_major, &browser_minor);
}

/*!
    \fn QNPInstance* QNPlugin::newInstance()

    Override this to return an appropriate derived class of
    QNPInstance.
*/

/*!
    \fn const char* QNPlugin::getMIMEDescription() const

    Override this to return the MIME description of the data formats
    supported by your plugin. The format of this string is shown by
    the following example:

\code
    const char* getMIMEDescription() const
    {
	return "image/x-png:png:PNG Image;"
	       "image/png:png:PNG Image;"
	       "image/x-portable-bitmap:pbm:PBM Image;"
	       "image/x-portable-graymap:pgm:PGM Image;"
	       "image/x-portable-pixmap:ppm:PPM Image;"
	       "image/bmp:bmp:BMP Image;"
	       "image/x-ms-bmp:bmp:BMP Image;"
	       "image/x-xpixmap:xpm:XPM Image;"
	       "image/xpm:xpm:XPM Image";
    }
\endcode
*/

/*!
    \fn const char* QNPlugin::getPluginNameString() const

    Returns the plain-text name of the plugin.
*/

/*!
    \fn const char* QNPlugin::getPluginDescriptionString() const

    Returns the plain-text description of the plugin.
*/

/*!
    Override to return a reference to the Java class that represents
    the plugin. The default returns 0, indicating no class.

    If you override this class, you must also override
    QNPlugin::unuseJavaClass().

    The return value is actually a \c{jref}; we use \c{void*} so as to
    avoid burdening plugins which do not require Java.

    \sa getJavaEnv(), QNPInstance::getJavaPeer()
*/
void* QNPlugin::getJavaClass()
{
    return NULL;
}

/*!
    This function is called when the plugin is shutting down, with \e
    jc set to the value returned earlier by getJavaClass(). The
    function should \e unuse the Java class.
*/
void QNPlugin::unuseJavaClass()
{
    qFatal("QNPlugin::unuseJavaClass() must be overridden along with getJavaClass()");
}

/*!
    Returns a pointer to the Java execution environment, or 0 if Java
    is disabled or an error occurred.

    The return value is actually a \c{JRIEnv*}; we use \c{void*} so as
    to avoid burdening plugins which do not require Java.

    \sa getJavaClass(), QNPInstance::getJavaPeer()
*/
void* QNPlugin::getJavaEnv() const
{
    return NPN_GetJavaEnv();
}
