#ifndef RULESFROMXML_H
#define RULESFROMXML_H

#include <QList>
#include <QStringList>
#include "qtsimplexml.h"
#include "tokenreplacements.h"

class QStringList;
class RulesFromXml
{
public:
    enum QtVersion{Qt3, Qt4};
    RulesFromXml(QString xmlFilePath);
    QList<TokenReplacement*> getNoPreprocessPortingTokenRules();
    QStringList getHeaderList(QtVersion qtVersion);
    QStringList getNeededHeaderList();
private:
    QtSimpleXml xml;
        
    bool isParsed;
    QList<TokenReplacement*> tokenRules;
    QStringList qt3Headers;
    QStringList qt4Headers;
    QStringList neededHeaders;
    QStringList inheritsQtClass;
    void parseXml();    
};



#endif
