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

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "treewalker.h"

class QTextStream;
class Driver;
class Uic;

struct Option;

struct Validator : public TreeWalker
{
    Validator(Uic *uic);

    void accept(DomUI *node);
    void accept(DomWidget *node);

private:
    Driver *driver;
    QTextStream &output;
    const Option &option;
};

#endif // VALIDATOR_H
