/****************************************************************************
**
** Implementation of the QAccessibleWidget class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessiblewidget.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qapplication.h"
#include "qtooltip.h"
#include "qwhatsthis.h"
#include "qwidget.h"

class QAccessibleWidgetPrivate : public QAccessible
{
public:
    QAccessibleWidgetPrivate()
	:role(Client), state(Normal)
    {}

    Role role;
    State state;
    QString name;
    QString description;
    QString value;
    QString help;
    int	    defAction;
    QString defActionName;
    QString accelerator;
};

/*!
    \class QAccessibleWidget qaccessiblewidget.h
    \brief The QAccessibleWidget class implements the QAccessibleInterface for QWidgets.
    \ingroup misc

    This class is convenient to use as a base class for custom implementations of
    QAccessibleInterfaces that provide information about widget objects.
*/

/*!
    Creates a QAccessibleWidget object for \a w.
    \a role, \a name, \a description, \a value, \a help, \a defAction, \a defActionName,
    \a accelerator and \a state are optional parameters for static values
    of the object's property.
*/
QAccessibleWidget::QAccessibleWidget(QWidget *w, Role role, QString name,
    QString description, QString value, QString help, int defAction, QString defActionName, 
    QString accelerator, State state)
: QAccessibleObject(w)
{
    Q_ASSERT(widget());
    d = new QAccessibleWidgetPrivate();
    d->role = role;
    d->state = state;
    d->name = name;
    d->description = description;
    d->value = value;
    d->help = help;
    d->defAction = SetFocus;
    d->defActionName = defAction;
    d->accelerator = accelerator;
}

/*!
    Destroys this object.
*/
QAccessibleWidget::~QAccessibleWidget()
{
    delete d;
}

/*!
    Returns the widget.
*/
QWidget *QAccessibleWidget::widget() const
{
    return qt_cast<QWidget*>(object());
}

/*! \reimp */
int QAccessibleWidget::childAt(int x, int y) const
{
    QWidget *w = widget();
    QPoint gp = w->mapToGlobal(QPoint(0, 0));
    if (!QRect(gp.x(), gp.y(), w->width(), w->height()).contains(x, y))
	return -1;

    QPoint rp = w->mapFromGlobal(QPoint(x, y));

    QObjectList list = w->queryList( "QWidget", 0, FALSE, FALSE );
    int ccount = childCount();

    // a complex control
    if ( list.count() < ccount ) {
	for (int i = 1; i <= ccount; ++i) {
	    if (rect(i).contains(x, y))
		return i;
	}
	return 0;
    }

    QList<QObject*>::Iterator it = list.begin();
    QWidget *child = 0;
    int index = 1;
    while (it != list.end()) {
	child = (QWidget*)*it;
	if (!child->isTopLevel() && !child->isHidden() && child->geometry().contains(rp)) {
	    return index;
	}
	++it;
	++index;
    }
    if (index > list.count())
	return 0;
    return index;
}

/*! \reimp */
QRect	QAccessibleWidget::rect( int control ) const
{
    if (control)
	qWarning( "QAccessibleWidget::rect: This implementation does not support subelements! (ID %d unknown for %s)", control, widget()->className() );

    QWidget *w = widget();
    QPoint wpos = w->mapToGlobal(QPoint(0, 0));

    return QRect(wpos.x(), wpos.y(), w->width(), w->height());
}

/*! \reimp */
QAccessible::Relation QAccessibleWidget::relationTo(int child, const QAccessibleInterface *other, int otherChild) const
{
    QObject *o = other ? other->object() : 0;
    if (!o || !o->isWidgetType())
	return Unrelated;

    if(o == object()) {
	if (child && !otherChild)
	    return Child;
	if (!child && otherChild)
	    return Ancestor;
	if (!child && !otherChild)
	    return Self;
    }

    QObject *parent = object()->parent();
    if (o == parent)
	return Child;

    if (o->parent() == parent) {
	int relation = Sibling;
	QWidget *sibling = static_cast<QWidget*>(o);
	if (widget()->x() < sibling->x())
	    relation |= QAccessible::Left;
	else
	    relation |= QAccessible::Right;
	if (widget()->y() < sibling->y())
	    relation |= QAccessible::Above;
	else
	    relation |= QAccessible::Below;

	return (Relation)relation;
    }

    while(parent) {
	if (parent == o)
	    return Descendent;
	parent = parent->parent();
    }

    QObjectList cl(object()->queryList("QWidget", 0, 0, FALSE));
    if (cl.contains(o))
	return Ancestor;

    for (int i = 0; i < cl.count(); ++i) {
	QObject *co = cl.at(i);
	QObjectList scl(co->queryList("QWidget", 0, 0, FALSE));
	if (scl.contains(o))
	    return Ancestor;
    }

    return Unrelated;
}

