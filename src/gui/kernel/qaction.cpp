/****************************************************************************
**
** Implementation of QAction class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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
    if(parent)
        parent->addAction(this);
}

QAction::QAction(QWidget* parent) : QObject(*(new QActionPrivate), parent)
{
}

QAction::QAction(const QString &text, QMenu *menu, QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->menu = menu;
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

QAction::QAction(const QString &text, QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

QAction::QAction(const QIconSet &icon, const QString &text, QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->icons = new QIconSet(icon);
    d->text = text;
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

QAction::QAction(const QString &text, QMenu *menu, QWidget* parent) : QObject(*(new QActionPrivate), parent)
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
    d->text = text;
    setAccel(accel);
    d->group = parent;
    if(parent)
        parent->addAction(this);
}

QAction::QAction(const QIconSet& icon, const QString& text, QKeySequence accel,
                   QActionGroup* parent) : QObject(*(new QActionPrivate), parent)
{
    d->text = text;
    setAccel(accel);
    d->icons = new QIconSet(icon);
    d->group = parent;
    if(parent)
        parent->addAction(this);
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
    if(group)
        group->addAction(this);
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

void QAction::setMenu(QMenu *menu)
{
    d->menu = menu;
    d->sendDataChanged();
}

QMenu *QAction::menu() const
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
    d->checked = b;
    d->sendDataChanged();
}

bool QAction::isChecked() const
{
    return d->checked;
}

void QAction::setEnabled(bool b)
{
    d->enabled = b;
    d->forceDisabled = !b;
    d->sendDataChanged();
}

bool QAction::isEnabled() const
{
    return d->enabled;
}

void QAction::setVisible(bool b)
{
    d->forceInvisible = !b;
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

QActionGroup::~QActionGroup()
{
}

QAction *QActionGroup::addAction(QAction* a)
{
    if(!d->actions.contains(a)) {
        d->actions.append(a);
        QObject::connect(a, SIGNAL(triggered()), this, SLOT(internalTriggered()));
        QObject::connect(a, SIGNAL(dataChanged()), this, SLOT(internalDataChanged()));
        QObject::connect(a, SIGNAL(hovered()), this, SLOT(internalHovered()));
    }
    if(d->exclusive)
        a->setCheckable(true);
    if(!a->d->forceDisabled) {
        a->setEnabled(d->enabled);
        a->d->forceDisabled = false;
    }
    if(!a->d->forceInvisible) {
        a->setVisible(d->visible);
        a->d->forceInvisible = false;
    }
    if(a->actionGroup() != this)
        a->setActionGroup(this);
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
    a->setActionGroup(0);
}

QList<QAction*> QActionGroup::actions() const
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
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it) {
        if(!(*it)->d->forceDisabled) {
            (*it)->setEnabled(b);
            (*it)->d->forceDisabled = false;
        }
    }
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
    for(QList<QAction*>::Iterator it = d->actions.begin(); it != d->actions.end(); ++it) {
        if(!(*it)->d->forceInvisible) {
            (*it)->setVisible(b);
            (*it)->d->forceInvisible = false;
        }
    }
}

bool QActionGroup::isVisible() const
{
    return d->visible;
}

void QActionGroup::childEvent(QChildEvent* e)
{
    if(e->type() == QEvent::ChildAdded) {
        if(QAction *action = qt_cast<QAction*>(e->child()))
            addAction(action);
    } else if(e->type() == QEvent::ChildRemoved) {
        if(QAction *action = qt_cast<QAction*>(e->child()))
            removeAction(action);
    }
    QObject::childEvent(e);
}

void QActionGroup::internalDataChanged()
{
    QAction *action = qt_cast<QAction*>(sender());
    if(!action)
        qWarning("not possible..");
    if(d->exclusive && action->isChecked() && action != d->current) {
        if(d->current)
            d->current->setChecked(false);
        d->current = action;
    }
}

void QActionGroup::internalTriggered()
{
    QAction *action = qt_cast<QAction*>(sender());
    if(!action)
        qWarning("not possible..");
    emit triggered(action);
}

void QActionGroup::internalHovered()
{
    QAction *action = qt_cast<QAction*>(sender());
    if(!action)
        qWarning("not possible..");
    emit hovered(action);
}
