#include "q4dockwindowlayout_p.h"

#include "q4mainwindowlayout_p.h"
#include "q4dockwindow.h"
#include "q4dockwindowseparator_p.h"

#include <qapplication.h>
#include <qdebug.h>
#include <qevent.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qvector.h>

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


static inline bool canExpand(Qt::Orientation o, const QSizePolicy &sp)
{
    QSizePolicy::ExpandData exp = sp.expanding();
    return (o == Qt::Horizontal ?
	    (exp & QSizePolicy::Horizontally) != 0 :
	    (exp & QSizePolicy::Vertically  ) != 0);
}
static inline bool canGrow(Qt::Orientation o, const QSizePolicy &sp)
{ return (o == Qt::Horizontal ? sp.mayGrowHorizontally() : sp.mayGrowVertically()); }



class Q4DockWindowSeparator;


class Q4DockWindowLayoutItem : public QWidgetItem
{
public:
    inline Q4DockWindowLayoutItem(QWidget *w, const QRect &r)
	: QWidgetItem(w), rect(r) { }

    inline QSize sizeHint() const
    { return rect.size(); }
    inline void setGeometry( const QRect &r)
    { rect = r; }
    inline QRect geometry() const
    { return rect; }

    QWidget *widget;
    QRect rect;
};

Q4DockWindowLayout::Q4DockWindowLayout(QWidget *widget, Orientation o)
    : QLayout(widget), orientation(o), save_layout_info(0),
      relayout_type(QInternal::RelayoutNormal)
{ }

Q4DockWindowLayout::Q4DockWindowLayout(QLayout *layout, Orientation o)
    : QLayout(layout), orientation(o), save_layout_info(0),
      relayout_type(QInternal::RelayoutNormal)
{ }

QLayoutItem *Q4DockWindowLayout::find(QWidget *widget)
{
    for (int i = 0; i < layout_info.count(); ++i) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep) continue;
	if (widget == info.item->widget())
	    return info.item;
    }
    return 0;
}

QLayoutItem *Q4DockWindowLayout::itemAt(int index) const
{
    VDEBUG("Q4DockWindowLayout::itemAt: index %d (%d)", index, layout_info.count());

    int x = 0;
    for (int i = 0; i < layout_info.count(); ++i) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep) continue;
	if (x++ == index) {
	    VDEBUG("END, widget:    pos %3d index %3d", i, x);
	    return info.item;
	}
    }

    VDEBUG("END, not found");

    return 0;
}

QLayoutItem *Q4DockWindowLayout::takeAt(int index)
{
    DEBUG("Q4DockWindowLayout::takeAt: index %d", index);

    int x = 0;
    for (int i = 0; i < layout_info.count(); ++i) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep) continue;
	if (x++ == index) {
	    QLayoutItem *layoutitem = info.item;

	    VDEBUG("Q4DockWindowLayout::takeAt: layoutitem '%s'",
		   layoutitem->widget() ? layoutitem->widget()->objectName().latin1() : "dummy");

	    int prev_separator = -1;
	    for (int it = 0; it < layout_info.count(); ++it) {
		const Q4DockWindowLayoutInfo &info = layout_info.at(it);
		if (info.is_sep) {
		    prev_separator = it;
		    continue;
		}
		if (info.item != layoutitem) continue;

		QWidget *widget = info.item->widget();
		VDEBUG("  index %3d pos %4d size %4d %s",
		       it, info.cur_pos, info.cur_size,
		       widget ? widget->objectName().latin1() : "dummy");

		// remove the item
		layout_info.removeAt(it);

		// remove the separator
		if (it == layout_info.count()) {
		    // we removed the last dockwindow, so we need to remove
		    // the separator that was above it
		    --it;
		    if (prev_separator == it)
			prev_separator = -1;
		}

		if (it != -1) {
		    Q4DockWindowLayoutInfo &sep_info = layout_info[it];
		    Q_ASSERT(sep_info.is_sep);

		    if (!save_layout_info) {
			delete sep_info.item->widget();
			delete sep_info.item;
		    }

		    VDEBUG("    removing separator at %d", it);
		    layout_info.removeAt(it);
		}

		break;
	    }

#ifdef LAYOUT_DEBUG_VERBOSE
	    dump();
#endif

	    if (layout_info.isEmpty()) {
                if (relayout_type == QInternal::RelayoutDropped) {
                    // probably splitting...
                } else if (!save_layout_info) {
                    deleteLater();
		} else {
		    QLayout *parentLayout = qt_cast<QLayout *>(parent());
		    if (parentLayout)
			parentLayout->removeItem(this);
		}
	    }

	    VDEBUG("END of remove");

	    return layoutitem;
	}
    }
    return 0;
}

