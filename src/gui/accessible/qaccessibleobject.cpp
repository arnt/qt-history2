/****************************************************************************
**
** Implementation of the QAccessibleObject and QAccessibleApplication classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessibleobject.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qapplication.h"
#include "qwidget.h"
#include "qpointer.h"

class QAccessibleObjectPrivate
{
public:
    QPointer<QObject> object;
};

/*!
    \class QAccessibleObject qaccessibleobject.h
    \brief The QAccessibleObject class implements parts of the
    QAccessibleInterface for QObjects.

    \ingroup accessibility

    This class is mainly provided for convenience. All subclasses of
    the QAccessibleInterface that provide implementations of non-widget objects
    should use this class as their base class.
*/

extern void qInsertAccessibleObject(QObject *object, QAccessibleInterface *iface);
extern void qRemoveAccessibleObject(QObject *object);

/*!
    Creates a QAccessibleObject for \a object.
*/
QAccessibleObject::QAccessibleObject(QObject *object)
{
    d = new QAccessibleObjectPrivate;
    d->object = object;

    qInsertAccessibleObject(object, this);
}

/*!
    Destroys the QAccessibleObject.

    This only happens when a call to release() decrements the internal
    reference counter to zero.
*/
QAccessibleObject::~QAccessibleObject()
{
    qRemoveAccessibleObject(d->object);

    delete d;
}

/*!
    \reimp
*/
QRESULT QAccessibleObject::queryInterface(const QUuid &uuid, QUnknownInterface **iface)
{
    *iface = 0;
    if (uuid == IID_QAccessible)
        *iface = (QAccessibleInterface*)this;
    else if (uuid == IID_QUnknown)
        *iface = (QUnknownInterface*)this;
    else
        return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

/*!
    \reimp
*/
QObject *QAccessibleObject::object() const
{
#ifndef QT_NO_DEBUG
    if (!isValid())
        qWarning("QAccessibleInterface is invalid. Crash pending...");
#endif
    return d->object;
}

/*!
    \reimp
*/
bool QAccessibleObject::isValid() const
{
    return !d->object.isNull();
}

/*! \reimp */
QRect QAccessibleObject::rect(int) const
{
    return QRect();
}

/*! \reimp */
void QAccessibleObject::setText(Text, int, const QString &)
{
}

/*!
    \class QAccessibleApplication qaccessibleobject.h
    \brief The QAccessibleApplication class implements the QAccessibleInterface for QApplication.
    \internal
*/

/*!
    Creates a QAccessibleApplication for the QApplication object referenced by qApp.
*/
QAccessibleApplication::QAccessibleApplication()
: QAccessibleObject(qApp)
{
}

// all toplevel widgets except popups and the desktop
static QWidgetList topLevelWidgets()
{
    QWidgetList list;
    const QWidgetList tlw(qApp->topLevelWidgets());
    for (int i = 0; i < tlw.count(); ++i) {
        QWidget *w = tlw.at(i);
        if (!w->isPopup() && !w->isDesktop())
            list.append(w);
    }

    return list;
}

/*! \reimp */
int QAccessibleApplication::childCount() const
{
    return topLevelWidgets().count();
}

/*! \reimp */
int QAccessibleApplication::indexOfChild(const QAccessibleInterface *child) const
{
    if (!child->object()->isWidgetType())
        return -1;

    const QWidgetList tlw(topLevelWidgets());
    int index = tlw.indexOf(static_cast<QWidget*>(child->object()));
    if (index != -1)
        ++index;
    return index;
}

/*! \reimp */
int QAccessibleApplication::childAt(int x, int y) const
{
    const QWidgetList tlw(topLevelWidgets());
    for (int i = 0; i < tlw.count(); ++i) {
        QWidget *w = tlw.at(i);
        if (w->frameGeometry().contains(x,y))
            return i+1;
    }
    return -1;
}

/*! \reimp */
int QAccessibleApplication::relationTo(int child, const QAccessibleInterface *other, int otherChild) const
{
    QObject *o = other ? other->object() : 0;
    if (!o)
        return Unrelated;

    if(o == object()) {
        if (child && !otherChild)
            return Child;
        if (!child && otherChild)
            return Ancestor;
        if (!child && !otherChild)
            return Self;
    }

    QWidgetList tlw(topLevelWidgets());
    if (tlw.contains(qt_cast<QWidget*>(o)))
        return Ancestor;

    for (int i = 0; i < tlw.count(); ++i) {
        QWidget *w = tlw.at(i);
        QObjectList cl(w->queryList());
        if (cl.contains(o))
            return Ancestor;
    }

    return Unrelated;
}

/*! \reimp */
int QAccessibleApplication::navigate(Relation relation, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    QObject *targetObject = 0;

    switch (relation) {
    case Self:
        const_cast<QAccessibleApplication*>(this)->queryInterface(IID_QAccessible, (QUnknownInterface**)target);
        return 0;
    case Child:
        if (entry > 0 && entry <= childCount()) {
            const QWidgetList tlw(topLevelWidgets());
            if (tlw.count() >= entry)
                targetObject = tlw.at(entry-1);
        } else {
            return -1;
        }
        break;
    case FocusChild:
        targetObject = qApp->activeWindow();
        break;
    default:
        break;
    }
    QAccessible::queryAccessibleInterface(targetObject, target);
    return *target ? 0 : -1;
}

/*! \reimp */
QString QAccessibleApplication::text(Text t, int) const
{
    switch (t) {
    case Name:
        if (qApp->mainWidget())
            return qApp->mainWidget()->windowTitle();
        break;
    case Description:
        return qApp->applicationFilePath();
        break;
    }
    return QString();
}

/*! \reimp */
QAccessible::Role QAccessibleApplication::role(int) const
{
    return Application;
}

/*! \reimp */
int QAccessibleApplication::state(int) const
{
    return qApp->activeWindow() ? Focused : Normal;
}

/*! \reimp */
int QAccessibleApplication::numActions(int) const
{
    return 1;
}

/*! \reimp */
bool QAccessibleApplication::doAction(int action, int)
{
    if (action == 0) {
        QWidget *w = qApp->mainWidget();
        if (!w)
            w = topLevelWidgets().at(0);
        if (!w)
            return false;
        w->setActiveWindow();
        return true;
    }
    return false;
}

/*! \reimp */
QString QAccessibleApplication::actionText(int action, Text text, int child) const
{
    QString str;
    if (action == 0 && !child) switch (text) {
    case Name:
        str = QApplication::tr("Activate");
        break;
    case Description:
        str = QApplication::tr("Activates the application main widget");
        break;
    }
    return str;
}

#endif //QT_ACCESSIBILITY_SUPPORT
