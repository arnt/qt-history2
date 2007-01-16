/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*!
    \class QMdiArea
    \brief The QMdiArea widget provides a workspace window that can be
    used in an MDI application.
    \ingroup application
    \mainclass

    Multiple Document Interface (MDI) applications are typically
    composed of a main window containing a menu bar, a toolbar, and
    a central QMdiArea widget. The workspace itself is used to display
    a number of child windows, each of which is a widget.

    The workspace itself is an ordinary Qt widget. It has a standard
    constructor that takes a parent widget.
    Workspaces can be placed in any layout, but are typically given
    as the central widget in a QMainWindow:

    \quotefromfile mainwindows/mdi/mainwindow.cpp
    \skipto MainWindow::MainWindow()
    \printuntil setCentralWidget(workspace);
    \dots
    \skipto /^\}/
    \printuntil /^\}/

    Child windows (MDI windows) are standard Qt widgets that are
    inserted into the workspace with addSubWindow(). As with top-level
    widgets, you can call functions such as show(), hide(),
    showMaximized(), and setWindowTitle() on a child window to change
    its appearance within the workspace. You can also provide widget
    flags to determine the layout of the decoration or the behavior of
    the widget itself.

    To change or retrieve the geometry of a child window, you must
    operate on its parentWidget(). The parentWidget() provides
    access to the decorated frame that contains the child window
    widget. When a child window is maximised, its decorated frame
    is hidden. If the top-level widget contains a menu bar, it will display
    the maximised window's operations menu to the left of the menu
    entries, and the window's controls to the right.

    A child window becomes active when it gets the keyboard focus,
    or when setFocus() is called. The user can activate a window by moving
    focus in the usual ways, for example by clicking a window or by pressing
    Tab. The workspace emits a signal subWindowActivated() when the active
    window changes, and the function activeWindow() returns a pointer to the
    active child window, or 0 if no window is active.

    The convenience function windowList() returns a list of all child
    windows. This information could be used in a popup menu
    containing a list of windows, for example. This feature is also
    available as part of the \l{Window Menu} Solution.

    QMdiArea provides two built-in layout strategies for child
    windows: cascadeSubWindows() and tileSubWindows(). Both are slots so you can easily
    connect menu entries to them.

    \img qmdiarea-arrange.png

    If you want your users to be able to work with child windows
    larger than the visible workspace area, set the scrollBarsEnabled
    property to true.

    \sa QMdiSubWindow QDockWindow, {MDI Example}
*/

/*!
    \fn QMdiArea::subWindowActivated(QMdiSubWindow *window)

    QMdiArea emits this signal after \a window has been activated. When \a
    window is 0, QMdiArea has just deactivated its last active window, and
    there are no active windows on the workspace.

    \sa QMdiArea::activeSubWindow()
*/

/*!
    \enum QMdiArea::WindowOrder

    Specifies the order in which child windows are returned from windowList().

    \value CreationOrder The windows are returned in the order of their creation
    \value StackingOrder The windows are returned in the order of their stacking
*/

#include "qmdiarea_p.h"

#include <QApplication>
#include <QStyle>
#include <QChildEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtAlgorithms>
#include <QMutableListIterator>
#include <QPainter>
#include <QDebug>
#include <math.h>

// Asserts in debug mode, gives warning otherwise.
static bool inline sanityCheck(const QMdiSubWindow * const child, const char *where)
{
    if (!child) {
        const char error[] = "null pointer";
        Q_ASSERT_X(false, where, error);
        qWarning("%s:%s", where, error);
        return false;
    }
    return true;
}

static bool inline sanityCheck(const QList<QWidget *> &widgets, const int index, const char *where)
{
    if (index < 0 || index >= widgets.size()) {
        const char error[] = "index out of range";
        Q_ASSERT_X(false, where, error);
        qWarning("%s:%s", where, error);
        return false;
    }
    if (!widgets.at(index)) {
        const char error[] = "null pointer";
        Q_ASSERT_X(false, where, error);
        qWarning("%s:%s", where, error);
        return false;
    }
    return true;
}

static void inline setIndex(int *index, int candidate, int min, int max, bool isIncreasing)
{
    if (!index)
        return;

    if (isIncreasing) {
        if (candidate > max)
            *index = min;
        else
            *index = candidate;
    } else {
        if (candidate < min)
            *index = max;
        else
            *index = candidate;
    }
}

