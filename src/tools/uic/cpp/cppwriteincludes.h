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

#ifndef CPPWRITEINCLUDES_H
#define CPPWRITEINCLUDES_H

#include "treewalker.h"

#include <QMap>
#include <QSet>
#include <QString>

class QTextStream;
class Driver;
class Uic;

namespace CPP {

struct WriteIncludes : public TreeWalker
{
    WriteIncludes(Uic *uic);

    void acceptUI(DomUI *node);
    void acceptWidget(DomWidget *node);
    void acceptLayout(DomLayout *node);
    void acceptSpacer(DomSpacer *node);
    void acceptScripts(const DomScripts &, DomWidget *, const DomWidgets &);

//
// custom widgets
//
    void acceptCustomWidgets(DomCustomWidgets *node);
    void acceptCustomWidget(DomCustomWidget *node);

//
// include hints
//
    void acceptIncludes(DomIncludes *node);
    void acceptInclude(DomInclude *node);

    bool scriptsActivated() const { return m_scriptsActivated; }

private:
    void add(const QString &className);

private:
    typedef QMap<QString, bool> OrderedSet;
    void insertIncludeForClass(const QString &className);
    void insertInclude(const QString &header, bool global);
    void writeHeaders(const OrderedSet &headers, bool global);
    QString headerForClassName(const QString &className) const;

    const Uic *m_uic;
    QTextStream &m_output;

    OrderedSet m_localIncludes;
    OrderedSet m_globalIncludes;
    QSet<QString> m_includeBaseNames;

    QSet<QString> m_knownClasses;

    typedef QMap<QString, QString> StringMap;
    StringMap m_classToHeader;
    StringMap m_oldHeaderToNewHeader;
    bool m_scriptsActivated;
};

} // namespace CPP

#endif // CPPWRITEINCLUDES_H
