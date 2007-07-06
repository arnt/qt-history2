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
    \brief The QMdiArea widget provides an area in which MDI windows are displayed.
    \since 4.3
    \ingroup application
    \mainclass

    QMdiArea functions, essentially, like a window manager for MDI
    windows. For instance, it draws the windows it manages on itself
    and arranges them in a cascading or tile pattern. QMdiArea is
    commonly used as the center widget in a QMainWindow to create MDI
    applications, but can also be placed in any layout. The following
    code adds an area to a main window:

    \quotefromfile snippets/mdiareasnippets.cpp
    \skipto /QMainWindow/
    \printuntil /setCentralWidget/

    Unlike the window managers for top-level windows, all window flags
    (Qt::WindowFlags) are supported by QMdiArea as long as the flags
    are supported by the current widget style. If a specific flag is
    not supported by the style (e.g., the
    \l{Qt::}{WindowShadeButtonHint}), you can still shade the window
    with showShaded().

    Subwindows in QMdiArea are instances of QMdiSubWindow. They
    are added to an MDI area with addSubWindow(). It is common to pass
    a QWidget, which is set as the internal widget, to this function,
    but it is also possible to pass a QMdiSubWindow directly.The class
    inherits QWidget, and you can use the same API as with a normal
    top-level window when programming. QMdiSubWindow also has behavior
    that is specific to MDI windows. See the QMdiSubWindow class
    description for more details.

    A subwindow becomes active when it gets the keyboard focus, or
    when setFocus() is called. The user activates a window by
    moving focus in the usual ways. The MDI area emits the subWindowActivated()
    signal when the active window changes, and the activeSubWindow()
    function returns the active subwindow.

    The convenience function subWindowList() returns a list of all
    subwindows. This information could be used in a popup menu
    containing a list of windows, for example.

    \omit
        // does this still hold?
        This feature is also
        available as part of the \l{Window Menu} Solution.
    \endomit

    QMdiArea provides two built-in layout strategies for
    subwindows: cascadeSubWindows() and tileSubWindows(). Both are
    slots and are easily connected to menu entries.

    \img qmdiarea-arrange.png

    \sa QMdiSubWindow
*/

/*!
    \fn QMdiArea::subWindowActivated(QMdiSubWindow *window)

    QMdiArea emits this signal after \a window has been activated. When \a
    window is 0, QMdiArea has just deactivated its last active window, and
    there are no active windows on the workspace.

    \sa QMdiArea::activeSubWindow()
*/

/*!
    \enum QMdiArea::AreaOption

    This enum describes options that customize the behavior of the
    QMdiArea.

    \value DontMaximizeSubWindowOnActivation When the active subwindow
    is maximized, the default behavior is to maximize the next
    subwindow that is activated. Set this option if you do not want
    this behavior.
*/

/*!
    \enum QMdiArea::WindowOrder

    Specifies the order in which child windows are returned from
    subWindowList(). The cascadeSubWindows() and tileSubWindows()
    functions follow this order when arranging the windows.

    \value CreationOrder The windows are returned in the order of
    their creation
    \value StackingOrder The windows are returned in the order in
    which they are stacked; the top-most window is
    last in the list.
*/

#include "qmdiarea_p.h"

#ifndef QT_NO_MDIAREA

#include <QApplication>
#include <QStyle>
#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)
#include <QMacStyle>
#endif
#include <QChildEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtAlgorithms>
#include <QMutableListIterator>
#include <QPainter>
#include <QFontMetrics>
#include <QStyleOption>
#include <QDesktopWidget>
#include <QDebug>
#include <private/qmath_p.h>

// Asserts in debug mode, gives warning otherwise.
static bool sanityCheck(const QMdiSubWindow * const child, const char *where)
{
    if (!child) {
        const char error[] = "null pointer";
        Q_ASSERT_X(false, where, error);
        qWarning("%s:%s", where, error);
        return false;
    }
    return true;
}

static bool sanityCheck(const QList<QWidget *> &widgets, const int index, const char *where)
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

static void setIndex(int *index, int candidate, int min, int max, bool isIncreasing)
{
    if (!index)
        return;

    if (isIncreasing) {
        if (candidate > max)
            *index = min;
        else
            *index = qMax(candidate, min);
    } else {
        if (candidate < min)
            *index = max;
        else
            *index = qMin(candidate, max);
    }
    Q_ASSERT(*index >= min && *index <= max);
}

