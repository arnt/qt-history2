/****************************************************************************
**
** Implementation of QAction class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaction_p.h"
#include "qapplication.h"
#include "qevent.h"
#include "qlist.h"

#define d d_func()
#define q q_func()

QAccel *QActionPrivate::actionAccels = 0;

/* QAction code */
void QActionPrivate::sendDataChanged()
{
    emit q->dataChanged();
    QActionEvent e(QEvent::ActionChanged, q);
    QApplication::sendEvent(q, &e);
}

QAction::QAction(QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->group = parent;
}

QAction::QAction(QWidget* parent) : QObject(*(new QActionPrivate), parent)
{
}

QAction::QAction(const QString &text, Q4Menu *menu, QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->group = parent;
    d->menu = menu;
}

QAction::QAction(const QString &text, QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->group = parent;
}

QAction::QAction(const QIconSet &icon, const QString &text, QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->icons = new QIconSet(icon);
    d->text = text;
    d->group = parent;
}

QAction::QAction(const QString &text, Q4Menu *menu, QWidget* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->menu = menu;
}

QAction::QAction(const QString &text, QWidget* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
}

QAction::QAction(const QIconSet &icon, const QString &text, QWidget* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->icons = new QIconSet(icon);
}

#ifndef QT_NO_ACCEL
QAction::QAction(const QString& text, QKeySequence accel, QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->group = parent;
    d->text = text;
    setAccel(accel);
}

QAction::QAction(const QIconSet& icon, const QString& text, QKeySequence accel,
		   QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->group = parent;
    d->text = text;
    setAccel(accel);
    d->icons = new QIconSet(icon);
}

QAction::QAction(const QString& text, QKeySequence accel, QWidget* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    setAccel(accel);
}

QAction::QAction(const QIconSet& icon, const QString& text, QKeySequence accel,
		   QWidget* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    setAccel(accel);
    d->icons = new QIconSet(icon);
}

void QAction::setAccel(const QKeySequence &accel)
{
    if(!accel.isEmpty()) {
	if(!QActionPrivate::actionAccels) {
	    QActionPrivate::actionAccels = new QAccel(0, qApp);
	} else if(d->accel != -1) {
	    QActionPrivate::actionAccels->removeItem(d->accel);
	    d->accel = -1;
	}
	d->accel = QActionPrivate::actionAccels->insertItem(accel);
	QActionPrivate::actionAccels->connectItem(d->accel, this, SLOT(sendAccelActivated()));
    }
    d->sendDataChanged();
}

QKeySequence QAction::accel() const
{
    QKeySequence ret;
    if(QActionPrivate::actionAccels && d->accel != -1)
	ret = QActionPrivate::actionAccels->key(d->accel);
    return ret;
}
#endif

QAction::~QAction()
{
}

void QAction::setActionGroup(QActionGroup *group)
{
    if(group == d->group)
	return;

    if(d->group)
	d->group->removeAction(this);
    d->group = group;
    d->group->addAction(this);
}

QActionGroup *QAction::actionGroup() const
{
    return d->group;
}

void QAction::setIcon(const QIconSet &icons)
{
    d->icons = new QIconSet(icons);
    d->sendDataChanged();
}

QIconSet QAction::icon() const
{
    if(d->icons)
	return *d->icons;
    return QIconSet();
}

void QAction::setMenu(Q4Menu *menu)
{
    d->menu = menu;
    d->sendDataChanged();
}

Q4Menu *QAction::menu() const
{
    return d->menu;
}

void QAction::setSeparator(bool b)
{
    d->separator = b;
    d->sendDataChanged();
}

bool QAction::isSeparator() const
{
    return d->separator;
}

void QAction::setText(const QString &text)
{
    d->text = text;
    d->sendDataChanged();
}


QString QAction::text() const
{
    return d->text;
}

void QAction::setToolTip(const QString &tooltip)
{
    d->tooltip = tooltip;
    d->sendDataChanged();
}

QString QAction::toolTip() const
{
    return d->tooltip;
}

