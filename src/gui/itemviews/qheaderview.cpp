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
#include <qabstractitemdelegate.h>

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
    its section(), and can be located at a particular visualIndex() in the
    header. A section can have a sort indicator set with
    setSortIndicator(); this indicates whether the items in the associated
    item view will be sorted in the order given by the section.

    A header can be fixed in place, or made movable with setMovable().
    It can be made clickable with setClickable(), and has resizing behavior
    in accordance with setResizeMode().

    A header emits sectionMoved() if the user moves a section,
    sectionResized() if the user resizes a section, and
    sectionClicked() and sectionHandleDoubleClicked() in response to
    mouse clicks. A header also emits sectionCountChanged() and
    sectionAutoResize().

    You can identify a section using the section() and sectionAt()
    functions, or by its index position, using the visualIndex() and visualIndexAt()
    functions. Note that the visual index can change if a section is moved.

    For a horizontal header the section is equivalent to a column in the
    model, and for a vertical header the section is equivalent to a row
    in the model.

    \sa \link model-view-programming.html Model/View Programming\endlink QListView QTableView QTreeView

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
    \fn void QHeaderView::sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)

    This signal is emitted when a section is moved. The section's logical
    index is specified by \a logicalIndex, the old index by \a oldVisualIndex,
    and the new index position by \a newVisualIndex.
*/

/*!
    \fn void QHeaderView::sectionResized(int logicalIndex, int oldSize, int newSize)

    This signal is emitted when a section is resized. The section's logical
    number is specified by \a logicalIndex, the old size by \a oldSize, and the
    new size by \a newSize.
*/

/*!
    \fn void QHeaderView::sectionPressed(int logicalIndex, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)

    This signal is emitted when a section is pressed. The section's logical
    index is specified by \a logicalIndex.
    The state of the mouse buttons is specified by \a button, and the
    \a modifiers reflect the state of the keyboard's modifier keys.
*/

/*!
    \fn void QHeaderView::sectionClicked(int logicalIndex, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)

    This signal is emitted when a section is clicked. The section's logical
    index is specified by \a logicalIndex.
    The state of the mouse buttons is specified by \a button, and the
    \a modifiers reflect the state of the keyboard's modifier keys.
*/

/*!
    \fn void QHeaderView::sectionCountChanged(int oldCount, int newCount)

    This signal is emitted when the number of sections changes; i.e.
    when sections are added or deleted. The original count is specified by
    \a oldCount, and the new count by \a newCount.
*/

/*!
    \fn void QHeaderView::sectionHandleDoubleClicked(int logicalIndex)

    This signal is emitted when a section is double-clicked. The
    section's logical index is specified by \a logicalIndex.
*/

/*!
    \fn void QHeaderView::sectionAutoResize(int logicalIndex, ResizeMode mode)

    This signal is emitted when a section is automatically resized.
    The section's logical index is specified by \a logicalIndex, and the
    resize mode by \a mode.
*/

/*!
    \property QHeaderView::highlightSections
    \brief whether the sections containing selected items are highlighted
*/

/*!
    Creates a new generic header with the given \a orientation and \a parent.
*/
QHeaderView::QHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QAbstractItemView(*new QHeaderViewPrivate, parent)
{
    d->orientation = orientation;
    initialize();
}

/*!
  \internal
*/
QHeaderView::QHeaderView(QHeaderViewPrivate &dd,
                         Qt::Orientation orientation, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    d->orientation = orientation;
    initialize();
}

/*!
  Destroys the header.
*/

QHeaderView::~QHeaderView()
{
}

/*!
  \internal
*/
void QHeaderView::initialize()
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(NoFrame);
    d->viewport->setMouseTracking(true);
    delete d->delegate;
}

