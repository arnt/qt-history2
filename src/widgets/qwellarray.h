/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qwellarray.h#15 $
**
** Definition of QWellArray widget class
**
** Created : 980114
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWELLARRAY_H
#define QWELLARRAY_H

#ifndef QT_H
#include "qtableview.h"
#endif // QT_H

struct QWellArrayData;

class Q_EXPORT QWellArray : public QTableView
{
    Q_OBJECT
    // #### Not const Q_PROPERTY( int, "numCols", numCols, 0 )
    // #### Not const Q_PROPERTY( int, "numRows", numRows, 0 )
    // #### Not const Q_PROPERTY( int, "selectedColumn", selectedColumn, 0 )
    // #### Not const Q_PROPERTY( int, "selectedRow", selectedRow, 0 )
	
public:
    QWellArray( QWidget *parent=0, const char *name=0, bool popup = FALSE );

    ~QWellArray() {}
    QString cellContent( int row, int col ) const;
    // ### Paul !!! virtual void setCellContent( int row, int col, const QString &);

    int numCols() { return nCols; }
    int numRows() { return nRows; }

    int selectedColumn() { return selCol; }
    int selectedRow() { return selRow; }

    virtual void setSelected( int row, int col );

    void setCellSize( int w, int h ) { setCellWidth(w);setCellHeight( h ); }

    QSize sizeHint() const;

    virtual void setDimension( int rows, int cols );
    virtual void setCellBrush( int row, int col, const QBrush & );
    QBrush cellBrush( int row, int col );

signals:
    void selected( int row, int col );

protected:
    virtual void setCurrent( int row, int col );

    virtual void drawContents( QPainter *, int row, int col, const QRect& );
    void drawContents( QPainter * );

    void paintCell( QPainter*, int row, int col );
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
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
