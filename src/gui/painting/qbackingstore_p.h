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

#ifndef QBACKINGSTORE_P_H
#define QBACKINGSTORE_P_H

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

#include "private/qpaintengine_raster_p.h"
#include "private/qwidget_p.h"

class QWindowSurface;

class Q_AUTOTEST_EXPORT QWidgetBackingStore
{
public:
    QWidgetBackingStore(QWidget *t);
    ~QWidgetBackingStore();
    bool bltRect(const QRect &rect, int dx, int dy, QWidget *widget);
    void dirtyRegion(const QRegion &rgn, QWidget *widget=0);
#ifdef Q_RATE_LIMIT_PAINTING
    void updateDirtyTlwRegion();
#endif
    void cleanRegion(const QRegion &rgn, QWidget *widget=0, bool recursiveCopyToScreen = true);
#if defined (Q_WS_QWS) || defined (Q_WS_WIN)
    void releaseBuffer();
#endif

    inline QPoint topLevelOffset() const { return tlwOffset; }
    static bool paintOnScreen(QWidget * = 0);
    static void copyToScreen(QWidget *, const QRegion &);
#ifdef Q_WS_WIN
    static void blitToScreen(const QRegion &rgn, QWidget *w);
#endif
#ifdef Q_WIDGET_USE_DIRTYLIST
    void removeDirtyWidget(QWidget *w);
#endif

    static bool isOpaque(const QWidget *widget);

private:
    QWidget *tlw;
#ifdef Q_WS_QWS
    QRegion dirtyOnScreen;
#else
    QRegion dirty;
#endif
#ifdef Q_WIDGET_USE_DIRTYLIST
    QList<QWidget*> dirtyWidgets;
#endif

    QWindowSurface *windowSurface;
#ifdef Q_BACKINGSTORE_SUBSURFACES
    QList<QWindowSurface*> subSurfaces;
#endif
    QPoint tlwOffset;

    void copyToScreen(const QRegion &rgn, QWidget *widget, const QPoint &offset, bool recursive = true);

    static void paintSiblingsRecursive(QPaintDevice *pdev, const QObjectList& children, int index, const QRegion &rgn, const QPoint &offset, int flags
#ifdef Q_BACKINGSTORE_SUBSURFACES
                                                 , const QWindowSurface *currentSurface
#endif
        );

    static void updateWidget(QWidget *that, const QRegion &rgn);

    friend void qt_syncBackingStore(QRegion, QWidget *);
#if defined(Q_WS_X11) || defined(Q_WS_QWS) || defined(Q_WS_WIN)
    friend void qt_syncBackingStore(QWidget *);
#endif
    friend class QWidgetPrivate;
    friend class QWidget;
    friend class QWSManagerPrivate;
    friend class QETWidget;
    friend class QWindowSurface;
    friend class QWSWindowSurface;
};

#endif // QBACKINGSTORE_P_H
