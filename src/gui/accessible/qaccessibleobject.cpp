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

#include "qaccessibleobject.h"

#ifndef QT_NO_ACCESSIBILITY

#include "qapplication.h"
#include "qwidget.h"
#include "qpointer.h"
#include "qmetaobject.h"
#include "qvarlengtharray.h"

class QAccessibleObjectPrivate
{
public:
    QPointer<QObject> object;

    QList<QByteArray> actionList() const;
};

QList<QByteArray> QAccessibleObjectPrivate::actionList() const
{
    QList<QByteArray> actionList;

    if (!object)
        return actionList;

    const QMetaObject *mo = object->metaObject();
    Q_ASSERT(mo);

    QByteArray defaultAction = QMetaObject::normalizedSignature(
        mo->classInfo(mo->indexOfClassInfo("DefaultSlot")).value());

    for (int i = 0; i < mo->memberCount(); ++i) {
        const QMetaMember member = mo->member(i);
        if (member.memberType() != QMetaMember::Slot && member.access() != QMetaMember::Public)
            continue;

        if (!qstrcmp(member.tag(), "QACCESSIBLE_SLOT")) {
            if (member.signature() == defaultAction)
                actionList.prepend(defaultAction);
            else
                actionList << member.signature();
        }
    }

    return actionList;
}

/*!
    \class QAccessibleObject qaccessibleobject.h
    \brief The QAccessibleObject class implements parts of the
    QAccessibleInterface for QObjects.

    \ingroup accessibility

    This class is mainly provided for convenience. All subclasses of
    the QAccessibleInterface that provide implementations of non-widget objects
    should use this class as their base class.
*/

/*!
    Creates a QAccessibleObject for \a object.
*/
QAccessibleObject::QAccessibleObject(QObject *object)
{
    d = new QAccessibleObjectPrivate;
    d->object = object;
}

/*!
    Destroys the QAccessibleObject.

    This only happens when a call to release() decrements the internal
    reference counter to zero.
*/
QAccessibleObject::~QAccessibleObject()
{
    delete d;
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

/*! \reimp */
int QAccessibleObject::userActionCount(int) const
{
    return 0;
}

/*! \reimp */
bool QAccessibleObject::doAction(int, int, const QVariantList &)
{
    return false;
}

static const char * const action_text[][5] =
{
    // Name, Description, Value, Help, Accelerator
    { "Press", "", "", "", "Space" },
    { "SetFocus", "Passes focus to this widget", "", "", "" },
    { "Increase", "", "", "", "" },
    { "Decrease", "", "", "", "" },
    { "Accept", "", "", "", "" },
    { "Cancel", "", "", "", "" },
    { "Select", "", "", "", "" },
    { "ClearSelection", "", "", "", "" },
    { "RemoveSelection", "", "", "", "" },
    { "ExtendSelection", "", "", "", "" },
    { "AddToSelection", "", "", "", "" }
};

/*! \reimp */
QString QAccessibleObject::actionText(int action, Text t, int child) const
{
    if (child || action > FirstStandardAction || action < LastStandardAction || t > Accelerator)
        return QString();

    return QString(action_text[-(action - FirstStandardAction)][t]);
}


/*!
    \class QAccessibleApplication qaccessibleobject.h
    \brief The QAccessibleApplication class implements the QAccessibleInterface for QApplication.

    \internal

    \ingroup accessibility
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
QAccessible::Relation QAccessibleApplication::relationTo(int child, const
        QAccessibleInterface *other, int otherChild) const
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
        QObjectList cl = w->findChildren(QString());
        if (cl.contains(o))
            return Ancestor;
    }

    return Unrelated;
}

/*! \reimp */
int QAccessibleApplication::navigate(RelationFlag relation, int entry,
                                     QAccessibleInterface **target) const
{
    if (!target)
        return -1;

    *target = 0;
    QObject *targetObject = 0;

    switch (relation) {
    case Self:
        targetObject = object();
        break;
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
    *target = QAccessible::queryAccessibleInterface(targetObject);
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
    default:
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
QAccessible::State QAccessibleApplication::state(int) const
{
    return qApp->activeWindow() ? Focused : Normal;
}

/*! \reimp */
int QAccessibleApplication::userActionCount(int) const
{
    return 1;
}

/*! \reimp */
bool QAccessibleApplication::doAction(int action, int child, const QVariantList &param)
{
    if (action == 0 || action == 1) {
        QWidget *w = qApp->mainWidget();
        if (!w)
            w = topLevelWidgets().at(0);
        if (!w)
            return false;
        w->activateWindow();
        return true;
    }
    return QAccessibleObject::doAction(action, child, param);
}

/*! \reimp */
QString QAccessibleApplication::actionText(int action, Text text, int child) const
{
    QString str;
    if ((action == 0 || action == 1) && !child) switch (text) {
    case Name:
        return QApplication::tr("Activate");
    case Description:
        return QApplication::tr("Activates the application main widget");
    default:
        break;
    }
    return QAccessibleObject::actionText(action, text, child);
}

#endif //QT_NO_ACCESSIBILITY
