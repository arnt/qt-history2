#include "qgenericheader.h"
#include <qvector.h>
#include <qbitarray.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qapplication.h>
#include <private/qabstractitemview_p.h>

class QGenericHeaderPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QGenericHeader)

public:
    QGenericHeaderPrivate()
        : state(NoState),
          offset(0),
          sortIndicatorOrder(Qt::Ascending),
          sortIndicatorSection(-1),
          movableSections(false),
          clickableSections(false),
          stretchSections(0),
          sectionIndicator(0) {}

    int sectionHandleAt(int pos);
    void setupSectionIndicator();
    void updateSectionIndicator();
    QRect sectionHandleRect(int section);

    inline bool reverse() const { return QApplication::reverseLayout() && orientation == Horizontal; }

    enum State { NoState, ResizeSection, MoveSection, SelectSection } state;
    int offset;

    Qt::Orientation orientation;
    Qt::SortOrder sortIndicatorOrder;
    int sortIndicatorSection;

    struct HeaderSection {
        int position;
        int section;
        uint hidden : 1;
        QGenericHeader::ResizeMode mode;
        inline bool operator>(int position) const
            { return (*this).position > position; }
    };
    QVector<HeaderSection> sections; // section = sections.at(index)
    QVector<int> indices; // index = indices.at(section)

    int lastPos;
    int section; // used for resizing and moving sections
    int target;
    bool movableSections;
    bool clickableSections;
    int stretchSections;
    QWidget *sectionIndicator;//, *sectionIndicator2;
};

static const int border = 4;
static const int minimum = 15;
static const int default_width = 100;
static const int default_height = 30;

#define d d_func()
#define q q_func()

/*!
  \class QGenericHeader qgenericheader.h

  \brief This class provides a header row or column, for itemviews.

*/

