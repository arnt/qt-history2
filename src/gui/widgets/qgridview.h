/****************************************************************************
**
** Definition of QGridView class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGRIDVIEW_H
#define QGRIDVIEW_H

#ifndef QT_H
#include "qscrollview.h"
#endif // QT_H

#ifndef QT_NO_GRIDVIEW

class QGridViewPrivate;

class Q_GUI_EXPORT QGridView : public QScrollView
{
    Q_OBJECT
    Q_PROPERTY(int numRows READ numRows WRITE setNumRows)
    Q_PROPERTY(int numCols READ numCols WRITE setNumCols)
    Q_PROPERTY(int cellWidth READ cellWidth WRITE setCellWidth)
    Q_PROPERTY(int cellHeight READ cellHeight WRITE setCellHeight)
public:

    QGridView(QWidget *parent=0, const char *name=0, WFlags f=0);
   ~QGridView();

    int numRows() const;
    virtual void setNumRows(int);
    int numCols() const;
    virtual void setNumCols(int);

    int cellWidth() const;
    virtual void setCellWidth(int);
    int cellHeight() const;
    virtual void setCellHeight(int);

    QRect cellRect() const;
    QRect cellGeometry(int row, int column);
    QSize gridSize() const;

    int rowAt(int y) const;
    int columnAt(int x) const;

    void repaintCell(int row, int column, bool erase=true);
    void updateCell(int row, int column);
    void ensureCellVisible(int row, int column);

protected:
    virtual void paintCell(QPainter *, int row, int col) = 0;
    virtual void paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch);

    void drawContents(QPainter *p, int cx, int cy, int cw, int ch);

    virtual void dimensionChange(int, int);

private:
    void drawContents(QPainter*);
    void updateGrid();

    int nrows;
    int ncols;
    int cellw;
    int cellh;
    QGridViewPrivate* d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGridView(const QGridView &);
    QGridView &operator=(const QGridView &);
#endif
};

inline int QGridView::cellWidth() const
{ return cellw; }

inline int QGridView::cellHeight() const
{ return cellh; }

inline int QGridView::rowAt(int y) const
{ return y / cellh; }

inline int QGridView::columnAt(int x) const
{ return x / cellw; }

inline int QGridView::numRows() const
{ return nrows; }

inline int QGridView::numCols() const
{return ncols; }

inline QRect QGridView::cellRect() const
{ return QRect(0, 0, cellw, cellh); }

inline QSize QGridView::gridSize() const
{ return QSize(ncols * cellw, nrows * cellh); }



#endif // QT_NO_GRIDVIEW


#endif // QTABLEVIEW_H
