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

#ifndef WRITEINCLUDES_H
#define WRITEINCLUDES_H

#include "treewalker.h"
#include <qhash.h>

class QTextStream;
class Driver;

struct Option;

struct WriteIncludes : public TreeWalker
{
    WriteIncludes(Driver *driver);

    void accept(DomUI *node);
    void accept(DomWidget *node);
    void accept(DomLayout *node);
    void accept(DomSpacer *node);

//
// custom widgets
//
    void accept(DomCustomWidgets *node);
    void accept(DomCustomWidget *node);

//
// include hints
//
    void accept(DomIncludeHints *node);

private:
    void add(const QString &className);

private:
    Driver *driver;
    QTextStream &output;
    const Option &option;

    QHash<QString, bool> m_includes;
    QHash<QString, bool> m_includeHints;
};

#endif // WRITEINCLUDES_H
