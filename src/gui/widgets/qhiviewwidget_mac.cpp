/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qhiviewwidget_mac_p.h>

static EventHandlerUPP mac_win_eventUPP = 0;


EventHandlerRef window_event;

static EventTypeSpec window_events[] = {
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp }
};

static void cleanup_win_eventUPP()
{
    DisposeEventHandlerUPP(mac_win_eventUPP);
    mac_win_eventUPP = 0;
}

const EventHandlerUPP QHIViewWidget::make_win_eventUPP()
{
    if(mac_win_eventUPP)
        return mac_win_eventUPP;

    qAddPostRoutine(cleanup_win_eventUPP);
    return mac_win_eventUPP = NewEventHandlerUPP(QHIViewWidget::qt_window_event);
}

QHIViewWidget::QHIViewWidget(WindowRef windowref, Qt::WFlags flags)
    : QWidget(0, flags)
{
    HIViewRef hiview;
    OSStatus err = HIViewFindByID(HIViewGetRoot(windowref), kHIViewWindowContentID, &hiview);
    if(err == errUnknownControl)
        hiview = HIViewGetRoot(windowref);
    create(WId(hiview), false, true);
    Rect rect;
    GetWindowBounds(windowref, kWindowContentRgn, &rect);
    setGeometry(QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top));
    QWidget *that = this;
    static const UInt32 kWidgetCreatorQt = 'cute';
    enum {
        kWidgetPropertyQWidget = 'QWId' //QWidget *
    };
    err = SetWindowProperty(windowref, kWidgetCreatorQt, kWidgetPropertyQWidget,
                            sizeof(this), &that);
    InstallWindowEventHandler(windowref, make_win_eventUPP(), GetEventTypeCount(window_events),
                              window_events, that, &window_event);
    if (err != noErr)
        qWarning("Qt:Internal error (%s:%d) %ld", __FILE__, __LINE__, err);
}

QHIViewWidget::~QHIViewWidget()
{
    qDebug("in here!!!");
}

QHIViewWidget::QHIViewWidget(HIViewRef hiviewref, QWidget *parent)
    : QWidget(parent)
{
    create(WId(hiviewref), false, true);
    setVisible(HIViewIsVisible(hiviewref));
}

OSStatus QHIViewWidget::qt_window_event(EventHandlerCallRef, EventRef event, void *)
{

    bool handled_event = true;
    UInt32 eclass = GetEventClass(event);
    switch(eclass) {
        case kEventClassMouse: {
            bool send_to_app = false;
            {
                WindowPartCode wpc;
                if (GetEventParameter(event, kEventParamWindowPartCode, typeWindowPartCode, 0,
                                      sizeof(wpc), 0, &wpc) == noErr && wpc != inContent)
                    send_to_app = true;
            }
            if(!send_to_app) {
                WindowRef window;
                if(GetEventParameter(event, kEventParamWindowRef, typeWindowRef, 0,
                                     sizeof(window), 0, &window) == noErr) {
                    HIViewRef hiview;
                    if(HIViewGetViewForMouseEvent(HIViewGetRoot(window), event, &hiview) == noErr) {
                        if(QWidget *w = QWidget::find((WId)hiview)) {
                            Q_UNUSED(w);
                            send_to_app = true;
                        }
                    }
                }
            }
            if(send_to_app) {
                return SendEventToEventTarget(event, GetApplicationEventTarget());
            }
            handled_event = false;
            break; }
        default:
            handled_event = false;
    }
    if(!handled_event) //let the event go through
        return eventNotHandledErr;
    return noErr; //we eat the event
}


void QHIViewWidget::createQWidgetsFromHIViews()
{
    // Nicely walk through and make HIViewWidget out of all the children
    addViews_recursive(HIViewGetFirstSubview(HIViewRef(winId())), this);
}

void QHIViewWidget::addViews_recursive(HIViewRef child, QWidget *parent)
{
    if (!child)
        return;

    QWidget *widget;
    if (HIObjectIsOfClass(HIObjectRef(child), CFSTR("com.trolltech.qt.widget")))
        widget = QWidget::find(WId(child));
    else
        widget = new QHIViewWidget(child, parent);

    addViews_recursive(HIViewGetFirstSubview(child), widget);
    addViews_recursive(HIViewGetNextView(child), parent);
}
