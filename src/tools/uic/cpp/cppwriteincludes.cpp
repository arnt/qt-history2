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

#include "cppwriteincludes.h"
#include "driver.h"
#include "ui4.h"
#include "uic.h"
#include "databaseinfo.h"
#include <qdebug.h>
#include <QFileInfo>
#include <QTextStream>

namespace {
    const bool debugWriteIncludes=false;
}

namespace CPP {

struct ClassInfoEntry
{
    const char *klass;
    const char *module;
    const char *header;
};

static ClassInfoEntry qclass_lib_map[] = {
#define QT_CLASS_LIB(klass, module, header) { #klass, #module, #header },
#include "qclass_lib_map.h"
#undef QT_CLASS_LIB
    { 0, 0, 0 }
};

WriteIncludes::WriteIncludes(Uic *uic)    :
    m_uic(uic),
    m_output(uic->output())
{
    for(const ClassInfoEntry *it = &qclass_lib_map[0]; it->klass != 0;  ++it) {
        QString newHeader = QLatin1String(it->module);
        newHeader += QLatin1Char('/');
        newHeader += QLatin1String(it->klass);
        m_classToHeader.insert(QLatin1String(it->klass),         newHeader);
        m_oldHeaderToNewHeader.insert(QLatin1String(it->header), newHeader);
    }
}

void WriteIncludes::acceptUI(DomUI *node)
{
    m_localIncludes.clear();
    m_globalIncludes.clear();
    m_knownClasses.clear();
    m_includeBaseNames.clear();

    if (node->elementIncludes())
        acceptIncludes(node->elementIncludes());

    if (node->elementCustomWidgets())
        TreeWalker::acceptCustomWidgets(node->elementCustomWidgets());

    add(QLatin1String("QApplication"));
    add(QLatin1String("QVariant"));
    add(QLatin1String("QAction"));

    add(QLatin1String("QButtonGroup")); // ### only if it is really necessary

    if (m_uic->hasExternalPixmap() && m_uic->pixmapFunction() == QLatin1String("qPixmapFromMimeSource"))
        add(QLatin1String("Q3MimeSourceFactory"));

    if (m_uic->databaseInfo()->connections().size()) {
        add(QLatin1String("QSqlDatabase"));
        add(QLatin1String("Q3SqlCursor"));
        add(QLatin1String("QSqlRecord"));
        add(QLatin1String("Q3SqlForm"));
    }

    TreeWalker::acceptUI(node);

    writeHeaders(m_globalIncludes, true);
    writeHeaders(m_localIncludes, false);

    m_output << QLatin1Char('\n');
}

void WriteIncludes::acceptWidget(DomWidget *node)
{
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

void WriteIncludes::insertIncludeForClass(const QString &className)
{
    // Known class
    const StringMap::const_iterator it = m_classToHeader.constFind(className);
    if ( it !=  m_classToHeader.constEnd()) {
        insertInclude(it.value(), true);
        return;
    }

    // Quick check by class name to detect includehints provided for custom widgets
    const QString lowerClassName = className.toLower();
    if (m_includeBaseNames.contains(lowerClassName)) {
        if (debugWriteIncludes)
            qDebug() << "WriteIncludes::insertIncludeForClass: class name match " << lowerClassName << '.';
        return;
    }
    // Create default header
    QString  header = lowerClassName;
    header += QLatin1String(".h");
    if (debugWriteIncludes)
        qDebug() << "WriteIncludes::insertIncludeForClass: creating default header " << header << " for " << className << '.';
    insertInclude(header, true);
}

void WriteIncludes::add(const QString &className)
{
    if (className.isEmpty() || m_knownClasses.contains(className))
        return;

    m_knownClasses.insert(className);
    if (className == QLatin1String("Line")) { // ### hmm, deprecate me!
        add(QLatin1String("QFrame"));
        return;
    }

    insertIncludeForClass(className);
}

void WriteIncludes::acceptCustomWidget(DomCustomWidget *node)
{
    const QString className = node->elementClass();
    if (className.isEmpty())
        return;

    if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3ListView"))  ||
        m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3Table"))) {
        add(QLatin1String("Q3Header"));
    }

    if (node->elementHeader() && node->elementHeader()->text().size()) {
        bool global = node->elementHeader()->attributeLocation().toLower() == QLatin1String("global");
        QString header = node->elementHeader()->text();
        const QString qtHeader = m_classToHeader.value(className); // check if the class is a built-in qt class
        if (!qtHeader.isEmpty()) {
            global = true;
            header = qtHeader;
        }
        insertInclude(header, global);
    } else {
        insertIncludeForClass(className);
    }
    m_knownClasses.insert(className);
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
    insertInclude(node->text(), global);
}
void WriteIncludes::insertInclude(const QString &header, bool global)
{
    OrderedSet &includes(global ?  m_globalIncludes : m_localIncludes);
    if (includes.contains(header))
        return;
    // Insert. Also remember base name for quick check of suspicious custom plugins
    includes.insert(header, false);
    const QString lowerBaseName = QFileInfo(header).completeBaseName ().toLower();
    m_includeBaseNames.insert(lowerBaseName);
}

void WriteIncludes::writeHeaders(const OrderedSet &headers, bool global)
{
    const QChar openingQuote = global ? QLatin1Char('<') : QLatin1Char('"');
    const QChar closingQuote = global ? QLatin1Char('>') : QLatin1Char('"');

    const OrderedSet::const_iterator cend = headers.constEnd();
    for ( OrderedSet::const_iterator sit = headers.constBegin(); sit != cend; ++sit) {
        const StringMap::const_iterator hit = m_oldHeaderToNewHeader.constFind(sit.key());
        const bool mapped =  hit != m_oldHeaderToNewHeader.constEnd();
        const  QString header =  mapped ? hit.value() : sit.key();
        if (!header.trimmed().isEmpty()) {
            m_output << "#include " << openingQuote << header << closingQuote << QLatin1Char('\n');
        }
    }
}
} // namespace CPP