/*!
  \reimp
*/
void QHeaderView::setModel(QAbstractItemModel *model)
{
    if (d->model) {
        if (d->orientation == Qt::Horizontal) {
            QObject::disconnect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                                this, SLOT(sectionsInserted(QModelIndex,int,int)));
            QObject::disconnect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                                this, SLOT(sectionsAboutToBeRemoved(QModelIndex,int,int)));
        } else {
            QObject::disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                                this, SLOT(sectionsInserted(QModelIndex,int,int)));
            QObject::disconnect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                                this, SLOT(sectionsAboutToBeRemoved(QModelIndex,int,int)));
        }
        QObject::disconnect(d->model, SIGNAL(headerDataChanged(Orientation,int,int)),
                            this, SLOT(headerDataChanged(Orientation,int,int)));
    }

    if (model) {
        if (d->orientation == Qt::Horizontal) {
            QObject::connect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                             this, SLOT(sectionsInserted(QModelIndex,int,int)));
            QObject::connect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sectionsAboutToBeRemoved(QModelIndex,int,int)));
        } else {
            QObject::connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                             this, SLOT(sectionsInserted(QModelIndex,int,int)));
            QObject::connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                             this, SLOT(sectionsAboutToBeRemoved(QModelIndex,int,int)));
        }
        QObject::connect(model, SIGNAL(headerDataChanged(Orientation,int,int)),
                         this, SLOT(headerDataChanged(Orientation,int,int)));
    }

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
    return d->offset;
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
        d->viewport->scroll(isRightToLeft() ? -ndelta : ndelta, 0);
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
        return d->sections.last().position;
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
    // FIXME: we should check all sections (slow)
    QSize firstHint = sectionSizeFromContents(logicalIndex(0));
    QSize lastHint = sectionSizeFromContents(logicalIndex(count() - 1));
    int width = qMax(firstHint.width(), lastHint.width());
    int height = qMax(firstHint.height(), lastHint.height());
    return QSize(width, height);
}

/*!
    Returns a suitable size hint for the section specified by \a logicalIndex.

    \sa sizeHint()
*/

int QHeaderView::sectionSizeHint(int logicalIndex) const
{
    QSize size = sectionSizeFromContents(logicalIndex);
    return orientation() == Qt::Horizontal ? size.width() : size.height();
}

/*!
    Returns the visual index of the section that covers the given \a position in the viewport.
*/

int QHeaderView::visualIndexAt(int position) const
{
    if (count() < 1)
        return -1;

    if (d->reverse())
        position = d->viewport->width() - position;
    position += d->offset;

    int start = 0;
    int end = count() - 1;
    int visual = (end + 1) / 2;

    const QHeaderViewPrivate::HeaderSection *sections = d->sections.constData();

    while (end - start > 0) {
        if (sections[visual].position > position)
            end = visual - 1;
        else
            start = visual;
        visual = (start + end + 1) / 2;
    }

    if (visual == end && sections[end + 1].position < position)
        return -1;
    return visual;
}

/*!
    Returns the section that covers the given \a position in the viewport.
*/

int QHeaderView::logicalIndexAt(int position) const
{
    int visual = visualIndexAt(position);
    if (visual < 0 || visual >= d->sections.count())
        return -1;
    return d->sections.at(visual).logical;
}

/*!
    Returns the width (or height for vertical headers) of the given \a logicalIndex.
*/

int QHeaderView::sectionSize(int logicalIndex) const
{
    if (logicalIndex < 0
        || logicalIndex >= d->sections.count() - 1
        || isSectionHidden(logicalIndex))
        return 0;
    int visual = visualIndex(logicalIndex);
    return d->sections.at(visual + 1).position - d->sections.at(visual).position;
}

/*!
    Returns the section position of the given \a logicalIndex.

    \sa sectionViewportPosition()
*/

int QHeaderView::sectionPosition(int logicalIndex) const
{
    if (logicalIndex < 0 || logicalIndex >= d->sections.count())
        return 0;
    int visual = visualIndex(logicalIndex);
    return d->sections.at(visual).position;
}

/*!
    Returns the section viewport position of the given \a logicalIndex.

    \sa sectionPosition()
*/

int QHeaderView::sectionViewportPosition(int logicalIndex) const
{
    int offsetPosition = sectionPosition(logicalIndex) - d->offset;
    if (d->reverse())
        return d->viewport->width() - (offsetPosition + sectionSize(logicalIndex));
    return offsetPosition;
}

/*!
  \fn int QHeaderView::logicalIndexAt(int x, int y) const

  Returns the logical index of the section at the given coordinate.
  If the header is horizontal \a x will be used, otherwise \a y
  will be used to find the logical index.
*/

