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

#ifndef QLAYOUT_H
#define QLAYOUT_H

#include "QtGui/qabstractlayout.h"
#ifdef QT_INCLUDE_COMPAT
#include "QtGui/qwidget.h"
#endif

#include <limits.h>

#ifndef QT_NO_LAYOUT

class QGridLayoutPrivate;

class Q_GUI_EXPORT QGridLayout : public QLayout
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGridLayout)
public:
    explicit QGridLayout(QWidget *parent);
    explicit QGridLayout(QLayout *parentLayout);
    QGridLayout();

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QGridLayout(QWidget *parent, int nRows, int nCols = 1, int border = 0,
                                      int spacing = -1, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QGridLayout(int nRows, int nCols = 1, int spacing = -1, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QGridLayout(QLayout *parentLayout, int nRows, int nCols = 1, int spacing = -1,
                                      const char *name = 0);
#endif
    ~QGridLayout();

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;

    void setRowStretch(int row, int stretch);
    void setColumnStretch(int column, int stretch);
    int rowStretch(int row) const;
    int columnStretch(int column) const;

    void setRowSpacing(int row, int minSize);
    void setColumnSpacing(int column, int minSize);
    int rowSpacing(int row) const;
    int columnSpacing(int column) const;

    int columnCount() const;
    int rowCount() const;
    QRect cellGeometry(int row, int column) const;

    bool hasHeightForWidth() const;
    int heightForWidth(int) const;
    int minimumHeightForWidth(int) const;

    QSizePolicy::ExpandData expanding() const;
    void invalidate();

    inline void addWidget(QWidget *w) { QLayout::addWidget(w); }
    void addWidget(QWidget *, int row, int column, Qt::Alignment = 0);
    void addWidget(QWidget *, int row, int column, int rowSpan, int columnSpan, Qt::Alignment = 0);
    void addLayout(QLayout *, int row, int column, Qt::Alignment = 0);
    void addLayout(QLayout *, int row, int column, int rowSpan, int columnSpan, Qt::Alignment = 0);

    void setOrigin(Qt::Corner);
    Qt::Corner origin() const;
    QLayoutItem *itemAt(int) const;
    QLayoutItem *takeAt(int);
    void setGeometry(const QRect&);

    void addItem(QLayoutItem *item, int row, int column, int rowSpan = 1, int columnSpan = 1, Qt::Alignment = 0);

    void setDefaultPositioning(int n, Qt::Orientation orient);
    void getItemPosition(int idx, int *row, int *column, int *rowSpan, int *columnSpan);

protected:
    bool findWidget(QWidget* w, int *r, int *c);
    void addItem(QLayoutItem *);

private:
    Q_DISABLE_COPY(QGridLayout)

#ifdef QT_COMPAT
public:
    QT_COMPAT void expand(int rows, int cols);
    inline QT_COMPAT void addRowSpacing(int row, int minsize) { addItem(new QSpacerItem(0,minsize), row, 0); }
    inline QT_COMPAT void addColSpacing(int col, int minsize) { addItem(new QSpacerItem(minsize,0), 0, col); }
    inline QT_COMPAT void addMultiCellWidget(QWidget *w, int fromRow, int toRow, int fromCol, int toCol, Qt::Alignment align = 0)
        { addWidget(w, fromRow, fromCol, (toRow < 0) ? -1 : toRow - fromRow + 1, (toCol < 0) ? -1 : toCol - fromCol + 1, align); }
    inline QT_COMPAT void addMultiCell(QLayoutItem *l, int fromRow, int toRow, int fromCol, int toCol, Qt::Alignment align = 0)
        { addItem(l, fromRow, fromCol, (toRow < 0) ? -1 : toRow - fromRow + 1, (toCol < 0) ? -1 : toCol - fromCol + 1, align); }
    inline QT_COMPAT void addMultiCellLayout(QLayout *layout, int fromRow, int toRow, int fromCol, int toCol, Qt::Alignment align = 0)
        { addLayout(layout, fromRow, fromCol, (toRow < 0) ? -1 : toRow - fromRow + 1, (toCol < 0) ? -1 : toCol - fromCol + 1, align); }

    inline QT_COMPAT int numRows() const { return rowCount(); }
    inline QT_COMPAT int numCols() const { return columnCount(); }
    inline QT_COMPAT void setColStretch(int col, int stretch) {setColumnStretch(col, stretch); }
    inline QT_COMPAT int colStretch(int col) const {return columnStretch(col); }
    inline QT_COMPAT void setColSpacing(int col, int minSize) { setColumnSpacing(col, minSize); }
    inline QT_COMPAT int colSpacing(int col) const { return columnSpacing(col); }
#endif
};

class QBoxLayoutPrivate;
class QDockWindow;

class Q_GUI_EXPORT QBoxLayout : public QLayout
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QBoxLayout)
public:
    enum Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop,
                     Down = TopToBottom, Up = BottomToTop };

    explicit QBoxLayout(Direction, QWidget *parent);
    explicit QBoxLayout(Direction, QLayout *parentLayout);
    explicit QBoxLayout(Direction);

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QBoxLayout(QWidget *parent, Direction, int border = 0, int spacing = -1,
                const char *name = 0);
    QT_COMPAT_CONSTRUCTOR  QBoxLayout(QLayout *parentLayout, Direction, int spacing = -1,
                const char *name = 0);
    QT_COMPAT_CONSTRUCTOR  QBoxLayout(Direction, int spacing, const char *name = 0);
