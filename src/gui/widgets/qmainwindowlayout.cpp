#include "qmainwindowlayout_p.h"
#include "qdockseparator_p.h"
#include "qdockwindowlayout_p.h"

#include "qdockwindow.h"
#include "qmainwindow.h"
#include "qtoolbar.h"

#include <qapplication.h>
#include <qdebug.h>
#include <qevent.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qstyle.h>
#include <qvarlengtharray.h>
#include <qstack.h>
#include <qmap.h>

#include <private/qlayoutengine_p.h>

// #define LAYOUT_DEBUG
#if defined(LAYOUT_DEBUG)
#  define DEBUG qDebug
#else
#  define DEBUG if(false)qDebug
#endif

// #define LAYOUT_DEBUG_VERBOSE
#if defined(LAYOUT_DEBUG_VERBOSE)
#  define VDEBUG qDebug
#else
#  define VDEBUG if(false)qDebug
#endif

// #define TOOLBAR_DEBUG
#if defined(TOOLBAR_DEBUG)
#  define TBDEBUG qDebug
#else
#  define TBDEBUG if(false)qDebug
#endif



enum POSITION {
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    CENTER,
    NPOSITIONS
};

static inline uint areaForPosition(int pos)
{ return ((1u << pos) & 0xf); }

static inline POSITION positionForArea(Qt::DockWindowArea area)
{
    switch (area) {
    case Qt::DockWindowAreaLeft:   return LEFT;
    case Qt::DockWindowAreaRight:  return RIGHT;
    case Qt::DockWindowAreaTop:    return TOP;
    case Qt::DockWindowAreaBottom: return BOTTOM;
    default: break;
    }
    return CENTER;
}

static inline POSITION positionForArea(Qt::ToolBarArea area)
{
    switch (area) {
    case Qt::ToolBarAreaLeft:   return LEFT;
    case Qt::ToolBarAreaRight:  return RIGHT;
    case Qt::ToolBarAreaTop:    return TOP;
    case Qt::ToolBarAreaBottom: return BOTTOM;
    default: break;
    }
    return CENTER;
}

static inline QCOORD pick(POSITION p, const QSize &s)
{ return p == TOP || p == BOTTOM ? s.height() : s.width(); }

static inline QCOORD pick(POSITION p, const QPoint &pt)
{ return p == TOP || p == BOTTOM ? pt.y() : pt.x(); }

static inline void set(POSITION p, QSize &s, int x)
{ if (p == LEFT || p == RIGHT) s.setWidth(x); else s.setHeight(x); }

static inline QCOORD pick_perp(POSITION p, const QSize &s)
{ return p == TOP || p == BOTTOM ? s.width() : s.height(); }

static inline QCOORD pick_perp(POSITION p, const QPoint &pt)
{ return p == TOP || p == BOTTOM ? pt.x() : pt.y(); }

static inline void set_perp(POSITION p, QSize &s, int x)
{ if (p == TOP || p == BOTTOM) s.setWidth(x); else s.setHeight(x); }

static inline void set_perp(POSITION p, QPoint &pt, int x)
{ if (p == TOP || p == BOTTOM) pt.setX(x); else pt.setY(x); }

class QMainWindowLayoutItem : public QWidgetItem
{
public:
    inline QMainWindowLayoutItem(QWidget *w, const QRect &r)
        : QWidgetItem(w), rect(r)
    { }

    inline QSize sizeHint() const
    { return rect.size(); }
    inline void setGeometry( const QRect &r)
    { rect = r; }
    inline QRect geometry() const
    { return rect; }

    QWidget *widget;
    QRect rect;
};


QMainWindowLayout::QMainWindowLayout(QMainWindow *mainwindow)
    : QLayout(mainwindow), statusbar(0), relayout_type(QInternal::RelayoutNormal), save_layout_info(0),
      save_tb_layout_info(0)
{
    setObjectName(mainwindow->objectName() + "_layout");

    corners[TopLeft]     = Qt::DockWindowAreaTop;
    corners[TopRight]    = Qt::DockWindowAreaTop;
    corners[BottomLeft]  = Qt::DockWindowAreaBottom;
    corners[BottomRight] = Qt::DockWindowAreaBottom;

    for (int i = 0; i < Qt::NDockWindowAreas + 1; ++i) {
        QMainWindowLayoutInfo info;
        info.item = 0;
        info.sep = 0;
        info.is_dummy = false;
        layout_info.append(info);
    }
}

QMainWindowLayout::~QMainWindowLayout()
{
}

QStatusBar *QMainWindowLayout::statusBar() const
{ return statusbar ? qt_cast<QStatusBar *>(statusbar->widget()) : 0; }

void QMainWindowLayout::setStatusBar(QStatusBar *sb)
{ statusbar = new QWidgetItem(sb); }

QWidget *QMainWindowLayout::centerWidget() const
{ return layout_info[CENTER].item ? layout_info[CENTER].item->widget() : 0; }

void QMainWindowLayout::setCenterWidget(QWidget *cw)
{
    Q_ASSERT(cw->parentWidget() == parentWidget());
    Q_ASSERT(!layout_info[CENTER].item);
    layout_info[CENTER].item = new QWidgetItem(cw);
    layout_info[CENTER].size = QSize();
}

QDockWindowLayout *QMainWindowLayout::layoutForArea(Qt::DockWindowArea area)
{
    POSITION pos = positionForArea(area);
    QMainWindowLayoutInfo &info = layout_info[pos];
    QDockWindowLayout *l = 0;

    if (!info.item) {
        // create new dock window layout
        static const Qt::Orientation orientations[] = {
            Qt::Vertical,   // LEFT
            Qt::Vertical,   // RIGHT
            Qt::Horizontal, // TOP
            Qt::Horizontal, // BOTTOM
        };

        l = new QDockWindowLayout(this, orientations[pos]);
        l->setObjectName(objectName() + "_dockWindowLayout" + QString::number(area, 16));

        info.item = l;

        // create separator
        Q_ASSERT(!info.sep);
        info.sep = new QWidgetItem(new QDockSeparator(l, parentWidget()));
    } else {
        l = qt_cast<QDockWindowLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
    }
    return l;
}

void QMainWindowLayout::add(QToolBar *toolbar, Qt::ToolBarArea area, bool linebreak)
{ add(toolbar, positionForArea(area), linebreak); }

void QMainWindowLayout::add(QToolBar *toolbar, int where, bool linebreak, const QPoint &offset)
{
    Q_ASSERT(toolbar->parentWidget() == parentWidget());

    removeToolBarInfo(toolbar);

    ToolBarLayoutInfo newinfo;
    newinfo.item = new QWidgetItem(toolbar);
    newinfo.where = where;
    newinfo.offset = offset;
    newinfo.linebreak = linebreak;
    newinfo.is_dummy = false;

    placeToolBarInfo(newinfo);
}

QLayoutItem *QMainWindowLayout::itemAt(int index) const
{
    int x = 0;
    for (int i = 0; i < NPOSITIONS; ++i) {
        if (!layout_info[i].item) continue;
        if (x++ == index) return layout_info[i].item;
    }
    return 0;
}

QLayoutItem *QMainWindowLayout::takeAt(int index)
{
    DEBUG("QMainWindowLayout::takeAt: index %d", index);

    int x = 0;
    for (int i = 0; i < NPOSITIONS; ++i) {
        if (!layout_info[i].item) continue;
        if (x++ == index) {
            VDEBUG() << "  where" << i;
            QLayoutItem *ret = layout_info[i].item;

            if (!save_layout_info) {
                layout_info[i].size = QSize();

                if (layout_info[i].sep) {
                    delete layout_info[i].sep->widget();
                    delete layout_info[i].sep;
                }
            }

            layout_info[i].item = 0;
            layout_info[i].sep = 0;

            VDEBUG() << "END of QMainWindowLayout::takeAt (removed" << i << ")";
            return ret;
        }
    }

    VDEBUG() << "END of QMainWindowLayout::takeAt (not found)";
    return 0;
}

