//
// Qt example: Table
//
// A simple, spreadsheet-like widget, made by inheriting  QTableView.
// 
// File: table.h
//
// Definition of the Table widget.
//

#ifndef TABLE_H
#define TABLE_H

#include <qtableview.h>

class QHeader;

class Table : public QTableView
{
    Q_OBJECT
public:
    Table( int numRows, int numCols, QWidget* parent=0, const char* name=0 );
    ~Table();
    
    const char* cellContent( int row, int col ) const;
    void setCellContent( int row, int col, const char* );

    void setHeader( QHeader * );

public slots:
    void update();

protected:
    void paintCell( QPainter*, int row, int col );
    void mousePressEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void focusInEvent( QFocusEvent* );
    void focusOutEvent( QFocusEvent* );

    int cellWidth( int);
    
private:
    int indexOf( int row, int col ) const;
    QString* contents;
    QHeader *head;
    int curRow;
    int curCol;
};

#endif // TABLE_H
