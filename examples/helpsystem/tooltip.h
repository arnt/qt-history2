/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <qtooltip.h>


class QTable;
class QHeader;

class HeaderToolTip : public QToolTip
{
public:
    HeaderToolTip( QHeader *header, QToolTipGroup *group = 0 );
    
protected:
    void maybeTip ( const QPoint &p );
};

class TableToolTip : public QToolTip
{
public:
    TableToolTip( QTable* table, QToolTipGroup *group = 0  );
    
protected:
    void maybeTip( const QPoint &p );
    
private:
    QTable *table;
};


#endif
