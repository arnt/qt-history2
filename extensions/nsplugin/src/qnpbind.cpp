// Remaining _WS_X11_ considerations:
//   - Strange after-shutdown crash
//   - What if !piApp upon NPP_NewStream?  Are we safe?
//      - Yes, but users need to know of this:  that no GUI can be
//         done until after setWindow is called.

// Remaining _WS_WIN_ considerations:
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



// Qt stuff
#include <qapp.h>
#include <qwidget.h>
#include <qobjcoll.h>
#include <qwidcoll.h>

#include "qnp.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#ifdef _WS_X11_
#define	 GC GC_QQQ
#endif

extern "C" {
//
// Netscape plugin API
//
#ifdef _WS_WIN_
#ifndef _WINDOWS
#define _WINDOWS
#endif
#endif
#ifdef _WS_X11_
#define XP_UNIX
#endif

#include "npapi.h"
//
// Stuff for the NPP_SetWindow method:
//
#ifdef _WS_X11_
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h> // for XtCreateWindow
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#endif
#ifdef _WS_WIN_
#include <windows.h>
#endif
}


struct _NPInstance
{
    NPWindow*        fWindow;
    uint16            fMode;

#ifdef _WS_WIN_
    HWND            window;
    WNDPROC            fDefaultWindowProc;
#endif

    NPP npp;

#ifdef _WS_X11_
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

/* ### debugging in ns3 needs a REALLY stderr */
#if 0
static
FILE* out()
{
    static FILE* f = 0;
    if (!f)
	f = stderr; // fdopen(4,"w"); // 2>&4 needed on cmd line for this
    return f;
}
#endif

// The single global plugin
static QNPlugin *qNP=0;
static int instance_count=0;

// The single global application
static class PluginSDK_QApplication *piApp=0;

// Temporary parameter passed `around the side' of calls to user methods
static _NPInstance* next_pi=0;

// To avoid looping when browser OR plugin can delete streams
static int qnps_no_call_back = 0;

// The currently in-focus widget.  This focus tracking is an auxiliary
// service which we provide, since we know it anyway.
static QNPWidget* focussedWidget=0;

#ifdef _WS_WIN_
// defined in qapp_win.cpp
extern "C" LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
extern bool qt_win_use_simple_timers;
#endif

#ifdef _WS_X11_
static XtEventDispatchProc qt_cascade_event_handler[LASTEvent];
static XtAppContext appcon;
void            qt_reset_color_avail();       // defined in qcol_x11.cpp
void            qt_activate_timers();         // defined in qapp_x11.cpp
timeval        *qt_wait_timer();              // defined in qapp_x11.cpp
void		qt_x11SendPostedEvents();     // defined in qapp_x11.cpp
static Boolean  qt_event_handler( XEvent* event );
#endif

class PluginSDK_QApplication : public QApplication {
#ifdef _WS_X11_
public:
    // Safe events are those that cannot result in this code being
    // dynamically unlinked during their handling by the browser,
    // and which we are interested in receiving while the mouse is
    // outside our plugin widgets.
    //
    // Dangerous are those that can.
    //
    // Blocked are those which we prevent Netscape from receiving while
    // our plugin is loaded (!).
    //
    enum FilterType { Safe, Dangerous, Blocked };

    PluginSDK_QApplication(Display* dpy) :
	QApplication(dpy)
    {
	filters_installed[Safe]=FALSE;
	filters_installed[Dangerous]=FALSE;
	filters_installed[Blocked]=FALSE;
	installXtEventFilters(Safe);
	installXtEventFilters(Blocked);
	piApp = this;
    }

    ~PluginSDK_QApplication()
    {
	removeXtEventFilters(Safe);
	removeXtEventFilters(Blocked);
	removeXtEventFilters(Dangerous);
	piApp = 0;
    }

    FilterType type(int event_type)
    {
	switch (event_type) {
	  case KeymapNotify:
	  case Expose:
	  case GraphicsExpose:
	  case NoExpose:
	  case VisibilityNotify:
	  //case ConfigureNotify:
	  //case ConfigureRequest:
	  //case GravityNotify:
	  //case CirculateNotify:
	  //case CirculateRequest:
	  case PropertyNotify:
	  case SelectionClear:
	  case SelectionRequest:
	  case SelectionNotify:
	  case ColormapNotify:
	  case ClientMessage: // Hmm... is this safe?  I want the wm_deletes
	    return Safe;
	  default:
	    return Dangerous;
	}
    }

