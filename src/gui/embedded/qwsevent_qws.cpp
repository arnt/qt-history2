/****************************************************************************
**
** Implementation of Qt/Embedded events.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
#ifndef QT_NO_COP
    case QWSEvent::QCopMessage:
        event = new QWSQCopMessageEvent;
        break;
#endif
    case QWSEvent::WindowOperation:
        event = new QWSWindowOperationEvent;
        break;

#ifndef QT_NO_QWS_IM
    case QWSEvent::IMEvent:
        event = new QWSIMEvent;
        break;
#endif
    default:
        qDebug("QWSDisplayData::readMore() : Protocol error - got %08x!", type);
    }
    return event;
}