/*
  Fixes the mininum and maximum sizes depending on the current corner
  configuration.
*/
void fix_minmax(QVector<QLayoutStruct> &ls,
                const QMainWindowLayout * const layout,
                POSITION pos)
{
    const struct
    {
        Qt::Corner corner1, corner2;
        POSITION perp1, perp2;
    } order[] = {
        // LEFT
        {
            Qt::TopLeft,
            Qt::BottomLeft,
            TOP,
            BOTTOM
        },
        // RIGHT
        {
            Qt::TopRight,
            Qt::BottomRight,
            TOP,
            BOTTOM
        },
        // TOP
        {
            Qt::TopLeft,
            Qt::TopRight,
            LEFT,
            RIGHT
        },
        // BOTTOM
        {
            Qt::BottomLeft,
            Qt::BottomRight,
            LEFT,
            RIGHT
        }
    };

    if (layout->layout_info[pos].item) {
        if ((layout->corners[order[pos].corner1] != Qt::DockWindowArea(pos)
             || !layout->layout_info[order[pos].perp1].item)
            && (layout->corners[order[pos].corner2] != Qt::DockWindowArea(pos)
                || !layout->layout_info[order[pos].perp2].item)) {
            ls[1].minimumSize = qMax(pick_perp(pos, layout->layout_info[pos].item->minimumSize()),
                                     ls[1].minimumSize);
            ls[1].maximumSize = qMin(pick_perp(pos, layout->layout_info[pos].item->maximumSize()),
                                     ls[1].maximumSize);
        } else {
            const int min = pick_perp(pos, layout->layout_info[pos].item->minimumSize()),
                      dis = min / 2;
            if (layout->layout_info[order[pos].perp1].item) {
                if (layout->corners[order[pos].corner1] == Qt::DockWindowArea(pos)
                    && layout->corners[order[pos].corner2] != Qt::DockWindowArea(pos)) {
                    ls[0].minimumSize = qMax(ls[0].minimumSize, dis);
                    ls[1].minimumSize = qMax(ls[1].minimumSize, min - dis);
                }
            } else if (layout->layout_info[order[pos].perp2].item) {
                if (layout->corners[order[pos].corner2] == Qt::DockWindowArea(pos)
                    && layout->corners[order[pos].corner1] != Qt::DockWindowArea(pos)) {
                    ls[2].minimumSize = qMax(ls[2].minimumSize, dis);
                    ls[1].minimumSize = qMax(ls[1].minimumSize, min - dis);
                }
            }
        }
    }
}

/*
  Initializes the layout struct with information from the specified
  dock window area.
*/
static void init_layout_struct(const QMainWindowLayout * const layout,
                               QLayoutStruct &ls,
                               const POSITION pos,
                               const int ext)
{
    const QMainWindowLayout::QMainWindowLayoutInfo &info = layout->layout_info[pos];
    ls.empty = info.item->isEmpty();
    if (!ls.empty) {
        ls.minimumSize = pick(pos, info.item->minimumSize()) + ext;
        ls.maximumSize = pick(pos, info.item->maximumSize()) + ext;
        ls.sizeHint  = info.size.isValid()
                       ? pick(pos, info.size)
                       : pick(pos, info.item->sizeHint()) + ext;
        // constrain sizeHint
        ls.sizeHint = qMin(ls.maximumSize, ls.sizeHint);
        ls.sizeHint = qMax(ls.minimumSize, ls.sizeHint);
    }
}

/*
  Returns the minimum size hint for the first user item in a tb
  layout.
*/
static QSize get_min_item_sz(QLayout *layout)
{
    QLayoutItem *item = layout->itemAt(1);
    if (item && item->widget())
	return item->widget()->minimumSizeHint();
    else
	return QSize(0, 0);
}

