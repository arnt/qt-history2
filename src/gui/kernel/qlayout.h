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

#ifndef QT_H
#include "qobject.h"
#include "qsizepolicy.h"
#include "qrect.h"
#ifdef QT_INCLUDE_COMPAT
#include "qwidget.h"
#endif
#endif // QT_H

#include <limits.h>

#ifndef QT_NO_LAYOUT

static const int QLAYOUTSIZE_MAX = INT_MAX/256/16;

class QLayout;
class QLayoutItem;
class QSpacerItem;
class QWidget;

class Q_GUI_EXPORT QLayoutItem
{
public:
    QLayoutItem(Qt::Alignment alignment = 0) : align(alignment) { }
    virtual ~QLayoutItem();
    virtual QSize sizeHint() const = 0;
    virtual QSize minimumSize() const = 0;
    virtual QSize maximumSize() const = 0;
    virtual QSizePolicy::ExpandData expanding() const = 0;
    virtual void setGeometry(const QRect&) = 0;
    virtual QRect geometry() const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool hasHeightForWidth() const;
    virtual int heightForWidth(int) const;
    virtual int minimumHeightForWidth(int) const;
    virtual void invalidate();

    virtual QWidget *widget();
    virtual QLayout *layout();
    virtual QSpacerItem *spacerItem();

    Qt::Alignment alignment() const { return align; }
    void setAlignment(Qt::Alignment a);

protected:
    Qt::Alignment align;
};

class Q_GUI_EXPORT QSpacerItem : public QLayoutItem
{
public:
    QSpacerItem(int w, int h,
                 QSizePolicy::SizeType hData = QSizePolicy::Minimum,
                 QSizePolicy::SizeType vData = QSizePolicy::Minimum)
        : width(w), height(h), sizeP(hData, vData) { }
    void changeSize(int w, int h,
                     QSizePolicy::SizeType hData = QSizePolicy::Minimum,
                     QSizePolicy::SizeType vData = QSizePolicy::Minimum);
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry(const QRect&);
    QRect geometry() const;
    QSpacerItem *spacerItem();

private:
    int width;
    int height;
    QSizePolicy sizeP;
    QRect rect;
};

class Q_GUI_EXPORT QWidgetItem : public QLayoutItem
{
public:
    QWidgetItem(QWidget *w) : wid(w) { }
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry(const QRect&);
    QRect geometry() const;
    virtual QWidget *widget();

    bool hasHeightForWidth() const;
    int heightForWidth(int) const;

private:
    QWidget *wid;
};

#ifdef QT_COMPAT
class Q_GUI_EXPORT QLayoutIterator
{
public:
    inline QT_COMPAT_CONSTRUCTOR QLayoutIterator(QLayout *i) : layout(i), index(0) {}
    inline QLayoutIterator(const QLayoutIterator &i)
	: layout(i.layout), index(i.index) {}
    inline QLayoutIterator &operator=(const QLayoutIterator &i) {
        layout = i.layout;
        index = i.index;
        return *this;
    }
    inline QT_COMPAT QLayoutItem *operator++();
    inline QT_COMPAT QLayoutItem *current();
    inline QT_COMPAT QLayoutItem *takeCurrent();
    inline QT_COMPAT void deleteCurrent();

private:
    // hack to avoid deprecated warning
    friend class QLayout;
    inline QLayoutIterator(QLayout *i, bool) : layout(i), index(0) {}
    QLayout *layout;
    int index;
};
#endif

class QLayoutPrivate;

class Q_GUI_EXPORT QLayout : public QObject, public QLayoutItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QLayout)

    Q_ENUMS(ResizeMode)
    Q_PROPERTY(int margin READ margin WRITE setMargin)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)

public:
    enum ResizeMode { Auto, FreeResize, Minimum, Fixed };

    QLayout(QWidget *parent);
    QLayout(QLayout *parentLayout);
    QLayout();
    ~QLayout();

    int margin() const;
    int spacing() const;

    void setMargin(int);
    void setSpacing(int);

    void setResizeMode(ResizeMode);
    ResizeMode resizeMode() const;

    void setMenuBar(QWidget *w);
    QWidget *menuBar() const;

    QWidget *parentWidget() const;
    bool isTopLevel() const;

    void invalidate();
    QRect geometry() const;
    bool activate();
    void update();

    void addWidget(QWidget *w);
    virtual void addItem(QLayoutItem *) = 0;

    void removeWidget(QWidget *w);
    void removeItem(QLayoutItem *);

    QSizePolicy::ExpandData expanding() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    void setGeometry(const QRect&) = 0;
    virtual QLayoutItem *itemAt(int index) const = 0;
    virtual QLayoutItem *takeAt(int index) = 0;
    bool isEmpty() const;

    int totalHeightForWidth(int w) const;
    QSize totalMinimumSize() const;
    QSize totalMaximumSize() const;
    QSize totalSizeHint() const;
    QLayout *layout();

    void setEnabled(bool);
    bool isEnabled() const;

    void freeze(int w=0, int h=0);

