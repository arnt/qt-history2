/****************************************************************************
** $Id: //depot/qt/main/examples/table/table.h#2 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef TABLE_H
#define TABLE_H

#include <qscrollview.h>
#include <qpixmap.h>
#include <qvector.h>

class QHeader;
class QLineEdit;

class TableItem
{
public:
    TableItem( const QString &t, const QPixmap p )
	: txt( t ), pix( p ) {}

    QPixmap pixmap() const { return pix; }
    QString text() const { return txt; }
    void setPixmap( const QPixmap &p ) { pix = p; }
    void setText( const QString &t ) { txt = t; }
    
private:
    QString txt;
    QPixmap pix;

};

class Table : public QScrollView
{
    Q_OBJECT
public:
    Table( int numRows, int numCols, QWidget* parent=0, const char* name=0 );
    ~Table();

    TableItem *cellContent( int row, int col ) const;
    void setCellContent( int row, int col, TableItem *item );
    void setCellText( int row, int col, const QString &text );
    void setCellPixmap( int row, int col, const QPixmap &pix );
    QString cellText( int row, int col ) const;
    QPixmap cellPixmap( int row, int col ) const;

    QRect cellGeometry( int row, int col ) const;
    int columnWidth( int col ) const;
    int rowHeight( int row ) const;
    int columnPos( int col ) const;
    int rowPos( int row ) const;
    int columnAt( int pos ) const;
    int rowAt( int pos ) const;
    QSize tableSize() const;

    int rows() const;
    int cols() const;

    void updateCell( int row, int col );

protected:
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    void contentsMousePressEvent( QMouseEvent* );
    void contentsMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void focusInEvent( QFocusEvent* );
    void focusOutEvent( QFocusEvent* );
    void resizeEvent( QResizeEvent * );
    void showEvent( QShowEvent *e );

    virtual void paintEmptyArea( QPainter *p, int cx, int cy, int cw, int ch );
    bool focusNextPrevChild( bool next );

protected slots:
    virtual void columnWidthChanged( int col, int os, int ns );
    virtual void rowHeightChanged( int col, int os, int ns );

private slots:
    void editorOk();
    void editorCancel();

private:
    void paintCell( QPainter *p, int row, int col, const QRect &cr );
    int indexOf( int row, int col ) const;
    void updateGeometries();

private:
    QVector<TableItem> contents;
    int curRow;
    int curCol;
    QHeader *leftHeader, *topHeader;
    QLineEdit *editor;

};

#endif // TABLE_H
