/****************************************************************************
**
** Definition of QTable widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the table module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTABLE_H
#define QTABLE_H

#ifndef QT_H
#include "qscrollview.h"
#include "qpixmap.h"
#include "qheader.h"
#include "qguardedptr.h"
#include "qshared.h"
#include "qhash.h"
#include "qstringlist.h"
#include "qvector.h"
#endif // QT_H


#ifndef QT_NO_TABLE

#if !defined(QT_MODULE_TABLE) || defined(QT_LICENSE_PROFESSIONAL) || defined(QT_INTERNAL_TABLE)
#define QM_EXPORT_TABLE
#else
#define QM_EXPORT_TABLE Q_GUI_EXPORT
#endif

class QTableHeader;
class QValidator;
class QTable;
class QPaintEvent;
class QTimer;
class QResizeEvent;
class QComboBox;
class QCheckBox;
class QDragObject;

struct QTablePrivate;
struct QTableHeaderPrivate;


class QM_EXPORT_TABLE QTableSelection
{
public:
    QTableSelection();
    QTableSelection(int start_row, int start_col, int end_row, int end_col);
    void init(int row, int col);
    void expandTo(int row, int col);
    bool operator==(const QTableSelection &s) const;
    bool operator!=(const QTableSelection &s) const { return !(operator==(s)); }

    int topRow() const { return tRow; }
    int bottomRow() const { return bRow; }
    int leftCol() const { return lCol; }
    int rightCol() const { return rCol; }
    int anchorRow() const { return aRow; }
    int anchorCol() const { return aCol; }
    int numRows() const;
    int numCols() const;

    bool isActive() const { return active; }
    bool isEmpty() const { return numRows() == 0; }

private:
    uint active : 1;
    uint inited : 1;
    int tRow, lCol, bRow, rCol;
    int aRow, aCol;
};

class QM_EXPORT_TABLE QTableItem : public Qt
{
    friend class QTable;

public:
    enum EditType { Never, OnTyping, WhenCurrent, Always };

    QTableItem(QTable *table, EditType et);
    QTableItem(QTable *table, EditType et, const QString &text);
    QTableItem(QTable *table, EditType et, const QString &text,
                const QPixmap &p);
    virtual ~QTableItem();

    virtual QPixmap pixmap() const;
    virtual QString text() const;
    virtual void setPixmap(const QPixmap &p);
    virtual void setText(const QString &t);
    QTable *table() const { return t; }

    virtual void setAlignment(int);
    virtual int alignment() const;
    virtual void setWordWrap(bool b);
    bool wordWrap() const;

    EditType editType() const;
    virtual QWidget *createEditor() const;
    virtual void setContentFromEditor(QWidget *w);
    virtual void setReplaceable(bool);
    bool isReplaceable() const;

    virtual QString key() const;
    virtual QSize sizeHint() const;

    virtual void setSpan(int rs, int cs);
    int rowSpan() const;
    int colSpan() const;

    virtual void setRow(int r);
    virtual void setCol(int c);
    int row() const;
    int col() const;

    virtual void paint(QPainter *p, const QPalette &pal,
                        const QRect &cr, bool selected);

    void updateEditor(int oldRow, int oldCol);

    virtual void setEnabled(bool b);
    bool isEnabled() const;

    virtual int rtti() const;
    enum { RTTI = 0 };

private:
    QString txt;
    QPixmap pix;
    QTable *t;
    EditType edType;
    uint wordwrap : 1;
    uint tcha : 1;
    uint enabled : 1;
    int rw, cl;
    int rowspan, colspan;
};

class QM_EXPORT_TABLE QComboTableItem : public QTableItem
{
public:
    QComboTableItem(QTable *table, const QStringList &list, bool editable = false);
    ~QComboTableItem();
    virtual QWidget *createEditor() const;
    virtual void setContentFromEditor(QWidget *w);
    virtual void paint(QPainter *p, const QPalette &pal,
                        const QRect &cr, bool selected);
    virtual void setCurrentItem(int i);
    virtual void setCurrentItem(const QString &i);
    int currentItem() const;
    QString currentText() const;
    int count() const;
#if !defined(Q_NO_USING_KEYWORD)
    using QTableItem::text;
#endif
    QString text(int i) const;
    virtual void setEditable(bool b);
    bool isEditable() const;
    virtual void setStringList(const QStringList &l);

    int rtti() const;
    enum { RTTI = 0 };

    QSize sizeHint() const;

private:
    QComboBox *cb;
    QStringList entries;
    int current;
    bool edit;
    static QComboBox *fakeCombo;
    static QWidget *fakeComboWidget;
    static int fakeRef;

};

class QM_EXPORT_TABLE QCheckTableItem : public QTableItem
{
public:
    QCheckTableItem(QTable *table, const QString &txt);
    void setText(const QString &t);
    virtual QWidget *createEditor() const;
    virtual void setContentFromEditor(QWidget *w);
    virtual void paint(QPainter *p, const QPalette &pal,
                        const QRect &cr, bool selected);
    virtual void setChecked(bool b);
    bool isChecked() const;

    int rtti() const;
    enum { RTTI = 2 };

    QSize sizeHint() const;

private:
    QCheckBox *cb;
    bool checked;

};

class QM_EXPORT_TABLE QTable : public QScrollView
{
    Q_OBJECT
    Q_ENUMS(SelectionMode FocusStyle)
    Q_PROPERTY(int numRows READ numRows WRITE setNumRows)
    Q_PROPERTY(int numCols READ numCols WRITE setNumCols)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid)
    Q_PROPERTY(bool rowMovingEnabled READ rowMovingEnabled WRITE setRowMovingEnabled)
    Q_PROPERTY(bool columnMovingEnabled READ columnMovingEnabled WRITE setColumnMovingEnabled)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(bool sorting READ sorting WRITE setSorting)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)
    Q_PROPERTY(FocusStyle focusStyle READ focusStyle WRITE setFocusStyle)
    Q_PROPERTY(int numSelections READ numSelections)

    friend class QTableHeader;
    friend class QComboTableItem;
    friend class QCheckTableItem;
    friend class QTableItem;

public:
    QTable(QWidget* parent=0, const char* name=0);
    QTable(int numRows, int numCols,
            QWidget* parent=0, const char* name=0);
    ~QTable();

    QHeader *horizontalHeader() const;
    QHeader *verticalHeader() const;

    enum SelectionMode { Single, Multi, SingleRow, MultiRow, NoSelection };
    virtual void setSelectionMode(SelectionMode mode);
    SelectionMode selectionMode() const;

    virtual void setItem(int row, int col, QTableItem *item);
    virtual void setText(int row, int col, const QString &text);
    virtual void setPixmap(int row, int col, const QPixmap &pix);
    virtual QTableItem *item(int row, int col) const;
    virtual QString text(int row, int col) const;
    virtual QPixmap pixmap(int row, int col) const;
    virtual void clearCell(int row, int col);

    virtual QRect cellGeometry(int row, int col) const;
    virtual int columnWidth(int col) const;
    virtual int rowHeight(int row) const;
    virtual int columnPos(int col) const;
    virtual int rowPos(int row) const;
    virtual int columnAt(int x) const;
    virtual int rowAt(int y) const;

    virtual int numRows() const;
    virtual int numCols() const;

    void updateCell(int row, int col);

    bool eventFilter(QObject * o, QEvent *);

    int currentRow() const { return curRow; }
    int currentColumn() const { return curCol; }
    void ensureCellVisible(int row, int col);

    bool isSelected(int row, int col) const;
    bool isRowSelected(int row, bool full = false) const;
    bool isColumnSelected(int col, bool full = false) const;
    int numSelections() const;
    QTableSelection selection(int num) const;
    virtual int addSelection(const QTableSelection &s);
    virtual void removeSelection(const QTableSelection &s);
    virtual void removeSelection(int num);
    virtual int currentSelection() const;

    void selectCells(int start_row, int start_col, int end_row, int end_col);
    void selectRow(int row);
    void selectColumn(int col);

    bool showGrid() const;

    bool columnMovingEnabled() const;
    bool rowMovingEnabled() const;

    virtual void sortColumn(int col, bool ascending = true,
                             bool wholeRows = false);
    bool sorting() const;

    virtual void takeItem(QTableItem *i);

    virtual void setCellWidget(int row, int col, QWidget *e);
    virtual QWidget *cellWidget(int row, int col) const;
    virtual void clearCellWidget(int row, int col);

    virtual QRect cellRect(int row, int col) const;

    virtual void paintCell(QPainter *p, int row, int col,
                            const QRect &cr, bool selected);
    virtual void paintCell(QPainter *p, int row, int col,
                            const QRect &cr, bool selected, const QPalette &pal);
    virtual void paintFocus(QPainter *p, const QRect &r);
    QSize sizeHint() const;

    bool isReadOnly() const;
    bool isRowReadOnly(int row) const;
    bool isColumnReadOnly(int col) const;

    void setEnabled(bool b);

    void repaintSelections();

    enum FocusStyle { FollowStyle, SpreadSheet };
    virtual void setFocusStyle(FocusStyle fs);
    FocusStyle focusStyle() const;

    void updateHeaderStates();

public slots:
    virtual void setNumRows(int r);
    virtual void setNumCols(int r);
    virtual void setShowGrid(bool b);
    virtual void hideRow(int row);
    virtual void hideColumn(int col);
    virtual void showRow(int row);
    virtual void showColumn(int col);
    bool isRowHidden(int row) const;
    bool isColumnHidden(int col) const;

    virtual void setColumnWidth(int col, int w);
    virtual void setRowHeight(int row, int h);

    virtual void adjustColumn(int col);
    virtual void adjustRow(int row);

    virtual void setColumnStretchable(int col, bool stretch);
    virtual void setRowStretchable(int row, bool stretch);
    bool isColumnStretchable(int col) const;
    bool isRowStretchable(int row) const;
    virtual void setSorting(bool b);
    virtual void swapRows(int row1, int row2, bool swapHeader = false);
    virtual void swapColumns(int col1, int col2, bool swapHeader = false);
    virtual void swapCells(int row1, int col1, int row2, int col2);

    virtual void setLeftMargin(int m);
    virtual void setTopMargin(int m);
    virtual void setCurrentCell(int row, int col);
    void clearSelection(bool repaint = true);
    virtual void setColumnMovingEnabled(bool b);
    virtual void setRowMovingEnabled(bool b);

    virtual void setReadOnly(bool b);
    virtual void setRowReadOnly(int row, bool ro);
    virtual void setColumnReadOnly(int col, bool ro);

    virtual void setDragEnabled(bool b);
    bool dragEnabled() const;

    virtual void insertRows(int row, int count = 1);
    virtual void insertColumns(int col, int count = 1);
    virtual void removeRow(int row);
    virtual void removeRows(const QVector<int> &rows);
    virtual void removeColumn(int col);
    virtual void removeColumns(const QVector<int> &cols);

    virtual void editCell(int row, int col, bool replace = false);

    void setRowLabels(const QStringList &labels);
    void setColumnLabels(const QStringList &labels);

protected:
    enum EditMode { NotEditing, Editing, Replacing };
    void drawContents(QPainter *p, int cx, int cy, int cw, int ch);
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseMoveEvent(QMouseEvent*);
    void contentsMouseDoubleClickEvent(QMouseEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void contentsContextMenuEvent(QContextMenuEvent * e);
    void changeEvent(QEvent *);
    void keyPressEvent(QKeyEvent*);
    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);
    void viewportResizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *e);
    void paintEvent(QPaintEvent *e);
    void setEditMode(EditMode mode, int row, int col);
#ifndef QT_NO_DRAGANDDROP
    virtual void contentsDragEnterEvent(QDragEnterEvent *e);
    virtual void contentsDragMoveEvent(QDragMoveEvent *e);
    virtual void contentsDragLeaveEvent(QDragLeaveEvent *e);
    virtual void contentsDropEvent(QDropEvent *e);
    virtual QDragObject *dragObject();
    virtual void startDrag();
#endif

    virtual void paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch);
    virtual void activateNextCell();
    virtual QWidget *createEditor(int row, int col, bool initFromCell) const;
    virtual void setCellContentFromEditor(int row, int col);
    virtual QWidget *beginEdit(int row, int col, bool replace);
    virtual void endEdit(int row, int col, bool accept, bool replace);

    virtual void resizeData(int len);
    virtual void insertWidget(int row, int col, QWidget *w);
    int indexOf(int row, int col) const;

    bool isEditing() const;
    EditMode editMode() const;
    int currEditRow() const;
    int currEditCol() const;

protected slots:
    virtual void columnWidthChanged(int col);
    virtual void rowHeightChanged(int row);
    virtual void columnIndexChanged(int section, int fromIndex, int toIndex);
    virtual void rowIndexChanged(int section, int fromIndex, int toIndex);
    virtual void columnClicked(int col);

signals:
    void currentChanged(int row, int col);
    void clicked(int row, int col, int button, const QPoint &mousePos);
    void doubleClicked(int row, int col, int button, const QPoint &mousePos);
    void pressed(int row, int col, int button, const QPoint &mousePos);
    void selectionChanged();
    void valueChanged(int row, int col);
    void contextMenuRequested(int row, int col, const QPoint &pos);
#ifndef QT_NO_DRAGANDDROP
    void dropped(QDropEvent *e);
#endif

private slots:
    void doAutoScroll();
    void doValueChanged();
    void updateGeometriesSlot();

private:
    void contentsMousePressEventEx(QMouseEvent*);
    void drawContents(QPainter*);
    void updateGeometries();
    void repaintSelections(QTableSelection *oldSelection,
                            QTableSelection *newSelection,
                            bool updateVertical = true,
                            bool updateHorizontal = true);
    QRect rangeGeometry(int topRow, int leftCol,
                         int bottomRow, int rightCol, bool &optimize);
    void fixRow(int &row, int y);
    void fixCol(int &col, int x);

    void init(int numRows, int numCols);
    QSize tableSize() const;
    void repaintCell(int row, int col);
    void contentsToViewport2(int x, int y, int& vx, int& vy);
    QPoint contentsToViewport2(const QPoint &p);
    void viewportToContents2(int vx, int vy, int& x, int& y);
    QPoint viewportToContents2(const QPoint &p);

    void updateRowWidgets(int row);
    void updateColWidgets(int col);
    bool isSelected(int row, int col, bool includeCurrent) const;
    void setCurrentCell(int row, int col, bool updateSelections, bool ensureVisible = false);
    void fixCell(int &row, int &col, int key);
    void delayedUpdateGeometries();
    struct TableWidget
    {
        TableWidget(QWidget *w, int r, int c) : wid(w), row(r), col (c) {}
        QWidget *wid;
        int row, col;
    };
    void saveContents(QVector<QTableItem *> &tmp,
                       QVector<TableWidget *> &tmp2);
    void updateHeaderAndResizeContents(QTableHeader *header,
                                        int num, int colRow,
                                        int width, bool &updateBefore);
    void restoreContents(QVector<QTableItem *> &tmp,
                          QVector<TableWidget *> &tmp2);
    void finishContentsResze(bool updateBefore);

private:
    QVector<QTableItem *> contents;
    QVector<QWidget *> widgets;
    int curRow;
    int curCol;
    QTableHeader *leftHeader, *topHeader;
    EditMode edMode;
    int editCol, editRow;
    QList<QTableSelection *> selections;
    QTableSelection *currentSel;
    QTimer *autoScrollTimer;
    int lastSortCol;
    bool sGrid : 1;
    bool mRows : 1;
    bool mCols : 1;
    bool asc : 1;
    bool doSort : 1;
    bool unused : 1;
    bool readOnly : 1;
    bool shouldClearSelection : 1;
    bool dEnabled : 1;
    bool context_menu : 1;
    bool drawActiveSelection : 1;
    bool was_visible : 1;
    SelectionMode selMode;
    int pressedRow, pressedCol;
    QTablePrivate *d;
    QHash<int, int> roRows;
    QHash<int, int> roCols;
    int startDragRow;
    int startDragCol;
    QPoint dragStartPos;
    int oldCurrentRow, oldCurrentCol;
    FocusStyle focusStl;

#if defined(Q_DISABLE_COPY)
    QTable(const QTable &);
    QTable &operator=(const QTable &);
#endif
};

#endif // QT_NO_TABLE
#endif // TABLE_H
