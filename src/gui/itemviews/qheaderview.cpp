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

#ifndef QT_NO_ITEMVIEWS
#include <qbitarray.h>
#include <qdebug.h>
#include <qevent.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qvector.h>
#include <qvarlengtharray.h>
#include <qabstractitemdelegate.h>
#include <qvariant.h>
#include <private/qheaderview_p.h>

/*!
    \class QHeaderView

    \brief The QHeaderView class provides a header row or header column for item views.

    \ingroup model-view
    \mainclass

    A QHeaderView displays the headers used in item views such as the
    QTableView and QTreeView classes. It takes the place of
    the \c QHeader class previously used for the same purpose, but uses
    the Qt's model/view architecture for consistency with the item view
    classes.

    The QHeaderView class is one of the \l{Model/View Classes} and is
    part of Qt's \l{Model/View Programming}{model/view framework}.

    The model gets the data for each section from the model using
    the getHeaderData() function.  You can set the data by using
    setHeaderData().

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

    You can identify a section using the logicalIndex() and logicalIndexAt()
    functions, or by its index position, using the visualIndex() and visualIndexAt()
    functions. The visual index will change if a section is moved while the logical
    index wont change.

    For a horizontal header the section is equivalent to a column in the
    model, and for a vertical header the section is equivalent to a row
    in the model.

    \sa {Model/View Programming}, QListView, QTableView, QTreeView

*/

/*!
    \enum QHeaderView::ResizeMode

    \value Interactive Don't automatically change the size (let the
    user do it manually).
    \value Stretch Fill all the available visible space.
    \value Custom Don't automatically change the size here, leave it
    to some other object.
    \value Contents Automatically resize to the contents of the view.
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
    \fn void QHeaderView::sectionPressed(int logicalIndex)

    This signal is emitted when a section is pressed. The section's logical
    index is specified by \a logicalIndex.
*/

/*!
    \fn void QHeaderView::sectionClicked(int logicalIndex)

    This signal is emitted when a section is clicked. The section's logical
    index is specified by \a logicalIndex.
*/

/*!
    \fn void QHeaderView::sectionDoubleClicked(int logicalIndex)

    This signal is emitted when a section is double-clicked. The
    section's logical index is specified by \a logicalIndex.
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
    \fn void QHeaderView::sectionAutoResize(int logicalIndex, QHeaderView::ResizeMode mode)

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
    Q_D(QHeaderView);
    d->setDefaultValues(orientation);
    initialize();
}

/*!
  \internal
*/
QHeaderView::QHeaderView(QHeaderViewPrivate &dd,
                         Qt::Orientation orientation, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    Q_D(QHeaderView);
    d->setDefaultValues(orientation);
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
    Q_D(QHeaderView);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(NoFrame);
    d->viewport->setMouseTracking(true);
    d->viewport->setBackgroundRole(QPalette::Button);
    delete d->delegate;
}

/*!
  \reimp
*/
void QHeaderView::setModel(QAbstractItemModel *model)
{
    Q_D(QHeaderView);
    if (d->model) {
        if (d->orientation == Qt::Horizontal) {
            QObject::disconnect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                                this, SLOT(sectionsInserted(QModelIndex,int,int)));
            QObject::disconnect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                                this, SLOT(sectionsAboutToBeRemoved(QModelIndex,int,int)));
            QObject::disconnect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                                this, SLOT(sectionsRemoved(QModelIndex,int,int)));
        } else {
            QObject::disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                                this, SLOT(sectionsInserted(QModelIndex,int,int)));
            QObject::disconnect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                                this, SLOT(sectionsAboutToBeRemoved(QModelIndex,int,int)));
            QObject::disconnect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                                this, SLOT(sectionsRemoved(QModelIndex,int,int)));
         }
        QObject::disconnect(d->model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                            this, SLOT(headerDataChanged(Qt::Orientation,int,int)));
    }

    if (model) {
        if (d->orientation == Qt::Horizontal) {
            QObject::connect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                             this, SLOT(sectionsInserted(QModelIndex,int,int)));
            QObject::connect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sectionsAboutToBeRemoved(QModelIndex,int,int)));
            QObject::connect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                this, SLOT(sectionsRemoved(QModelIndex,int,int)));
        } else {
            QObject::connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                             this, SLOT(sectionsInserted(QModelIndex,int,int)));
            QObject::connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                             this, SLOT(sectionsAboutToBeRemoved(QModelIndex,int,int)));
            QObject::connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                             this, SLOT(sectionsRemoved(QModelIndex,int,int)));
        }
        QObject::connect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                         this, SLOT(headerDataChanged(Qt::Orientation,int,int)));
    }
    d->clear();

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
    Q_D(const QHeaderView);
    return d->orientation;
}

/*!
    Returns the offset of the header: this is the header's left-most
    (or top-most for vertical headers) visible pixel.

    \sa setOffset()
*/

int QHeaderView::offset() const
{
    Q_D(const QHeaderView);
    return d->offset;
}

/*!
    \fn void QHeaderView::setOffset(int offset)

    Sets the header's offset to \a offset.

    \sa offset()
*/

void QHeaderView::setOffset(int newOffset)
{
    Q_D(QHeaderView);
    if (d->offset == newOffset)
        return;
    int ndelta = d->offset - newOffset;
    d->offset = newOffset;
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
    Q_D(const QHeaderView);
    Q_ASSERT(d->headerLength() == d->length);
    return d->length;
}

/*!
    Returns a suitable size hint for this header.

    \sa sectionSizeHint()
*/

QSize QHeaderView::sizeHint() const
{
    Q_D(const QHeaderView);
    if (d->model == 0 || count() < 1)
        return QSize(0, 0);
    if (d->cachedSizeHint.isValid())
        return d->cachedSizeHint;
    int width = 0;
    int height = 0;
    // get size hint for the first n sections
    int c = qMin(count(), 100);
    for (int i = 0; i < c; ++i) {
        QSize hint = sectionSizeFromContents(i);
        width = qMax(hint.width(), width);
        height = qMax(hint.height(), height);
    }
    // get size hint for the last n sections
    c = qMax(count() - 100, c);
    for (int j = count() - 1; j >= c; --j) {
        QSize hint = sectionSizeFromContents(j);
        width = qMax(hint.width(), width);
        height = qMax(hint.height(), height);
    }
    d->cachedSizeHint = QSize(width, height);
    return d->cachedSizeHint;
}

/*!
    Returns a suitable size hint for the section specified by \a logicalIndex.

    \sa sizeHint()
*/

int QHeaderView::sectionSizeHint(int logicalIndex) const
{
    Q_D(const QHeaderView);
    if (d->model == 0 || logicalIndex < 0 || logicalIndex >= count())
        return -1;
    QSize size = sectionSizeFromContents(logicalIndex);
    return orientation() == Qt::Horizontal ? size.width() : size.height();
}

/*!
    Returns the visual index of the section that covers the given \a position in the viewport.

    \sa logicalIndexAt()
*/

int QHeaderView::visualIndexAt(int position) const
{
    Q_D(const QHeaderView);
    d->executePostedLayout();
    int count = d->sectionCount;
    if (count < 1)
        return -1;
    if (d->reverse())
        position = d->viewport->width() - position;
    position += d->offset;
    if (position < 0 || position > length())
        return -1;
    int visual = d->headerVisualIndexAt(position);
    while (d->isVisualIndexHidden(visual) && visual < count)
        ++visual;
    if (visual >= count)
        return -1;
    return visual;
}

/*!
    Returns the section that covers the given \a position in the viewport.

    \sa visualIndexAt()
*/

int QHeaderView::logicalIndexAt(int position) const
{
    if (position >= 0 && position < length())
        return logicalIndex(visualIndexAt(position));
    return -1;
}

/*!
    Returns the width (or height for vertical headers) of the given \a logicalIndex.
*/

