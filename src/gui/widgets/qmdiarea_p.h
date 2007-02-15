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

#ifndef QMDIAREA_P_H
#define QMDIAREA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmdiarea.h"
#include "qmdisubwindow.h"

#include <QList>
#include <QRect>
#include <QPoint>
#include <private/qabstractscrollarea_p.h>

class Rearranger
{
public:
    // Rearranges widgets relative to domain.
    virtual void rearrange(QList<QWidget *> &widgets, const QRect &domain) const = 0;
    virtual ~Rearranger() {}
};

class RegularTiler : public Rearranger
{
    // Rearranges widgets according to a regular tiling pattern
    // covering the entire domain.
    // Both positions and sizes may change.
    void rearrange(QList<QWidget *> &widgets, const QRect &domain) const;
};

class SimpleCascader : public Rearranger
{
    // Rearranges widgets according to a simple, regular cascading pattern.
    // Widgets are resized to minimumSize.
    // Both positions and sizes may change.
    void rearrange(QList<QWidget *> &widgets, const QRect &domain) const;
};

class IconTiler : public Rearranger
{
    // Rearranges icons (assumed to be the same size) according to a regular
    // tiling pattern filling up the domain from the bottom.
    // Only positions may change.
    void rearrange(QList<QWidget *> &widgets, const QRect &domain) const;
};

class Placer
{
public:
    // Places the rectangle defined by 'size' relative to 'rects' and 'domain'.
    // Returns the position of the resulting rectangle.
    virtual QPoint place(
        const QSize &size, const QList<QRect> &rects, const QRect &domain) const = 0;
    virtual ~Placer() {}
};

class MinOverlapPlacer : public Placer
{
    QPoint place(const QSize &size, const QList<QRect> &rects, const QRect &domain) const;
    static int accumulatedOverlap(const QRect &source, const QList<QRect> &rects);
    static QRect findMinOverlapRect(const QList<QRect> &source, const QList<QRect> &rects);
    static void getCandidatePlacements(
        const QSize &size, const QList<QRect> &rects, const QRect &domain,
        QList<QRect> &candidates);
    static QPoint findBestPlacement(
        const QRect &domain, const QList<QRect> &rects, QList<QRect> &source);
    static void findNonInsiders(
        const QRect &domain, QList<QRect> &source, QList<QRect> &result);
    static void findMaxOverlappers(
        const QRect &domain, const QList<QRect> &source, QList<QRect> &result);
};

class QMdiAreaPrivate : public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(QMdiArea)
public:
    QMdiAreaPrivate();

    // Variables.
    QList< QPointer<QMdiSubWindow> > childWindows;
    QList<int> indicesToStackedChildren;
    QPointer<QMdiSubWindow> active;
    QPointer<QMdiSubWindow> aboutToBecomeActive;
    QBrush background;
    bool ignoreGeometryChange;
    bool isActivated;
    bool isSubWindowsTiled;
    int indexToNextWindow;
    int indexToPreviousWindow;

    // Slots.
    void _q_deactivateAllWindows();
    void _q_processWindowStateChanged(Qt::WindowStates oldState, Qt::WindowStates newState);

    // Functions.
    void appendChild(QMdiSubWindow *child);
    void place(const Placer &placer, QMdiSubWindow *child);
    void rearrange(const Rearranger &rearranger, bool icons = false);
    void activateWindow(QMdiSubWindow *child);
    void emitWindowActivated(QMdiSubWindow *child);
    void resetActiveWindow(QMdiSubWindow *child = 0);
    void updateActiveWindow(int removedIndex);
    void updateScrollBars();
    void internalRaise(QMdiSubWindow *child) const;
};

#endif // QMDIAREA_P_H