QGenericHeader::QGenericHeader(QAbstractItemModel *model, Orientation o, QWidget *parent)
    : QAbstractItemView(*new QGenericHeaderPrivate, model, parent)
{
    setVerticalScrollBarPolicy(ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(ScrollBarAlwaysOff);
    d->orientation = o;
    setFrameStyle(NoFrame);

    setMouseTracking(true);
    setFocusPolicy(NoFocus);

    // FIXME: this should also be called in setModel()
    contentsInserted(model->topLeft(root()), model->bottomRight(root()));
}

QGenericHeader::~QGenericHeader()
{
}

Qt::Orientation QGenericHeader::orientation() const
{
    return d->orientation;
}

int QGenericHeader::offset() const
{
    return d->reverse() ? -d->offset : d->offset;
}

void QGenericHeader::setOffset(int o)
{
    int ndelta = d->offset - o;
    d->offset = o;
    if (d->orientation == Horizontal)
        d->viewport->scroll(QApplication::reverseLayout() ? -ndelta : ndelta, 0);
    else
        d->viewport->scroll(0, ndelta);
}

int QGenericHeader::size() const
{
    if (d->sections.count())
        return d->sections.at(count()).position;
    return 0;
}

QSize QGenericHeader::sizeHint() const
{
    if (d->sections.isEmpty())
            return QSize();
    QModelIndex br = model()->bottomRight(root());
    QItemOptions options;
    getViewOptions(&options);
    int row = orientation() == Horizontal ? 0 : section(count() - 1);
    int col = orientation() == Horizontal ? section(count() - 1) : 0;
    QModelIndex::Type type = orientation() == Horizontal ? QModelIndex::HorizontalHeader : QModelIndex::VerticalHeader;
    QModelIndex item(row, col, 0, type);
    QSize hint = itemDelegate()->sizeHint(fontMetrics(), options, item);
    if (orientation() == Vertical)
        return QSize(hint.width() + border, size());
    return QSize(size(), hint.height() + border);
}

int QGenericHeader::sectionSizeHint(int section) const
{
    QItemOptions options;
    getViewOptions(&options);
    QAbstractItemDelegate *delegate = itemDelegate();
    int hint = 0;
    int row = orientation() == Vertical ? section : 0;
    int col = orientation() == Vertical ? 0 : section;
    QModelIndex::Type type = orientation() == Horizontal ?
                             QModelIndex::HorizontalHeader :
                             QModelIndex::VerticalHeader;
    QModelIndex header = model()->index(row, col, 0, type);
    if (orientation() == Vertical)
        hint = delegate->sizeHint(fontMetrics(), options, header).height();
    else
        hint = delegate->sizeHint(fontMetrics(), options, header).width();

    return hint + border;
}

void QGenericHeader::paintEvent(QPaintEvent *e)
{
    QPainter painter(d->viewport);
    QRect area = e->rect();

    int offset = this->offset();

    QItemOptions options;
    getViewOptions(&options);

    int start, end;
    if (orientation() == Horizontal) {
        start = indexAt(offset + area.left());
        end = indexAt(offset + area.right());
    } else {
        start = indexAt(offset + area.top());
        end = indexAt(offset + area.bottom());
    }

    int tmp = start;
    start = qMin(start, end);
    end = qMax(tmp, end);
    start = start == -1 ? 0 : start;
    end = end == -1 ? count() : end;

    QModelIndex item;
    if (d->sections.isEmpty())
        return;
    const QGenericHeaderPrivate::HeaderSection *sections = d->sections.constData();

    int section;
    if (d->orientation == Horizontal) {
        for (int i = start; i <= end; ++i) {
            if (sections[i].hidden)
                continue;
            section = sections[i].section;
            item = model()->index(0, section, 0, QModelIndex::HorizontalHeader);
            options.itemRect.setRect(sectionPosition(section) - offset, 0,
                                     sectionSize(section), height());
            paintSection(&painter, &options, item);
        }
    } else {
        for (int i = start; i <= end; ++i) {
            if (sections[i].hidden)
                continue;
            section = sections[i].section;
            item = model()->index(section, 0, 0, QModelIndex::VerticalHeader);
            options.itemRect.setRect(0, sectionPosition(section) - offset, width(), sectionSize(section));
            paintSection(&painter, &options, item);
        }
    }
}

void QGenericHeader::paintSection(QPainter *painter, QItemOptions *options, const QModelIndex &item)
{
    QStyle::SFlags flags = QStyle::Style_Off | (orientation() == Horizontal ? QStyle::Style_Horizontal : 0);
    if (isEnabled())
        flags |= QStyle::Style_Enabled;
    QStyle::SFlags arrowFlags = flags;
    if (d->clickableSections && (orientation() == Horizontal ?
          selectionModel()->isColumnSelected(item.column(), model()->parent(item)) :
          selectionModel()->isRowSelected(item.row(), model()->parent(item))))
        flags |= QStyle::Style_Down;
    else
        flags |= QStyle::Style_Raised;
    style().drawPrimitive(QStyle::PE_HeaderSection, painter, options->itemRect, palette(), flags);

    itemDelegate()->paint(painter, *options, item); // draw item

    int section = orientation() == Horizontal ? item.column() : item.row();
    if (sortIndicatorSection() == section) {
        //bool alignRight = style().styleHint(QStyle::SH_Header_ArrowAlignment, this) & AlignRight;
        // FIXME: use alignRight and RTL
        QRect arrowRect;
        int height = options->itemRect.height();
        int x = options->itemRect.x();
        int y = options->itemRect.y();
        if (orientation() == Qt::Horizontal)
            arrowRect.setRect(x + sectionSize(section) - border * 2 - (height / 2), y + 5,
                              height / 2, height - border * 2);
        else
            arrowRect.setRect(x + 5, y + sectionSize(section) - height, height / 2, height - border * 2);
        arrowFlags |= (sortIndicatorOrder() == Qt::Ascending ? QStyle::Style_Down : QStyle::Style_Up);
        style().drawPrimitive(QStyle::PE_HeaderArrow, painter, arrowRect, palette(), arrowFlags);
    }
}

int QGenericHeader::indexAt(int position) const
{
    if (count() < 1)
        return -1;

    if (d->reverse())
        position = size() - position;

    int start = 0;
    int end = count() - 1;
    int idx = (end + 1) / 2;

    const QGenericHeaderPrivate::HeaderSection *sections = d->sections.constData();

    while (end - start > 0) {
        if (sections[idx].position > position)
            end = idx - 1;
        else
            start = idx;
        idx = (start + end + 1) / 2;
    }
    return idx;
}

int QGenericHeader::sectionAt(int position) const
{
    int idx = indexAt(position);
    if (idx < 0 || idx >= d->sections.count())
        return -1;
    return d->sections.at(idx).section;
}

int QGenericHeader::sectionSize(int section) const
{
    if (section < 0 || section >= d->sections.count() - 1 || isSectionHidden(section))
        return 0;
    int idx = index(section);
    return d->sections.at(idx + 1).position - d->sections.at(idx).position;
}

int QGenericHeader::sectionPosition(int section) const
{
    if (section < 0 || section >= d->sections.count())
        return 0;
    if (d->reverse())
        return size() - d->sections.at(index(section)).position - sectionSize(section);
    return d->sections.at(index(section)).position;
}

void QGenericHeader::initializeSections(int start, int end)
{
    int oldCount = count();
    end += 1; // one past the last item, so we get the end position of the last section
    d->sections.resize(end + 1);
    if (oldCount >= count()) {
        d->viewport->update();
        emit sectionCountChanged(start, end);
        return;
    }

    int pos = (start <= 0 ? 0 : d->sections.at(start).position);
    QGenericHeaderPrivate::HeaderSection *sections = d->sections.data() + start;
    int s = start;
    int num = end - start + 1;
    int size = orientation() == Horizontal ? default_width : default_height;;

    // unroll loop - to initialize the arrays as fast as possible
    while (num >= 4) {

        sections[0].hidden = false;
        sections[0].mode = Interactive;
        sections[0].section = s++;
        sections[0].position = pos;
        pos += size;

        sections[1].hidden = false;
        sections[1].mode = Interactive;
        sections[1].section = s++;
        sections[1].position = pos;
        pos += size;

        sections[2].hidden = false;
        sections[2].mode = Interactive;
        sections[2].section = s++;
        sections[2].position = pos;
        pos += size;

        sections[3].hidden = false;
        sections[3].mode = Interactive;
        sections[3].section = s++;
        sections[3].position = pos;
        pos += size;

        sections += 4;
        num -= 4;
    }
    if (num > 0) {
        sections[0].hidden = false;
        sections[0].mode = Interactive;
        sections[0].section = s++;
        sections[0].position = pos;
        pos += size;
        if (num > 1) {
            sections[1].hidden = false;
            sections[1].mode = Interactive;
            sections[1].section = s++;
            sections[1].position = pos;
            pos += size;
            if (num > 2) {
                sections[2].hidden = false;
                sections[2].mode = Interactive;
                sections[2].section = s++;
                sections[2].position = pos;
                pos += size;
            }
        }
    }
    emit sectionCountChanged(oldCount, count());
    d->viewport->update();
}
/*
void QGenericHeader::contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QModelIndex parent = model()->parent(topLeft);
//    Q_ASSERT(parent == model()->parent(bottomRight));
    if (parent != root())
        return;
    QModelIndex::Type type = topLeft.type();
    int from = orientation() == Vertical ? topLeft.row() : topLeft.column();
    int to =  orientation() == Vertical ? bottomRight.row() : bottomRight.column();
    for (int idx = from; idx < to; ++idx) {
        int sec = section(idx);
        QModelIndex item = (orientation() == Vertical ?
                            model()->index(sec, 0, 0, type) :
                            model()->index(0, sec, 0, type));
        if (d->sections.at(idx).mode == Custom) { // resize the section to fit the new content
            QItemOptions options;
            getViewOptions(&options);
            int hint = 0;
            if (orientation() == Vertical)
                hint = itemDelegate()->sizeHint(fontMetrics(), options, item).height();
            else
                hint = itemDelegate()->sizeHint(fontMetrics(), options, item).width();
            int size = sectionSize(sec);
            if (hint > size)
                resizeSection(sec, hint);
            else // we have to check them all
                resizeSection(sec, sectionSizeHint(sec)); // FIXME: get the size of the section from the contents
            resizeSections();
        }
    }
    QAbstractItemView::contentsChanged(topLeft, bottomRight);
}
*/
void QGenericHeader::contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft.isValid() && bottomRight.isValid()) {
        QModelIndex parent = model()->parent(topLeft);
        if (orientation() == Horizontal)
            initializeSections(topLeft.column(), bottomRight.column());
        else
            initializeSections(topLeft.row(), bottomRight.row());
    }
}

