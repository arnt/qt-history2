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

#ifndef QSPLITTER_P_H
#define QSPLITTER_P_H

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

#include <private/qframe_p.h>

class QSplitterLayoutStruct;
class QSplitterPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QSplitter)
public:
    QSplitterPrivate() : rubberBand(0), opaque(false), firstShow(true),
                         childrenCollapsible(true), compatMode(false), handleWidth(0) {}

    QPointer<QRubberBand> rubberBand;
    mutable QList<QSplitterLayoutStruct *> list;
    Qt::Orientation orient;
    bool opaque : 8;
    bool firstShow : 8;
    bool childrenCollapsible : 8;
    bool compatMode : 8;
    int handleWidth;

    inline int pick(const QPoint &p) const
    { return orient == Qt::Horizontal ? p.x() : p.y(); }
    inline int pick(const QSize &s) const
    { return orient == Qt::Horizontal ? s.width() : s.height(); }

    inline int trans(const QPoint &p) const
    { return orient == Qt::Vertical ? p.x() : p.y(); }
    inline int trans(const QSize &s) const
    { return orient == Qt::Vertical ? s.width() : s.height(); }

    void init();
    void recalc(bool update = false);
    void doResize();
    void storeSizes();
    void getRange(int id, int *, int *, int *, int *);
    void addContribution(int, int *, int *, bool);
    int adjustPos(int, int, int *, int *, int *, int *);
    bool collapsible(QSplitterLayoutStruct *);
    QSplitterLayoutStruct *findWidget(QWidget *);
    QSplitterLayoutStruct *addWidget(QWidget *, bool prepend = false);
    void recalcId();
    void doMove(bool backwards, int pos, int id, int delta,
                bool mayCollapse, int *positions, int *widths);
    void setGeo(QSplitterLayoutStruct *s, int pos, int size, bool splitterMoved);
    int findWidgetJustBeforeOrJustAfter(int id, int delta, int &collapsibleSize);
    void updateHandles();

};

#endif // QSPLITTER_P_H
