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

#include "qaccessiblewidgets.h"

#include <qstyle.h>
#include <qgroupbox.h>

/*!
  \class QAccessibleViewport qaccessiblewidget.h
  \brief The QAccessibleViewport class hides the viewport of scrollviews for accessibility.
  \internal

  \ingroup accessibility
*/

QAccessibleViewport::QAccessibleViewport(QWidget *o, QWidget *sv)
    : QAccessibleWidget(o)
{
    Q_ASSERT(sv->inherits("QScrollView"));
    scrollview = static_cast<QAccessibleScrollView *>(queryAccessibleInterface(sv));
}

QAccessibleViewport::~QAccessibleViewport()
{
    delete scrollview;
}

int QAccessibleViewport::childAt(int x, int y) const
{
    int child = QAccessibleWidget::childAt(x, y);
    if (child > 0)
        return child;

    QPoint p = widget()->mapFromGlobal(QPoint(x,y));
    return scrollview->itemAt(p.x(), p.y());
}

QRect QAccessibleViewport::rect(int child) const
{
    if (!child)
        return QAccessibleWidget::rect(child);
    QRect rect = scrollview->itemRect(child);
    QPoint tl = widget()->mapToGlobal(QPoint(0,0));
    return QRect(tl.x() + rect.x(), tl.y() + rect.y(), rect.width(), rect.height());
}

/*
int QAccessibleViewport::navigate(NavDirection direction, int startControl) const
{
    if (direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl)
        return QAccessibleWidget::navigate(direction, startControl);

    // ### call itemUp/Down etc. here
    const int items = scrollview->itemCount();
    switch(direction) {
    case NavFirstChild:
        return 1;
    case NavLastChild:
        return items;
    case NavNext:
    case NavDown:
        return startControl + 1 > items ? -1 : startControl + 1;
    case NavPrevious:
    case NavUp:
        return startControl - 1 < 1 ? -1 : startControl - 1;
    default:
        break;
    }

    return -1;
}
*/

int QAccessibleViewport::childCount() const
{
    int widgets = QAccessibleWidget::childCount();
    return widgets ? widgets : scrollview->itemCount();
}

QString QAccessibleViewport::text(Text t, int child) const
{
    return scrollview->text(t, child);
}

bool QAccessibleViewport::doAction(int action, int child, const QVariantList &params)
{
    return scrollview->doAction(action, child, params);
}

QAccessible::Role QAccessibleViewport::role(int child) const
{
    return scrollview->role(child);
}

QAccessible::State QAccessibleViewport::state(int child) const
{
    return scrollview->state(child);
}

bool QAccessibleViewport::setSelected(int /*child*/, bool /*on*/, bool /*extend*/)
{
//###    return scrollview->setSelected(child, on, extend);
    return 0;
}

void QAccessibleViewport::clearSelection()
{
//###    scrollview->clearSelection();
}

QVector<int> QAccessibleViewport::selection() const
{
//###    return scrollview->selection();
    return QVector<int>();
}

/*!
  \class QAccessibleScrollView qaccessiblewidget.h
  \brief The QAccessibleScrollView class implements the QAccessibleInterface for scrolled widgets.
  \internal

  \ingroup accessibility
*/

/*!
  Constructs a QAccessibleScrollView object for a widtet, \a w.
  The \a role is propagated to the QAccessibleWidget constructor.
*/
QAccessibleScrollView::QAccessibleScrollView(QWidget *w, Role role)
: QAccessibleWidget(w, role)
{
}

/*!
  Returns the ID of the item at viewport position \a x, \a y.
*/
int QAccessibleScrollView::itemAt(int /*x*/, int /*y*/) const
{
    return 0;
}

/*!
  Returns the location in viewport coordinates of the item with ID \a
  item.
*/
QRect QAccessibleScrollView::itemRect(int /*item*/) const
{
    return QRect();
}

/*!
  Returns the number of items.
*/
int QAccessibleScrollView::itemCount() const
{
    return 0;
}
