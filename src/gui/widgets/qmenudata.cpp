/****************************************************************************
**
** Definition of QWidget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/#include "qmenudata.h"

#ifdef QT_COMPAT
#include <qaction.h>
#include <qsignal.h>
#include <private/qaction_p.h>
#define d d_func()

class QMenuItemEmitter : public QObject
{
    Q_OBJECT
    QMenuItem *menuitem;
    QSignalEmitter *sig;
public:
    inline QMenuItemEmitter(QMenuItem *mi) : QObject(mi), menuitem(mi) {
        sig = new QSignalEmitter("int");
        QObject::connect(mi, SIGNAL(triggered()), this, SLOT(doSignalEmit()));
    }
    inline ~QMenuItemEmitter() { delete sig; }
    inline QSignalEmitter *signal() const { return sig; }
 private slots:
    void doSignalEmit() {
        int value = menuitem->signalValue();
        sig->activate(&value);
    }
};
#include "qmenudata.moc"

QMenuItem::QMenuItem() : QAction((QWidget*)0)
{
}
 
void QMenuItem::setId(int id)
{
    d->param = d->id = id;
}

int QMenuItem::id() const
{
    return d->id;
}

QSignalEmitter *QMenuItem::signal() const
{
    if(!d->act_signal) {
        QMenuItem *that = const_cast<QMenuItem*>(this);
        that->d->act_signal = new QMenuItemEmitter(that);
    }
    return d->act_signal->signal();
}

void QMenuItem::setSignalValue(int param)
{
    d->param = param;
}

int QMenuItem::signalValue() const
{
    return d->param;
}
#endif
