/****************************************************************************
** $Id: //depot/qt/main/examples/sheet/table.h#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef TABLE_H
#define TABLE_H
#include <qtableview.h>
#include <qlineedit.h>

class MyTableLabel : public QTableView
{
    Q_OBJECT
public:
    MyTableLabel( char base, const char * str,
		  QWidget *parent=0, const char *name=0 );
    void setNumCols( int n ) { QTableView::setNumCols(n); }
    void setNumRows( int n ) { QTableView::setNumRows(n); }

    int tWidth() { return totalWidth(); }
    int tHeight() { return totalHeight(); }

public slots:
    void setStart( int n ) { start = n; repaint(); }
protected:
    virtual void paintCell( QPainter *p, int row, int col );
private:
    int start;
    char base;
    QString text;
    friend class Sheet; //###
};


class MyTableView : public QTableView
{
    Q_OBJECT
public:
    //MyTableView( QWidget *parent=0, const char *name=0 );
    MyTableView( int rows, int cols, QWidget *parent=0, int flags = -1,
		 const char *name=0 );
    ~MyTableView();
    int tWidth() { return totalWidth() + extraW; }
    int tHeight() { return totalHeight() + extraH; }

    int numColsVisible() { return lastColVisible() - leftCell() + 1; }
    int numRowsVisible() { return lastRowVisible() - topCell() + 1; }
    void setText( int row, int col, QString, bool paint = TRUE );

public slots:
    void showText( int row, int col, QString s) { setText( row, col, s); }
    void nextInput();
    void moveInput( int row, int col );
    void makeVisible( int row, int col );

signals:
    void selected( int row, int col );
    void newText( int row, int col, const QString &);
    void newRow(int);
    void newCol(int);
    void recalc();
    //void recalc( int row, int col );

protected:
    virtual void paintCell( QPainter *p, int row, int col );
    virtual void mousePressEvent( QMouseEvent * );
protected slots:
    void scrollHorz(int);
    void scrollVert(int);
    void setInputText(QString);

private:
    int extraW;
    int extraH;
    QString (*table);

    int index( int r, int c ) { return c+r*numCols(); }

    QLineEdit *input;
    int inRow;
    int inCol;
    void placeInput();

    friend class Sheet; //###
};

#endif
