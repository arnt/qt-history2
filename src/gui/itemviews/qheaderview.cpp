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
#include <private/qheaderview_p.h>
#include <qbitarray.h>
#include <qdebug.h>
#include <qevent.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qvector.h>

#define d d_func()
#define q q_func()

/*!
    \class QHeaderView qheaderview.h

    \brief The QHeaderView class provides a header row or header column for item views.

    \ingroup model-view
    \mainclass

    A QHeaderView displays the headers used in item views such as the
    QTableView and QTreeView classes. It takes the place of
    the \c QHeader class previously used for the same purpose, but uses
    the Qt's model/view architecture for consistency with the item view
    classes.

    Each header has an orientation() and a number of sections, given by
    the count() function. Sections can be moved and resized using
    moveSection() and resizeSection(); they can be hidden and shown
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

    You can identify a section using the section() and sectionAt()
    functions, or by its index position, using the index() and indexAt()
    functions. Note that the index can change if a section is moved.

    For a horizontal header the section is equivalent to a column in the
    model, and for a vertical header the section is equivalent to a row
    in the model.

    \sa \link model-view-programming.html Model/View Programming\endlink QListView QTableView QTreeView QHeaderWidget

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
  \fn void QHeaderView::sectionPressed(int section, Qt::ButtonState state);

  This signal is emitted when a section is pressed. The section's
  number is specified by \a section, and the button by \a state.
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
    \property QHeaderView::highlightCurrentSection
    \brief whether the current selection is highlighted
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
    if (d->orientation == Qt::Horizontal) {
        QObject::disconnect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                            this, SLOT(sectionsInserted(QModelIndex,int,int)));
        QObject::disconnect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                            this, SLOT(sectionsRemoved(QModelIndex,int,int)));
        QObject::connect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                         this, SLOT(sectionsInserted(QModelIndex,int,int)));
        QObject::connect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                         this, SLOT(sectionsRemoved(QModelIndex,int,int)));
    } else {
        QObject::disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                            this, SLOT(sectionsInserted(QModelIndex,int,int)));
        QObject::disconnect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                            this, SLOT(sectionsRemoved(QModelIndex,int,int)));
        QObject::connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                         this, SLOT(sectionsInserted(QModelIndex,int,int)));
        QObject::connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                         this, SLOT(sectionsRemoved(QModelIndex,int,int)));
    }
    QObject::disconnect(d->model, SIGNAL(headerDataChanged(Orientation,int,int)),
                        this, SLOT(headerDataChanged(Orientation,int,int)));
    QObject::connect(d->model, SIGNAL(headerDataChanged(Orientation,int,int)),
                     this, SLOT(headerDataChanged(Orientation,int,int)));
    QAbstractItemView::setModel(model);
    // Users want to set sizes and modes before the widget is shown.
    // Thus, we have to initialize when the model is set,
    // and not lazily like we do in the other views.
    initializeSections();
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
  Returns the length along the orientation of the header.

  \sa sizeHint()
*/

int QHeaderView::length() const
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
    if (d->sections.isEmpty())
        return QSize();
    QSize hint = sectionSizeFromContents(section(count() - 1));
    // FIXME: we should check all sections
    return QSize(hint.width(), hint.height());
}

/*!
    Returns a suitable size hint for the given \a section.

    \sa sizeHint()
*/

int QHeaderView::sectionSizeHint(int section) const
{
    QSize size = sectionSizeFromContents(section);
    return orientation() == Qt::Horizontal ? size.width() : size.height();
}

/*!
    Returns the index of the section that covers the given \a position.
*/

int QHeaderView::indexAt(int position) const
{
    if (count() < 1)
        return -1;

    if (d->reverse())
        position = length() - position;

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
        return length() - d->sections.at(index(section)).position - sectionSize(section);
    return d->sections.at(index(section)).position;
}

/*!
  \fn QHeaderView::sectionAt(int x, int y) const

  Returns the section at the given coordinate.
  If the header is horizontal \a x will be used, otherwise \a y
  will be used to find the section.
*/