protected:
    void widgetEvent(QEvent *);
    void childEvent(QChildEvent *e);
    void addChildLayout(QLayout *l);
    void addChildWidget(QWidget *w);
    void deleteAllItems();

    QRect alignmentRect(const QRect&) const;
protected:
    QLayout(QLayoutPrivate &d, QLayout*, QWidget*);

private:
    friend class QApplication;
    static void activateRecursiveHelper(QLayoutItem *item);

private:
#if defined(Q_DISABLE_COPY)
    QLayout(const QLayout &);
    QLayout &operator=(const QLayout &);
#endif

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QLayout(QWidget *parent, int margin, int spacing = -1,
                             const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QLayout(QLayout *parentLayout, int spacing, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QLayout(int spacing, const char *name = 0);
    inline QT_COMPAT QWidget *mainWidget() const { return parentWidget(); }
    inline QT_COMPAT void remove(QWidget *w) { removeWidget(w); }
    inline QT_COMPAT void add(QWidget *w) { addWidget(w); }

    QT_COMPAT void setAutoAdd(bool a);
    QT_COMPAT bool autoAdd() const;
    inline QT_COMPAT QLayoutIterator iterator() { return QLayoutIterator(this,true); }

    inline QT_COMPAT int defaultBorder() const { return spacing(); }
#endif
};

#ifdef QT_COMPAT
inline QLayoutItem *QLayoutIterator::operator++() { return layout->itemAt(++index); }
inline QLayoutItem *QLayoutIterator::current() { return layout->itemAt(index); }
inline QLayoutItem *QLayoutIterator::takeCurrent() { return layout->takeAt(index); }
inline void QLayoutIterator::deleteCurrent() { delete  layout->takeAt(index); }
#endif


class QGridLayoutPrivate;

class Q_GUI_EXPORT QGridLayout : public QLayout
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGridLayout)
public:
    QGridLayout(QWidget *parent);
    QGridLayout(QLayout *parentLayout);
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
protected:
    bool findWidget(QWidget* w, int *r, int *c);
    void addItem(QLayoutItem *);

private:
#if defined(Q_DISABLE_COPY)
    QGridLayout(const QGridLayout &);
    QGridLayout &operator=(const QGridLayout &);
#endif

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

    QBoxLayout(Direction, QWidget *parent);
    QBoxLayout(Direction, QLayout *parentLayout);
    QBoxLayout(Direction);

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
#if defined(Q_DISABLE_COPY)
    QBoxLayout(const QBoxLayout &);
    QBoxLayout &operator=(const QBoxLayout &);
#endif
};

class Q_GUI_EXPORT QHBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QHBoxLayout();
    QHBoxLayout(QWidget *parent);
    QHBoxLayout(QLayout *parentLayout);
    ~QHBoxLayout();

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QHBoxLayout(QWidget *parent, int border,
                 int spacing = -1, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QHBoxLayout(QLayout *parentLayout,
                 int spacing, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QHBoxLayout(int spacing, const char *name = 0);
#endif

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QHBoxLayout(const QHBoxLayout &);
    QHBoxLayout &operator=(const QHBoxLayout &);
#endif
};

class Q_GUI_EXPORT QVBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QVBoxLayout();
    QVBoxLayout(QWidget *parent);
    QVBoxLayout(QLayout *parentLayout);
    ~QVBoxLayout();

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QVBoxLayout(QWidget *parent, int border,
                 int spacing = -1, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QVBoxLayout(QLayout *parentLayout,
                 int spacing, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QVBoxLayout(int spacing, const char *name = 0);
#endif

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QVBoxLayout(const QVBoxLayout &);
    QVBoxLayout &operator=(const QVBoxLayout &);
#endif
};


#endif // QT_NO_LAYOUT
#endif // QLAYOUT_H
