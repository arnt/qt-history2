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

#include "qheaderview.h"
#include <private/qabstractitemview_p.h>
#include <qapplication.h>
#include <qbitarray.h>
#include <qdebug.h>
#include <qevent.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qvector.h>

class QHeaderViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QHeaderView)

public:
    QHeaderViewPrivate()
        : state(NoState),
          offset(0),
          sortIndicatorOrder(Qt::AscendingOrder),
          sortIndicatorSection(-1),
          movableSections(false),
          clickableSections(false),
          stretchSections(0),
          sectionIndicator(0) {}

    int sectionHandleAt(int pos);
    void setupSectionIndicator();
    void updateSectionIndicator();
    QRect sectionHandleRect(int section);

    inline bool reverse() const
        { return QApplication::reverseLayout() && orientation == Qt::Horizontal; }

    enum State { NoState, ResizeSection, MoveSection, SelectSection } state;

    int offset;
    Qt::Orientation orientation;
    Qt::SortOrder sortIndicatorOrder;
    int sortIndicatorSection;

    struct HeaderSection {
        int position;
        int section;
        uint hidden : 1;
        QHeaderView::ResizeMode mode;
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
    QStyleOptionHeader getStyleOption() const;
};

static const int border = 4;
static const int minimum = 15;
static const int default_width = 100;
static const int default_height = 30;

#define d d_func()
#define q q_func()

/*!
    \class QHeaderView qheaderview.h

    \brief The QHeaderView class provides a header row or header column for item views.

    \ingroup model-view

    A QHeaderView displays the headers used in item views such as the
    QGenericTableView and QGenericTreeView classes. It takes the place of
    the \c QHeader class previously used for the same purpose, but uses
    the Qt's model/view architecture for consistency with the item view
    classes.

    Each header has an orientation() and a number of sections, given by
    the count() function. Sections can be moved and resized using
    moveSection() and resizeSection(), and they can be hidden and shown
    with hideSection() and showSection().

    Each section of a header is described by a section ID, specified by
    its section(), and can be located at a particular index() in the
    header. A section can have a sort indicator set with
    setSortIndicator(); this indicates whether the items in the associated
    item view will be sorted in the order given by the section.

    A header can be fixed in place, or made movable with setMovable().
    It can be made clickable with setClickable(), and has resizing behavior
    in accordance with setResizeMode().

    A header emits sectionIndexChanged() if the user moves a section,
    sectionSizeChanged() if the user resizes a section, and
    sectionClicked() and sectionHandleDoubleClicked() in response to
    mouse clicks. A header also emits sectionCountChanged() and
    sectionAutoResize().

    You can identify a section by its ID, using the section() and sectionAt()
    functions, or by its index position, using the index() and indexAt()
    functions. Note that the index can change if a section is moved.

    \sa \link model-view-programming.html Model/View Programming\endlink
        QGenericListView QGenericTableView QGenericTreeView

*/

/*!
    \enum QHeaderView::ResizeMode

    \value Interactive Don't automatically change the size (let the
    user do it manually).
    \value Stretch Fill all the available visible space.
    \value Custom Don't automatically change the size here, leave it
    to some other object.
*/

/*!
    \fn void QHeaderView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)

    Selects the items in the given \a rect in accordance with the \a
    flags.

    The base class implementation does nothing.
*/

/*!
    \fn void QHeaderView::sectionIndexChanged(int section, int oldIndex, int newIndex);

    This signal is emitted when a section is moved. The section's
    number is specified by \a section, the old index position by \a
    oldIndex, and the new index position by \a newIndex.
*/

/*!
    \fn void QHeaderView::sectionSizeChanged(int section, int oldSize, int newSize);

    This signal is emitted when a section is resized. The section's
    number is specified by \a section, the old size by \a oldSize, and the
    new size by \a newSize.
*/

/*!
    \fn void QHeaderView::sectionClicked(int section, Qt::ButtonState state);

    This signal is emitted when a section is clicked. The section's
    number is specified by \a section, and the button by \a state.
*/

/*!
    \fn void QHeaderView::sectionCountChanged(int oldCount, int newCount);

    This signal is emitted when the number of sections changes; i.e.
    when sections are added or deleted. The original count is specified by
    \a oldCount, and the new count by \a newCount.
*/