/*! \reimp */
int QAccessibleWidget::navigate(Relation relation, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    QObject *targetObject = 0;

    QObjectList childList = widget()->queryList( "QWidget", 0, FALSE, FALSE );
    bool complexWidget = childList.count() < childCount();

    switch (relation) {
    case Self:
	const_cast<QAccessibleWidget*>(this)->queryInterface(IID_QAccessible, (QUnknownInterface**)target);
	return 0;
    case Child:
	if (complexWidget) {
	    return entry;
	}else {
	    if (childList.count() >= entry)
		targetObject = childList.at(entry - 1);
	}
	break;
    case Ancestor:
	{
	    targetObject = widget()->parentWidget();
	    int i;
	    for (i = entry; i > 1; --i)
		targetObject = targetObject->parent();
	    if (!targetObject && i == 1)
		targetObject = qApp;
	}
	break;
    case Sibling:
	{
	    QWidget *parentWidget = widget()->parentWidget();
	    if (!parentWidget)
		return -1;
	    QObjectList ol = parentWidget->queryList("QWidget", 0, 0, FALSE);
	    if (ol.count() >= entry)
		targetObject = ol.at(entry - 1);
	}
	break;
    case FocusChild:
	{
	    if (widget()->hasFocus())
		return 0;

	    QWidget *fw = widget()->focusWidget();
	    if ( !fw )
		return -1;

	    QWidget *parent = fw;
	    while (parent && !targetObject) {
		parent = parent->parentWidget();
		if (parent == widget())
		    targetObject = fw;
	    }
	}
	break;
    case QAccessible::Left:
	if (complexWidget && entry) {
	    if (entry < 2 || widget()->height() > widget()->width() + 20) // looks vertical
		return -1;
	    return entry - 1;
	}
	// fall through
    case QAccessible::Right:
	if (complexWidget && entry) {
	    if (entry >= childCount() || widget()->height() > widget()->width() + 20) // looks vertical
		return -1;
	    return entry + 1;
	}
	// fall through
    case QAccessible::Above:
	if (complexWidget && entry) {
	    if (entry < 2 || widget()->width() > widget()->height() + 20) // looks horizontal
		return - 1;
	    return entry - 1;
	}
	// fall through
    case QAccessible::Below:
	if (complexWidget && entry) {
	    if (entry >= childCount() || widget()->width() > widget()->height()  + 20) // looks horizontal
		return - 1;
	    return entry + 1;
	} else {
	    QWidget *start = widget();
	    QWidget *parentWidget = start->parentWidget();
	    if (!parentWidget)
		return -1;
	    QObjectList ol = parentWidget->queryList("QWidget", 0, 0, FALSE);

	    QRect startg = start->geometry();
	    QWidget *candidate = 0;
	    int mindist = 100000;
	    for (int i = 0; i < ol.count(); ++i) {
		QWidget *sibling = static_cast<QWidget*>(ol.at(i));
		if (sibling == start)
		    continue;
		QRect sibg = sibling->geometry();
		QPoint sibp;
		QPoint startp;
		QPoint distp;
		switch (relation) {
		case QAccessible::Left:
		    startp = QPoint(startg.left(), startg.bottom() - startg.top() / 2);
		    sibp = QPoint(sibg.right(), sibg.bottom() - sibg.top() / 2);
		    distp = sibp - startp;
		    if (distp.x() >= 0)
			continue;
		    break;
		case QAccessible::Right:
		    startp = QPoint(startg.right(), startg.bottom() - startg.top() / 2);
		    sibp = QPoint(sibg.left(), sibg.bottom() - sibg.top() / 2);
		    distp = sibp - startp;
		    if (distp.x() <= 0)
			continue;
		    break;
		case QAccessible::Above:
		    startp = QPoint(startg.right() - startg.left() / 2, startg.top());
		    sibp = QPoint(sibg.right() - sibg.left() / 2, sibg.bottom());
		    distp = sibp - startp;
		    if (distp.y() >= 0)
			continue;
		    break;
		case QAccessible::Below:
		    startp = QPoint(startg.right() - startg.left() / 2, startg.bottom());
		    sibp = QPoint(sibg.right() - sibg.left() / 2, sibg.top());
		    distp = sibp - startp;
		    if (distp.y() <= 0)
			continue;
		    break;
		}

		int dist = distp.manhattanLength();
		if (dist < mindist ) {
		    QWidget *oldcandidate = candidate;
		    candidate = sibling;
		    if (candidate && candidate != oldcandidate)
			mindist = dist;
		}
	    }
	    targetObject = candidate;
	}
	break;
    default:
	break;
    }
    QAccessible::queryAccessibleInterface(targetObject, target);
    return *target ? 0 : -1;
}