/*!
    \internal
*/
void RegularTiler::rearrange(QList<QWidget *> &widgets, const QRect &domain) const
{
    if (widgets.isEmpty())
        return;

    const int n = widgets.size();
    const int ncols = qMax(qCeil(qSqrt(qreal(n))), 1);
    const int nrows = qMax((n % ncols) ? (n / ncols + 1) : (n / ncols), 1);
    const int nspecial = (n % ncols) ? (ncols - n % ncols) : 0;
    const int dx = domain.width()  / ncols;
    const int dy = domain.height() / nrows;

    int i = 0;
    for (int row = 0; row < nrows; ++row) {
        const int y1 = int(row * (dy + 1));
        for (int col = 0; col < ncols; ++col) {
            if (row == 1 && col < nspecial)
                continue;
            const int x1 = int(col * (dx + 1));
            int x2 = int(x1 + dx);
            int y2 = int(y1 + dy);
            if (row == 0 && col < nspecial) {
                y2 *= 2;
                if (nrows != 2)
                    y2 += 1;
                else
                    y2 = domain.bottom();
            }
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

    QStyleOptionTitleBar options;
    options.initFrom(widgets.at(0));
    int titleBarHeight = widgets.at(0)->style()->pixelMetric(QStyle::PM_TitleBarHeight, &options);
#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)
    // ### Remove this after the mac style has been fixed
    if (qobject_cast<QMacStyle *>(widgets.at(0)->style()))
        titleBarHeight -= 4;
#endif
    const QFontMetrics fontMetrics = QFontMetrics(QApplication::font("QWorkspaceTitleBar"));
    const int dy = qMax(titleBarHeight - (titleBarHeight - fontMetrics.height()) / 2, 1);

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
            QRect newGeometry = QRect(QPoint(x, y), widget->sizeHint());
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
    foreach (QRect rect, rects) {
        if (!rect.isValid())
            return QPoint();
    }

    QList<QRect> candidates;
    getCandidatePlacements(size, rects, domain, candidates);
    return findBestPlacement(domain, rects, candidates);
}

/*!
    \internal
*/
QMdiAreaPrivate::QMdiAreaPrivate()
    : cascader(0),
      regularTiler(0),
      iconTiler(0),
      placer(0),
      activationOrder(QMdiArea::CreationOrder),
      ignoreGeometryChange(false),
      ignoreWindowStateChange(false),
      isActivated(false),
      isSubWindowsTiled(false),
      showActiveWindowMaximized(false),
      tileCalledFromResizeEvent(false),
      updatesDisabledByUs(false),
      indexToNextWindow(-1),
      indexToPreviousWindow(-1),
      resizeTimerId(-1)
{
}

/*!
    \internal
*/
void QMdiAreaPrivate::_q_deactivateAllWindows(QMdiSubWindow *aboutToActivate)
{
    if (ignoreWindowStateChange)
        return;

    Q_Q(QMdiArea);
    if (!aboutToActivate)
        aboutToBecomeActive = qobject_cast<QMdiSubWindow *>(q->sender());
    else
        aboutToBecomeActive = aboutToActivate;
    Q_ASSERT(aboutToBecomeActive);

    foreach (QMdiSubWindow *child, childWindows) {
        if (!sanityCheck(child, "QMdiArea::deactivateAllWindows") || aboutToBecomeActive == child)
            continue;
        // We don't want to handle signals caused by child->showNormal().
        ignoreWindowStateChange = true;
        if(!(options & QMdiArea::DontMaximizeSubWindowOnActivation) && !showActiveWindowMaximized)
            showActiveWindowMaximized = child->isMaximized() && child->isVisible();
        if (showActiveWindowMaximized && child->isMaximized()) {
            if (q->updatesEnabled()) {
                updatesDisabledByUs = true;
                q->setUpdatesEnabled(false);
            }
            child->showNormal();
        }
        if (child->isMinimized() && !child->isShaded() && !windowStaysOnTop(child))
            child->lower();
        ignoreWindowStateChange = false;
        child->d_func()->setActive(false);
    }
}

/*!
    \internal
*/
void QMdiAreaPrivate::_q_processWindowStateChanged(Qt::WindowStates oldState,
                                                   Qt::WindowStates newState)
{
    if (ignoreWindowStateChange)
        return;

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
    if (!(oldState & Qt::WindowMinimized) && (newState & Qt::WindowMinimized)) {
        isSubWindowsTiled = false;
        arrangeMinimizedSubWindows();
    // windowMaximized
    } else if (!(oldState & Qt::WindowMaximized) && (newState & Qt::WindowMaximized)) {
        internalRaise(child);
    // windowRestored
    } else if (!(newState & (Qt::WindowMaximized | Qt::WindowMinimized))) {
        internalRaise(child);
        if (oldState & Qt::WindowMinimized)
            arrangeMinimizedSubWindows();
    }
}

/*!
    \internal
*/
void QMdiAreaPrivate::appendChild(QMdiSubWindow *child)
{
    Q_Q(QMdiArea);
    Q_ASSERT(child && childWindows.indexOf(child) == -1);

    if (child->parent() != q->viewport())
        child->setParent(q->viewport(), child->windowFlags());
    childWindows.append(QPointer<QMdiSubWindow>(child));

    if (!child->testAttribute(Qt::WA_Resized) && q->isVisible()) {
        QSize newSize(child->sizeHint().boundedTo(q->viewport()->size()));
        child->resize(newSize.expandedTo(child->minimumSize()));
    }

    if (!placer)
        placer = new MinOverlapPlacer;
    place(placer, child);

    if (q->horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)
        child->setOption(QMdiSubWindow::AllowOutsideAreaHorizontally, true);
    else
        child->setOption(QMdiSubWindow::AllowOutsideAreaHorizontally, false);

    if (q->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)
        child->setOption(QMdiSubWindow::AllowOutsideAreaVertically, true);
    else
        child->setOption(QMdiSubWindow::AllowOutsideAreaVertically, false);

    internalRaise(child);
    indicesToActivatedChildren.prepend(childWindows.size() - 1);
    Q_ASSERT(indicesToActivatedChildren.size() == childWindows.size());

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
void QMdiAreaPrivate::place(Placer *placer, QMdiSubWindow *child)
{
    if (!placer || !child)
        return;

    Q_Q(QMdiArea);
    if (!q->isVisible()) {
        // The window is only laid out when it's added to QMdiArea,
        // so there's no need to check that we don't have it in the
        // list already. appendChild() ensures that.
        pendingPlacements.append(child);
        return;
    }

    QList<QRect> rects;
    QRect parentRect = q->rect();
    foreach (QMdiSubWindow *window, childWindows) {
        if (!sanityCheck(window, "QMdiArea::place") || window == child || !window->isVisibleTo(q)
                || !window->testAttribute(Qt::WA_Moved)) {
            continue;
        }
        QRect occupiedGeometry;
        if (window->isMaximized()) {
            occupiedGeometry = QRect(window->d_func()->oldGeometry.topLeft(),
                                     window->d_func()->restoreSize);
        } else {
            occupiedGeometry = window->geometry();
        }
        rects.append(QStyle::visualRect(child->layoutDirection(), parentRect, occupiedGeometry));
    }
    QPoint newPos = placer->place(child->size(), rects, parentRect);
    QRect newGeometry = QRect(newPos.x(), newPos.y(), child->width(), child->height());
    child->setGeometry(QStyle::visualRect(child->layoutDirection(), parentRect, newGeometry));
}

/*!
    \internal
*/
void QMdiAreaPrivate::rearrange(Rearranger *rearranger)
{
    if (!rearranger)
        return;

    Q_Q(QMdiArea);
    if (!q->isVisible()) {
        // Compress if we already have the rearranger in the list.
        int index = pendingRearrangements.indexOf(rearranger);
        if (index != -1)
            pendingRearrangements.move(index, pendingRearrangements.size() - 1);
        else
            pendingRearrangements.append(rearranger);
        return;
    }

    QList<QWidget *> widgets;
    const bool reverseList = rearranger->type() == Rearranger::RegularTiler;
    const QList<QMdiSubWindow *> subWindows = subWindowList(QMdiArea::StackingOrder, reverseList);
    QSize minSubWindowSize;
    foreach (QMdiSubWindow *child, subWindows) {
        if (!sanityCheck(child, "QMdiArea::rearrange") || !child->isVisible())
            continue;
        if (rearranger->type() == Rearranger::IconTiler) {
            if (child->isMinimized() && !child->isShaded())
                widgets.append(child);
        } else {
            if (child->isMinimized() && !child->isShaded())
                continue;
            if (child->isMaximized() || child->isShaded())
                child->showNormal();
            minSubWindowSize = minSubWindowSize.expandedTo(child->minimumSize());
            widgets.append(child);
        }
    }

    if (active && rearranger->type() == Rearranger::RegularTiler) {
        // Move active window in front if necessary. That's the case if we
        // have any windows with staysOnTopHint set.
        int indexToActive = widgets.indexOf((QWidget *)active);
        if (indexToActive > 0)
            widgets.move(indexToActive, 0);
    }

    QRect domain = q->viewport()->rect();
    if (rearranger->type() == Rearranger::RegularTiler && !widgets.isEmpty())
        domain = resizeToMinimumTileSize(minSubWindowSize, widgets.count());

    rearranger->rearrange(widgets, domain);

    if (rearranger->type() == Rearranger::RegularTiler && !widgets.isEmpty()) {
        isSubWindowsTiled = true;
        updateScrollBars();
    } else if (rearranger->type() == Rearranger::SimpleCascader) {
        isSubWindowsTiled = false;
    }
}

/*!
    \internal

    Arranges all minimized windows at the bottom of the workspace.
*/
void QMdiAreaPrivate::arrangeMinimizedSubWindows()
{
    if (!iconTiler)
        iconTiler = new IconTiler;
    rearrange(iconTiler);
}

/*!
    \internal
*/
void QMdiAreaPrivate::activateWindow(QMdiSubWindow *child)
{
    if (childWindows.isEmpty()) {
        Q_ASSERT(!child);
        Q_ASSERT(!active);
        return;
    }

    if (!child) {
        if (active) {
            Q_ASSERT(active->d_func()->isActive);
            active->d_func()->setActive(false);
            resetActiveWindow();
        }
        return;
    }

    if (child->isHidden() || child == active)
        return;
    child->d_func()->setActive(true);
}

/*!
    \internal
*/
void QMdiAreaPrivate::activateCurrentWindow()
{
    QMdiSubWindow *current = q_func()->currentSubWindow();
    if (current && !isExplicitlyDeactivated(current))
        current->d_func()->setActive(true);
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
    Q_ASSERT(activeWindow->d_func()->isActive);

    if (!aboutToBecomeActive)
        _q_deactivateAllWindows(activeWindow);
    Q_ASSERT(aboutToBecomeActive);

    // This is true only if 'DontMaximizeSubWindowOnActivation' is disabled
    // and the previous active window was maximized.
    if (showActiveWindowMaximized) {
        if (!activeWindow->isMaximized())
            activeWindow->showMaximized();
        showActiveWindowMaximized = false;
    }

    // Put in front to update activation order.
    const int indexToActiveWindow = childWindows.indexOf(activeWindow);
    Q_ASSERT(indexToActiveWindow != -1);
    const int index = indicesToActivatedChildren.indexOf(indexToActiveWindow);
    Q_ASSERT(index != -1);
    indicesToActivatedChildren.move(index, 0);
    internalRaise(activeWindow);

    if (updatesDisabledByUs) {
        q->setUpdatesEnabled(true);
        updatesDisabledByUs = false;
    }

    Q_ASSERT(aboutToBecomeActive == activeWindow);
    active = activeWindow;
    aboutToBecomeActive = 0;
    Q_ASSERT(active->d_func()->isActive);
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
        if ((aboutToBecomeActive || isActivated || lastWindowAboutToBeDestroyed())
                && !isExplicitlyDeactivated(deactivatedWindow)) {
            return;
        }
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
void QMdiAreaPrivate::updateActiveWindow(int removedIndex, bool activeRemoved)
{
    Q_ASSERT(indicesToActivatedChildren.size() == childWindows.size());
    if (childWindows.isEmpty()) {
        showActiveWindowMaximized = false;
        resetActiveWindow();
        return;
    }

    // Update indices list
    for (int i = 0; i < indicesToActivatedChildren.size(); ++i) {
        int *index = &indicesToActivatedChildren[i];
        if (*index > removedIndex)
            --*index;
    }

    if (!activeRemoved)
        return;

    // Activate next window.
    QMdiSubWindow *next = nextVisibleSubWindow(0, removedIndex);
    if (next)
        activateWindow(next);
}

/*!
    \internal
*/
void QMdiAreaPrivate::updateScrollBars()
{
    Q_Q(QMdiArea);
    if (ignoreGeometryChange || (q->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff
                                 && q->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)) {
        return;
    }

    QRect viewportRect = q->viewport()->rect();
    QRect childrenRect = q->viewport()->childrenRect();
    int startX = q->isLeftToRight() ? childrenRect.left() : viewportRect.right()
                                                            - childrenRect.right();
    // Horizontal scroll bar.
    QScrollBar *hBar = q->horizontalScrollBar();
    if (isSubWindowsTiled && hBar->value() != 0)
        hBar->setValue(0);
    int xOffset = startX + hBar->value();
    hBar->setRange(qMin(0, xOffset),
                   qMax(0, xOffset + childrenRect.width() - viewportRect.width()));
    hBar->setPageStep(childrenRect.width());
    hBar->setSingleStep(childrenRect.width() / 20);

    // Vertical scroll bar.
    QScrollBar *vBar = q->verticalScrollBar();
    if (isSubWindowsTiled && vBar->value() != 0)
        vBar->setValue(0);
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
    if (!sanityCheck(mdiChild, "QMdiArea::internalRaise") || childWindows.size() < 2)
        return;

    QMdiSubWindow *stackUnderChild = 0;
    if (!windowStaysOnTop(mdiChild)) {
        foreach (QObject *object, q_func()->viewport()->children()) {
            QMdiSubWindow *child = qobject_cast<QMdiSubWindow *>(object);
            if (!child || !childWindows.contains(child))
                continue;
            if (!child->isHidden() && windowStaysOnTop(child)) {
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

QRect QMdiAreaPrivate::resizeToMinimumTileSize(const QSize &minSubWindowSize, int subWindowCount)
{
    Q_Q(QMdiArea);
    if (!minSubWindowSize.isValid() || subWindowCount <= 0)
        return q->viewport()->rect();

    // Calculate minimum size.
    const int columns = qMax(qCeil(qSqrt(qreal(subWindowCount))), 1);
    const int rows = qMax((subWindowCount % columns) ? (subWindowCount / columns + 1)
                                                     : (subWindowCount / columns), 1);
    const int minWidth = minSubWindowSize.width() * columns;
    const int minHeight = minSubWindowSize.height() * rows;

    // Increase area size if necessary. Scroll bars are provided if we're not able
    // to resize to the minimum size.
    if (!tileCalledFromResizeEvent) {
        QWidget *topLevel = q;
        // Find the topLevel for this area, either a real top-level or a sub-window.
        while (topLevel && !topLevel->isWindow() && topLevel->windowType() != Qt::SubWindow)
            topLevel = topLevel->parentWidget();
        // We don't want sub-subwindows to be placed at the edge, thus add 2 pixels.
        int minAreaWidth = minWidth + left + right + 2;
        int minAreaHeight = minHeight + top + bottom + 2;
        if (q->horizontalScrollBar()->isVisible())
            minAreaHeight += q->horizontalScrollBar()->height();
        if (q->verticalScrollBar()->isVisible())
            minAreaWidth += q->verticalScrollBar()->width();
        if (q->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents, 0, q)) {
            const int frame = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            minAreaWidth += 2 * frame;
            minAreaHeight += 2 * frame;
        }
        const QSize diff = QSize(minAreaWidth, minAreaHeight).expandedTo(q->size()) - q->size();
        topLevel->resize(topLevel->size() + diff);
    }

    QRect domain = q->viewport()->rect();

    // Adjust domain width and provide horizontal scroll bar.
    if (domain.width() < minWidth) {
        domain.setWidth(minWidth);
        if (q->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
            q->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        else if (q->horizontalScrollBar()->value() != 0)
            q->horizontalScrollBar()->setValue(0);
    }
    // Adjust domain height and provide vertical scroll bar.
    if (domain.height() < minHeight) {
        domain.setHeight(minHeight);
        if (q->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
            q->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        else if (q->verticalScrollBar()->value() != 0)
            q->verticalScrollBar()->setValue(0);
    }
    return domain;
}

/*!
    \internal
*/
bool QMdiAreaPrivate::scrollBarsEnabled() const
{
    Q_Q(const QMdiArea);
    return (q->horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)
           && (q->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff);
}

/*!
    \internal
*/
bool QMdiAreaPrivate::lastWindowAboutToBeDestroyed() const
{
    if (childWindows.count() != 1)
        return false;

    QMdiSubWindow *last = childWindows.at(0);
    if (!last)
        return true;

    if (!last->testAttribute(Qt::WA_DeleteOnClose))
        return false;

    return last->d_func()->data.is_closing;
}

/*!
    \internal
*/
void QMdiAreaPrivate::setChildActivationEnabled(bool enable, bool onlyNextActivationEvent) const
{
    foreach (QMdiSubWindow *subWindow, childWindows) {
        if (!subWindow || !subWindow->isVisible())
            continue;
        if (onlyNextActivationEvent)
            subWindow->d_func()->ignoreNextActivationEvent = !enable;
        else
            subWindow->d_func()->activationEnabled = enable;
    }
}

/*!
    \internal
    \reimp
*/
void QMdiAreaPrivate::scrollBarPolicyChanged(Qt::Orientation orientation, Qt::ScrollBarPolicy policy)
{
    if (childWindows.isEmpty())
        return;

    const QMdiSubWindow::SubWindowOption option = orientation == Qt::Horizontal ?
        QMdiSubWindow::AllowOutsideAreaHorizontally : QMdiSubWindow::AllowOutsideAreaVertically;
    const bool enable = policy != Qt::ScrollBarAlwaysOff;
    foreach (QMdiSubWindow *child, childWindows) {
        if (!sanityCheck(child, "QMdiArea::scrollBarPolicyChanged"))
            continue;
        child->setOption(option, enable);
    }
    updateScrollBars();
}

QList<QMdiSubWindow *> QMdiAreaPrivate::subWindowList(QMdiArea::WindowOrder order, bool reversed) const
{
    QList<QMdiSubWindow *> list;
    if (childWindows.isEmpty())
        return list;

    if (order == QMdiArea::CreationOrder) {
        foreach (QMdiSubWindow *child, childWindows) {
            if (!child)
                continue;
            if (!reversed)
                list.append(child);
            else
                list.prepend(child);
        }
    } else if (order == QMdiArea::StackingOrder) {
        foreach (QObject *object, viewport->children()) {
            QMdiSubWindow *child = qobject_cast<QMdiSubWindow *>(object);
            if (!child || !childWindows.contains(child))
                continue;
            if (!reversed)
                list.append(child);
            else
                list.prepend(child);
        }
    } else { // ActivationHistoryOrder
        Q_ASSERT(indicesToActivatedChildren.size() == childWindows.size());
        for (int i = indicesToActivatedChildren.count() - 1; i >= 0; --i) {
            QMdiSubWindow *child = childWindows.at(indicesToActivatedChildren.at(i));
            if (!child)
                continue;
            if (!reversed)
                list.append(child);
            else
                list.prepend(child);
        }
    }
    return list;
}

/*!
    \internal
*/
QMdiSubWindow *QMdiAreaPrivate::nextVisibleSubWindow(int increaseFactor, int removedIndex) const
{
    if (childWindows.isEmpty())
        return 0;

    Q_Q(const QMdiArea);
    const QList<QMdiSubWindow *> subWindows = q->subWindowList(activationOrder);
    QMdiSubWindow *current = removedIndex >= 0 ? 0 : q->currentSubWindow();

    // There's no current sub-window (removed or deactivated),
    // so we have to pick the last active or the next in creation order.
    if (!current) {
        if (removedIndex >= 0 && activationOrder == QMdiArea::CreationOrder) {
            int candidateIndex = -1;
            setIndex(&candidateIndex, removedIndex, 0, subWindows.size() - 1, true);
            current = childWindows.at(candidateIndex);
        } else {
            current = subWindows.back();
        }
    }
    Q_ASSERT(current);

    // Find the index for the current sub-window in the given activation order
    const int indexToCurrent = subWindows.indexOf(current);
    const bool increasing = increaseFactor > 0 ? true : false;

    // and use that index + increseFactor as a candidate.
    int index = -1;
    setIndex(&index, indexToCurrent + increaseFactor, 0, subWindows.size() - 1, increasing);
    Q_ASSERT(index != -1);

    // Try to find another window if the candidate is hidden.
    while (subWindows.at(index)->isHidden()) {
        setIndex(&index, index + increaseFactor, 0, subWindows.size() - 1, increasing);
        if (index == indexToCurrent)
            break;
    }

    if (!subWindows.at(index)->isHidden())
        return subWindows.at(index);
    return 0;
}

/*!
    Constructs an empty mdi area. \a parent is passed to QWidget's
    constructor.
*/
QMdiArea::QMdiArea(QWidget *parent)
    : QAbstractScrollArea(*new QMdiAreaPrivate, parent)
{
    setBackgroundRole(QPalette::Dark);
    setBackground(palette().brush(QPalette::Dark));
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewport(0);
    setFocusPolicy(Qt::NoFocus);
    QApplication::instance()->installEventFilter(this);
}

/*!
    Destroys the MDI area.
*/
QMdiArea::~QMdiArea()
{
    Q_D(QMdiArea);
    delete d->cascader;
    d->cascader = 0;

    delete d->regularTiler;
    d->regularTiler = 0;

    delete d->iconTiler;
    d->iconTiler = 0;

    delete d->placer;
    d->placer = 0;
}

/*!
    \reimp
*/
QSize QMdiArea::sizeHint() const
{
    // Calculate a proper scale factor for QDesktopWidget::size().
    // This also takes into account that we can have nested workspaces.
    int nestedCount = 0;
    QWidget *widget = this->parentWidget();
    while (widget) {
        if (qobject_cast<QMdiArea *>(widget))
            ++nestedCount;
        widget = widget->parentWidget();
    }
    const int scaleFactor = 3 * (nestedCount + 1);

    QSize desktopSize = QApplication::desktop()->size();
    QSize size(desktopSize.width() * 2 / scaleFactor, desktopSize.height() * 2 / scaleFactor);
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
    Q_D(const QMdiArea);
    QSize size(style()->pixelMetric(QStyle::PM_MdiSubWindowMinimizedWidth),
               style()->pixelMetric(QStyle::PM_TitleBarHeight));
    size = size.expandedTo(QAbstractScrollArea::minimumSizeHint());
    if (!d->scrollBarsEnabled()) {
        foreach (QMdiSubWindow *child, d->childWindows) {
            if (!sanityCheck(child, "QMdiArea::sizeHint"))
                continue;
            size = size.expandedTo(child->minimumSizeHint());
        }
    }
    return size.expandedTo(QApplication::globalStrut());
}

/*!
    Returns a pointer to the current subwindow, or 0 if there is
    no current subwindow.

    This function will return the same as activeSubWindow() if
    the QApplication containing QMdiArea is active.

    \sa activeSubWindow(), QApplication::activeWindow()
*/
QMdiSubWindow *QMdiArea::currentSubWindow() const
{
    Q_D(const QMdiArea);
    if (d->childWindows.isEmpty())
        return 0;

    if (d->active)
        return d->active;

    if (d->isActivated)
        return 0;

    Q_ASSERT(d->indicesToActivatedChildren.count() > 0);
    int index = d->indicesToActivatedChildren.at(0);
    Q_ASSERT(index >= 0 && index < d->childWindows.size());
    QMdiSubWindow *current = d->childWindows.at(index);
    Q_ASSERT(current);
    return current;
}

/*!
    Returns a pointer to the current active subwindow. If no
    window is currently active, 0 is returned.

    Subwindows are treated as top-level windows with respect to
    window state, i.e., if a widget outside the MDI area is the active
    window, no subwindow will be active. Note that if a widget in the
    window in which the MDI area lives gains focus, the window will be
    activated.

    \sa setActiveSubWindow(), Qt::WindowState
*/
QMdiSubWindow *QMdiArea::activeSubWindow() const
{
    Q_D(const QMdiArea);
    return d->active;
}

/*!
    Activates the subwindow \a window. If \a window is 0, any
    current active window is deactivated.

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
    if (d->active)
        d->active->close();
}

/*!
    Returns a list of all subwindows in the MDI area. If \a order
    is CreationOrder (the default), the windows are sorted in the
    order in which they were inserted into the workspace. If \a order
    is StackingOrder, the windows are listed in their stacking order,
    with the topmost window as the last item in the list.

    \sa WindowOrder
*/
QList<QMdiSubWindow *> QMdiArea::subWindowList(WindowOrder order) const
{
    Q_D(const QMdiArea);
    return d->subWindowList(order, false);
}

/*!
    Closes all subwindows by sending a QCloseEvent to each window.
    You may receive subWindowActivated() signals from subwindows
    before they are closed (if the MDI area activates the subwindow
    when another is closing).

    Subwindows that ignore the close event will remain open.

    \sa closeActiveSubWindow()
*/
void QMdiArea::closeAllSubWindows()
{
    Q_D(QMdiArea);
    if (d->childWindows.isEmpty())
        return;

    d->isSubWindowsTiled = false;
    foreach (QMdiSubWindow *child, d->childWindows) {
        if (!sanityCheck(child, "QMdiArea::closeAllSubWindows"))
            continue;
        child->close();
    }
}

/*!
    Gives the keyboard focus to the next window in the list of child
    windows.  The windows are activated in the order in which they are
    created (CreationOrder).

    \sa activatePreviousSubWindow()
*/
void QMdiArea::activateNextSubWindow()
{
    Q_D(QMdiArea);
    if (d->childWindows.isEmpty())
        return;

    QMdiSubWindow *next = d->nextVisibleSubWindow(1);
    if (next)
        d->activateWindow(next);
}

/*!
    Gives the keyboard focus to the previous window in the list of
    child windows. The windows are activated in the order in which
    they are created (CreationOrder).

    \sa activateNextSubWindow()
*/
void QMdiArea::activatePreviousSubWindow()
{
    Q_D(QMdiArea);
    if (d->childWindows.isEmpty())
        return;

    QMdiSubWindow *previous = d->nextVisibleSubWindow(-1);
    if (previous)
        d->activateWindow(previous);
}

/*!
    Adds \a widget as a new subwindow to the MDI area.  If \a
    windowFlags are non-zero, they will override the flags set on the
    widget.

    The \a widget can be either a QMdiSubWindow or another QWidget
    (in which case the MDI area will create a subwindow and set the \a
    widget as the internal widget).

    \quotefromfile snippets/mdiareasnippets.cpp
    \skipto /QMdiArea mdiArea/
    \printto /subWindow1->show/

    When you create your own subwindow, you must set the
    Qt::WA_DeleteOnClose widget attribute if you want the window to be
    deleted when closed in the MDI area. If not, the window will be
    hidden and the MDI area will not activate the next subwindow.

    Returns the QMdiSubWindow that is added to the MDI area.

    \sa removeSubWindow()
*/
QMdiSubWindow *QMdiArea::addSubWindow(QWidget *widget, Qt::WindowFlags windowFlags)
{
    if (!widget) {
        qWarning("QMdiArea::addSubWindow: null pointer to widget");
        return 0;
    }

    Q_D(QMdiArea);
    // QWidget::setParent clears focusWidget so store it
    QWidget *childFocus = widget->focusWidget();
    QMdiSubWindow *child = qobject_cast<QMdiSubWindow *>(widget);

    // Widget is already a QMdiSubWindow
    if (child) {
        if (d->childWindows.indexOf(child) != -1) {
            qWarning("QMdiArea::addSubWindow: window is already added");
            return child;
        }
        child->setParent(viewport(), windowFlags ? windowFlags : child->windowFlags());
    // Create a QMdiSubWindow
    } else {
        child = new QMdiSubWindow(viewport(), windowFlags);
        child->setAttribute(Qt::WA_DeleteOnClose);
        child->setWidget(widget);
        Q_ASSERT(child->testAttribute(Qt::WA_DeleteOnClose));
    }

    if (childFocus)
        childFocus->setFocus();
    d->appendChild(child);
    return child;
}

/*!
    Removes \a widget from the MDI area. The \a widget must be
    either a QMdiSubWindow or a widget that is the internal widget of
    a subwindow. Note that the subwindow is not deleted by QMdiArea
    and that its parent is set to 0.

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
        d->indicesToActivatedChildren.removeAll(index);
        d->updateActiveWindow(index, d->active == child);
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

/*!
    \property QMdiArea::background
    \brief the background brush for the workspace

    This property sets the background brush for the workspace area
    itself. By default, it is a gray color, but can be any brush
    (e.g., colors, gradients or pixmaps).
*/
QBrush QMdiArea::background() const
{
    return d_func()->background;
}

void QMdiArea::setBackground(const QBrush &brush)
{
    Q_D(QMdiArea);
    if (d->background != brush) {
        d->background = brush;
        update();
    }
}

QMdiArea::WindowOrder QMdiArea::activationOrder() const
{
    Q_D(const QMdiArea);
    return d->activationOrder;
}

void QMdiArea::setActivationOrder(WindowOrder order)
{
    Q_D(QMdiArea);
    if (order != d->activationOrder)
        d->activationOrder = order;
}

/*!
    If \a on is true, \a option is enabled on the MDI area; otherwise
    it is disabled. See AreaOption for the effect of each option.

    \sa AreaOption, testOption()
*/
void QMdiArea::setOption(AreaOption option, bool on)
{
    Q_D(QMdiArea);
    if (on && !(d->options & option))
        d->options |= option;
    else if (!on && (d->options & option))
        d->options &= ~option;
}

/*!
    Returns true if \a option is enabled; otherwise returns false.

    \sa AreaOption, setOption()
*/
bool QMdiArea::testOption(AreaOption option) const
{
    return d_func()->options & option;
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

    // Re-tile the views if we're in tiled mode. Re-tile means we will change
    // the geometry of the children, which in turn means 'isSubWindowsTiled'
    // is set to false, so we have to update the state at the end.
    if (d->isSubWindowsTiled) {
        d->tileCalledFromResizeEvent = true;
        tileSubWindows();
        d->tileCalledFromResizeEvent = false;
        d->isSubWindowsTiled = true;
        d->startResizeTimer();
        // We don't have scroll bars or any maximized views.
        return;
    }

    d->updateScrollBars();

    // Resize maximized views.
    bool hasMaximizedSubWindow = false;
    foreach (QMdiSubWindow *child, d->childWindows) {
        if (sanityCheck(child, "QMdiArea::resizeEvent") && child->isMaximized()
                && child->size() != resizeEvent->size()) {
            child->resize(resizeEvent->size());
            if (!hasMaximizedSubWindow)
                hasMaximizedSubWindow = true;
        }
    }

    // Minimized views are stacked under maximized views so there's
    // no need to re-arrange minimized views on-demand. Start a timer
    // just to make things faster with subsequent resize events.
    if (hasMaximizedSubWindow)
        d->startResizeTimer();
    else
        d->arrangeMinimizedSubWindows();
}

/*!
    \reimp
*/
void QMdiArea::timerEvent(QTimerEvent *timerEvent)
{
    Q_D(QMdiArea);
    if (timerEvent->timerId() == d->resizeTimerId) {
        killTimer(d->resizeTimerId);
        d->resizeTimerId = -1;
        d->arrangeMinimizedSubWindows();
    }
}

/*!
    \reimp
*/
void QMdiArea::showEvent(QShowEvent *showEvent)
{
    Q_D(QMdiArea);
    if (!d->pendingRearrangements.isEmpty()) {
        bool skipPlacement = false;
        foreach (Rearranger *rearranger, d->pendingRearrangements) {
            // If this is the case, we don't have to lay out pending child windows
            // since the rearranger will find a placement for them.
            if (rearranger->type() != Rearranger::IconTiler && !skipPlacement)
                skipPlacement = true;
            d->rearrange(rearranger);
        }
        d->pendingRearrangements.clear();

        if (skipPlacement && !d->pendingPlacements.isEmpty())
            d->pendingPlacements.clear();
    }

    if (!d->pendingPlacements.isEmpty()) {
        foreach (QMdiSubWindow *window, d->pendingPlacements) {
            if (!window)
                continue;
            if (!window->testAttribute(Qt::WA_Resized)) {
                QSize newSize(window->sizeHint().boundedTo(viewport()->size()));
                window->resize(newSize.expandedTo(window->minimumSize()));
            }
            if (!window->testAttribute(Qt::WA_Moved) && !window->isMinimized()
                    && !window->isMaximized()) {
                d->place(d->placer, window);
            }
        }
        d->pendingPlacements.clear();
    }

    d->setChildActivationEnabled(true);
    d->activateCurrentWindow();

    QAbstractScrollArea::showEvent(showEvent);
}

/*!
    \reimp
*/
bool QMdiArea::viewportEvent(QEvent *event)
{
    Q_D(QMdiArea);
    switch (event->type()) {
    case QEvent::ChildRemoved: {
        d->isSubWindowsTiled = false;
        QObject *removedChild = static_cast<QChildEvent *>(event)->child();
        for (int i = 0; i < d->childWindows.size(); ++i) {
            QObject *child = d->childWindows.at(i);
            if (!child || child == removedChild || !child->parent()
                    || child->parent() != viewport()) {
                if (!testOption(DontMaximizeSubWindowOnActivation)) {
                    // In this case we can only rely on the child being a QObject
                    // (or 0), but let's try and see if we can get more information.
                    QWidget *mdiChild = qobject_cast<QWidget *>(removedChild);
                    if (mdiChild && mdiChild->isMaximized())
                        d->showActiveWindowMaximized = true;
                }
                const bool activeRemoved = i == d->indicesToActivatedChildren.at(0);
                d->childWindows.removeAt(i);
                d->indicesToActivatedChildren.removeAll(i);
                d->updateActiveWindow(i, activeRemoved);
                d->arrangeMinimizedSubWindows();
                break;
            }
        }
        d->updateScrollBars();
        break;
    }
    case QEvent::Destroy:
        d->isSubWindowsTiled = false;
        d->resetActiveWindow();
        d->childWindows.clear();
        qWarning("QMdiArea: Deleting the view port is undefined, use setViewport instead.");
        break;
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
    const bool wasSubWindowsTiled = d->isSubWindowsTiled;
    d->ignoreGeometryChange = true;
    viewport()->scroll(isLeftToRight() ? dx : -dx, dy);
    d->arrangeMinimizedSubWindows();
    d->ignoreGeometryChange = false;
    if (wasSubWindowsTiled)
        d->isSubWindowsTiled = true;
}

/*!
    Arranges all child windows in a tile pattern.

    \sa cascadeSubWindows()
*/
void QMdiArea::tileSubWindows()
{
    Q_D(QMdiArea);
    if (!d->regularTiler)
        d->regularTiler = new RegularTiler;
    d->rearrange(d->regularTiler);
}

/*!
    Arranges all the child windows in a cascade pattern.

    \sa tileSubWindows()
*/
void QMdiArea::cascadeSubWindows()
{
    Q_D(QMdiArea);
    if (!d->cascader)
        d->cascader = new SimpleCascader;
    d->rearrange(d->cascader);
}

/*!
    \reimp
*/
bool QMdiArea::event(QEvent *event)
{
    Q_D(QMdiArea);
    switch (event->type()) {
    case QEvent::WindowActivate: {
        d->isActivated = true;
        if (d->childWindows.isEmpty())
            break;
        if (!d->active)
            d->activateCurrentWindow();
        d->setChildActivationEnabled(false, true);
        break;
    }
    case QEvent::WindowDeactivate:
        d->isActivated = false;
        d->setChildActivationEnabled(false, true);
        break;
    case QEvent::StyleChange:
        // Re-tile the views if we're in tiled mode. Re-tile means we will change
        // the geometry of the children, which in turn means 'isSubWindowsTiled'
        // is set to false, so we have to update the state at the end.
        if (d->isSubWindowsTiled) {
            tileSubWindows();
            d->isSubWindowsTiled = true;
        }
        break;
    case QEvent::WindowIconChange:
        foreach (QMdiSubWindow *window, d->childWindows) {
            if (sanityCheck(window, "QMdiArea::WindowIconChange"))
                QApplication::sendEvent(window, event);
        }
        break;
    case QEvent::Hide:
        d->setActive(d->active, false);
        d->setChildActivationEnabled(false);
        break;
    default:
        break;
    }
    return QAbstractScrollArea::event(event);
}

/*!
    \reimp
*/
bool QMdiArea::eventFilter(QObject *object, QEvent *event)
{
    Q_D(QMdiArea);
    if (!qobject_cast<QMdiSubWindow *>(object)) {
        // QApplication events:
        if (event->type() == QEvent::ApplicationActivate && !d->active)
            d->activateCurrentWindow();
        else if (event->type() == QEvent::ApplicationDeactivate && d->active)
            d->setActive(d->active, false);
        return QAbstractScrollArea::eventFilter(object, event);
    }

    // QMdiSubWindow events:
    switch (event->type()) {
    case QEvent::Move:
    case QEvent::Resize:
        if (d->tileCalledFromResizeEvent)
            break;
        d->updateScrollBars();
        if (!static_cast<QMdiSubWindow *>(object)->isMinimized())
            d->isSubWindowsTiled = false;
        break;
    case QEvent::Show:
    case QEvent::Hide:
        d->isSubWindowsTiled = false;
        break;
    default:
        break;
    }
    return QAbstractScrollArea::eventFilter(object, event);
}

/*!
    \reimp
*/
void QMdiArea::paintEvent(QPaintEvent *paintEvent)
{
    Q_D(QMdiArea);
    QPainter painter(viewport());
    const QVector<QRect> exposedRects = paintEvent->region().rects();
    for (int i = 0; i < exposedRects.size(); ++i)
        painter.fillRect(exposedRects.at(i), d->background);
}

/*!
    This slot is called by QAbstractScrollArea after setViewport() has been
    called. Reimplement this function in a subclass of QMdiArea to
    initialize the new \a viewport before it is used.

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

#endif // QT_NO_MDIAREA
