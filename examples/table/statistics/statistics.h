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

#ifndef STATISTICS_H
#define STATISTICS_H

#include <qtable.h>
#include <qcombobox.h>

class TableItem : public QTableItem
{
public:
    TableItem( QTable *t, EditType et, const QString &txt ) : QTableItem( t, et, txt ) {}
    void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
};

class ComboItem : public QTableItem
{
public:
    ComboItem( QTable *t, EditType et );
    QWidget *createEditor() const;
    void setContentFromEditor( QWidget *w );
    void setText( const QString &s );
    
private:
    QComboBox *cb;

};

class Table : public QTable
{
    Q_OBJECT

public:
    Table();
    void sortColumn( int col, bool ascending, bool wholeRows );

private slots:
    void recalcSum( int row, int col );

private:
    void initTable();

};

#endif
