/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qwellarray.h#12 $
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
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
public:
    QWellArray( QWidget *parent=0, const char *name=0, bool popup = FALSE );

    ~QWellArray() {}
    QString cellContent( int row, int col ) const;
    // ### Paul !!! virtual void setCellContent( int row, int col, const QString &);

    int numCols() { return nCols; }
    int numRows() { return nRows; }

    QSize sizeHint() const;

    virtual void setDimension( int rows, int cols );
    virtual void setCellBrush( int row, int col, const QBrush & );

protected:
    virtual void setSelected( int row, int col );
    virtual void setCurrent( int row, int col );

    void drawContents( QPainter *, int row, int col, const QRect& );
    void drawContents( QPainter * );

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