/*! reimp */
void Q4DockWindowLayout::addItem(QLayoutItem *layoutitem)
{
    if (relayout_type == QInternal::RelayoutDropped && layoutitem->layout()) {
        // dropping a nested layout!
        return;
    }

    (void) insert(-1, layoutitem);
    invalidate();
}

/*! reimp */
void Q4DockWindowLayout::setGeometry(const QRect &rect)
{
    VDEBUG("Q4DockWindowLayout::setGeometry: width %4d height %4d", rect.width(), rect.height());

    QLayout::setGeometry(rect);

    if (relayout_type == QInternal::RelayoutNormal) {
        bool first = true;
        for (int i = 0; i < layout_info.count(); ++i) {
            const Q4DockWindowLayoutInfo &info = layout_info.at(i);
            if (info.is_sep) continue;
            if (info.is_dummy) continue;

            const bool empty = info.item->isEmpty();
            if (i > 0)
                layout_info.at(i - 1).item->widget()->setHidden(first || empty);
            if (!empty)
                first = false;
        }
    }

    QVector<QLayoutStruct> a(layout_info.count());
    int x;
    const int separator_extent =
	qApp->style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent);

    for (int pass = 0; pass < 2; ++pass) {
	bool need_second_pass = true;

	VDEBUG("  PASS %d", pass);
	for (x = 0; x < layout_info.count(); ++x) {
	    const Q4DockWindowLayoutInfo &info = layout_info.at(x);

	    QLayoutStruct &ls = a[x];
	    ls.init();
	    ls.empty = info.item->isEmpty();

            if (ls.empty) continue;

	    if (info.is_sep) {
		VDEBUG("    separator");
                ls.sizeHint = ls.minimumSize = ls.maximumSize = separator_extent;
            } else {
		const QSizePolicy &sp =
                    info.item->widget()
                    ? info.item->widget()->sizePolicy()
                    : QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

		bool grow = pass == 0 ?
			    canExpand(orientation, sp) :
			    canGrow(orientation, sp);

		if (info.is_dummy || info.is_dropped) {
		    Q_ASSERT(relayout_type != QInternal::RelayoutNormal);

		    // layout a dummy item
		    ls.minimumSize = pick(orientation, info.item->minimumSize());
		    ls.sizeHint = info.cur_size;
		    ls.maximumSize = grow ?
				     pick(orientation, info.item->maximumSize()) :
				     pick(orientation, info.item->sizeHint());

		    // do not use stretch for dummy/dropped items... we want them in the
		    // exact size we specify
		    ls.stretch = 0;
		} else {
		    ls.minimumSize = pick(orientation, info.item->minimumSize());

		    if (grow) {
			ls.maximumSize = pick(orientation, info.item->maximumSize());
			ls.sizeHint = info.cur_size == -1 ?
				      pick(orientation, info.item->sizeHint()) :
				      info.cur_size;

			if (pass == 0) need_second_pass = false;
		    } else {
			ls.maximumSize = pick(orientation, info.item->sizeHint());
			ls.sizeHint = info.cur_size == -1 ? ls.minimumSize : info.cur_size;
		    }

		    // use stretch that is the same as the size hint... this way widgets
		    // will grow proportinally to their existing size, i.e. big widgets
		    // grow "faster" than smaller widgets.
		    ls.stretch = ls.sizeHint;
		}

		// sanity checks
		ls.sizeHint = qMax(ls.sizeHint, ls.minimumSize);
		ls.minimumSize = qMin(ls.minimumSize, ls.maximumSize);

		VDEBUG("    dockwindow cur %4d min %4d max %4d, hint %4d stretch %4d",
		       info.cur_size, ls.minimumSize, ls.maximumSize, ls.sizeHint, ls.stretch);
	    }
	}

	if (!need_second_pass) break;
    }

    qGeomCalc(a, 0, a.count(), 0, pick(orientation, rect.size()), 0);

    // DO IT
    VDEBUG("  final placement:");
    for (int i = 0; i < a.count(); ++i) {
	const QLayoutStruct &ls = a.at(i);
        if (ls.empty) continue;

	Q4DockWindowLayoutInfo &info = layout_info[i];

	if (info.is_sep) {
	    VDEBUG("    separator  cur %4d", ls.size);
	} else {
	    VDEBUG("    dockwindow cur %4d min %4d max %4d pos %4d",
		   ls.size, ls.minimumSize, ls.maximumSize, ls.pos);
	}


	if (relayout_type == QInternal::RelayoutDragging && !info.is_dummy) {
	    // we are testing a layout, so don't actually change
	    // anything... but make sure that we update all the dummy
	    // items with the correct geometry
	    continue;
	}

	info.cur_pos = ls.pos;
	info.cur_size = ls.size;
	info.min_size = ls.minimumSize;
	info.max_size = ls.maximumSize;

	info.item->setGeometry(((orientation == Horizontal) ?
                                QRect(rect.x() + ls.pos, rect.y(), ls.size, rect.height()) :
                                QRect(rect.x(), rect.y() + ls.pos, rect.width(), ls.size)));
    }

    VDEBUG("END");
}