/*!
    \internal
*/
void RegularTiler::rearrange(QList<QWidget *> &widgets, const QRect &domain) const
{
    if (widgets.isEmpty())
        return;

    const int n = widgets.size();
    const int ncols = qMax(int(ceil(sqrt(float(n)))), 1);
    const int nrows = qMax((n % ncols) ? (n / ncols + 1) : (n / ncols), 1);
    const int nspecial = (n % ncols) ? (ncols - n % ncols) : 0;
    const qreal dx = domain.width()  / qreal(ncols);
    const qreal dy = domain.height() / qreal(nrows);

    int i = 0;
    for (int row = 0; row < nrows; ++row) {
        for (int col = 0; col < ncols; ++col) {
            if (row == 1 && col < nspecial)
                continue;
            const int x1 = int(col * dx);
            const int y1 = int(row * dy);
            int x2 = int(x1 + dx);
            int y2 = int(y1 + dy);
            if (row == 0 && col < nspecial)
                y2 *= 2;
            if (col == ncols - 1 && x2 != domain.right())
                x2 = domain.right();
            if (row == nrows - 1 && y2 != domain.bottom())
                y2 = domain.bottom();
            if (!sanityCheck(widgets, i, "RegularTiler"))
                continue;
            QWidget *widget = widgets.at(i++);
            QRect newGeometry = QRect(QPoint(x1, y1), QPoint(x2, y2));
            widget->setGeometry(QStyle::visualRect(widget->layoutDirection(), domain, newGeometry));
        }
    }
}

/*!
    \internal
*/
void SimpleCascader::rearrange(QList<QWidget *> &widgets, const QRect &domain) const
{
    if (widgets.isEmpty())
        return;

    // Tunables:
    const int topOffset = 0;
    const int bottomOffset = 50;
    const int leftOffset = 0;
    const int rightOffset = 100;
    const int dx = 10;
    const int dy = 20;

    const int n = widgets.size();
    const int nrows = qMax((domain.height() - (topOffset + bottomOffset)) / dy, 1);
    const int ncols = qMax(n / nrows + ((n % nrows) ? 1 : 0), 1);
    const int dcol = (domain.width() - (leftOffset + rightOffset)) / ncols;

    int i = 0;
    for (int row = 0; row < nrows; ++row) {
        for (int col = 0; col < ncols; ++col) {
            const int x = leftOffset + row * dx + col * dcol;
            const int y = topOffset + row * dy;
            if (!sanityCheck(widgets, i, "SimpleCascader"))
                continue;
            QWidget *widget = widgets.at(i++);
            QRect newGeometry = QRect(QPoint(x, y), widget->minimumSizeHint());
            widget->setGeometry(QStyle::visualRect(widget->layoutDirection(), domain, newGeometry));
            if (i == n)
                return;
        }
    }
}

/*!
    \internal
*/
void IconTiler::rearrange(QList<QWidget *> &widgets, const QRect &domain) const
{
    if (widgets.isEmpty() || !sanityCheck(widgets, 0, "IconTiler"))
        return;

    const int n = widgets.size();
    const int width = widgets.at(0)->width();
    const int height = widgets.at(0)->height();
    const int ncols = qMax(domain.width() / width, 1);
    const int nrows = n / ncols + ((n % ncols) ? 1 : 0);

    int i = 0;
    for (int row = 0; row < nrows; ++row) {
        for (int col = 0; col < ncols; ++col) {
            const int x = col * width;
            const int y = domain.height() - height - row * height;
            if (!sanityCheck(widgets, i, "IconTiler"))
                continue;
            QWidget *widget = widgets.at(i++);
            QPoint newPos(x, y);
            QRect newGeometry = QRect(newPos.x(), newPos.y(), widget->width(), widget->height());
            widget->setGeometry(QStyle::visualRect(widget->layoutDirection(), domain, newGeometry));
            if (i == n)
                return;
        }
    }
}

/*!
    \internal
    Calculates the accumulated overlap (intersection area) between 'source' and 'rects'.
*/
int MinOverlapPlacer::accumulatedOverlap(const QRect &source, const QList<QRect> &rects)
{
    int accOverlap = 0;
    foreach (QRect rect, rects) {
        QRect intersection = source.intersected(rect);
        accOverlap += intersection.width() * intersection.height();
    }
    return accOverlap;
}


/*!
    \internal
    Finds among 'source' the rectangle with the minimum accumulated overlap with the
    rectangles in 'rects'.
*/
QRect MinOverlapPlacer::findMinOverlapRect(const QList<QRect> &source, const QList<QRect> &rects)
{
    int minAccOverlap = -1;
    QRect minAccOverlapRect;
    foreach (QRect srcRect, source) {
        const int accOverlap = accumulatedOverlap(srcRect, rects);
        if (accOverlap < minAccOverlap || minAccOverlap == -1) {
            minAccOverlap = accOverlap;
            minAccOverlapRect = srcRect;
        }
    }
    return minAccOverlapRect;
}

/*!
    \internal
    Gets candidates for the final placement.
*/
void MinOverlapPlacer::getCandidatePlacements(const QSize &size, const QList<QRect> &rects,
                                              const QRect &domain,QList<QRect> &candidates)
{
    QSet<int> xset;
    QSet<int> yset;
    xset << domain.left() << domain.right() - size.width() + 1;
    yset << domain.top();
    if (domain.bottom() - size.height() + 1 >= 0)
        yset << domain.bottom() - size.height() + 1;
    foreach (QRect rect, rects) {
        xset << rect.right() + 1;
        yset << rect.bottom() + 1;
    }

    QList<int> xlist = xset.values();
    qSort(xlist.begin(), xlist.end());
    QList<int> ylist = yset.values();
    qSort(ylist.begin(), ylist.end());

    foreach (int y, ylist)
        foreach (int x, xlist)
            candidates << QRect(QPoint(x, y), size);
}

