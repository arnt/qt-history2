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

#ifndef QSCROLLVIEW_P_H
#define QSCROLLVIEW_P_H

#include <qhash.h>
#include <qtimer.h>

#include <private/qframe_p.h>

struct QSVChildRec;
class QViewportWidget;
class QClipperWidget;

class QScrollViewPrivate: public QFramePrivate
{
    Q_DECLARE_PUBLIC(QScrollView)

public:
    QScrollViewPrivate();
    virtual ~QScrollViewPrivate();

    void init();
    QSVChildRec* rec(QWidget* w);
    QSVChildRec* ancestorRec(QWidget* w);
    QSVChildRec* addChildRec(QWidget* w, int x, int y);
    void deleteChildRec(QSVChildRec* r);

    void hideOrShowAll(QScrollView* sv, bool isScroll = false);
    void moveAllBy(int dx, int dy);
    bool anyVisibleChildren() const;
    void autoMove(QScrollView* sv);
    void autoResize(QScrollView* sv);
    void autoResizeHint(QScrollView* sv);
    void viewportResized(int w, int h);

    QScrollBar*  hbar;
    QScrollBar*  vbar;
    bool hbarPressed;
    bool vbarPressed;
    QViewportWidget*        viewport;
    QClipperWidget*        clipped_viewport;
    Qt::WFlags flags;
    QList<QSVChildRec *> children;
    QHash<QWidget *, QSVChildRec *> childDict;
    QWidget*    corner, *defaultCorner;
    int         vx, vy, vwidth, vheight; // for drawContents-style usage
    int         l_marg, r_marg, t_marg, b_marg;
    QScrollView::ResizePolicy policy;
    QScrollView::ScrollBarMode  vMode;
    QScrollView::ScrollBarMode  hMode;
#ifndef QT_NO_DRAGANDDROP
    QPoint cpDragStart;
    QTimer autoscroll_timer;
    int autoscroll_time;
    int autoscroll_accel;
    bool drag_autoscroll;
#endif
    QTimer scrollbar_timer;

    uint static_bg : 1;
    uint fake_scroll : 1;

    // This variable allows ensureVisible to move the contents then
    // update both the sliders.  Otherwise, updating the sliders would
    // cause two image scrolls, creating ugly flashing.
    //
    uint signal_choke : 1;

    // This variables indicates in updateScrollBars() that we are
    // in a resizeEvent() and thus don't want to flash scrollbars
    uint inresize : 1;
    uint use_cached_size_hint : 1;
    mutable QSize cachedSizeHint;

    inline int contentsX() const { return -vx; }
    inline int contentsY() const { return -vy; }
    inline int contentsWidth() const { return vwidth; }

};

#endif
