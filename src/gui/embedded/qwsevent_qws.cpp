/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwsevent_qws.h"

QWSEvent *QWSEvent::factory(int type)
{
    QWSEvent *event = 0;
    switch (type) {
    case QWSEvent::Connected:
        event = new QWSConnectedEvent;
        break;
    case QWSEvent::MaxWindowRect:
        event = new QWSMaxWindowRectEvent;
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
    case QWSEvent::Region:
        event = new QWSRegionEvent;
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
#ifndef QT_NO_COP
    case QWSEvent::QCopMessage:
        event = new QWSQCopMessageEvent;
        break;
#endif
    case QWSEvent::WindowOperation:
        event = new QWSWindowOperationEvent;
        break;

#ifndef QT_NO_QWS_INPUTMETHODS
    case QWSEvent::IMEvent:
        event = new QWSIMEvent;
        break;
    case QWSEvent::IMQuery:
        event = new QWSIMQueryEvent;
        break;
    case QWSEvent::IMInit:
        event = new QWSIMInitEvent;
        break;
#endif
    default:
        qDebug("QWSDisplayData::readMore() : Protocol error - got %08x!", type);
    }
    return event;
}

/*!
    \class QWSEvent
    \ingroup qws

    \brief The QWSEvent class encapsulates an event in Qtopia Core.

    When running a Qtopia Core application, it either runs as a server
    or connects to an existing server. In the client/server protocol,
    each event is sent as a QWSEvent object to the server.

    QWSEvent provides the Type enum specifying the origin of
    the event. Internally, each type is represented by a QWSEvent
    subclass, e.g. QWSKeyEvent.

    \sa QWSServer, QWSClient
*/

/*!
    \enum QWSEvent::Type

    This enum specifies the event's type.

    \value NoEvent No event has occurred.
    \value Connected An application has connected to the server.
    \value Mouse A mouse button is pressed or released, or the mouse cursor is moved.
             See also \l {Pointer Handling}.
    \value Focus A window has lost or received focus.
    \value Key A key is pressed or released. See also \l {Character Input}.
    \value Region A region has changed.
    \value Creation The server has created an ID, typically for a window.
    \value PropertyNotify A property has changed.
    \value PropertyReply The server is responding to a request for a property's value.
    \value SelectionClear A selection is deleted.
    \value SelectionRequest The server has queried for a selection.
    \value SelectionNotify A new selection has been created.
    \value MaxWindowRect The server has changed the maximum window for an application.
    \value QCopMessage A new Qt Cop message has appeared.
    \value WindowOperation A window operation, e.g. resizing, has occured.
    \value IMEvent An input method has been used  to enter text for languages with
              non-Latin alphabets. See also QWSInputMethod.
    \value IMQuery An input method query for a specified property has occurred.
             See also QWSInputMethod.
    \value IMInit
    \value NEvent The number of events has changed.
*/

/*!
   \fn  QWSMouseEvent *QWSEvent::asMouse()
   \internal
*/

/*!
   \fn  int QWSEvent::window()
   \internal
*/

/*!
   \fn  QWSEvent *QWSEvent::factory(int type)
   \internal
*/

/*!
   \fn  QWSEvent::QWSEvent( int t, int len, char * ptr)
   \internal
*/