#endif
    ~QBoxLayout();


    Direction direction() const;
    void setDirection(Direction);

    void addSpacing(int size);
    void addStretch(int stretch = 0);
    void addWidget(QWidget *, int stretch = 0, Qt::Alignment alignment = 0);
    void addLayout(QLayout *layout, int stretch = 0);
    void addStrut(int);
    void addItem(QLayoutItem *);

    void insertSpacing(int index, int size);
    void insertStretch(int index, int stretch = 0);
    void insertWidget(int index, QWidget *widget, int stretch = 0,
                       Qt::Alignment alignment = 0);
    void insertLayout(int index, QLayout *layout, int stretch = 0);

    bool setStretchFactor(QWidget *w, int stretch);
    bool setStretchFactor(QLayout *l, int stretch);
    bool setAlignment(QWidget *w, Qt::Alignment alignment);
    bool setAlignment(QLayout *l, Qt::Alignment alignment);
    inline void setAlignment(Qt::Alignment alignment) { QLayoutItem::setAlignment(alignment); }

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;

    bool hasHeightForWidth() const;
    int heightForWidth(int) const;
    int minimumHeightForWidth(int) const;

    QSizePolicy::ExpandData expanding() const;
    void invalidate();
    QLayoutItem *itemAt(int) const;
    QLayoutItem *takeAt(int);
    void setGeometry(const QRect&);

    int findWidget(QWidget* w);

protected:
    void insertItem(int index, QLayoutItem *);

private:
    Q_DISABLE_COPY(QBoxLayout)
};

class Q_GUI_EXPORT QHBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QHBoxLayout();
    explicit QHBoxLayout(QWidget *parent);
    explicit QHBoxLayout(QLayout *parentLayout);
    ~QHBoxLayout();

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QHBoxLayout(QWidget *parent, int border,
                 int spacing = -1, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QHBoxLayout(QLayout *parentLayout,
                 int spacing, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QHBoxLayout(int spacing, const char *name = 0);
#endif

private:
    Q_DISABLE_COPY(QHBoxLayout)
};

class Q_GUI_EXPORT QVBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QVBoxLayout();
    explicit QVBoxLayout(QWidget *parent);
    explicit QVBoxLayout(QLayout *parentLayout);
    ~QVBoxLayout();

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QVBoxLayout(QWidget *parent, int border,
                 int spacing = -1, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QVBoxLayout(QLayout *parentLayout,
                 int spacing, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QVBoxLayout(int spacing, const char *name = 0);
#endif

private:
    Q_DISABLE_COPY(QVBoxLayout)
};

#endif // QT_NO_LAYOUT

#endif // QLAYOUT_H