/*!
  \fn int QHeaderView::logicalIndexAt(const QPoint &pos) const

  Returns the logical index of the section at the position given in \a pos.
  If the header is horizontal the x-coordinate will be used to find the
  logical index; otherwise the y-coordinate will be used.
*/

/*!
    Moves the section at visual index \a from to occupy visual index \a to.
*/

void QHeaderView::moveSection(int from, int to)
{
    if (from == -1 || to == -1)
        return;

    if (from == to) {
        updateSection(visualIndex(from));
        return;
    }

    // if we haven't moved anything previously, initialize the indices array
    int count = d->sections.count();
    if (d->visualIndices.count() != count) {
        d->visualIndices.resize(count);
        for (int s = 0; s < count; ++s)
            d->visualIndices[s] = s;
    }

    // move sections and indices
    QHeaderViewPrivate::HeaderSection *sections = d->sections.data();
    int *visualIndices = d->visualIndices.data();
    int logical = sections[from].logical;
    int visual = from;
    if (to > from) {
        while (visual < to) {
            sections[visual].logical = sections[visual + 1].logical;
            visualIndices[sections[visual].logical] = visual;
            ++visual;
        }
    } else {
        while (visual > to) {
            sections[visual].logical = sections[visual - 1].logical;
            visualIndices[sections[visual].logical] = visual;
            --visual;
        }
    }
    sections[to].logical = logical;
    visualIndices[logical] = to;

    // move positions
    if (to > from) {
        for (visual = from; visual < to; ++visual)
            sections[visual + 1].position -= sectionSize(logicalIndex(visual))
                                             - sectionSize(logicalIndex(visual + 1));
    } else {
        int tmp;
        int size = sectionSize(logicalIndex(from));
        for (visual = to; visual < from; ++visual) {
            tmp = sectionSize(logicalIndex(visual));
            sections[visual + 1].position = sections[visual].position + size;
            size = tmp;
        }
    }

    d->viewport->update();
    emit sectionMoved(logical, from, to);
}

/*!
    Resizes the given \a logicalIndex to the given \a size.
*/

void QHeaderView::resizeSection(int logicalIndex, int size)
{
    int oldSize = sectionSize(logicalIndex);
    if (oldSize == size)
        return;

    int diff = size - oldSize;
    int visual = visualIndex(logicalIndex);
    QHeaderViewPrivate::HeaderSection *sections = d->sections.data() + visual + 1;
    int num = d->sections.size() - visual - 1;

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
    int pos = sectionViewportPosition(logicalIndex);
    QRect r;
    if (orientation() == Qt::Horizontal)
        if (isRightToLeft())
            r.setRect(0, 0, pos + size, h);
        else
            r.setRect(pos, 0, w - pos, h);
    else
        r.setRect(0, pos, w, h - pos);
    if (d->stretchSections || d->stretchLastSection) {
        resizeSections();
        r = d->viewport->rect();
    }
    d->viewport->update(r.normalize());
    emit sectionResized(logicalIndex, oldSize, size);
}

/*!
  \fn void QHeaderView::hideSection(int logicalIndex)
    Hides the section specified by \a logicalIndex.

    \sa showSection()
*/

/*!
  \fn void QHeaderView::showSection(int logicalIndex)
   Shows the section specified by \a logicalIndex.

   \sa hideSection()
*/

/*!
    Returns true if the section specified by \a logicalIndex is
    hidden from the user; otherwise returns false.
*/

bool QHeaderView::isSectionHidden(int logicalIndex) const
{
    int visual = visualIndex(logicalIndex);
    return d->sections.at(visual).hidden;
}

/*!
  If \a hide is true the section specified by \a logicalIndex is hidden,
  otherwise the section is shown.
*/

