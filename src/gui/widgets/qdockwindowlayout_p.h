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

#ifndef QDOCKWINDOWLAYOUT_P_H
#define QDOCKWINDOWLAYOUT_P_H

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

class QDockWindow;
class QDockWindowSeparator;

struct QDockWindowLayoutInfo
{
    QLayoutItem *item;

    int cur_pos;
    int cur_size;
    int min_size;
    int max_size;

    uint is_sep     : 1;
    uint is_dummy   : 1;
    uint is_dropped : 1;
    uint reserved   : 13;

    inline QDockWindowLayoutInfo(QLayoutItem *i)
	: item(i), cur_pos(-1), cur_size(-1), min_size(1), max_size(-1),
	  is_sep(0), is_dummy(0), is_dropped(0), reserved(0)
    { }
};

class QDockWindowLayout : public QLayout
{
    Q_OBJECT

public:
    Qt::DockWindowArea area;
    Qt::Orientation orientation;
    QList<QDockWindowLayoutInfo> layout_info;
    QList<QDockWindowLayoutInfo> *save_layout_info;

    QDockWindowLayout(Qt::DockWindowArea a, Qt::Orientation o);
    ~QDockWindowLayout();

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
    QSize sizeHint() const;
    QSize minimumSize() const;
    void invalidate();
    bool isEmpty() const;

    QInternal::RelayoutType relayout_type;
    void relayout(QInternal::RelayoutType type = QInternal::RelayoutNormal);

    void setOrientation(Qt::Orientation o);
    QLayoutItem *find(QWidget *widget);
    QDockWindowLayoutInfo &insert(int index, QLayoutItem *layoutitem, bool dummy = false);

    void dump();

    void saveLayoutInfo();
    void resetLayoutInfo();
    void discardLayoutInfo();

    QPoint constrain(QDockWindowSeparator *sep, int delta);

    struct Location {
        int index;
        Qt::DockWindowArea area;
    };
    Location locate(const QPoint &mouse) const;
    QRect place(QDockWindow *dockwindow, const QRect &r, const QPoint &mouse);
    void drop(QDockWindow *dockwindow, const QRect &r, const QPoint &mouse);

    void extend(QDockWindow *dockwindow, Qt::Orientation direction);
    void split(QDockWindow *existing, QDockWindow *with, Qt::DockWindowArea area);

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

#endif // QDOCKWINDOWLAYOUT_P_H
