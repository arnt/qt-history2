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

#include "qaccel.h"
#include "qapplication.h"
#include "qgroupbox.h"
#include "qlabel.h"
#include "qtooltip.h"
#include "qwhatsthis.h"
#include "qwidget.h"

#include <math.h>

QString buddyString(const QWidget *widget)
{
    if (!widget)
	return QString();
    const QWidget *parent = widget->parentWidget();
    if (!parent)
	return QString();
    QObjectList ol = parent->queryList("QLabel", 0, FALSE, FALSE);
    for (int i = 0; i < ol.count(); ++i) {
	QLabel *label = static_cast<QLabel*>(ol.at(i));
	if (label->buddy() == widget)
	    return label->text();
    }

    QGroupBox *groupbox = qt_cast<QGroupBox*>(parent);
    if (groupbox)
	return groupbox->title();

    return QString();
}

QString Q_EXPORT qacc_stripAmp(const QString &text)
{
    if (text.isEmpty())
	return text;

    const QChar *ch = text.unicode();
    int length = text.length();
    QString str;
    str.reserve(length);
    while (length > 0) {
	if (*ch == '&') {
	    ++ch;
	    if (!ch)
		--ch;
	}
	str += *ch;
	++ch;
	--length;
    }
    return str;
}

QString Q_EXPORT qacc_hotKey(const QString &text)
{
    if (text.isEmpty())
	return text;

    int fa = 0;
    QChar ac;
    while ((fa = text.find('&', fa)) != -1) {
	if (fa == text.length() - 1 || text.at(fa+1) != '&') {
	    ac = text.at(fa+1);
	    break;
	}
    }
    if (ac.isNull())
	return QString();
    return QAccel::keyToString(Qt::Key_Alt) + QString(ac.lower());
}

class QAccessibleWidgetPrivate : public QAccessible
{
public:
    QAccessibleWidgetPrivate()
	:role(Client), defAction(0)
    {}

