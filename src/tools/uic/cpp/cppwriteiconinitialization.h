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

#ifndef CPPWRITEICONINITIALIZATION_H
#define CPPWRITEICONINITIALIZATION_H

#include "treewalker.h"

QT_BEGIN_NAMESPACE

class QTextStream;
class Driver;
class Uic;

struct Option;

namespace CPP {

class WriteIconInitialization : public TreeWalker
{
public:
    WriteIconInitialization(Uic *uic);

    void acceptUI(DomUI *node);
    void acceptImages(DomImages *images);
    void acceptImage(DomImage *image);

private:
    Uic *uic;
    Driver *driver;
    QTextStream &output;
    const Option &option;
};

} // namespace CPP

QT_END_NAMESPACE

#endif // CPPWRITEICONINITIALIZATION_H
