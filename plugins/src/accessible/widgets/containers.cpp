#include "containers.h"

#include <qwidgetstack.h>

/*!
  \class QAccessibleWidgetStack qaccessible.h
  \brief The QAccessibleWidgetStack class implements the QAccessibleInterface for widget stacks.
*/

/*!
  Creates a QAccessibleWidgetStack object for \a w.
*/
QAccessibleWidgetStack::QAccessibleWidgetStack(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(widgetStack());
}

/*! Returns the widget stack. */
QWidgetStack *QAccessibleWidgetStack::widgetStack() const
{
    return qt_cast<QWidgetStack*>(object());
}

/*! \reimp */
int QAccessibleWidgetStack::childAt(int, int) const
{
    return widgetStack()->id(widgetStack()->visibleWidget()) + 1;
}