void QMainWindowLayout::setGeometry(const QRect &_r)
{
    QLayout::setGeometry(_r);
    QRect r = _r;

    if (statusbar) {
        QRect sbr(QPoint(0, 0),
                  QSize(r.width(), statusbar->heightForWidth(r.width()))
                  .expandedTo(statusbar->minimumSize()));
        sbr.moveBottom(r.bottom());
        statusbar->setGeometry(sbr);
        r.setBottom(sbr.top() - 1);
    }

    // layout toolbars

    // calculate the respective tool bar rectangles and store the
    // width/height of the largest tb on each line
    QVarLengthArray<QRect> tb_rect(tb_layout_info.size());

    QMap<int, QSize> rest_sz;
    QStack<QSize> bottom_sz;
    QStack<QSize> right_sz;

    QStack<QRect> bottom_rect;
    QStack<QRect> right_rect;

    // calculate the width/height of the different tool bar lines -
    // the order of the bottom and right tool bars needs to reversed
    for (int k = 0; k < tb_layout_info.size(); ++k) {
	QSize tb_sz;
        for (int i = 0; i < tb_layout_info.at(k).size(); ++i) {
	    const ToolBarLayoutInfo &info = tb_layout_info.at(k).at(i);
	    QSize sh = info.item->sizeHint();
	    if (tb_sz.width() < sh.width())
		tb_sz.setWidth(sh.width());
	    if (tb_sz.height() < sh.height())
		tb_sz.setHeight(sh.height());

	}
	switch(tb_layout_info.at(k).at(0).where) {
	case TOP:
	case LEFT:
	    rest_sz[k] = tb_sz;
	    break;
	case BOTTOM:
	    bottom_sz.push(tb_sz);
	    break;
	case RIGHT:
	    right_sz.push(tb_sz);
	    break;
	default:
	    Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
	}
    }

    // calculate the rect for each tool bar line
    for (int k = 0; k < tb_layout_info.size(); ++k) {
	QSize tb_sz;
	tb_rect[k] = r;

	switch (tb_layout_info.at(k).at(0).where) {
	case TOP:
	    tb_sz = rest_sz[k];
	    tb_rect[k].setBottom(tb_rect[k].top() + tb_sz.height());
	    r.setTop(tb_rect[k].bottom() + 1);
	    break;
	case LEFT:
	    tb_sz = rest_sz[k];
	    tb_rect[k].setRight(tb_rect[k].x() + tb_sz.width());
	    r.setLeft(tb_rect[k].right() + 1);
	    break;
	case BOTTOM:
 	    tb_sz = bottom_sz.pop();
	    tb_rect[k].setTop(tb_rect[k].bottom() - tb_sz.height());
	    bottom_rect.push(tb_rect[k]);
	    r.setBottom(tb_rect[k].top() - 1);
	    break;
	case RIGHT:
 	    tb_sz = right_sz.pop();
	    tb_rect[k].setLeft(tb_rect[k].right() - tb_sz.width());
	    right_rect.push(tb_rect[k]);
	    r.setRight(tb_rect[k].left() - 1);
	    break;
	default:
	    Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
	}
    }

    // put the right and bottom rects back in correct order
    for (int k = 0; k < tb_layout_info.size(); ++k) {
	if (tb_layout_info.at(k).at(0).where == BOTTOM)
	    tb_rect[k] = bottom_rect.pop();
	else if (tb_layout_info.at(k).at(0).where == RIGHT)
	    tb_rect[k] = right_rect.pop();
    }


    // at this point the space for the tool bars have been shaved off
    // the main rect, continue laying out each tool bar line

    int tb_fill = 0;
    if (tb_layout_info.size() != 0) {
	tb_fill = QApplication::style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent)
		  + 16 // ## size of extension - get this from somewhere else
		  + qt_cast<QToolBar *>(tb_layout_info.at(0).at(0).item->widget())->frameWidth() * 2
		  + qt_cast<QBoxLayout *>(tb_layout_info.at(0).at(0).item->widget()->layout())->margin() * 2
		  + qt_cast<QBoxLayout *>(tb_layout_info.at(0).at(0).item->widget()->layout())->spacing() * 3;
    }

    for (int k = 0; k < tb_layout_info.size(); ++k) {
	int num_tbs = tb_layout_info.at(k).size();
	POSITION where = static_cast<POSITION>(tb_layout_info.at(k).at(0).where);
	QPoint lastPos = tb_rect[k].topLeft();

        for (int i = 0; i < num_tbs; ++i) {
            ToolBarLayoutInfo &info = tb_layout_info[k][i];

 	    set(where, info.size, pick(where, tb_rect[k].size()));

	    // position
 	    if (i == 0) { // first tool bar can't have an offset
		if (num_tbs > 1 && pick_perp(where, info.offset) > pick_perp(where, info.size)) {
		    // swap if dragging it past the next one
		    ToolBarLayoutInfo &next = tb_layout_info[k][i+1];
		    next.pos = tb_rect[k].topLeft();
		    next.size.setHeight(pick_perp(where, get_min_item_sz(next.item->widget()->layout())) + tb_fill);
		    next.offset = QPoint();
		    if (where == LEFT || where == RIGHT)
			info.pos = QPoint(tb_rect[k].left(), next.pos.y() + next.size.height());
		    else
			info.pos = QPoint(next.pos.x() + next.size.width(), tb_rect[k].top());
		    info.offset = QPoint(); // has to be done before swapping
		    tb_layout_info[k].swap(i, i+1);
		} else {
		    info.pos = tb_rect[k].topLeft();
		    info.offset = QPoint();
		}
	    } else {
		ToolBarLayoutInfo &prev = tb_layout_info[k][i-1];
		QSize min = info.item->widget()->layout()->itemAt(1)->widget()->minimumSizeHint();
		set_perp(where, min, pick_perp(where, min) + tb_fill);
 		const int cur_pt = pick_perp(where, prev.pos) + pick_perp(where, prev.size);
		const int prev_min = pick_perp(where, get_min_item_sz(prev.item->widget()->layout())) + tb_fill;

		if (where == LEFT || where == RIGHT)
		    info.pos = QPoint(tb_rect[k].left(), cur_pt + info.offset.y());
		else
		    info.pos = QPoint(cur_pt + info.offset.x(), tb_rect[k].top());

		if (pick_perp(where, info.offset) < 0) { // left/up motion
		    if (pick_perp(where, prev.size) + pick_perp(where, info.offset) >= prev_min) {
			// shrink the previous one and increase size of current with same
			QSize sz(0, 0);
			set_perp(where, sz, pick_perp(where, info.offset));
			prev.size += sz;
			info.size -= sz;
		    } else {
			// can't shrink - push the previous one if possible
			bool can_push = false;
			for (int l = i-2; l >= 0; --l) {
			    ToolBarLayoutInfo &t = tb_layout_info[k][l];
			    if (pick_perp(where, t.size) + pick_perp(where, info.offset) >
				pick_perp(where, get_min_item_sz(t.item->widget()->layout())) + tb_fill) {
				QSize sz(0, 0);
				set_perp(where, sz, pick_perp(where, info.offset) + pick_perp(where, prev.size) - prev_min);
				t.size += sz;
				can_push = true;
				break;
			    }
			}
			if (can_push) {
			    set_perp(where, prev.pos, pick_perp(where, prev.pos) + pick_perp(where, info.offset));
			    set_perp(where, prev.size, prev_min);
			    set_perp(where, info.pos, pick_perp(where, info.pos) + pick_perp(where, info.offset));
			    QSize sz(0,0);
			    set_perp(where, sz, pick_perp(where, info.offset));
			    info.size -= sz;
			} else {
			    QSize sz(0,0);
			    set_perp(where, sz, pick_perp(where, prev.size) - prev_min);
			    info.size += sz;

			    if (pick_perp(where, info.pos) < pick_perp(where, prev.pos))
				tb_layout_info[k].swap(i, i-1);
			}
		    }

		} else if (pick_perp(where, info.offset) > 0) { // right/down motion
		    if (pick_perp(where, info.size) - pick_perp(where, info.offset) > pick_perp(where, min)) {
			QSize sz(0, 0);
			set_perp(where, sz, pick_perp(where, info.offset));
			info.size -= sz;
		    } else {
			bool can_push = false;
			for (int l = i+1; l < num_tbs; ++l) {
			    ToolBarLayoutInfo &t = tb_layout_info[k][l];
			    if (pick_perp(where, t.size) - pick_perp(where, info.offset)
				> pick_perp(where, get_min_item_sz(t.item->widget()->layout())) + tb_fill) {
				QPoint pt;
				set_perp(where, pt, pick_perp(where, info.offset));
				t.pos += pt;
				t.size -= QSize(pt.x(), pt.y());
				can_push = true;
				break;
			    }
			}
			if (!can_push) {
			    int can_remove = pick_perp(where, info.size) - pick_perp(where, min);
			    set_perp(where, info.pos, cur_pt + can_remove);
			    QSize sz(0, 0);
			    set_perp(where, sz, can_remove);
			    info.size -= sz;
			    if (i+1 < num_tbs) {
				ToolBarLayoutInfo &t = tb_layout_info[k][i+1];

				if (pick_perp(where, info.pos) + pick_perp(where, info.offset) > pick_perp(where, t.pos))
				    tb_layout_info[k].swap(i, i+1);
			    }
			}
		    }
		}

		if (pick_perp(where, info.pos) < pick_perp(where, prev.pos) + prev_min) {
		    set_perp(where, info.pos, pick_perp(where, prev.pos) + prev_min);
		}
		info.offset = QPoint();
	    }

	    // size
	    if (num_tbs == 1)
		set_perp(where, info.size, pick_perp(where, tb_rect[k].size()));
	    else if (i == num_tbs-1) {
		set_perp(where, info.size, pick_perp(where, tb_rect[k].size()));
		if (where == LEFT || where == RIGHT)
		    info.size.setHeight(tb_rect[k].bottom() - info.pos.y() + 1);
		else
		    info.size.setWidth(tb_rect[k].right() - info.pos.x() + 1);
		if (pick_perp(where, info.size) < 1)
		    set_perp(where, info.size, pick_perp(where, get_min_item_sz(info.item->widget()->layout())) + tb_fill);
	    }
	    if (i > 0) {
		// assumes that all tbs are positioned at the correct
		// pos - fill in the gaps btw them
		ToolBarLayoutInfo &prev = tb_layout_info[k][i-1];
		set_perp(where, prev.size, pick_perp(where, info.pos) - pick_perp(where, prev.pos));
	    }
	}

	for (int i = 0; i < num_tbs; ++i) {
	    ToolBarLayoutInfo &info = tb_layout_info[k][i];
	    QRect tb(info.pos, info.size);
	    if (tb.isValid() && (relayout_type == QInternal::RelayoutNormal || info.is_dummy))
		info.item->setGeometry(tb);
	}
    }

    // layout dockwindows and center widget
    const int ext = QApplication::style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent);

    if (relayout_type == QInternal::RelayoutNormal) {
        // hide separators for empty layouts
        for (int i = 0; i < 4; ++i) {
            if (!layout_info[i].item) continue;
            layout_info[i].sep->widget()->setHidden(layout_info[i].item->isEmpty());
        }
    }

    {
        QVector<QLayoutStruct> lsH(3);
        for (int i = 0; i < 3; ++i)
            lsH[i].init();

        if (layout_info[LEFT].item)
            init_layout_struct(this, lsH[0], LEFT, ext);
        if (layout_info[RIGHT].item)
            init_layout_struct(this, lsH[2], RIGHT, ext);

        lsH[1].empty = false;
        lsH[1].minimumSize =
            layout_info[CENTER].item ? layout_info[CENTER].item->minimumSize().width() : 0;
        lsH[1].maximumSize =
            layout_info[CENTER].item ? layout_info[CENTER].item->maximumSize().width() : INT_MAX;
        lsH[1].sizeHint =
            layout_info[CENTER].size.isValid()
            ? layout_info[CENTER].size.width()
            : (layout_info[CENTER].item ? layout_info[CENTER].item->sizeHint().width() : 0);
        // fix min/max sizes
        fix_minmax(lsH, this, TOP);
        fix_minmax(lsH, this, BOTTOM);
        // constrain sizeHint
        lsH[1].sizeHint = qMin(lsH[1].maximumSize, lsH[1].sizeHint);
        lsH[1].sizeHint = qMax(lsH[1].minimumSize, lsH[1].sizeHint);
        // the center widget always gets stretch
        lsH[1].stretch = lsH[1].sizeHint;

        qGeomCalc(lsH, 0, lsH.count(), 0, r.width(), 0);

        if (!lsH[0].empty)
            layout_info[LEFT].size.setWidth(lsH[0].size);
        layout_info[CENTER].size.setWidth(lsH[1].size);
        if (!lsH[2].empty)
            layout_info[RIGHT].size.setWidth(lsH[2].size);
    }

    {
        QVector<QLayoutStruct> lsV(3);
        for (int i = 0; i < 3; ++i)
            lsV[i].init();

        if (layout_info[TOP].item)
            init_layout_struct(this, lsV[0], TOP, ext);
        if (layout_info[BOTTOM].item)
            init_layout_struct(this, lsV[2], BOTTOM, ext);

        lsV[1].empty = false;
        lsV[1].minimumSize =
            layout_info[CENTER].item ? layout_info[CENTER].item->minimumSize().height() : 0;
        lsV[1].maximumSize =
            layout_info[CENTER].item ? layout_info[CENTER].item->maximumSize().height() : INT_MAX;
        lsV[1].sizeHint =
            layout_info[CENTER].size.isValid()
            ? layout_info[CENTER].size.height()
            : (layout_info[CENTER].item ? layout_info[CENTER].item->sizeHint().height() : 0);
        // fix min/max sizes
        fix_minmax(lsV, this, LEFT);
        fix_minmax(lsV, this, RIGHT);
        // constrain sizeHint
        lsV[1].sizeHint = qMin(lsV[1].maximumSize, lsV[1].sizeHint);
        lsV[1].sizeHint = qMax(lsV[1].minimumSize, lsV[1].sizeHint);
        // the center widget always gets stretch
        lsV[1].stretch = lsV[1].sizeHint;

        qGeomCalc(lsV, 0, lsV.count(), 0, r.height(), 0);

        if (!lsV[0].empty)
            layout_info[TOP].size.setHeight(lsV[0].size);
        layout_info[CENTER].size.setHeight(lsV[1].size);
        if (!lsV[2].empty)
            layout_info[BOTTOM].size.setHeight(lsV[2].size);
    }

    QRect rect[4],
        &left   = rect[LEFT],
        &right  = rect[RIGHT],
        &top    = rect[TOP],
        &bottom = rect[BOTTOM];

    for (int i = 0; i < 4; ++i) {
        if (!layout_info[i].item || layout_info[i].item->isEmpty())
            rect[i].setSize(QSize(0, 0));
        else
            rect[i].setSize(layout_info[i].size);
    }

    left.moveLeft(r.left());
    right.moveRight(r.right());
    top.moveTop(r.top());
    bottom.moveBottom(r.bottom());

    switch (corners[TopLeft]) {
    case Qt::DockWindowAreaTop:
        top.setLeft(r.left());
        left.setTop(top.bottom() + 1);
        break;
    case Qt::DockWindowAreaLeft:
        left.setTop(r.top());
        top.setLeft(left.right() + 1);
        break;
    default:
        Q_ASSERT(false);
    }

    switch (corners[BottomLeft]) {
    case Qt::DockWindowAreaBottom:
        bottom.setLeft(r.left());
        left.setBottom(bottom.top() - 1);
        break;
    case Qt::DockWindowAreaLeft:
        left.setBottom(r.bottom());
        bottom.setLeft(left.right() + 1);
        break;
    default:
        Q_ASSERT(false);
    }

    switch (corners[TopRight]) {
    case Qt::DockWindowAreaTop:
        top.setRight(r.right());
        right.setTop(top.bottom() + 1);
        break;
    case Qt::DockWindowAreaRight:
        right.setTop(r.top());
        top.setRight(right.left() - 1);
        break;
    default:
        Q_ASSERT(false);
    }

    switch (corners[BottomRight]) {
    case Qt::DockWindowAreaBottom:
        bottom.setRight(r.right());
        right.setBottom(bottom.top() - 1);
        break;
    case Qt::DockWindowAreaRight:
        right.setBottom(r.bottom());
        bottom.setRight(right.left() - 1);
        break;
    default:
        Q_ASSERT(false);
    }

    for (int i = 0; i < 4; ++i) {
        if (!layout_info[i].item) continue;
        layout_info[i].size = rect[i].size();
        QRect x, s;

        switch (i) {
        case LEFT:
            x.setCoords(rect[i].left(),
                        rect[i].top(),
                        rect[i].right() - ext,
                        rect[i].bottom());
            s.setCoords(x.right() + 1,
                        rect[i].top(),
                        rect[i].right(),
                        rect[i].bottom());
            break;
        case RIGHT:
            x.setCoords(rect[i].left() + ext,
                        rect[i].top(),
                        rect[i].right(),
                        rect[i].bottom());
            s.setCoords(rect[i].left(),
                        rect[i].top(),
                        x.left() - 1,
                        rect[i].bottom());
            break;
        case TOP:
            x.setCoords(rect[i].left(),
                        rect[i].top(),
                        rect[i].right(),
                        rect[i].bottom() - ext);
            s.setCoords(rect[i].left(),
                        x.bottom() + 1,
                        rect[i].right(),
                        rect[i].bottom());
            break;
        case BOTTOM:
            x.setCoords(rect[i].left(),
                        rect[i].top() + ext,
                        rect[i].right(),
                        rect[i].bottom());
            s.setCoords(rect[i].left(),
                        rect[i].top(),
                        rect[i].right(),
                        x.top() - 1);
            break;
        default:
            Q_ASSERT(false);
        }

        if (relayout_type == QInternal::RelayoutNormal || layout_info[i].is_dummy)
            layout_info[i].item->setGeometry(x);
        if (relayout_type == QInternal::RelayoutNormal)
            layout_info[i].sep->setGeometry(s);
    }

    if (layout_info[CENTER].item) {
        QRect c;
        c.setCoords(left.right() + 1, top.bottom() + 1, right.left() - 1, bottom.top() - 1);
        layout_info[CENTER].size = c.size();
        if (relayout_type == QInternal::RelayoutNormal)
            layout_info[CENTER].item->setGeometry(c);
    }
}

