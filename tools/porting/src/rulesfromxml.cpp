#include "rulesfromxml.h"
#include <QFile>


RulesFromXml::RulesFromXml(QString xmlFilePath)
:isParsed(false)
{
    QFile f(xmlFilePath);
    if(!f.open(QIODevice::ReadOnly)) {
        qFatal("Could not find rule file %s", xmlFilePath.latin1());
    }
    if(!xml.setContent(&f))
        qFatal("Xml parsing failed! Error: %s", xml.errorString().latin1());
}

QList<TokenReplacement*> RulesFromXml::getNoPreprocessPortingTokenRules()
{
    if(!isParsed) {
        parseXml();
        isParsed=true;
    }
    return tokenRules;
}

QStringList RulesFromXml::getHeaderList(QtVersion qtVersion)
{
    if(!isParsed) {
       parseXml();
       isParsed=true;
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
    
    return neededHeaders;
}


void RulesFromXml::parseXml()
{
    int ruleCount = xml["Rules"]["Count"].text().toInt();
        
    for(int rule=0; rule<ruleCount; ++rule) {
        QtSimpleXml &currentRule = xml["Rules"][rule];
        
        if(currentRule.attribute("Type")=="RenamedHeader") {
             tokenRules.append(new IncludeTokenReplacement(
                     currentRule["Qt3"].text().latin1(),
                     currentRule["Qt4"].text().latin1()));
        } 
        else if(currentRule.attribute("Type")=="RenamedClass" ||
                currentRule.attribute("Type")=="RenamedToken" ) {
            tokenRules.append(new GenericTokenReplacement(
                    currentRule["Qt3"].text().latin1(),
                    currentRule["Qt4"].text().latin1()));
        } 
        else if(currentRule.attribute("Type")=="RenamedEnumvalue" ||
                currentRule.attribute("Type")=="RenamedType" ||
                currentRule.attribute("Type")=="RenamedQtSymbol" ) {
            tokenRules.append(new ScopedTokenReplacement(
                    currentRule["Qt3"].text().latin1(),
                    currentRule["Qt4"].text().latin1()));
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




