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

using std::cout;
using std::endl;


RulesFromXml::RulesFromXml(QString xmlFilePath)
:isParsed(false)
{
    QFile f(xmlFilePath);
    if(!f.open(QIODevice::ReadOnly)) {
        qFatal("Could not find rule file %s", xmlFilePath.toLatin1().constData());
    }
    if(!xml.setContent(&f))
        qFatal("Xml parsing failed! Error: %s", xml.errorString().toLatin1().constData());
}

QList<TokenReplacement*> RulesFromXml::getNoPreprocessPortingTokenRules()
{
    if(!isParsed) {
        parseXml();
        isParsed=true;
    }

    if(tokenRules.isEmpty()) {
         cout << "Warning: token rules list is empty" << endl;
    }
    return tokenRules;
}

QStringList RulesFromXml::getHeaderList(QtVersion qtVersion)
{
    if(!isParsed) {
       parseXml();
       isParsed=true;
    }
    if(qt3Headers.isEmpty() || qt4Headers.isEmpty()) {
         cout << "Warning: headers list is empty" << endl;
    }

    if (qtVersion==Qt3)
        return qt3Headers;
    else //Qt4
        return qt4Headers;
}

QStringList RulesFromXml::getNeededHeaderList()
{
    if(!isParsed) {
       parseXml();
       isParsed=true;
    }

    if(tokenRules.isEmpty()) {
         cout << "Warning: needed Headers list is empty" << endl;
    }

    return neededHeaders;
}


void RulesFromXml::parseXml()
{
    int ruleCount = xml["Rules"]["Count"].text().toInt();
    ++ruleCount; //Hack! compensate for off-by-one error somewhere in QtSimpleXml

    //parse InheritsQt first, since ScopedTokenReplacement take this list
    //as a parameter
    for(int rule=0; rule<ruleCount; ++rule) {
        QtSimpleXml &currentRule = xml["Rules"][rule];
        if(currentRule.attribute("Type")=="InheritsQt") {
            inheritsQtClass << currentRule.text();
        }
    }

    for(int rule=0; rule<ruleCount; ++rule) {
        QtSimpleXml &currentRule = xml["Rules"][rule];

        if(currentRule.attribute("Type")=="RenamedHeader") {
                      tokenRules.append(new IncludeTokenReplacement(
                     currentRule["Qt3"].text().toLatin1(),
                     currentRule["Qt4"].text().toLatin1()));
        }
        else if(currentRule.attribute("Type")=="RenamedClass" ||
                currentRule.attribute("Type")=="RenamedToken" ) {
            tokenRules.append(new GenericTokenReplacement(
                    currentRule["Qt3"].text().toLatin1(),
                    currentRule["Qt4"].text().toLatin1()));
        }
        else if(currentRule.attribute("Type")=="RenamedEnumvalue" ||
                currentRule.attribute("Type")=="RenamedType" ||
                currentRule.attribute("Type")=="RenamedQtSymbol" ) {
            tokenRules.append(new ScopedTokenReplacement(
                    currentRule["Qt3"].text().toLatin1(),
                    currentRule["Qt4"].text().toLatin1(),
                    inheritsQtClass));
        }
        else if(currentRule.attribute("Type")=="NeedHeader") {
            neededHeaders += currentRule["Header"].text();
        }
        else if(currentRule.attribute("Type")=="qt3Header") {
            qt3Headers += currentRule.text();
        }
        else if(currentRule.attribute("Type")=="qt4Header") {
            qt4Headers += currentRule.text();
        }
    }
}