int QHeaderView::sectionSize(int logicalIndex) const
{
    Q_D(const QHeaderView);
    if (isSectionHidden(logicalIndex))
        return 0;
    int visual = visualIndex(logicalIndex);
    Q_ASSERT(visual != -1);
    return d->headerSectionSize(visual);
}

/*!
    Returns the section position of the given \a logicalIndex.

    \sa sectionViewportPosition()
*/

int QHeaderView::sectionPosition(int logicalIndex) const
{
    Q_D(const QHeaderView);
    int visual = visualIndex(logicalIndex);
    // in some cases users may change the selections
    // before we have a chance to do the layout
    if (visual == -1)
        return -1;
    return d->headerSectionPosition(visual);
}

/*!
    Returns the section viewport position of the given \a logicalIndex.
    If the section is hidden the function returns -1.

    \sa sectionPosition()
*/

int QHeaderView::sectionViewportPosition(int logicalIndex) const
{
    Q_D(const QHeaderView);
    if (logicalIndex < 0 || logicalIndex >= count())
        return -1;
    int position = sectionPosition(logicalIndex);
    if (position < 0)
        return position; // the section was hidden
    int offsetPosition = position - d->offset;
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

    \sa sectionsMoved()
*/

void QHeaderView::moveSection(int from, int to)
{
    Q_D(QHeaderView);

    d->executePostedLayout();
    Q_ASSERT(from >= 0 && from < d->sectionCount);
    Q_ASSERT(to >= 0 && to < d->sectionCount);

    if (from == to) {
        int logical = logicalIndex(from);
        Q_ASSERT(logical != -1);
        updateSection(logical);
        return;
    }

    //int oldHeaderLength = length(); // ### for debugging; remove later

    // if we haven't moved anything previously, initialize the indices array
    int count = d->sectionCount;
    if (d->visualIndices.count() != count || d->logicalIndices.count() != count) {
        d->visualIndices.resize(count);
        d->logicalIndices.resize(count);
        for (int s = 0; s < count; ++s) {
            d->visualIndices[s] = s;
            d->logicalIndices[s] = s;
        }
    }

    QBitArray sectionHidden = d->sectionHidden;
    QHeaderView::ResizeMode *sectionResizeMode = d->sectionResizeMode.isEmpty() ?
                                                 0 : d->sectionResizeMode.data();
    QHeaderView::ResizeMode sectionResizeModeFrom = d->visualIndexResizeMode(from);
    int *visualIndices = d->visualIndices.data();
    int *logicalIndices = d->logicalIndices.data();
    int logical = logicalIndices[from];
    int visual = from;

    QVarLengthArray<int> sizes(qAbs(to - from) + 1);

    // move sections and indices
    if (to > from) {
        sizes[to - from] = d->headerSectionSize(from);
        while (visual < to) {
            sizes[visual - from] = d->headerSectionSize(visual + 1);
            if (!sectionHidden.isEmpty())
                sectionHidden.setBit(visual, sectionHidden.testBit(visual + 1));
            if (sectionResizeMode)
                sectionResizeMode[visual] = sectionResizeMode[visual + 1];
            visualIndices[logicalIndices[visual + 1]] = visual;
            logicalIndices[visual] = logicalIndices[visual + 1];
            ++visual;
        }
    } else {
        sizes[0] = d->headerSectionSize(from);
        while (visual > to) {
            sizes[visual - to] = d->headerSectionSize(visual - 1);
            if (!sectionHidden.isEmpty())
                sectionHidden.setBit(visual, sectionHidden.testBit(visual - 1));
            if (sectionResizeMode)
                sectionResizeMode[visual] = sectionResizeMode[visual - 1];
            visualIndices[logicalIndices[visual - 1]] = visual;
            logicalIndices[visual] = logicalIndices[visual - 1];
            --visual;
        }
    }
    if (!sectionHidden.isEmpty()) {
        sectionHidden.setBit(to, d->sectionHidden.testBit(from));
        d->sectionHidden = sectionHidden;
    }
    if (sectionResizeMode)
        sectionResizeMode[to] = sectionResizeModeFrom;
    visualIndices[logical] = to;
    logicalIndices[to] = logical;

    //Q_ASSERT(oldHeaderLength == length());
    // move sizes
    // ### check for spans of section sizes here
    if (to > from) {
        for (visual = from; visual <= to; ++visual) {
            int size = sizes[visual - from];
            d->createSectionSpan(visual, visual, size);
        }
    } else {
        for (visual = to; visual <= from; ++visual) {
            int size = sizes[visual - to];
            d->createSectionSpan(visual, visual, size);
        }
    }
    //Q_ASSERT(d->headerLength() == length());
    //Q_ASSERT(oldHeaderLength == length());

    d->viewport->update();
    emit sectionMoved(logical, from, to);
}

/*!
    Resizes the given \a logicalIndex to the given \a size.
*/

void QHeaderView::resizeSection(int logicalIndex, int size)
{
    Q_D(QHeaderView);

    if (isSectionHidden(logicalIndex))
        return;

    int oldSize = sectionSize(logicalIndex);
    if (oldSize == size)
        return;

    d->executePostedLayout();
    d->invalidateCachedSizeHint();

    int visual = visualIndex(logicalIndex);
    Q_ASSERT(visual != -1);
    d->createSectionSpan(visual, visual, size);

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
    if (d->hasAutoResizeSections()) {
        resizeSections();
        r = d->viewport->rect();
    }
    d->viewport->update(r.normalized());
    emit sectionResized(logicalIndex, oldSize, size);
}

/*!
    Resizes the sections according to the given \a mode, ignoring the current
    resize mode.

    \sa resizeMode()
*/

void QHeaderView::resizeSections(QHeaderView::ResizeMode mode)
{
    Q_D(QHeaderView);
    d->resizeSections(mode, true);
}

/*!
  \fn void QHeaderView::hideSection(int logicalIndex)
    Hides the section specified by \a logicalIndex.

    \sa showSection() isSectionHidden()
*/

/*!
  \fn void QHeaderView::showSection(int logicalIndex)
   Shows the section specified by \a logicalIndex.

   \sa hideSection() isSectionHidden()
*/

/*!
    Returns true if the section specified by \a logicalIndex is
    explicitly hidden from the user; otherwise returns false.

    \sa setSectionHidden()
*/

bool QHeaderView::isSectionHidden(int logicalIndex) const
{
    Q_D(const QHeaderView);
    Q_ASSERT(logicalIndex >= 0);
    d->executePostedLayout();
    if (logicalIndex >= d->sectionHidden.count())
        return false; // there are no hidden sections
    int visual = visualIndex(logicalIndex);
    Q_ASSERT(visual != -1);
    return d->sectionHidden.testBit(visual);
}

/*!
    \since 4.1

    Returns the number of sections in the header that has been hidden.

    \sa setSectionHidden()
*/
int QHeaderView::hiddenSectionCount() const
{
    Q_D(const QHeaderView);
    return d->hiddenSectionSize.count();
}

/*!
  If \a hide is true the section specified by \a logicalIndex is hidden,
  otherwise the section is shown.

  \sa isSectionHidden()
*/

void QHeaderView::setSectionHidden(int logicalIndex, bool hide)
{
    Q_D(QHeaderView);
    d->executePostedLayout();
    int visual = visualIndex(logicalIndex);
    Q_ASSERT(visual != -1);
    if (hide && d->isVisualIndexHidden(visual))
        return;
    if (hide) {
        int size = sectionSize(logicalIndex);
        resizeSection(logicalIndex, 0);
        d->hiddenSectionSize.insert(logicalIndex, size);
        if (d->sectionHidden.count() < count())
            d->sectionHidden.resize(count());
        d->sectionHidden.setBit(visual, true);
        // A bit of a hack because resizeSection has to called before hiding FIXME
//         if (d->hasAutoResizeSections())
//             resizeSections(); // changes because of the new/old hiddne
    } else if (d->isVisualIndexHidden(visual)) {
        int size = d->hiddenSectionSize.value(logicalIndex, d->defaultSectionSize);
        d->hiddenSectionSize.remove(logicalIndex);
        if (d->hiddenSectionSize.isEmpty()) {
            d->sectionHidden.clear();
        } else {
            Q_ASSERT(visual <= d->sectionHidden.count());
            d->sectionHidden.setBit(visual, false);
        }
        resizeSection(logicalIndex, size);
    }
}

/*!
    Returns the number of sections in the header.
*/

int QHeaderView::count() const
{
    Q_D(const QHeaderView);
    //Q_ASSERT(d->sectionCount == d->headerSectionCount());
    return d->sectionCount;
}

/*!
    Returns the visual index position of the section specified by the
    given \a logicalIndex, or -1 otherwise.
    Hidden sections still have valid visual indexes.

    \sa logicalIndex()
*/

int QHeaderView::visualIndex(int logicalIndex) const
{
    Q_D(const QHeaderView);
    Q_ASSERT(logicalIndex >= 0);
    d->executePostedLayout();
    if (d->visualIndices.isEmpty()) { // nothing has been moved, so we have no mapping
        if (logicalIndex < d->sectionCount)
            return logicalIndex;
    } else if (logicalIndex < d->visualIndices.count()) {
        int visual = d->visualIndices.at(logicalIndex);
        Q_ASSERT(visual < d->sectionCount);
        return visual;
    }
    return -1;
}

/*!
    Returns the logicalIndex for the section at the given \a
    visualIndex position, or -1 otherwise.

    \sa visualIndex(), sectionPosition()
*/

int QHeaderView::logicalIndex(int visualIndex) const
{
    Q_D(const QHeaderView);
    if (visualIndex < 0 || visualIndex >= d->sectionCount)
        return -1;
    return d->logicalIndex(visualIndex);
}

/*!
    If \a movable is true, the header may be moved by the user;
    otherwise it is fixed in place.

    \sa isMovable()
*/

void QHeaderView::setMovable(bool movable)
{
    Q_D(QHeaderView);
    d->movableSections = movable;
}

/*!
    Returns true if the header can be moved by the user; otherwise
    returns false.

    \sa setMovable()
*/

bool QHeaderView::isMovable() const
{
    Q_D(const QHeaderView);
    return d->movableSections;
}

/*!
    If \a clickable is true, the header will respond to single clicks.

    \sa isClickable() showSortIndicator()
*/

void QHeaderView::setClickable(bool clickable)
{
    Q_D(QHeaderView);
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
    Q_D(const QHeaderView);
    return d->clickableSections;
}

void QHeaderView::setHighlightSections(bool highlight)
{
    Q_D(QHeaderView);
    d->highlightSelected = highlight;
}

bool QHeaderView::highlightSections() const
{
    Q_D(const QHeaderView);
    return d->highlightSelected;
}

/*!
    Sets the constraints on how the header can be resized to those
    described by the given \a mode.

    \sa resizeMode()
*/

void QHeaderView::setResizeMode(ResizeMode mode)
{
    Q_D(QHeaderView);
    initializeSections();
    d->stretchSections = (mode == Stretch ? count() : 0);
    d->contentsSections =  (mode == ResizeToContents ? count() : 0);
    d->globalResizeMode = mode;
    d->sectionResizeMode.clear();
    if (d->hasAutoResizeSections())
        resizeSections(); // section sizes may change as a result of the new mode
}

/*!
    \overload

    Sets the constraints on how the section specified by \a logicalIndex
    in the header can be resized to those described by the given \a mode.
*/

void QHeaderView::setResizeMode(int logicalIndex, ResizeMode mode)
{
    Q_D(QHeaderView);
    int visual = visualIndex(logicalIndex);
    Q_ASSERT(visual != -1);
    if (d->sectionResizeMode.count() < d->sectionCount)
        d->sectionResizeMode.fill(d->globalResizeMode, d->sectionCount);
    ResizeMode old = d->sectionResizeMode.at(visual);
    d->sectionResizeMode[visual] = mode;

    if (mode == Stretch && old != Stretch)
        ++d->stretchSections;
    else if (mode == ResizeToContents && old != ResizeToContents)
        ++d->contentsSections;
    else if (mode != Stretch && old == Stretch)
        --d->stretchSections;
    else if (mode != ResizeToContents && old == ResizeToContents)
        --d->contentsSections;

    if (d->hasAutoResizeSections())
        resizeSections(); // section sizes may change as a result of the new mode
}

/*!
    Returns the resize mode that applies to the section specified by the given \a logicalIndex.

    \sa setResizeMode()
*/

QHeaderView::ResizeMode QHeaderView::resizeMode(int logicalIndex) const
{
    Q_D(const QHeaderView);
    int visual = visualIndex(logicalIndex);
    Q_ASSERT(visual != -1);
    return d->visualIndexResizeMode(visual);
}

/*!
    \since 4.1

    Returns the number of sections that are set to resize mode stretch.
    In views this can be used to see if the headerview needs to resize the sections when the view geometry changes.

    \sa stretchLastSection, resizeMode()
*/

int QHeaderView::stretchSectionCount() const
{
    Q_D(const QHeaderView);
    return d->stretchSections;
}

/*!
  \property QHeaderView::showSortIndicator
  \brief whether the sort indicator is shown

  \sa setClickable()
*/

void QHeaderView::setSortIndicatorShown(bool show)
{
    Q_D(QHeaderView);
    d->sortIndicatorShown = show;

    if (sortIndicatorSection() < 0 || sortIndicatorSection() > count())
        return;

    if (d->visualIndexResizeMode(sortIndicatorSection()) == ResizeToContents) {
        resizeSections();
        d->viewport->update();
    }
}

bool QHeaderView::isSortIndicatorShown() const
{
    Q_D(const QHeaderView);
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
    Q_D(QHeaderView);

    Q_ASSERT(logicalIndex >= 0);

    // This is so that people can set the position of the sort indicator before the fill the model
    int old = d->sortIndicatorSection;
    d->sortIndicatorSection = logicalIndex;
    d->sortIndicatorOrder = order;

    if (logicalIndex >= d->sectionCount)
        return; // nothing to do

    if (old != logicalIndex && resizeMode(logicalIndex) == ResizeToContents) {
        resizeSections();
        d->viewport->update();
    } else {
        if (old != logicalIndex)
            updateSection(old);
        updateSection(logicalIndex);
    }
}

/*!
    Returns the logical index of the section that has a sort
    indicator. By default this is section 0.

    \sa setSortIndicator() sortIndicatorOrder() setSortIndicatorShown()
*/

int QHeaderView::sortIndicatorSection() const
{
    Q_D(const QHeaderView);
    return d->sortIndicatorSection;
}

/*!
    Returns the order for the sort indicator. If no section has a sort
    indicator the return value of this function is undefined.

    \sa setSortIndicator() sortIndicatorSection()
*/

Qt::SortOrder QHeaderView::sortIndicatorOrder() const
{
    Q_D(const QHeaderView);
    return d->sortIndicatorOrder;
}

/*!
    \property QHeaderView::stretchLastSection
    \brief whether the last visible section in the header takes up all the available space.  The default value is false.

    \sa setResizeMode()
*/
bool QHeaderView::stretchLastSection() const
{
    Q_D(const QHeaderView);
    return d->stretchLastSection;
}

void QHeaderView::setStretchLastSection(bool stretch)
{
    Q_D(QHeaderView);
    d->stretchLastSection = stretch;
    if (stretch)
        resizeSections();
    else if (count())
        resizeSection(count() -1, d->defaultSectionSize);
}

/*!
    \property QHeaderView::defaultSectionSize
    \brief the default size of the header sections before resizing.

*/
int QHeaderView::defaultSectionSize() const
{
    Q_D(const QHeaderView);
    return d->defaultSectionSize;
}

void QHeaderView::setDefaultSectionSize(int size)
{
    Q_D(QHeaderView);
    d->defaultSectionSize = size;
}

Qt::Alignment QHeaderView::defaultAlignment() const
{
    Q_D(const QHeaderView);
    return d->defaultAlignment;
}

/*!
    \since 4.1
    \property QHeaderView::defaultAlignment
    \brief the default alignment of the text in each header section
*/
void QHeaderView::setDefaultAlignment(Qt::Alignment alignment)
{
    Q_D(QHeaderView);
    d->defaultAlignment = alignment;
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
  Returns true if sections in the header has been moved;
  otherwise returns false;

  \sa moveSection()
*/
bool QHeaderView::sectionsMoved() const
{
    Q_D(const QHeaderView);
    return !d->visualIndices.isEmpty();
}

/*!
    \since 4.1

    Returns true if sections in the header has been hidden;
    otherwise returns false;

    \sa setSectionHidden()
*/
bool QHeaderView::sectionsHidden() const
{
    Q_D(const QHeaderView);
    return !d->hiddenSectionSize.isEmpty();
}

/*!
  Updates the changed header sections with the given \a orientation, from
  \a logicalFirst to \a logicalLast inclusive.
*/
void QHeaderView::headerDataChanged(Qt::Orientation orientation, int logicalFirst, int logicalLast)
{
    Q_D(QHeaderView);
    if (d->orientation != orientation)
        return;
    // Before initilization the size isn't set
    if (count() == 0)
        return;

    Q_ASSERT(logicalFirst >= 0);
    Q_ASSERT(logicalLast >= 0);
    Q_ASSERT(logicalFirst < count());
    Q_ASSERT(logicalLast < count());

    d->invalidateCachedSizeHint();

    if (orientation == Qt::Horizontal) {
        int left = sectionViewportPosition(logicalFirst);
        int right = sectionViewportPosition(logicalLast);
        right += sectionSize(logicalLast);
        d->viewport->update(left, 0, right - left, d->viewport->height());
    } else {
        int top = sectionViewportPosition(logicalFirst);
        int bottom = sectionViewportPosition(logicalLast);
        bottom += sectionSize(logicalLast);
        d->viewport->update(0, top, d->viewport->width(), bottom - top);
    }
}

/*!
    \internal

    Updates the section specified by the given \a logicalIndex.
*/

void QHeaderView::updateSection(int logicalIndex)
{
    Q_D(QHeaderView);
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
    Q_D(QHeaderView);
    d->resizeSections(Interactive, false); // no global resize mode
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
    Q_D(QHeaderView);
    if (parent != rootIndex() || !d->model)
        return; // we only handle changes in the top level
    int lastSection;
    if (d->orientation == Qt::Horizontal)
        lastSection = qMax(d->model->columnCount(rootIndex()) - 1, 0);
    else
        lastSection = qMax(d->model->rowCount(rootIndex()) -  1, 0);
    initializeSections(logicalFirst, lastSection);
    d->invalidateCachedSizeHint();
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
    Q_UNUSED(parent);
    Q_UNUSED(logicalFirst);
    Q_UNUSED(logicalLast);
}

void QHeaderViewPrivate::sectionsRemoved(const QModelIndex &parent,
                                           int logicalFirst, int logicalLast)
{
    Q_Q(QHeaderView);
    if (parent != q->rootIndex() || !model)
        return; // we only handle changes in the top level
    if (qMin(logicalFirst, logicalLast) < 0
        || qMax(logicalLast, logicalFirst) >= sectionCount)
        return; // should could assert here, but since models could emit signals with strange args, we are a bit forgiving
    int oldCount = q->count();
    int changeCount = logicalLast - logicalFirst + 1;
    if (visualIndices.isEmpty() && logicalIndices.isEmpty()) {
        for (int i = logicalFirst; i <= changeCount+logicalFirst; ++i)
            hiddenSectionSize.remove(i);
        //Q_ASSERT(headerSectionCount() == sectionCount);
        removeSectionsFromSpans(logicalFirst, logicalLast);
    } else {
        for (int l = logicalLast; l >= logicalFirst; --l) {
            int visual = visualIndices.at(l);
            for (int v = 0; v < sectionCount; ++v) {
                if (logicalIndex(v) > l) // no need to move the positions before l
                    --(logicalIndices[v]);
                if (v > visual) {
                    int logical = logicalIndex(v);
                    --(visualIndices[logical]);
                }
            }
            hiddenSectionSize.remove(l);
            logicalIndices.remove(visual);
            visualIndices.remove(l);
            //Q_ASSERT(headerSectionCount() == sectionCount);
            removeSectionsFromSpans(visual, visual);
        }
        // ### handle sectionSelection, sectionResizeMode, sectionHidden
    }
    sectionCount -= changeCount;

    // if we only have the last section (the "end" position) left, the header is empty
    if (sectionCount <= 0)
        clear();
    invalidateCachedSizeHint();
    emit q->sectionCountChanged(oldCount, q->count());
    viewport->update();
}

/*!
  \internal
*/

void QHeaderView::initializeSections()
{
    Q_D(QHeaderView);
    if (!model()) {
        d->clear();
        return;
    }
    if (d->orientation == Qt::Horizontal) {
        int c = model()->columnCount(rootIndex());
        if (c == 0) {
            int oldCount = count();
            d->clear();
            emit sectionCountChanged(oldCount, 0);
        } else if (c != count() && c > 0) {
            initializeSections(0, c - 1);
        }
    } else {
        int r = model()->rowCount(rootIndex());
        if (r == 0) {
            int oldCount = count();
            d->clear();
            emit sectionCountChanged(oldCount, 0);
        } else if (r != count() && r > 0) {
            initializeSections(0, r - 1);
        }
    }
}

/*!
    \internal
*/

void QHeaderView::initializeSections(int start, int end)
{
    Q_D(QHeaderView);

    Q_ASSERT(start >= 0);
    Q_ASSERT(end >= 0);

    int oldCount = d->sectionCount;
    d->sectionCount = end + 1;

    if (!d->logicalIndices.isEmpty() && start > 0) {
        d->logicalIndices.resize(d->sectionCount);
        d->visualIndices.resize(d->sectionCount);
        for (int i = start; i < d->sectionCount; ++i){
            d->logicalIndices[i] = i;
            d->visualIndices[i] = i;
        }
    }

    if (d->globalResizeMode == Stretch)
        d->stretchSections = d->sectionCount;
    else if (d->globalResizeMode == ResizeToContents)
        d->contentsSections = d->sectionCount;

    d->createSectionSpan(start, end, (end - start + 1) * d->defaultSectionSize);
    //Q_ASSERT(d->headerLength() == d->length);

    emit sectionCountChanged(oldCount,  d->sectionCount);
    d->viewport->update();
}

/*!
  \reimp
*/

void QHeaderView::currentChanged(const QModelIndex &current, const QModelIndex &old)
{
    Q_D(QHeaderView);

    if (d->orientation == Qt::Horizontal && current.column() != old.column()) {
        if (old.isValid() && old.parent() == rootIndex())
            d->setDirtyRegion(QRect(sectionViewportPosition(old.column()), 0,
                                    sectionSize(old.column()), d->viewport->height()));
        if (current.isValid() && current.parent() == rootIndex())
            d->setDirtyRegion(QRect(sectionViewportPosition(current.column()), 0,
                                    sectionSize(current.column()), d->viewport->height()));
    } else if (d->orientation == Qt::Vertical && current.row() != old.row()) {
        if (old.isValid() && old.parent() == rootIndex())
            d->setDirtyRegion(QRect(0, sectionViewportPosition(old.row()),
                                    d->viewport->width(), sectionSize(old.row())));
        if (current.isValid() && current.parent() == rootIndex())
            d->setDirtyRegion(QRect(0, sectionViewportPosition(current.row()),
                                    d->viewport->width(), sectionSize(current.row())));
    }
    d->updateDirtyRegion();
}


/*!
  \reimp
*/

bool QHeaderView::event(QEvent *e)
{
    Q_D(QHeaderView);
    switch (e->type()) {
    case QEvent::HoverEnter: {
        QHoverEvent *he = static_cast<QHoverEvent*>(e);
        d->hover = logicalIndexAt(he->pos());
        if (d->hover != -1)
            updateSection(d->hover);
        break; }
    case QEvent::HoverLeave: {
        if (d->hover != -1)
            updateSection(d->hover);
        d->hover = -1;
        break; }
    case QEvent::HoverMove: {
        QHoverEvent *he = static_cast<QHoverEvent*>(e);
        int oldHover = d->hover;
        d->hover = logicalIndexAt(he->pos());
        if (d->hover != oldHover) {
            if (oldHover != -1)
                updateSection(oldHover);
            if (d->hover != -1)
                updateSection(d->hover);
        }
        break; }
    default:
        break;
    }
    return QAbstractItemView::event(e);
}

/*!
  \reimp
*/

void QHeaderView::paintEvent(QPaintEvent *e)
{
    Q_D(QHeaderView);

    if (d->model == 0 || count() == 0)
        return;

    QPainter painter(d->viewport);
    const QPoint offset = d->scrollDelayOffset;
    QRect area = e->rect();
    area.translate(offset);

    int start = -1;
    int end = -1;
    if (orientation() == Qt::Horizontal) {
        start = visualIndexAt(area.left());
        end = visualIndexAt(area.right());
    } else {
        start = visualIndexAt(area.top());
        end = visualIndexAt(area.bottom());
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

    d->prepareSectionSelected(); // clear and resize the bit array

    QRect rect;
    int logical;
    bool highlight = false;
    const int width = d->viewport->width();
    const int height = d->viewport->height();
    const bool active = isActiveWindow();
    const QFont fnt(painter.font()); // save the painter font
    for (int i = start; i <= end; ++i) {
        if (d->isVisualIndexHidden(i))
            continue;
        logical = logicalIndex(i);
        if (orientation() == Qt::Horizontal) {
            rect.setRect(sectionViewportPosition(logical), 0, sectionSize(logical), height);
            highlight = d->selectionModel->columnIntersectsSelection(logical, rootIndex());
        } else {
            rect.setRect(0, sectionViewportPosition(logical), width, sectionSize(logical));
            highlight = d->selectionModel->rowIntersectsSelection(logical, rootIndex());
        }
        rect.translate(offset);
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

#if 0
    // ### visualize section spans
    for (int a = 0, i = 0; i < d->sectionSpans.count(); ++i) {
        QColor color((i & 4 ? 255 : 0), (i & 2 ? 255 : 0), (i & 1 ? 255 : 0));
        if (orientation() == Qt::Horizontal)
            painter.fillRect(a - d->offset, 0, d->sectionSpans.at(i).size, 4, color);
        else
            painter.fillRect(0, a - d->offset, 4, d->sectionSpans.at(i).size, color);
        a += d->sectionSpans.at(i).size;
    }
 
#endif
}

/*!
  \reimp
*/

void QHeaderView::mousePressEvent(QMouseEvent *e)
{
    Q_D(QHeaderView);
    if (d->state != QHeaderViewPrivate::NoState || e->button() != Qt::LeftButton)
        return;
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
        if (d->clickableSections && d->pressed != -1) {
            updateSection(d->pressed);
            emit sectionPressed(d->pressed);
        }
    } else if (resizeMode(handle) == Interactive) {
        d->state = QHeaderViewPrivate::ResizeSection;
        d->section = handle;
    }
    d->firstPos = pos;
    d->lastPos = pos;
}

/*!
  \reimp
*/

void QHeaderView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QHeaderView);
    int pos = orientation() == Qt::Horizontal ? e->x() : e->y();
    if (pos < 0)
        return;
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
            if (qAbs(pos - d->firstPos) >= QApplication::startDragDistance()) {
                int indicatorCenter = (orientation() == Qt::Horizontal
                                       ? d->sectionIndicator->width()
                                       : d->sectionIndicator->height()) / 2;
                int centerOffset = indicatorCenter - d->sectionIndicatorOffset;
                // This will drop the moved section to the position under the center of the indicator.
                // If centerOffset is 0, the section will be moved to the position of the mouse cursor.
                int visual = visualIndexAt(pos + centerOffset);
                if (visual == -1)
                    return;
                d->target = d->logicalIndex(visual);
                d->updateSectionIndicator(d->section, pos);
            } else {
                int visual = visualIndexAt(d->firstPos);
                if (visual == -1)
                    return;
                d->target = d->logicalIndex(visual);
                d->updateSectionIndicator(d->section, d->firstPos);
            }
            return;
        }
        case QHeaderViewPrivate::NoState: {
#ifndef QT_NO_CURSOR
            int handle = d->sectionHandleAt(pos);
            if (handle != -1 && resizeMode(handle) == Interactive
                && !(handle == count() - 1 && stretchLastSection()))
                setCursor(orientation() == Qt::Horizontal ? Qt::SplitHCursor : Qt::SplitVCursor);
            else
                setCursor(Qt::ArrowCursor);
#endif
            return;
        }
    }
}