    void installXtEventFilters(FilterType types)
    {
	if (filters_installed[types]) return;
	// Get Xt out of our face - install filter on every event type
	for (int et=2; et < LASTEvent; et++) {
	    if (type(et) == types) {
		qt_cascade_event_handler[et] = XtSetEventDispatcher(
		    qt_xdisplay(), et, qt_event_handler );
	    }
	}
	filters_installed[types] = TRUE;
    }

    void removeXtEventFilters(FilterType types)
    {
	if (!filters_installed[types]) return;
	for (int et=2; et < LASTEvent; et++) {
	    if (type(et) == types) {
		XtSetEventDispatcher(
		    qt_xdisplay(), et, qt_cascade_event_handler[et] );
	    }
	}
	filters_installed[types] = FALSE;
    }

    void removeXtEventFiltersIfOutsideQNPWidget(XLeaveWindowEvent* e)
    {
	// If QApplication doesn't know about the widget at the
	// event point, we must should remove our filters.
	// ### is widgetAt efficient enough?
	QWidget* w = QApplication::widgetAt(e->x_root, e->y_root);

	if ( !w ) {
	    if ( focussedWidget ) {
		focussedWidget->leaveInstance();
		focussedWidget = 0;
	    }
	    removeXtEventFilters( Dangerous );
	} else if ( w->isTopLevel() ) {
	    for ( QNPWidget* npw = npwidgets.first();
		npw; npw = npwidgets.next())
	    {
		if ( npw == w ) {
		    if ( focussedWidget != npw ) {
			if ( focussedWidget ) {
			    focussedWidget->leaveInstance();
			}
			focussedWidget = npw;
			focussedWidget->enterInstance();
		    }

		    break;
		}
	    }
	}
    }

    // When we are in an event loop of QApplication rather than the browser's
    // event loop (eg. for a modal dialog), we still send repaint events to
    // the browser.
    bool x11EventFilter( XEvent* e )
    {
	if ( filters_installed[Safe] ) {
	    QWidget* qw = QWidget::find( e->xany.window );
	    if ( qw ) return FALSE;
	    Widget xtw = XtWindowToWidget( e->xany.display, e->xany.window );
	    if ( xtw && type( e->type ) == Safe ) {
		// Let the browser process the event
		return qt_cascade_event_handler[e->type]( e );
	    }
	}
	return FALSE;
    }

#endif

#ifdef _WS_WIN_
private:
    static int argc;
    static char** argv;
    int mousecheck;
    int timerchoke;

public:
    PluginSDK_QApplication() :
	QApplication(argc, argv),
	mousecheck(0),
	timerchoke(0)
    {
    }

    void timerEvent(QTimerEvent* event)
    {
	if (event->timerId() == mousecheck) {
	    if ( timerchoke ) {
		timerchoke--;
		return;
	    }
	    checkFocussedWidget();
	} else {
	    QApplication::timerEvent(event);
	}
    }

    void checkFocussedWidget()
    {
	POINT curPos;
	if ( GetCursorPos( &curPos ) ) {
	    QPoint p(curPos.x, curPos.y);
	    
	    QNPWidget *newFocussedWidget = 0;
	    for ( QNPWidget* npw = npwidgets.first();
		npw; npw = npwidgets.next() )
	    {
		QRect r = npw->rect();
		r.moveTopLeft( npw->mapToGlobal(QPoint(0,0)) );
		if ( r.contains(p) ) {
		    newFocussedWidget = npw;
		    break;
		}
	    }
	    
	    if (newFocussedWidget != focussedWidget && focussedWidget)
		focussedWidget->leaveInstance();
	    
	    if (newFocussedWidget) {
		newFocussedWidget->enterInstance();
	    } else {
		killTimer( mousecheck );
		mousecheck = 0;
	    }
	    
	    focussedWidget = newFocussedWidget;
	}
    }

