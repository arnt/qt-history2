#include "rulesfromxml.h"
#include <QFile>
#include <QStringList>

RulesFromXml::RulesFromXml(QString xmlFilePath)
{
    QFile f(xmlFilePath);
    if(!f.open(QIODevice::ReadOnly)) {
        qFatal("Could not find rule file %s", xmlFilePath.latin1());
    }
    if(!xml.setContent(&f))
        qFatal("Xml parsing failed! Error: %s", xml.errorString().latin1());
}

QList<TokenReplacement*> RulesFromXml::getTokenReplacementRules()
{
    int ruleCount = xml["Rules"]["Count"].text().toInt();

    QList<TokenReplacement*> tokenReplacements;

    int rule;
    for(rule=0; rule<ruleCount; ++rule) {
        if(xml["Rules"][rule].attribute("Type")=="RenamedHeader") {
            TokenReplacement* testReplacement = new IncludeTokenReplacement(xml["Rules"][rule]["Qt3"].text().latin1(),
                                                                            xml["Rules"][rule]["Qt4"].text().latin1());
            tokenReplacements.append(testReplacement);
        }
    }

    return tokenReplacements;
}

QList<TokenReplacement*> RulesFromXml::getNoPreprocessPortingTokenRules()
{
    QList<TokenReplacement*> masterReplaceList;
    masterReplaceList += getTokenReplacementRules();
    //TODO: add further rules here.

    int ruleCount = xml["Rules"]["Count"].text().toInt();
    int rule;
    for(rule=0; rule<ruleCount; ++rule) {
        if(xml["Rules"][rule].attribute("Type")=="RenamedClass" ||
           xml["Rules"][rule].attribute("Type")=="RenamedToken"   ) {
            TokenReplacement* tokenReplacement = new GenericTokenReplacement(xml["Rules"][rule]["Qt3"].text().latin1(),
                                                                             xml["Rules"][rule]["Qt4"].text().latin1());
            masterReplaceList.append(tokenReplacement);
        }

        if(xml["Rules"][rule].attribute("Type")=="RenamedEnumvalue" ||
           xml["Rules"][rule].attribute("Type")=="RenamedType"      ||
           xml["Rules"][rule].attribute("Type")=="RenamedQtSymbol" )
         {
            TokenReplacement* tokenReplacement = new ScopedTokenReplacement(xml["Rules"][rule]["Qt3"].text().latin1(),
                                                                            xml["Rules"][rule]["Qt4"].text().latin1());
            masterReplaceList.append(tokenReplacement);
        }
    }
    return masterReplaceList;
}

QStringList RulesFromXml::getHeaderList(QtVersion qtVersion)
{
    QString ruleText;
    if (qtVersion == Qt3)
        ruleText = "qt3Header";
    else
        ruleText = "qt4Header";
    QStringList headers;
    int ruleCount = xml["Rules"]["Count"].text().toInt();
    for(int rule=0; rule<ruleCount; ++rule) {
         if(xml["Rules"][rule].attribute("Type")==ruleText)
             headers+=xml["Rules"][rule].text();
   }
   return headers;
}

QStringList RulesFromXml::getNeededHeaderList()
{
    QStringList headers;
    int ruleCount = xml["Rules"]["Count"].text().toInt();
    for(int rule=0; rule<ruleCount; ++rule) {
         if(xml["Rules"][rule].attribute("Type")=="NeedHeader")
             headers+=xml["Rules"][rule]["Header"].text();
    }
    return headers;
}





