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

#include "writeincludes.h"
#include "driver.h"
#include "ui4.h"
#include "uic.h"
#include "databaseinfo.h"

#include <qtextstream.h>

WriteIncludes::WriteIncludes(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option())
{
    this->uic = uic;
}

void WriteIncludes::acceptUI(DomUI *node)
{
    m_includes.clear();
    m_customWidgets.clear();

    if (node->elementIncludes())
        acceptIncludes(node->elementIncludes());

    if (node->elementCustomWidgets())
        TreeWalker::acceptCustomWidgets(node->elementCustomWidgets());

    m_includes.insert(QLatin1String("qapplication.h"), true);
    m_includes.insert(QLatin1String("qvariant.h"), true);
    m_includes.insert(QLatin1String("qaction.h"), true);

    m_includes.insert(QLatin1String("qbuttongroup.h"), true);

    if (uic->hasExternalPixmap()
            && uic->pixmapFunction() == QLatin1String("qPixmapFromMimeSource"))
        m_includes.insert(QLatin1String("q3mimefactory.h"), true); //compat

    if (uic->databaseInfo()->connections().size()) {
        m_includes.insert(QLatin1String("qsqldatabase.h"), true);
        m_includes.insert(QLatin1String("qsqlcursor.h"), true);
        m_includes.insert(QLatin1String("qsqlrecord.h"), true);
        m_includes.insert(QLatin1String("qsqlform.h"), true);
    }

    TreeWalker::acceptUI(node);

    QHashIterator<QString, bool> it(m_includes);
    while (it.hasNext()) {
        it.next();

        if (it.value())
            output << "#include <" << it.key() << ">\n";
        else
            output << "#include \"" << it.key() << "\"\n";
    }
    output << "\n";
}

void WriteIncludes::acceptWidget(DomWidget *node)
{
    if (node->attributeClass() == QLatin1String("Line"))
        add(QLatin1String("QFrame"));
    else
        add(node->attributeClass());

    TreeWalker::acceptWidget(node);
}

void WriteIncludes::acceptLayout(DomLayout *node)
{
    add(node->attributeClass());
    TreeWalker::acceptLayout(node);
}

void WriteIncludes::acceptSpacer(DomSpacer *node)
{
    add(QLatin1String("QSpacerItem"));
    TreeWalker::acceptSpacer(node);
}

void WriteIncludes::add(const QString &className)
{
    if (className.isEmpty())
        return;

    QString header = className.toLower() + QLatin1String(".h");

    if (className == QLatin1String("QVBoxLayout")  // special case for layouts
            || className == QLatin1String("QHBoxLayout")
            || className == QLatin1String("QBoxLayout")
            || className == QLatin1String("QLayout")
            || className == QLatin1String("QGridLayout")
            || className == QLatin1String("QSpacerItem")) {
        m_includes.insert(QLatin1String("qlayout.h"), true);
    } else if (className == QLatin1String("QDoubleSpinBox")) {
        m_includes.insert(QLatin1String("qspinbox.h"), true);
    } else if (className == QLatin1String("Line")) {
        m_includes.insert(QLatin1String("qframe.h"), true);
    } else if (className == QLatin1String("QDateEdit") // special case for datetime
            || className == QLatin1String("QTimeEdit")) {
        m_includes.insert(QLatin1String("qdatetimeedit.h"), true);
    } else if (className == QLatin1String("Q3DateEdit") // special case for d3atetime
            || className == QLatin1String("Q3TimeEdit")) {
        m_includes.insert(QLatin1String("q3datetimeedit.h"), true);
    } else if (!m_includes.contains(header)
            && !m_customWidgets.contains(className)) {
        m_includes.insert(header, true);
    }

    if (uic->customWidgetsInfo()->extends(className, QLatin1String("Q3ListView"))
            || uic->customWidgetsInfo()->extends(className, QLatin1String("QTable"))) {
        m_includes.insert(QLatin1String("q3header.h"), true);
    }
}

void WriteIncludes::acceptCustomWidget(DomCustomWidget *node)
{
    if (node->elementClass().isEmpty())
        return;

    m_customWidgets.insert(node->elementClass(), true);

    bool global = true;
    if (node->elementHeader() && node->elementHeader()->text().size()) {
        global = node->elementHeader()->attributeLocation().toLower() == QLatin1String("global");
        m_includes.insert(node->elementHeader()->text(), global);
    } else {
        add(node->elementClass());
    }
    
}

void WriteIncludes::acceptCustomWidgets(DomCustomWidgets *node)
{
    Q_UNUSED(node);
}

void WriteIncludes::acceptIncludes(DomIncludes *node)
{
    TreeWalker::acceptIncludes(node);
}

void WriteIncludes::acceptInclude(DomInclude *node)
{
    bool global = true;
    if (node->hasAttributeLocation())
        global = node->attributeLocation() == QLatin1String("global");
    m_includes.insert(node->text(), global);
}