/*!
  \reimp
*/

void QHeaderView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QHeaderView);
    int pos = orientation() == Qt::Horizontal ? e->x() : e->y();
    switch (d->state) {
    case QHeaderViewPrivate::MoveSection:
        if (!d->sectionIndicator->isHidden()) { // moving
            int from = visualIndex(d->section);
            Q_ASSERT(from != -1);
            int to = visualIndex(d->target);
            Q_ASSERT(to != -1);
            moveSection(from, to);
            d->section = d->target = -1;
            d->updateSectionIndicator(d->section, pos);
            break;
        } // not moving
    case QHeaderViewPrivate::NoState:
        if (d->clickableSections) {
            int section = logicalIndexAt(pos);
            if (section != -1 && section == d->pressed)
                emit sectionClicked(logicalIndexAt(pos));
            if (d->pressed != -1)
                updateSection(d->pressed);
        }
        break;
    case QHeaderViewPrivate::ResizeSection:
        break;
    }
    d->state = QHeaderViewPrivate::NoState;
    d->pressed = -1;
}

/*!
  \reimp
*/

void QHeaderView::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_D(QHeaderView);
    int pos = orientation() == Qt::Horizontal ? e->x() : e->y();
    int handle = d->sectionHandleAt(pos);
    while (handle > -1 && isSectionHidden(handle)) handle--;
    if (handle > -1 && resizeMode(handle) == Interactive)
        emit sectionHandleDoubleClicked(handle);
    else
        emit sectionDoubleClicked(logicalIndexAt(e->pos()));
}

