/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwsembedwidget.h"

#ifndef QT_NO_QWSEMBEDWIDGET

#include <qwsdisplay_qws.h>
#include <private/qwidget_p.h>
#include <private/qwsdisplay_qws_p.h>
#include <private/qwscommand_qws_p.h>

// TODO:
// Must remove window decorations from the embedded window
// Focus In/Out, Keyboard/Mouse...
//
// BUG: what if my parent change parent?

class QWSEmbedWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QWSEmbedWidget);

public:
    QWSEmbedWidgetPrivate(int winId);
    void updateWindow();
    void resize(const QSize &size);

    QWidget *window;
    WId windowId;
    WId embeddedId;
};

QWSEmbedWidgetPrivate::QWSEmbedWidgetPrivate(int winId)
    : window(0), windowId(0), embeddedId(winId)
{
}

void QWSEmbedWidgetPrivate::updateWindow()
{
    Q_Q(QWSEmbedWidget);

    QWidget *win = q->window();
    if (win == window)
        return;

    if (window) {
        window->removeEventFilter(q);
        QWSEmbedCommand command;
        command.setData(windowId, embeddedId, QWSEmbedEvent::StopEmbed);
        QWSDisplay::instance()->d->sendCommand(command);
    }

    window = win;
    if (!window)
        return;
    windowId = window->winId();

    QWSEmbedCommand command;
    command.setData(windowId, embeddedId, QWSEmbedEvent::StartEmbed);
    QWSDisplay::instance()->d->sendCommand(command);
    window->installEventFilter(q);
}

void QWSEmbedWidgetPrivate::resize(const QSize &size)
{
    if (!window)
        return;

    Q_Q(QWSEmbedWidget);

    QWSEmbedCommand command;
    command.setData(windowId, embeddedId, QWSEmbedEvent::Region,
                    QRect(q->mapToGlobal(QPoint(0, 0)), size));
    QWSDisplay::instance()->d->sendCommand(command);
}

/*!
    \class QWSEmbedWidget
    \since 4.2

    \brief The QWSEmbedWidget class provides embedding of a top-level window.

    The window ID of the window to be embedded must be known and be passed
    in the constructor.
*/

/*!
    Constructs a QWSEmbedWidget object which embeds the window with the given
    \a id and \a parent.
*/
QWSEmbedWidget::QWSEmbedWidget(WId id, QWidget *parent)
    : QWidget(*new QWSEmbedWidgetPrivate(id), parent, 0)
{
    Q_D(QWSEmbedWidget);
    d->updateWindow();
}

/*!
    Destroys the QWSEmbedWidget object.
*/
QWSEmbedWidget::~QWSEmbedWidget()
{
    Q_D(QWSEmbedWidget);
    if (!d->window)
        return;

    QWSEmbedCommand command;
    command.setData(d->windowId, d->embeddedId, QWSEmbedEvent::StopEmbed);
    QWSDisplay::instance()->d->sendCommand(command);
}

/*!
    \reimp
*/
bool QWSEmbedWidget::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Move)
        resizeEvent(0);
    return QWidget::eventFilter(object, event);
}

/*!
    \reimp
*/
void QWSEmbedWidget::changeEvent(QEvent *event)
{
    Q_D(QWSEmbedWidget);
    if (event->type() == QEvent::ParentChange)
        d->updateWindow();
}

/*!
    \reimp
*/
void QWSEmbedWidget::resizeEvent(QResizeEvent*)
{
    Q_D(QWSEmbedWidget);
    d->resize(rect().size());
}

/*!
    \reimp
*/
void QWSEmbedWidget::moveEvent(QMoveEvent*)
{
    resizeEvent(0);
}

/*!
    \reimp
*/
void QWSEmbedWidget::hideEvent(QHideEvent*)
{
    Q_D(QWSEmbedWidget);
    d->resize(QSize());
}

/*!
    \reimp
*/
void QWSEmbedWidget::showEvent(QShowEvent*)
{
    Q_D(QWSEmbedWidget);
    d->resize(rect().size());
}


#endif // QT_NO_QWSEMBEDWIDGET
