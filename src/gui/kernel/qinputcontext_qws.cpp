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

#include "qinputcontext_p.h"
#include "qstring.h"
#include "qwindowsystem_qws.h"
#include "qpaintdevice.h"
#include "qwsdisplay_qws.h"
#include "qapplication.h"
#include "qevent.h"

#ifndef QT_NO_QWS_IM
QWidget* QInputContext::activeWidget = 0;
QString* QInputContext::composition = 0;
bool QInputContext::composeMode = false;

static bool sendEndToPrev = false;
static QWidget* prevFocusW = 0;


bool qt_sendSpontaneousEvent(QObject *obj, QEvent *event); //in qapplication_qws.cpp

#include <qlineedit.h>

void QInputContext::retrieveMarkedText(QWidget *)
{
    QString s;
    //Only lineedit and multilineedit are IM-enabled anyway, so
    //we might as well do it all here instead of sending events
/*
#ifndef QT_NO_LINEEDIT
    if (w->inherits("QLineEdit")) {
        s = ((QLineEdit*)w)->markedText();
    }
# ifndef QT_NO_MULTILINEEDIT
    else
# endif
#endif
#ifndef QT_NO_MULTILINEEDIT
    if (w->inherits("QMultiLineEdit")) {
        s = ((QMultiLineEdit*)w)->markedText();
    }
#endif
*/
    int len =  s.length()*sizeof(QChar);
    QByteArray ba((const char*)s.unicode(), len);
    QPaintDevice::qwsDisplay()->
        setProperty(0, QT_QWS_PROPERTY_MARKEDTEXT,
                     QWSPropertyManager::PropReplace, ba);
}

void QInputContext::translateIMEvent(QWSIMEvent *e, QWidget *keywidget)
{
#if 0
    if (e->simpleData.type == QWSServer::IMMarkedText) {
        retrieveMarkedText(keywidget);
        return;
    }

    //generate end event for previous widget if focus has changed
    //### should not happen
    if (composeMode && activeWidget != keywidget && !sendEndToPrev) {
        cleanup();
    }

    QString txt(e->text, e->simpleData.textLen);


    if (e->simpleData.type == QWSServer::InputMethodCompose) {
        //generate start event if we haven't done so already
        if (!composeMode) {
            QInputMethodEvent out(QEvent::InputMethodStart, "", -1);
            qt_sendSpontaneousEvent(keywidget, &out);
            activeWidget = keywidget;
            composeMode = true;
            if (!composition)
                composition = new QString;
        }

        const int cpos = qMax(0, qMin(e->simpleData.cpos, int(txt.length())));
        const int selLen = qMin(e->simpleData.selLen, int(txt.length())-cpos);

        QInputMethodEvent out(QEvent::InputMethodCompose, txt,
                             cpos,
                             selLen);
        qt_sendSpontaneousEvent(keywidget, &out);

        *composition = txt;
        //qDebug("InputMethodCompose on widget %p", keywidget);
    } else if (e->simpleData.type == QWSServer::InputMethodEnd) {
        //InputMethodEnd also known as IMInput
        //Allow multiple InputMethodEnd events:
        //generate start event if we haven't seen one
        //but only if we actually need to send something.


        //qDebug("IM_END sendEndToPrev %d, preFocusW %p, focusW %p", sendEndToPrev, prevFocusW, activeWidget);


        if (composeMode) {
            QWidget  *target = (sendEndToPrev && prevFocusW) ? prevFocusW : activeWidget;
            QInputMethodEvent out(QEvent::InputMethodEnd, txt, e->simpleData.cpos);
            qt_sendSpontaneousEvent(target, &out);
        } else if (!txt.isEmpty()) {
            if (sendEndToPrev && prevFocusW)
                keywidget = prevFocusW;
            QInputMethodEvent start(QEvent::InputMethodStart, "", -1);
            qt_sendSpontaneousEvent(keywidget, &start);
            QInputMethodEvent end(QEvent::InputMethodEnd, txt, e->simpleData.cpos);
            qt_sendSpontaneousEvent(keywidget, &end);
        }
        composeMode = false;
        if (composition)
            *composition = QString::null;

    }
    sendEndToPrev = false;
#endif
}

void QInputContext::reset(QWidget *f)
{
    if (f) {
        prevFocusW = f;
        sendEndToPrev = true;
    } else {
        prevFocusW = 0;
        sendEndToPrev = false;
        composeMode = false;
    }

    activeWidget = 0;

    //server is obliged to send an InputMethodEnd event in response to this call
    QPaintDevice::qwsDisplay()->resetIM();
}

void QInputContext::setMicroFocusWidget(QWidget *w)
{
    if (activeWidget && w != activeWidget)
        reset();

    activeWidget = w;
}


void QInputContext::notifyWidgetDeletion(QWidget *w)
{
    if (w == activeWidget) {
        reset();
    } else if (sendEndToPrev && w == prevFocusW) {
        sendEndToPrev =  false;
        prevFocusW = 0;
    }
}



//Cleaning up if the IM hasn't done so
void QInputContext::cleanup()
{
#if 0
    qDebug("============= QInputContext::cleanup =========");
    //send appropriate InputMethodEnd event if necessary
    if (composeMode) {
        QInputMethodEvent out(QEvent::InputMethodEnd, *composition, -1);
        qt_sendSpontaneousEvent(activeWidget, &out);
        activeWidget = 0;
        composeMode = false;
        *composition = QString::null;
    }
#endif
    reset();
}

#endif //QT_NO_QWS_IM
