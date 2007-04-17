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

#include "cppwritedeclaration.h"
#include "cppwriteicondeclaration.h"
#include "cppwriteinitialization.h"
#include "cppwriteiconinitialization.h"
#include "driver.h"
#include "extractimages.h"
#include "ui4.h"
#include "uic.h"
#include "databaseinfo.h"
#include "customwidgetsinfo.h"

#include <QTextStream>

namespace {
    void openNameSpaces(const QStringList &namespaceList, QTextStream &output) {
        if (namespaceList.empty())
            return;
        const QStringList::const_iterator cend = namespaceList.constEnd();
        for (QStringList::const_iterator it = namespaceList.constBegin(); it != cend; ++it) {
            if (!it->isEmpty()) {
                output << "namespace " << *it << " {\n";
            }
        }
    }

    void closeNameSpaces(const QStringList &namespaceList, QTextStream &output) {
        if (namespaceList.empty())
            return;

        QListIterator<QString> it(namespaceList);
        it.toBack();
        while (it.hasPrevious()) {
            const QString ns = it.previous();
            if (!ns.isEmpty()) {
                output << "} // namespace " << ns << "\n";
            }
        }
    }

    void writeScriptContextClass(const QString &indent, QTextStream &str) {
         str << indent << "class ScriptContext\n"
             << indent << "{\n"
             << indent << "public:\n"
             << indent << "    void run(const QString &script, QWidget *widget, const QWidgetList &childWidgets)\n"
             << indent << "    {\n"
             << indent << "        QScriptValue widgetObject =  scriptEngine.newQObject(widget);\n"
             << indent << "        QScriptValue childWidgetArray = scriptEngine.newArray (childWidgets.size());\n"
             << indent << "        for (int i = 0; i < childWidgets.size(); i++)\n"
             << indent << "               childWidgetArray.setProperty(i, scriptEngine.newQObject(childWidgets[i]));\n"
             << indent << "        scriptEngine.globalObject().setProperty(\"widget\", widgetObject);\n"
             << indent << "        scriptEngine.globalObject().setProperty(\"childWidgets\", childWidgetArray);\n\n"
             << indent << "        if (!scriptEngine.canEvaluate(script)) {\n"
             << indent << "            qWarning() << \"Cannot evaluate script for \" << widget->objectName() << \": \" << script;\n"
             << indent << "            return;\n"
             << indent << "        }\n\n"
             << indent << "        scriptEngine.evaluate(script);\n"
             << indent << "        if (scriptEngine.hasUncaughtException ()) {\n"
             << indent << "            qWarning() << \"An exception has occurred at line \" << scriptEngine.uncaughtExceptionLineNumber()\n"
             << indent << "                       << \" of the script for \" << widget->objectName() << \": \" << engineError() << '\\n'\n"
             << indent << "                       << script;\n"
             << indent << "        }\n\n"
             << indent << "    }\n\n"
             << indent << "private:\n"
             << indent << "    QString engineError()\n"
             << indent << "    {\n"
             << indent << "        QScriptValue error = scriptEngine.evaluate(\"Error\");\n"
             << indent << "        return error.toString();\n"
             << indent << "    }\n\n"
             << indent << "    QScriptEngine scriptEngine;\n"
             << indent << "};\n\n";
    }
}

namespace CPP {

WriteDeclaration::WriteDeclaration(Uic *uic, bool activateScripts)  :
    m_uic(uic),
    m_driver(uic->driver()),
    m_output(uic->output()),
    m_option(uic->option()),
    m_activateScripts(activateScripts)
{
}

void WriteDeclaration::acceptUI(DomUI *node)
{
    QString qualifiedClassName = node->elementClass() + m_option.postfix;
    QString className = qualifiedClassName;

    QString varName = m_driver->findOrInsertWidget(node->elementWidget());
    QString widgetClassName = node->elementWidget()->attributeClass();

    QString exportMacro = node->elementExportMacro();
    if (!exportMacro.isEmpty())
        exportMacro.append(QLatin1Char(' '));

    QStringList namespaceList = qualifiedClassName.split(QLatin1String("::"));
    if (namespaceList.count()) {
        className = namespaceList.last();
        namespaceList.removeLast();
    }

    openNameSpaces(namespaceList, m_output);

    if (namespaceList.count())
        m_output << "\n";

    m_output << "class " << exportMacro << m_option.prefix << className << "\n"
           << "{\n"
           << "public:\n";

    const QStringList connections = m_uic->databaseInfo()->connections();
    for (int i=0; i<connections.size(); ++i) {
        const QString connection = connections.at(i);

        if (connection == QLatin1String("(default)"))
            continue;

        m_output << m_option.indent << "QSqlDatabase " << connection << "Connection;\n";
    }

    TreeWalker::acceptWidget(node->elementWidget());

    m_output << "\n";

    WriteInitialization(m_uic, m_activateScripts).acceptUI(node);

    if (node->elementImages()) {
        if (m_option.extractImages) {
            ExtractImages(m_uic->option()).acceptUI(node);
        } else {
            m_output << "\n"
                << "protected:\n"
                << m_option.indent << "enum IconID\n"
                << m_option.indent << "{\n";
            WriteIconDeclaration(m_uic).acceptUI(node);

            m_output << m_option.indent << m_option.indent << "unknown_ID\n"
                << m_option.indent << "};\n";

            WriteIconInitialization(m_uic).acceptUI(node);
        }
    }

    if (m_activateScripts) {
        m_output << "\nprivate:\n\n";
        writeScriptContextClass(m_option.indent, m_output);
    }

    m_output << "};\n\n";

    closeNameSpaces(namespaceList, m_output);

    if (namespaceList.count())
        m_output << "\n";

    if (m_option.generateNamespace && !m_option.prefix.isEmpty()) {
        namespaceList.append(QLatin1String("Ui"));

        openNameSpaces(namespaceList, m_output);

        m_output << m_option.indent << "class " << exportMacro << className << ": public " << m_option.prefix << className << " {};\n";

        closeNameSpaces(namespaceList, m_output);

        if (namespaceList.count())
            m_output << "\n";
    }
}

void WriteDeclaration::acceptWidget(DomWidget *node)
{
    QString className = QLatin1String("QWidget");
    if (node->hasAttributeClass())
        className = node->attributeClass();

    m_output << m_option.indent << m_uic->customWidgetsInfo()->realClassName(className) << " *" << m_driver->findOrInsertWidget(node) << ";\n";

    TreeWalker::acceptWidget(node);
}

void WriteDeclaration::acceptLayout(DomLayout *node)
{
    QString className = QLatin1String("QLayout");
    if (node->hasAttributeClass())
        className = node->attributeClass();

    m_output << m_option.indent << className << " *" << m_driver->findOrInsertLayout(node) << ";\n";

    TreeWalker::acceptLayout(node);
}

void WriteDeclaration::acceptActionGroup(DomActionGroup *node)
{
    m_output << m_option.indent << "QActionGroup *" << m_driver->findOrInsertActionGroup(node) << ";\n";

    TreeWalker::acceptActionGroup(node);
}

void WriteDeclaration::acceptAction(DomAction *node)
{
    m_output << m_option.indent << "QAction *" << m_driver->findOrInsertAction(node) << ";\n";

    TreeWalker::acceptAction(node);
}

} // namespace CPP
