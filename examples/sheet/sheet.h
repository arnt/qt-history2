/****************************************************************************
** $Id: //depot/qt/main/examples/sheet/sheet.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SHEET_H
#define SHEET_H
#include "table.h"
#include "parser.h"


class PieView;

class Sheet : public QWidget
{
    Q_OBJECT
public:
    Sheet( QWidget *parent=0, const char *name=0 );
    ~Sheet();
    int tWidth() { return tableView->tWidth() + extraW; }
    int tHeight() { return tableView->tHeight() + extraH; }

protected slots:
    void exportText( int row, int col );
    void importText( int row, int col, const QString &);
    void setHorzBar(int);
    void setVertBar(int);
    void showPie();
    void hidePie();
protected:
    virtual void resizeEvent( QResizeEvent *);

private:
    MyTableView * tableView;
    MyTableLabel * head;
    MyTableLabel * side;

    QScrollBar * horz;
    QScrollBar * vert;

    int extraW;
    int extraH;
    ParsedArray *table;
    PieView *pie;

};



#endif
