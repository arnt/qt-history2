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

#ifndef PORTINGRULES_H
#define PORTINGRULES_H

#include "qtsimplexml.h"
#include "tokenreplacements.h"
#include <QList>
#include <QPair>
#include <QHash>
#include <QSet>
#include <QStringList>

class RuleDescription
{
public:
    explicit RuleDescription(QtSimpleXml &replacementRule) {
        qt3 = replacementRule[QLatin1String("Qt3")].text();
        qt4 = replacementRule[QLatin1String("Qt4")].text();
        ruleType = replacementRule.attribute(QLatin1String("Type"));
    }
    QString qt3;
    QString qt4;
    QString ruleType;
    bool operator==(const RuleDescription &other) const
    {
        return (qt3 == other.qt3 && qt4 == other.qt4 && ruleType == other.ruleType);
    }
};

class PortingRules
{
public:
    static void createInstance(QString xmlFilePath);
    static PortingRules *instance();
    static void deleteInstance();

    enum QtVersion{Qt3, Qt4};
    PortingRules(QString xmlFilePath);
    QList<TokenReplacement*> getTokenReplacementRules();
    QStringList getHeaderList(QtVersion qtVersion);
    QHash<QByteArray, QByteArray> getNeededHeaders();
    QStringList getInheritsQt();
    QHash<QByteArray, QByteArray> getClassLibraryList();
    QHash<QByteArray, QByteArray> getHeaderReplacements();
private:
    static PortingRules *theInstance;

    QList<TokenReplacement*> tokenRules;
    QStringList qt3Headers;
    QStringList qt4Headers;
    QHash<QByteArray, QByteArray> neededHeaders;
    QStringList inheritsQtClass;
    QList<RuleDescription> disabledRules;
    QHash<QByteArray, QByteArray> classLibraryList;
    QHash<QByteArray, QByteArray> headerReplacements;


    void parseXml(const QString fileName);
    void checkScopeAddRule(/*const */QtSimpleXml &currentRule);
    QtSimpleXml *loadXml(const QString fileName) const ;
    QString resolveFileName(const QString currentFileName,
                            const QString includeFileName) const;
    bool isReplacementRule(const QString ruleType) const;
    void disableRule(QtSimpleXml &replacementRule);
    bool isRuleDisabled(QtSimpleXml &replacementRule) const;
    void addLogWarning(const QString text) const;
    void addLogError(const QString text) const;
};

#endif // PORTINGRULES_H
