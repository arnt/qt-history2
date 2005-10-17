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

#ifndef QWSEVENT_QWS_H
#define QWSEVENT_QWS_H

#include <QtGui/qwsutils_qws.h>
#include <QtGui/qwscommand_qws.h> //QWSProtocolItem lives there, for now
#include <QtCore/qrect.h>

QT_MODULE(Gui)

struct QWSMouseEvent;

struct QWSEvent : QWSProtocolItem {

    QWSEvent(int t, int len, char *ptr) : QWSProtocolItem(t,len,ptr) {}



    enum Type {
        NoEvent,
        Connected,
        Mouse,
        Focus,
        Key,
        Region,
        Creation,
        PropertyNotify,
        PropertyReply,
        SelectionClear,
        SelectionRequest,
        SelectionNotify,
        MaxWindowRect,
        QCopMessage,
        WindowOperation,
        IMEvent,
        IMQuery,
        IMInit,
        NEvent
    };

    QWSMouseEvent *asMouse()
        { return type == Mouse ? reinterpret_cast<QWSMouseEvent*>(this) : 0; }
    int window() { return *(reinterpret_cast<int*>(simpleDataPtr)); }
    static QWSEvent *factory(int type);
};


//All events must start with windowID

struct QWSConnectedEvent : QWSEvent {
    QWSConnectedEvent()
        : QWSEvent(QWSEvent::Connected, sizeof(simpleData),
                reinterpret_cast<char*>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSEvent::setData(d, len, allocateMem);
        display = reinterpret_cast<char*>(rawDataPtr);
    }

    struct SimpleData {
        int window;
        int len;
        int clientId;
    } simpleData;

    char *display;
};

struct QWSMaxWindowRectEvent : QWSEvent {
    QWSMaxWindowRectEvent()
        : QWSEvent(MaxWindowRect, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) { }
    struct SimpleData {
        int window;
        QRect rect;
    } simpleData;
};

struct QWSMouseEvent : QWSEvent {
    QWSMouseEvent()
        : QWSEvent(QWSEvent::Mouse, sizeof(simpleData),
                reinterpret_cast<char*>(&simpleData)) {}
    struct SimpleData {
        int window;
        int x_root, y_root, state, delta;
        int time; // milliseconds
    } simpleData;
};

struct QWSFocusEvent : QWSEvent {
    QWSFocusEvent()
        : QWSEvent(QWSEvent::Focus, sizeof(simpleData), reinterpret_cast<char*>(&simpleData))
        { memset(reinterpret_cast<char*>(&simpleData),0,sizeof(simpleData)); }
    struct SimpleData {
        int window;
        uint get_focus:1;
    } simpleData;
};

struct QWSKeyEvent: QWSEvent {
    QWSKeyEvent()
        : QWSEvent(QWSEvent::Key, sizeof(simpleData),
              reinterpret_cast<char*>(&simpleData)) {}
    struct SimpleData {
        int window;
        uint keycode;
        Qt::KeyboardModifiers modifiers;
        ushort unicode;
        uint is_press:1;
        uint is_auto_repeat:1;
    } simpleData;
};


struct QWSCreationEvent : QWSEvent {
    QWSCreationEvent()
        : QWSEvent(QWSEvent::Creation, sizeof(simpleData),
              reinterpret_cast<char*>(&simpleData)) {}
    struct SimpleData {
        int objectid;
    } simpleData;
};

#ifndef QT_NO_QWS_PROPERTIES
struct QWSPropertyNotifyEvent : QWSEvent {
    QWSPropertyNotifyEvent()
        : QWSEvent(QWSEvent::PropertyNotify, sizeof(simpleData),
              reinterpret_cast<char*>(&simpleData)) {}
    enum State {
        PropertyNewValue,
        PropertyDeleted
    };
    struct SimpleData {
        int window;
        int property;
        int state;
    } simpleData;
};
#endif

struct QWSSelectionClearEvent : QWSEvent {
    QWSSelectionClearEvent()
        : QWSEvent(QWSEvent::SelectionClear, sizeof(simpleData),
              reinterpret_cast<char*>(&simpleData)) {}
    struct SimpleData {
        int window;
    } simpleData;
};

struct QWSSelectionRequestEvent : QWSEvent {
    QWSSelectionRequestEvent()
        : QWSEvent(QWSEvent::SelectionRequest, sizeof(simpleData),
              reinterpret_cast<char*>(&simpleData)) {}
    struct SimpleData {
        int window;
        int requestor; // window which wants the selection
        int property; // property on requestor into which the selection should be stored, normally QWSProperty::PropSelection
        int mimeTypes; // Value is stored in the property mimeType on the requestor window. This value may contain
        // multiple mimeTypes separated by ;; where the order reflects the priority
    } simpleData;
};