/*! reimp */
QSize Q4DockWindowLayout::minimumSize() const
{
    VDEBUG("Q4DockWindow::minimumSize");

    int size = 0, perp = 0;
    const int sep_extent =
	QApplication::style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent);

    for (int it = 0; it < layout_info.count(); ++it) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(it);
	int s, p;
	if (info.is_sep) {
            s = p = (!info.is_dummy && info.item->widget()->isHidden()) ? 0 : sep_extent;
	} else {
	    QSize sz = info.item->minimumSize();
	    s = pick(orientation, sz);
	    p = pick_perp(orientation, sz);
	}

	VDEBUG("  size %d perp %d", s, p);
	size += s;
	perp = qMax(perp, p);
    }

    VDEBUG("END: size %4d perp %4d", size, perp);

    return ((orientation == Horizontal ? QSize(size, perp) : QSize(perp, size)) +
	    layout_info.count() * QSize(spacing(), spacing()));
}

/*! reimp */
QSize Q4DockWindowLayout::sizeHint() const
{
    VDEBUG("Q4DockWindow::sizeHint");

    int size = 0, perp = 0;
    const int sep_extent =
	QApplication::style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent);

    for (int it = 0; it < layout_info.count(); ++it) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(it);
	int s, p;
	if (info.is_sep) {
            s = p = (!info.is_dummy && info.item->widget()->isHidden()) ? 0 : sep_extent;
	} else {
	    QSize sz = info.item->sizeHint();
	    s = pick(orientation, sz);
	    p = pick_perp(orientation, sz);
	}

	VDEBUG("  size %d perp %d", s, p);
	size += s;
	perp = qMax(perp, p);
    }

    VDEBUG("END: size %4d perp %4d", size, perp);

    return ((orientation == Horizontal ? QSize(size, perp) : QSize(perp, size)) +
	    layout_info.count() * QSize(spacing(), spacing()));
}

void Q4DockWindowLayout::invalidate()
{
    if (relayout_type != QInternal::RelayoutDragging)
        QLayout::invalidate();
}

bool Q4DockWindowLayout::isEmpty() const
{
    for (int i = 0; i < layout_info.count(); ++i) {
        const Q4DockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep) continue;

        if (!info.item->isEmpty())
            return false;
    }
    return true;
}

void Q4DockWindowLayout::setOrientation(Qt::Orientation o)
{
    orientation = o;
    invalidate();
}

/*!
 */
