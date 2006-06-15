/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FINDSOURCESVISITOR_H
#define FINDSOURCESVISITOR_H
#include "proitems.h"
#include "abstractproitemvisitor.h"
#include <QtCore/QIODevice>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QStack>

class ProFile;

class FindSourcesVisitor : public AbstractProItemVisitor {
public:
    typedef enum {
        TT_Unknown = 0,
        TT_Application,
        TT_Library,
        TT_Subdirs
    } TemplateType;

    typedef enum {
        MT_Error            = 1,    // Catastrophic error, processing stops
        MT_DebugLevel1,             // serious errors
        MT_DebugLevel2,             // default
        MT_DebugLevel3,             // less serious
        MT_ProMessage       = 100,  // output of the profiles' message(string).
        MT_ProError,                // output of error(string). In this implementation, processing will not stop.
    } MessageType;

    FindSourcesVisitor();
    ~FindSourcesVisitor();

    bool visitBeginProBlock(ProBlock * /*block*/)
    {
        // ignore
        return true;
    }
    bool visitEndProBlock(ProBlock * /*block*/)
    {
        // ignore
        return true;
    }

    bool visitBeginProVariable(ProVariable *variable)
    {
        m_lastVarName = variable->variable();
        m_variableOperator = variable->variableOperator();
        return true;
    }
    bool visitEndProVariable(ProVariable * /*variable*/)
    {
        m_lastVarName.clear();
        return true;
    }

    bool visitBeginProFile(ProFile * value);
    bool visitEndProFile(ProFile * value);
    bool visitProValue(ProValue *value);

    bool visitProFunction(ProFunction *function);
    bool visitProOperator(ProOperator * /*oper*/)
    {
        // ignore
        return true;
    }
    bool visitProCondition(ProCondition * /*cond*/)
    {
        // ignore
        return true;
    }

    FindSourcesVisitor::TemplateType getTemplateType();
    QStringList getVariable(const QString &variableName) const;
    QStringList expandVariableToAbsoluteFileNames(const QString &variableName, const QString &originProfile);

protected:
    virtual void logMessage(const QString &message, MessageType messagetype = MT_DebugLevel2);
private:
    void logMessage(MessageType mt, const char *msg, ...);
    QString expandVariableReferences(const QString &value);
    QString evaluateExpandFunction(const QByteArray &func, const QString &arguments);
    bool evaluateConditionalFunction(const QByteArray &function, const QString &arguments, bool *result);
    bool evaluateFile(const QString &fileName, bool enableBackSlashFixing, bool readFeatures,
                                    bool mustexist, bool *result);
    bool evaluateFeatureFile(const QString &fileName, bool *result);

    QString currentFileName() const;
    QString getcwd() const;
    ProFile *currentProFile() const;
    QString locationSpecifier() const;              // error reporting

    QStringList qmake_feature_paths();
    QByteArray m_lastVarName;
    ProVariable::VariableOperator m_variableOperator;

    QMap<QByteArray, QStringList> m_valuemap;       // VariableName must be us-ascii, the content however can be non-us-ascii.
    QStack<ProFile*> m_profileStack;                // To handle 'include(a.pri), so we can track back to 'a.pro' when finished with 'a.pri'
    int m_lineNo;                                   // Error reporting
    QString m_oldPath;                              // To restore the current path to the path

}; //class FindSourcesVisitor

#endif // FINDSOURCESVISITOR_H