/*!
    \fn void QHeaderView::sectionHandleDoubleClicked(int section, Qt::ButtonState state);

    This signal is emitted when a section is double-clicked. The
    section's number is specified by \a section, and the button by \a
    state.
*/

/*!
    \fn void QHeaderView::sectionAutoResize(int section, ResizeMode mode);

    This signal is emitted when a section is automatically resized.
    The section's number is specified by \a section, and the resize mode by
    \a mode.
*/


/*!
    Creates a new generic header with the given \a orientation and \a parent.
*/
QHeaderView::QHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QAbstractItemView(*new QHeaderViewPrivate, parent)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->orientation = orientation;
    setFrameStyle(NoFrame);
    d->viewport->setMouseTracking(true);
}

/*!
  \internal
*/
QHeaderView::QHeaderView(QHeaderViewPrivate &dd,
                               Qt::Orientation orientation, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->orientation = orientation;
    setFrameStyle(NoFrame);
    d->viewport->setMouseTracking(true);
}

/*!
  Destroys the header.
*/

QHeaderView::~QHeaderView()
{
}

/*!
  \reimp
*/
void QHeaderView::setModel(QAbstractItemModel *model)
{
    QAbstractItemView::setModel(model);
    if (!model)
        return;
    if (d->orientation == Qt::Horizontal) {
        QObject::disconnect(model, SIGNAL(columnsInserted(const QModelIndex&, int, int)),
                            this, SLOT(sectionsInserted(const QModelIndex&, int, int)));
        QObject::disconnect(model, SIGNAL(columnsRemoved(const QModelIndex&, int, int)),
                            this, SLOT(sectionsRemoved(const QModelIndex&, int, int)));
        QObject::connect(model, SIGNAL(columnsInserted(const QModelIndex&, int, int)),
                         this, SLOT(sectionsInserted(const QModelIndex&, int, int)));
        QObject::connect(model, SIGNAL(columnsRemoved(const QModelIndex&, int, int)),
                         this, SLOT(sectionsRemoved(const QModelIndex&, int, int)));
    } else {
        QObject::disconnect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                            this, SLOT(sectionsInserted(const QModelIndex&, int, int)));
        QObject::disconnect(model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                            this, SLOT(sectionsRemoved(const QModelIndex&, int, int)));
        QObject::connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                         this, SLOT(sectionsInserted(const QModelIndex&, int, int)));
        QObject::connect(model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                         this, SLOT(sectionsRemoved(const QModelIndex&, int, int)));
    }
    reset();
}

/*!
  Returns the orientation of the header.

  \sa Qt::Orientation
*/

Qt::Orientation QHeaderView::orientation() const
{
    return d->orientation;
}

/*!
    Returns the offset of the header: this is the header's left-most
    (or top-most for vertical headers) visible pixel.

    \sa setOffset()
*/

int QHeaderView::offset() const
{
    return d->reverse() ? -d->offset : d->offset;
}

/*!
    \fn void QHeaderView::setOffset(int offset)

    Sets the header's offset to \a offset.

    \sa offset()
*/

void QHeaderView::setOffset(int o)
{
    int ndelta = d->offset - o;
    d->offset = o;
    if (d->orientation == Qt::Horizontal)
        d->viewport->scroll(QApplication::reverseLayout() ? -ndelta : ndelta, 0);
    else
        d->viewport->scroll(0, ndelta);
}

/*!
*/

int QHeaderView::size() const
{
    if (d->sections.count())
        return d->sections.at(count()).position;
    return 0;
}

/*!
    Returns a suitable size hint for this header.

    \sa sectionSizeHint()
*/

QSize QHeaderView::sizeHint() const
{
    if (d->sections.isEmpty() || !isVisible())
            return QSize();
    QStyleOptionViewItem option = viewOptions();
    int row = orientation() == Qt::Horizontal ? 0 : section(count() - 1);
    int col = orientation() == Qt::Horizontal ? section(count() - 1) : 0;
    QModelIndex::Type type = orientation() == Qt::Horizontal
                             ? QModelIndex::HorizontalHeader
                             : QModelIndex::VerticalHeader;
    QModelIndex index = model()->index(row, col, QModelIndex(), type);
    if (!index.isValid())
        return QSize();
    QSize hint = itemDelegate()->sizeHint(fontMetrics(), option, model(), index);
    if (orientation() == Qt::Vertical)
        return QSize(hint.width() + border, 192);
    return QSize(256, hint.height() + border);
}