/*!
    \internal
    Finds all rectangles in 'source' not completely inside 'domain'. The result is stored
    in 'result' and also removed from 'source'.
*/
void MinOverlapPlacer::findNonInsiders(const QRect &domain, QList<QRect> &source,
                                       QList<QRect> &result)
{
    QMutableListIterator<QRect> it(source);
    while (it.hasNext()) {
        const QRect srcRect = it.next();
        if (!domain.contains(srcRect)) {
            result << srcRect;
            it.remove();
        }
    }
}

/*!
   \internal
    Finds all rectangles in 'source' that overlaps 'domain' by the maximum overlap area
    between 'domain' and any rectangle in 'source'. The result is stored in 'result'.
*/
void MinOverlapPlacer::findMaxOverlappers(const QRect &domain, const QList<QRect> &source,
                                          QList<QRect> &result)
{
    int maxOverlap = -1;
    foreach (QRect srcRect, source) {
        QRect intersection = domain.intersected(srcRect);
        const int overlap = intersection.width() * intersection.height();
        if (overlap >= maxOverlap || maxOverlap == -1) {
            if (overlap > maxOverlap) {
                maxOverlap = overlap;
                result.clear();
            }
            result << srcRect;
        }
    }
}

/*!
   \internal
    Finds among the rectangles in 'source' the best placement. Here, 'best' means the
    placement that overlaps the rectangles in 'rects' as little as possible while at the
    same time being as much as possible inside 'domain'.
*/
QPoint MinOverlapPlacer::findBestPlacement(const QRect &domain, const QList<QRect> &rects,
                                           QList<QRect> &source)
{
    QList<QRect> nonInsiders;
    findNonInsiders(domain, source, nonInsiders);

    if (!source.empty())
        return findMinOverlapRect(source, rects).topLeft();

    QList<QRect> maxOverlappers;
    findMaxOverlappers(domain, nonInsiders, maxOverlappers);
    return findMinOverlapRect(maxOverlappers, rects).topLeft();
}


/*!
    \internal
    Places the rectangle defined by 'size' relative to 'rects' and 'domain' so that it
    overlaps 'rects' as little as possible and 'domain' as much as possible.
    Returns the position of the resulting rectangle.
*/
QPoint MinOverlapPlacer::place(const QSize &size, const QList<QRect> &rects,
                               const QRect &domain) const
{
    if (size.isEmpty() || !domain.isValid())
        return QPoint();
    foreach (QRect rect, rects)
        if (!rect.isValid())
            return QPoint();

    QList<QRect> candidates;
    getCandidatePlacements(size, rects, domain, candidates);
    return findBestPlacement(domain, rects, candidates);
}

/*!
    \internal
*/
QMdiAreaPrivate::QMdiAreaPrivate()
    : ignoreGeometryChange(false),
      isActivated(false),
      indexToNextWindow(-1),
      indexToPreviousWindow(-1)
{
    background.setColor(Qt::darkGray);
    background.setStyle(Qt::SolidPattern);
}

/*!
    \internal
*/
void QMdiAreaPrivate::_q_deactivateAllWindows()
{
    aboutToBecomeActive = qobject_cast<QMdiSubWindow *>(q_func()->sender());
    if (childWindows.isEmpty())
        return;

    int numDeactivated = 0;
    foreach (QMdiSubWindow *child, childWindows) {
        if (!sanityCheck(child, "QMdiArea::deactivateAllWindows"))
            continue;
        if (aboutToBecomeActive == child)
            continue;
        if (child->windowState() & Qt::WindowActive) {
            QEvent windowDeactivate(QEvent::WindowDeactivate);
            QApplication::sendEvent(child, &windowDeactivate);
            ++numDeactivated;
        }
    }
    Q_ASSERT(numDeactivated == 0 || numDeactivated == 1);
}

void QMdiAreaPrivate::_q_processWindowStateChanged(Qt::WindowStates oldState,
                                                      Qt::WindowStates newState)
{
    Q_Q(QMdiArea);
    QMdiSubWindow *child = qobject_cast<QMdiSubWindow *>(q->sender());
    if (!child)
        return;

    // windowActivated
    if (!(oldState & Qt::WindowActive) && (newState & Qt::WindowActive))
        emitWindowActivated(child);
    // windowDeactivated
    else if ((oldState & Qt::WindowActive) && !(newState & Qt::WindowActive))
        resetActiveWindow(child);

    // windowMinimized
    if (!(oldState & Qt::WindowMinimized) && (newState & Qt::WindowMinimized))
        q->arrangeMinimizedSubWindows();
    // windowMaximized
    else if (!(oldState & Qt::WindowMaximized) && (newState & Qt::WindowMaximized))
        internalRaise(child);
    // windowRestored
    else if (!(newState & (Qt::WindowMaximized | Qt::WindowMinimized)))
        internalRaise(child);
}