/*!
  \fn QHeaderView::sectionAt(const QPoint &pos) const

  Returns the section at the position given in \a pos.
  If the header is horizontal \a x will be used, otherwise \a y
  will be used to find the section.
*/

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
            sections[idx + 1].position -= sectionSize(section(idx))
                                          - sectionSize(section(idx + 1));
    } else {
        int tmp;
        int size = sectionSize(section(from));
        for (idx = to; idx < from; ++idx) {
            tmp = sectionSize(section(idx));
            sections[idx + 1].position = sections[idx].position + size;
            size = tmp;
        }
    }

    d->viewport->update();
    emit sectionIndexChanged(sec, from, to);
}

/*!
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
  \fn void QHeaderView::hideSection(int section)
    Hides the specified \a section.

    \sa showSection()
*/

/*!
  \fn void QHeaderView::showSection(int section)
   Shows the specified \a section.

   \sa hideSection()
*/

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
  If \a hide is true the \a section is hidden, otherwise the \a section is shown.
*/

void QHeaderView::setSectionHidden(int section, bool hide)
{
    if (hide) {
        resizeSection(section, 0);
        d->sections[index(section)].hidden = true;
    } else {
        d->sections[index(section)].hidden = false;
        resizeSection(section, orientation() == Qt::Horizontal ? default_width : default_height);
        // FIXME: when you show a section, you should get the old section size bach
    }
}

/*!
    Returns the number of sections in the header.
*/