/*!
    Returns a suitable size hint for the given \a section.

    \sa sizeHint()
*/

int QHeaderView::sectionSizeHint(int section) const
{
    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    int hint = 0;
    int row = orientation() == Qt::Vertical ? section : 0;
    int col = orientation() == Qt::Vertical ? 0 : section;
    QModelIndex::Type type = orientation() == Qt::Horizontal ?
                             QModelIndex::HorizontalHeader :
                             QModelIndex::VerticalHeader;
    QModelIndex header = model()->index(row, col, QModelIndex(), type);
    if (orientation() == Qt::Vertical) {
        QSize size = delegate->sizeHint(fontMetrics(), option, model(), header);
        hint = size.height();
        if (sortIndicatorSection() == section)
            hint += size.width();
    } else {
        QSize size = delegate->sizeHint(fontMetrics(), option, model(), header);
        hint = size.width();
        if (sortIndicatorSection() == section)
            hint += size.height();
    }

    return hint + border;
}

/*!
  \reimp
*/

void QHeaderView::paintEvent(QPaintEvent *e)
{
    QStyleOptionViewItem option = viewOptions();
    QPainter painter(d->viewport);
    QRect area = e->rect();

    int offset = this->offset();
    int start, end;
    if (orientation() == Qt::Horizontal) {
        start = indexAt(offset + area.left());
        end = indexAt(offset + area.right() - 1);
    } else {
        start = indexAt(offset + area.top());
        end = indexAt(offset + area.bottom() - 1);
    }

    start = start == -1 ? 0 : start;
    end = end == -1 ? count() - 1 : end;

    int tmp = start;
    start = qMin(start, end);
    end = qMax(tmp, end);

    QModelIndex index;
    if (d->sections.isEmpty())
        return;
    const QHeaderViewPrivate::HeaderSection *sections = d->sections.constData();
    QModelIndex current = selectionModel()->currentItem();

    int section;
    int width = d->viewport->width();
    int height = d->viewport->height();
    if (d->orientation == Qt::Horizontal) {
        for (int i = start; i <= end; ++i) {
            if (sections[i].hidden)
                continue;
            section = sections[i].section;
            index = model()->index(0, section, QModelIndex(), QModelIndex::HorizontalHeader);
            if (!index.isValid())
                continue;
            option.font.setBold(index.column() == current.column());
            option.rect.setRect(sectionPosition(section) - offset, 0, sectionSize(section), height);
            paintSection(&painter, option, index);
        }
        if (option.rect.right() < area.right()) {
            QStyleOptionHeader opt = d->getStyleOption();
            opt.state |= QStyle::Style_Off | QStyle::Style_Raised;
            opt.rect.setRect(option.rect.right() + 1, 0, width - option.rect.right() - 1, height);
            style().drawPrimitive(QStyle::PE_HeaderSection, &opt, &painter, this);
        }
    } else {
        for (int i = start; i <= end; ++i) {
            if (sections[i].hidden)
                continue;
            section = sections[i].section;
            index = model()->index(section, 0, QModelIndex(), QModelIndex::VerticalHeader);
            if (!index.isValid())
                continue;
            option.font.setBold(index.row() == current.row());
            option.rect.setRect(0, sectionPosition(section) - offset, width, sectionSize(section));
            paintSection(&painter, option, index);
        }
        if (option.rect.bottom() < area.bottom()) {
            QStyleOptionHeader opt = d->getStyleOption();
            opt.state |= QStyle::Style_Off | QStyle::Style_Raised;
            opt.rect.setRect(0, option.rect.bottom() + 1, width, height - option.rect.bottom() - 1);
            style().drawPrimitive(QStyle::PE_HeaderSection, &opt, &painter, this);
        }
    }
}

/*!
    Paints the section using the given \a painter with the \a option
    and model \a index.

    You normally would not need to use this function.
*/

