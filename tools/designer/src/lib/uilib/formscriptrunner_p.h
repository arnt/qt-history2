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
         DisableCustomWidgetScripts = 0x2,
         DisableScripts = 0x4
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