int QHeaderView::count() const
{
    int c = d->sections.count();
    return c > 0 ? c - 1 : 0;
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

void QHeaderView::setHighlightCurrentSection(bool highlight)
{
    d->highlightCurrent = highlight;
}

bool QHeaderView::highlightCurrentSection() const
{
    return d->highlightCurrent;
}

/*!
    Sets the constraints on how the header can be resized to those
    described by the given \a mode.

    \sa QLayout::ResizeMode
*/

void QHeaderView::setResizeMode(ResizeMode mode)
{
    initializeSections();
    QHeaderViewPrivate::HeaderSection *sections = d->sections.data();
    for (int i = 0; i < d->sections.count(); ++i)
        sections[i].mode = mode;
    d->stretchSections = (mode == Stretch ? count() : 0);
    d->globalResizeMode = mode;
}

/*!
    \overload

    Sets the constraints on how the \a section in the header can be
    resized to those described by the given \a mode.

    \sa QLayout::ResizeMode
*/

void QHeaderView::setResizeMode(ResizeMode mode, int section)
{
    initializeSections();
    if (section >= d->sections.count()) {
        qWarning("setResizeMode: section %d does not exist", section);
        return;
    }
    ResizeMode old = d->sections[index(section)].mode;
    d->sections[index(section)].mode = mode;
    if (mode == Stretch && old != Stretch)
        ++d->stretchSections;
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
  \property QHeaderView::showSortIndicator
  \brief whether the sort indicator is shown
*/

void QHeaderView::setSortIndicatorShown(bool show)
{
    d->sortIndicatorShown = show;
    if (d->sections.at(sortIndicatorSection()).mode == Custom) {
        resizeSections();
        d->viewport->update();
    }
}

bool QHeaderView::isSortIndicatorShown() const
{
    return d->sortIndicatorShown;
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

    if (section >= d->sections.count())
        return; // nothing to do

    if (old != section && d->sections.at(section).mode == Custom) {
        resizeSections();
        d->viewport->update();
    } else {
        if (old != section)
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

void QHeaderView::doItemsLayout()
{
    initializeSections();
    QAbstractItemView::doItemsLayout();
}

/*!
  Updates the changed header sections
*/
void QHeaderView::headerDataChanged(Qt::Orientation orientation, int first, int last)
{
    if (d->orientation != orientation)
        return;
    if (orientation == Qt::Horizontal) {
        int left = sectionPosition(first);
        int right = sectionPosition(last) + sectionSize(last);
        d->viewport->update(left, 0, right - left, d->viewport->height());
    } else {
        int top = sectionPosition(first);
        int bottom = sectionPosition(last) + sectionSize(last);
        d->viewport->update(0, top, d->viewport->width(), bottom - top);
    }
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
            if (orientation() == Qt::Horizontal) {
                if (par)
                    secSize = par->columnSizeHint(i);
                secSize = qMax(secSize, sectionSizeHint(secs[i].section));
            } else {
                if (par)
                    secSize = par->rowSizeHint(i);
                secSize = qMax(secSize, sectionSizeHint(secs[i].section));
            }
        }
        section_sizes.append(secSize);
        stretchSize -= secSize;
    }
    int position = 0;
    QSize strut = QApplication::globalStrut();
    int minimum = orientation() == Qt::Horizontal ? strut.width() : strut.height();
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
    This slot is called when sections are inserted into the \a parent,
    \a first and \a last signify where the new sections are
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
    This slot is called when sections are removed from the \a parent,
    \a first and \a last signify where the sections are removed
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
*/

void QHeaderView::initializeSections()
{
    if (d->orientation == Qt::Horizontal) {
        int c = d->model->columnCount(root());
        if (c != count())
            initializeSections(0, c > 0 ? c - 1 : 0);
    } else {
        int r = d->model->rowCount(root());
        if (r != count())
            initializeSections(0, r > 0 ? r - 1 : 0);
    }
}

/*!
    \internal
*/

void QHeaderView::initializeSections(int start, int end)
{
    int oldCount = count();
    end += 1; // one past the last item, so we get the end position of the last section
    d->sections.resize(end + 1);

    int pos = (start > 0 ? d->sections.at(start).position : 0);
    QHeaderViewPrivate::HeaderSection *sections = d->sections.data() + start;
    int s = start;
    int num = end - start + 1;
    int size = orientation() == Qt::Horizontal ? default_width : default_height;

    // set resize mode
    ResizeMode mode = d->globalResizeMode;
    if (mode == Stretch)
        d->stretchSections += num;

    // unroll loop - to initialize the arrays as fast as possible
    while (num >= 4) {

        sections[0].hidden = false;
        sections[0].mode = mode;
        sections[0].section = s++;
        sections[0].position = pos;
        pos += size;

        sections[1].hidden = false;
        sections[1].mode = mode;
        sections[1].section = s++;
        sections[1].position = pos;
        pos += size;

        sections[2].hidden = false;
        sections[2].mode = mode;
        sections[2].section = s++;
        sections[2].position = pos;
        pos += size;

        sections[3].hidden = false;
        sections[3].mode = mode;
        sections[3].section = s++;
        sections[3].position = pos;
        pos += size;

        sections += 4;
        num -= 4;
    }
    if (num > 0) {
        sections[0].hidden = false;
        sections[0].mode = mode;
        sections[0].section = s++;
        sections[0].position = pos;
        pos += size;
        if (num > 1) {
            sections[1].hidden = false;
            sections[1].mode = mode;
            sections[1].section = s++;
            sections[1].position = pos;
            pos += size;
            if (num > 2) {
                sections[2].hidden = false;
                sections[2].mode = mode;
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
  \reimp
*/

void QHeaderView::paintEvent(QPaintEvent *e)
{
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

    if (count() == 0)
        return;
    const QVector<QHeaderViewPrivate::HeaderSection> sections = d->sections;

    QRect rect;
    int section;
    int width = d->viewport->width();
    int height = d->viewport->height();
//    bool focus = d->highlightCurrent && parentWidget() && parentWidget()->hasFocus();
    for (int i = start; i <= end; ++i) {
        if (sections.at(i).hidden)
            continue;
        section = sections.at(i).section;
        if (orientation() == Qt::Horizontal)
            rect.setRect(sectionPosition(section) - offset, 0, sectionSize(section), height);
        else
            rect.setRect(0, sectionPosition(section) - offset, width, sectionSize(section));
        paintSection(&painter, rect, section);
    }

    if (rect.right() < area.right()) {
        painter.fillRect(rect.right() + 1, 0,
                         width - rect.right() - 1, height,
                         palette().background());
    } else if (rect.bottom() < area.bottom()) {
        painter.fillRect(0, rect.bottom() + 1,
                         width, height - rect.bottom() - 1,
                         palette().background());
    }
}

/*!
  \reimp
*/

void QHeaderView::mousePressEvent(QMouseEvent *e)
{
    int pos = orientation() == Qt::Horizontal ? e->x() : e->y();
    if (e->state() & Qt::ControlButton && d->movableSections) {
        d->section = d->target = d->pressed = sectionAt(pos + offset());
        if (d->section == -1)
            return;
        d->state = QHeaderViewPrivate::MoveSection;
        d->setupSectionIndicator(d->section, pos);
        d->updateSectionIndicator(d->section, pos);
    } else {
        int handle = d->sectionHandleAt(pos + offset());
        while (handle > -1 && isSectionHidden(handle)) handle--;
        if (handle == -1) {
            d->pressed = sectionAt(pos + offset());
            updateSection(d->pressed);
            emit sectionPressed(d->pressed, e->state());
        } else if (resizeMode(handle) == Interactive) {
            d->state = QHeaderViewPrivate::ResizeSection;
            d->lastPos = (orientation() == Qt::Horizontal ? e->x() : e->y());
            d->section = handle;
        }
    }
    d->viewport->grabMouse();
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
            QSize strut = QApplication::globalStrut();
            int minimum = orientation() == Qt::Horizontal ? strut.width() : strut.height();
            if (size > minimum) {
                resizeSection(d->section, size);
                d->lastPos = (orientation() == Qt::Horizontal ? e->x() : e->y());
            }
            return;
        }
        case QHeaderViewPrivate::MoveSection: {
            int indicatorCenter = (orientation() == Qt::Horizontal
                                   ? d->sectionIndicator->width()
                                   : d->sectionIndicator->height()) / 2;
            int centerOffset = indicatorCenter - d->sectionIndicatorOffset;
            // This will drop the moved section to the position under the center of the indicator.
            // If centerOffset is 0, the section will be moved to the position of the mouse cursor.
            int position = pos + d->offset + centerOffset;
            int idx = indexAt(position);
            if (idx < 0)
                return;
            d->target = d->sections.at(idx).section;
            d->updateSectionIndicator(d->section, pos);
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
    }
}

/*!
  \reimp
*/

void QHeaderView::mouseReleaseEvent(QMouseEvent *e)
{
    int pos = orientation() == Qt::Horizontal ? e->x() : e->y();
    switch (d->state)
    case QHeaderViewPrivate::MoveSection: {
        moveSection(index(d->section), index(d->target));
        d->section = d->target = -1;
        d->updateSectionIndicator(d->section, pos);
        break;
    case QHeaderViewPrivate::NoState:
        updateSection(d->pressed);
        emit sectionClicked(sectionAt(pos + offset()), e->state());
        break;
    case QHeaderViewPrivate::ResizeSection:
        break;
    }
    d->viewport->releaseMouse();
    d->state = QHeaderViewPrivate::NoState;
    d->pressed = -1;
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
    Paints the section using the given \a painter with the \a option
    and model \a index.

    You normally would not need to use this function.
*/

void QHeaderView::paintSection(QPainter *painter, const QRect &rect, int section) const
{
    QStyleOptionHeader opt = d->getStyleOption();
    opt.rect = rect;
    opt.section = section;

    if (d->clickableSections) {
        bool selected = false;
        if (d->orientation == Qt::Horizontal)
            selected = selectionModel()->isColumnSelected(section, QModelIndex::Null);
        else
            selected = selectionModel()->isRowSelected(section, QModelIndex::Null);
        if (selected)
            opt.state |= QStyle::Style_Down;
    } else {
        if (section == d->pressed)
            opt.state |= QStyle::Style_Down;
//             opt.state |= QStyle::Style_Sunken;
    }

    opt.text = d->model->headerData(section, orientation(),
                                    QAbstractItemModel::DisplayRole).toString();
    opt.icon = d->model->headerData(section, orientation(),
                                    QAbstractItemModel::DecorationRole).toIconSet();
    opt.textAlignment = static_cast<Qt::Alignment>(d->model->headerData(section, orientation(),
                                                                        QAbstractItemModel::TextAlignmentRole).toInt());
    opt.iconAlignment = Qt::AlignVCenter;

    style().drawPrimitive(QStyle::PE_HeaderSection, &opt, painter, this);
    opt.rect = style().subRect(QStyle::SR_HeaderLabel, &opt, fontMetrics(), this);
    style().drawControl(QStyle::CE_HeaderLabel, &opt, painter, this);

    if (isSortIndicatorShown() && sortIndicatorSection() == section) {
        opt.rect = rect;
        opt.rect = style().subRect(QStyle::SR_HeaderArrow, &opt, fontMetrics(), this);
        opt.state = (sortIndicatorOrder() == Qt::AscendingOrder
                     ? QStyle::Style_Down : QStyle::Style_Up) | QStyle::Style_Off;
        style().drawPrimitive(QStyle::PE_HeaderArrow, &opt, painter, this);
    }
}

/*!
  Returns the size of the contents of the give \a section.
*/

QSize QHeaderView::sectionSizeFromContents(int section) const
{
    QSize size(100, 30);
    QStyleOptionHeader opt = d->getStyleOption();
    opt.text = d->model->headerData(section, orientation(),
                                    QAbstractItemModel::DisplayRole).toString();
    opt.icon = d->model->headerData(section, orientation(),
                                    QAbstractItemModel::DecorationRole).toIconSet();
    size = style().sizeFromContents(QStyle::CT_HeaderSection, &opt, size, fontMetrics(), this);

    if (isSortIndicatorShown() && sortIndicatorSection() == section) {
        int margin = style().pixelMetric(QStyle::PM_HeaderMargin);
        if (orientation() == Qt::Horizontal)
            size.rwidth() += size.height() + margin;
        else
            size.rheight() += size.width() + margin;
    }

    return size;
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

void QHeaderView::updateGeometries()
{
    if (d->stretchSections)
        resizeSections();
}

/*!
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

QRect QHeaderView::itemViewportRect(const QModelIndex &) const
{
    return QRect();
}

/*!
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

void QHeaderView::ensureItemVisible(const QModelIndex &)
{
    // do nothing - the header only displays sections
}

/*!
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

QModelIndex QHeaderView::itemAt(int, int) const
{
    return QModelIndex::Null;
}

/*!
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

bool QHeaderView::isIndexHidden(const QModelIndex &) const
{
    return true; // the header view has no items, just sections
}

/*!
    \internal

    Empty implementation because the header doesn't show QModelIndex items.
*/

QModelIndex QHeaderView::moveCursor(QAbstractItemView::CursorAction, Qt::ButtonState)
{
    return QModelIndex::Null;
}

/*!
  \internal

  Empty implementation because the header doesn't have selections.
*/

void QHeaderView::setSelection(const QRect&, QItemSelectionModel::SelectionFlags)
{
    // do nothing
}

/*!
    \internal

    Empty implementation because the header doesn't have selections.
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


// private implementation

int QHeaderViewPrivate::sectionHandleAt(int position)
{
    int idx = q->indexAt(position);
    if (idx < 0)
        return -1;
    int sec = sections.at(idx).section;
    int pos = q->sectionPosition(sec);
    int grip = q->style().pixelMetric(QStyle::PM_HeaderGripMargin);
    if (d->reverse()) {
        if (position < pos + grip)
            return sec;
        if (idx > 0 && position > pos + q->sectionSize(sec) - grip)
            return q->section(idx - 1);
    } else {
        if (idx > 0 && position < pos + grip)
            return q->section(idx - 1);
        if (position > pos + q->sectionSize(sec) - grip)
            return sec;
    }
    return -1;
}

void QHeaderViewPrivate::setupSectionIndicator(int section, int position)
{
    if (!sectionIndicator) {
        sectionIndicator = new QLabel(viewport);
        sectionIndicator->setWindowOpacity(0.75);
    }

    int x, y, w, h;
    int p = q->sectionPosition(section) - offset;
    if (orientation == Qt::Horizontal) {
        x = p;
        y = 0;
        w = q->sectionSize(section);
        h = viewport->height();
    } else {
        x = 0;
        y = p;
        w = viewport->width();
        h = q->sectionSize(section);
    }
    sectionIndicator->resize(w, h);
    QPixmap pix = QPixmap::grabWidget(viewport, x, y, w, h);
    sectionIndicator->setPixmap(pix);
    d->sectionIndicatorOffset = position - qMax(p, 0);

}

void QHeaderViewPrivate::updateSectionIndicator(int section, int position)
{
    if (section == -1 || target == -1 ) {
        sectionIndicator->hide();
        return;
    }

    if (orientation == Qt::Horizontal)
        sectionIndicator->move(position - d->sectionIndicatorOffset, 0);
    else
        sectionIndicator->move(0, position - d->sectionIndicatorOffset);

    sectionIndicator->show();
}

QStyleOptionHeader QHeaderViewPrivate::getStyleOption() const
{
    QStyleOptionHeader opt;
    opt.rect = q->rect();
    opt.palette = q->palette();
    opt.state = QStyle::Style_None | QStyle::Style_Raised;
    if (orientation == Qt::Horizontal)
        opt.state |= QStyle::Style_Horizontal;
    if (q->isEnabled())
        opt.state |= QStyle::Style_Enabled;
    opt.section = 0;
    return opt;
}