    bool winEventFilter( MSG *msg )
    {
	if (!mousecheck) {
	    // Only way to get "leave" events is to poll.
	    mousecheck = startTimer(200);
	    checkFocussedWidget();
	}
	if (msg->message != WM_TIMER)
	    timerchoke = 0; // ### something like this changed to 1

	return FALSE;
    }
#endif

    void addQNPWidget(QNPWidget* w)
    {
	npwidgets.append(w);
    }

    void removeQNPWidget(QNPWidget* w)
    {
	if (w == focussedWidget) focussedWidget = 0;
	npwidgets.remove(w);
    }

private:
    bool filters_installed[2];
    QList<QNPWidget> npwidgets;
};

#ifdef _WS_WIN_
int PluginSDK_QApplication::argc=0;
char **PluginSDK_QApplication::argv={ 0 };
#endif

#ifdef _WS_X11_
static XtIntervalId timerid = 0;
static void    qt_do_timers( XtPointer p, XtIntervalId* id )
{
    if (!piApp) return;

    qt_activate_timers();

    timeval *tm = qt_wait_timer();

    if (tm) {
	int interval = QMIN(tm->tv_sec,INT_MAX/1000)*1000 + tm->tv_usec/1000;

	// Ensure we only have one timeout in progress - QApplication is
	// computing the one amount of time we need to wait.
	if ( !id && timerid ) {
	    XtRemoveTimeOut( timerid );
	}
	timerid = XtAppAddTimeOut(appcon, interval, qt_do_timers, p);
    } else {
	timerid = 0;
    }
}


static int in_handler=0;
static Boolean qt_event_handler( XEvent* event )
{
    in_handler++;
    qt_x11SendPostedEvents();
    if ( piApp->x11ProcessEvent( event ) == -1 ) {
	// Problem.  This event could cause the unloading of this
	// very code (it could be a mouse-release on the Close Window
	// item for example).

	// Qt did not recognize the event
	Boolean b = True;
	if ( piApp->type(event->type) != piApp->Blocked ) {
	    b = qt_cascade_event_handler[event->type]( event );
	}
	in_handler--;
	return b;
    } else {
	// Is the event a LeaveNotify on any of our QNPWidgets?
	// If so, we must remove these event filters (see Problem above).
	if (event->type == LeaveNotify) {
	    XLeaveWindowEvent* e = (XLeaveWindowEvent*)event;
	    piApp->removeXtEventFiltersIfOutsideQNPWidget(e);
	}

	// Qt recognized the event (it may not have actually used it
	// in a widget, but that is irrelevant here).
	qt_do_timers(0,0);
	qt_reset_color_avail();
	in_handler--;
	return True;
    }
}

#endif









/*******************************************************************************
 * Plug-in Calls - these are called by Netscape
 ******************************************************************************/


// Instance state information about the plugin.

#ifdef _WS_X11_

extern "C" char*
NPP_GetMIMEDescription(void)
{
    if (!qNP) qNP = QNPlugin::actual();
    return (char*)qNP->getMIMEDescription();
}



extern "C" NPError
NPP_GetValue(void * /*future*/, NPPVariable variable, void *value)
{
    if (!qNP) qNP = QNPlugin::actual();
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
#ifdef _WS_WIN_
    qt_win_use_simple_timers = TRUE;
    // Nothing more - we do it in DLLMain
#endif

    if (!qNP) qNP = QNPlugin::actual();
    return NPERR_NO_ERROR;
}

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
    return NULL;  // None for this class
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
}

