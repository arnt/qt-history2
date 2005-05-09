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

#include "portingrules.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <iostream>
#include "logger.h"
#include "qtsimplexml.h"
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
        cout << "Error: must create a PortingRules instance with"
             << "createInstance() before calling instance()" << endl;
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
    parseXml(xmlFilePath);
}

QList<TokenReplacement*> PortingRules::getTokenReplacementRules()
{
    if(tokenRules.isEmpty())
         addLogWarning("Warning: token rules list is empty");
    return tokenRules;
}

QStringList PortingRules::getHeaderList(QtVersion qtVersion)
{
    if(qt3Headers.isEmpty() || qt4Headers.isEmpty())
         addLogWarning("Warning: headers list is empty");

    if (qtVersion==Qt3)
        return qt3Headers;
    else //Qt4
        return qt4Headers;
}

QHash<QByteArray, QByteArray> PortingRules::getNeededHeaders()
{
    if(neededHeaders.isEmpty())
         addLogWarning("Warning: needed headers list is empty");
    return neededHeaders;
}

QStringList PortingRules::getInheritsQt()
{
    if(tokenRules.isEmpty())
        addLogWarning("Warning: inheritsQtClass list is empty");
    return inheritsQtClass;
}

QHash<QByteArray, QByteArray> PortingRules::getClassLibraryList()
{
    if(classLibraryList.isEmpty())
        addLogWarning("Warning: classLibraryList list is empty");
    return classLibraryList;
}
/*
    Loads rule xml file given by fileName, and sets up data structures.
    The rules can generally be divided into to types, replacement rules and
    info rules.

    Replacement rules has the form Qt3Symobl -> Qt4Symbol
    Info rules includes the NeedHeader, Qt3Header, Qt4Header, InhertitsQt
    rule types.
*/
void PortingRules::parseXml(QString fileName)
{
    QtSimpleXml *xmlPointer = loadXml(fileName);
    QtSimpleXml &xml = *xmlPointer;

    int ruleCount = xml["Rules"].numChildren();
    ++ruleCount;

    for(int rule=0; rule<ruleCount; ++rule) {
        QtSimpleXml &currentRule = xml["Rules"][rule];
        QString ruleType = currentRule.attribute("Type");

        if(isReplacementRule(ruleType)) {
            QString qt3Symbol = currentRule["Qt3"].text();
            QString qt4Symbol = currentRule["Qt4"].text();

            QString disable = currentRule.attribute("Disable");
            if(disable == "True" || disable == "true") {
                disableRule(currentRule);
                continue;
            }

            if (isRuleDisabled(currentRule))
                continue;

            if(ruleType == "RenamedHeader") {
                tokenRules.append(new IncludeTokenReplacement(
                        qt3Symbol.toLatin1(), qt4Symbol.toLatin1()));
            } else if(ruleType == "RenamedClass" || ruleType == "RenamedToken" ) {
                tokenRules.append(new ClassNameReplacement(
                        qt3Symbol.toLatin1(), qt4Symbol.toLatin1()));
            } else if(ruleType == "RenamedEnumvalue" || ruleType == "RenamedType" ||
                    ruleType == "RenamedQtSymbol" ) {
                checkScopeAddRule(currentRule);
            }
        } else if(ruleType == "NeedHeader") {
            const QByteArray className = currentRule["Class"].text().toLatin1();
            const QByteArray headerName = currentRule["Header"].text().toLatin1();
            neededHeaders.insert(className, headerName);
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
        else if(ruleType == "Qt4Class") {
            // Get library name, make it lowercase and chop of the "Qt" prefix.
            const QByteArray libraryName = currentRule["Library"].text().toLatin1().toLower().mid(2);
            classLibraryList.insert(currentRule["Name"].text().toLatin1(), libraryName);
        }
    }

    QString includeFile = xml["Rules"]["Include"].text();

    if(includeFile != QString()) {
        QString resolvedIncludeFile = resolveFileName(fileName, includeFile);
        if (!resolvedIncludeFile.isEmpty())
            parseXml(resolvedIncludeFile);
    }

    delete xmlPointer;
}

/*
    Check if the rule in currentRule describes a qualified name
    (like QButton::ToggleState). If so, create a scoped ScopedTokenReplacement,
    else create a GenericTokenReplacement
*/
void PortingRules::checkScopeAddRule(/*const */QtSimpleXml &currentRule)
{
    QByteArray oldToken = currentRule["Qt3"].text().toLatin1();
    QByteArray newToken = currentRule["Qt4"].text().toLatin1();

    if (oldToken.contains("::"))
        tokenRules.append(new ScopedTokenReplacement(oldToken, newToken));
    else
        tokenRules.append(new GenericTokenReplacement(oldToken, newToken));
}

/*
    Loads the xml-file given by fileName into a new'ed QtSimpleXml, which is
    returned by pointer.
*/
QtSimpleXml *PortingRules::loadXml(const QString fileName) const
{
    QFile f(fileName);
    if(!f.open(QIODevice::ReadOnly)) {
        qFatal("Could not find rule file %s", fileName.toLatin1().constData());
    }
    QtSimpleXml *xml = new QtSimpleXml();
    if(!xml->setContent(&f))
        addLogError(QByteArray("Xml parsing failed: ") + xml->errorString().toLatin1());

    return xml;
}

/*
    Resolves includeFilePath against currentFilePath. If currentFilePath
    contains foo/bar.xml, and includeFilePath contains bar2.xml, the returned
    result will be foo/bar2.xml. If includeFilePath is absolute, it is returned
    unmodified.
*/
QString PortingRules::resolveFileName(const QString currentFilePath,
                                      const QString includeFilePath) const
{
    if(QFileInfo(includeFilePath).isAbsolute())
        return includeFilePath;
    QString relativeDirectory = QFileInfo(currentFilePath).dir().dirName();
    QString testFileName = relativeDirectory + "/" + includeFilePath;
    if (QFile::exists(testFileName))
        return testFileName;

    return QString();
}
/*
    Checks if a rule is a replacement rule.
*/
bool PortingRules::isReplacementRule(const QString ruleType) const
{
    return (ruleType == "RenamedHeader" || ruleType == "RenamedClass" ||
            ruleType == "RenamedToken" || ruleType == "RenamedEnumvalue" ||
            ruleType == "RenamedType" || ruleType == "RenamedQtSymbol" );
}

/*
    Disables a replacement rule given by the replacementRule parameter
*/
void PortingRules::disableRule(QtSimpleXml &replacementRule)
{
    RuleDescription ruleDescription(replacementRule);
    disabledRules.append(ruleDescription);
}

/*
    Checks if a replacement rule is disabled or not
*/
bool PortingRules::isRuleDisabled(QtSimpleXml &replacementRule) const
{
    RuleDescription ruleDescription(replacementRule);
    return disabledRules.contains(ruleDescription);
}

/*
    Adds a warning to the global logger.
*/
void PortingRules::addLogWarning(const QString text) const
{
    Logger::instance()->addEntry(new PlainLogEntry("Warning", "Porting", text));
}

/*
    Adds an error to the global logger.
*/
void PortingRules::addLogError(const QString text) const
{
    Logger::instance()->addEntry(new PlainLogEntry("Error", "Porting", text));
}