void QMainWindowLayout::addItem(QLayoutItem *item)
{
    Q_ASSERT_X(item->layout(), "QMainWindowLayout::addItem", "internal error");
}

QSize QMainWindowLayout::sizeHint() const
{
    int left = 0, right = 0, top = 0, bottom = 0;

    // layout toolbars
    for (int k = 0; k < tb_layout_info.size(); ++k) {
        QSize sz;
        // need to get the biggest size hint for all tool bars on each line
        for (int i = 0; i < tb_layout_info.at(k).size(); ++i) {
            const ToolBarLayoutInfo &info = tb_layout_info.at(k).at(i);
            QSize ms = info.item->sizeHint();

            if (((info.where == LEFT || info.where == RIGHT) && (ms.width() > sz.width()))
                || (ms.height() > sz.height()))
                sz = ms;
        }
        switch (tb_layout_info.at(k).at(0).where) {
        case LEFT:
            left += sz.width();
            break;

        case RIGHT:
            right += sz.width();
            break;

        case TOP:
            top += sz.height();
            break;

        case BOTTOM:
            bottom += sz.height();
            break;

        default:
            Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
        }
    }

    const QSize szC = layout_info[CENTER].item
                      ? layout_info[CENTER].item->sizeHint()
                      : QSize(0, 0),
                szL = layout_info[LEFT].item
                      ? layout_info[LEFT].item->sizeHint()
                      : QSize(0, 0),
                szT = layout_info[TOP].item
                      ? layout_info[TOP].item->sizeHint()
                      : QSize(0, 0),
                szR = layout_info[RIGHT].item
                      ? layout_info[RIGHT].item->sizeHint()
                      : QSize(0, 0),
                szB = layout_info[BOTTOM].item
                      ? layout_info[BOTTOM].item->sizeHint()
                      : QSize(0, 0);
    int h1, h2, h3, w1, w2, w3;

    w1 = (corners[TopLeft] == Qt::DockWindowAreaLeft ? szL.width() : 0)
         + szT.width()
         + (corners[TopRight] == Qt::DockWindowAreaRight ? szR.width() : 0);
    w2 = szL.width() + szR.width() + szC.width();
    w3 = (corners[BottomLeft] == Qt::DockWindowAreaLeft ? szL.width() : 0)
         + szB.width()
         + (corners[BottomRight] == Qt::DockWindowAreaRight ? szR.width(): 0);

    h1 = (corners[TopLeft] == Qt::DockWindowAreaTop ? szT.height() : 0)
         + szL.height()
         + (corners[BottomLeft] == Qt::DockWindowAreaBottom ? szB.height(): 0);
    h2 = szT.height() + szB.height() + szC.height();
    h3 = (corners[TopRight] == Qt::DockWindowAreaTop ? szT.height() : 0)
         + szR.height()
         + (corners[BottomRight] == Qt::DockWindowAreaBottom ? szB.height() : 0);

    const int ext = QApplication::style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
    if (layout_info[LEFT].item && szL.isValid())
        left += ext;
    if (layout_info[RIGHT].item && szR.isValid())
        right += ext;
    if (layout_info[TOP].item && szT.isValid())
        top += ext;
    if (layout_info[BOTTOM].item && szT.isValid())
        bottom += ext;

    VDEBUG("QMainWindowLayout::sizeHint:\n"
           "    %4dx%4d (l %4d r %4d t %4d b %4d w1 %4d w2 %4d w3 %4d, h1 %4d h2 %4d h3 %4d)",
           qMax(qMax(w1,w2),w3), qMax(qMax(h1,h2),h3),
           left, right, top, bottom, w1, w2, w3, h1, h2, h3);

    QSize szSB = statusbar ? statusbar->sizeHint() : QSize(0, 0);
    return QSize(qMax(szSB.width(), qMax(qMax(w1,w2),w3) + left + right),
                 szSB.height() + qMax(qMax(h1,h2),h3) + top + bottom);
}

