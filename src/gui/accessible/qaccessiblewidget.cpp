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
    QObjectList ol = parent->queryList("QLabel", 0, false, false);
    for (int i = 0; i < ol.size(); ++i) {
	QLabel *label = static_cast<QLabel*>(ol.at(i));
	if (label->buddy() == widget)
	    return label->text();
    }

    QGroupBox *groupbox = qt_cast<QGroupBox*>(parent);
    if (groupbox)
	return groupbox->title();

    return QString();
}

QString Q_GUI_EXPORT qacc_stripAmp(const QString &text)
{
    if (text.isEmpty())
	return text;

    const QChar *ch = text.unicode();
    int length = text.length();
    QString str;
    while (length > 0) {
	if (*ch == '&') {
	    ++ch;
	    --length;
	    if (!ch)
		--ch;
	}
	str += *ch;
	++ch;
	--length;
    }
    return str;
}

QString Q_GUI_EXPORT qacc_hotKey(const QString &text)
{
    if (text.isEmpty())
	return text;

    int fa = 0;
    QChar ac;
    while ((fa = text.indexOf('&', fa)) != -1) {
	if (fa == text.length() - 1 || text.at(fa+1) != '&') {
	    ac = text.at(fa+1);
	    break;
	}
    }
    if (ac.isNull())
	return QString();
    return (QString)QKeySequence(Qt::ALT) + ac.toUpper();
}

class QAccessibleWidgetPrivate : public QAccessible
{
public:
    QAccessibleWidgetPrivate()
	:role(Client)
    {}

