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

#include "qdockwindowlayout_p.h"
#include "qdockwindow.h"

#include <qapplication.h>
#include <qdebug.h>
#include <qevent.h>
#include <qstyle.h>
#include <qvector.h>

#include <private/qlayoutengine_p.h>

#include "qdockwindow_p.h"
#include "qdockwindowseparator_p.h"
#include "qmainwindowlayout_p.h"

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



class QDockWindowSeparator;


class QDockWindowLayoutItem : public QWidgetItem
{
public:
    inline QDockWindowLayoutItem(QWidget *w, const QRect &r)
	: QWidgetItem(w), rect(r) { }

    inline QSize sizeHint() const
    { return rect.size(); }
    inline void setGeometry( const QRect &r)
    { rect = r; }
    inline QRect geometry() const
    { return rect; }
    inline bool isEmpty() const
    { return false; }

    QWidget *widget;
    QRect rect;
};

QDockWindowLayout::QDockWindowLayout(Qt::DockWindowArea a, Qt::Orientation o)
    : QLayout(static_cast<QWidget*>(0)), area(a), orientation(o), save_layout_info(0),
      relayout_type(QInternal::RelayoutNormal)
{ connect(this, SIGNAL(emptied()), SLOT(maybeDelete()), Qt::QueuedConnection); }

QDockWindowLayout::~QDockWindowLayout()
{
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            info.item->widget()->deleteLater();
        if (!info.item->layout())
            delete info.item;
    }
}

void QDockWindowLayout::saveState(QDataStream &stream) const
{
    stream << (uchar) Marker;
    stream << (uchar) orientation;
    stream << (layout_info.count() + 1) / 2;
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            continue;
	if (info.item->widget()) {
            const QWidget * const widget = info.item->widget();
            stream << (uchar) WidgetMarker;
            stream << widget->windowTitle();
            stream << (uchar) widget->isShown();
            stream << info.cur_pos;
            stream << info.cur_size;
            stream << info.min_size;
            stream << info.max_size;
        } else if (info.item->layout()) {
            stream << (uchar) Marker;
            stream << info.cur_pos;
            stream << info.cur_size;
            stream << info.min_size;
            stream << info.max_size;
            const QDockWindowLayout * const layout =
                qt_cast<const QDockWindowLayout *>(info.item->layout());
            layout->saveState(stream);
        }
    }
}

bool QDockWindowLayout::restoreState(QDataStream &stream)
{
    uchar marker;
    int size;
    stream >> marker;
    if (marker != Marker)
        return false;

    relayout_type = QInternal::RelayoutDropped;

    uchar o;
    stream >> o;
    orientation = static_cast<Qt::Orientation>(o);
    stream >> size;
    QList<QWidget *> widgets = qFindChildren<QWidget *>(parentWidget());
    for (int i = 0; i < size; ++i) {
        uchar nextMarker;
        stream >> nextMarker;
        switch (nextMarker) {
        case WidgetMarker:
            {
                QString windowTitle;
                stream >> windowTitle;
                uchar shown;
                stream >> shown;

                // find widget
                QWidget *widget = 0;
                for (int t = 0; t < widgets.size(); ++t) {
                    if (widgets.at(t)->windowTitle() == windowTitle) {
                        widget = widgets.at(t);
                        break;
                    }
                }
                if (!widget) {
                    // discard size/position data for unknown widget
                    QDockWindowLayoutInfo info(0);
                    stream >> info.cur_pos;
                    stream >> info.cur_size;
                    stream >> info.min_size;
                    stream >> info.max_size;
                    continue;
                }

                QDockWindowLayoutInfo &info = insert(-1, new QWidgetItem(widget));
                widget->setShown(shown);
                stream >> info.cur_pos;
                stream >> info.cur_size;
                stream >> info.min_size;
                stream >> info.max_size;
                break;
            }

        case Marker:
            {
                QDockWindowLayout *layout = new QDockWindowLayout(area, orientation);
                layout->setParent(this);
                QDockWindowLayoutInfo &info = insert(-1, layout);
                stream >> info.cur_pos;
                stream >> info.cur_size;
                stream >> info.min_size;
                stream >> info.max_size;
                if (!layout->restoreState(stream)) {
                    relayout_type = QInternal::RelayoutNormal;
                    return false;
                }
                break;
            }

        default:
            Q_ASSERT(false);
        }
    }

    relayout_type = QInternal::RelayoutNormal;

    return true;
}