QSize QMainWindowLayout::minimumSize() const
{
    int left = 0, right = 0, top = 0, bottom = 0;

    // layout toolbars
    for (int k = 0; k < tb_layout_info.size(); ++k) {
        QSize sz;
        // need to get the biggest min size for all tool bars on each line
        for (int i = 0; i < tb_layout_info.at(k).size(); ++i) {
            const ToolBarLayoutInfo &info = tb_layout_info.at(k).at(i);
            QSize ms = info.item->minimumSize();

            if (((info.where == LEFT || info.where == RIGHT) && (ms.width() > sz.width()))
                || (ms.height() > sz.height()))
                sz = ms;
        }
        switch (tb_layout_info.at(k).at(0).where) {
        case LEFT:
            left += sz.width();
            break;

        case RIGHT:
            right += sz.width();
            break;

        case TOP:
            top += sz.height();
            break;

        case BOTTOM:
            bottom += sz.height();
            break;

        default:
            Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
        }
    }

    const QSize szC = layout_info[CENTER].item
                      ? layout_info[CENTER].item->minimumSize()
                      : QSize(0, 0),
                szL = layout_info[LEFT].item
                      ? layout_info[LEFT].item->minimumSize()
                      : QSize(0, 0),
                szT = layout_info[TOP].item
                      ? layout_info[TOP].item->minimumSize()
                      : QSize(0, 0),
                szR = layout_info[RIGHT].item
                      ? layout_info[RIGHT].item->minimumSize()
                      : QSize(0, 0),
                szB = layout_info[BOTTOM].item
                      ? layout_info[BOTTOM].item->minimumSize()
                      : QSize(0, 0);
    int h1, h2, h3, w1, w2, w3;

    w1 = (corners[TopLeft] == Qt::DockWindowAreaLeft ? szL.width() : 0)
         + szT.width()
         + (corners[TopRight] == Qt::DockWindowAreaRight ? szR.width() : 0);
    w2 = szL.width() + szR.width() + szC.width();
    w3 = (corners[BottomLeft] == Qt::DockWindowAreaLeft ? szL.width() : 0)
         + szB.width()
         + (corners[BottomRight] == Qt::DockWindowAreaRight ? szR.width() : 0);

    h1 = (corners[TopLeft] == Qt::DockWindowAreaTop ? szT.height() : 0)
         + szL.height()
         + (corners[BottomLeft] == Qt::DockWindowAreaBottom ? szB.height() : 0);
    h2 = szT.height() + szB.height() + szC.height();
    h3 = (corners[TopRight] == Qt::DockWindowAreaTop ? szT.height() : 0)
         + szR.height()
         + (corners[BottomRight] == Qt::DockWindowAreaBottom ? szB.height() : 0);

    const int ext = QApplication::style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
    if (layout_info[LEFT].item && szL.isValid())
        left += ext;
    if (layout_info[RIGHT].item && szR.isValid())
        right += ext;
    if (layout_info[TOP].item && szT.isValid())
        top += ext;
    if (layout_info[BOTTOM].item && szT.isValid())
        bottom += ext;

    VDEBUG("QMainWindowLayout::minimumSizeHint:\n"
           "    %4dx%4d (l %4d r %4d t %4d b %4d w1 %4d w2 %4d w3 %4d, h1 %4d h2 %4d h3 %4d)",
           qMax(qMax(w1,w2),w3), qMax(qMax(h1,h2),h3),
           left, right, top, bottom, w1, w2, w3, h1, h2, h3);

    QSize szSB = statusbar ? statusbar->minimumSize() : QSize(0, 0);
    return QSize(qMax(szSB.width(), qMax(qMax(w1,w2),w3) + left + right),
                 szSB.height() + qMax(qMax(h1,h2),h3) + top + bottom);
}

