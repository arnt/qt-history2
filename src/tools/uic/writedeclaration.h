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

#ifndef WRITEDECLARATION_H
#define WRITEDECLARATION_H

#include "treewalker.h"

class QTextStream;
class Driver;
class Uic;

struct Option;

struct WriteDeclaration : public TreeWalker
{
    WriteDeclaration(Uic *uic);

    void accept(DomUI *node);
    void accept(DomWidget *node);
    void accept(DomLayout *node);
    void accept(DomSpacer *node);
    void accept(DomActionGroup *node);
    void accept(DomAction *node);

private:
    Uic *uic;
    Driver *driver;
    QTextStream &output;
    const Option &option;
};


#endif // WRITEDECLARATION_H