/*!
    \internal
*/
void QMdiAreaPrivate::appendChild(QMdiSubWindow *child)
{
    Q_Q(QMdiArea);
    Q_ASSERT(child && childWindows.indexOf(child) == -1);
    Q_ASSERT(child->sizeHint().isValid());

    child->setParent(q->viewport(), child->windowFlags());
    childWindows.append(QPointer<QMdiSubWindow>(child));

    if (!child->testAttribute(Qt::WA_Resized))
        child->resize(child->sizeHint());
    place(MinOverlapPlacer(), child);

    if (q->scrollBarsEnabled())
        child->setOption(QMdiSubWindow::AllowOutsideArea, true);
    else
        child->setOption(QMdiSubWindow::AllowOutsideArea, false);

    internalRaise(child);
    indicesToStackedChildren.prepend(childWindows.size() - 1);
    Q_ASSERT(indicesToStackedChildren.size() == childWindows.size());

    if (!(child->windowFlags() & Qt::SubWindow))
        child->setWindowFlags(Qt::SubWindow);
    child->installEventFilter(q);

    QObject::connect(child, SIGNAL(aboutToActivate()), q, SLOT(_q_deactivateAllWindows()));
    QObject::connect(child, SIGNAL(windowStateChanged(Qt::WindowStates, Qt::WindowStates)),
                     q, SLOT(_q_processWindowStateChanged(Qt::WindowStates, Qt::WindowStates)));
}

/*!
    \internal
*/
void QMdiAreaPrivate::place(const Placer &placer, QMdiSubWindow *child)
{
    QList<QRect> rects;
    QRect parentRect = q_func()->rect();
    foreach (QMdiSubWindow *existingChild, childWindows) {
        if (!sanityCheck(existingChild, "QMdiArea::place"))
            continue;
        if (existingChild != child && existingChild->isVisible()) {
            rects.append(QStyle::visualRect(child->layoutDirection(), parentRect,
                                            existingChild->geometry()));
        }
    }
    QPoint newPos = placer.place(child->size(), rects, parentRect);
    QRect newGeometry = QRect(newPos.x(), newPos.y(), child->width(), child->height());
    child->setGeometry(QStyle::visualRect(child->layoutDirection(), parentRect, newGeometry));
}

/*!
    \internal
*/
void QMdiAreaPrivate::rearrange(const Rearranger &rearranger, bool icons)
{
    QList<QWidget *> widgets;
    foreach (QMdiSubWindow *child, childWindows) {
        if (!sanityCheck(child, "QMdiArea::rearrange") || !child->isVisible())
            continue;
        if (icons) {
            if (child->isMinimized() && !child->isShaded())
                widgets.append(child);
        } else {
            if (child->isMinimized() && !child->isShaded())
                continue;
            if (child->isMaximized() || child->isShaded())
                child->showNormal();
            widgets.append(child);
            internalRaise(child);
        }
    }

    if (active) {
        int indexToActive = widgets.indexOf(active);
        if (indexToActive >= 0)
            widgets.move(indexToActive, widgets.size() - 1);
    }
    rearranger.rearrange(widgets, q_func()->viewport()->rect());
}

/*!
    \internal
*/
void QMdiAreaPrivate::activateWindow(QMdiSubWindow *child)
{
    Q_ASSERT(!childWindows.isEmpty());
    if (!child) {
        if (active) {
            Q_ASSERT(active->windowState() & Qt::WindowActive);
            QEvent windowDeactivate(QEvent::WindowDeactivate);
            QApplication::sendEvent(active, &windowDeactivate);
            resetActiveWindow();
        }
        return;
    }

    if (child->isHidden() || child == active)
        return;

    Q_ASSERT(!(child->windowState() & Qt::WindowActive));
    QEvent windowActivate(QEvent::WindowActivate);
    QApplication::sendEvent(child, &windowActivate);
}

/*!
    \internal
*/
void QMdiAreaPrivate::emitWindowActivated(QMdiSubWindow *activeWindow)
{
    Q_Q(QMdiArea);
    Q_ASSERT(activeWindow);
    if (activeWindow == active)
        return;
    Q_ASSERT(activeWindow->windowState() & Qt::WindowActive);

    int indexToActiveWindow = childWindows.indexOf(activeWindow);
    Q_ASSERT(indexToActiveWindow != -1);
    setIndex(&indexToNextWindow, indexToActiveWindow + 1, 0, childWindows.size() - 1 , true);
    setIndex(&indexToPreviousWindow, indexToActiveWindow - 1, 0, childWindows.size() - 1, false);

    // Put in front to update stacking order
    int index = indicesToStackedChildren.indexOf(indexToActiveWindow);
    Q_ASSERT(index != -1);
    indicesToStackedChildren.move(index, 0);
    internalRaise(activeWindow);

    Q_ASSERT(aboutToBecomeActive == activeWindow);
    active = activeWindow;
    aboutToBecomeActive = 0;
    Q_ASSERT(active->windowState() & Qt::WindowActive);
    emit q->subWindowActivated(active);
}

