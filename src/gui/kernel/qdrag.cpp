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

#include <qwidget.h>
#include <qdrag.h>
#include <qpixmap.h>
#include <qpoint.h>
#include "qdnd_p.h"


QDrag::QDrag(QWidget *dragSource)
    : QObject(*new QDragPrivate, dragSource)
{
    Q_D(QDrag);
    d->source = dragSource;
    d->target = 0;
    d->data = 0;
    d->hotspot = QPoint(-10, -10);
    d->request_action = QDrag::CopyAction;
    d->executed_action = QDrag::IgnoreAction;
}

QDrag::~QDrag()
{
    QDragManager *manager = QDragManager::self();
    if (manager && manager->object == this)
        manager->cancel(false);
}

void QDrag::setMimeData(QMimeData *data)
{
    Q_D(QDrag);
    d->data = data;
}

QMimeData *QDrag::mimeData() const
{
    Q_D(const QDrag);
    return d->data;
}

void QDrag::setPixmap(const QPixmap &pixmap)
{
    Q_D(QDrag);
    d->pixmap = pixmap;
}

QPixmap QDrag::pixmap() const
{
    Q_D(const QDrag);
    return d->pixmap;
}

void QDrag::setHotSpot(const QPoint& hotspot)
{
    Q_D(QDrag);
    d->hotspot = hotspot;
}

QPoint QDrag::hotSpot() const
{
    Q_D(const QDrag);
    return d->hotspot;
};

QWidget *QDrag::source() const
{
    Q_D(const QDrag);
    return d->source;
}

QWidget *QDrag::target() const
{
    return 0; // ########### d->target;
}

QDrag::DropAction QDrag::start(QDrag::DropActions request)
{
    Q_D(QDrag);
    QDragManager *manager = QDragManager::self();
    d->request_action = request;
    if (manager)
        d->executed_action = manager->drag(this);
    return d->executed_action;
}