    Role role;
    QString name;
    QString description;
    QString value;
    QString help;
    int	    defAction;
    QString defActionName;
    QString accelerator;
    QStringList primarySignals;
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
    \a role and \a name are optional parameters for static values
    of the object's respective property.
*/
QAccessibleWidget::QAccessibleWidget(QWidget *w, Role role, QString name)
: QAccessibleObject(w)
{
    Q_ASSERT(widget());
    d = new QAccessibleWidgetPrivate();
    d->role = role;
    d->name = name;
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

    QObjectList list = w->queryList("QWidget", 0, FALSE, FALSE);
    int ccount = childCount();

    // a complex child
    if (list.count() < ccount) {
	for (int i = 1; i <= ccount; ++i) {
	    if (rect(i).contains(x, y))
		return i;
	}
	return 0;
    }

    QPoint rp = w->mapFromGlobal(QPoint(x, y));
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
QRect	QAccessibleWidget::rect(int child) const
{
    if (child)
	qWarning("QAccessibleWidget::rect: This implementation does not support subelements! (ID %d unknown for %s)", child, widget()->className());

    QWidget *w = widget();
    QPoint wpos = w->mapToGlobal(QPoint(0, 0));

    return QRect(wpos.x(), wpos.y(), w->width(), w->height());
}


// ### I don't like this at all. QObject or QMetaObject
// should have a better API for something like this.
// Introspection is a Qt feature - hiding it in obscure
// privat headers makes this very difficult to use
#include <private/qobject_p.h>

class FindConnectionObject : public QObject
{
public:
    bool isConnected(const QObject *receiver, const char *signal);
};

bool FindConnectionObject::isConnected(const QObject *receiver, const char *signal)
{
    int sigindex = metaObject()->indexOfSignal(signal);
    if (sigindex < 0)
	return FALSE;

    int i = 0;
    QObjectPrivate::Connections::Connection *connections = d->findConnection(sigindex, i);
    if (connections) do {
	if (connections->receiver == receiver)
	    return TRUE;
	connections = d->findConnection(sigindex, i);
    } while (connections);
    return FALSE;
}

#undef d

/*! 
    Registers \a signal as a controlling signal.
    
    An object is a Controller to any other object connected to a controlling signal.
*/
void QAccessibleWidget::addControllingSignal(const QString &signal)
{
    QByteArray s = QMetaObject::normalizedSignature(signal.ascii());
    if (object()->metaObject()->indexOfSignal(s) < 0)
	qWarning("Signal %s unknown in %s", (const char*)s, object()->className());
    d->primarySignals << s;
}

/*!
    Sets the value of this interface implementation to \a value. 
    
    The default implementation of text() return the set value for 
    the Value text.

    Note that the object wrapped by this interface is not modified.
*/
void QAccessibleWidget::setValue(const QString &value)
{
    d->value = value;
}

/*!
    Sets the description of this interface implementation to \a desc.
    
    The default implementation of text() return the set value for 
    the Description text.

    Note that the object wrapped by this interface is not modified.
*/
void QAccessibleWidget::setDescription(const QString &desc)
{
    d->description = desc;
}

/*!
    Sets the help of this interface implementation to \a help.
    
    The default implementation of text() return the set value for 
    the Help text.

    Note that the object wrapped by this interface is not modified.
*/
void QAccessibleWidget::setHelp(const QString &help)
{
    d->help = help;
}

/*!
    Sets the accelerator of this interface implementation to \a accel.
    
    The default implementation of text() return the set value for 
    the Accelerator text.

    Note that the object wrapped by this interface is not modified.
*/
void QAccessibleWidget::setAccelerator(const QString &accelerator)
{
    d->accelerator = accelerator;
}

/*!
    Sets the default action of this interface implementation to \a defAction,
    and the name of that action to \a name.
    
    The default implementation of defaultAction() return the set
    default action, and the default implementation of actionText() returns the
    set name for the Name text of the default action.

    Note that the object wrapped by this interface is not modified.
*/
void QAccessibleWidget::setDefaultAction(int defAction, const QString &name)
{
    d->defAction = defAction;
}

/*! \reimp */
int QAccessibleWidget::relationTo(int child, const QAccessibleInterface *other, int otherChild) const
{
    int relation = Unrelated;
    QObject *o = other ? other->object() : 0;
    if (!o || !o->isWidgetType())
	return relation;

    QWidget *focus = widget()->focusWidget();
    if (object() == focus) {
	QObject *focusParent = focus->parent();
	bool focusIsChild = FALSE;
	while(focusParent && !focusIsChild) {
	    focusIsChild = focusParent == o;
	    focusParent = focusParent ->parent();
	}

	if(focusIsChild)
	    relation |= FocusChild;
    }

    FindConnectionObject *findConnection = (FindConnectionObject*)object();
    for (int sig = 0; sig < d->primarySignals.count(); ++sig) {
	if (findConnection->isConnected(o, d->primarySignals.at(sig).ascii())) {
	    relation |= Controller;
	    break;
	}
    }

    if(o == object()) {
	if (child && !otherChild)
	    return relation | Child;
	if (!child && otherChild)
	    return relation | Ancestor;
	if (!child && !otherChild)
	    return relation | Self;
    }

    QObject *parent = object()->parent();
    if (o == parent)
	return relation | Child;

    if (o->parent() == parent) {
	relation |= Sibling;
	QWidget *sibling = static_cast<QWidget*>(o);
	QRect wg = widget()->geometry();
	QRect sg = sibling->geometry();
	if (wg.intersects(sg)) {
	    int wi = parent->children().indexOf(widget());
	    int si = parent->children().indexOf(sibling);

	    if (wi < si)
		relation |= QAccessible::Covers;
	    else
		relation |= QAccessible::Covered;
	} else {
	    QPoint wc = wg.center();
	    QPoint sc = sg.center();
	    if (wc.x() < sc.x())
		relation |= QAccessible::Left;
	    else if(wc.x() > sc.x())
		relation |= QAccessible::Right;
	    if (wc.y() < sc.y())
		relation |= QAccessible::Up;
	    else if (wc.y() > sc.y())
		relation |= QAccessible::Down;
	}

	return relation;
    }

    if (o->isAncestorOf(object()))
	return relation | Descendent;
    if (object()->isAncestorOf(o))
	return relation | Ancestor;

    return relation;
}

/*! \reimp */
int QAccessibleWidget::navigate(Relation relation, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    QObject *targetObject = 0;

    QObjectList childList = widget()->queryList("QWidget", 0, FALSE, FALSE);
    bool complexWidget = childList.count() < childCount();

    switch (relation) {
    // Hierarchical
    case Self:
	const_cast<QAccessibleWidget*>(this)->queryInterface(IID_QAccessible, (QUnknownInterface**)target);
	return 0;
    case Child:
	if (complexWidget) {
	    return entry;
	}else {
	    if (entry > 0 && childList.count() >= entry)
		targetObject = childList.at(entry - 1);
	}
	break;
    case Ancestor:
	{
	    if (entry <= 0)
		return -1;
	    targetObject = widget()->parentWidget();
	    int i;
	    for (i = entry; i > 1 && targetObject; --i)
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
	    if (entry > 0 && ol.count() >= entry)
		targetObject = ol.at(entry - 1);
	}
	break;

    // Geometrical
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
    case QAccessible::Up:
	if (complexWidget && entry) {
	    if (entry < 2 || widget()->width() > widget()->height() + 20) // looks horizontal
		return - 1;
	    return entry - 1;
	}
	// fall through
    case QAccessible::Down:
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
	    QPoint startc = startg.center();
	    QWidget *candidate = 0;
	    int mindist = 100000;
	    for (int i = 0; i < ol.count(); ++i) {
		QWidget *sibling = static_cast<QWidget*>(ol.at(i));
		if (sibling == start)
		    continue;
		QRect sibg = sibling->geometry();
		QPoint sibc = sibg.center();
		QPoint sibp;
		QPoint startp;
		QPoint distp;
		switch (relation) {
		case QAccessible::Left:
		    startp = QPoint(startg.left(), startg.top() + startg.height() / 2);
		    sibp = QPoint(sibg.right(), sibg.top() + sibg.height() / 2);
		    if (QPoint(sibc - startc).x() >= 0)
			continue;
		    distp = sibp - startp;
		    break;
		case QAccessible::Right:
		    startp = QPoint(startg.right(), startg.top() + startg.height() / 2);
		    sibp = QPoint(sibg.left(), sibg.top() + sibg.height() / 2);
		    if (QPoint(sibc - startc).x() <= 0)
			continue;
		    distp = sibp - startp;
		    break;
		case QAccessible::Up:
		    startp = QPoint(startg.left() + startg.width() / 2, startg.top());
		    sibp = QPoint(sibg.left() + sibg.width() / 2, sibg.bottom());
		    if (QPoint(sibc - startc).y() >= 0)
			continue;
		    distp = sibp - startp;
		    break;
		case QAccessible::Down:
		    startp = QPoint(startg.left() + startg.width() / 2, startg.bottom());
		    sibp = QPoint(sibg.left() + sibg.width() / 2, sibg.top());
		    if (QPoint(sibc - startc).y() <= 0)
			continue;
		    distp = sibp - startp;
		    break;
		}

		int dist = (int)sqrt(distp.x() * distp.x() + distp.y() * distp.y());
		if (dist < mindist) {
		    QWidget *oldcandidate = candidate;
		    candidate = sibling;
		    if (candidate && candidate != oldcandidate)
			mindist = dist;
		}
	    }
	    targetObject = candidate;
	}
	break;

    // Logical
    case FocusChild:
	{
	    if (widget()->hasFocus())
		return 0;

	    QWidget *fw = widget()->focusWidget();
	    if (!fw)
		return -1;

	    QWidget *parent = fw;
	    while (parent && !targetObject) {
		parent = parent->parentWidget();
		if (parent == widget())
		    targetObject = fw;
	    }
	}
	break;
    case Label:
	{
	    // Tricky one - either our parent is a groupbox
	    // or we have to go through all labels and check
	    // their buddies.
	    // Ideally we would go through all objects and
	    // check if they are a label to us, but that might
	    // be very expensive
	}
	break;
    case Controller:
	{
	    // Need some sort of "controllingSlot" or
	    // "controllingProperty" here, then we can check
	    // all senders we are connected to, and see if they
	    // are a controller to us.
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
    QObjectList cl = widget()->queryList("QWidget", 0, FALSE, FALSE);
    return cl.count();
}

/*! \reimp */
int QAccessibleWidget::indexOfChild(const QAccessibleInterface *child) const
{
    QObjectList cl = widget()->queryList("QWidget", 0, FALSE, FALSE);
    int index = cl.indexOf(child->object());
    if (index != -1)
	++index;
    return index;
}

/*! \reimp */
bool QAccessibleWidget::doAction(int action, int child)
{
    if (child)
	qWarning("QAccessibleWidget::doAction: This implementation does not support subelements! (ID %d unknown for %s)", child, widget()->className());

    if (action == SetFocus && widget()->focusPolicy() != QWidget::NoFocus) {
	widget()->setFocus();
	return TRUE;
    }
    return FALSE;
}

/*! \reimp */
int QAccessibleWidget::defaultAction(int child) const
{
    if (child)
	qWarning("QAccessibleWidget::defaultAction: This implementation does not support subelements! (ID %d unknown for %s)", child, widget()->className());

    return SetFocus;
}

/*! \reimp */
QString QAccessibleWidget::text(Text t, int child) const
{
    QString str;

    switch (t) {
    case Name:
	if (!d->name.isEmpty())
	    str = d->name;
	else if (!child && widget()->isTopLevel())
	    str = widget()->caption();
	else
	    str = qacc_stripAmp(buddyString(widget()));
	break;
    case Description:
	if (!d->description.isEmpty())
	    str = d->description;
	else
	    str = QToolTip::textFor(widget());
	break;
    case Help:
	if (!d->help.isEmpty())
	    str = d->help;
	else
	    str = QWhatsThis::textFor(widget());
	break;
    case Accelerator:
	if (!d->accelerator.isEmpty())
	    str = d->accelerator;
	else
	    str = qacc_hotKey(buddyString(widget()));
	break;
    case Value:
	str = d->value;
	break;
    default:
	break;
    }
    return str;
}

/*! \reimp */
QString QAccessibleWidget::actionText(int action, Text t, int child) const
{
    if (child || t != Name || action != defaultAction(0))
	return QString();
    return d->defActionName;
}

/*! \reimp */
QAccessible::Role QAccessibleWidget::role(int child) const
{
    if (!child)
	return d->role;
    return NoRole;
}

/*! \reimp */
QAccessible::State QAccessibleWidget::state(int child) const
{
    if (child)
	return Normal;

    int state = Normal;

    QWidget *w = widget();
    if (w->isHidden())
	state |= Invisible;
    if (w->focusPolicy() != QWidget::NoFocus && w->isActiveWindow())
	state |= Focusable;
    if (w->hasFocus())
	state |= Focused;
    if (!w->isEnabled())
	state |= Unavailable;
    if (w->isTopLevel()) {
	if (w->testWFlags(WStyle_Title))
	    state |= Moveable;
	if (w->minimumSize() != w->maximumSize())
	    state |= Sizeable;
    }

    return (State)state;
}

#endif //QT_ACCESSIBILITY_SUPPORT
