/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "writeincludes.h"
#include "driver.h"
#include "ui4.h"

#include <qtextstream.h>

WriteIncludes::WriteIncludes(Driver *drv)
    : driver(drv), output(drv->output()), option(drv->option())
{
}

void WriteIncludes::accept(DomUI *node)
{
    m_includes.clear();

    if (node->elementCustomWidgets())
        TreeWalker::accept(node->elementCustomWidgets());

    m_includes.insert("qapplication.h", true);
    m_includes.insert("qvariant.h", true);

    TreeWalker::accept(node);

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

void WriteIncludes::accept(DomWidget *node)
{
    if (node->attributeClass() == QLatin1String("Line"))
        add(QLatin1String("QFrame"));
    else
        add(node->attributeClass());

    TreeWalker::accept(node);
}

void WriteIncludes::accept(DomLayout *node)
{
    add(node->attributeClass());
    TreeWalker::accept(node);
}

void WriteIncludes::accept(DomSpacer *node)
{
    add(QLatin1String("QSpacerItem"));
    TreeWalker::accept(node);
}

void WriteIncludes::add(const QString &className)
{
    if (className.isEmpty())
        return;

    QString header = className.toLower() + ".h";
    // ### move in a simplified MetaDataBase

    if (className == QLatin1String("QVBoxLayout")  // special case for layouts
            || className == QLatin1String("QHBoxLayout")
            || className == QLatin1String("QBoxLayout")
            || className == QLatin1String("QLayout")
            || className == QLatin1String("QGridLayout")
            || className == QLatin1String("QSpacerItem")) {
        m_includes.insert("qlayout.h", true);
    } else if (className == QLatin1String("Line")) {
        m_includes.insert("qframe.h", true);
    } else if (className == QLatin1String("QDateEdit") // special case for datetime
            || className == QLatin1String("QTimeEdit")) {
        m_includes.insert("qdatetimeedit.h", true);
    } else if (!m_includes.contains(header)) {
        m_includes.insert(header, true);
    }
}

void WriteIncludes::accept(DomCustomWidget *node)
{
    if (node->elementClass().isEmpty())
        return;

    bool global = node->elementHeader()->attributeLocation().toLower() == QLatin1String("global");
    m_includes.insert(node->elementHeader()->text(), global);
}

void WriteIncludes::accept(DomCustomWidgets *node)
{
    Q_UNUSED(node);
}