/*!
  \reimp
*/

bool QHeaderView::viewportEvent(QEvent *e)
{
    switch (e->type()) {
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip: {
        if (!isActiveWindow())
            break;
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        int logical = logicalIndexAt(he->pos());
        if (logical != -1) {
            QVariant variant = model()->headerData(logical, orientation(), Qt::ToolTipRole);
            if (variant.isValid()) {
                QToolTip::showText(he->globalPos(), variant.toString(), this);
                return true;
            }
        }
        break; }
#endif
#ifndef QT_NO_WHATSTHIS
    case QEvent::QueryWhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        int logical = logicalIndexAt(he->pos());
        if (logical != -1
            && model()->headerData(logical, orientation(), Qt::WhatsThisRole).isValid())
            return true;
        break; }
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        int logical = logicalIndexAt(he->pos());
        if (logical != -1) {
             QString whatsthis = model()->headerData(logical, orientation(),
                                                     Qt::WhatsThisRole).toString();
             QWhatsThis::showText(he->globalPos(), whatsthis, this);
             return true;
        }
        break; }
#endif // QT_NO_WHATSTHIS
#ifndef QT_NO_STATUSTIP
    case QEvent::StatusTip: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        int logical = logicalIndexAt(he->pos());
        if (logical != -1) {
            QString statustip = model()->headerData(logical, orientation(),
                                                    Qt::StatusTipRole).toString();
            if (!statustip.isEmpty())
                setStatusTip(statustip);
        }
        return true; }
