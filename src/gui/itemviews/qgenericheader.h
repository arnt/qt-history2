#ifndef QGENERICHEADER_H
#define QGENERICHEADER_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QGenericHeaderPrivate;

class Q_GUI_EXPORT QGenericHeader : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGenericHeader)

public:

    enum ResizeMode
    {
        Interactive, // don't change the size (let the user decide)
        Stretch, // fill available visible space
        Custom // let somebody else do the resize
    };

    QGenericHeader(QAbstractItemModel *model, Qt::Orientation orientation, QWidget *parent = 0);
    virtual ~QGenericHeader();

    Qt::Orientation orientation() const;
    int offset() const;
    int size() const;
    QSize sizeHint() const;
    int sectionSizeHint(int section) const;

    int indexAt(int position) const;
    int sectionAt(int position) const;
    int sectionSize(int section) const;
    int sectionPosition(int section) const;

    void moveSection(int from, int to);
    void resizeSection(int section, int size);

    void hideSection(int section);
    void showSection(int section);
    bool isSectionHidden(int section) const;

    int count() const;
    int index(int section) const;
    int section(int index) const;

    void setMovable(bool movable);
    bool isMovable() const;

    void setClickable(bool clickable);
    bool isClickable() const;

    void setResizeMode(ResizeMode mode);
    void setResizeMode(ResizeMode mode, int section);
    ResizeMode resizeMode(int section) const;
    int stretchSectionCount() const;

    void setSortIndicator(int section, Qt::SortOrder order);
    int sortIndicatorSection() const;
    Qt::SortOrder sortIndicatorOrder() const;

    QRect itemViewportRect(const QModelIndex &item) const;
    void ensureItemVisible(const QModelIndex &index);
    QModelIndex itemAt(int x, int y) const;

public slots:
    void setOffset(int offset);

signals:
    void sectionIndexChanged(int section, int oldIndex, int newIndex);
    void sectionSizeChanged(int section, int oldSize, int newSize);
    void sectionClicked(int section, Qt::ButtonState state);
    void sectionCountChanged(int oldCount, int newCount);
    void sectionHandleDoubleClicked(int section, Qt::ButtonState state);
    void sectionAutoResize(int section, ResizeMode mode);

protected slots:
    void updateSection(int section);
    void resizeSections();

protected:
    void sectionsInserted(const QModelIndex &parent, int start, int end);
    void sectionsRemoved(const QModelIndex &parent, int start, int end);    
    void initializeSections(int start, int end);

    void paintEvent(QPaintEvent *e);

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

    virtual void paintSection(QPainter *painter, QItemOptions *options, const QModelIndex &item);

    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state);
    QModelIndex item(int section) const;
    void setSelection(const QRect&, QItemSelectionModel::SelectionFlags) {}
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void updateGeometries();
};

#endif
