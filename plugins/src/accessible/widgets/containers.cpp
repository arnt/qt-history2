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
: QAccessibleWidget(w, LayeredPane)
{
    Q_ASSERT(widgetStack());
    setDescription("This is a widgetstack");
}

/*! Returns the widget stack. */
QWidgetStack *QAccessibleWidgetStack::widgetStack() const
{
    return qt_cast<QWidgetStack*>(object());
}

/*! \reimp */
int QAccessibleWidgetStack::childCount() const
{
    // a widget stack has always only one accessible widget
    return 1;
}

/*! \reimp */
int QAccessibleWidgetStack::indexOfChild(const QAccessibleInterface *child) const
{
    QObject *childObject = child ? child->object() : 0;
    if (childObject != widgetStack()->visibleWidget())
	return -1;
    return 1;
}

/*! \reimp */
int QAccessibleWidgetStack::childAt(int, int) const
{
    QWidget *curPage = widgetStack()->visibleWidget();
    if (!curPage)
	return 0;
    return 1;
}

/*! \reimp */
int QAccessibleWidgetStack::navigate(Relation rel, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    QObject *targetObject = 0;
    switch (rel) {
    // Hierarchical
    case Self:
	const_cast<QAccessibleWidgetStack*>(this)->queryInterface(IID_QAccessible, (QUnknownInterface**)target);
	return 0;
    case Child:
	if (entry != 1)
	    return -1;
	targetObject = widgetStack()->visibleWidget();
	break;
    default:
	return QAccessibleWidget::navigate(rel, entry, target);
    }
    QAccessible::queryAccessibleInterface(targetObject, target);
    return *target ? 0 : -1;
}
