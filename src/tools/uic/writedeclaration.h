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

#ifndef WRITEDECLARATION_H
#define WRITEDECLARATION_H

#include "treewalker.h"

class QTextStream;
class Driver;

struct Option;

struct WriteDeclaration : public TreeWalker
{
    WriteDeclaration(Driver *driver);

    void accept(DomUI *node);
    void accept(DomWidget *node);
    void accept(DomLayout *node);
    void accept(DomSpacer *node);

private:
    Driver *driver;
    QTextStream &output;
    const Option &option;
};


#endif // WRITEDECLARATION_H