Q4DockWindowLayoutInfo &Q4DockWindowLayout::insert(int index, QLayoutItem *layoutitem, bool dummy)
{
    DEBUG("Q4DockWindowLayout::insert: index %d, layoutitem '%s'",
          index < 0 ? layout_info.count() : index,
          ((layoutitem->widget() || layoutitem->layout()) && !dummy
           ? (layoutitem->layout()
              ? layoutitem->layout()->objectName().latin1()
              : layoutitem->widget()->objectName().latin1())
           : "dummy"));

    bool append = index < 0;
    if (!append) {
	int it = 0;
	int idx = 0;

	// skip to the specified index
	while (it < layout_info.count()) {
	    if (idx == index) {
		VDEBUG("found index %d at %d", index, it);
		break;
	    }

	    if (it + 1 == layout_info.count()) {
		++idx;
		++it; // ran off the end, force append
		break;
	    }

	    ++idx;
	    it += 2;
	}

	if (it == layout_info.count()) {
	    append = true;
	} else {
	    VDEBUG("inserting at %d (%d of %d)", it, idx, index);

	    // insert the dockwindow
	    VDEBUG("    inserting dockwindow at %d", it);
	    Q4DockWindowLayoutInfo dockwindow_info(layoutitem);
	    dockwindow_info.is_dummy = dummy;
	    int save_it = it;
	    layout_info.insert(it++, dockwindow_info);

	    // insert a separator
	    QLayoutItem *sep_item = 0;
	    if (!dummy) {
		Q4DockWindowLayoutInfo &next_info = layout_info[it];
                Q_ASSERT(!next_info.is_sep);

		Q4DockWindowSeparator *sep = new Q4DockWindowSeparator(this, parentWidget());
		if (parentWidget()->isVisible())
		    QTimer::singleShot(0, sep, SLOT(show()));
		sep_item = new QWidgetItem(sep);
	    } else {
		const int sep_extent =
		    QApplication::style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
		sep_item = new QSpacerItem(sep_extent, sep_extent,
					   QSizePolicy::Fixed, QSizePolicy::Fixed);
	    }

	    Q4DockWindowLayoutInfo sep_info(sep_item);
	    sep_info.is_sep = 1;
            sep_info.is_dummy = dummy;
	    VDEBUG("    inserting separator at %d", it);
	    layout_info.insert(it++, sep_info);

	    return layout_info[save_it];
	}
    }

    Q_ASSERT(append == true);

    // append the dockwindow
    if (!layout_info.isEmpty()) {
	Q4DockWindowLayoutInfo &info = layout_info.last();
	Q_ASSERT(!info.is_sep);

	// insert a separator before the dockwindow
	QLayoutItem *sep_item = 0;
	if (!dummy) {
	    Q4DockWindowSeparator *sep = new Q4DockWindowSeparator(this, parentWidget());
	    if (parentWidget()->isVisible())
		QTimer::singleShot(0, sep, SLOT(show()));
	    sep_item = new QWidgetItem(sep);
	} else {
	    const int sep_extent =
		QApplication::style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
	    sep_item = new QSpacerItem(sep_extent, sep_extent,
				       QSizePolicy::Fixed, QSizePolicy::Fixed);
	}

	Q4DockWindowLayoutInfo sep_info(sep_item);
	sep_info.is_sep = 1;
        sep_info.is_dummy = dummy;
	layout_info.append(sep_info);
    }

    Q4DockWindowLayoutInfo dockwindow_info(layoutitem);
    dockwindow_info.is_dummy = dummy;
    layout_info.append(dockwindow_info);

    return layout_info.last();
}

void Q4DockWindowLayout::dump()
{
    DEBUG("Q4DockWindowLayout::dump");
    for (int i = 0; i < layout_info.count(); ++i) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(i);

	if (info.is_sep) {
	    DEBUG("  index %3d pos %4d size %4d SEPARATOR", i, info.cur_pos, info.cur_size);
	} else if (info.item->layout()) {
            DEBUG("  index %3d pos %4d size %4d %s", i, info.cur_pos, info.cur_size,
		  info.item->layout() ? info.item->layout()->objectName().latin1() : "dummy");
            Q4DockWindowLayout *l = qt_cast<Q4DockWindowLayout *>(info.item->layout());
            Q_ASSERT(l != 0);
            l->dump();
        } else {
            DEBUG("  index %3d pos %4d size %4d %s", i, info.cur_pos, info.cur_size,
		  info.item->widget() ? info.item->widget()->objectName().latin1() : "dummy");
	}
    }
    DEBUG("END of dump");
}

void Q4DockWindowLayout::saveLayoutInfo()
{
    Q_ASSERT(save_layout_info == 0);
    save_layout_info = new QList<Q4DockWindowLayoutInfo>(layout_info);

    for (int i = 0; i < layout_info.count(); ++i) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep) continue;
        if (!info.item->layout()) continue;

        Q4DockWindowLayout *l = qt_cast<Q4DockWindowLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        l->saveLayoutInfo();
    }
}

void Q4DockWindowLayout::resetLayoutInfo()
{
    Q_ASSERT(save_layout_info != 0);
    layout_info = *save_layout_info;

    for (int i = 0; i < layout_info.count(); ++i) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep) continue;
        if (!info.item->layout()) continue;

        Q4DockWindowLayout *l = qt_cast<Q4DockWindowLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        l->resetLayoutInfo();
    }
}

void Q4DockWindowLayout::discardLayoutInfo()
{
    Q_ASSERT(save_layout_info != 0);
    delete save_layout_info;
    save_layout_info = 0;

    for (int i = 0; i < layout_info.count(); ++i) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep) continue;
        if (!info.item->layout()) continue;

        Q4DockWindowLayout *l = qt_cast<Q4DockWindowLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        l->discardLayoutInfo();
    }
}

