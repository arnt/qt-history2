/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnpsupport.cpp#3 $
**
** Low-level support for Netscape Plugins.
** This has to go in the dynamic library, because otherwise it may
** be loaded out too early (we consider that a netscape bug).
**
** Created : 970601
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <limits.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qnpsupport.cpp#3 $");


void            qt_reset_color_avail();       // defined in qcol_x11.cpp
void            qt_activate_timers();         // defined in qapp_x11.cpp
timeval        *qt_wait_timer();              // defined in qapp_x11.cpp
void            qt_x11SendPostedEvents();     // defined in qapp_x11.cpp

typedef int (*SameAsXtEventDispatchProc)(XEvent*);
SameAsXtEventDispatchProc qt_np_cascade_event_handler[LASTEvent];
int		qt_np_count = 0;
unsigned long   qt_np_timerid = 0;
bool		qt_np_filters_installed[3]={FALSE,FALSE,FALSE};
void		(*qt_np_leave_cb)(XLeaveWindowEvent*) = 0;

typedef void (*IntervalSetter)(int);

struct ISList {
    ISList(IntervalSetter is, ISList* n) : setter(is), next(n) { }
    IntervalSetter setter;
    ISList *next;
};
ISList* islist=0;

void qt_np_set_timer( int interval )
{
    if ( islist ) {
	// Only the first one does the work.
	islist->setter( interval );
    }
}

void qt_np_add_timer_setter( IntervalSetter is )
{
    islist = new ISList(is, islist);
}

void qt_np_remove_timer_setter( IntervalSetter is )
{
    ISList** cursor = &islist;
    while (*cursor) {
	if ((*cursor)->setter == is) {
	    ISList* n = (*cursor)->next;
	    delete *cursor;
	    *cursor = n;
	    return;
	}
	cursor = &(*cursor)->next;
    }
}



typedef void (*SameAsXtTimerCallbackProc)(void*,void*);

struct CBList {
    CBList(SameAsXtTimerCallbackProc cb, CBList* n) : callback(cb), next(n) { }
    SameAsXtTimerCallbackProc callback;
    CBList *next;
};
CBList* cblist=0;

void qt_np_timeout( void* p, void* id )
{
    if ( cblist ) {
	// Only the first one does the work.
	cblist->callback( p, id );
    }
}

void qt_np_add_timeoutcb( SameAsXtTimerCallbackProc cb )
{
    cblist = new CBList(cb, cblist);
}

void qt_np_remove_timeoutcb( SameAsXtTimerCallbackProc cb )
{
    CBList** cursor = &cblist;
    while (*cursor) {
	if ((*cursor)->callback == cb) {
	    CBList* n = (*cursor)->next;
	    delete *cursor;
	    *cursor = n;
	    return;
	}
	cursor = &(*cursor)->next;
    }
}


int qt_event_handler( XEvent* event )
{
    qt_x11SendPostedEvents();
    if ( qApp->x11ProcessEvent( event ) == -1
	&& !QApplication::activePopupWidget()
	&& !QApplication::activePopupWidget()
    ) {
        // Qt did not recognize the event
	return qt_np_cascade_event_handler[event->type]( event );
    } else {
        // Qt recognized the event (it may not have actually used it
        // in a widget, but that is irrelevant here).
	if ( event->type == LeaveNotify && qt_np_leave_cb
	  && !QApplication::activePopupWidget()
          && !QApplication::activeModalWidget())
	{
	    XLeaveWindowEvent* e = (XLeaveWindowEvent*)event;
	    qt_np_leave_cb(e);
	}
        if ( islist ) {
	    qt_activate_timers();
	    timeval *tm = qt_wait_timer();
	    if (tm) {
		int interval = QMIN(tm->tv_sec,INT_MAX/1000)*1000 + tm->tv_usec/1000;
		qt_np_set_timer(interval);
	    }
	}
        qt_reset_color_avail();
        return True;
    }
}


typedef void (*ForeignEventProc)(XEvent*);

struct FEPList {
    FEPList(ForeignEventProc fep, FEPList* n) : callback(fep), next(n) { }
    ForeignEventProc callback;
    FEPList *next;
};
FEPList* feplist=0;

void qt_np_process_foreign_event(XEvent* event)
{
    if ( feplist ) {
	// Only the first one does the work.
	feplist->callback( event );
    }
}

void qt_np_add_event_proc( ForeignEventProc fep )
{
    feplist = new FEPList(fep, feplist);
}

void qt_np_remove_event_proc( ForeignEventProc fep )
{
    FEPList** cursor = &feplist;
    while (*cursor) {
	if ((*cursor)->callback == fep) {
	    FEPList* n = (*cursor)->next;
	    delete *cursor;
	    *cursor = n;
	    return;
	}
	cursor = &(*cursor)->next;
    }
}
