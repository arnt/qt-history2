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

#ifndef CONNECTIONTABLE_H
#define CONNECTIONTABLE_H

#include <qtable.h>

class ConnectionTable : public QTable
{
    Q_OBJECT

public:
    ConnectionTable( QWidget *parent, const char *name );

    void sortColumn( int col, bool ascending, bool wholeRows );

signals:
    void resorted();

};

#endif