QPoint Q4DockWindowLayout::constrain(Q4DockWindowSeparator *sep, int delta)
{
    VDEBUG("Q4DockWindowLayout::constrain: delta %4d", delta);
    QList<Q4DockWindowLayoutInfo> local_list;

    for (int pass = 0; pass < 2; ++pass) {
	VDEBUG("  PASS %d", pass);
	/*
	  During pass 1, we compute the feedback constraint.  During
	  pass 2, we update layout_info using the calculated
	  constraint.
	*/

	// find 'sep'
	local_list = save_layout_info ? *save_layout_info : layout_info;
	QListMutableIterator<Q4DockWindowLayoutInfo> f_it(local_list), b_it(local_list);
	while (f_it.hasNext()) {
	    const Q4DockWindowLayoutInfo &info = f_it.peekNext();
	    if (info.is_sep && qt_cast<Q4DockWindowSeparator*>(info.item->widget()) == sep) break;
	    (void)f_it.next();
	    (void)b_it.next();
	}
	// at this point, the iterator is just before 'sep'

	// get info for 'sep->prev' and move to just after sep->prev
	Q4DockWindowLayoutInfo &info1 = b_it.previous(); // move to just after previous separator

	(void)f_it.next(); // move to before sep->next

	 // get info for 'sep->next' and move to just before next separator
	Q4DockWindowLayoutInfo &info2 = f_it.next();

	// subtract delta to the current size of sep->next
	int x = info2.cur_size;
	info2.cur_size -= delta;

	// constrain the new size according to our min/max size
	info2.cur_size = qMax(info2.cur_size, info2.min_size);
	info2.cur_size = qMin(info2.cur_size, info2.max_size);
	int delta2 = x - info2.cur_size;

	VDEBUG("next: new %4d old %4d", info2.cur_size, x);

	if (delta2 != delta) {
	    // distribute space to widgets below if possible
	    int remain = delta - delta2;

	    VDEBUG("remaining below: %d", remain);

	    if (f_it.hasNext()) {
		while (remain != 0) {
		    (void)f_it.next(); // skip separator

		    Q4DockWindowLayoutInfo &f_info = f_it.next();

		    // subtract delta to the current size
		    x = f_info.cur_size;
		    f_info.cur_size -= remain;

		    // constrain the new size according to our min/max size
		    f_info.cur_size = qMax(f_info.cur_size, f_info.min_size);
		    f_info.cur_size = qMin(f_info.cur_size, f_info.max_size);
		    remain -= x - f_info.cur_size;

		    VDEBUG("  done, new %4d old %4d remaining %d", f_info.cur_size, x, remain);

		    if (!f_it.hasNext()) break; // at the end
		}
	    }

	    // constrain delta to the absolute minimum of all windows below 'sep'
	    delta -= remain;
	}

	// add delta from current size of sep->next
	x = info1.cur_size;
	info1.cur_size += delta;

	// constrain the delta according to our min/max size
	info1.cur_size = qMax(info1.cur_size, info1.min_size);
	info1.cur_size = qMin(info1.cur_size, info1.max_size);
	int delta1 = info1.cur_size - x;

	VDEBUG("prev: new %4d old %4d", info1.cur_size, x);

	if (delta1 != delta) {
	    // distribute space to widgets above if possible
	    int remain = delta - delta1;

	    VDEBUG("remaining above: %d", remain);

	    if (b_it.hasPrevious()) {
		while (remain != 0) {
		    // (void)b_it.prev(); // skip separator

		    Q4DockWindowLayoutInfo &b_info = b_it.previous();

		    // add delta from current size of sep->next
		    x = b_info.cur_size;
		    b_info.cur_size += remain;

		    // constrain the delta according to our min/max size
		    b_info.cur_size = qMax(b_info.cur_size, b_info.min_size);
		    b_info.cur_size = qMin(b_info.cur_size, b_info.max_size);
		    remain -= b_info.cur_size - x;

		    VDEBUG("  done, new %4d old %4d remaining %d", b_info.cur_size, x, remain);

		    if (!b_it.hasPrevious()) break; // at the beginning
		}
	    }

	    // constrain delta to the absolute minimum of all windows above 'sep'
	    delta -= remain;
	}

	VDEBUG("  end of pass %d, delta %4d", pass, delta);
    }

    // save the calculated
    layout_info = local_list;

    VDEBUG("END");

    return orientation == Horizontal ? QPoint(delta, 0) : QPoint(0, delta);
}

void Q4DockWindowLayout::relayout(QInternal::RelayoutType type)
{
    QInternal::RelayoutType save_type = relayout_type;
    relayout_type = type;
    setGeometry(geometry());
    relayout_type = save_type;
}

/*!
 */
