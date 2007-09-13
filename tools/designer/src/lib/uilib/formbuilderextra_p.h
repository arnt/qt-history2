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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "uilib_global.h"

#ifndef QT_FORMBUILDER_NO_SCRIPT
#    include "formscriptrunner_p.h"
#endif

#include <QtCore/QHash>
#include <QtCore/QPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

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
    void storeCustomWidgetScript(const QString &className, const QString &script);
    QString customWidgetScript(const QString &className) const;
#endif

    void setProcessingLayoutWidget(bool processing);
    bool processingLayoutWidget() const;

    static QFormBuilderExtra *instance(const QAbstractFormBuilder *afb);
    static void removeInstance(const QAbstractFormBuilder *afb);

private:
    typedef QHash<QLabel*, QString> BuddyHash;
    BuddyHash m_buddies;

#ifndef QT_FORMBUILDER_NO_SCRIPT
    QFormScriptRunner m_FormScriptRunner;

    typedef QHash<QString, QString> CustomWidgetScriptHash;
    CustomWidgetScriptHash m_customWidgetScriptHash;
#endif

    bool m_layoutWidget;

    QPointer<QWidget> m_rootWidget;
};

void uiLibWarning(const QString &message);

// Struct with static accessor that provides most strings used in the form builder.
struct QFormBuilderStrings {
    QFormBuilderStrings();

    static const QFormBuilderStrings &instance();

    const QString buddyProperty;
    const QString cursorProperty;
    const QString objectNameProperty;
    const QString trueValue;
    const QString falseValue;
    const QString horizontalPostFix;
    const QString separator;
    const QString defaultTitle;
    const QString titleAttribute;
    const QString labelAttribute;
    const QString toolTipAttribute;
    const QString iconAttribute;
    const QString pixmapAttribute;
    const QString textAttribute;
    const QString currentIndexProperty;
    const QString toolBarAreaAttribute;
    const QString toolBarBreakAttribute;
    const QString dockWidgetAreaAttribute;
    const QString marginProperty;
    const QString spacingProperty;
    const QString leftMarginProperty;
    const QString topMarginProperty;
    const QString rightMarginProperty;
    const QString bottomMarginProperty;
    const QString horizontalSpacingProperty;
    const QString verticalSpacingProperty;
    const QString sizeHintProperty;
    const QString sizeTypeProperty;
    const QString orientationProperty;
    const QString qtHorizontal;
    const QString qtVertical;
    const QString currentRowProperty;
    const QString tabSpacingProperty;
    const QString qWidgetClass;
    const QString lineClass;
    const QString geometryProperty;
    const QString scriptWidgetVariable;
    const QString scriptChildWidgetsVariable;
};
#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

#endif // ABSTRACTFORMBUILDERPRIVATE_H
