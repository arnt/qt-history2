#ifndef QGENERICTREEVIEW_H
#define QGENERICTREEVIEW_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QGenericTreeViewPrivate;
class QGenericHeader;

class Q_GUI_EXPORT QGenericTreeView : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGenericTreeView);

public:
    QGenericTreeView(QAbstractItemModel *model, QWidget *parent = 0);
    ~QGenericTreeView();

    QGenericHeader *header() const;
    void setHeader(QGenericHeader *header);

    int indentation() const;
    void setIndentation(int i);

    int editColumn() const;
    void setEditColumn(int column);

    bool showRootDecoration() const;
    void setShowRootDecoration(bool show);

    int columnViewportPosition(int column) const;
    int columnWidth(int column) const;
    int columnAt(int x) const;

    bool isColumnHidden(int column) const;
    bool isOpen(const QModelIndex &item) const;

public slots:
    void hideColumn(int column);
    void open(const QModelIndex &item);
    void close(const QModelIndex &item);

protected slots:
    void resizeColumnToContents(int column);
    void columnWidthChanged(int column, int oldSize, int newSize);
    void columnCountChanged(int oldCount, int newCount);
    void contentsChanged();

protected:
    void scrollContentsBy(int dx, int dy);
    void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsRemoved(const QModelIndex &parent, const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void startItemsLayout();

    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state);
    QModelIndex itemAt(int x, int y) const;
    int horizontalOffset() const;
    int verticalOffset() const;

    QRect itemViewportRect(const QModelIndex &item) const;
    void ensureItemVisible(const QModelIndex &item);

    void setSelection(const QRect &rect, int command);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void paintEvent(QPaintEvent *e);
    virtual void drawRow(QPainter *painter, QItemOptions *options, const QModelIndex &index) const;
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

    void mousePressEvent(QMouseEvent *e);

    void updateGeometries();
    void verticalScrollbarAction(int action);
    void horizontalScrollbarAction(int action);

    int columnSizeHint(int column) const;
};

#endif