static void early_shutdown(QNPInstance* from)
{
#ifdef _WS_X11_
    if (in_handler) {
	warning("Due to over-eager unloading by your browser:\n"
	    "   %s\n"
	    "the plugin:\n"
	    "   %s\n"
	    "is being unloaded during processing of an event.  It will\n"
	    "probably now crash.  Please contact the manufacturer of your\n"
	    "browser if you are not happy with this situation.",
		from->userAgent(),
		qNP->getPluginNameString());
    }
#endif

    if (qNP) {
	delete qNP;
	qNP = 0;
    }

#ifdef _WS_X11_
    if (timerid) XtRemoveTimeOut( timerid );
#endif

    delete piApp;
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
/*
    {
	uchar* ndata;
	ndata = ((NS_Private*)(instance->ndata))->a;
	printf("New %p->ndata->a = %p:\n", instance, ndata);
	for (int i=0; i<64; i++) {
	    printf("%2x ",ndata[i]);
	    if (i%8==7) printf("\n");
	}
	ndata = (uchar*)((NS_Private*)instance->ndata)->b;
	printf("New %p->ndata->b = %p:\n", instance, ndata);
	for (int i=0; i<64; i++) {
	    printf("%2x ",ndata[i]);
	    if (i%8==7) printf("\n");
	}
    }
*/

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
    This->fWindow = NULL;
    This->fMode = mode;

    This->window = 0;

#ifdef _WS_WIN_
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
/*
    {
	uchar* ndata;
	ndata = ((NS_Private*)(instance->ndata))->a;
	printf("Destroy %p->ndata->a = %p:\n", instance, ndata);
	for (int i=0; i<64; i++) {
	    printf("%2x ",ndata[i]);
	    if (i%8==7) printf("\n");
	}
	ndata = (uchar*)((NS_Private*)instance->ndata)->b;
	printf("Destroy %p->ndata->b = %p:\n", instance, ndata);
	for (int i=0; i<64; i++) {
	    printf("%2x ",ndata[i]);
	    if (i%8==7) printf("\n");
	}
    }
*/
    _NPInstance* This;

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    This = (_NPInstance*) instance->pdata;

    if (This != NULL) {
#ifdef _WS_WIN_
	if (This->fDefaultWindowProc && This->window) {
	    SetWindowLong( This->window, GWL_WNDPROC,
		(LONG)This->fDefaultWindowProc);
	}
#endif
	if (This->widget) {
#ifdef _WS_X11_
	    This->widget->unsetWindow();
#endif
	    delete This->widget;
	}

	instance_count--;
	if (!instance_count) early_shutdown(This->instance);

	delete This->instance;
	delete [] This->argn;
	delete [] This->argv;

        delete This;
        instance->pdata = NULL;
    }

    return NPERR_NO_ERROR;
}

#ifdef _WS_WIN_
const char* gInstanceLookupString = "instance->pdata";
#endif


extern "C" NPError 
NPP_SetWindow(NPP instance, NPWindow* window)
{
    if (!qNP) qNP = QNPlugin::actual();

    NPError result = NPERR_NO_ERROR;
    _NPInstance* This;

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    This = (_NPInstance*) instance->pdata;

    /*
     * PLUGIN DEVELOPERS:
     *    Before setting window to point to the
     *    new window, you may wish to compare the new window
     *    info to the previous window (if any) to note window
     *    size changes, etc.
     */
    
    if (!window) {
	if (This->widget) {
#ifdef _WS_X11_
	    This->widget->unsetWindow();
#endif
#ifdef _WS_WIN_
	    SetWindowLong( This->window, GWL_WNDPROC,
		(LONG)This->fDefaultWindowProc);
	    This->fDefaultWindowProc = NULL;
	    This->window = NULL;
#endif
	    delete This->widget;
	    This->widget = 0;
	}
#ifdef _WS_X11_
    } else if (This->window != (Window) window->window) {
	This->window = (Window) window->window;
#endif
#ifdef _WS_WIN_
    } else if (This->window != (HWND) window->window) {
	This->window = (HWND) window->window;
#endif
	This->x = window->x;
	This->y = window->y;
	This->width = window->width;
	This->height = window->height;

#ifdef _WS_X11_
	This->display =
	    ((NPSetWindowCallbackStruct *)window->ws_info)->display;
#endif

	if (!piApp) {
#ifdef _WS_X11_
	    piApp = new PluginSDK_QApplication(This->display);
	    //XSynchronize(This->display,True);  // Helps debugging
	    appcon = XtDisplayToApplicationContext(This->display);
#endif
#ifdef _WS_WIN_
	    piApp = new PluginSDK_QApplication();
#endif
	}

	if (!This->widget) {
#ifdef _WS_WIN_
	    // At this point, we will subclass
	    // window->window so that we can begin drawing and
	    // receiving window messages.
	    This->window = (HWND) window->window;
	    SetProp( This->window, gInstanceLookupString, (HANDLE)This);

	    InvalidateRect( This->window, NULL, TRUE );
	    UpdateWindow( This->window );
#endif
	    // New widget on this new window.
	    next_pi = This;
	    /* This->widget = */ // (happens sooner - in QNPWidget constructor)
		This->instance->newWindow();
	} else {
	    // New window for existing widget, and all its children.
	    This->widget->setWindow();
	}
    } else {
	// ### Need a geometry setter that bypasses some Qt code,
	// ### but first need to know when netscape does this.
	//This->widget->setGeometry(window->x,window->y, window->width, window->height);
    }

    This->fWindow = window;
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
	if (!This->instance->newStream(qnps, sm)) {
	    return NPERR_GENERIC_ERROR;
	}
	*stype = sm;
    }

    return NPERR_NO_ERROR;
}


