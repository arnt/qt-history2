
#include "qwsevent_qws.h"

QWSEvent *QWSEvent::factory( int type )
{
    QWSEvent *event = 0;
    switch ( type ) {
    case QWSEvent::Connected:
	event = new QWSConnectedEvent;
	break;
    case QWSEvent::DesktopRect:
	event = new QWSDesktopRectEvent;
	break;
    case QWSEvent::Mouse:
	event = new QWSMouseEvent;
	break;
    case QWSEvent::Focus:
	event = new QWSFocusEvent;
	break;
    case QWSEvent::Key:
	event = new QWSKeyEvent;
	break;
    case QWSEvent::RegionModified:
	event = new QWSRegionModifiedEvent;
	break;
    case QWSEvent::Creation:
	event = new QWSCreationEvent;
	break;
#ifndef QT_NO_QWS_PROPERTIES
    case QWSEvent::PropertyNotify:
	event = new QWSPropertyNotifyEvent;
	break;
    case QWSEvent::PropertyReply:
	event = new QWSPropertyReplyEvent;
	break;
#endif // QT_NO_QWS_PROPERTIES	
    case QWSEvent::SelectionClear:
	event = new QWSSelectionClearEvent;
	break;
    case QWSEvent::SelectionRequest:
	event = new QWSSelectionRequestEvent;
	break;
    case QWSEvent::SelectionNotify:
	event = new QWSSelectionNotifyEvent;
	break;
    default:
	qDebug( "QWSDisplayData::readMore() : Protocol error - got %08x!", type );
    }    
    return event;
}