Q4DockWindowLayout::Location Q4DockWindowLayout::locate(const QPoint &mouse) const
{
    // figure out where the dockwindow goes in the layout
    const QPoint p = parentWidget()->mapFromGlobal(mouse) - geometry().topLeft();
    const int pos = pick(orientation, p);
    const bool horizontal = orientation == Horizontal;

    DEBUG() << "  locate: mouse at" << p;

    Location ret;
    for (int i = 0; i < layout_info.count(); ++i) {
        const Q4DockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep) continue;

        if (pos < (info.cur_pos + info.cur_size - 1)) {
            const QRect current =
                (horizontal
                 ? QRect(info.cur_pos, 0, info.cur_size, geometry().height())
                 : QRect(0, info.cur_pos, geometry().width(), info.cur_size));
            const QPoint p2 =
                current.topLeft() + QPoint(current.width() / 2, current.height() / 2);
            const int dx = QABS(p.x() - p2.x()),
                      dy = QABS(p.y() - p2.y());

            ret.index = i;
            ret.area = ((dx > dy)
                        ? ((p.x() < p2.x()) ? DockWindowAreaLeft : DockWindowAreaRight)
                        : ((p.y() < p2.y()) ? DockWindowAreaTop : DockWindowAreaBottom));
            DEBUG() << "  result: index" << ret.index << "area" << ret.area;
            return ret;
        }
    }

    ret.index = layout_info.count() - 1;
    ret.area = (horizontal ? DockWindowAreaRight : DockWindowAreaBottom);
    DEBUG() << "  result: index" << ret.index << "area" << ret.area << "(off-end)";
    return ret;
}

static QRect trySplit(Qt::Orientation orientation,
                      Qt::DockWindowArea area,
                      const QRect &r,
                      const QSize &sz1,
                      const QSize &sz2)
{
    QRect ret;

    switch (orientation) {
    case Qt::Horizontal:
        if (area != Qt::DockWindowAreaTop && area != Qt::DockWindowAreaBottom) {
            DEBUG() << "    wrong place" << area <<"to split";
            break;
        }

        if (!r.contains(QRect(r.x(),
                              r.y(),
                              qMax(sz1.width(), sz2.width()),
                              sz1.height() + sz2.height()))) {
            DEBUG() << "    does not fit";
            break;
        }

        if (area == Qt::DockWindowAreaTop)
            ret.setRect(r.x(), r.y(), r.width(), r.height() / 2);
        else
            ret.setRect(r.x(), r.y() + r.height() / 2, r.width(), r.height() / 2);

        break;

    case Qt::Vertical:
        if (area != Qt::DockWindowAreaLeft && area != Qt::DockWindowAreaRight)
            break;

        if (!r.contains(QRect(r.x(),
                              r.y(),
                              sz1.width() + sz2.width(),
                              qMax(sz1.height(), sz2.height()))))
            break;

        if (area == Qt::DockWindowAreaLeft)
            ret.setRect(r.x(), r.y(), r.width() / 2, r.height());
        else
            ret.setRect(r.x() + r.width() / 2, r.y(), r.width() / 2, r.height());

        break;

    default:
        Q_ASSERT(false);
    }

    if (ret.isValid())
        ret.setSize(ret.size().expandedTo(sz1));
    return ret;
}

/*!
 */