void QMainWindowLayout::relayout(QInternal::RelayoutType type)
{
    QInternal::RelayoutType save_type = relayout_type;
    relayout_type = type;
    setGeometry(geometry());
    relayout_type = save_type;
}

void QMainWindowLayout::invalidate()
{
    if (relayout_type != QInternal::RelayoutDragging)
        QLayout::invalidate();
}

void QMainWindowLayout::saveLayoutInfo()
{
    Q_ASSERT(save_layout_info == 0);
    save_layout_info = new QVector<QMainWindowLayoutInfo>(layout_info);
    relayout_type = QInternal::RelayoutDragging;

    for (int i = 0; i < 4; ++i) {
        if (!layout_info[i].item) continue;

        QDockWindowLayout *layout =
            qt_cast<QDockWindowLayout *>(layout_info[i].item->layout());
        Q_ASSERT(layout != 0);
        layout->saveLayoutInfo();
    }
}

void QMainWindowLayout::resetLayoutInfo()
{
    Q_ASSERT(save_layout_info != 0);
    layout_info = *save_layout_info;

    for (int i = 0; i < 4; ++i) {
        if (!layout_info[i].item) continue;

        QDockWindowLayout *layout =
            qt_cast<QDockWindowLayout *>(layout_info[i].item->layout());
        Q_ASSERT(layout != 0);
        layout->resetLayoutInfo();
    }
}

void QMainWindowLayout::discardLayoutInfo()
{
    Q_ASSERT(save_layout_info != 0);
    delete save_layout_info;
    save_layout_info = 0;

    relayout_type = QInternal::RelayoutNormal;

    for (int i = 0; i < 4; ++i) {
        if (!layout_info[i].item) continue;

        QDockWindowLayout *layout =
            qt_cast<QDockWindowLayout *>(layout_info[i].item->layout());
        Q_ASSERT(layout != 0);
        layout->discardLayoutInfo();
    }
}

void QMainWindowLayout::beginConstrain()
{
    save_tb_layout_info = new QList<ToolBarLineInfo>(tb_layout_info);
}

void QMainWindowLayout:: endConstrain()
{
    delete save_tb_layout_info;
    save_tb_layout_info = 0;
}

int QMainWindowLayout::constrain(QDockWindowLayout *dock, int delta)
{
    QVector<QMainWindowLayoutInfo> info = save_layout_info ? *save_layout_info : layout_info;

    const POSITION order[] = {
        // LEFT
        RIGHT,
        // RIGHT
        LEFT,
        // TOP
        BOTTOM,
        // BOTTOM
        TOP
    };

    // which dock are we constraining?
    POSITION pos;
    if (info[LEFT].item && info[LEFT].item->layout() == dock) {
        pos = LEFT;
    } else if (info[TOP].item && info[TOP].item->layout() == dock) {
        pos = TOP;
    } else if (info[RIGHT].item && info[RIGHT].item->layout() == dock) {
        pos = RIGHT;
        delta = -delta;
    } else if (info[BOTTOM].item && info[BOTTOM].item->layout() == dock) {
        pos = BOTTOM;
        delta = -delta;
    } else {
        Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
    }

    // remove delta from 'dock'
    int current = pick(pos, info[pos].size)
                  + pick(pos, info[CENTER].size)
                  + pick(pos, info[order[pos]].size);

    const int _ext = QApplication::style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
    const QSize ext(_ext, _ext);
    const QSize dmin = info[pos].item->minimumSize() + ext,
                dmax = info[pos].item->maximumSize() + ext;
    if (pick(pos, info[pos].size) + delta < pick(pos, dmin))
        delta = pick(pos, dmin) - pick(pos, info[pos].size);
    if (pick(pos, info[pos].size) + delta > pick(pos, dmax))
        delta = pick(pos, dmax) - pick(pos, info[pos].size);

    // remove delta from the center widget
    int from_center = delta;
    if (from_center) {
        const QSize cmin = info[CENTER].item->minimumSize(),
                    cmax = info[CENTER].item->maximumSize();
        if (pick(pos, info[CENTER].size) - from_center < pick(pos, cmin))
            from_center = pick(pos, info[CENTER].size) - pick(pos, cmin);
        if (pick(pos, info[CENTER].size) - from_center > pick(pos, cmax))
            from_center = pick(pos, cmax) - pick(pos, info[CENTER].size);
        set(pos, info[CENTER].size, pick(pos, info[CENTER].size) - from_center);
    }

    // remove remaining delta from the other dock (i.e. the one opposite us)
    int from_other = 0;
    if (info[order[pos]].item) {
        from_other = delta - from_center;
        if (from_other) {
            const QSize omin = info[order[pos]].item->minimumSize() + ext,
                        omax = info[order[pos]].item->maximumSize() + ext;
            if (pick(pos, info[order[pos]].size) - from_other < pick(pos, omin))
                from_other = pick(pos, info[order[pos]].size) - pick(pos, omin);
            if (pick(pos, info[order[pos]].size) - from_other > pick(pos, omax))
                from_other = pick(pos, omax) - pick(pos, info[order[pos]].size);
            set(pos, info[order[pos]].size, pick(pos, info[order[pos]].size) - from_other);
        }
    }

    // the real delta is how much we stole from the center and right widgets
    delta = from_center + from_other;
    set(pos, info[pos].size, pick(pos, info[pos].size) + delta);

    int new_current = (pick(pos, info[pos].size)
                       + pick(pos, info[CENTER].size)
                       + pick(pos, info[order[pos]].size));
    Q_ASSERT(current == new_current);

    delta = info[pos].size == layout_info[pos].size ? 0 : 1;

    layout_info = info;

    return delta;
}

int
QMainWindowLayout::locateDockWindow(QDockWindow *dockwindow, const QPoint &mouse) const
{
    VDEBUG() << "  locate: mouse" << mouse;

    QPoint p = parentWidget()->mapFromGlobal(mouse);

    /*
      if there is a window dock layout under the mouse, forward the
      place request
    */
    for (int i = 0; i < 4; ++i) {
        if (!layout_info[i].item) continue;

        Qt::DockWindowArea area = static_cast<Qt::DockWindowArea>(areaForPosition(i));

        if (! (dockwindow->allowedAreas() & area)) continue;

        if (!layout_info[i].item->geometry().contains(p)
            && !layout_info[i].sep->geometry().contains(p)) continue;
        VDEBUG() << "  result: mouse over item" << i;
        return i;
    }

    POSITION pos = CENTER;
    const int width = parentWidget()->width(),
             height = parentWidget()->height(),
                 dx = QABS(p.x() - (width / 2)),
                 dy = QABS(p.y() - (height / 2));

    if (dx > dy) {
        if (p.x() < width / 2 && dockwindow->isDockable(Qt::DockWindowAreaLeft)) {
            // left side
            if (p.y() < height / 3 && dockwindow->isDockable(Qt::DockWindowAreaTop))
                pos = positionForArea(corners[TopLeft]);
            else if (p.y() > height * 2 / 3 && dockwindow->isDockable(Qt::DockWindowAreaBottom))
                pos = positionForArea(corners[BottomLeft]);
            else
                pos = LEFT;
        } else if (p.x() > width / 2 && dockwindow->isDockable(Qt::DockWindowAreaRight)) {
            // right side
            if (p.y() < height / 3 && dockwindow->isDockable(Qt::DockWindowAreaTop))
                pos = positionForArea(corners[TopRight]);
            else if (p.y() > height * 2 / 3 && dockwindow->isDockable(Qt::DockWindowAreaBottom))
                pos = positionForArea(corners[BottomRight]);
            else
                pos = RIGHT;
        }

        if (pos == CENTER) {
            if (p.y() < height / 2 && dockwindow->isDockable(Qt::DockWindowAreaTop))
                pos = TOP;
            else if (p.y() >= height / 2 && dockwindow->isDockable(Qt::DockWindowAreaBottom))
                pos = BOTTOM;
        }
    } else {
        if (p.y() < height / 2 && dockwindow->isDockable(Qt::DockWindowAreaTop)) {
            // top side
            if (p.x() < width / 3 && dockwindow->isDockable(Qt::DockWindowAreaLeft))
                pos = positionForArea(corners[TopLeft]);
            else if (p.x() > width * 2 / 3 && dockwindow->isDockable(Qt::DockWindowAreaRight))
                pos = positionForArea(corners[TopRight]);
            else
                pos = TOP;
        } else if (p.y() > height / 2 && dockwindow->isDockable(Qt::DockWindowAreaBottom)) {
            // bottom side
            if (p.x() < width / 3 && dockwindow->isDockable(Qt::DockWindowAreaLeft))
                pos = positionForArea(corners[BottomLeft]);
            else if (p.x() > width * 2 / 3 && dockwindow->isDockable(Qt::DockWindowAreaRight))
                pos = positionForArea(corners[BottomRight]);
            else
                pos = BOTTOM;
        }

        if (pos == CENTER) {
            if (p.x() < width / 2 && dockwindow->isDockable(Qt::DockWindowAreaLeft))
                pos = LEFT;
            else if (p.x() >= width / 2 && dockwindow->isDockable(Qt::DockWindowAreaRight))
                pos = RIGHT;
        }
    }

    VDEBUG() << "  result:" << pos;
    return pos;
}

