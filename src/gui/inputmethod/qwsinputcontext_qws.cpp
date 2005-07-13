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

#include "qwsinputcontext_p.h"
#include "qwsdisplay_qws.h"
#include "qwsevent_qws.h"
#include "qwscommand_qws.h"
#include "qwindowsystem_qws.h"
#include "qevent.h"
#include "qtextformat.h"

#include <qbuffer.h>

#include <qdebug.h>

#ifndef QT_NO_QWS_IM

static QWidget* activeWidget = 0;

//#define EXTRA_DEBUG

QWSInputContext::QWSInputContext(QObject *parent)
    :QInputContext(parent)
{
}

void QWSInputContext::reset()
{
    QPaintDevice::qwsDisplay()->resetIM();
}


void QWSInputContext::setFocusWidget( QWidget *w )
{
  QWidget *oldFocus = focusWidget();
    if (oldFocus == w)
        return;

    if (oldFocus) {
        QWidget *tlw = oldFocus->window();
        int winid = tlw->winId();

        int widgetid = oldFocus->winId();
        QPaintDevice::qwsDisplay()->sendIMUpdate(QWSIMUpdateCommand::FocusOut, winid, widgetid);
    }

    QInputContext::setFocusWidget(w);

    if (!w)
        return;

    QWidget *tlw = w->window();
    int winid = tlw->winId();

    int widgetid = w->winId();
    QPaintDevice::qwsDisplay()->sendIMUpdate(QWSIMUpdateCommand::FocusIn, winid, widgetid);

    //setfocus ???

    update();
}


void QWSInputContext::widgetDestroyed(QWidget *w)
{
    if (w == ::activeWidget)
        ::activeWidget = 0;
    QInputContext::widgetDestroyed(w);
}

void QWSInputContext::update()
{
    QWidget *w = focusWidget();
    if (!w)
        return;

    QWidget *tlw = w->window();
    int winid = tlw->winId();

    int widgetid = w->winId();
    QPaintDevice::qwsDisplay()->sendIMUpdate(QWSIMUpdateCommand::Update, winid, widgetid);

}

void QWSInputContext::mouseHandler( int x, QMouseEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)
        QPaintDevice::qwsDisplay()->sendIMMouseEvent( x, event->type() == QEvent::MouseButtonPress );
}

QWidget *QWSInputContext::activeWidget()
{
    return ::activeWidget;
}


bool QWSInputContext::isComposing() const
{
    return activeWidget != 0;
}

bool QWSInputContext::translateIMQueryEvent(QWidget *w, const QWSIMQueryEvent *e)
{
    QVariant result = w->inputMethodQuery(static_cast<Qt::InputMethodQuery>(e->simpleData.property));
    QWidget *tlw = w->window();
    int winId = tlw->winId();

    QPaintDevice::qwsDisplay()->sendIMResponse(winId, e->simpleData.property, result);


    return false;
}

bool QWSInputContext::translateIMInitEvent(const QWSIMInitEvent *e)
{
    qDebug("### QWSInputContext::translateIMInitEvent not implemented ###");
    return false;
}

bool QWSInputContext::translateIMEvent(QWidget *w, const QWSIMEvent *e)
{
    QInputContext *qic = w->inputContext();
    if (!qic)
        return false;

    QString preedit;
    QString commit;
    QList<QInputMethodEvent::Attribute> attrs;

    QDataStream stream(e->streamingData);

    stream >> preedit;
    stream >> commit;

    while (!stream.atEnd()) {
        int type = -1;
        int start = -1;
        int length = -1;
        QVariant data;
        stream >> type >> start >> length >> data;
        if (stream.status() != QDataStream::Ok) {
            qWarning("corrupted QWSIMEvent");
            //qic->reset(); //???
            return false;
        }
        if (type == QInputMethodEvent::TextFormat)
            data = qic->standardFormat(static_cast<QInputContext::StandardFormat>(data.toInt()));
        attrs << QInputMethodEvent::Attribute(static_cast<QInputMethodEvent::AttributeType>(type), start, length, data);
    }
#ifdef EXTRA_DEBUG
    qDebug() << "preedit" << preedit << "len" << preedit.length() <<"commit" << commit << "len" << commit.length()
             << "n attr" << attrs.count();
#endif


    QInputMethodEvent ime(preedit, attrs);
    if (!commit.isEmpty() || e->simpleData.replaceLength > 0)
        ime.setCommitString(commit, e->simpleData.replaceFrom, e->simpleData.replaceLength);

    if (preedit.isEmpty() && ::activeWidget)
        w = ::activeWidget;

    qt_sendSpontaneousEvent(w, &ime);

    if (preedit.isEmpty())
        ::activeWidget = 0;
    else
        ::activeWidget = w;

    return true;
}





#endif // QT_NO_QWS_IM