void QHeaderView::setSectionHidden(int logicalIndex, bool hide)
{
    // FIXME: if the size of the table changes, the hidden sections are forgotten
    if (hide) {
        resizeSection(logicalIndex, 0);
        d->sections[visualIndex(logicalIndex)].hidden = true;
    } else {
        d->sections[visualIndex(logicalIndex)].hidden = false;
        resizeSection(logicalIndex, d->defaultSectionSize());
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
    Returns the visual index position of the section specified by the
    given \a logicalIndex, or -1 otherwise.

    \sa section()
*/

int QHeaderView::visualIndex(int logicalIndex) const
{
    if (d->visualIndices.count() <= 0)
        return logicalIndex; // nothing has been moved yet
    if (logicalIndex < 0 || logicalIndex >= d->visualIndices.count())
        return -1;
    return d->visualIndices.at(logicalIndex);
}

/*!
    Returns the logicalIndex for the section at the given \a
    visualIndex position, or -1 otherwise.

    \sa visualIndex()
*/

int QHeaderView::logicalIndex(int visualIndex) const
{
    if (visualIndex < 0 || visualIndex >= d->sections.count())
        return -1;
    return d->sections.at(visualIndex).logical;
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

void QHeaderView::setHighlightSections(bool highlight)
{
    d->highlightSelected = highlight;
}

bool QHeaderView::highlightSections() const
{
    return d->highlightSelected;
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

    Sets the constraints on how the section specified by \a logicalIndex
    in the header can be resized to those described by the given \a mode.

    \sa QLayout::ResizeMode
*/

void QHeaderView::setResizeMode(ResizeMode mode, int logicalIndex)
{
    initializeSections();
    if (logicalIndex >= d->sections.count()) {
        qWarning("setResizeMode: section %d does not exist", logicalIndex);
        return;
    }
    int visual = visualIndex(logicalIndex);
    ResizeMode old = d->sections[visual].mode;
    d->sections[visual].mode = mode;
    if (mode == Stretch && old != Stretch)
        ++d->stretchSections;
}

/*!
    Returns the resize mode that applies to the section specified by the given \a logicalIndex.
*/

QHeaderView::ResizeMode QHeaderView::resizeMode(int logicalIndex) const
{
    if (logicalIndex >= d->sections.count())
        return Interactive;
    int visual = visualIndex(logicalIndex);
    return d->sections.at(visual).mode;
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
    Sets the sort indicator for the section specified by the given \a logicalIndex in the direction
    specified by \a order, and removes the sort indicator from any
    other section that was showing it.

    \sa sortIndicatorSection() sortIndicatorOrder()
*/

void QHeaderView::setSortIndicator(int logicalIndex, Qt::SortOrder order)
{
    int old = d->sortIndicatorSection;
    d->sortIndicatorSection = logicalIndex;
    d->sortIndicatorOrder = order;

    if (logicalIndex >= d->sections.count())
        return; // nothing to do

    // FIXME: sections.at() uses visible indexes
    if (old != logicalIndex && d->sections.at(logicalIndex).mode == Custom) {
        resizeSections();
        d->viewport->update();
    } else {
        if (old != logicalIndex)
            updateSection(old);
        updateSection(logicalIndex);
    }
}

// ### DOC: I made up the -1 case...
/*!
    Returns the logical index of the section that has a sort indicator, or -1
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

/*
  \property QHeaderView:: streatchLastSection

  ###
*/
bool QHeaderView::stretchLastSection() const
{
    return d->stretchLastSection;
}

void QHeaderView::setStretchLastSection(bool stretch)
{
    d->stretchLastSection = stretch;
}

/*!
    \internal
*/
void QHeaderView::doItemsLayout()
{
    initializeSections();
    QAbstractItemView::doItemsLayout();
}

/*!
  Updates the changed header sections
*/
void QHeaderView::headerDataChanged(Qt::Orientation orientation, int logicalFirst, int logicalLast)
{
    if (d->orientation != orientation)
        return;
    if (orientation == Qt::Horizontal) {
        int left = sectionViewportPosition(logicalFirst);
        int right = sectionViewportPosition(logicalLast) + sectionSize(logicalLast);
        d->viewport->update(left, 0, right - left, d->viewport->height());
    } else {
        int top = sectionViewportPosition(logicalFirst);
        int bottom = sectionViewportPosition(logicalLast) + sectionSize(logicalLast);
        d->viewport->update(0, top, d->viewport->width(), bottom - top);
    }
}

/*!
    \internal

    Updates the section specified by the given \a logicalIndex.
*/

void QHeaderView::updateSection(int logicalIndex)
{
    if (orientation() == Qt::Horizontal)
        d->viewport->update(QRect(sectionViewportPosition(logicalIndex),
                                  0, sectionSize(logicalIndex), d->viewport->height()));
    else
        d->viewport->update(QRect(0, sectionViewportPosition(logicalIndex),
                                  d->viewport->width(), sectionSize(logicalIndex)));
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
    int stretchSize = orientation() == Qt::Horizontal
                      ? d->viewport->width() : d->viewport->height();
    QList<int> section_sizes;
    int count = qMax(d->sections.count() - 1, 0);
    int last = (d->stretchLastSection ? count - 1 : -1);
    const QVector<QHeaderViewPrivate::HeaderSection> secs = d->sections;
    if (secs.isEmpty())
        return;
    for (int i = 0; i < count; ++i) {
        mode = (i == last ? Stretch : secs.at(i).mode);
        if (mode == Stretch) {
            ++stretchSecs;
            continue;
        }
        if (mode == Interactive) {
            secSize = sectionSize(secs.at(i).logical);
        } else {// mode == QHeaderView::Custom
            // FIXME: this is a bit hacky; see if we can find a cleaner solution
            QAbstractItemView *par = ::qt_cast<QAbstractItemView*>(parent());
            if (orientation() == Qt::Horizontal) {
                if (par)
                    secSize = par->sizeHintForColumn(i);
                secSize = qMax(secSize, sectionSizeHint(secs.at(i).logical));
            } else {
                if (par)
                    secSize = par->sizeHintForRow(i);
                secSize = qMax(secSize, sectionSizeHint(secs.at(i).logical));
            }
        }
        section_sizes.append(secSize);
        stretchSize -= secSize;
    }
    int position = 0;
    QSize strut = QApplication::globalStrut();
    int minimum = orientation() == Qt::Horizontal ? strut.width() : strut.height();
    int hint = stretchSecs > 0 ? stretchSize / stretchSecs : 0;
    int stretchSectionSize = qMax(hint, minimum);
    for (int i = 0; i < count; ++i) {
        d->sections[i].position = position;
        mode = (i == last ? Stretch : secs.at(i).mode);
        if (mode == Stretch) {
            position += stretchSectionSize;
        } else {
            position += section_sizes.front();
            section_sizes.removeFirst();
        }
    }
    d->sections[count].position = position;
}

/*!
    This slot is called when sections are inserted into the \a parent,
    \a logicalFirst and \a logicalLast indexes signify where the new
    sections are inserted.

    \a logicalFirst and \a logicalLast will be the same if just one
    section is inserted.
*/

void QHeaderView::sectionsInserted(const QModelIndex &parent, int logicalFirst, int)
{
    if (parent != rootIndex())
        return; // we only handle changes in the top level
    if (d->orientation == Qt::Horizontal)
        initializeSections(logicalFirst, d->model->columnCount(rootIndex()) - 1);
    else
        initializeSections(logicalFirst, d->model->rowCount(rootIndex()) - 1);
}

/*!
    This slot is called when sections are removed from the \a parent,
    \a logicalFirst and \a logicalLast signify where the sections are removed
    from. (\a logicalFirst and \a logicalLast will be the same if just one section
    is removed.)
*/

void QHeaderView::sectionsAboutToBeRemoved(const QModelIndex &parent,
                                           int logicalFirst, int logicalLast)
{
    if (parent != rootIndex())
        return; // we only handle changes in the top level
    // the sections have not been removed from the model yet
    int count = logicalLast - logicalFirst + 1;
    if (d->orientation == Qt::Horizontal)
        initializeSections(logicalFirst, d->model->columnCount(rootIndex()) - count - 1);
    else
        initializeSections(logicalFirst, d->model->rowCount(rootIndex()) - count - 1);
}

/*!
  \internal
*/

void QHeaderView::initializeSections()
{
    if (!model())
        return;
    if (d->orientation == Qt::Horizontal) {
        int c = model()->columnCount(rootIndex());
        if (c != count())
            initializeSections(0, c > 0 ? c - 1 : 0);
    } else {
        int r = model()->rowCount(rootIndex());
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
    int l = start;
    int num = end - start + 1;
    int size = d->defaultSectionSize();

    // set resize mode
    ResizeMode mode = d->globalResizeMode;
    if (mode == Stretch)
        d->stretchSections += num;

    // unroll loop - to initialize the arrays as fast as possible
    while (num >= 4) {

        sections[0].hidden = false;
        sections[0].mode = mode;
        sections[0].logical = l++;
        sections[0].position = pos;
        pos += size;

        sections[1].hidden = false;
        sections[1].mode = mode;
        sections[1].logical = l++;
        sections[1].position = pos;
        pos += size;

        sections[2].hidden = false;
        sections[2].mode = mode;
        sections[2].logical = l++;
        sections[2].position = pos;
        pos += size;

        sections[3].hidden = false;
        sections[3].mode = mode;
        sections[3].logical = l++;
        sections[3].position = pos;
        pos += size;

        sections += 4;
        num -= 4;
    }
    if (num > 0) {
        sections[0].hidden = false;
        sections[0].mode = mode;
        sections[0].logical = l++;
        sections[0].position = pos;
        pos += size;
        if (num > 1) {
            sections[1].hidden = false;
            sections[1].mode = mode;
            sections[1].logical = l++;
            sections[1].position = pos;
            pos += size;
            if (num > 2) {
                sections[2].hidden = false;
                sections[2].mode = mode;
                sections[2].logical = l++;
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
        oldRect = QRect(sectionViewportPosition(old.column()), 0,
                        sectionSize(old.column()), d->viewport->height());
        currentRect = QRect(sectionViewportPosition(current.column()), 0,
                            sectionSize(current.column()), d->viewport->height());
    } else {
        oldRect = QRect(0, sectionViewportPosition(old.row()),
                        d->viewport->width(), sectionSize(old.row()));
        currentRect = QRect(0, sectionViewportPosition(current.row()),
                            d->viewport->width(), sectionSize(current.row()));
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

    int start, end;
    if (orientation() == Qt::Horizontal) {
        start = visualIndexAt(area.left());
        end = visualIndexAt(area.right() - 1);
    } else {
        start = visualIndexAt(area.top());
        end = visualIndexAt(area.bottom() - 1);
    }

    if (d->reverse()) {
        start = (start == -1 ? count() - 1 : start);
        end = (end == -1 ? 0 : end);
    } else {
        start = (start == -1 ? 0 : start);
        end = (end == -1 ? count() - 1 : end);
    }

    int tmp = start;
    start = qMin(start, end);
    end = qMax(tmp, end);

    if (count() == 0)
        return;
    const QVector<QHeaderViewPrivate::HeaderSection> sections = d->sections;

    QRect rect;
    int logical;
    bool highlight = false;
    const int width = d->viewport->width();
    const int height = d->viewport->height();
    const bool active = isActiveWindow();
    const QFont fnt(painter.font()); // save the painter font
    for (int i = start; i <= end; ++i) {
        if (sections.at(i).hidden)
            continue;
        logical = sections.at(i).logical;
        if (orientation() == Qt::Horizontal) {
            rect.setRect(sectionViewportPosition(logical), 0, sectionSize(logical), height);
            highlight = d->selectionModel->columnIntersectsSelection(logical, rootIndex());
        } else {
            rect.setRect(0, sectionViewportPosition(logical), width, sectionSize(logical));
            highlight = d->selectionModel->rowIntersectsSelection(logical, rootIndex());
        }
        if (d->highlightSelected && highlight && active) {
            QFont bf(fnt);
            bf.setBold(true);
            painter.setFont(bf);
            paintSection(&painter, rect, logical);
            painter.setFont(fnt); // restore the font
        } else {
            paintSection(&painter, rect, logical);
        }
    }

    if (d->reverse()) {
        if (rect.left() > area.left())
            painter.fillRect(area.left(), 0, rect.left() - area.left(), height,
                             palette().background());
    } else if (rect.right() < area.right()) {
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
    int handle = d->sectionHandleAt(pos);
    while (handle > -1 && isSectionHidden(handle)) --handle;
    if (handle == -1) {
        d->pressed = logicalIndexAt(pos);
        if (d->movableSections) {
            d->section = d->target = d->pressed;
            if (d->section == -1)
                return;
            d->state = QHeaderViewPrivate::MoveSection;
            d->setupSectionIndicator(d->section, pos);
        }
        if (d->clickableSections) {
            updateSection(d->pressed);
            emit sectionPressed(d->pressed);
        }
    } else if (resizeMode(handle) == Interactive) {
        d->state = QHeaderViewPrivate::ResizeSection;
        d->section = handle;
    }
    d->lastPos = pos;
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
            int visual = visualIndexAt(pos + centerOffset);
            if (visual < 0)
                return;
            d->target = d->sections.at(visual).logical;
            d->updateSectionIndicator(d->section, pos);
            return;
        }
        case QHeaderViewPrivate::NoState: {
            int handle = d->sectionHandleAt(pos);
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
    switch (d->state) {
    case QHeaderViewPrivate::MoveSection:
        if (pos != d->lastPos) { // moving
            moveSection(visualIndex(d->section), visualIndex(d->target));
            d->section = d->target = -1;
            d->updateSectionIndicator(d->section, pos);
            break;
        } // not moving
    case QHeaderViewPrivate::NoState:
        if (d->clickableSections) {
            updateSection(d->pressed);
            emit sectionClicked(logicalIndexAt(pos));
        }
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
    int handle = d->sectionHandleAt(pos);
    while (handle > -1 && isSectionHidden(handle)) handle--;
    if (handle > -1 && resizeMode(handle) == Interactive)
        emit sectionHandleDoubleClicked(handle);
    else
        emit sectionDoubleClicked(logicalIndexAt(e->pos()));
}

/*!
    Paints the section specified by the given \a logicalIndex, using the given \a painter and \a rect.

    You normally would not need to use this function.
*/

void QHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    bool showSortIndicator = isSortIndicatorShown() && sortIndicatorSection() == logicalIndex;

    QStyle::State state = QStyle::State_None;
    if (isEnabled())
        state |= QStyle::State_Enabled;

    if (window()->isActiveWindow())
        state |= QStyle::State_Active;

    if (d->clickableSections) {
        if (logicalIndex == d->pressed) {
            state |= QStyle::State_Sunken;
        } else if (d->highlightSelected) {
            bool selected = false;
            if (d->orientation == Qt::Horizontal)
                selected = selectionModel()->isColumnSelected(logicalIndex, QModelIndex());
            else
                selected = selectionModel()->isRowSelected(logicalIndex, QModelIndex());
            if (selected)
                state |= QStyle::State_On;
        }
    }
    if (showSortIndicator) {
        state |= (sortIndicatorOrder() == Qt::AscendingOrder
                     ? QStyle::State_Down : QStyle::State_Up);

    }

    int textAlignment = d->model->headerData(logicalIndex, orientation(),
                                             QAbstractItemModel::TextAlignmentRole).toInt();

    QStyleOptionHeader opt = d->getStyleOption();
    opt.rect = rect;
    opt.section = logicalIndex;
    opt.state |= state;
    opt.textAlignment = Qt::Alignment(textAlignment);
    opt.iconAlignment = Qt::AlignVCenter;
    opt.text = d->model->headerData(logicalIndex, orientation(),
                                    QAbstractItemModel::DisplayRole).toString();
    opt.icon = qVariant_to<QIcon>(d->model->headerData(logicalIndex, orientation(),
                                    QAbstractItemModel::DecorationRole));

    style()->drawControl(QStyle::CE_Header, &opt, painter, this);
}

/*!
  Returns the size of the contents of the section specified by the give \a logicalIndex.
*/

QSize QHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QSize size(100, 30);
    QStyleOptionHeader opt = d->getStyleOption();
    QFont fnt = font();
    fnt.setBold(true);
    opt.fontMetrics = QFontMetrics(fnt); // do the metrics with a bold font
    opt.text = d->model->headerData(logicalIndex, orientation(),
                                    QAbstractItemModel::DisplayRole).toString();
    opt.icon = qVariant_to<QIcon>(d->model->headerData(logicalIndex, orientation(),
                                    QAbstractItemModel::DecorationRole));
    size = style()->sizeFromContents(QStyle::CT_HeaderSection, &opt, size, this);

    if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex) {
        int margin = style()->pixelMetric(QStyle::PM_HeaderMargin);
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
        return d->offset;
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
        return d->offset;
    return 0;
}

/*!
  \reimp
  \internal
*/

void QHeaderView::updateGeometries()
{
    if (d->stretchSections || d->stretchLastSection)
        resizeSections();
}

/*!
  \reimp
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/
void QHeaderView::dataChanged(const QModelIndex &, const QModelIndex &)
{
    // do nothing
}

/*!
  \reimp
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/
void QHeaderView::rowsInserted(const QModelIndex &, int, int)
{
    // do nothing
}

/*!
  \reimp
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

QRect QHeaderView::viewportRectForIndex(const QModelIndex &) const
{
    return QRect();
}

/*!
  \reimp
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

void QHeaderView::ensureVisible(const QModelIndex &)
{
    // do nothing - the header only displays sections
}

/*!
  \reimp
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

QModelIndex QHeaderView::indexAt(int, int) const
{
    return QModelIndex();
}

/*!
  \reimp
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

bool QHeaderView::isIndexHidden(const QModelIndex &) const
{
    return true; // the header view has no items, just sections
}

/*!
  \reimp
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

QModelIndex QHeaderView::moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers)
{
    return QModelIndex();
}

/*!
    \reimp

    Selects the items in the given \a rect according to the specified
    \a flags.

    The base class implementation does nothing.
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
        int left = d->model->columnCount(rootIndex()) - 1;
        int right = 0;
        int rangeLeft, rangeRight;

        for (int i = 0; i < selection.count(); ++i) {
            QItemSelectionRange r = selection.at(i);
            if (r.parent().isValid())
                continue; // we only know about toplevel items
            // FIXME an item inside the range may be the leftmost or rightmost
            rangeLeft = visualIndex(r.left());
            rangeRight = visualIndex(r.right());
            if (rangeLeft < left)
                left = rangeLeft;
            if (rangeRight > right)
                right = rangeRight;
        }

        int logicalLeft = logicalIndex(left);
        int logicalRight = logicalIndex(right);

        int leftPos = sectionViewportPosition(logicalLeft);
        int rightPos = sectionViewportPosition(logicalRight) + sectionSize(logicalRight);

        return QRect(leftPos, 0, rightPos - leftPos, height());
    }
    // orientation() == Qt::Vertical
    int top = d->model->rowCount(rootIndex()) - 1;
    int bottom = 0;
    int rangeTop, rangeBottom;

    for (int i = 0; i < selection.count(); ++i) {
        QItemSelectionRange r = selection.at(i);
        if (r.parent().isValid())
            continue; // we only know about toplevel items
        // FIXME an item inside the range may be the leftmost or rightmost
        rangeTop = visualIndex(r.top());
        rangeBottom = visualIndex(r.bottom());
        if (rangeTop < top)
            top = rangeTop;
        if (rangeBottom > bottom)
            bottom = rangeBottom;
    }

    int logicalTop = logicalIndex(top);
    int logicalBottom = logicalIndex(bottom);

    int topPos = sectionViewportPosition(logicalTop);
    int bottomPos = sectionViewportPosition(logicalBottom) + sectionSize(logicalBottom);

    return QRect(0, topPos, width(), bottomPos - topPos);
}


// private implementation

int QHeaderViewPrivate::sectionHandleAt(int position)
{
    int visual = q->visualIndexAt(position);
    if (visual < 0)
        return -1;
    int log = sections.at(visual).logical;
    int pos = q->sectionViewportPosition(log);
    int grip = q->style()->pixelMetric(QStyle::PM_HeaderGripMargin);
    if (d->reverse()) { // FIXME:
        if (position < pos + grip)
            return log;
        if (visual > 0 && position > pos + q->sectionSize(log) - grip)
            return q->logicalIndex(visual - 1);
    } else {
        if (visual > 0 && position < pos + grip)
            return q->logicalIndex(visual - 1);
        if (position > pos + q->sectionSize(log) - grip)
            return log;
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
    int p = q->sectionViewportPosition(section);
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
    opt.state = QStyle::State_None | QStyle::State_Raised;
    if (orientation == Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;
    if (q->isEnabled())
        opt.state |= QStyle::State_Enabled;
    opt.section = 0;
    return opt;
}
