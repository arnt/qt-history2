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

#ifndef QGRID_H
#define QGRID_H

#include <qgridwidget.h>

class Q_COMPAT_EXPORT QGrid : public QGridWidget
{
    Q_OBJECT
public:
    QGrid(int n, QWidget* parent=0, const char* name=0, Qt::WFlags f=0)
        : QGridWidget(n,parent,name,f) {}
    QGrid(int n, Qt::Orientation orientation, QWidget* parent=0, const char* name=0, Qt::WFlags f=0)
        : QGridWidget(n,orientation, parent,name,f) {}
};

#endif
