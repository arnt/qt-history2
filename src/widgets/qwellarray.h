/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qwellarray.h#3 $
**
** Definition of QWellArray widget class
**
** Created : 980114
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QWELLARRAY_H
#define QWELLARRAY_H

#ifndef QT_H
#include "qtablevw.h"
#endif // QT_H

struct QWellArrayData;

class QWellArray : public QTableView
{
	Q_OBJECT
public:
    QWellArray( QWidget *parent=0, const char *name=0, bool popup = FALSE );

    ~QWellArray() {}
    const char* cellContent( int row, int col ) const;
    void setCellContent( int row, int col, const char* );

    int numCols() { return nCols; }
    int numRows() { return nRows; }

    QSize sizeHint() const;

    virtual void setDimension( int rows, int cols );
    void setCellBrush( int row, int col, const QBrush & );

protected:
    void setSelected( int row, int col );
    void setCurrent( int row, int col );

    void drawContents( QPainter *, int row, int col, const QRect& );

    void paintCell( QPainter*, int row, int col );
    void mousePressEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void focusInEvent( QFocusEvent* );
    void focusOutEvent( QFocusEvent* );

private:
    int curRow;
    int curCol;
    int selRow;
    int selCol;
    int nCols;
    int nRows;
    bool smallStyle;
    QWellArrayData *d;
};

#endif
