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

#ifndef WRITEICONDECLARATION_H
#define WRITEICONDECLARATION_H

#include "treewalker.h"

class QTextStream;
class Driver;
class Uic;

struct Option;

class WriteIconDeclaration : public TreeWalker
{
public:
    WriteIconDeclaration(Uic *uic);

    void accept(DomUI *node);
    void accept(DomImages *images);
    void accept(DomImage *image);

private:
    Uic *uic;
    Driver *driver;
    QTextStream &output;
    const Option &option;
};

#endif // WRITEICONDECLARATION_H
