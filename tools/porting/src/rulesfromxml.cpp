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

#include "rulesfromxml.h"
#include <QFile>
#include <iostream>
#include "logger.h"
using std::cout;
using std::endl;

PortingRules *PortingRules::theInstance  = 0;

void PortingRules::createInstance(QString xmlFilePath)
{
    deleteInstance();
    theInstance  = new PortingRules(xmlFilePath);
}

PortingRules *PortingRules::instance()
{
    if(theInstance) {
        return theInstance;
    } else {
        cout << "Error: must create a PortingRules"
             << "instance before calling instance()" << endl;
        return 0;
    }
}

void PortingRules::deleteInstance()
{
    if(theInstance) {
        delete theInstance;
        theInstance = 0;
    }
}

PortingRules::PortingRules(QString xmlFilePath)
{
    QFile f(xmlFilePath);
    if(!f.open(QIODevice::ReadOnly)) {
        qFatal("Could not find rule file %s", xmlFilePath.toLatin1().constData());
    }
    if(!xml.setContent(&f))
        qFatal("Xml parsing failed! Error: %s", xml.errorString().toLatin1().constData());
    parseXml();
}

QList<TokenReplacement*> PortingRules::getNoPreprocessPortingTokenRules()
{
    if(tokenRules.isEmpty())
         Logger::instance()->addEntry(new PlainLogEntry("Warning", "Porting", "Warning: token rules list is empty"));
    return tokenRules;
}

QStringList PortingRules::getHeaderList(QtVersion qtVersion)
{
    if(qt3Headers.isEmpty() || qt4Headers.isEmpty())
         Logger::instance()->addEntry(new PlainLogEntry("Warning", "Porting", "Warning: headers list is empty"));

    if (qtVersion==Qt3)
        return qt3Headers;
    else //Qt4
        return qt4Headers;
}

QStringList PortingRules::getNeededHeaderList()
{
    if(neededHeaders.isEmpty())
         Logger::instance()->addEntry(new PlainLogEntry("Warning", "Porting", "Warning: headers list is empty"));
    return neededHeaders;
}

QStringList PortingRules::getInheritsQt()
{
    if(tokenRules.isEmpty())
        Logger::instance()->addEntry(new PlainLogEntry("Warning", "Porting", "Warning: inheritsQtClass list is empty"));
    return inheritsQtClass;
}

void PortingRules::parseXml()
{
    int ruleCount = xml["Rules"]["Count"].text().toInt();
    ++ruleCount; //Hack! compensate for off-by-one error somewhere in QtSimpleXml

    for(int rule=0; rule<ruleCount; ++rule) {
        QtSimpleXml &currentRule = xml["Rules"][rule];

        QString ruleType = currentRule.attribute("Type");
        if(ruleType == "RenamedHeader") {
                      tokenRules.append(new IncludeTokenReplacement(
                     currentRule["Qt3"].text().toLatin1(),
                     currentRule["Qt4"].text().toLatin1()));
        }
        else if(ruleType == "RenamedClass" || ruleType == "RenamedToken" ) {
            tokenRules.append(new GenericTokenReplacement(
                    currentRule["Qt3"].text().toLatin1(),
                    currentRule["Qt4"].text().toLatin1()));
        }
        else if(ruleType == "RenamedEnumvalue" ||
                ruleType == "RenamedType" ||
                ruleType == "RenamedQtSymbol" ) {
            tokenRules.append(new ScopedTokenReplacement(
                    currentRule["Qt3"].text().toLatin1(),
                    currentRule["Qt4"].text().toLatin1()));
        }
        else if(ruleType == "NeedHeader") {
            neededHeaders += currentRule["Header"].text();
        }
        else if(ruleType == "qt3Header") {
            qt3Headers += currentRule.text();
        }
        else if(ruleType == "qt4Header") {
            qt4Headers += currentRule.text();
        }
        else if(ruleType == "InheritsQt") {
            inheritsQtClass += currentRule.text();
        }
    }
}
