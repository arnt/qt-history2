/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef OUTLINETREE_H
#define OUTLINETREE_H

#include <q3listview.h>
#include <qdom.h>

class OutlineTree : public Q3ListView
{
    Q_OBJECT

public:
    OutlineTree( const QString fileName, QWidget *parent = 0, const char *name = 0 );
    ~OutlineTree();

private:
    QDomDocument domTree;
    void getHeaderInformation( const QDomElement &header );
    void buildTree( Q3ListViewItem *parentItem, const QDomElement &parentElement );
};

#endif // OUTLINETREE_H