/*!
    \internal
*/
void QMdiAreaPrivate::resetActiveWindow(QMdiSubWindow *deactivatedWindow)
{
    Q_Q(QMdiArea);
    if (deactivatedWindow) {
        if (deactivatedWindow != active)
            return;
        active = 0;
        if (aboutToBecomeActive || isActivated)
            return;
        emit q->subWindowActivated(0);
        return;
    }

    if (aboutToBecomeActive)
        return;
    active = 0;
    emit q->subWindowActivated(0);
}

/*!
    \internal
*/
void QMdiAreaPrivate::updateActiveWindow(int removedIndex)
{
    Q_ASSERT(indicesToStackedChildren.size() == childWindows.size());
    if (childWindows.isEmpty()) {
        indexToNextWindow = -1;
        indexToPreviousWindow = -1;
        resetActiveWindow();
        return;
    }

    // Update indices list
    for (int i = 0; i < indicesToStackedChildren.size(); ++i) {
        int *index = &indicesToStackedChildren[i];
        if (*index > removedIndex)
            --*index;
    }

    // This is -1 only if there's no active window
    if (indexToNextWindow == -1) {
        Q_ASSERT(!active);
        return;
    }

    // Check if active window was removed
    if (indexToNextWindow - 1 == removedIndex) {
        activateWindow(childWindows.at(removedIndex));
    } else if (indexToNextWindow == 0 && removedIndex == childWindows.size()) {
        activateWindow(childWindows.at(0));
    // Active window is still in charge, update next and previous indices
    // if necessary.
    } else if (indexToNextWindow > removedIndex) {
        int nextCandidate = indexToNextWindow - 1;
        int prevCandidate = indexToPreviousWindow;
        if (indexToPreviousWindow >= removedIndex)
            --prevCandidate;
        setIndex(&indexToNextWindow, nextCandidate, 0, childWindows.size(), true);
        setIndex(&indexToPreviousWindow, prevCandidate, 0, childWindows.size(), false);
    }
}

/*!
    \internal
*/
void QMdiAreaPrivate::updateScrollBars()
{
    Q_Q(QMdiArea);

    if (!q->scrollBarsEnabled() || ignoreGeometryChange)
        return;

    QRect viewportRect = q->viewport()->rect();
    QRect childrenRect = q->viewport()->childrenRect();

    QScrollBar *hBar = q->horizontalScrollBar();
    int startX = q->isLeftToRight() ? childrenRect.left() : viewportRect.right()
                                                            - childrenRect.right();
    int xOffset = startX + hBar->value();
    int minX = qMin(0, xOffset);
    int maxX = qMax(0, xOffset + childrenRect.width() - viewportRect.width());
    hBar->setRange(minX, maxX);
    hBar->setPageStep(childrenRect.width());
    hBar->setSingleStep(childrenRect.width() / 20);

    QScrollBar *vBar = q->verticalScrollBar();
    int yOffset = childrenRect.top() + vBar->value();
    vBar->setRange(qMin(0, yOffset),
                   qMax(0, yOffset + childrenRect.height() - viewportRect.height()));
    vBar->setPageStep(childrenRect.height());
    vBar->setSingleStep(childrenRect.height() / 20);
}

/*!
    \internal
*/
void QMdiAreaPrivate::internalRaise(QMdiSubWindow *mdiChild) const
{
    if (!sanityCheck(mdiChild, "QMdiArea::internalRaise"))
        return;

    QMdiSubWindow *stackUnderChild = 0;
    if (!(mdiChild->windowFlags() & Qt::WindowStaysOnTopHint)) {
        foreach (QMdiSubWindow *child, childWindows) {
            if (!sanityCheck(child, "QMdiArea::internalRaise"))
                continue;
            if (!child->isHidden() && (child->windowFlags() & Qt::WindowStaysOnTopHint)) {
                if (stackUnderChild)
                    child->stackUnder(stackUnderChild);
                else
                    child->raise();
                stackUnderChild = child;
            }
        }
    }

    if (stackUnderChild)
        mdiChild->stackUnder(stackUnderChild);
    else
        mdiChild->raise();
}

/*!
    Constructs an empty workspace widget. \a parent is passed to QWidget's
    constructor.
*/
QMdiArea::QMdiArea(QWidget *parent)
    : QAbstractScrollArea(*new QMdiAreaPrivate, parent)
{
    setBackgroundRole(QPalette::Base);
    setFrameStyle(QFrame::NoFrame);
    setScrollBarsEnabled(false);
    setViewport(0);
    setFocusPolicy(Qt::NoFocus);
}

/*!
    Destroys the workspace.
*/
QMdiArea::~QMdiArea()
{
}