#endif // QT_NO_STATUSTIP
    default:
        break;
    }
    return QAbstractItemView::viewportEvent(e);
}

/*!
    Paints the section specified by the given \a logicalIndex, using the given \a painter and \a rect.

    You normally would not need to use this function.
*/

void QHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    Q_D(const QHeaderView);
    if (!d->model || !rect.isValid())
        return;
    // get the state of the section
    QStyleOptionHeader opt = d->getStyleOption();
    QStyle::State state = QStyle::State_None;
    if (isEnabled())
        state |= QStyle::State_Enabled;
    if (window()->isActiveWindow())
        state |= QStyle::State_Active;
    if (d->clickableSections) {
        if (logicalIndex == d->hover)
            state |= QStyle::State_MouseOver;
        if (logicalIndex == d->pressed)
            state |= QStyle::State_Sunken;
        else if (d->highlightSelected && d->isSectionSelected(logicalIndex))
            state |= QStyle::State_On | QStyle::State_Sunken;
    }
    if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex)
        opt.sortIndicator = (sortIndicatorOrder() == Qt::AscendingOrder)
                            ? QStyleOptionHeader::SortDown : QStyleOptionHeader::SortUp;

    // setup the style options structure
    QVariant textAlignment = d->model->headerData(logicalIndex, orientation(),
                                                  Qt::TextAlignmentRole);
    opt.rect = rect;
    opt.section = logicalIndex;
    opt.state |= state;
    opt.textAlignment = Qt::Alignment(textAlignment.isValid()
                                      ? Qt::Alignment(textAlignment.toInt())
                                      : d->defaultAlignment);
    opt.iconAlignment = Qt::AlignVCenter;
    opt.text = d->model->headerData(logicalIndex, orientation(),
                                    Qt::DisplayRole).toString();
    opt.icon = qvariant_cast<QIcon>(d->model->headerData(logicalIndex, orientation(),
                                    Qt::DecorationRole));

    QVariant textColor = d->model->headerData(logicalIndex, orientation(),
                                    Qt::TextColorRole);
    if (textColor.isValid() && qvariant_cast<QColor>(textColor).isValid())
        opt.palette.setColor(QPalette::ButtonText, qvariant_cast<QColor>(textColor));

    // the section position
    int visual = visualIndex(logicalIndex);
    Q_ASSERT(visual != -1);
    if (count() == 1)
        opt.position = QStyleOptionHeader::OnlyOneSection;
    else if (visual == 0)
        opt.position = QStyleOptionHeader::Beginning;
    else if (visual == count() - 1)
        opt.position = QStyleOptionHeader::End;
    else
        opt.position = QStyleOptionHeader::Middle;
    opt.orientation = d->orientation;
    // the selected position
    bool previousSelected = d->isSectionSelected(this->logicalIndex(visual - 1));
    bool nextSelected =  d->isSectionSelected(this->logicalIndex(visual + 1));
    if (previousSelected && nextSelected)
        opt.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
    else if (previousSelected)
        opt.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
    else if (nextSelected)
        opt.selectedPosition = QStyleOptionHeader::NextIsSelected;
    else
        opt.selectedPosition = QStyleOptionHeader::NotAdjacent;
    // draw the section
    style()->drawControl(QStyle::CE_Header, &opt, painter, this);
}

