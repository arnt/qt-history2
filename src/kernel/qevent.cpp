/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qevent.cpp#2 $
**
** Implementation of event classes
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qevent.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qevent.cpp#2 $";
#endif


void qRemovePostedEvent( QEvent * );		// defined in qapp_xxx.cpp


void QEvent::peErrMsg()				// posted event error message
{
#if defined(CHECK_STATE)
    char *n = 0;
    switch ( t ) {				// convert type to msg string
	case Event_Timer:
	    n = "Timer";
	    break;
	case Event_MouseButtonPress:
	    n = "MouseButtonPress";
	    break;
	case Event_MouseButtonRelease:
	    n = "MouseButtonRelease";
	    break;
	case Event_MouseButtonDblClick:
	    n = "MouseButtonDblClick";
	    break;
	case Event_MouseMove:
	    n = "MouseMove";
	    break;
	case Event_KeyPress:
	    n = "KeyPress";
	    break;
	case Event_KeyRelease:
	    n = "KeyRelease";
	    break;
	case Event_FocusIn:
	    n = "FocusIn";
	    break;
	case Event_FocusOut:
	    n = "FocusOut";
	    break;
	case Event_Enter:
	    n = "Enter";
	    break;
	case Event_Leave:
	    n = "Leave";
	    break;
	case Event_Paint:
	    n = "Paint";
	    break;
	case Event_Move:
	    n = "Move";
	    break;
	case Event_Resize:
	    n = "Resize";
	    break;
	case Event_Create:
	    n = "Create";
	    break;
	case Event_Destroy:
	    n = "Destroy";
	    break;
	case Event_Show:
	    n = "Show";
	    break;
	case Event_Hide:
	    n = "Hide";
	    break;
	case Event_Close:
	    n = "Close";
	    break;
	case Event_Quit:
	    n = "Quit";
	    break;
    }
    if ( n )
	warning( "QEvent: Posted event %s cannot be stack variable, ignored",
		 n );
    else
	warning( "QEvent: Posted event %d cannot be stack variable, ignored",
		 t );
#endif
    qRemovePostedEvent( this );
}
