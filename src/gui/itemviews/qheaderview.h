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

#include <qabstractitemview.h>

class QHeaderViewPrivate;

class Q_GUI_EXPORT QHeaderView : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QHeaderView)
    Q_PROPERTY(bool showSortIndicator READ isSortIndicatorShown WRITE setSortIndicatorShown)
    Q_PROPERTY(bool highlightCurrentSection READ highlightCurrentSection WRITE setHighlightCurrentSection)
    Q_ENUMS(ResizeMode)

public:

    enum ResizeMode
    {
        Interactive, // don't change the size (let the user decide)
        Stretch, // fill available visible space
        Custom // let somebody else do the resize
    };

    QHeaderView(Qt::Orientation orientation, QWidget *parent = 0);
    virtual ~QHeaderView();

    void setModel(QAbstractItemModel *model);

    Qt::Orientation orientation() const;
    int offset() const;
    int length() const;
    QSize sizeHint() const;
    int sectionSizeHint(int section) const;

    int indexAt(int position) const;
    int sectionAt(int position) const;
    int sectionSize(int section) const;
    int sectionPosition(int section) const;

    inline int sectionAt(int x, int y) const
        { return orientation() == Qt::Horizontal ? sectionAt(x) : sectionAt(y); }
    inline int sectionAt(QPoint pos) const
        { return sectionAt(pos.x(), pos.y()); }

    void moveSection(int from, int to);
    void resizeSection(int section, int size);

    inline void hideSection(int section)
        { setSectionHidden(section, true); }
    inline void showSection(int section)
        { setSectionHidden(section, false); }

    bool isSectionHidden(int section) const;
    void setSectionHidden(int section, bool hide);

    int count() const;
    int index(int section) const;
    int section(int index) const;

    void setMovable(bool movable);
    bool isMovable() const;

    void setClickable(bool clickable);
    bool isClickable() const;

    void setHighlightCurrentSection(bool highlight);
    bool highlightCurrentSection() const;

    void setResizeMode(ResizeMode mode);
    void setResizeMode(ResizeMode mode, int section);
    ResizeMode resizeMode(int section) const;
    int stretchSectionCount() const;

    void setSortIndicatorShown(bool show);
    bool isSortIndicatorShown() const;

    void setSortIndicator(int section, Qt::SortOrder order);
    int sortIndicatorSection() const;
    Qt::SortOrder sortIndicatorOrder() const;

    QRect itemViewportRect(const QModelIndex &index) const;
    void ensureItemVisible(const QModelIndex &index);

    void doItemsLayout();

public slots:
    void setOffset(int offset);
    void headerDataChanged(Qt::Orientation orientation, int first, int last);

signals:
    void sectionIndexChanged(int section, int oldIndex, int newIndex);
    void sectionSizeChanged(int section, int oldSize, int newSize);
    void sectionPressed(int section, Qt::ButtonState state);
    void sectionClicked(int section, Qt::ButtonState state);
    void sectionCountChanged(int oldCount, int newCount);
    void sectionHandleDoubleClicked(int section, Qt::ButtonState state);
    void sectionAutoResize(int section, ResizeMode mode);

protected slots:
    void updateSection(int section);
    void resizeSections();
    void sectionsInserted(const QModelIndex &parent, int first, int last);
    void sectionsRemoved(const QModelIndex &parent, int first, int last);

protected:
    QHeaderView(QHeaderViewPrivate &, Qt::Orientation orientation, QWidget *parent);

    void initializeSections();
    void initializeSections(int start, int end);
    void currentChanged(const QModelIndex &old, const QModelIndex &current);

    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

    virtual void paintSection(QPainter *painter, const QRect &rect, int section) const;
    virtual QSize sectionSizeFromContents(int section) const;

    QModelIndex itemAt(int x, int y) const;
    bool isIndexHidden(const QModelIndex &index) const;

    int horizontalOffset() const;
    int verticalOffset() const;
    
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state);
    void setSelection(const QRect&, QItemSelectionModel::SelectionFlags) {}
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void updateGeometries();
};

#endif
