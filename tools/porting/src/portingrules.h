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

#ifndef RULESFROMXML_H
#define RULESFROMXML_H

#include "qtsimplexml.h"
#include "tokenreplacements.h"
#include <QList>
#include <QStringList>

class PortingRules
{
public:
    static void createInstance(QString xmlFilePath);
    static PortingRules *instance();
    static void deleteInstance();

    enum QtVersion{Qt3, Qt4};
    PortingRules(QString xmlFilePath);
    QList<TokenReplacement*> getNoPreprocessPortingTokenRules();
    QStringList getHeaderList(QtVersion qtVersion);
    QStringList getNeededHeaderList();
    QStringList getInheritsQt();
private:
    static PortingRules *theInstance;
    QtSimpleXml xml;
    QList<TokenReplacement*> tokenRules;
    QStringList qt3Headers;
    QStringList qt4Headers;
    QStringList neededHeaders;
    QStringList inheritsQtClass;
    void parseXml();
};

#endif