/*! \reimp */
int QAccessibleWidget::childCount() const
{
    QObjectList cl = widget()->queryList( "QWidget", 0, FALSE, FALSE );
    return cl.count();
}

/*! \reimp */
int QAccessibleWidget::indexOfChild(const QAccessibleInterface *child) const
{
    QObjectList cl = widget()->queryList( "QWidget", 0, FALSE, FALSE );
    int index = cl.indexOf(child->object());
    if (index != -1)
	++index;
    return index;
}

/*! \reimp */
bool QAccessibleWidget::doAction(int action, int control)
{
    if ( control )
	qWarning( "QAccessibleWidget::doAction: This implementation does not support subelements! (ID %d unknown for %s)", control, widget()->className() );

    if (action == SetFocus && widget()->focusPolicy() != QWidget::NoFocus ) {
	widget()->setFocus();
	return TRUE;
    }
    return FALSE;
}

/*! \reimp */
int QAccessibleWidget::defaultAction(int control) const
{
    if ( control )
	qWarning( "QAccessibleWidget::defaultAction: This implementation does not support subelements! (ID %d unknown for %s)", control, widget()->className() );

    return SetFocus;
}

/*! \reimp */
QString QAccessibleWidget::text( Text t, int control ) const
{
    switch ( t ) {
    case Description:
	if ( !control && d->description.isEmpty() ) {
	    QString desc = QToolTip::textFor(widget());
	    return desc;
	}
	return d->description;
    case Help:
	if ( !control && d->help.isEmpty() ) {
	    QString help = QWhatsThis::textFor( widget() );
	    return help;
	}
	return d->help;
    case Accelerator:
	return d->accelerator;
    case Name:
	{
	    if ( !control && d->name.isEmpty() && widget()->isTopLevel() )
		return widget()->caption();
	    return d->name;
	}
    case Value:
	return d->value;
    default:
	break;
    }
    return QString();
}

/*! \reimp */
QString QAccessibleWidget::actionText(int action, Text t, int control) const
{
    if (action != d->defAction || t != Name || control)
	return QString();
    return d->defActionName;
}

/*! \reimp */
QAccessible::Role QAccessibleWidget::role( int control ) const
{
    if ( !control )
	return d->role;
    return NoRole;
}

/*! \reimp */
QAccessible::State QAccessibleWidget::state( int control ) const
{
    if ( control )
	return Normal;

    if ( d->state != Normal )
	return d->state;

    int state = Normal;

    QWidget *w = widget();
    if ( w->isHidden() )
	state |= Invisible;
    if ( w->focusPolicy() != QWidget::NoFocus && w->isActiveWindow() )
	state |= Focusable;
    if ( w->hasFocus() )
	state |= Focused;
    if ( !w->isEnabled() )
	state |= Unavailable;
    if ( w->isTopLevel() ) {
	state |= Moveable;
	if ( w->minimumSize() != w->maximumSize() )
	    state |= Sizeable;
    }

    return (State)state;
}

#endif //QT_ACCESSIBILITY_SUPPORT
