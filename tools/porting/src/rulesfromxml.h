#ifndef RULESFROMXML_H
#define RULESFROMXML_H

#include <QList>
#include "qtsimplexml.h"
#include "tokenreplacements.h"

class QStringList;
class RulesFromXml
{
public:
    enum QtVersion{Qt3, Qt4};
    RulesFromXml(QString xmlFilePath);
    QList<TokenReplacement*> getTokenReplacementRules();
    QList<TokenReplacement*> getNoPreprocessPortingTokenRules();
    QStringList getHeaderList(QtVersion qtVersion);
    QStringList getNeededHeaderList();
private:
    QtSimpleXml xml;
};



#endif