QRect Q4DockWindowLayout::place(Q4DockWindow *dockwindow, const QRect &r, const QPoint &mouse)
{
    DEBUG("Q4DockWindowLayout::place");

    QRect target;
    Location location = locate(mouse);

    {
        const Q4DockWindowLayoutInfo &info = layout_info.at(location.index);

        if (info.item->layout()) {
            // forward the place to the nested layout
            Q4DockWindowLayout *l = qt_cast<Q4DockWindowLayout *>(info.item->layout());
            Q_ASSERT(l != 0);
            DEBUG("  forwarding...");
            target = l->place(dockwindow, r, mouse);
            DEBUG("END of Q4MainWindowLayout::place (forwarded)");
            return target;
        } else if (dockwindow == info.item->widget()) {
            if (orientation == Qt::Horizontal) {
                target.setRect(geometry().x() + info.cur_pos,
                               geometry().y(),
                               info.cur_size,
                               geometry().height());
            } else {
                target.setRect(geometry().x(),
                               geometry().y() + info.cur_pos,
                               geometry().width(),
                               info.cur_size);
            }
            DEBUG() << "  placed back at original position" << target;
        } else {
            const QSize sz1 = dockwindow->minimumSizeHint(),
                        sz2 = info.item->minimumSize();
            const QRect r = ((orientation == Qt::Horizontal)
                             ? QRect(geometry().x() + info.cur_pos,
                                     geometry().y(),
                                     info.cur_size,
                                     geometry().height())
                             : QRect(geometry().x(),
                                     geometry().y() + info.cur_pos,
                                     geometry().width(),
                                     info.cur_size));
            DEBUG() << "  splitting" << sz1 << sz2 << "into" << r;
            target = ::trySplit(orientation, location.area, r, sz1, sz2);
            if (!target.isValid()) {
                DEBUG() << "  could not split";
                const QPoint p = parentWidget()->mapFromGlobal(mouse) - geometry().topLeft();
                const int pos = pick(orientation, p);
                const bool horizontal = orientation == Horizontal;
                if (pos > (info.cur_pos + (info.cur_size / 2) - 1)) {
                    location.area = horizontal
                                    ? Qt::DockWindowAreaRight
                                    : Qt::DockWindowAreaBottom;
                } else {
                    location.area = horizontal
                                    ? Qt::DockWindowAreaLeft
                                    : Qt::DockWindowAreaTop;
                }
            }
        }
    }

    if (target.isValid()) {
        DEBUG("END of place (target already found %d,%d %dx%d)",
              target.x(), target.y(), target.width(), target.height());

        target.moveTopLeft(parentWidget()->mapToGlobal(target.topLeft()));
        return target;
    }

    Q4MainWindowLayout *layout =
        qt_cast<Q4MainWindowLayout *>(parentWidget()->layout());
    Q_ASSERT(layout != 0);
    layout->removeRecursive(dockwindow);

    /*
      Now that we know where to put the dockwindow, we need to
      determine the screen position and provide this feedback to the
      user.  We do this by inserting dummy layout items into the
      layout and performing a 'test' relayout to determine where
      everything will be placed.
    */

    const QSize cur = parentWidget()->size();

    // add dummy layout item
    int index = location.index / 2; // :)
    if (location.area == Qt::DockWindowAreaRight || location.area == Qt::DockWindowAreaBottom)
        ++index;
    Q4DockWindowLayoutItem layoutitem(dockwindow, r);
    Q4DockWindowLayoutInfo &info = insert(index, &layoutitem, true);
    const QSize new_min = minimumSize();

    info.cur_size = pick(orientation, r.size());

    relayout(QInternal::RelayoutDragging);

    const bool forbid = cur.width() < new_min.width() || cur.height() < new_min.height();

    // map the layout item's geometry to screen coordinates
    if (!forbid) {
        target = layoutitem.geometry();
        DEBUG("    target now %d,%d %dx%d",
              target.x(), target.y(), target.width(), target.height());
        target.moveTopLeft(parentWidget()->mapToGlobal(target.topLeft()));
    } else {
        DEBUG("    FORBIDDEN: cur %dx%d new_min %dx%d",
              cur.width(), cur.height(),
              new_min.width(), new_min.height());
    }

    DEBUG("END of place");

    return target;
}

/*!
 */
void Q4DockWindowLayout::drop(Q4DockWindow *dockwindow, const QRect &r, const QPoint &mouse)
{
    DEBUG("Q4DockWindowLayout::drop");

    Location location = locate(mouse);

    {
        const Q4DockWindowLayoutInfo &info = layout_info.at(location.index);

        if (info.item->layout()) {
            // forward the drop to the nested layout
            Q4DockWindowLayout *l = qt_cast<Q4DockWindowLayout *>(info.item->layout());
            Q_ASSERT(l != 0);
            DEBUG("  forwarding...");
            l->drop(dockwindow, r, mouse);
            DEBUG("END of Q4MainWindowLayout::drop (forwarded)");
            return;
        } else if (dockwindow == info.item->widget()) {
            // placed back at original position
            DEBUG("END of drop (shortcut - dropped at original position)");
            return;
        }

        Q4MainWindowLayout *layout =
            qt_cast<Q4MainWindowLayout *>(parentWidget()->layout());
        Q_ASSERT(layout != 0);
        layout->removeRecursive(dockwindow);

        const QSize sz1 = dockwindow->minimumSizeHint(),
                    sz2 = info.item->minimumSize();
        const QRect r = ((orientation == Qt::Horizontal)
                         ? QRect(geometry().x() + info.cur_pos,
                                 geometry().y(),
                                 info.cur_size,
                                 geometry().height())
                         : QRect(geometry().x(),
                                 geometry().y() + info.cur_pos,
                                 geometry().width(),
                                 info.cur_size));
        DEBUG() << "  splitting" << sz1 << sz2 << "into" << r;
        QRect target = ::trySplit(orientation, location.area, r, sz1, sz2);
        if (target.isValid()) {
            split(qt_cast<Q4DockWindow *>(info.item->widget()), dockwindow, location.area);

            DEBUG("END of drop (nested window dock)");
            return;
        } else {
            const QPoint p = parentWidget()->mapFromGlobal(mouse) - geometry().topLeft();
            const int pos = pick(orientation, p);
            const bool horizontal = orientation == Horizontal;
            if (pos > (info.cur_pos + (info.cur_size / 2) - 1))
                location.area = horizontal ? Qt::DockWindowAreaRight : Qt::DockWindowAreaBottom;
            else
                location.area = horizontal ? Qt::DockWindowAreaLeft  : Qt::DockWindowAreaTop;
        }
    }

    int index = location.index / 2; // :)
    if (location.area == Qt::DockWindowAreaRight || location.area == Qt::DockWindowAreaBottom)
        ++index;
    Q4DockWindowLayoutInfo &info = insert(index, new QWidgetItem(dockwindow));
    info.is_dropped = true;
    info.cur_size = pick(orientation, r.size());

    relayout(QInternal::RelayoutDropped);
    info.is_dropped = false;

    if (dockwindow->isFloated()) {
        // reparent the dockwindow to the new dock
        dockwindow->setFloated(false);
        dockwindow->show();
    }

    DEBUG("END of drop");
}

