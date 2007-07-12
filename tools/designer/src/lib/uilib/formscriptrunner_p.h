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

#ifndef FORMSCRIPTRUNNER_H
#define FORMSCRIPTRUNNER_H

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

#include <QtDesigner/uilib_global.h>
#include <QtCore/QList>
#include <QtCore/QFlags>
#include <QtCore/QString>

class QWidget;

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

class DomWidget;

class QDESIGNER_UILIB_EXPORT QFormScriptRunner
{
public:
    QFormScriptRunner();
    ~QFormScriptRunner();

    typedef QList<QWidget*> WidgetList;

    bool run(const DomWidget *domWidget,
             const QString &customWidgetScript,
             QWidget *widget, const WidgetList &children,
             QString *errorMessage);

    struct Error {
        QString objectName;
        QString script;
        QString errorMessage;
    };
    typedef QList<Error> Errors;
    Errors errors() const;
    void clearErrors();

    enum Option {
         NoOptions = 0x0,
         DisableWarnings = 0x1,
         DisableScripts = 02
     };
     Q_DECLARE_FLAGS(Options, Option)

    Options options() const;
    void setOptions(Options options);

private:
    class QFormScriptRunnerPrivate;
    QFormScriptRunnerPrivate *m_impl;

    QFormScriptRunner(const QFormScriptRunner &);
    void operator = (const QFormScriptRunner &);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFormScriptRunner::Options)

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

#endif // FORMSCRIPTRUNNER_H
