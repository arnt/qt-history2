#ifndef QGENERICTABLEVIEW_H
#define QGENERICTABLEVIEW_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QGenericHeader;
class QGenericTableViewPrivate;

class Q_GUI_EXPORT QGenericTableView : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGenericTableView);

public:
    QGenericTableView(QAbstractItemModel *model, QWidget *parent = 0);
    ~QGenericTableView();

    QGenericHeader *topHeader() const;
    QGenericHeader *leftHeader() const;
    void setTopHeader(QGenericHeader *header);
    void setLeftHeader(QGenericHeader *header);

    int rowViewportPosition(int row) const;
    int rowHeight(int row) const;
    int rowAt(int y) const;
    int columnViewportPosition(int column) const;
    int columnWidth(int column) const;
    int columnAt(int x) const;
    bool isRowHidden(int row) const;
    bool isColumnHidden(int column) const;
    void setShowGrid(bool show);
    bool showGrid() const;

    int contentsX() const;
    int contentsY() const;
    int contentsWidth() const;
    int contentsHeight() const;
    
public slots:
    void selectRow(int row, ButtonState state = Qt::NoButton);
    void selectColumn(int column, ButtonState state = Qt::NoButton);
    void hideRow(int row);
    void hideColumn(int column);
    void showRow(int row);
    void showColumn(int column);

protected slots:
    void rowIndexChanged(int row, int oldIndex, int newIndex);
    void columnIndexChanged(int column, int oldIndex, int newIndex);
    void rowHeightChanged(int row, int oldHeight, int newHeight);
    void columnWidthChanged(int column, int oldWidth, int newWidth);
    void rowCountChanged(int oldCount, int newCount);
    void columnCountChanged(int oldCount, int newCount);

protected:
    QGenericTableView(QGenericTableViewPrivate &, QAbstractItemModel *model, QWidget *parent = 0);
    void scrollContentsBy(int dx, int dy);
    
    virtual void drawGrid(QPainter *p, int x, int y, int w, int h) const;
    void paintEvent(QPaintEvent *e);
    bool event(QEvent *e);
    
    QModelIndex itemAt(int x, int y) const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state);
    QRect itemViewportRect(const QModelIndex &item) const;
    void ensureItemVisible(const QModelIndex &item);

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionUpdateMode mode);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void updateGeometries();

    int rowSizeHint(int row) const;
    int columnSizeHint(int column) const;

    void verticalScrollbarAction(int action);
    void horizontalScrollbarAction(int action);
};

#endif
