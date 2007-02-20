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

class QWindowSurface;

class QWidgetBackingStore
{
public:
    QWidgetBackingStore(QWidget *t);
    ~QWidgetBackingStore();
    void bltRect(const QRect &rect, int dx, int dy, QWidget *widget);
    void dirtyRegion(const QRegion &rgn, QWidget *widget=0);
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
    void removeDirtyWidget(QWidget *w);

private:
    QWidget *tlw;
#ifndef Q_WS_QWS
    QRegion dirty;
#endif
    QList<QWidget*> dirtyWidgets;

    QWindowSurface *windowSurface;

    QPoint tlwOffset;

    static bool isOpaque(const QWidget *widget);

    void copyToScreen(const QRegion &rgn, QWidget *widget, const QPoint &offset, bool recursive = true);

    static void paintSiblingsRecursive(QPaintDevice *pdev, const QObjectList& children, int index, const QRegion &rgn, const QPoint &offset, int flags);

    friend void qt_syncBackingStore(QRegion, QWidget *);
#if defined(Q_WS_X11) || defined(Q_WS_QWS) || defined(Q_WS_WIN)
    friend void qt_syncBackingStore(QWidget *);
#endif
    friend class QWidgetPrivate;
    friend class QWidget;
    friend class QWSManagerPrivate;
    friend class QETWidget;
    friend class QWSWindowSurface;
};

#endif // QBACKINGSTORE_P_H