void QAction::setStatusTip(const QString &statustip)
{
    d->statustip = statustip;
    d->sendDataChanged();
}

QString QAction::statusTip() const
{
    return d->statustip;
}

void QAction::setWhatsThis(const QString &whatsthis)
{
    d->whatsthis = whatsthis;
    d->sendDataChanged();
}

QString QAction::whatsThis() const
{
    return d->whatsthis;
}

void QAction::setCheckable(bool b)
{
    d->checkable = b;
    d->sendDataChanged();
}

bool QAction::isCheckable() const
{
    return d->checkable;
}

void QAction::setChecked(bool b)
{
    d->checkable = b;
    d->sendDataChanged();
}

bool QAction::isChecked() const
{
    return d->checkable;
}

void QAction::setEnabled(bool b)
{
    d->enabled = b;
    d->sendDataChanged();
}

bool QAction::isEnabled() const
{
    return d->enabled;
}

void QAction::setVisible(bool b)
{
    d->visible = b;
    d->sendDataChanged();
}


bool QAction::isVisible() const
{
    return d->visible;
}

void QAction::sendAccelActivated()
{
    activate(Trigger);
}

void QAction::activate(ActionEvent event)
{
    if(event == Trigger)
	emit triggered();
    else if(event == Hover)
	emit hovered();
}

/* QActionGroup code */
QActionGroup::QActionGroup(QObject* parent) : QObject(*new QActionGroupPrivate, parent)
{
}

QActionGroup::QActionGroup(QObject* parent, bool e) : QObject(*new QActionGroupPrivate, parent)
{
    d->exclusive = e;
}

QActionGroup::~QActionGroup()
{
}

QAction *QActionGroup::addAction(QAction* a)
{
    if(!d->actions.contains(a)) {
	d->actions.append(a);
	QObject::connect(a, SIGNAL(triggered()), this, SLOT(internalTriggered()));
	QObject::connect(a, SIGNAL(hovered()), this, SLOT(internalHovered()));
    }
    if(d->exclusive)
	a->setCheckable(true);
    a->setEnabled(d->enabled);
    a->setVisible(d->visible);
    return a;
}

#ifndef QT_NO_ACCEL
QAction *QActionGroup::addAction(const QString& text, QKeySequence accel)
{
    return new QAction(text, accel, this);
}

QAction *QActionGroup::addAction(const QIconSet& icon, const QString& text, QKeySequence accel)
{
    return new QAction(icon, text, accel, this);
}
#endif

void QActionGroup::removeAction(QAction *a)
{
    d->actions.remove(a);
}

QList<QAction*> QActionGroup::actionList() const
{
    return d->actions;
}

void QActionGroup::setExclusive(bool b)
{
    d->exclusive = b;
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it)
	(*it)->setCheckable(b);
}

bool QActionGroup::isExclusive() const
{
    return d->exclusive;
}

void QActionGroup::setEnabled(bool b)
{
    d->enabled = b;
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it)
	(*it)->setEnabled(b);
}

bool QActionGroup::isEnabled() const
{
    return d->enabled;
}

QAction *QActionGroup::checked() const
{
    return d->current;
}

void QActionGroup::setVisible(bool b)
{
    d->visible = b;
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it)
	(*it)->setVisible(b);
}

bool QActionGroup::isVisible() const
{
    return d->visible;
}

void QActionGroup::childEvent(QChildEvent* e)
{
    if(e->type() == QEvent::ChildAdded) {
	if(QAction *action = qt_cast<QAction*>(sender()))
	    addAction(action);
    }
    QObject::childEvent(e);
}

void QActionGroup::internalTriggered()
{
    QAction *action = qt_cast<QAction*>(sender());
    if(!action)
	qWarning("not possible..");
    d->current = action;
    emit triggered(action);
}

void QActionGroup::internalHovered()
{
    QAction *action = qt_cast<QAction*>(sender());
    if(!action)
	qWarning("not possible..");
    d->current = action;
    emit hovered(action);
}
