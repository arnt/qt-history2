/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ABSTRACTFORMBUILDERPRIVATE_H
#define ABSTRACTFORMBUILDERPRIVATE_H

#include "uilib_global.h"

#ifndef QT_FORMBUILDER_NO_SCRIPT
#    include "formscriptrunner_p.h"
#endif

#include <QtCore/QHash>
#include <QtCore/QPointer>
#include <QtCore/QString>

class QObject;
class QVariant;
class QWidget;
class QObject;
class QLabel;

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

class QAbstractFormBuilder;

class QDESIGNER_UILIB_EXPORT QFormBuilderExtra
{
    QFormBuilderExtra();
public:
    void clear();

    bool applyPropertyInternally(QObject *o, const QString &propertyName, const QVariant &value);

    enum BuddyMode { BuddyApplyAll, BuddyApplyVisibleOnly };

    void applyInternalProperties() const;
    static bool applyBuddy(const QString &buddyName, BuddyMode applyMode, QLabel *label);

    const QPointer<QWidget> &rootWidget() const;
    void setRootWidget(const QPointer<QWidget> &w);

#ifndef QT_FORMBUILDER_NO_SCRIPT
    QFormScriptRunner &formScriptRunner();
#endif

    static QFormBuilderExtra *instance(const QAbstractFormBuilder *afb);
    static void removeInstance(const QAbstractFormBuilder *afb);

private:
    const QString m_buddyPropertyName;

    typedef QHash<QLabel*, QString> BuddyHash;
    BuddyHash m_buddies;

#ifndef QT_FORMBUILDER_NO_SCRIPT
    QFormScriptRunner m_FormScriptRunner;
#endif

    QPointer<QWidget> m_rootWidget;
};

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

#endif // ABSTRACTFORMBUILDERPRIVATE_H
