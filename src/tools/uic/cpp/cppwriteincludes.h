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

struct Option;

namespace CPP {

struct WriteIncludes : public TreeWalker
{
    WriteIncludes(Uic *uic);

    void acceptUI(DomUI *node);
    void acceptWidget(DomWidget *node);
    void acceptLayout(DomLayout *node);
    void acceptSpacer(DomSpacer *node);

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

private:
    void add(const QString &className);

private:
    const Uic *m_uic;
    QTextStream &m_output;

    typedef QMap<QString, bool> IncludeGlobalMap;
    IncludeGlobalMap m_includes;
    QSet<QString> m_customWidgets;
    QMap<QString, QString> m_classToHeader;
    QMap<QString, QString> m_oldHeaderToNewHeader;
};

} // namespace CPP

#endif // CPPWRITEINCLUDES_H