/* PLUGIN DEVELOPERS:
 *    These next 2 functions are directly relevant in a plug-in which
 *    handles the data in a streaming manner. If you want zero bytes
 *    because no buffer space is YET available, return 0. As long as
 *    the stream has not been written to the plugin, Navigator will
 *    continue trying to send bytes.  If the plugin doesn't want them,
 *    just return some large number from NPP_WriteReady(), and
 *    ignore them in NPP_Write().  For a NP_ASFILE stream, they are
 *    still called but can safely be ignored using this strategy.
 */

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

    if (qnps_no_call_back) {
	This = (_NPInstance*) instance->pdata;

	QNPStream* qnps = (QNPStream*)stream->pdata;

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

extern "C" void 
NPP_Print(NPP instance, NPPrint* printInfo)
{
    if(printInfo == NULL)
        return;

    if (instance != NULL) {
        // _NPInstance* This = (_NPInstance*) instance->pdata;
    
        if (printInfo->mode == NP_FULL) {
            /*
             * PLUGIN DEVELOPERS:
             *    If your plugin would like to take over
             *    printing completely when it is in full-screen mode,
             *    set printInfo->pluginPrinted to TRUE and print your
             *    plugin as you see fit.  If your plugin wants Netscape
             *    to handle printing in this case, set
             *    printInfo->pluginPrinted to FALSE (the default) and
             *    do nothing.  If you do want to handle printing
             *    yourself, printOne is true if the print button
             *    (as opposed to the print menu) was clicked.
             *    On the Macintosh, platformPrint is a THPrint; on
             *    Windows, platformPrint is a structure
             *    (defined in npapi.h) containing the printer name, port,
             *    etc.
             */

            // void* platformPrint =
            //     printInfo->print.fullPrint.platformPrint;
            // NPBool printOne =
            //     printInfo->print.fullPrint.printOne;
            
            /* Do the default*/
            printInfo->print.fullPrint.pluginPrinted = FALSE;
        }
        else {    /* If not fullscreen, we must be embedded */
            /*
             * PLUGIN DEVELOPERS:
             *    If your plugin is embedded, or is full-screen
             *    but you returned false in pluginPrinted above, NPP_Print
             *    will be called with mode == NP_EMBED.  The NPWindow
             *    in the printInfo gives the location and dimensions of
             *    the embedded plugin on the printed page.  On the
             *    Macintosh, platformPrint is the printer port; on
             *    Windows, platformPrint is the handle to the printing
             *    device context.
             */

            // NPWindow* printWindow =
            //     &(printInfo->print.embedPrint.window);
            // void* platformPrint =
            //     printInfo->print.embedPrint.platformPrint;
        }
    }
}

extern "C" void
NPP_URLNotify(NPP /*instance*/,
    const char* /*url*/,
    NPReason /*reason*/,
    void* /*notifyData*/)
{
}










/*******************************************************************************
 * The QNPWidget widget - a QWidget that is a Netscape plugin window
 ******************************************************************************/



QNPWidget::QNPWidget() :
    pi(next_pi)
{
    if (!next_pi) {
	fatal("QNPWidget must only be created within call to newWindow");
    }
    next_pi->widget = this;
    next_pi = 0;

    setWindow();

    piApp->addQNPWidget(this);
}

QNPWidget::~QNPWidget()
{
    piApp->removeQNPWidget(this);
}

void QNPWidget::enterInstance()
{
}

void QNPWidget::leaveInstance()
{
}

#ifdef _WS_X11_

// Called when a top-level widget (which has an Xt widget's window) is entered.
static
void enter_event_handler(Widget, XtPointer xtp, XEvent* event, Boolean* cont)
{
    _NPInstance* This = (_NPInstance*)xtp;

    if (piApp) {
	piApp->installXtEventFilters(PluginSDK_QApplication::Dangerous);
	if ( xtp ) {
	    if ( focussedWidget )
		focussedWidget->leaveInstance();

	    focussedWidget = This->widget;

	    if ( focussedWidget )
		focussedWidget->enterInstance();
	}
	// Post the event
	*cont = qt_event_handler(event);
    } else {
	*cont = FALSE;
    }
}

// Called when a top-level widget (which has an Xt widget's window) is left.
static
void leave_event_handler(Widget, XtPointer, XEvent*, Boolean* cont)
{
    if (piApp) {
	if ( focussedWidget ) {
	    focussedWidget->leaveInstance();
	    focussedWidget = 0;
	}
	piApp->removeXtEventFilters(PluginSDK_QApplication::Dangerous);
    }
    *cont = FALSE;
}

#endif

class QFixableWidget : public QWidget {
public:
    void fix()
    {
	QRect g = geometry();
	QColor bg = backgroundColor();
	bool mt = hasMouseTracking();
	bool hascurs = testWFlags( WCursorSet );
	QCursor curs = cursor();
	clearWFlags( WState_Created );
	clearWFlags( WState_Visible );
	create( 0, TRUE, FALSE );
	setGeometry(g);
	setBackgroundColor( bg );
	setMouseTracking( mt );
	if ( hascurs ) {
	    setCursor( curs );
	}
    }
};

static
void createNewWindowsForAllChildren(QWidget* parent, int indent=0)
{
    QObjectList* list = parent->queryList("QWidget", 0, FALSE, FALSE);

    if ( list ) {
	QObjectListIt it( *list );
	QFixableWidget* c;
	while ( (c = (QFixableWidget*)it.current()) ) {
	    bool vis = c->isVisible();
	    c->fix();
	    createNewWindowsForAllChildren(c,indent+1);
	    if ( vis ) c->show(); // Now that all children are valid.
	    ++it;
	}
	delete list;
    }
}

void QNPWidget::setWindow()
{
    saveWId = winId(); // ### Don't need this anymore

    create((WId)pi->window, FALSE, FALSE);

#ifdef _WS_X11_
    // It's open.  Believe me.
    setWFlags( WState_Visible );

    Widget w = XtWindowToWidget (qt_xdisplay(), pi->window);
    XtAddEventHandler(w, EnterWindowMask, FALSE, enter_event_handler, pi);
    XtAddEventHandler(w, LeaveWindowMask, FALSE, leave_event_handler, pi);
    Pixmap bgpm=0;
    XColor col;
    XtVaGetValues(w,
	XtNbackground, &col.pixel,
	XtNbackgroundPixmap, &bgpm,
	0, 0);
    XQueryColor(qt_xdisplay(), x11Colormap(), &col);
    setBackgroundColor(QColor(col.red >> 8, col.green >> 8, col.blue >> 8));
    if (bgpm) {
	// ### Need an under-the-hood function here, or we have to
	// ### rewrite lots of code from QPixmap::convertToImage().
	// ### Doesn't matter yet, because Netscape doesn't ever set
	// ### the background image of the window it gives us.
    }
#endif
#ifdef _WS_WIN_
    if ( !pi->fDefaultWindowProc )
	pi->fDefaultWindowProc = (WNDPROC)SetWindowLong( pi->window, GWL_WNDPROC,
	   (LONG)WndProc );
    SetWindowLong( pi->window, GWL_STYLE,
	   GetWindowLong( pi->window, GWL_STYLE ) | WS_CLIPCHILDREN );
#endif

    createNewWindowsForAllChildren(this);
}

void QNPWidget::unsetWindow()
{
#ifdef _WS_X11_
    WId wi = winId();
    Widget w = XtWindowToWidget (qt_xdisplay(), wi);
    if ( w ) {
	XtRemoveEventHandler(w, LeaveWindowMask, FALSE, leave_event_handler, pi);
	XtRemoveEventHandler(w, EnterWindowMask, FALSE, enter_event_handler, pi);
    }
#endif
#ifdef _WS_WIN_
    // Nothing special
#endif

    destroy( FALSE );
}







/*******************************************************************************
 * The QNPInstance class - a QObject that is a Netscape plugin
 * Plugins don't necessary have a window (a QNPWidget).
 ******************************************************************************/


QNPInstance::QNPInstance() :
    pi(next_pi)
{
    if (!next_pi) {
	fatal("QNPInstance must only be created within call to newInstance");
    }
    next_pi->instance = this;
    next_pi = 0;
}

QNPInstance::~QNPInstance()
{
}


QNPWidget* QNPInstance::newWindow()
{
    // No window by default
    next_pi = 0;
    return 0;
}

QNPWidget* QNPInstance::widget()
{
    return pi->widget;
}

bool QNPInstance::newStream(QNPStream*, StreamMode&)
{
    return FALSE;
}

void QNPInstance::streamAsFile(QNPStream*, const char*)
{
}

void QNPInstance::streamDestroyed(QNPStream*)
{
}

int QNPInstance::writeReady(QNPStream*)
{
    // Yes, we can handle any amount of data at once.
    return 0X0FFFFFFF;
}

int QNPInstance::write(QNPStream*, int, int len, void*)
{
    // Yes, we processed it all... into the bit bucket.
    return len;
}

void QNPInstance::getURL(const char* url, const char* window)
{
    NPN_GetURL( pi->npp, url, window );
}

void QNPInstance::postURL(const char* url, const char* window,
	     uint len, const char* buf, bool file)
{
    NPN_PostURL( pi->npp, url, window, len, buf, file );
}

int QNPInstance::argc() const
{
    return pi->argc;
}

const char* QNPInstance::argn(int i) const
{
    return pi->argn[i];
}

const char* QNPInstance::argv(int i) const
{
    return pi->argv[i];
}

const char* QNPInstance::arg(const char* name) const
{
    for (int i=0; i<pi->argc; i++) {
	// SGML: names are case insensitive
	if ( stricmp( name, pi->argn[i] ) == 0 )
	    return pi->argv[i];
    }
    return 0;
}

const char* QNPInstance::userAgent() const
{
    return NPN_UserAgent(pi->npp);
}

QNPStream* QNPInstance::newStream(const char* mimetype, const char* window,
    bool as_file)
{
    NPStream* s=0;
    NPN_NewStream(pi->npp, (char*)mimetype, window, &s);
    return s ? new QNPStream(this, mimetype, s, as_file) : 0;
}

void QNPInstance::status(const char* msg)
{
    NPN_Status(pi->npp, msg);
}



/*******************************************************************************
 * Streams from the net.
 * ### Perhaps we should let the plugin create a derived class?
 ******************************************************************************/


QNPStream::QNPStream(QNPInstance* in,const char* mt, NPStream* st, bool se) :
    inst(in),
    stream(st),
    mtype(mt),
    seek(se)
{
}

QNPStream::~QNPStream()
{
    if (!qnps_no_call_back) {
	qnps_no_call_back++;
	NPN_DestroyStream(inst->pi->npp, stream, 0);
	qnps_no_call_back--;
    }
}


const char* QNPStream::url() const
{
    return stream->url;
}

uint QNPStream::end() const
{
    return stream->end;
}

uint QNPStream::lastModified() const
{
    return stream->lastmodified;
}


const char* QNPStream::type() const
{
    return mtype;
}

bool QNPStream::seekable() const
{
    return seek;
}

void QNPStream::requestRead(int offset, uint length)
{
    NPByteRange range;
    range.offset = offset;
    range.length = length;
    range.next = 0; // ### Only one support at this time
    NPN_RequestRead(stream, &range);
}

int QNPStream::write( int len, void* buffer )
{
    return NPN_Write(inst->pi->npp, stream, len, buffer);
}





/*******************************************************************************
 * The plugin itself - only one ever exists, created by QNPlugin::actual()
 ******************************************************************************/


QNPlugin::QNPlugin()
{
}

QNPlugin::~QNPlugin()
{
}

void QNPlugin::getVersionInfo(int& plugin_major, int& plugin_minor,
	     int& browser_major, int& browser_minor)
{
    NPN_Version(&plugin_major, &plugin_minor, &browser_major, &browser_minor);
}

// Hackery for X11:  make Qt's toplevels widgets be Xt widgets too.

#ifdef _WS_X11_

// Relacement for Qt function - add Xt stuff for top-level widgets
Window qt_XCreateWindow( const QWidget* qw, Display *display, Window parent,
			 int x, int y, uint w, uint h,
			 int borderwidth, int depth,
			 uint windowclass, Visual *visual,
			 ulong valuemask, XSetWindowAttributes *attributes )
{
    if ( qw->isTopLevel() && !qw->isA("QNPWidget") ) {
	// ### not sure it is good to use name() and className().
	Widget xtw = XtVaAppCreateShell( qw->name(), qw->className(),
	    applicationShellWidgetClass, display,
	    XtNx, x, XtNy, y, XtNwidth, w, XtNheight, h,
	    XtNborderWidth, borderwidth, XtNdepth, depth,
	    0, 0 );

	// Ensure it has a window, and get it.
	XtSetMappedWhenManaged( xtw, FALSE );
	XtRealizeWidget( xtw );
	Window xw = XtWindow( xtw );

	// Set the attributes (directly)
	XChangeWindowAttributes( display, xw, valuemask, attributes );

	// Inform us on enter/leave
	XtAddEventHandler( xtw, EnterWindowMask, TRUE, enter_event_handler, 0 );
	XtAddEventHandler( xtw, LeaveWindowMask, TRUE, leave_event_handler, 0 );

	// Return Xt's window for the widget
	return xw;
    } else {
	return XCreateWindow( display, parent, x, y, w, h, borderwidth, depth,
			      windowclass, visual, valuemask, attributes );
    }
}


// Relacement for Qt function - add Xt stuff for top-level widgets
Window qt_XCreateSimpleWindow( const QWidget* qw, Display *display, Window parent,
			       int x, int y, uint w, uint h, int borderwidth,
			       ulong border, ulong background )
{
    if ( qw->isTopLevel() && !qw->isA("QNPWidget") ) {
	XSetWindowAttributes attributes;
	attributes.border_pixel = border;
	attributes.background_pixel = background;
	return qt_XCreateWindow (
	    qw, display, parent, x, y, w, h, borderwidth,
	    CopyFromParent, CopyFromParent, CopyFromParent,
	    CWBackPixel | CWBorderPixel, &attributes );
    } else {
	return XCreateSimpleWindow( display, parent, x, y, w, h, borderwidth,
				    border, background );
    }
}


// Relacement for Qt function - add Xt stuff for top-level widgets
void qt_XDestroyWindow( const QWidget* qw, Display *display, Window window )
{
    if ( qw->isTopLevel() && !qw->isA("QNPWidget") ) {
	Widget xtw = XtWindowToWidget( display, window );
	if ( xtw ) {
	    XtRemoveEventHandler(xtw, LeaveWindowMask, TRUE, leave_event_handler, 0);
	    XtRemoveEventHandler(xtw, EnterWindowMask, TRUE, enter_event_handler, 0);
	    XtDestroyWidget( xtw );
	} else {
	    XDestroyWindow( display, window );
	}
    } else {
	XDestroyWindow( display, window );
    }
}

#endif





#ifdef _WS_WIN_

BOOL   WINAPI   DllMain (HANDLE hInst, 
                        ULONG ul_reason_for_call,
                        LPVOID lpReserved)
{
    // Call Qt's WinMain
    WinMain( hInst, 0, "", SW_SHOW );
    return TRUE;
}

int main(int argc, char** argv)
{
    return 0;
}

#endif