QLayoutItem *QDockWindowLayout::find(QWidget *widget)
{
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            continue;
	if (widget == info.item->widget())
	    return info.item;
    }
    return 0;
}

QLayoutItem *QDockWindowLayout::itemAt(int index) const
{
    VDEBUG("QDockWindowLayout::itemAt: index %d (%d)", index, layout_info.count());

    int x = 0;
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            continue;
	if (x++ == index) {
	    VDEBUG("END, widget:    pos %3d index %3d", i, x);
	    return info.item;
	}
    }

    VDEBUG("END, not found");

    return 0;
}

QLayoutItem *QDockWindowLayout::takeAt(int index)
{
    DEBUG("QDockWindowLayout::takeAt: index %d", index);

    int x = 0;
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            continue;
	if (x++ == index) {
	    QLayoutItem *layoutitem = info.item;

	    VDEBUG("QDockWindowLayout::takeAt: layoutitem '%s'",
                   (layoutitem->widget()
                    ? layoutitem->widget()->objectName().toLatin1().constData()
                    : "dummy"));

	    int prev_separator = -1;
	    for (int it = 0; it < layout_info.count(); ++it) {
		const QDockWindowLayoutInfo &info = layout_info.at(it);
		if (info.is_sep) {
		    prev_separator = it;
		    continue;
		}
		if (info.item != layoutitem)
                    continue;

		QWidget *widget = info.item->widget();
		VDEBUG("  index %3d pos %4d size %4d %s",
		       it, info.cur_pos, info.cur_size,
		       widget ? widget->objectName().toLatin1().constData() : "dummy");

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
		    QDockWindowLayoutInfo &sep_info = layout_info[it];
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
                    emit emptied();
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

/*! \reimp */
void QDockWindowLayout::addItem(QLayoutItem *layoutitem)
{
    if (relayout_type == QInternal::RelayoutDropped && layoutitem->layout()) {
        // dropping a nested layout!
        return;
    }

    (void) insert(-1, layoutitem);
    invalidate();
}

/*! \reimp */
void QDockWindowLayout::setGeometry(const QRect &rect)
{
    VDEBUG("QDockWindowLayout::setGeometry: width %4d height %4d", rect.width(), rect.height());

    QLayout::setGeometry(rect);

    if (relayout_type != QInternal::RelayoutDragging) {
        bool first = true;
        for (int i = 0; i < layout_info.count(); ++i) {
            const QDockWindowLayoutInfo &info = layout_info.at(i);
            if (info.is_sep)
                continue;
            if (info.is_dummy)
                continue;

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
	qApp->style()->pixelMetric(QStyle::PM_DockWindowSeparatorExtent);

    for (int pass = 0; pass < 2; ++pass) {
	bool need_second_pass = true;

	VDEBUG("  PASS %d", pass);
	for (x = 0; x < layout_info.count(); ++x) {
	    const QDockWindowLayoutInfo &info = layout_info.at(x);

	    QLayoutStruct &ls = a[x];
	    ls.init();
	    ls.empty = info.item->isEmpty();

            if (ls.empty)
                continue;

	    if (info.is_sep) {
		VDEBUG("    separator");
                ls.sizeHint = ls.minimumSize = ls.maximumSize = separator_extent;
            } else {
		const QSizePolicy &sp =
                    info.item->widget()
                    ? info.item->widget()->sizePolicy()
                    : QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

		bool grow = pass == 0
                            ? canExpand(orientation, sp)
                            : canGrow(orientation, sp);

		if (info.is_dummy || info.is_dropped) {
		    Q_ASSERT(relayout_type != QInternal::RelayoutNormal);

		    // layout a dummy item
		    ls.minimumSize = pick(orientation, info.item->minimumSize());
		    ls.sizeHint = info.cur_size;
		    ls.maximumSize = grow
                                     ? pick(orientation, info.item->maximumSize())
                                     : pick(orientation, info.item->sizeHint());

		    // do not use stretch for dummy/dropped items... we want them in the
		    // exact size we specify
		    ls.stretch = 0;
		} else {
		    ls.minimumSize = pick(orientation, info.item->minimumSize());

		    if (grow) {
			ls.maximumSize = pick(orientation, info.item->maximumSize());
			ls.sizeHint = info.cur_size == -1
                                      ? pick(orientation, info.item->sizeHint())
                                      : info.cur_size;

			if (pass == 0)
                            need_second_pass = false;
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

	if (!need_second_pass)
            break;
    }

    qGeomCalc(a, 0, a.count(), 0, pick(orientation, rect.size()), 0);

    // DO IT
    VDEBUG("  final placement:");
    for (int i = 0; i < a.count(); ++i) {
	const QLayoutStruct &ls = a.at(i);
        if (ls.empty)
            continue;

	QDockWindowLayoutInfo &info = layout_info[i];

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

	info.item->setGeometry(((orientation == Qt::Horizontal) ?
                                QRect(rect.x() + ls.pos, rect.y(), ls.size, rect.height()) :
                                QRect(rect.x(), rect.y() + ls.pos, rect.width(), ls.size)));
    }

    VDEBUG("END");
}

/*! \reimp */
QSize QDockWindowLayout::minimumSize() const
{
    VDEBUG("QDockWindow::minimumSize");

    int size = 0, perp = 0;
    const int sep_extent =
	QApplication::style()->pixelMetric(QStyle::PM_DockWindowSeparatorExtent);

    for (int it = 0; it < layout_info.count(); ++it) {
	const QDockWindowLayoutInfo &info = layout_info.at(it);
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

    return (orientation == Qt::Horizontal) ? QSize(size, perp) : QSize(perp, size);
}

/*! \reimp */
QSize QDockWindowLayout::sizeHint() const
{
    VDEBUG("QDockWindow::sizeHint");

    int size = 0, perp = 0;
    const int sep_extent =
	QApplication::style()->pixelMetric(QStyle::PM_DockWindowSeparatorExtent);

    for (int it = 0; it < layout_info.count(); ++it) {
	const QDockWindowLayoutInfo &info = layout_info.at(it);
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

    return (orientation == Qt::Horizontal) ? QSize(size, perp) : QSize(perp, size);
}

void QDockWindowLayout::invalidate()
{
    if (relayout_type != QInternal::RelayoutDragging)
        QLayout::invalidate();
}

bool QDockWindowLayout::isEmpty() const
{
    for (int i = 0; i < layout_info.count(); ++i) {
        const QDockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (!info.item->isEmpty())
            return false;
    }
    return true;
}

void QDockWindowLayout::setOrientation(Qt::Orientation o)
{
    orientation = o;
    invalidate();
}

/*!
 */
QDockWindowLayoutInfo &QDockWindowLayout::insert(int index, QLayoutItem *layoutitem, bool dummy)
{
    DEBUG("QDockWindowLayout::insert: index %d, layoutitem '%s'",
          index < 0 ? layout_info.count() : index,
          ((layoutitem->widget() || layoutitem->layout()) && !dummy
           ? (layoutitem->layout()
              ? layoutitem->layout()->objectName().toLatin1().constData()
              : layoutitem->widget()->objectName().toLatin1().constData())
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
	    QDockWindowLayoutInfo dockwindow_info(layoutitem);
	    dockwindow_info.is_dummy = dummy;
	    int save_it = it;
	    layout_info.insert(it++, dockwindow_info);

	    // insert a separator
	    QLayoutItem *sep_item = 0;
	    if (!dummy) {
                Q_ASSERT(!layout_info[it].is_sep);

		QDockWindowSeparator *sep = new QDockWindowSeparator(this, parentWidget());
		sep_item = new QWidgetItem(sep);
	    } else {
		const int sep_extent =
		    QApplication::style()->pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
		sep_item = new QSpacerItem(sep_extent, sep_extent,
					   QSizePolicy::Fixed, QSizePolicy::Fixed);
	    }

	    QDockWindowLayoutInfo sep_info(sep_item);
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
	Q_ASSERT(!layout_info.last().is_sep);

	// insert a separator before the dockwindow
	QLayoutItem *sep_item = 0;
	if (!dummy) {
	    QDockWindowSeparator *sep = new QDockWindowSeparator(this, parentWidget());
	    sep_item = new QWidgetItem(sep);
	} else {
	    const int sep_extent =
		QApplication::style()->pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
	    sep_item = new QSpacerItem(sep_extent, sep_extent,
				       QSizePolicy::Fixed, QSizePolicy::Fixed);
	}

	QDockWindowLayoutInfo sep_info(sep_item);
	sep_info.is_sep = 1;
        sep_info.is_dummy = dummy;
	layout_info.append(sep_info);
    }

    QDockWindowLayoutInfo dockwindow_info(layoutitem);
    dockwindow_info.is_dummy = dummy;
    layout_info.append(dockwindow_info);

    return layout_info.last();
}

void QDockWindowLayout::dump()
{
    DEBUG("QDockWindowLayout::dump");
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);

	if (info.is_sep) {
	    DEBUG("  index %3d pos %4d size %4d SEPARATOR", i, info.cur_pos, info.cur_size);
	} else if (info.item->layout()) {
            DEBUG("  index %3d pos %4d size %4d %s", i, info.cur_pos, info.cur_size,
		  (info.item->layout()
                   ? info.item->layout()->objectName().toLatin1().constData()
                   : "dummy"));
            QDockWindowLayout *l = qt_cast<QDockWindowLayout *>(info.item->layout());
            Q_ASSERT(l != 0);
            l->dump();
        } else {
            DEBUG("  index %3d pos %4d size %4d %s", i, info.cur_pos, info.cur_size,
		  (info.item->widget()
                   ? info.item->widget()->objectName().toLatin1().constData()
                   : "dummy"));
	}
    }
    DEBUG("END of dump");
}

void QDockWindowLayout::saveLayoutInfo()
{
    Q_ASSERT(save_layout_info == 0);
    save_layout_info = new QList<QDockWindowLayoutInfo>(layout_info);

    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (!info.item->layout())
            continue;

        QDockWindowLayout *l = qt_cast<QDockWindowLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        l->saveLayoutInfo();
    }
}

void QDockWindowLayout::resetLayoutInfo()
{
    Q_ASSERT(save_layout_info != 0);
    layout_info = *save_layout_info;

    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (!info.item->layout())
            continue;

        QDockWindowLayout *l = qt_cast<QDockWindowLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        l->resetLayoutInfo();
    }
}

void QDockWindowLayout::discardLayoutInfo()
{
    Q_ASSERT(save_layout_info != 0);
    delete save_layout_info;
    save_layout_info = 0;

    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (!info.item->layout())
            continue;

        QDockWindowLayout *l = qt_cast<QDockWindowLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        l->discardLayoutInfo();
    }
}

QPoint QDockWindowLayout::constrain(QDockWindowSeparator *sep, int delta)
{
    VDEBUG("QDockWindowLayout::constrain: delta %4d", delta);
    QList<QDockWindowLayoutInfo> local_list;

    for (int pass = 0; pass < 2; ++pass) {
	VDEBUG("  PASS %d", pass);
	/*
	  During pass 1, we compute the feedback constraint.  During
	  pass 2, we update layout_info using the calculated
	  constraint.
	*/

	// find 'sep'
	local_list = save_layout_info ? *save_layout_info : layout_info;
	QMutableListIterator<QDockWindowLayoutInfo> f_it(local_list), b_it(local_list);
	while (f_it.hasNext()) {
	    const QDockWindowLayoutInfo &info = f_it.peekNext();
	    if (info.is_sep && qt_cast<QDockWindowSeparator*>(info.item->widget()) == sep) break;
	    (void)f_it.next();
	    (void)b_it.next();
	}
	// at this point, the iterator is just before 'sep'

	// get info for 'sep->prev' and move to just after sep->prev
	QDockWindowLayoutInfo &info1 = b_it.previous(); // move to just after previous separator

	(void)f_it.next(); // move to before sep->next

	 // get info for 'sep->next' and move to just before next separator
	QDockWindowLayoutInfo &info2 = f_it.next();

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

		    QDockWindowLayoutInfo &f_info = f_it.next();

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

		    QDockWindowLayoutInfo &b_info = b_it.previous();

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

    return orientation == Qt::Horizontal ? QPoint(delta, 0) : QPoint(0, delta);
}

void QDockWindowLayout::relayout(QInternal::RelayoutType type)
{
    QInternal::RelayoutType save_type = relayout_type;
    relayout_type = type;
    setGeometry(geometry());
    relayout_type = save_type;
}

/*!
 */
QDockWindowLayout::Location QDockWindowLayout::locate(const QPoint &mouse) const
{
    // figure out where the dockwindow goes in the layout
    const QPoint p = parentWidget()->mapFromGlobal(mouse) - geometry().topLeft();
    const int pos = pick(orientation, p);
    const bool horizontal = orientation == Qt::Horizontal;

    DEBUG() << "  locate: mouse at" << p;

    Location ret;
    for (int i = 0; i < layout_info.count(); ++i) {
        const QDockWindowLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;

        if (pos < (info.cur_pos + info.cur_size - 1)) {
            const QRect current =
                (horizontal
                 ? QRect(info.cur_pos, 0, info.cur_size, geometry().height())
                 : QRect(0, info.cur_pos, geometry().width(), info.cur_size));
            const QPoint p2 =
                current.topLeft() + QPoint(current.width() / 2, current.height() / 2);
            const int dx = qAbs(p.x() - p2.x()),
                      dy = qAbs(p.y() - p2.y());

            ret.index = i;
            ret.area = ((dx > dy)
                        ? ((p.x() < p2.x()) ? Qt::LeftDockWindowArea : Qt::RightDockWindowArea)
                        : ((p.y() < p2.y()) ? Qt::TopDockWindowArea : Qt::BottomDockWindowArea));
            DEBUG() << "  result: index" << ret.index << "area" << ret.area;
            return ret;
        }
    }

    ret.index = layout_info.count() - 1;
    ret.area = (horizontal ? Qt::RightDockWindowArea : Qt::BottomDockWindowArea);
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
        if (area != Qt::TopDockWindowArea && area != Qt::BottomDockWindowArea) {
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

        if (area == Qt::TopDockWindowArea)
            ret.setRect(r.x(), r.y(), r.width(), r.height() / 2);
        else
            ret.setRect(r.x(), r.y() + r.height() / 2, r.width(), r.height() / 2);

        break;

    case Qt::Vertical:
        if (area != Qt::LeftDockWindowArea && area != Qt::RightDockWindowArea)
            break;

        if (!r.contains(QRect(r.x(),
                              r.y(),
                              sz1.width() + sz2.width(),
                              qMax(sz1.height(), sz2.height()))))
            break;

        if (area == Qt::LeftDockWindowArea)
            ret.setRect(r.x(), r.y(), r.width() / 2, r.height());
        else
            ret.setRect(r.x() + r.width() / 2, r.y(), r.width() / 2, r.height());

        break;

    default:
        Q_ASSERT(false);
    }

    if (!ret.isEmpty())
        ret.setSize(ret.size().expandedTo(sz1));
    return ret;
}

/*!
 */
QRect QDockWindowLayout::place(QDockWindow *dockwindow, const QRect &r, const QPoint &mouse)
{
    DEBUG("QDockWindowLayout::place");

    QRect target;
    Location location = locate(mouse);

    {
        const QDockWindowLayoutInfo &info = layout_info.at(location.index);

        if (info.item->layout()) {
            // forward the place to the nested layout
            QDockWindowLayout *l = qt_cast<QDockWindowLayout *>(info.item->layout());
            Q_ASSERT(l != 0);
            DEBUG("  forwarding...");
            target = l->place(dockwindow, r, mouse);
            DEBUG("END of QDockWindowLayout::place (forwarded)");
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
            if (target.isEmpty()) {
                DEBUG() << "  could not split";
                const QPoint p = parentWidget()->mapFromGlobal(mouse) - geometry().topLeft();
                const int pos = pick(orientation, p);
                const bool horizontal = orientation == Qt::Horizontal;
                if (pos > (info.cur_pos + (info.cur_size / 2) - 1)) {
                    location.area = horizontal
                                    ? Qt::RightDockWindowArea
                                    : Qt::BottomDockWindowArea;
                } else {
                    location.area = horizontal
                                    ? Qt::LeftDockWindowArea
                                    : Qt::TopDockWindowArea;
                }
            }
        }
    }

    if (!target.isEmpty()) {
        DEBUG("END of place (target already found %d,%d %dx%d)",
              target.x(), target.y(), target.width(), target.height());

        target.moveTopLeft(parentWidget()->mapToGlobal(target.topLeft()));
        return target;
    }

    QMainWindowLayout *layout =
        qt_cast<QMainWindowLayout *>(parentWidget()->layout());
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
    if (location.area == Qt::RightDockWindowArea || location.area == Qt::BottomDockWindowArea)
        ++index;
    QDockWindowLayoutItem layoutitem(dockwindow, r);
    QDockWindowLayoutInfo &info = insert(index, &layoutitem, true);
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
void QDockWindowLayout::drop(QDockWindow *dockwindow, const QRect &r, const QPoint &mouse)
{
    DEBUG("QDockWindowLayout::drop");

    Location location = locate(mouse);

    {
        const QDockWindowLayoutInfo &info = layout_info.at(location.index);

        if (info.item->layout()) {
            // forward the drop to the nested layout
            QDockWindowLayout *l = qt_cast<QDockWindowLayout *>(info.item->layout());
            Q_ASSERT(l != 0);
            DEBUG("  forwarding...");
            l->drop(dockwindow, r, mouse);
            DEBUG("END of QDockWindowLayout::drop (forwarded)");
            return;
        } else if (dockwindow == info.item->widget()) {
            // placed back at original position
            DEBUG("END of drop (shortcut - dropped at original position)");
            return;
        }

        QMainWindowLayout *layout =
            qt_cast<QMainWindowLayout *>(parentWidget()->layout());
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
        if (!target.isEmpty()) {
            split(qt_cast<QDockWindow *>(info.item->widget()), dockwindow, location.area);

            DEBUG("END of drop (nested window dock)");
            return;
        } else {
            const QPoint p = parentWidget()->mapFromGlobal(mouse) - geometry().topLeft();
            const int pos = pick(orientation, p);
            const bool horizontal = orientation == Qt::Horizontal;
            if (pos > (info.cur_pos + (info.cur_size / 2) - 1))
                location.area = horizontal ? Qt::RightDockWindowArea : Qt::BottomDockWindowArea;
            else
                location.area = horizontal ? Qt::LeftDockWindowArea  : Qt::TopDockWindowArea;
        }
    }

    int index = location.index / 2; // :)
    if (location.area == Qt::RightDockWindowArea || location.area == Qt::BottomDockWindowArea)
        ++index;
    QDockWindowLayoutInfo &info = insert(index, new QWidgetItem(dockwindow));
    info.is_dropped = true;
    info.cur_size = pick(orientation, r.size());

    relayout(QInternal::RelayoutDropped);
    info.is_dropped = false;

    if (dockwindow->isTopLevel()) {
        // reparent the dock window into the main window
        dockwindow->setTopLevel(false);
        dockwindow->show();
    }

    DEBUG("END of drop");
}

void QDockWindowLayout::extend(QDockWindow *dockwindow, Qt::Orientation direction)
{
    if (direction == orientation) {
        addWidget(dockwindow);
    } else {
        Q_ASSERT(relayout_type == QInternal::RelayoutNormal);
        relayout_type = QInternal::RelayoutDropped;

        QDockWindowLayout *nestedLayout = new QDockWindowLayout(area, orientation);
        nestedLayout->setParent(this);
        nestedLayout->setObjectName(objectName() + QLatin1String("_nestedCopy"));

        for (int i = 0; i < layout_info.count(); ++i) {
            const QDockWindowLayoutInfo &info = layout_info.at(i);
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

void QDockWindowLayout::split(QDockWindow *existing, QDockWindow *with, Qt::DockWindowArea area)
{
    int which = -1;
    for (int i = 0; which == -1 && i < layout_info.count(); ++i) {
	const QDockWindowLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            continue;
	if (existing == info.item->widget())
            which = i;
    }
    Q_ASSERT(which != -1);
    const QDockWindowLayoutInfo &info = layout_info.at(which);

    Q_ASSERT(relayout_type == QInternal::RelayoutNormal);
    relayout_type = QInternal::RelayoutDropped;

    int save_size = info.cur_size;
    removeWidget(existing);
    // note: info is invalid from now on

    // create a nested window dock in place of the current widget
    QDockWindowLayout *nestedLayout =
        new QDockWindowLayout(area, orientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal);
    nestedLayout->setParent(this);
    nestedLayout->setObjectName(objectName() + "_nestedLayout");
    insert(which / 2, nestedLayout).cur_size = save_size;
    invalidate();

    switch (area) {
    case Qt::LeftDockWindowArea:
    case Qt::TopDockWindowArea:
        nestedLayout->addWidget(with);
        nestedLayout->addWidget(existing);
        break;

    case Qt::RightDockWindowArea:
    case Qt::BottomDockWindowArea:
        nestedLayout->addWidget(existing);
        nestedLayout->addWidget(with);
        break;

    default:
        Q_ASSERT(false);
    }

    relayout_type = QInternal::RelayoutNormal;
}

void QDockWindowLayout::maybeDelete()
{
    if (layout_info.isEmpty())
        delete this;
}