void QGenericHeader::contentsRemoved(const QModelIndex &parent,
                                     const QModelIndex &topLeft, const QModelIndex &)
{
    if (orientation() == Horizontal)
        initializeSections(topLeft.column(), model()->columnCount(parent) - 1);
    else
        initializeSections(topLeft.row(), model()->rowCount(parent) - 1);
}

void QGenericHeader::ensureItemVisible(const QModelIndex &)
{
    // this should only update the scrollvalue, and only if the item is not visible
//     if (orientation() == Horizontal)
//         setOffset(sectionPosition(index.column()));
//     else
//         setOffset(sectionPosition(index.row()));
}

void QGenericHeader::updateSection(int section)
{
    if (orientation() == Horizontal)
        d->viewport->update(QRect(sectionPosition(section) - offset(), 0, sectionSize(section), height()));
    else
        d->viewport->update(QRect(0, sectionPosition(section) - offset(), width(), sectionSize(section)));
}

void QGenericHeader::resizeSections()
{
    ResizeMode mode;
    int secSize;
    int stretchSecs = 0;
    int stretchSize = orientation() == Horizontal ? d->viewport->width() : d->viewport->height();
    QList<int> section_sizes;
    int count = qMax(d->sections.count() - 1, 0);
    QGenericHeaderPrivate::HeaderSection *secs = d->sections.data();
    for (int i = 0; i < count; ++i) {
        mode = secs[i].mode;
        if (mode == Stretch) {
            ++stretchSecs;
            continue;
        }
        if (mode == Interactive)
            secSize = sectionSize(secs[i].section);
        else //if (mode == QGenericHeader::Custom)
            secSize = sectionSizeHint(secs[i].section); // FIXME: get the size of the section from the contents
        section_sizes.append(secSize);
        stretchSize -= secSize;
    }
    int position = 0;
    int stretchSectionSize = qMax(stretchSecs > 0 ? stretchSize / stretchSecs : 0, minimum);
    for (int i = 0; i < count; ++i) { // FIXME: last pos
        secs[i].position = position;
        mode = secs[i].mode;
        if (mode == Stretch) {
            position += stretchSectionSize;
        } else {
            position += section_sizes.front();
            section_sizes.removeFirst();
        }
    }
    secs[count].position = position;
}

