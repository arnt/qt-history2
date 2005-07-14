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

#ifndef QHEADERVIEW_H
#define QHEADERVIEW_H

#include <QtGui/qabstractitemview.h>

QT_MODULE(Gui)

#ifndef QT_NO_ITEMVIEWS

class QHeaderViewPrivate;

class Q_GUI_EXPORT QHeaderView : public QAbstractItemView
{
    Q_OBJECT
    Q_PROPERTY(bool showSortIndicator READ isSortIndicatorShown WRITE setSortIndicatorShown)
    Q_PROPERTY(bool highlightSections READ highlightSections WRITE setHighlightSections)
    Q_PROPERTY(bool stretchLastSection READ stretchLastSection WRITE setStretchLastSection)
    Q_ENUMS(ResizeMode)

public:

    enum ResizeMode
    {
        Interactive, // don't change the size (let the user decide)
        Stretch, // fill available visible space
        Custom // let somebody else do the resize
    };

    explicit QHeaderView(Qt::Orientation orientation, QWidget *parent = 0);
    virtual ~QHeaderView();

    void setModel(QAbstractItemModel *model);

    Qt::Orientation orientation() const;
    int offset() const;
    int length() const;
    QSize sizeHint() const;
    int sectionSizeHint(int logicalIndex) const;

    int visualIndexAt(int position) const;
    int logicalIndexAt(int position) const;

    inline int logicalIndexAt(int x, int y) const;
    inline int logicalIndexAt(const QPoint &pos) const;

    int sectionSize(int logicalIndex) const;
    int sectionPosition(int logicalIndex) const;
    int sectionViewportPosition(int logicalIndex) const;

    void moveSection(int from, int to);
    void resizeSection(int logicalIndex, int size);

    bool isSectionHidden(int logicalIndex) const;
    void setSectionHidden(int logicalIndex, bool hide);

    inline void hideSection(int logicalIndex);
    inline void showSection(int logicalIndex);

    int count() const;
    int visualIndex(int logicalIndex) const;
    int logicalIndex(int visualIndex) const;

    void setMovable(bool movable);
    bool isMovable() const;

    void setClickable(bool clickable);
    bool isClickable() const;

    void setHighlightSections(bool highlight);
    bool highlightSections() const;

    void setResizeMode(ResizeMode mode);
    void setResizeMode(int logicalIndex, ResizeMode mode);
    ResizeMode resizeMode(int logicalIndex) const;
    int stretchSectionCount() const;

    void setSortIndicatorShown(bool show);
    bool isSortIndicatorShown() const;

    void setSortIndicator(int logicalIndex, Qt::SortOrder order);
    int sortIndicatorSection() const;
    Qt::SortOrder sortIndicatorOrder() const;

    bool stretchLastSection() const;
    void setStretchLastSection(bool stretch);

    void doItemsLayout();
    bool sectionsMoved() const;

public slots:
    void setOffset(int offset);
    void headerDataChanged(Qt::Orientation orientation, int logicalFirst, int logicalLast);

signals:
    void sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    void sectionResized(int logicalIndex, int oldSize, int newSize);
    void sectionPressed(int logicalIndex);
    void sectionClicked(int logicalIndex);
    void sectionDoubleClicked(int logicalIndex);
    void sectionCountChanged(int oldCount, int newCount);
    void sectionHandleDoubleClicked(int logicalIndex);
    void sectionAutoResize(int logicalIndex, ResizeMode mode);

protected slots:
    void updateSection(int logicalIndex);
    void resizeSections();
    void sectionsInserted(const QModelIndex &parent, int logicalFirst, int logicalLast);
    void sectionsAboutToBeRemoved(const QModelIndex &parent, int logicalFirst, int logicalLast);

protected:
    QHeaderView(QHeaderViewPrivate &dd, Qt::Orientation orientation, QWidget *parent = 0);
    void initialize();

    void initializeSections();
    void initializeSections(int start, int end);
    void currentChanged(const QModelIndex &old, const QModelIndex &current);

    bool event(QEvent *e);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

    virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    virtual QSize sectionSizeFromContents(int logicalIndex) const;

    int horizontalOffset() const;
    int verticalOffset() const;
    void updateGeometries();
    void scrollContentsBy(int dx, int dy);

    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void rowsInserted(const QModelIndex &parent, int start, int end);

    QRect visualRect(const QModelIndex &index) const;
    void scrollTo(const QModelIndex &index, ScrollHint hint);

    QModelIndex indexAt(const QPoint &p) const;
    bool isIndexHidden(const QModelIndex &index) const;

    QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
    void setSelection(const QRect&, QItemSelectionModel::SelectionFlags);
    QRegion visualRegionForSelection(const QItemSelection &selection) const;

private:
    Q_DECLARE_PRIVATE(QHeaderView)
    Q_DISABLE_COPY(QHeaderView)
};

inline int QHeaderView::logicalIndexAt(int ax, int ay) const
{ return orientation() == Qt::Horizontal ? logicalIndexAt(ax) : logicalIndexAt(ay); }

inline int QHeaderView::logicalIndexAt(const QPoint &apos) const
{ return logicalIndexAt(apos.x(), apos.y()); }

inline void QHeaderView::hideSection(int alogicalIndex)
{ setSectionHidden(alogicalIndex, true); }
inline void QHeaderView::showSection(int alogicalIndex)
{ setSectionHidden(alogicalIndex, false); }

#endif // QT_NO_ITEMVIEWS
#endif // QHEADERVIEW_H