    Role role;
    QString name;
    QString description;
    QString value;
    QString help;
    QString accelerator;
    QStringList primarySignals;
    const QAccessibleInterface *asking;
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
QAccessibleWidget::QAccessibleWidget(QWidget *w, Role role, const QString &name)
: QAccessibleObject(w)
{
    Q_ASSERT(widget());
    d = new QAccessibleWidgetPrivate();
    d->role = role;
    d->name = name;
    d->asking = 0;
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

/*!
    Returns the parent object, which is either the parent widget, or
    qApp for toplevel widgets.
*/
QObject *QAccessibleWidget::parentObject() const
{
    QObject *parent = object()->parent();
    if (!parent)
	parent = qApp;
    return parent;
}

/*! \reimp */
int QAccessibleWidget::childAt(int x, int y) const
{
    QWidget *w = widget();
    QPoint gp = w->mapToGlobal(QPoint(0, 0));
    if (!QRect(gp.x(), gp.y(), w->width(), w->height()).contains(x, y))
	return -1;

    QObjectList list = w->queryList("QWidget", 0, false, false);
    int ccount = childCount();

    // a complex child
    if (list.size() < ccount) {
	for (int i = 1; i <= ccount; ++i) {
	    if (rect(i).contains(x, y))
		return i;
	}
	return 0;
    }

    QPoint rp = w->mapFromGlobal(QPoint(x, y));
    QWidget *child = 0;
    for (int i = 0; i<list.size(); ++i) {
	child = static_cast<QWidget *>(list.at(i));
	if (!child->isTopLevel() && !child->isHidden() && child->geometry().contains(rp)) {
	    return i + 1;
	}
    }
    return -1;
}

/*! \reimp */
QRect QAccessibleWidget::rect(int child) const
{
    if (child)
	qWarning("QAccessibleWidget::rect: This implementation does not support subelements! (ID %d unknown for %s)", child, widget()->className());

    QWidget *w = widget();
    QPoint wpos = w->mapToGlobal(QPoint(0, 0));

    return QRect(wpos.x(), wpos.y(), w->width(), w->height());
}

#include <private/qobject_p.h>

class ConnectionObject : public QObject
{
    Q_DECLARE_PRIVATE(QObject);
public:
    bool isSender(const QObject *receiver, const char *signal) const;
    QList<QObject*> receiverList(const char *signal) const;
    QList<QObject*> senders() const;
};

bool ConnectionObject::isSender(const QObject *receiver, const char *signal) const
{
    int sigindex = metaObject()->indexOfSignal(signal);
    if (sigindex < 0)
	return false;
    int i = 0;
    QObjectPrivate::Connections::Connection *connections = d_func()->findConnection(sigindex, i);
    if (connections) do {
	if (connections->receiver == receiver)
	    return true;
	connections = d_func()->findConnection(sigindex, i);
    } while (connections);
    return false;
}

QList<QObject*> ConnectionObject::receiverList(const char *signal) const
{
    QList<QObject*> receivers;

    int sigindex = metaObject()->indexOfSignal(signal);
    if (sigindex < 0)
	return receivers;
    int i = 0;
    QObjectPrivate::Connections::Connection *connections = d_func()->findConnection(sigindex, i);
    if (connections) do {
	receivers << connections->receiver;
    	connections = d_func()->findConnection(sigindex, i);
    } while (connections);
    return receivers;
}

QList<QObject*> ConnectionObject::senders() const
{
    QList<QObject*> senders;
    if (!d_func()->senders)
	return senders;
    for (int i = 0; i < d_func()->senders->count; ++i)
	senders << d_func()->senders->senders[i].sender;
    return senders;
}

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
void QAccessibleWidget::setAccelerator(const QString &accel)
{
    d->accelerator = accel;
}

/*! \reimp */
int QAccessibleWidget::relationTo(int child, const QAccessibleInterface *other, int otherChild) const
{
    int relation = Unrelated;
    if (d->asking == this) // recursive call
	return relation;

    QObject *o = other ? other->object() : 0;
    if (!o)
	return relation;

    QWidget *focus = widget()->focusWidget();
    if (object() == focus && o->isAncestorOf(focus))
	relation |= FocusChild;

    ConnectionObject *connectionObject = (ConnectionObject*)object();
    for (int sig = 0; sig < d->primarySignals.count(); ++sig) {
	if (connectionObject->isSender(o, d->primarySignals.at(sig).ascii())) {
	    relation |= Controller;
	    break;
	}
    }
    // test for passive relationships.
    // d->asking protects from endless recursion.
    d->asking = this;
    int inverse = other->relationTo(otherChild, this, child);
    d->asking = 0;

    if (inverse & Controller)
	relation |= Controlled;
    if (inverse & Label)
	relation |= Labelled;

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
	QAccessibleInterface *sibIface = 0;
	QAccessible::queryAccessibleInterface(o, &sibIface);
	Q_ASSERT(sibIface);
	QRect wg = rect(0);
	QRect sg = sibIface->rect(0);
	if (wg.intersects(sg)) {
	    QAccessibleInterface *pIface = 0;
	    sibIface->navigate(Ancestor, 1, &pIface);
	    if (pIface && !((sibIface->state(0) | state(0)) & Invisible)) {
		int wi = pIface->indexOfChild(this);
		int si = pIface->indexOfChild(sibIface);

		if (wi > si)
		    relation |= QAccessible::Covers;
		else
		    relation |= QAccessible::Covered;
		pIface->release();
	    }
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
	sibIface->release();

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

    QObjectList childList = widget()->queryList("QWidget", 0, false, false);
    bool complexWidget = childList.size() < childCount();

    switch (relation) {
    // Hierarchical
    case Self:
	const_cast<QAccessibleWidget*>(this)->queryInterface(IID_QAccessible, (QUnknownInterface**)target);
	return 0;
    case Child:
	if (complexWidget) {
	    if (entry > 0 && entry <= childCount())
		return entry;
	    return -1;
	}else {
	    if (entry > 0 && childList.size() >= entry)
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
	    QObject *parent = parentObject();
	    QAccessibleInterface *iface = 0;
	    QAccessible::queryAccessibleInterface(parent, &iface);
	    if (!iface)
		return -1;

	    iface->navigate(Child, entry, target);
	    iface->release();
	    if (*target)
		return 0;
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
	    QObject *parent = parentObject();
	    QAccessibleInterface *pIface = 0;
	    QAccessible::queryAccessibleInterface(parent, &pIface);
	    if (!pIface)
		return -1;

	    QRect startg = rect(0);
	    QPoint startc = startg.center();
	    QAccessibleInterface *candidate = 0;
	    int mindist = 100000;
	    int sibCount = pIface->childCount();
	    for (int i = 0; i < sibCount; ++i) {
		QAccessibleInterface *sibling = 0;
		pIface->navigate(Child, i+1, &sibling);
		Q_ASSERT(sibling);
		if ((relationTo(0, sibling, 0) & Self) || (sibling->state(0) & QAccessible::Invisible))
		    //ignore ourself and invisible siblings
		    continue;

		QRect sibg = sibling->rect(0);
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

		int dist = (int)sqrt((double)distp.x() * distp.x() + distp.y() * distp.y());
		if (dist < mindist) {
		    if (candidate)
			candidate->release();
		    candidate = sibling;
		    candidate->addRef();
		    mindist = dist;
		}
		sibling->release();
	    }
	    pIface->release();
	    *target = candidate;
	    if (*target)
		return 0;
	}
	break;
    case Covers:
	if (entry > 0) {
	    QObject *parent = parentObject();
	    QAccessibleInterface *pIface = 0;
	    QAccessible::queryAccessibleInterface(parent, &pIface);
	    if (!pIface)
		return -1;

	    QRect r = rect(0);
	    int sibCount = pIface->childCount();
	    QAccessibleInterface *sibling = 0;
	    for (int i = pIface->indexOfChild(this) + 1; i <= sibCount && entry; ++i) {
		pIface->navigate(Child, i, &sibling);
		Q_ASSERT(sibling);
		if (!sibling || (sibling->state(0) & Invisible))
		    continue;
		if (sibling->rect(0).intersects(r))
		    --entry;
		if (!entry)
		    break;
		sibling->release();
		sibling = 0;
	    }
	    pIface->release();
	    if (sibling) {
		*target = sibling;
		return 0;
	    }
	}
	break;
    case Covered:
	if (entry > 0) {
	    QObject *parent = parentObject();
	    QAccessibleInterface *pIface = 0;
	    QAccessible::queryAccessibleInterface(parent, &pIface);
	    if (!pIface)
		return -1;

	    QRect r = rect(0);
	    int index = pIface->indexOfChild(this);
	    QAccessibleInterface *sibling = 0;
	    for (int i = 1; i < index && entry; ++i) {
		pIface->navigate(Child, i, &sibling);
		Q_ASSERT(sibling);
		if (!sibling || (sibling->state(0) & Invisible))
		    continue;
		if (sibling->rect(0).intersects(r))
		    --entry;
		if (!entry)
		    break;
		sibling->release();
		sibling = 0;
	    }
	    pIface->release();
	    if (sibling) {
		*target = sibling;
		return 0;
	    }
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
	if (entry > 0) {
	    QObject *parent = parentObject();
	    QAccessibleInterface *pIface = 0;
	    QAccessible::queryAccessibleInterface(parent, &pIface);
	    if (!pIface)
		return -1;
	    // first check for all siblings that are labels to us
	    // ideally we would go through all objects and check, but that
	    // will be too expensive
	    int sibCount = pIface->childCount();
	    QAccessibleInterface *candidate = 0;
	    for (int i = 0; i < sibCount && entry; ++i) {
		pIface->navigate(Child, i+1, &candidate);
		Q_ASSERT(candidate);
		if (candidate->relationTo(0, this, 0) & Label)
		    --entry;
		if (!entry)
		    break;
		candidate->release();
		candidate = 0;
	    }
	    if (!candidate) {
		if (pIface->relationTo(0, this, 0) & Label)
		    --entry;
		if (!entry)
		    candidate = pIface;
	    }
	    if (pIface != candidate)
		pIface->release();
	    if (candidate) {
		*target = candidate;
		return 0;
	    }
	}
	break;
    case Labelled: // only implemented in subclasses
	break;
    case Controller:
	if (entry > 0) {
	    // check all senders we are connected to,
	    // and figure out which one are controllers to us
	    ConnectionObject *connectionObject = (ConnectionObject*)object();
	    QList<QObject*> allSenders = connectionObject->senders();
	    QList<QObject*> senders;
	    for (int s = 0; s < allSenders.size(); ++s) {
		QAccessibleInterface *test = 0;
		QObject *sender = allSenders.at(s);
		QAccessible::queryAccessibleInterface(sender, &test);
		if (!test)
		    continue;
		if (test->relationTo(0, this, 0)&Controller)
		    senders << sender;
		test->release();
	    }
	    if (entry <= senders.size())
		targetObject = senders.at(entry-1);
	}
	break;
    case Controlled:
	if (entry > 0) {
	    QList<QObject*> allReceivers;
	    ConnectionObject *connectionObject = (ConnectionObject*)object();
	    for (int sig = 0; sig < d->primarySignals.count(); ++sig) {
		QList<QObject*> receivers = connectionObject->receiverList(d->primarySignals.at(sig).ascii());
		allReceivers += receivers;
	    }
	    if (entry <= allReceivers.size())
		targetObject = allReceivers.at(entry-1);
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
    QObjectList cl = widget()->queryList("QWidget", 0, false, false);
    return cl.size();
}

/*! \reimp */
int QAccessibleWidget::indexOfChild(const QAccessibleInterface *child) const
{
    QObjectList cl = widget()->queryList("QWidget", 0, false, false);
    int index = cl.indexOf(child->object());
    if (index != -1)
	++index;
    return index;
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
	    str = widget()->windowTitle();
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
	    str = widget()->whatsThis();
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
int QAccessibleWidget::numActions(int child) const
{
    if (child)
	return 0;

    return (widget()->focusPolicy() != QWidget::NoFocus || widget()->isTopLevel()) ? 1 : 0;
}

/*! \reimp */
QString QAccessibleWidget::actionText(int action, Text t, int child) const
{
    if (child || action)
	return QString();
    switch (t) {
    case Name:
	return "Set Focus";
    case Description:
	return "Passes focus to this widget.";
    }
    return QString();
}

/*! \reimp */
bool QAccessibleWidget::doAction(int action, int child)
{
    if (action != 0 || child || !widget()->isEnabled())
	return false;
    if (widget()->focusPolicy() != QWidget::NoFocus)
	widget()->setFocus();
    else if (widget()->isTopLevel())
	widget()->setActiveWindow();
    else
	return false;
    return true;
}

/*! \reimp */
QAccessible::Role QAccessibleWidget::role(int child) const
{
    if (!child)
	return d->role;
    return NoRole;
}

/*! \reimp */
int QAccessibleWidget::state(int child) const
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
