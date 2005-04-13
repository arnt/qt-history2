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
#include "qmenudata.h"

#ifdef QT3_SUPPORT
#include <qaction.h>
#include <qsignal.h>
#include <private/qaction_p.h>

struct QActionAccessor { QActionPrivate *d_ptr; };
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
        int value = reinterpret_cast<QActionAccessor*>(menuitem)->d_ptr->param;
        sig->activate(&value);
    }
};
#include "qmenudata.moc"

/*!
    \class QMenuItem
    \brief The QMenuItem class represents an item in a menu.

    \compat

    Use QAction instead.
*/

/*!
    \compat
    Constructs a new menu item.
*/
QMenuItem::QMenuItem() : QAction((QWidget*)0)
{
}

void QMenuItem::setId(int id)
{
    d_func()->param = d_func()->id = id;
}

/*!
    \compat
    Returns the menu item's ID.
*/
int QMenuItem::id() const
{
    return d_func()->id;
}

/*!
    \compat
    Returns the signal emitter for the menu item.
*/
QSignalEmitter *QMenuItem::signal() const
{
    if(!d_func()->act_signal) {
        QMenuItem *that = const_cast<QMenuItem*>(this);
        that->d_func()->act_signal = new QMenuItemEmitter(that);
    }
    return d_func()->act_signal->signal();
}

void QMenuItem::setSignalValue(int param)
{
    d_func()->param = param;
}

/*!
    \compat
    Returns the signal value for the menu item.
*/
int QMenuItem::signalValue() const
{
    return d_func()->param;
}
#endif
