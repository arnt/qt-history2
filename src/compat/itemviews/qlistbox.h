/****************************************************************************
**
** Definition of QListBox widget class.
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

#ifndef QLISTBOX_H
#define QLISTBOX_H

#ifndef QT_H
#include "qscrollview.h"
#include "qpixmap.h"
#endif // QT_H

#ifndef QT_NO_LISTBOX


class QListBoxPrivate;
class QListBoxItem;
class QString;
class QStringList;


class Q_COMPAT_EXPORT QListBox : public QScrollView
{
    friend class QListBoxItem;
    friend class QListBoxPrivate;

    Q_OBJECT
    Q_ENUMS(SelectionMode LayoutMode)
    Q_PROPERTY(uint count READ count)
    Q_PROPERTY(int numItemsVisible READ numItemsVisible)
    Q_PROPERTY(int currentItem READ currentItem WRITE setCurrentItem)
    Q_PROPERTY(QString currentText READ currentText)
    Q_PROPERTY(int topItem READ topItem WRITE setTopItem DESIGNABLE false)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)
    Q_PROPERTY(bool multiSelection READ isMultiSelection WRITE setMultiSelection DESIGNABLE false)
    Q_PROPERTY(LayoutMode columnMode READ columnMode WRITE setColumnMode)
    Q_PROPERTY(LayoutMode rowMode READ rowMode WRITE setRowMode)
    Q_PROPERTY(int numColumns READ numColumns)
    Q_PROPERTY(int numRows READ numRows)
    Q_PROPERTY(bool variableWidth READ variableWidth WRITE setVariableWidth)
    Q_PROPERTY(bool variableHeight READ variableHeight WRITE setVariableHeight)

public:
    QListBox(QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~QListBox();

    uint count() const;

    void insertStringList(const QStringList&, int index=-1);
// ### fix before Qt 4.0
#if defined(QT_COMPAT) && 0
    QT_COMPAT void insertStrList(const QStrList *, int index=-1);
    QT_COMPAT void insertStrList(const QStrList &, int index=-1);
#endif
    void insertStrList(const char **,
                        int numStrings=-1, int index=-1);

    void insertItem(const QListBoxItem *, int index=-1);
    void insertItem(const QListBoxItem *, const QListBoxItem *after);
    void insertItem(const QString &text, int index=-1);
    void insertItem(const QPixmap &pixmap, int index=-1);
    void insertItem(const QPixmap &pixmap, const QString &text, int index=-1);

    void removeItem(int index);

    QString text(int index)        const;
    const QPixmap *pixmap(int index)        const;

    void changeItem(const QListBoxItem *, int index);
    void changeItem(const QString &text, int index);
    void changeItem(const QPixmap &pixmap, int index);
    void changeItem(const QPixmap &pixmap, const QString &text, int index);

    void takeItem(const QListBoxItem *);

    int numItemsVisible() const;

    int currentItem() const;
    QString currentText() const { return text(currentItem()); }
    virtual void setCurrentItem(int index);
    virtual void setCurrentItem(QListBoxItem *);
    void centerCurrentItem() { ensureCurrentVisible(); }
    int topItem() const;
    virtual void setTopItem(int index);
    virtual void setBottomItem(int index);

    long maxItemWidth() const;

    enum SelectionMode { Single, Multi, Extended, NoSelection };
    virtual void setSelectionMode(SelectionMode);
    SelectionMode selectionMode() const;

    void setMultiSelection(bool multi);
    bool isMultiSelection() const;

    virtual void setSelected(QListBoxItem *, bool);
    void setSelected(int, bool);
    bool isSelected(int) const;
    bool isSelected(const QListBoxItem *) const;
    QListBoxItem* selectedItem() const;

    QSize sizeHint() const;
    QSize        minimumSizeHint() const;

    QListBoxItem *item(int index) const;
    int index(const QListBoxItem *) const;

    enum StringComparisonMode {
        CaseSensitive   = 0x00001, // 0 0001
        BeginsWith      = 0x00002, // 0 0010
        EndsWith        = 0x00004, // 0 0100
        Contains        = 0x00008, // 0 1000
        ExactMatch      = 0x00010  // 1 0000
    };
    typedef uint ComparisonFlags;
    QListBoxItem *findItem(const QString &text, ComparisonFlags compare = BeginsWith) const;

    void triggerUpdate(bool doLayout);

    bool itemVisible(int index);
    bool itemVisible(const QListBoxItem *);

    enum LayoutMode { FixedNumber,
                      FitToWidth, FitToHeight = FitToWidth,
                      Variable };
    virtual void setColumnMode(LayoutMode);
    virtual void setColumnMode(int);
    virtual void setRowMode(LayoutMode);
    virtual void setRowMode(int);

    LayoutMode columnMode() const;
    LayoutMode rowMode() const;

    int numColumns() const;
    int numRows() const;

    bool variableWidth() const;
    virtual void setVariableWidth(bool);

    bool variableHeight() const;
    virtual void setVariableHeight(bool);

    void viewportPaintEvent(QPaintEvent *);

#ifdef QT_COMPAT
    QT_COMPAT bool dragSelect() const { return true; }
    QT_COMPAT void setDragSelect(bool) {}
    QT_COMPAT bool autoScroll() const { return true; }
    QT_COMPAT void setAutoScroll(bool) {}
    QT_COMPAT bool autoScrollBar() const { return vScrollBarMode() == Auto; }
    QT_COMPAT void setAutoScrollBar(bool enable) { setVScrollBarMode(enable ? Auto : AlwaysOff); }
    QT_COMPAT bool scrollBar() const { return vScrollBarMode() != AlwaysOff; }
    QT_COMPAT void setScrollBar(bool enable) { setVScrollBarMode(enable ? AlwaysOn : AlwaysOff); }
    QT_COMPAT bool autoBottomScrollBar() const { return hScrollBarMode() == Auto; }
    QT_COMPAT void setAutoBottomScrollBar(bool enable) { setHScrollBarMode(enable ? Auto : AlwaysOff); }
    QT_COMPAT bool bottomScrollBar() const { return hScrollBarMode() != AlwaysOff; }
    QT_COMPAT void setBottomScrollBar(bool enable) { setHScrollBarMode(enable ? AlwaysOn : AlwaysOff); }
    QT_COMPAT bool smoothScrolling() const { return false; }
    QT_COMPAT void setSmoothScrolling(bool) {}
    QT_COMPAT bool autoUpdate() const { return true; }
    QT_COMPAT void setAutoUpdate(bool) {}
    QT_COMPAT void setFixedVisibleLines(int lines) { setRowMode(lines); }
    QT_COMPAT int inSort(const QListBoxItem *);
    QT_COMPAT int inSort(const QString& text);
    QT_COMPAT int cellHeight(int i) const { return itemHeight(i); }
    QT_COMPAT int cellHeight() const { return itemHeight(); }
    QT_COMPAT int cellWidth() const { return maxItemWidth(); }
    QT_COMPAT int cellWidth(int i) const { Q_ASSERT(i==0); Q_UNUSED(i) return maxItemWidth(); }
    QT_COMPAT int        numCols() const { return numColumns(); }
#endif

    int itemHeight(int index = 0) const;
    QListBoxItem * itemAt(const QPoint &) const;

    QRect itemRect(QListBoxItem *item) const;

    QListBoxItem *firstItem() const;

    void sort(bool ascending = true);

public slots:
    void clear();
    virtual void ensureCurrentVisible();
    virtual void clearSelection();
    virtual void selectAll(bool select);
    virtual void invertSelection();

signals:
    void highlighted(int index);
    void selected(int index);
    void highlighted(const QString &);
    void selected(const QString &);
    void highlighted(QListBoxItem *);
    void selected(QListBoxItem *);

    void selectionChanged();
    void selectionChanged(QListBoxItem *);
    void currentChanged(QListBoxItem *);
    void clicked(QListBoxItem *);
    void clicked(QListBoxItem *, const QPoint &);
    void pressed(QListBoxItem *);
    void pressed(QListBoxItem *, const QPoint &);

    void doubleClicked(QListBoxItem *);
    void returnPressed(QListBoxItem *);
    void rightButtonClicked(QListBoxItem *, const QPoint &);
    void rightButtonPressed(QListBoxItem *, const QPoint &);
    void mouseButtonPressed(int, QListBoxItem*, const QPoint&);
    void mouseButtonClicked(int, QListBoxItem*, const QPoint&);

    void contextMenuRequested(QListBoxItem *, const QPoint &);

    void onItem(QListBoxItem *item);
    void onViewport();

protected:
    void changeEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void contentsContextMenuEvent(QContextMenuEvent *);

    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);

    bool eventFilter(QObject *o, QEvent *e);

    void updateItem(int index);
    void updateItem(QListBoxItem *);

#ifdef QT_COMPAT
    QT_COMPAT void updateCellWidth() { }
    QT_COMPAT int totalWidth() const { return contentsWidth(); }
    QT_COMPAT int totalHeight() const { return contentsHeight(); }
#endif

    virtual void paintCell(QPainter *, int row, int col);

    void toggleCurrentItem();
    bool isRubberSelecting() const;

    void doLayout() const;

#ifdef QT_COMPAT
    QT_COMPAT int findItem(int yPos) const { return index(itemAt(QPoint(0,yPos))); }
#endif

protected slots:
    void clearInputString();

private slots:
    void refreshSlot();
    void doAutoScroll();
    void adjustItems();

private:
    void mousePressEventEx(QMouseEvent *);
    void tryGeometry(int, int) const;
    int currentRow() const;
    int currentColumn() const;
    void updateSelection();
    void repaintSelection();
    void drawRubber();
    void doRubberSelection(const QRect &old, const QRect &rubber);
    void handleItemChange(QListBoxItem *old, bool shift, bool control);
    void selectRange(QListBoxItem *from, QListBoxItem *to, bool invert, bool includeFirst, bool clearSel = false);

    void emitChangedSignal(bool);

    int columnAt(int) const;
    int rowAt(int) const;

    QListBoxPrivate * d;

    static QListBox * changedListBox;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBox(const QListBox &);
    QListBox &operator=(const QListBox &);
#endif
};


class Q_COMPAT_EXPORT QListBoxItem
{
public:
    QListBoxItem(QListBox* listbox = 0);
    QListBoxItem(QListBox* listbox, QListBoxItem *after);
    virtual ~QListBoxItem();

    virtual QString text() const;
    virtual const QPixmap *pixmap() const;

    virtual int         height(const QListBox *) const;
    virtual int         width(const QListBox *)  const;

    bool isSelected() const { return s; }
    bool isCurrent() const;

#ifdef QT_COMPAT
    QT_COMPAT bool selected() const { return isSelected(); }
    QT_COMPAT bool current() const { return isCurrent(); }
#endif

    QListBox *listBox() const;

    void setSelectable(bool b) { selectable = b; }
    bool isSelectable() const { return selectable; }

    QListBoxItem *next() const;
    QListBoxItem *prev() const;

    virtual int rtti() const;
    enum { RTTI = 0 };

protected:
    virtual void paint(QPainter *) = 0;
    virtual void setText(const QString &text) { txt = text; }
    void setCustomHighlighting(bool);

private:
    QString txt;
    uint selectable : 1;
    uint s : 1;
    uint dirty:1;
    uint custom_highlight : 1;
    uint unused : 28;
    QListBoxItem * p, * n;
    QListBox* lbox;
    friend class QListBox;
    friend class QListBoxPrivate;
    friend class QComboBox;
    friend class QComboBoxPopupItem;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBoxItem(const QListBoxItem &);
    QListBoxItem &operator=(const QListBoxItem &);
#endif
};


class Q_COMPAT_EXPORT QListBoxText : public QListBoxItem
{
public:
    QListBoxText(QListBox* listbox, const QString & text=QString::null);
    QListBoxText(const QString & text=QString::null);
    QListBoxText(QListBox* listbox, const QString & text, QListBoxItem *after);
   ~QListBoxText();

    int         height(const QListBox *) const;
    int         width(const QListBox *)  const;

    int rtti() const;
    enum { RTTI = 1 };

protected:
    void  paint(QPainter *);

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBoxText(const QListBoxText &);
    QListBoxText &operator=(const QListBoxText &);
#endif
};


class Q_COMPAT_EXPORT QListBoxPixmap : public QListBoxItem
{
public:
    QListBoxPixmap(QListBox* listbox, const QPixmap &);
    QListBoxPixmap(const QPixmap &);
    QListBoxPixmap(QListBox* listbox, const QPixmap & pix, QListBoxItem *after);
    QListBoxPixmap(QListBox* listbox, const QPixmap &, const QString&);
    QListBoxPixmap(const QPixmap &, const QString&);
    QListBoxPixmap(QListBox* listbox, const QPixmap & pix, const QString&, QListBoxItem *after);
   ~QListBoxPixmap();

    const QPixmap *pixmap() const { return &pm; }

    int         height(const QListBox *) const;
    int         width(const QListBox *)  const;

    int rtti() const;
    enum { RTTI = 2 };

protected:
    void paint(QPainter *);

private:
    QPixmap pm;
private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBoxPixmap(const QListBoxPixmap &);
    QListBoxPixmap &operator=(const QListBoxPixmap &);
#endif
};


#endif // QT_NO_LISTBOX

#endif // QLISTBOX_H