void QGenericHeader::mousePressEvent(QMouseEvent *e)
{
    int pos = orientation() == Horizontal ? e->x() : e->y();
    if (e->state() & ControlButton && d->movableSections) {
        d->section = d->target = sectionAt(pos + offset());
        if (d->section == -1)
            return;
        d->state = QGenericHeaderPrivate::MoveSection;
        d->setupSectionIndicator();
        d->updateSectionIndicator();
    } else {
        int handle = d->sectionHandleAt(pos + offset());
        while (handle > -1 && isSectionHidden(handle)) handle--;
        if (handle == -1) {
            int sec = sectionAt(pos + offset());
            emit sectionClicked(sec, e->state());
            return;
        } else if (resizeMode(handle) == Interactive) {
            d->state = QGenericHeaderPrivate::ResizeSection;
            d->lastPos = (orientation() == Horizontal ? e->x() : e->y());
            d->section = handle;
        }
    }
}

void QGenericHeader::mouseMoveEvent(QMouseEvent *e)
{
    int pos = orientation() == Horizontal ? e->x() : e->y();

    switch (d->state) {
        case QGenericHeaderPrivate::ResizeSection: {
            int delta = d->reverse() ? d->lastPos - pos : pos - d->lastPos;
            int size = sectionSize(d->section) + delta;
            if (size > minimum) {
                resizeSection(d->section, size);
                d->lastPos = (orientation() == Horizontal ? e->x() : e->y());
            }
            return;
        }
        case QGenericHeaderPrivate::MoveSection: {
            int idx = indexAt(pos + offset());
            if (idx < 0)
                return;
            int loc = pos + offset() - d->sections.at(idx).position;
            int sec = d->sections.at(idx).section;
            if (loc > sectionSize(sec) / 2)
                sec = d->sections.at(qMin(idx + 1, d->sections.count() - 1)).section;
            d->target = sec;
            d->updateSectionIndicator();
            return;
        }
        case QGenericHeaderPrivate::NoState: {
            int handle = d->sectionHandleAt(pos + offset());
            if (handle != -1 && resizeMode(handle) == Interactive)
                setCursor(orientation() == Horizontal ? SplitHCursor : SplitVCursor);
            else
                setCursor(ArrowCursor);
            return;
        }

        case QGenericHeaderPrivate::SelectSection:
            //qDebug("SelectSection is not implemented");
            break; // FIXME: not implemented
    }
}