/*!
    \reimp
*/
QSize QMdiArea::sizeHint() const
{
    QSize size = QAbstractScrollArea::sizeHint() + baseSize();
    foreach (QMdiSubWindow *child, d_func()->childWindows) {
        if (!sanityCheck(child, "QMdiArea::sizeHint"))
            continue;
        size = size.expandedTo(child->sizeHint());
    }
    return size.expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
*/
QSize QMdiArea::minimumSizeHint() const
{
    QSize size(style()->pixelMetric(QStyle::PM_MDIMinimizedWidth),
               style()->pixelMetric(QStyle::PM_TitleBarHeight));
    size = size.expandedTo(QAbstractScrollArea::minimumSizeHint());
    foreach (QMdiSubWindow *child, d_func()->childWindows) {
        if (!sanityCheck(child, "QMdiArea::sizeHint"))
            continue;
        size = size.expandedTo(child->minimumSizeHint());
    }
    return size.expandedTo(QApplication::globalStrut());
}

/*!
    Returns a pointer to the current active subwindow. If no window is
    currently active, 0 is returned.

    \sa setActiveSubWindow()
*/
QMdiSubWindow *QMdiArea::activeSubWindow() const
{
    Q_D(const QMdiArea);
    if (d->active && d->isActivated)
        return d->active;
    return 0;
}

/*!
    Activates the subwindow \a window. If \a window is 0, any current active
    window is deactivated.

    \sa activeSubWindow()
*/
void QMdiArea::setActiveSubWindow(QMdiSubWindow *window)
{
    Q_D(QMdiArea);
    if (!window) {
        d->activateWindow(0);
        return;
    }

    if (d->childWindows.isEmpty()) {
        qWarning("QMdiArea::setActiveSubWindow: workspace is empty");
        return;
    }

    if (d->childWindows.indexOf(window) == -1) {
        qWarning("QMdiArea::setActiveSubWindow: window is not inside workspace");
        return;
    }

    d->activateWindow(window);
}

/*!
    Closes the active subwindow.

    \sa closeAllSubWindows()
*/
void QMdiArea::closeActiveSubWindow()
{
    Q_D(QMdiArea);
    if (d->active && d->active->isActiveWindow())
        d->active->close();
}

/*!
    Returns a list of all visible or minimized subwindows. If \a order is
    CreationOrder (the default), the windows are listed in the order in which
    they were inserted into the workspace. If \a order is StackingOrder, the
    windows are listed in their stacking order, with the topmost window as the
    last item in the list.

    \sa windowList(), WindowOrder
*/
QList<QMdiSubWindow *> QMdiArea::subWindowList(WindowOrder order) const
{
    Q_D(const QMdiArea);
    QList<QMdiSubWindow *> list;
    if (d->childWindows.isEmpty())
        return list;

    if (order == CreationOrder) {
        foreach (QMdiSubWindow *child, d->childWindows) {
            if (!sanityCheck(child, "QMdiArea::subWindowList"))
                continue;
            list.append(child);
        }
    } else {
        Q_ASSERT(d->indicesToStackedChildren.size() == d->childWindows.size());
        QList<QMdiSubWindow *> staysOnTopChildren;
        for (int i = d->indicesToStackedChildren.count() - 1; i >= 0; --i) {
            QMdiSubWindow *child = d->childWindows.at(d->indicesToStackedChildren.at(i));
            if (!sanityCheck(child, "QMdiArea::subWindowList"))
                continue;
            if (child->windowFlags() & Qt::WindowStaysOnTopHint)
                staysOnTopChildren.append(child);
            else
                list.append(child);
        }
        // Append children with Qt::WindowStaysOnTopHint at end (stacked on top)
        Q_ASSERT(staysOnTopChildren.count() + list.count() == d->childWindows.count());
        if (!staysOnTopChildren.isEmpty()) {
            foreach (QMdiSubWindow *child, staysOnTopChildren)
                list.append(child);
        }
    }
    return list;
}

/*!
    Closes all subwindows by sending one QCloseEvent to each window.

    Subwindows that ignore the close event will remain open.

    \sa closeActiveSubWindow()
*/
void QMdiArea::closeAllSubWindows()
{
    Q_D(QMdiArea);
    if (d->childWindows.isEmpty())
        return;

    foreach (QMdiSubWindow *child, d->childWindows) {
        if (!sanityCheck(child, "QMdiArea::closeAllSubWindows"))
            continue;
        child->close();
    }
}

/*!
    Gives the input focus to the next window in the list of child windows.

    \sa activatePreviousSubWindow()
*/
void QMdiArea::activateNextSubWindow()
{
    Q_D(QMdiArea);
    if (d->childWindows.isEmpty())
        return;

    int index = qMax(d->indexToNextWindow, 0);
    Q_ASSERT(index >= 0 && index < d->childWindows.size());
    if (!sanityCheck(d->childWindows.at(index), "QMdiArea::activateNextSubWindow"))
        return;

    int originalIndex = index;
    while (d->childWindows.at(index)->isHidden()) {
        setIndex(&index, index + 1, 0, d->childWindows.size() - 1, true);
        Q_ASSERT(index >= 0 && index < d->childWindows.size());
        if (index == originalIndex)
            break;
    }

    if (!d->childWindows.at(index)->isHidden())
        d->activateWindow(d->childWindows.at(index));
}

/*!
    Gives the input focus to the previous window in the list of child
    windows.

    \sa activateNextSubWindow()
*/
void QMdiArea::activatePreviousSubWindow()
{
    Q_D(QMdiArea);
    if (d->childWindows.isEmpty())
        return;

    int index = d->indexToPreviousWindow >= 0 ? d->indexToPreviousWindow
                                                : d->childWindows.size() - 1;
    Q_ASSERT(index >= 0 && index < d->childWindows.size());
    if (!sanityCheck(d->childWindows.at(index), "QMdiArea::activatePreviousSubWindow"))
        return;

    int originalIndex = index;
    while (d->childWindows.at(index)->isHidden()) {
        setIndex(&index, index - 1, 0, d->childWindows.size() - 1, false);
        Q_ASSERT(index >= 0 && index < d->childWindows.size());
        if (index == originalIndex)
            break;
    }

    if (!d->childWindows.at(index)->isHidden())
        d->activateWindow(d->childWindows.at(index));
}

/*!
    Adds widget \a widget as new sub window to the workspace.  If \a
    windowFlags are non-zero, they will override the flags set on the widget.

    Returns the widget used for the window frame.

    \sa addChildWindow(), removeSubWindow()
*/
QMdiSubWindow *QMdiArea::addSubWindow(QWidget *widget, Qt::WindowFlags windowFlags)
{
    if (!widget) {
        qWarning("QMdiArea::addSubWindow: null pointer to widget");
        return 0;
    }

    if (QMdiSubWindow *child = qobject_cast<QMdiSubWindow *>(widget)) {
        if (d_func()->childWindows.indexOf(child) != -1) {
            qWarning("QMdiArea::addSubWindow: window is already added");
            return child;
        }
        child->setParent(this, windowFlags ? windowFlags : child->windowFlags());
        return child;
    }

    QMdiSubWindow *child = new QMdiSubWindow(this, windowFlags);
    child->setAttribute(Qt::WA_DeleteOnClose);
    child->setWidget(widget);
    widget->setAutoFillBackground(true); // ### strange
    Q_ASSERT(child->testAttribute(Qt::WA_DeleteOnClose));

    return child;
}

/*!
    Removes the widget \a widget from the workspace.

    \sa addSubWindow()
*/
void QMdiArea::removeSubWindow(QWidget *widget)
{
    if (!widget) {
        qWarning("QMdiArea::removeSubWindow: null pointer to widget");
        return;
    }

    Q_D(QMdiArea);
    if (d->childWindows.isEmpty())
        return;

    if (QMdiSubWindow *child = qobject_cast<QMdiSubWindow *>(widget)) {
        int index = d->childWindows.indexOf(child);
        if (index == -1) {
            qWarning("QMdiArea::removeSubWindow: window is not inside workspace");
            return;
        }
        d->childWindows.removeAll(child);
        d->indicesToStackedChildren.removeAll(index);
        d->updateActiveWindow(index);
        child->setParent(0);
        return;
    }

    bool found = false;
    foreach (QMdiSubWindow *child, d->childWindows) {
        if (!sanityCheck(child, "QMdiArea::removeSubWindow"))
            continue;
        if (child->widget() == widget) {
            child->setWidget(0);
            Q_ASSERT(!child->widget());
            found = true;
            break;
        }
    }

    if (!found)
        qWarning("QMdiArea::removeSubWindow: widget is not child of any window inside QMdiArea");
}

bool QMdiArea::scrollBarsEnabled() const
{
    return (horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)
           && (verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff);
}

/*!
    \property QMdiArea::scrollBarsEnabled
    \brief whether the workspace provides scrollbars

    If this property is true, the workspace will provide scrollbars if any
    of the child windows extend beyond the edges of the visible
    workspace. The workspace area will automatically increase to
    contain child windows if they are resized beyond the right or
    bottom edges of the visible area.

    If this property is false (the default), resizing child windows
    out of the visible area of the workspace is not permitted, although
    it is still possible to position them partially outside the visible area.

    \sa QAbstractScrollArea::horizontalScrollBar(), QAbstractScrollArea::verticalScrollBar()
*/
void QMdiArea::setScrollBarsEnabled(bool enable)
{
    Q_D(QMdiArea);
    Qt::ScrollBarPolicy policy = enable ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff;
    setHorizontalScrollBarPolicy(policy);
    setVerticalScrollBarPolicy(policy);

    foreach (QMdiSubWindow *child, d->childWindows) {
        if (!sanityCheck(child, "QMdiArea::setScrollBarsEnabled"))
            continue;
        child->setOption(QMdiSubWindow::AllowOutsideArea, enable);
    }
    d->updateScrollBars();
}

/*!
    \property QMdiArea::background
    \brief the background brush for the workspace

    This property sets the background brush for the workspace area itself. By
    default, a gray color is used, but you can use any brush for the
    background (e.g., colors, gradients or pixmaps) by assigning a custom
    background brush.
*/
QBrush QMdiArea::background() const
{
    return d_func()->background;
}
void QMdiArea::setBackground(const QBrush &brush)
{
    d_func()->background = brush;
}

/*!
    \reimp
*/
void QMdiArea::childEvent(QChildEvent *childEvent)
{
    Q_D(QMdiArea);
    if (childEvent->type() == QEvent::ChildPolished) {
        if (QMdiSubWindow *mdiChild = qobject_cast<QMdiSubWindow *>(childEvent->child())) {
            if (d->childWindows.indexOf(mdiChild) == -1)
                d->appendChild(mdiChild);
        }
    }
}

/*!
    \reimp
*/
void QMdiArea::resizeEvent(QResizeEvent *resizeEvent)
{
    Q_D(QMdiArea);
    if (d->childWindows.isEmpty()) {
        resizeEvent->ignore();
        return;
    }
    d->updateScrollBars();
    arrangeMinimizedSubWindows();
}

/*!
    \reimp
*/
bool QMdiArea::viewportEvent(QEvent *event)
{
    Q_D(QMdiArea);
    switch (event->type()) {
    case QEvent::ChildRemoved: {
        QObject *removedChild = static_cast<QChildEvent *>(event)->child();
        for (int i = 0; i < d->childWindows.size(); ++i) {
            QObject *child = d->childWindows.at(i);
            if (!child || child == removedChild || !child->parent()
                    || child->parent() != viewport()) {
                d->childWindows.removeAt(i);
                d->indicesToStackedChildren.removeAll(i);
                d->updateActiveWindow(i);
                break;
            }
        }
        break;
    }
    case QEvent::Destroy:
        d->indexToNextWindow = -1;
        d->indexToPreviousWindow = -1;
        d->resetActiveWindow();
        d->childWindows.clear();
        qWarning("QMdiArea: Deleting the view port is undefined, use setViewport instead.");
        break;
    case QEvent::Resize: {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
        foreach (QMdiSubWindow *child, d->childWindows) {
            if (!sanityCheck(child, "QMdiArea::viewportEvent"))
                continue;
            if (child->isMaximized()) {
                Qt::WindowStates oldChildState = child->windowState();
                Qt::WindowStates oldWidgetState = child->widget() ?
                                                  child->widget()->windowState() : Qt::WindowNoState;
                child->resize(resizeEvent->size());
                child->overrideWindowState(oldChildState | Qt::WindowMaximized);
                if (child->widget())
                    child->widget()->overrideWindowState(oldWidgetState | Qt::WindowMaximized);
            }
        }
        break;
    }
    default:
        break;
    }
    return QAbstractScrollArea::viewportEvent(event);
}

/*!
    \reimp
*/
void QMdiArea::scrollContentsBy(int dx, int dy)
{
    Q_D(QMdiArea);
    d->ignoreGeometryChange = true;
    viewport()->scroll(isLeftToRight() ? dx : -dx, dy);
    d->ignoreGeometryChange = false;
}

/*!
    Arranges all child windows in a tile pattern.

    \sa cascadeSubWindows(), arrangeMinimizedSubWindows()
*/
void QMdiArea::tileSubWindows()
{
    d_func()->rearrange(RegularTiler(), false);
}

/*!
    Arranges all the child windows in a cascade pattern.

    \sa tileSubWindows(), arrangeMinimizedSubWindows()
*/
void QMdiArea::cascadeSubWindows()
{
    d_func()->rearrange(SimpleCascader());
}

/*!
    Arranges all iconified windows at the bottom of the workspace.

    \sa cascadeSubWindows(), tileSubWindows()
*/
void QMdiArea::arrangeMinimizedSubWindows()
{
    if (QMdiSubWindow *child = qobject_cast<QMdiSubWindow *>(sender()))
        child->lower();
    d_func()->rearrange(IconTiler(), true);
}

/*!
    \reimp
*/
bool QMdiArea::event(QEvent *event)
{
    Q_D(QMdiArea);
    if (event->type() == QEvent::WindowActivate) {
        d->isActivated = true;
        if (d->childWindows.isEmpty())
            return QAbstractScrollArea::event(event);
        if (!d->active)
            d->activateWindow(d->childWindows.at(d->indicesToStackedChildren.at(0)));
        return true;
    } else if (event->type() == QEvent::WindowDeactivate) {
        d->isActivated = false;
    }
    return QAbstractScrollArea::event(event);
}

/*!
    \reimp
*/
bool QMdiArea::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Move || event->type() == QEvent::Resize)
        d_func()->updateScrollBars();
    return QAbstractScrollArea::eventFilter(object, event);
}

/*!
    \reimp
*/
void QMdiArea::paintEvent(QPaintEvent * /*paintEvent*/)
{
    QPainter painter(viewport());
    painter.fillRect(viewport()->rect(), d_func()->background);
}

/*!
    This slot is called by QAbstractScrollArea after setViewport() has been
    called. Reimplement this function in a subclass of QMdiArea to
    initialize the new viewport \a viewport before it is used.

    \sa setViewport()
*/
void QMdiArea::setupViewport(QWidget *viewport)
{
    foreach (QMdiSubWindow *child, d_func()->childWindows) {
        if (!sanityCheck(child, "QMdiArea::setupViewport"))
            continue;
        child->setParent(viewport, child->windowFlags());
    }
}

#include "moc_qmdiarea.cpp"

