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

#include "q3whatsthis.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qevent.h"

Q3WhatsThis::Q3WhatsThis(QWidget *w)
    : QObject(w)
{
    if (w)
        w->installEventFilter(this);
}

Q3WhatsThis::~Q3WhatsThis()
{
}

bool Q3WhatsThis::eventFilter(QObject *o, QEvent *e)
{
    if (o != parent() || !o->isWidgetType())
        return false;

    if (e->type() == QEvent::WhatsThis) {
        QString s = text(static_cast<QHelpEvent*>(e)->pos());
        if (!s.isEmpty())
            QWhatsThis::showText(static_cast<QHelpEvent*>(e)->globalPos(), s, static_cast<QWidget*>(o));
    } else if (e->type() == QEvent::WhatsThisClicked) {
        QString href = static_cast<QWhatsThisClickedEvent*>(e)->href();
        if (clicked(href))
            QWhatsThis::hideText();
    } else {
        return false;
    }
    return true;
}

QString Q3WhatsThis::text(const QPoint &)
{
    if (parent() && parent()->isWidgetType())
        return static_cast<QWidget*>(parent())->whatsThis();
    return QString();
}

bool Q3WhatsThis::clicked(const QString &)
{ return true;}