struct QWSSelectionNotifyEvent : QWSEvent {
    QWSSelectionNotifyEvent()
        : QWSEvent(QWSEvent::SelectionNotify, sizeof(simpleData),
              reinterpret_cast<char*>(&simpleData)) {}
    struct SimpleData {
        int window;
        int requestor; // the window which wanted the selection and to which this event is sent
        int property; // property of requestor in which the data of the selection is stored
        int mimeType; // a property on the requestor in which the mime type in which the selection is, is stored
    } simpleData;
};

//complex events:

struct QWSRegionEvent : QWSEvent {
    QWSRegionEvent()
        : QWSEvent(QWSEvent::Region, sizeof(simpleData),
                reinterpret_cast<char*>(&simpleData))
        { memset(reinterpret_cast<char*>(&simpleData),0,sizeof(simpleData)); }

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSEvent::setData(d, len, allocateMem);
        rectangles = reinterpret_cast<QRect*>(rawDataPtr);
    }

    enum Type {Allocation, Request};
    struct SimpleData {
        int window;
        int nrectangles;
        uint type:8;
    } simpleData;

    QRect *rectangles;
};

#ifndef QT_NO_QWS_PROPERTIES
struct QWSPropertyReplyEvent : QWSEvent {
    QWSPropertyReplyEvent()
        : QWSEvent(QWSEvent::PropertyReply, sizeof(simpleData),
                reinterpret_cast<char*>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSEvent::setData(d, len, allocateMem);
        data = reinterpret_cast<char*>(rawDataPtr);
    }

    struct SimpleData {
        int window;
        int property;
        int len;
    } simpleData;
    char *data;
};
#endif //QT_NO_QWS_PROPERTIES

#ifndef QT_NO_COP
struct QWSQCopMessageEvent : QWSEvent {
    QWSQCopMessageEvent()
        : QWSEvent(QWSEvent::QCopMessage, sizeof(simpleData),
                reinterpret_cast<char*>(&simpleData))
        { memset(reinterpret_cast<char*>(&simpleData),0,sizeof(simpleData)); }

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSEvent::setData(d, len, allocateMem);
        char* p = rawDataPtr;
	channel = QByteArray(p, simpleData.lchannel);
        p += simpleData.lchannel;
        message = QByteArray(p, simpleData.lmessage);
        p += simpleData.lmessage;
        data = QByteArray::fromRawData(p, simpleData.ldata);
    }

    struct SimpleData {
        bool is_response;
        int lchannel;
        int lmessage;
        int ldata;
    } simpleData;

    QByteArray channel;
    QByteArray message;
    QByteArray data;
};

#endif

struct QWSWindowOperationEvent : QWSEvent {
    QWSWindowOperationEvent()
        : QWSEvent(WindowOperation, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) { }

    enum Operation { Show, Hide, ShowMaximized, ShowNormal, ShowMinimized, Close };
    struct SimpleData {
        int window;
        Operation op;
    } simpleData;
};

#ifndef QT_NO_QWS_INPUTMETHODS


struct QWSIMEvent : QWSEvent {
    QWSIMEvent()
        : QWSEvent(IMEvent, sizeof(simpleData), reinterpret_cast<char*>(&simpleData))
   { memset(reinterpret_cast<char*>(&simpleData),0,sizeof(simpleData)); }

    struct SimpleData {
        int window;
        int replaceFrom;
        int replaceLength;
    } simpleData;

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSEvent::setData(d, len, allocateMem);
        streamingData = QByteArray::fromRawData(rawDataPtr, len);
    }
    QByteArray streamingData;
};


struct QWSIMInitEvent : QWSEvent {
    QWSIMInitEvent()
        : QWSEvent(IMInit, sizeof(simpleData), reinterpret_cast<char*>(&simpleData))
   { memset(reinterpret_cast<char*>(&simpleData),0,sizeof(simpleData)); }

    struct SimpleData {
        int window;
        int existence;
    } simpleData;

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSEvent::setData(d, len, allocateMem);
        streamingData = QByteArray::fromRawData(rawDataPtr, len);
    }
    QByteArray streamingData;
};


struct QWSIMQueryEvent : QWSEvent {
    QWSIMQueryEvent()
        : QWSEvent(QWSEvent::IMQuery, sizeof(simpleData),
              reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int window;
        int property;
    } simpleData;

};

#endif

#endif // QWSEVENT_QWS_H
