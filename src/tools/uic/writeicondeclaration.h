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

#ifndef WRITEICONDECLARATION_H
#define WRITEICONDECLARATION_H

#include "treewalker.h"

class QTextStream;
class Driver;

struct Option;

class WriteIconDeclaration : public TreeWalker
{
public:
    WriteIconDeclaration(Driver *driver);

    void accept(DomUI *node);
    void accept(DomImages *images);
    void accept(DomImage *image);

private:
    Driver *driver;
    QTextStream &output;
    const Option &option;
};

#endif // WRITEICONDECLARATION_H