/*!
  Returns the size of the contents of the section specified by the give \a logicalIndex.
*/

QSize QHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    Q_D(const QHeaderView);
    Q_ASSERT(logicalIndex >= 0);
    if (!d->model)
        return QSize();
    QSize size(100, 30);
    QStyleOptionHeader opt = d->getStyleOption();
    QFont fnt = font();
    fnt.setBold(true);
    opt.fontMetrics = QFontMetrics(fnt); // do the metrics with a bold font
    opt.text = d->model->headerData(logicalIndex, orientation(),
                                    Qt::DisplayRole).toString();
    opt.icon = qvariant_cast<QIcon>(d->model->headerData(logicalIndex, orientation(),
                                    Qt::DecorationRole));
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
    Q_D(const QHeaderView);
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
    Q_D(const QHeaderView);
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
    Q_D(QHeaderView);
    if (d->hasAutoResizeSections()) {
        setViewportMargins(0, 0, 0, 0); // ### force layoutChildren in QAbstractScrollArea
        resizeSections();
    }
}

/*!
  \reimp
  \internal
*/

void QHeaderView::scrollContentsBy(int dx, int dy)
{
    Q_D(QHeaderView);
    d->scrollDirtyRegion(dx, dy);
}

/*!
  \reimp
  \internal
*/
void QHeaderView::dataChanged(const QModelIndex &, const QModelIndex &)
{
    Q_D(QHeaderView);
    d->invalidateCachedSizeHint();
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

QRect QHeaderView::visualRect(const QModelIndex &) const
{
    return QRect();
}

/*!
  \reimp
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

void QHeaderView::scrollTo(const QModelIndex &, ScrollHint)
{
    // do nothing - the header only displays sections
}

/*!
  \reimp
  \internal

  Empty implementation because the header doesn't show QModelIndex items.
*/

QModelIndex QHeaderView::indexAt(const QPoint &) const
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

QModelIndex QHeaderView::moveCursor(CursorAction, Qt::KeyboardModifiers)
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
*/

QRegion QHeaderView::visualRegionForSelection(const QItemSelection &selection) const
{
    Q_D(const QHeaderView);
    if (!d->model)
        return QRegion();
    if (orientation() == Qt::Horizontal) {
        int left = d->model->columnCount(rootIndex()) - 1;
        int right = 0;
        int rangeLeft, rangeRight;

        for (int i = 0; i < selection.count(); ++i) {
            QItemSelectionRange r = selection.at(i);
            if (r.parent().isValid() || !r.isValid())
                continue; // we only know about toplevel items and we don't want invalid ranges
            // FIXME an item inside the range may be the leftmost or rightmost
            rangeLeft = visualIndex(r.left());
            if (rangeLeft == -1) // in some cases users may change the selections
                continue;        // before we have a chance to do the layout
            rangeRight = visualIndex(r.right());
            if (rangeRight == -1) // in some cases users may change the selections
                continue;         // before we have a chance to do the layout
            if (rangeLeft < left)
                left = rangeLeft;
            if (rangeRight > right)
                right = rangeRight;
        }

        int logicalLeft = logicalIndex(left);
        int logicalRight = logicalIndex(right);

        if (logicalLeft < 0  || logicalLeft >= count() ||
            logicalRight < 0 || logicalRight >= count())
            return QRegion();

        int leftPos = sectionViewportPosition(logicalLeft);
        int rightPos = sectionViewportPosition(logicalRight);
        rightPos += sectionSize(logicalRight);
        return QRect(leftPos, 0, rightPos - leftPos, height());
    }
    // orientation() == Qt::Vertical
    int top = d->model->rowCount(rootIndex()) - 1;
    int bottom = 0;
    int rangeTop, rangeBottom;

    for (int i = 0; i < selection.count(); ++i) {
        QItemSelectionRange r = selection.at(i);
        if (r.parent().isValid() || !r.isValid())
            continue; // we only know about toplevel items
        // FIXME an item inside the range may be the leftmost or rightmost
        rangeTop = visualIndex(r.top());
        if (rangeTop == -1) // in some cases users may change the selections
            continue;       // before we have a chance to do the layout
        rangeBottom = visualIndex(r.bottom());
        if (rangeBottom == -1) // in some cases users may change the selections
            continue;          // before we have a chance to do the layout
        if (rangeTop < top)
            top = rangeTop;
        if (rangeBottom > bottom)
            bottom = rangeBottom;
    }

    int logicalTop = logicalIndex(top);
    int logicalBottom = logicalIndex(bottom);

    if (logicalTop == -1 || logicalBottom == -1)
        return QRect();

    int topPos = sectionViewportPosition(logicalTop);
    int bottomPos = sectionViewportPosition(logicalBottom) + sectionSize(logicalBottom);

    return QRect(0, topPos, width(), bottomPos - topPos);
}


// private implementation

int QHeaderViewPrivate::sectionHandleAt(int position)
{
    Q_Q(QHeaderView);
    int visual = q->visualIndexAt(position);
    if (visual == -1)
        return -1;
    int log = logicalIndex(visual);
    int pos = q->sectionViewportPosition(log);
    if (pos < 0)
        return -1;
    int grip = q->style()->pixelMetric(QStyle::PM_HeaderGripMargin);
    if (reverse()) {
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
    Q_Q(QHeaderView);
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
    sectionIndicatorOffset = position - qMax(p, 0);

}

void QHeaderViewPrivate::updateSectionIndicator(int section, int position)
{
    if (section == -1 || target == -1) {
        sectionIndicator->hide();
        return;
    }

    if (orientation == Qt::Horizontal)
        sectionIndicator->move(position - sectionIndicatorOffset, 0);
    else
        sectionIndicator->move(0, position - sectionIndicatorOffset);

    sectionIndicator->show();
}

QStyleOptionHeader QHeaderViewPrivate::getStyleOption() const
{
    Q_Q(const QHeaderView);
    QStyleOptionHeader opt;
    opt.rect = q->rect();
    opt.palette = q->palette();
    opt.state = QStyle::State_None | QStyle::State_Raised;
    opt.orientation = orientation;
    if (orientation == Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;
    if (q->isEnabled())
        opt.state |= QStyle::State_Enabled;
    opt.section = 0;
    return opt;
}

bool QHeaderViewPrivate::isSectionSelected(int section) const
{
    Q_Q(const QHeaderView);
    int i = section * 2;
    if (i < 0 || i >= sectionSelected.count())
        return false;
    if (sectionSelected.testBit(i)) // if the value was cached
        return sectionSelected.testBit(i + 1);
    bool s = false;
    if (orientation == Qt::Horizontal)
        s = q->selectionModel()->isColumnSelected(section, QModelIndex());
    else
        s = q->selectionModel()->isRowSelected(section, QModelIndex());
    sectionSelected.setBit(i + 1, s); // selection state
    sectionSelected.setBit(i, true); // cache state
    return s;
}

/*!
    Go through and reize all of the sections appling stretchLastSection, manualy stretches, sizes, and useGlobalMode
 */
void QHeaderViewPrivate::resizeSections(QHeaderView::ResizeMode globalMode, bool useGlobalMode)
{
    Q_Q(QHeaderView);

    executePostedLayout();
    if (sectionCount == 0)
        return;
    invalidateCachedSizeHint();

    // If stretchLastSection, find it
    int stretchSection = -1;
    if (stretchLastSection && !useGlobalMode) {
        for (int i = sectionCount - 1; i >= 0; --i) {
            if (!isVisualIndexHidden(i)) {
                stretchSection = i;
                break;
            }
        }
    }

    // Count up the number of strected sections and how much space left for them
    int lengthToStrech = (orientation == Qt::Horizontal ? viewport->width() : viewport->height());
    int numberOfStretchedSections = 0;
    QList<int> section_sizes; //Store for later so they don't have to be re-calculated
    for (int i = 0; i < sectionCount; ++i) {
        if (isVisualIndexHidden(i))
            continue;

        QHeaderView::ResizeMode resizeMode;
        if (useGlobalMode)
            resizeMode = globalMode;
        else
            resizeMode = (i == stretchSection ? QHeaderView::Stretch : visualIndexResizeMode(i));

        if (resizeMode == QHeaderView::Stretch) {
            ++numberOfStretchedSections;
            continue;
        }

        // because it isn't stretch determine its width and remove that from lengthToStrech
        int sectionSize = 0;
        if (resizeMode == QHeaderView::Interactive || resizeMode == QHeaderView::Custom) {
            sectionSize = headerSectionSize(i);
        } else { // resizeMode == QHeaderView::ResizeToContents
            int logicalIndex = q->logicalIndex(i);
            QAbstractItemView *parent = ::qobject_cast<QAbstractItemView*>(q->parent());
            if (parent)
                sectionSize = (orientation == Qt::Horizontal
                               ? parent->sizeHintForColumn(logicalIndex)
                               : parent->sizeHintForRow(logicalIndex));
            sectionSize = qMax(sectionSize, q->sectionSizeHint(logicalIndex));
        }
        section_sizes.append(sectionSize);
        lengthToStrech -= sectionSize;
    }

    // Calculate the new length for all of the stretched sections
    int stretchSectionLength = 0;
    int pixelReminder = 0;
    if (numberOfStretchedSections > 0) {
        QSize strut = QApplication::globalStrut();
        int minimumLength = orientation == Qt::Horizontal
                  ? qMax(strut.width(), q->fontMetrics().maxWidth())
                  : qMax(strut.height(), q->fontMetrics().height());
        int hintLengthForEveryStretchedSection = lengthToStrech / numberOfStretchedSections;
        stretchSectionLength = qMax(hintLengthForEveryStretchedSection, minimumLength);
        pixelReminder = lengthToStrech % numberOfStretchedSections;
    }

    int spanStartSection = 0;
    int previousSectionLength = 0;

    // set the position of each section along the total length
    for (int i = 0; i < sectionCount; ++i) {

        int oldSectionLength = headerSectionSize(i);
        int newSectionLength = -1;
            
        if (isVisualIndexHidden(i)) {
            newSectionLength = 0;
        } else {
            QHeaderView::ResizeMode resizeMode;
            if (useGlobalMode)
                resizeMode = globalMode;
            else
                resizeMode = (i == stretchSection ? QHeaderView::Stretch : visualIndexResizeMode(i));
            if (resizeMode == QHeaderView::Stretch) {
                newSectionLength = stretchSectionLength;
                if (pixelReminder > 0) {
                    newSectionLength += 1;
                    --pixelReminder;
                }
            } else {
                newSectionLength = section_sizes.front();
                section_sizes.removeFirst();
            }
        }

        //Q_ASSERT(newSectionLength > 0);
        if (previousSectionLength != newSectionLength && i > 0) {
            int spanLength = (i - spanStartSection) * previousSectionLength;
            createSectionSpan(spanStartSection, i - 1, spanLength);
            //Q_ASSERT(headerLength() == length);
            spanStartSection = i;
        }

        if (newSectionLength != oldSectionLength)
            emit q->sectionResized(i, oldSectionLength, newSectionLength);

        previousSectionLength = newSectionLength;
    }

    createSectionSpan(spanStartSection, sectionCount - 1,
                      (sectionCount - spanStartSection) * previousSectionLength);
    //Q_ASSERT(headerLength() == length);

    viewport->update();
}

void QHeaderViewPrivate::createSectionSpan(int start, int end, int size)
{
    // ### the code for merging spans does not merge at all opertuneties
    // ### what if the number of sections is reduced ?

    SectionSpan span(size, (end - start) + 1);
    int start_section = 0;
    int initial_section_count = headerSectionCount(); // ### debug code
    
    QList<int> spansToRemove;
    for (int i = 0; i < sectionSpans.count(); ++i) {
        int end_section = start_section + sectionSpans.at(i).count - 1;
        int section_count = sectionSpans.at(i).count;
        if (start <= start_section && end > end_section) {
            // the existing span is entirely coveded by the new span
            spansToRemove.append(i);
        } else if (start < start_section && end >= end_section) {
            // the existing span is entirely coveded by the new span
            spansToRemove.append(i);
        } else if (start == start_section && end == end_section) {
            // the new span is covered by an existin span
            length -= sectionSpans.at(i).size;
            length += size;
            sectionSpans[i].size = size;
            // ### check if we can merge the section with any of its neighbours
            removeSpans(spansToRemove);
            Q_ASSERT(initial_section_count == headerSectionCount());
            return;
        } else if (start > start_section && end < end_section) {
            if (sectionSpans.at(i).sectionSize() == span.sectionSize()) {
                Q_ASSERT(initial_section_count == headerSectionCount());
                return;
            }
            // the new span is in the middle of the old span, so we have to split it
            length -= sectionSpans.at(i).size;
            int section_size = sectionSpans.at(i).sectionSize();
            int span_count = sectionSpans.at(i).count;
            // first span
            int first_span_count = start - start_section;
            int first_span_size = section_size * first_span_count;
            sectionSpans[i].count = first_span_count;
            sectionSpans[i].size = first_span_size;
            length += first_span_size;
            // middle span
            int mid_span_count = span.count;
            int mid_span_size = span.size;
            sectionSpans.insert(i + 1, span);
            length += mid_span_size;
            // last span
            int last_span_count = end_section - end;
            int last_span_size = section_size * last_span_count;
            sectionSpans.insert(i + 2, SectionSpan(last_span_size, last_span_count));
            length += last_span_size;
            Q_ASSERT(span_count == first_span_count + mid_span_count + last_span_count);
            removeSpans(spansToRemove);
            Q_ASSERT(initial_section_count == headerSectionCount());
            return;
        } else if (start > start_section && start <= end_section && end >= end_section) {
            // the new span covers the last part of the existing span
            length -= sectionSpans.at(i).size;
            int removed_count = (end_section - start + 1);
            int span_count = sectionSpans.at(i).count - removed_count;
            int section_size = sectionSpans.at(i).sectionSize();
            int span_size = section_size * span_count;
            sectionSpans[i].count = span_count;
            sectionSpans[i].size = span_size;
            length += span_size;
            if (end == end_section) {
                sectionSpans.insert(i + 1, span); // insert after
                length += span.size;                
                removeSpans(spansToRemove);
                Q_ASSERT(initial_section_count == headerSectionCount());
                return;
            }
        } else if (end < end_section && end >= start_section && start <= start_section) {
            // the new span covers the first part of the existing span
            length -= sectionSpans.at(i).size;
            int removed_count = (end - start_section + 1);
            int section_size = sectionSpans.at(i).sectionSize();
            int span_count = sectionSpans.at(i).count - removed_count;
            int span_size = section_size * span_count;
            sectionSpans[i].count = span_count;
            sectionSpans[i].size = span_size;
            length += span_size;
            sectionSpans.insert(i, span); // insert before
            length += span.size;
            removeSpans(spansToRemove);
            Q_ASSERT(initial_section_count == headerSectionCount());
            return;
        }
        start_section += section_count;
    }
        
    // ### adding and removing _ sections_  in addition to spans
    // ### add some more checks here

    if (spansToRemove.isEmpty()) {
        if (!sectionSpans.isEmpty() && sectionSpans.last().sectionSize() == span.sectionSize()) {
            length += span.size;
            int last = sectionSpans.count() - 1;
            sectionSpans[last].count += span.count;
            sectionSpans[last].size += span.size;
        } else {
            length += span.size;
            sectionSpans.append(span);
        }
    } else {
        removeSpans(spansToRemove);
        length += span.size;
        sectionSpans.insert(spansToRemove.first(), span);
        //Q_ASSERT(initial_section_count == headerSectionCount());
    }
}

void QHeaderViewPrivate::removeSectionsFromSpans(int start, int end)
{
    // remove sections
    int start_section = 0;
    QList<int> spansToRemove;
    for (int i = 0; i < sectionSpans.count(); ++i) {
        int end_section = start_section + sectionSpans.at(i).count - 1;
        int section_size = sectionSpans.at(i).sectionSize();
        int section_count = sectionSpans.at(i).count;
        if (start <= start_section && end >= end_section) {
            // the change covers the entire span
            spansToRemove.append(i);
            if (end == end_section)
                break;
        } else if (start > start_section && end < end_section) {
            // all the removed sections are inside the span
            int change = (end - start + 1);
            sectionSpans[i].count -= change;
            sectionSpans[i].size = section_size * sectionSpans.at(i).count;
            length -= (change * section_size);
            break;
        } else if (start >= start_section && start <= end_section) {
            // the some of the removed sections are inside the span,at the end
            int change = qMin(end_section - start + 1, end - start + 1);
            sectionSpans[i].count -= change;
            sectionSpans[i].size = section_size * sectionSpans.at(i).count;
            start += change;
            length -= (change * section_size);
            // the change affects several spans
        } else if (end >= start_section && end <= end_section) {
            // the some of the removed sections are inside the span, at the begining
            int change = qMin((end - start_section + 1), end - start + 1);
            sectionSpans[i].count -= change;
            sectionSpans[i].size = section_size * sectionSpans.at(i).count;
            length -= (change * section_size);
            break;
        }
        start_section += section_count;
    }

    for (int i = spansToRemove.count() - 1; i >= 0; --i) {
        int s = spansToRemove.at(i);
        length -= sectionSpans.at(s).size;
        sectionSpans.remove(s);
        // ### merge remaining spans
    }
}

void QHeaderViewPrivate::clear()
{
    length = 0;
    sectionCount = 0;
    sectionResizeMode.clear();
    visualIndices.clear();
    logicalIndices.clear();
    sectionSelected.clear();
    sectionHidden.clear();
    hiddenSectionSize.clear();
    sectionSpans.clear();
}

int QHeaderViewPrivate::headerSectionSize(int visual) const
{
    // ### stupid iteration; use binary search
    int section_start = 0;
    for (int i = 0; i < sectionSpans.count(); ++i) {
        int section_end = section_start + sectionSpans.at(i).count - 1;
        if (visual >= section_start && visual <= section_end)
            return sectionSpans.at(i).sectionSize();
        section_start = section_end + 1;
    }
    return -1;
}

int QHeaderViewPrivate::headerSectionPosition(int visual) const
{
    // ### stupid iteration; use binary search
    int section_start = 0;
    int span_position = 0;
    for (int i = 0; i < sectionSpans.count(); ++i) {
        int section_end = section_start + sectionSpans.at(i).count - 1;
        if (visual >= section_start && visual <= section_end)
            return span_position + (visual - section_start) * sectionSpans.at(i).sectionSize();
        section_start = section_end + 1;
        span_position += sectionSpans.at(i).size;
    }
    return -1;
}

int QHeaderViewPrivate::headerVisualIndexAt(int position) const
{
    // ### stupid iteration; use binary search
    int span_start_section = 0;
    int span_position = 0;
    for (int i = 0; i < sectionSpans.count(); ++i) {
        int next_span_start_section = span_start_section + sectionSpans.at(i).count;
        int next_span_position = span_position + sectionSpans.at(i).size;
        if (position == span_position)
            return span_start_section;// spans with no size
        if (position > span_position && position < next_span_position) {
            int position_in_span = position - span_position;
            return span_start_section + (position_in_span / sectionSpans.at(i).sectionSize());
        }
        span_start_section = next_span_start_section;
        span_position = next_span_position;
    }
    return -1;
}

#endif // QT_NO_ITEMVIEWS

#include "moc_qheaderview.cpp"