void QGenericHeader::mouseReleaseEvent(QMouseEvent *)
{
    // ### Unused variable:
    // int pos = orientation() == Horizontal ? e->x() : e->y();
    switch (d->state) {
    case QGenericHeaderPrivate::MoveSection:
        moveSection(index(d->section), index(d->target));
        d->section = d->target = -1;
        d->updateSectionIndicator();
        break;
    case QGenericHeaderPrivate::NoState:
        break;
    case QGenericHeaderPrivate::ResizeSection:
        break;
    case QGenericHeaderPrivate::SelectSection:
        qDebug("SelectSection is not implemented");
        break;
    }
    d->state = QGenericHeaderPrivate::NoState;
}

void QGenericHeader::mouseDoubleClickEvent(QMouseEvent *e)
{
    int pos = orientation() == Horizontal ? e->x() : e->y();
    int handle = d->sectionHandleAt(pos + offset());
    while (handle > -1 && isSectionHidden(handle)) handle--;
    if (handle > -1 && resizeMode(handle) == Interactive)
        emit sectionHandleDoubleClicked(handle, e->state());
}

void QGenericHeader::moveSection(int from, int to)
{
    if (from == to)
        return;

    // if we haven't moved anything previously, initialize the indices array
    int count = d->sections.count();
    if (d->indices.count() != count) {
        d->indices.resize(count);
        for (int s = 0; s < count; ++s)
            d->indices[s] = s;
    }

    // move sections and indices
    QGenericHeaderPrivate::HeaderSection *sections = d->sections.data();
    int *indices = d->indices.data();
    int sec = sections[from].section;
    int idx = from;
    if (to > from) {
        while (idx < to) {
            sections[idx].section = sections[idx+1].section;
            indices[sections[idx].section] = idx++;
        }
    } else {
        while (idx > to) {
            sections[idx].section = sections[idx-1].section;
            indices[sections[idx].section] = idx--;
        }
    }
    sections[to].section = sec;
    indices[sec] = to;

    // move positions
    if (to > from) {
        for (idx = from; idx < to; ++idx)
            sections[idx+1].position -= sectionSize(section(idx)) - sectionSize(section(idx+1));
    } else {
        int tmp;
        int size = sectionSize(section(from));
        for (idx = to; idx < from; ++idx) {
            tmp = sectionSize(section(idx));
            sections[idx+1].position = sections[idx].position + size;
            size = tmp;
        }
    }

    d->viewport->update();
    emit sectionIndexChanged(sec, from, to);
}

void QGenericHeader::resizeSection(int section, int size)
{
    int oldSize = sectionSize(section);
    if (oldSize == size)
        return;

    int diff = size - oldSize;
    int idx = index(section);
    QGenericHeaderPrivate::HeaderSection *sections = d->sections.data() + idx + 1;
    int num = d->sections.size() - idx - 1;

    while (num >= 4) {
        sections[0].position += diff;
        sections[1].position += diff;
        sections[2].position += diff;
        sections[3].position += diff;
        sections += 4;
        num -= 4;
    }
    if (num > 0) {
        sections[0].position += diff;
        if (num > 1) {
            sections[1].position += diff;
            if (num > 2) {
                sections[2].position += diff;
            }
        }
    }

    bool reverse = QApplication::reverseLayout();
    int pos = sectionPosition(section) - offset() - (reverse ? size : 0);
    QRect r;
    if (orientation() == Horizontal)
        r = QRect(pos, 0, width() - pos, height());
    else
        r = QRect(0, pos, width(), height() - pos);
    d->viewport->update(r.normalize());
    emit sectionSizeChanged(section, oldSize, size);
}

