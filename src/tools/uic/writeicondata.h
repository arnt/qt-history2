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

#ifndef WRITEICONDATA_H
#define WRITEICONDATA_H

#include "treewalker.h"

class QTextStream;
class Driver;
class Uic;

struct Option;

class WriteIconData : public TreeWalker
{
public:
    WriteIconData(Uic *uic);

    void acceptUI(DomUI *node);
    void acceptImages(DomImages *images);
    void acceptImage(DomImage *image);

private:
    Driver *driver;
    QTextStream &output;
    const Option &option;
};

#endif // WRITEICONDATA_H
