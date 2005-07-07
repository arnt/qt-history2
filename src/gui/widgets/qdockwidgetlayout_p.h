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

#ifndef QDOCKWIDGETLAYOUT_P_H
#define QDOCKWIDGETLAYOUT_P_H

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

#include <qlayout.h>
#include <qlist.h>

#ifndef QT_NO_MAINWINDOW

class QDockWidget;
class QDockWidgetSeparator;

struct QDockWidgetLayoutInfo
{
    QLayoutItem *item;

    int cur_pos;
    int cur_size;
    int min_size;
    int max_size;

    uint is_sep     : 1;
    uint is_dropped : 1;
    uint reserved   : 13;

    inline QDockWidgetLayoutInfo(QLayoutItem *i)
	: item(i), cur_pos(-1), cur_size(-1), min_size(1), max_size(-1),
	  is_sep(0), is_dropped(0), reserved(0)
    { }
};

class QDockWidgetLayout : public QLayout
{
    Q_OBJECT

public:
    Qt::DockWidgetArea area;
    Qt::Orientation orientation;
    QList<QDockWidgetLayoutInfo> layout_info;
    QList<QDockWidgetLayoutInfo> *save_layout_info;
    mutable QSize minSize;
    mutable QSize szHint;

    QDockWidgetLayout(Qt::DockWidgetArea a, Qt::Orientation o);
    ~QDockWidgetLayout();

    enum { // sentinel values used to validate state data
        Marker = 0xfc,
        WidgetMarker = 0xfb
    };
    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream);

    // QLayout interface
    void addItem(QLayoutItem *layoutitem);
    void setGeometry(const QRect &rect);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);
    int count() const;
    QSize sizeHint() const;
    QSize minimumSize() const;
    void invalidate();
    bool isEmpty() const;

    QInternal::RelayoutType relayout_type;
    void relayout(QInternal::RelayoutType type = QInternal::RelayoutNormal);

    void setOrientation(Qt::Orientation o);
    QDockWidgetLayoutInfo &insert(int index, QLayoutItem *layoutitem);

    void dump();

    void saveLayoutInfo();
    void resetLayoutInfo();
    void discardLayoutInfo();

    QPoint constrain(QDockWidgetSeparator *sep, int delta);

    struct Location {
        int index;
        Qt::DockWidgetArea area;
    };
    Location locate(const QPoint &mouse) const;
    QRect place(QDockWidget *dockwidget, const QRect &r, const QPoint &mouse);
    void drop(QDockWidget *dockwidget, const QRect &r, const QPoint &mouse);

    void extend(QDockWidget *dockwidget, Qt::Orientation direction);
    void split(QDockWidget *existing, QDockWidget *with, Qt::DockWidgetArea area);

signals:
    void emptied();

private slots:
    void maybeDelete();
};

static inline int pick(Qt::Orientation o, const QPoint &p)
{ return o == Qt::Horizontal ? p.x() : p.y(); }
static inline int pick(Qt::Orientation o, const QSize &s)
{ return o == Qt::Horizontal ? s.width() : s.height(); }
static inline int pick_perp(Qt::Orientation o, const QPoint &p)
{ return o == Qt::Vertical ? p.x() : p.y(); }
static inline int pick_perp(Qt::Orientation o, const QSize &s)
{ return o == Qt::Vertical ? s.width() : s.height(); }

#endif // QT_NO_MAINWINDOW
#endif // QDOCKWIDGETLAYOUT_P_H