void QGenericHeader::hideSection(int section)
{
    resizeSection(section, 0); // FIXME: see below
    d->sections[index(section)].hidden = true;
}

void QGenericHeader::showSection(int section)
{
    d->sections[index(section)].hidden = false;
    resizeSection(section, orientation() == Horizontal ? default_width : default_height);
    // FIXME: when you show a section, you should get the section size bach
}

bool QGenericHeader::isSectionHidden(int section) const
{
    return d->sections[index(section)].hidden;
}

QModelIndex QGenericHeader::itemAt(int x, int y) const
{
    return (orientation() == Horizontal ?
            model()->index(0, sectionAt(x + offset()), 0, QModelIndex::HorizontalHeader) :
            model()->index(sectionAt(y + offset()), 0, 0, QModelIndex::VerticalHeader));
}

int QGenericHeader::horizontalOffset() const
{
    if (orientation() == Qt::Horizontal)
        return offset();
    return 0;
}

int QGenericHeader::verticalOffset() const
{
    if (orientation() == Qt::Vertical)
        return offset();
    return 0;
}

void QGenericHeader::autoScroll(int, int)
{
    QRect area = d->viewport->geometry();
    qDebug("QGenericHeader::autoScroll");
}

QModelIndex QGenericHeader::moveCursor(QAbstractItemView::CursorAction, ButtonState)
{
    return QModelIndex();
}

QRect QGenericHeader::itemViewportRect(const QModelIndex &item) const
{
    if (!item.isValid() || item.type() == QModelIndex::View)
        return QRect();
    if (orientation() == Horizontal)
        return QRect(sectionPosition(item.column()) - offset(), 0, sectionSize(item.column()), height());
    return QRect(0, sectionPosition(item.row()) - offset(), width(), sectionSize(item.row()));
}

QModelIndex QGenericHeader::item(int section) const
{
    if (orientation() == Horizontal)
        return model()->index(0, section, 0, QModelIndex::HorizontalHeader);
    return model()->index(section, 0, 0, QModelIndex::VerticalHeader);
}

QRect QGenericHeader::selectionViewportRect(const QItemSelection &selection) const
{
    QModelIndex bottomRight = model()->bottomRight(0);
    if (orientation() == Horizontal) {
        int left = bottomRight.column();
        int right = 0;
        int rangeLeft, rangeRight;

        for (int i = 0; i < selection.count(); ++i) {
            QItemSelectionRange r = selection.at(i);
            if (r.parent().isValid())
                continue; // we only know about toplevel items
            // FIXME an item inside the range may be the leftmost or rightmost
            rangeLeft = index(r.left());
            rangeRight = index(r.right());
            if (rangeLeft < left)
                left = rangeLeft;
            if (rangeRight > right)
                right = rangeRight;
        }

        int leftSec = section(left);
        int rightSec = section(right);

        int leftPos = sectionPosition(leftSec) - offset();
        int rightPos = sectionPosition(rightSec) + sectionSize(rightSec) - offset();

        return QRect(leftPos, 0, rightPos - leftPos, height());
    }
    // orientation() == Vertical
    int top = bottomRight.row();
    int bottom = 0;
    int rangeTop, rangeBottom;

    for (int i = 0; i < selection.count(); ++i) {
        QItemSelectionRange r = selection.at(i);
        if (r.parent().isValid())
            continue; // we only know about toplevel items
        // FIXME an item inside the range may be the leftmost or rightmost
        rangeTop = index(r.top());
        rangeBottom = index(r.bottom());
        if (rangeTop < top)
            top = rangeTop;
        if (rangeBottom > bottom)
            bottom = rangeBottom;
    }

    int topSec = section(top);
    int bottomSec = section(bottom);

    int topPos = sectionPosition(topSec) - offset();
    int bottomPos = sectionPosition(bottomSec) + sectionSize(bottomSec) - offset();

    return QRect(0, topPos, width(), bottomPos - topPos);
}