QRect QMainWindowLayout::placeDockWindow(QDockWindow *dockwindow,
                                          const QRect &r,
                                          const QPoint &mouse)
{
    DEBUG("QMainWindowLayout::placeDockWindow");

    POSITION pos = static_cast<POSITION>(locateDockWindow(dockwindow, mouse));
    QRect target;

    if (pos == CENTER) {
        DEBUG() << "END of QMainWindowLayout::placeDockWindow (failed to place)";
        return target;
    }

    // if there is a window dock layout already here, forward the place
    if (layout_info[pos].item) {
        DEBUG("  forwarding...");
        QDockWindowLayout *l = qt_cast<QDockWindowLayout *>(layout_info[pos].item->layout());
        Q_ASSERT(l != 0);
        target = l->place(dockwindow, r, mouse);
        DEBUG("END of QMainWindowLayout::placeDockWindow (forwarded)");
        return target;
    }

    // remove dockwindow from current position in the layout
    removeRecursive(dockwindow);

    // see if the tool window will fix in the main window
    const QSize cur = parentWidget()->size();

    QMainWindowLayoutItem layoutitem(dockwindow, r);
    layout_info[pos].item = &layoutitem;
    layout_info[pos].size = r.size();
    DEBUG() << "  pos" << pos << " size" << layout_info[pos].size;
    layout_info[pos].is_dummy = true;

    relayout(QInternal::RelayoutDragging);

    const QSize new_min = minimumSize();
    const bool forbid = cur.width() < new_min.width() || cur.height() < new_min.height();

    if (!forbid) {
        DEBUG() << "  placed at " << layoutitem.geometry();
        target = layoutitem.geometry();
        target.moveTopLeft(parentWidget()->mapToGlobal(target.topLeft()));
    } else {
        DEBUG() << "  forbidden, minimum size" << new_min << " larger than current size" << cur;
    }

    DEBUG() << "END of QMainWindowLayout::placeDockWindow, target" << target;

    return target;
}

void QMainWindowLayout::dropDockWindow(QDockWindow *dockwindow,
                                        const QRect &r,
                                        const QPoint &mouse)
{
    DEBUG("QMainWindowLayout::dropDockWindow");

    POSITION pos = static_cast<POSITION>(locateDockWindow(dockwindow, mouse));

    if (pos == CENTER) {
        DEBUG() << "END of QMainWindowLayout::dropDockWindow (failed to place)";
        return;
    }

    // if there is a window dock layout already here, forward the drop
    if (layout_info[pos].item) {
        DEBUG() << "  forwarding...";
        QDockWindowLayout *l = qt_cast<QDockWindowLayout *>(layout_info[pos].item->layout());
        Q_ASSERT(l);
        l->drop(dockwindow, r, mouse);
        DEBUG() << "END of QMainWindowLayout::dropDockWindow (forwarded)";
        return;
    }

    // remove dockwindow from current position in the layout
    removeRecursive(dockwindow);

    dockwindow->setParent(qt_cast<QMainWindow *>(parentWidget()));
    QDockWindowLayout *dwl = layoutForArea(static_cast<Qt::DockWindowArea>(areaForPosition(pos)));
    dwl->addWidget(dockwindow);

    layout_info[pos].size = r.size();

    relayout();

    dockwindow->show();
    layout_info[pos].sep->widget()->show();

    DEBUG() << "END of QMainWindowLayout::dropDockWindow";
}

static bool removeWidgetRecursively(QLayoutItem *li, QWidget *w, bool dummy)
{
    QLayout *lay = li->layout();
    if (!lay)
        return false;
    int i = 0;
    QLayoutItem *child;
    while ((child = lay->itemAt(i))) {
        if (child->widget() == w) {
            QLayoutItem *item = lay->takeAt(i);
            if (!dummy)
                delete item;
            return true;
        } else if (removeWidgetRecursively(child, w, dummy)) {
            return true;
        } else {
            ++i;
        }
    }
    return false;
}

void QMainWindowLayout::removeRecursive(QDockWindow *dockwindow)
{
    removeWidgetRecursively(this, dockwindow, save_layout_info != 0);
}

int QMainWindowLayout::locateToolBar(const QPoint &mouse) const
{
    const int width = parentWidget()->width(),
             height = parentWidget()->height();
    const QPoint p = parentWidget()->mapFromGlobal(mouse),
                p2 = QPoint(width / 2, height / 2);
    const int dx = QABS(p.x() - p2.x()),
              dy = QABS(p.y() - p2.y());
    POSITION where = ((dx > dy)
		      ? ((p.x() < p2.x()) ? LEFT : RIGHT)
		      : ((p.y() < p2.y()) ? TOP : BOTTOM));

    for (int k = 0; k < tb_layout_info.size(); ++k) {
	bool break_it = false;
        for (int i = 0; i < tb_layout_info.at(k).size(); ++i) {
	    const ToolBarLayoutInfo &info = tb_layout_info.at(k).at(i);
	    if (!info.item) continue;
 	    if (!info.item->geometry().contains(p)) continue;
	    where = static_cast<POSITION>(info.where);
	    break_it = true;
	    break;
	}
	if (break_it)
	    break;
    }
    return where;
}