void QHeaderView::paintSection(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index)
{
    QStyleOptionHeader opt = d->getStyleOption();
    QStyle::SFlags arrowFlags = QStyle::Style_Off;
    opt.rect = option.rect;
    opt.section = index.column();
    if (d->clickableSections && (d->orientation == Qt::Horizontal ?
          selectionModel()->isColumnSelected(index.column(), model()->parent(index)) :
          selectionModel()->isRowSelected(index.row(), model()->parent(index))))
        opt.state |= QStyle::Style_Down;
    else
        opt.state |= QStyle::Style_Raised;
    style().drawPrimitive(QStyle::PE_HeaderSection, &opt, painter, this);
#if 1
    itemDelegate()->paint(painter, option, model(), index);
#else
    opt.text = d->model->data(index, QAbstractItemModel::DisplayRole).toString();
    opt.icon = d->model->data(index, QAbstractItemModel::DecorationRole).toIconSet();
    style().drawControl(QStyle::CE_HeaderLabel, &opt, painter, this);
#endif

    int section = orientation() == Qt::Horizontal ? index.column() : index.row();
    if (sortIndicatorSection() == section) {
        //bool alignRight = style().styleHint(QStyle::SH_Header_ArrowAlignment, this) & Qt::AlignRight;
        // FIXME: use alignRight and RTL
        int h = option.rect.height();
        int x = option.rect.x();
        int y = option.rect.y();
        int secSize = sectionSize(section);
        if (d->orientation == Qt::Horizontal)
            opt.rect.setRect(x + secSize - border * 2 - (h / 2), y + 5, h / 2, h - border * 2);
        else
            opt.rect.setRect(x + 5, y + secSize - h, h / 2, h - border * 2);
        arrowFlags |= (sortIndicatorOrder() == Qt::AscendingOrder
                       ? QStyle::Style_Down : QStyle::Style_Up);
        opt.state = arrowFlags;
        style().drawPrimitive(QStyle::PE_HeaderArrow, &opt, painter, this);
    }
}

/*!
    Returns the index of the section that covers the given \a position.
*/

int QHeaderView::indexAt(int position) const
{
    if (count() < 1)
        return -1;

    if (d->reverse())
        position = size() - position;

    int start = 0;
    int end = count() - 1;
    int idx = (end + 1) / 2;

    const QHeaderViewPrivate::HeaderSection *sections = d->sections.constData();

    while (end - start > 0) {
        if (sections[idx].position > position)
            end = idx - 1;
        else
            start = idx;
        idx = (start + end + 1) / 2;
    }

    if (idx == end && sections[end + 1].position < position)
        return -1;

    return idx;
}

/*!
    Returns the section that covers the given \a position.
*/

int QHeaderView::sectionAt(int position) const
{
    int idx = indexAt(position);
    if (idx < 0 || idx >= d->sections.count())
        return -1;
    return d->sections.at(idx).section;
}

/*!
    Returns the width (or height for vertical headers) of the given \a
    section.
*/

int QHeaderView::sectionSize(int section) const
{
    if (section < 0 || section >= d->sections.count() - 1 || isSectionHidden(section))
        return 0;
    int idx = index(section);
    return d->sections.at(idx + 1).position - d->sections.at(idx).position;
}

/*!
    Returns the index position of the given \a section.
*/

int QHeaderView::sectionPosition(int section) const
{
    if (section < 0 || section >= d->sections.count())
        return 0;
    if (d->reverse())
        return size() - d->sections.at(index(section)).position - sectionSize(section);
    return d->sections.at(index(section)).position;
}

/*!
    \internal
*/

void QHeaderView::reset()
{
    if (d->orientation == Qt::Horizontal)
        initializeSections(0, d->model->columnCount(root()) - 1);
    else
        initializeSections(0, d->model->rowCount(root()) - 1);
}

/*!
    \internal
*/

