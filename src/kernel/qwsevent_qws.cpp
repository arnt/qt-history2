
#include "qwsevent_qws.h"

QWSEvent *QWSEvent::factory( int type )
{
    QWSEvent *event = 0;
    switch ( type ) {
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
    case QWSEvent::PropertyNotify:
	event = new QWSPropertyNotifyEvent;
	break;
    case QWSEvent::PropertyReply:
	event = new QWSPropertyReplyEvent;
	break;
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

