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

/*! \class Q3WhatsThis
    \compat
*/

/*!
    Constructs a new What's This object for widget \a w.
*/
Q3WhatsThis::Q3WhatsThis(QWidget *w)
    : QObject(w)
{
    if (w)
        w->installEventFilter(this);
}

/*!
    Destructs the What's This object.
*/
Q3WhatsThis::~Q3WhatsThis()
{
}

/*!
    Handles What's This events.
*/
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

/*!
    Returns the parent's What's This text or an empty string.
*/
QString Q3WhatsThis::text(const QPoint &)
{
    if (parent() && parent()->isWidgetType())
        return static_cast<QWidget*>(parent())->whatsThis();
    return QString();
}

/*!
    Returns true.
*/
bool Q3WhatsThis::clicked(const QString &)
{ return true;}

/*!
    \fn void Q3WhatsThis::enterWhatsThisMode()

*/

/*!
    \fn bool Q3WhatsThis::inWhatsThisMode()

*/

/*!
    \fn void Q3WhatsThis::leaveWhatsThisMode()

*/

/*!
    \fn void Q3WhatsThis::add(QWidget *w, const QString &s)

*/

/*!
    \fn void Q3WhatsThis::remove(QWidget *w)

*/

/*!
    \fn void Q3WhatsThis::leaveWhatsThisMode(const QString& text = QString::null, const QPoint& pos = QCursor::pos(), QWidget* w = 0)

*/

/*!
    \fn void Q3WhatsThis::display(const QString& text, const QPoint& pos = QCursor::pos(), QWidget* w = 0)

*/

/*!
    \fn QToolButton* Q3WhatsThis::whatsThisButton(QWidget * parent)

*/