void QHeaderView::initializeSections(int start, int end)
{
    int oldCount = count();
    end += 1; // one past the last item, so we get the end position of the last section
    d->sections.resize(end + 1);
    if (oldCount >= count()) {
        d->viewport->update();
        emit sectionCountChanged(oldCount, count());
        return;
    }

    int pos = (start <= 0 ? 0 : d->sections.at(start).position);
    QHeaderViewPrivate::HeaderSection *sections = d->sections.data() + start;
    int s = start;
    int num = end - start + 1;
    int size = orientation() == Qt::Horizontal ? default_width : default_height;;

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

/*!
  \reimp
*/

void QHeaderView::currentChanged(const QModelIndex &old, const QModelIndex &current)
{
    QRect oldRect, currentRect;
    if (d->orientation == Qt::Horizontal) {
        oldRect = QRect(sectionPosition(old.column()) - offset(), 0,
                        sectionSize(old.column()), height());
        currentRect = QRect(sectionPosition(current.column()) - offset(), 0,
                            sectionSize(current.column()), height());
    } else {
        oldRect = QRect(0, sectionPosition(old.row()) - offset(),
                        width(), sectionSize(old.row()));
        currentRect = QRect(0, sectionPosition(current.row()) - offset(),
                            width(), sectionSize(current.row()));
    }
    d->viewport->repaint(oldRect|currentRect);
}


/*!
    This slot is called when sections are inserted into the \a parent
    model; \a first and \a last signify where the new sections are
    inserted. (\a first and \a last will be the same if just one
    section is inserted.)
*/

void QHeaderView::sectionsInserted(const QModelIndex &parent, int first, int)
{
    if (parent != root())
        return; // we only handle changes in the top level
    if (d->orientation == Qt::Horizontal)
        initializeSections(first, d->model->columnCount(root()) - 1);
    else
        initializeSections(first, d->model->rowCount(root()) - 1);
}

/*!
    This slot is called when sections are removed from the \a parent
    model; \a first and \a last signify where the sections are removed
    from. (\a first and \a last will be the same if just one section
    is removed.)
*/

void QHeaderView::sectionsRemoved(const QModelIndex &parent, int first, int last)
{
    if (parent != root())
        return; // we only handle changes in the top level
    // the sections have not been removed from the model yet
    int count = last - first + 1;
    if (d->orientation == Qt::Horizontal)
        initializeSections(first, d->model->columnCount(root()) - count - 1);
    else
        initializeSections(first, d->model->rowCount(root()) - count - 1);
}

/*!
    \internal

    This makes sure that the item at position \a index is visible in
    the view. You should not normally need to call this function.
*/

void QHeaderView::ensureItemVisible(const QModelIndex &)
{
    // do nothing - this should be handled by the parent view
}

/*!
    \internal

    Updates the given \a section.
*/

void QHeaderView::updateSection(int section)
{
    if (orientation() == Qt::Horizontal)
        d->viewport->update(QRect(sectionPosition(section) - offset(),
                                  0, sectionSize(section), height()));
    else
        d->viewport->update(QRect(0, sectionPosition(section) - offset(),
                                  width(), sectionSize(section)));
}

/*!
    \internal

    Resizes the sections according to their size hints. You should not
    normally need to call this function.
*/

void QHeaderView::resizeSections()
{
    ResizeMode mode;
    int secSize = 0;
    int stretchSecs = 0;
    int stretchSize = orientation() == Qt::Horizontal ? d->viewport->width() : d->viewport->height();
    QList<int> section_sizes;
    int count = qMax(d->sections.count() - 1, 0);
    QHeaderViewPrivate::HeaderSection *secs = d->sections.data();
    for (int i = 0; i < count; ++i) {
        mode = secs[i].mode;
        if (mode == Stretch) {
            ++stretchSecs;
            continue;
        }
        if (mode == Interactive) {
            secSize = sectionSize(secs[i].section);
        } else {// mode == QHeaderView::Custom
            // FIXME: get the size of the section from the contents;  this is just a temprary solution
            QAbstractItemView *par = ::qt_cast<QAbstractItemView*>(parent());
            if (par)
                secSize = (orientation() == Qt::Horizontal
                           ? par->columnSizeHint(i) : par->rowSizeHint(i));
            secSize = qMax(secSize, sectionSizeHint(secs[i].section));
        }
        section_sizes.append(secSize);
        stretchSize -= secSize;
    }
    int position = 0;
    int stretchSectionSize = qMax(stretchSecs > 0 ? stretchSize / stretchSecs : 0, minimum);
    for (int i = 0; i < count; ++i) {
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

/*!
  \reimp
*/

void QHeaderView::mousePressEvent(QMouseEvent *e)
{
    int pos = orientation() == Qt::Horizontal ? e->x() : e->y();
    if (e->state() & Qt::ControlButton && d->movableSections) {
        d->section = d->target = sectionAt(pos + offset());
        if (d->section == -1)
            return;
        d->state = QHeaderViewPrivate::MoveSection;
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
            d->state = QHeaderViewPrivate::ResizeSection;
            d->lastPos = (orientation() == Qt::Horizontal ? e->x() : e->y());
            d->section = handle;
        }
    }
}

/*!
  \reimp
*/

void QHeaderView::mouseMoveEvent(QMouseEvent *e)
{
    int pos = orientation() == Qt::Horizontal ? e->x() : e->y();

    switch (d->state) {
        case QHeaderViewPrivate::ResizeSection: {
            int delta = d->reverse() ? d->lastPos - pos : pos - d->lastPos;
            int size = sectionSize(d->section) + delta;
            if (size > minimum) {
                resizeSection(d->section, size);
                d->lastPos = (orientation() == Qt::Horizontal ? e->x() : e->y());
            }
            return;
        }
        case QHeaderViewPrivate::MoveSection: {
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
        case QHeaderViewPrivate::NoState: {
            int handle = d->sectionHandleAt(pos + offset());
            if (handle != -1 && resizeMode(handle) == Interactive)
                setCursor(orientation() == Qt::Horizontal ? Qt::SplitHCursor : Qt::SplitVCursor);
            else
                setCursor(Qt::ArrowCursor);
            return;
        }

        case QHeaderViewPrivate::SelectSection:
            //qDebug("SelectSection is not implemented");
            break; // FIXME: not implemented
    }
}

/*!
  \reimp
*/

void QHeaderView::mouseReleaseEvent(QMouseEvent *)
{
    switch (d->state) {
    case QHeaderViewPrivate::MoveSection:
        moveSection(index(d->section), index(d->target));
        d->section = d->target = -1;
        d->updateSectionIndicator();
        break;
    case QHeaderViewPrivate::NoState:
        break;
    case QHeaderViewPrivate::ResizeSection:
        break;
    case QHeaderViewPrivate::SelectSection:
        qDebug("SelectSection is not implemented");
        break;
    }
    d->state = QHeaderViewPrivate::NoState;
}

/*!
  \reimp
*/

void QHeaderView::mouseDoubleClickEvent(QMouseEvent *e)
{
    int pos = orientation() == Qt::Horizontal ? e->x() : e->y();
    int handle = d->sectionHandleAt(pos + offset());
    while (handle > -1 && isSectionHidden(handle)) handle--;
    if (handle > -1 && resizeMode(handle) == Interactive)
        emit sectionHandleDoubleClicked(handle, e->state());
}

/*!
    Moves the section at index position \a from to occupy index
    position \a to.
*/

void QHeaderView::moveSection(int from, int to)
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
    QHeaderViewPrivate::HeaderSection *sections = d->sections.data();
    int *indices = d->indices.data();
    int sec = sections[from].section;
    int idx = from;
    if (to > from) {
        while (idx < to) {
            sections[idx].section = sections[idx + 1].section;
            indices[sections[idx].section] = idx++;
        }
    } else {
        while (idx > to) {
            sections[idx].section = sections[idx - 1].section;
            indices[sections[idx].section] = idx--;
        }
    }
    sections[to].section = sec;
    indices[sec] = to;

    // move positions
    if (to > from) {
        for (idx = from; idx < to; ++idx)
            sections[idx+1].position -= sectionSize(section(idx))
                                        - sectionSize(section(idx + 1));
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

/*!
    \internal

    Resizes the given \a section to the given \a size.
*/

void QHeaderView::resizeSection(int section, int size)
{
    int oldSize = sectionSize(section);
    if (oldSize == size)
        return;

    int diff = size - oldSize;
    int idx = index(section);
    QHeaderViewPrivate::HeaderSection *sections = d->sections.data() + idx + 1;
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

    int w = d->viewport->width();
    int h = d->viewport->height();
    int pos = sectionPosition(section) - offset();
    if (d->reverse())
        pos -= size;
    QRect r;
    if (orientation() == Qt::Horizontal)
        r.setRect(pos, 0, w - pos, h);
    else
        r.setRect(0, pos, w, h - pos);
    if (d->stretchSections) {
        resizeSections();
        r = d->viewport->rect();
    }
    d->viewport->update(r.normalize());
    emit sectionSizeChanged(section, oldSize, size);
}

/*!
    Hides the specified \a section.

    \sa showSection()
*/

void QHeaderView::hideSection(int section)
{
    resizeSection(section, 0);
    d->sections[index(section)].hidden = true;
}

/*!
   Shows the specified \a section.

   \sa hideSection()
*/

void QHeaderView::showSection(int section)
{
    int i = index(section);
    d->sections[i].hidden = false;
    resizeSection(section, orientation() == Qt::Horizontal ? default_width : default_height);
    // FIXME: when you show a section, you should get the old section size bach
}

/*!
    Returns true if the \a section is hidden from the user; otherwise
    returns false.
*/

bool QHeaderView::isSectionHidden(int section) const
{
    int i = index(section);
    return d->sections.at(i).hidden;
}

/*!
    Returns the number of the section at position (\a x, \a y) in
    contents coordinates..
*/

QModelIndex QHeaderView::itemAt(int x, int y) const
{
    return (orientation() == Qt::Horizontal ?
            model()->index(0, sectionAt(x + offset()), QModelIndex(), QModelIndex::HorizontalHeader) :
            model()->index(sectionAt(y + offset()), 0, QModelIndex(), QModelIndex::VerticalHeader));
}

/*!
    \internal

    Resizes the sections in the header.
*/

void QHeaderView::doItemsLayout()
{
    if (d->stretchSections)
        resizeSections();
}

/*!
    Returns the horizontal offset of the header. This is 0 for
    vertical headers.

    \sa offset()
*/

int QHeaderView::horizontalOffset() const
{
    if (orientation() == Qt::Horizontal)
        return offset();
    return 0;
}

/*!
    Returns the vertical offset of the header. This is 0 for
    horizontal headers.

    \sa offset()
*/

int QHeaderView::verticalOffset() const
{
    if (orientation() == Qt::Vertical)
        return offset();
    return 0;
}

/*!
    \internal
*/

QModelIndex QHeaderView::moveCursor(QAbstractItemView::CursorAction, Qt::ButtonState)
{
    return QModelIndex();
}

/*!
    \internal
*/

QRect QHeaderView::itemViewportRect(const QModelIndex &index) const
{
    if (!index.isValid() || index.type() == QModelIndex::View)
        return QRect();
    if (orientation() == Qt::Horizontal)
        return QRect(sectionPosition(index.column()) - offset(),
                     0, sectionSize(index.column()), height());
    return QRect(0, sectionPosition(index.row()) - offset(),
                 width(), sectionSize(index.row()));
}

/*!
    Returns the model item index for the item in the given \a section.
*/

QModelIndex QHeaderView::item(int section) const
{
    if (orientation() == Qt::Horizontal)
        return model()->index(0, section, QModelIndex(), QModelIndex::HorizontalHeader);
    return model()->index(section, 0, QModelIndex(), QModelIndex::VerticalHeader);
}

/*!
    \internal
*/

QRect QHeaderView::selectionViewportRect(const QItemSelection &selection) const
{
    if (orientation() == Qt::Horizontal) {
        int left = d->model->columnCount(root()) - 1;
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
    // orientation() == Qt::Vertical
    int top = d->model->rowCount(root()) - 1;
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

/*!
    Returns the number of sections in the header.
*/

int QHeaderView::count() const
{
    int c = d->sections.count();
    return c ? c - 1 : 0;
}

/*!
    Returns the index position of the given \a section.

    \sa section()
*/

int QHeaderView::index(int section) const
{
    if (d->indices.count() <= 0)
        return section; // nothing has been moved yet
    if (section < 0 || section >= d->indices.count())
        return 0;
    return d->indices.at(section);
}

/*!
    Returns the number of the section at the given \a index position.

    \sa index()
*/

int QHeaderView::section(int index) const
{
    if (index < 0 || index >= d->sections.count())
        return 0;
    return d->sections.at(index).section;
}

/*!
    If \a movable is true, the header may be moved by the user;
    otherwise it is fixed in place.

    \sa isMovable()
*/

void QHeaderView::setMovable(bool movable)
{
    d->movableSections = movable;
}

/*!
    Returns true if the header can be moved by the user; otherwise
    returns false.

    \sa setMovable()
*/

bool QHeaderView::isMovable() const
{
    return d->movableSections;
}

/*!
    If \a clickable is true, the header will respond to single clicks.

    \sa isClickable()
*/

void QHeaderView::setClickable(bool clickable)
{
    d->clickableSections = clickable;
}

/*!
    Returns true if the header is clickable; otherwise returns false.
    A clickable header could be set up to allow the user to change the
    representation of the data in the view related to the header.

    \sa setClickable()
*/

bool QHeaderView::isClickable() const
{
    return d->clickableSections;
}

/*!
    Sets the constraints on how the header can be resized to those
    described by the given \a mode.

    \sa QLayout::ResizeMode
*/

void QHeaderView::setResizeMode(ResizeMode mode)
{
    QHeaderViewPrivate::HeaderSection *sections = d->sections.data();
    for (int i = 0; i < d->sections.count(); ++i)
        sections[i].mode = mode;
    d->stretchSections = (mode == Stretch ? count() : 0);
}

/*!
    \overload

    Sets the constraints on how the \a section in the header can be
    resized to those described by the given \a mode.

    \sa QLayout::ResizeMode
*/

void QHeaderView::setResizeMode(ResizeMode mode, int section)
{
    if (section >= d->sections.count()) {
        qWarning("setResizeMode: section %d does not exist", section);
        return;
    }
    ResizeMode old = d->sections[index(section)].mode;
    d->sections[index(section)].mode = mode;
    if (mode == Stretch && old != Stretch)
        d->stretchSections++;
}

/*!
    Returns the resize mode that applies to the given \a section.
*/

QHeaderView::ResizeMode QHeaderView::resizeMode(int section) const
{
    if (section >= d->sections.count())
        return Interactive;
    return d->sections.at(index(section)).mode;
}

/*!
    \internal
*/

int QHeaderView::stretchSectionCount() const
{
    return d->stretchSections;
}

/*!
    Sets the sort indicator for the given \a section in the direction
    specified by \a order, and removes the sort indicator from any
    other section that was showing it.

    \sa sortIndicatorSection() sortIndicatorOrder()
*/

void QHeaderView::setSortIndicator(int section, Qt::SortOrder order)
{
    int old = d->sortIndicatorSection;
    d->sortIndicatorSection = section;
    d->sortIndicatorOrder = order;

    if (d->sections.at(section).mode == Custom
        || (old > -1 && d->sections.at(old).mode == Custom)) {
        resizeSections();
        d->viewport->update();
    } else {
        if (old > -1 && old != section)
            updateSection(old);
        updateSection(section);
    }
}

// ### DOC: I made up the -1 case...
/*!
    Returns the number of the section that has a sort indicator, or -1
    if no section has a sort indicator.

    \sa setSortIndicator() sortIndicatorOrder()
*/

int QHeaderView::sortIndicatorSection() const
{
    return d->sortIndicatorSection;
}

/*!
    Returns the order for the sort indicator. If no section has a sort
    indicator the return value of this function is undefined.

    \sa setSortIndicator() sortIndicatorSection()
*/

Qt::SortOrder QHeaderView::sortIndicatorOrder() const
{
    return d->sortIndicatorOrder;
}

/*!
    \internal
*/

void QHeaderView::updateGeometries()
{
    doItemsLayout();
}

int QHeaderViewPrivate::sectionHandleAt(int position)
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

void QHeaderViewPrivate::setupSectionIndicator()
{
    if (!sectionIndicator) {
        sectionIndicator = new QWidget(q->d->viewport);
        sectionIndicator->setBackgroundRole(QPalette::Text);
    }
}

void QHeaderViewPrivate::updateSectionIndicator()
{
    if (section == -1 || target == -1 ) {
        sectionIndicator->hide();
        return;
    }
    QRect geometry = sectionHandleRect(target);
    sectionIndicator->setGeometry(geometry);
    sectionIndicator->show();
}

QRect QHeaderViewPrivate::sectionHandleRect(int section)
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

QStyleOptionHeader QHeaderViewPrivate::getStyleOption() const
{
    QStyleOptionHeader opt(0);
    opt.rect = q->rect();
    opt.palette = q->palette();
    opt.state = QStyle::Style_Default;
    if (orientation == Qt::Horizontal)
        opt.state |= QStyle::Style_Horizontal;
    if (q->isEnabled())
        opt.state |= QStyle::Style_Enabled;
    opt.section = 0;
    return opt;
}
