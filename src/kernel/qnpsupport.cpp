/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qglobal.h"
#include "qapplication.h"
#include "qcolor_p.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <limits.h>

// int		qt_activate_timers();	      // in qapplication_x11.cpp
timeval	       *qt_wait_timer();	      // in qapplication_x11.cpp

typedef int (*SameAsXtEventDispatchProc)(XEvent*);
SameAsXtEventDispatchProc qt_np_cascade_event_handler[LASTEvent];
int		qt_np_count = 0;
unsigned long   qt_np_timerid = 0;
bool		qt_np_filters_installed[3]={FALSE,FALSE,FALSE};
void		(*qt_np_leave_cb)(XLeaveWindowEvent*) = 0;

typedef void (*IntervalSetter)(int);

struct QISList {
    QISList(IntervalSetter is, QISList* n) : setter(is), next(n) { }
    IntervalSetter setter;
    QISList *next;
};
static QISList* islist=0;

void qt_np_set_timer( int interval )
{
    if ( islist ) {
	// Only the first one does the work.
	islist->setter( interval );
    }
}

void qt_np_add_timer_setter( IntervalSetter is )
{
    islist = new QISList(is, islist);
}

void qt_np_remove_timer_setter( IntervalSetter is )
{
    QISList** cursor = &islist;
    while (*cursor) {
	if ((*cursor)->setter == is) {
	    QISList* n = (*cursor)->next;
	    delete *cursor;
	    *cursor = n;
	    return;
	}
	cursor = &(*cursor)->next;
    }
}



typedef void (*SameAsXtTimerCallbackProc)(void*,void*);

struct QCBList {
    QCBList(SameAsXtTimerCallbackProc cb, QCBList* n) : callback(cb), next(n) { }
    SameAsXtTimerCallbackProc callback;
    QCBList *next;
};
static QCBList* cblist=0;

void qt_np_timeout( void* p, void* id )
{
    if ( cblist ) {
	// Only the first one does the work.
	cblist->callback( p, id );
    }
}

void qt_np_add_timeoutcb( SameAsXtTimerCallbackProc cb )
{
    cblist = new QCBList(cb, cblist);
}

void qt_np_remove_timeoutcb( SameAsXtTimerCallbackProc cb )
{
    QCBList** cursor = &cblist;
    while (*cursor) {
	if ((*cursor)->callback == cb) {
	    QCBList* n = (*cursor)->next;
	    delete *cursor;
	    *cursor = n;
	    return;
	}
	cursor = &(*cursor)->next;
    }
}

void qt_np_enable_timers()
{
    if ( islist ) {
	timeval *tm = qt_wait_timer();
	if (tm) {
	    int interval = (int)QMIN(tm->tv_sec,INT_MAX/1000)*1000 + (int)tm->tv_usec/1000;
	    qt_np_set_timer(interval);
	}
    }
}

int qt_event_handler( XEvent* event )
{
    QApplication::sendPostedEvents();
    if ( qApp->x11ProcessEvent( event ) == -1
	 && !QApplication::activePopupWidget()
	 && !QApplication::activeModalWidget()
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
	// qt_activate_timers();
	qt_np_enable_timers();
	qt_reset_color_avail();
	QApplication::sendPostedEvents();
	// send the event to Xt in any case (think about XGrabPointer with ownerEvents TRUE
	if ( event->type != 7 && event->type != 8 )
	    (void) qt_np_cascade_event_handler[event->type]( event );
	return True;
    }
}


typedef void (*ForeignEventProc)(XEvent*);

struct QFEPList {
    QFEPList(ForeignEventProc fep, QFEPList* n) : callback(fep), next(n) { }
    ForeignEventProc callback;
    QFEPList *next;
};
static QFEPList* feplist=0;

void qt_np_process_foreign_event(XEvent* event)
{
    if ( feplist ) {
	// Only the first one does the work.
	feplist->callback( event );
    }
}

void qt_np_add_event_proc( ForeignEventProc fep )
{
    feplist = new QFEPList(fep, feplist);
}

void qt_np_remove_event_proc( ForeignEventProc fep )
{
    QFEPList** cursor = &feplist;
    while (*cursor) {
	if ((*cursor)->callback == fep) {
	    QFEPList* n = (*cursor)->next;
	    delete *cursor;
	    *cursor = n;
	    return;
	}
	cursor = &(*cursor)->next;
    }
}