void Q4DockWindowLayout::extend(Q4DockWindow *dockwindow, Orientation direction)
{
    if (direction == orientation) {
        addWidget(dockwindow);
    } else {
        Q_ASSERT(relayout_type == QInternal::RelayoutNormal);
        relayout_type = QInternal::RelayoutDropped;

        Q4DockWindowLayout *nestedLayout = new Q4DockWindowLayout(this, orientation);
        nestedLayout->setObjectName(objectName() + QLatin1String("_nestedCopy"));

        for (int i = 0; i < layout_info.count(); ++i) {
            const Q4DockWindowLayoutInfo &info = layout_info.at(i);
            if (info.is_sep) {
                delete info.item->widget();
                delete info.item;
            } else {
                nestedLayout->addItem(info.item);
            }
        }

        relayout_type = QInternal::RelayoutNormal;

        layout_info.clear();
        setOrientation(direction);

        addItem(nestedLayout);
        addWidget(dockwindow);
    }
}

void Q4DockWindowLayout::split(Q4DockWindow *dockwindow, Orientation direction)
{
    int last = layout_info.count() - 1;
    if (last < 0) {
        addWidget(dockwindow);
    } else {
        const Q4DockWindowLayoutInfo &info =layout_info.at(last);
        if (info.item->widget() && direction == orientation) {
            addWidget(dockwindow);
        } else {
            if (info.item->widget()) {
                split(qt_cast<Q4DockWindow *>(info.item->widget()), dockwindow,
                      (direction == Horizontal
                       ? Qt::DockWindowAreaRight
                       : Qt::DockWindowAreaBottom));
            } else {
                Q4DockWindowLayout *nestedLayout =
                    qt_cast<Q4DockWindowLayout *>(info.item->layout());
                nestedLayout->setObjectName(objectName() + QLatin1String("_nestedLayout"));
                Q_ASSERT(nestedLayout != 0);
                nestedLayout->split(dockwindow, direction);
            }
        }
    }
}

void Q4DockWindowLayout::split(Q4DockWindow *existing, Q4DockWindow *with, Qt::DockWindowArea area)
{
    int which = -1;
    for (int i = 0; which == -1 && i < layout_info.count(); ++i) {
	const Q4DockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep) continue;
	if (existing == info.item->widget())
            which = i;
    }
    Q_ASSERT(which != -1);
    const Q4DockWindowLayoutInfo &info = layout_info.at(which);

    Q_ASSERT(relayout_type == QInternal::RelayoutNormal);
    relayout_type = QInternal::RelayoutDropped;

    int save_size = info.cur_size;
    removeWidget(existing);
    // note: info is invalid from now on

    // create a nested window dock in place of the current widget
    Q4DockWindowLayout *nestedLayout =
        new Q4DockWindowLayout(this, orientation == Horizontal ? Vertical : Horizontal);
    nestedLayout->setObjectName(objectName() + "_nestedLayout");
    insert(which / 2, nestedLayout).cur_size = save_size;
    invalidate();

    switch (area) {
    case Qt::DockWindowAreaTop:
    case Qt::DockWindowAreaLeft:
        nestedLayout->addWidget(with);
        nestedLayout->addWidget(existing);
        break;

    case Qt::DockWindowAreaBottom:
    case Qt::DockWindowAreaRight:
        nestedLayout->addWidget(existing);
        nestedLayout->addWidget(with);
        break;

    default:
        Q_ASSERT(false);
    }

    relayout_type = QInternal::RelayoutNormal;
}