QRect QMainWindowLayout::placeToolBar(QToolBar *toolbar, const QPoint &mouse, const QPoint &offset)
{
    POSITION where = static_cast<POSITION>(locateToolBar(mouse));

    // save layout info
    beginConstrain();

    // remove the item we're trying to place and insert a dummy item
    // to test the layout
    QPoint old_offset;
    for (int k = 0; k < tb_layout_info.size(); ++k) {
        for (int i = 0; i < tb_layout_info.at(k).size(); ++i) {
            if (tb_layout_info.at(k).at(i).item->widget() == toolbar) {
		old_offset = tb_layout_info.at(k).at(i).offset;
		tb_layout_info[k].removeAt(i);
                if (tb_layout_info.at(k).size() == 0) // remove empty lines
                    tb_layout_info.removeAt(k--);
                break;
            }
        }
    }

    int cur_pos = positionForArea(toolbar->currentArea());
    QSize sh;

    // need to calc new size hint when we change orientation of the
    // tool bar
    if (((cur_pos == LEFT || cur_pos == RIGHT) && where != LEFT && where != RIGHT)
        || ((cur_pos == TOP || cur_pos == BOTTOM) && where != TOP && where != BOTTOM))
    {
	QLayoutItem *item = 0;
	QLayout *layout = toolbar->layout();
	int i = 0;
	while ((item = layout->itemAt(i++))) {
	    const QSize mh = item->widget()->sizeHint();
	    if (where == LEFT || where == RIGHT) {
		if (sh.width() < mh.width())
		    sh.setWidth(mh.width());
		sh.setHeight(sh.height() + mh.height() + layout->spacing());
	    } else if (where == TOP || where == BOTTOM) {
		if (sh.height() < mh.height())
		    sh.setHeight(mh.height());
		sh.setWidth(sh.width() + mh.width() + layout->spacing());
	    }
	}
	int marg = layout->margin()*2 + toolbar->frameWidth()*2;
	sh += QSize(marg, marg);
	//         qDebug() << "  ## pos change detected: " << docknames[cur_pos] << " -> " << docknames[where] << "sh: " << sh;
    } else {
	sh = toolbar->size();
    }

    ToolBarLayoutInfo newinfo;
    QMainWindowLayoutItem layoutitem(toolbar, QRect(0, 0, sh.width(), sh.height()));
    newinfo.item = &layoutitem;
    newinfo.size = sh;
    newinfo.where = where;
    newinfo.offset = offset + old_offset;
    newinfo.is_dummy = true;
    placeToolBarInfo(newinfo);

    QRect target;
    const QSize cur = parentWidget()->size();

    relayout(QInternal::RelayoutDragging);

    const QSize new_min = minimumSize();
//     const bool forbid = cur.width() < new_min.width() ||
//                         cur.height() < new_min.height();
    //     qDebug() << (forbid ? "#FORBID" : "ALLOW") << "CURRENT SZ:" << cur << "NEW MIN:" << new_min << "sh:" << sh;

//    if (!forbid) {
        target = layoutitem.geometry();
        target.moveTopLeft(parentWidget()->mapToGlobal(target.topLeft()));
	//  }

    tb_layout_info = *save_tb_layout_info;
    endConstrain();

    return target;
}

void QMainWindowLayout::dropToolBar(QToolBar *toolbar, const QPoint &mouse, const QPoint &offset)
{
    POSITION where = static_cast<POSITION>(locateToolBar(mouse));

    if (positionForArea(toolbar->currentArea()) == where) {

#ifdef TOOLBAR_DEBUG
	TBDEBUG() << "###";
	for (int l = 0; l < tb_layout_info.size(); ++l) {
	    for (int i = 0; i < tb_layout_info.at(l).size(); ++i) {
		ToolBarLayoutInfo &tmp = tb_layout_info[l][i];
 		qDebug() << "bar: " << tmp.where << l << i << tmp.item->widget() << tmp.item->widget()->geometry();
	    }
	}
#endif

	int l = 0, i = 0;
	ToolBarLayoutInfo info;

	for (l = 0; l < tb_layout_info.size(); ++l) {
	    bool break_it = false;
	    for (i = 0; i < tb_layout_info.at(l).size(); ++i) {
		ToolBarLayoutInfo &tmp = tb_layout_info[l][i];
		if (tmp.item->widget() == toolbar) {
		    info = tmp;
		    tmp.offset += offset;
		    break_it = true;
		    break;
		}
	    }
	    if (break_it) break;
	}

	if (pick(where, offset) < 0) { // move left/up
	    TBDEBUG() << "left/up" << offset << l << where;
	    if (l > 0 && tb_layout_info.at(l-1).at(0).where == where) { // is this the first line in this tb area?
		tb_layout_info[l].removeAt(i);
		if (tb_layout_info[l].size() == 0)
		    tb_layout_info.removeAt(l);
		if (tb_layout_info.at(l-1).at(0).where == where) {
		    TBDEBUG() << "br 1 appending to existing" << info.item->widget() << info.item->widget()->geometry();
		    tb_layout_info[l-1].append(info);
		} else {
		    ToolBarLineInfo line;
		    line.append(info);
		    TBDEBUG() << "br 2 new tb_line";
		    tb_layout_info.insert(l, line);
		}
	    } else if (tb_layout_info.at(l).size() > 1) {
		tb_layout_info[l].removeAt(i);
		ToolBarLineInfo line;
		line.append(info);
		tb_layout_info.insert(l, line);
		TBDEBUG() << "br3 new tb line" << l << toolbar;
	    }
	} else if (pick(where, offset) > pick(where, info.size)) { // move right/down
	    TBDEBUG() << "right/down" << offset;
	    if (l < tb_layout_info.size()-1 && tb_layout_info.at(l+1).at(0).where == where) {
		tb_layout_info[l].removeAt(i);

		if (tb_layout_info.at(l).size() == 0)
		    tb_layout_info.removeAt(l--);

		if (tb_layout_info.at(l+1).at(0).where == where) {
		    tb_layout_info[l+1].append(info);
		    TBDEBUG() << "1. appending to exisitng";
		} else {
		    ToolBarLineInfo line;
		    line.append(info);
		    tb_layout_info.insert(l, line);
		    TBDEBUG() << "2. inserting new";
		}
	    } else if (tb_layout_info.at(l).size() > 1) { // can remove
		tb_layout_info[l].removeAt(i);
		ToolBarLineInfo line;
		line.append(info);
		tb_layout_info.insert(l+1, line);
		TBDEBUG() << "3. new line";

	    }
	}
    } else { // changed area?
	add(toolbar, where, false, offset);
	toolbar->setCurrentArea(static_cast<Qt::ToolBarArea>(areaForPosition(where)));
    }
    relayout();
}

void QMainWindowLayout::placeToolBarInfo(const ToolBarLayoutInfo &newinfo)
{
    if (!newinfo.linebreak) {
	// see if we have an existing line in the tb - append it in the last in line
	for (int k = 0; k < tb_layout_info.size(); ++k) {
	    POSITION where = static_cast<POSITION>(tb_layout_info.at(k).at(0).where);
	    if (where == newinfo.where) {
		while (k < tb_layout_info.size()-1 && where == tb_layout_info.at(k+1).at(0).where) ++k;
		tb_layout_info[k].append(newinfo);
		return;
	    }
	}
    }

    switch (newinfo.where) {
    case TOP:
    case BOTTOM:
    {
	for (int k = 0; k < tb_layout_info.size(); ++k) {
	    POSITION where = static_cast<POSITION>(tb_layout_info.at(k).at(0).where);
 	    if (where == LEFT || where == RIGHT) {
		ToolBarLineInfo line;
		line.append(newinfo);
		tb_layout_info.insert(k, line);
		line[0].linebreak = false;
		return;
	    }
	}
    }
    // fall through intended
    default:
        ToolBarLineInfo line;
        line.append(newinfo);
        tb_layout_info.append(line);
	line[0].linebreak = false;
        break;
    }
}

void QMainWindowLayout::removeToolBarInfo(QToolBar *toolbar)
{
    for (int k = 0; k < tb_layout_info.size(); ++k) {
	for (int i = 0; i < tb_layout_info.at(k).size(); ++i) {
	    const ToolBarLayoutInfo &info = tb_layout_info.at(k).at(i);
	    if (info.item->widget() == toolbar) {
		delete info.item;
		tb_layout_info[k].removeAt(i);
		if (tb_layout_info[k].size() == 0)
		    tb_layout_info.removeAt(k--);
		break;
	    }
	}
    }
}