int QGenericHeader::count() const
{
    int c = d->sections.count();
    return c ? c - 1 : 0;
}

int QGenericHeader::index(int section) const
{
    if (d->indices.count() <= 0)
        return section; // nothing has been moved yet
    if (section < 0 || section >= d->indices.count())
        return 0;
    return d->indices.at(section);
}

int QGenericHeader::section(int index) const
{
    if (index < 0 || index >= d->sections.count())
        return 0;
    return d->sections.at(index).section;
}

void QGenericHeader::setMovable(bool movable)
{
    d->movableSections = movable;
}

bool QGenericHeader::isMovable() const
{
    return d->movableSections;
}

void QGenericHeader::setClickable(bool clickable)
{
    d->clickableSections = clickable;
}

bool QGenericHeader::isClickable() const
{
    return d->clickableSections;
}

void QGenericHeader::setResizeMode(ResizeMode mode)
{
    QGenericHeaderPrivate::HeaderSection *sections = d->sections.data();
    for (int i = 0; i < d->sections.count(); ++i)
        sections[i].mode = mode;
    d->stretchSections = (mode == Stretch ? count() : 0);
}

void QGenericHeader::setResizeMode(ResizeMode mode, int section)
{
    if (section >= d->sections.count())
        return;
    ResizeMode old = d->sections[index(section)].mode;
    d->sections[index(section)].mode = mode;
    if (mode == Stretch && old != Stretch)
        d->stretchSections++;
    // FIXME: handle this
}

QGenericHeader::ResizeMode QGenericHeader::resizeMode(int section) const
{
    if (section >= d->sections.count())
        return Interactive;
    return d->sections.at(index(section)).mode;
}

int QGenericHeader::stretchSectionCount() const
{
    return d->stretchSections;
}

void QGenericHeader::setSortIndicator(int section, SortOrder order)
{
    if (section != d->sortIndicatorSection)
        updateSection(d->sortIndicatorSection);
    d->sortIndicatorSection = section;
    d->sortIndicatorOrder = order;
    updateSection(section);
}

int QGenericHeader::sortIndicatorSection() const
{
    return d->sortIndicatorSection;
}

Qt::SortOrder QGenericHeader::sortIndicatorOrder() const
{
    return d->sortIndicatorOrder;
}

void QGenericHeader::updateGeometries()
{
    if (d->stretchSections)
        resizeSections();
}

int QGenericHeaderPrivate::sectionHandleAt(int position)
{
    int idx = q->indexAt(position);
    if (idx < 0)
        return -1;
    int sec = sections.at(idx).section;
    int pos = q->sectionPosition(sec);
    if (d->reverse()) {
        if (position < pos + 5)
            return sec;
        if (idx > 0 && position > pos + q->sectionSize(sec) - 5)
            return q->section(idx - 1);
    } else {
        if (idx > 0 && position < pos + 5)
            return q->section(idx - 1);
        if (position > pos + q->sectionSize(sec) - 5)
            return sec;
    }
    return -1;
}

void QGenericHeaderPrivate::setupSectionIndicator()
{
    if (!sectionIndicator) {
        sectionIndicator = new QWidget(q->d->viewport);
        sectionIndicator->setBackgroundRole(QPalette::Text);
    }
}

void QGenericHeaderPrivate::updateSectionIndicator()
{
    if (section == -1 || target == -1 ) {
        sectionIndicator->hide();
        return;
    }
    QRect geometry = sectionHandleRect(target);
    sectionIndicator->setGeometry(geometry);
    sectionIndicator->show();
}

QRect QGenericHeaderPrivate::sectionHandleRect(int section)
{
    QRect rect;
    static const int padding = 1;
    int size = 2 * padding + 1;
    int position = qMax(q->sectionPosition(section) - q->offset() - padding - 1, 0);
    if (q->orientation() == Qt::Horizontal)
        rect.setRect(position, 0, size, q->height());
    else
        rect.setRect(0, position, q->width(), size);
    return rect;
}
