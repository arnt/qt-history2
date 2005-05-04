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

#include "qdockwidgetlayout_p.h"
#include "qdockwidget.h"

#include <qapplication.h>
#include <qdebug.h>
#include <qevent.h>
#include <qstyle.h>
#include <qvector.h>

#include <private/qlayoutengine_p.h>

#include "qdockwidget_p.h"
#include "qdockwidgetseparator_p.h"
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


static inline bool canGrow(Qt::Orientation o, const QSizePolicy &sp)
{ return (o == Qt::Horizontal ? sp.horizontalPolicy() : sp.verticalPolicy()) & QSizePolicy::GrowFlag; }


QDockWidgetLayout::QDockWidgetLayout(Qt::DockWidgetArea a, Qt::Orientation o)
    : QLayout(static_cast<QWidget*>(0)), area(a), orientation(o), save_layout_info(0),
      relayout_type(QInternal::RelayoutNormal)
{ connect(this, SIGNAL(emptied()), SLOT(maybeDelete()), Qt::QueuedConnection); }

QDockWidgetLayout::~QDockWidgetLayout()
{
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWidgetLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            info.item->widget()->deleteLater();
        if (!info.item->layout())
            delete info.item;
    }
}

void QDockWidgetLayout::saveState(QDataStream &stream) const
{
    stream << (uchar) Marker;
    stream << (uchar) orientation;
    stream << (layout_info.count() + 1) / 2;
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWidgetLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            continue;
	if (info.item->widget()) {
            const QWidget * const widget = info.item->widget();
            stream << (uchar) WidgetMarker;
            stream << widget->windowTitle();
            stream << (uchar) !widget->isHidden();
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
            const QDockWidgetLayout * const layout =
                qobject_cast<const QDockWidgetLayout *>(info.item->layout());
            layout->saveState(stream);
        }
    }
}

bool QDockWidgetLayout::restoreState(QDataStream &stream)
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
                    QDockWidgetLayoutInfo info(0);
                    stream >> info.cur_pos;
                    stream >> info.cur_size;
                    stream >> info.min_size;
                    stream >> info.max_size;
                    continue;
                }

                QDockWidgetLayoutInfo &info = insert(-1, new QWidgetItem(widget));
                widget->setVisible(shown);
                stream >> info.cur_pos;
                stream >> info.cur_size;
                stream >> info.min_size;
                stream >> info.max_size;
                break;
            }

        case Marker:
            {
                QDockWidgetLayout *layout = new QDockWidgetLayout(area, orientation);
                layout->setParent(this);
                QDockWidgetLayoutInfo &info = insert(-1, layout);
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
            // corrupt data
            relayout_type = QInternal::RelayoutNormal;
            return false;
        }
    }

    relayout_type = QInternal::RelayoutNormal;

    return true;
}

int QDockWidgetLayout::count() const
{
    qWarning("QDockWidgetLayout::count() is wrong");
    return layout_info.count();
}

QLayoutItem *QDockWidgetLayout::itemAt(int index) const
{
    VDEBUG("QDockWidgetLayout::itemAt: index %d (%d)", index, layout_info.count());

    int x = 0;
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWidgetLayoutInfo &info = layout_info.at(i);
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

QLayoutItem *QDockWidgetLayout::takeAt(int index)
{
    DEBUG("QDockWidgetLayout::takeAt: index %d", index);

    int x = 0;
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWidgetLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            continue;
	if (x++ == index) {
	    QLayoutItem *layoutitem = info.item;
            QWidget *widget = info.item->widget();

	    VDEBUG("QDockWidgetLayout::takeAt: layoutitem '%s'\n"
                   "  index %3d pos %4d size %4d %s",
                   (layoutitem->widget()
                    ? layoutitem->widget()->objectName().toLatin1().constData()
                    : "dummy"),
                   i, info.cur_pos, info.cur_size,
                   (widget
                    ? widget->objectName().toLatin1().constData()
                    : "dummy"));

            // remove the item
            layout_info.removeAt(i);

            // remove the separator
            if (i == layout_info.count()) {
                // we removed the last dockwidget, so we need to remove
                // the separator that was above it
                --i;
            }

            if (i != -1) {
                QDockWidgetLayoutInfo &sep_info = layout_info[i];
                Q_ASSERT(sep_info.is_sep);

                if (!save_layout_info) {
                    delete sep_info.item->widget();
                    delete sep_info.item;
                }

                VDEBUG("    removing separator at %d", i);
                layout_info.removeAt(i);
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
		    QLayout *parentLayout = qobject_cast<QLayout *>(parent());
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
void QDockWidgetLayout::addItem(QLayoutItem *layoutitem)
{
    if (relayout_type == QInternal::RelayoutDropped && layoutitem->layout()) {
        // dropping a nested layout!
        return;
    }

    (void) insert(-1, layoutitem);
    invalidate();
}

/*! \reimp */
void QDockWidgetLayout::setGeometry(const QRect &rect)
{
    VDEBUG("QDockWidgetLayout::setGeometry: width %4d height %4d", rect.width(), rect.height());

    QLayout::setGeometry(rect);

    if (relayout_type != QInternal::RelayoutDragging) {
        bool first = true;
        for (int i = 0; i < layout_info.count(); ++i) {
            const QDockWidgetLayoutInfo &info = layout_info.at(i);
            if (info.is_sep)
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
	qApp->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);

    for (x = 0; x < layout_info.count(); ++x) {
        const QDockWidgetLayoutInfo &info = layout_info.at(x);

        QLayoutStruct &ls = a[x];
        ls.init();
        ls.empty = info.item->isEmpty();
        if (ls.empty && !info.is_dropped)
            continue;

        if (info.is_sep) {
            VDEBUG("    separator");
            ls.sizeHint = ls.minimumSize = ls.maximumSize = separator_extent;
        } else {
            const QSizePolicy &sp =
                info.item->widget()
                ? info.item->widget()->sizePolicy()
                : QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

            if (info.is_dropped) {
                Q_ASSERT(relayout_type != QInternal::RelayoutNormal);
                Q_ASSERT(info.cur_size > 0);

                // item was just dropped into the layout
                ls.minimumSize = info.cur_size;
                ls.sizeHint = info.cur_size;
                ls.maximumSize = info.cur_size;
                // do not use stretch for dropped items... we want them in the
                // exact size we specify
                ls.stretch = 0;
            } else {
                ls.minimumSize = pick(orientation, info.item->minimumSize());
                ls.maximumSize = pick(orientation, info.item->maximumSize());

                if (canGrow(orientation, sp)) {
                    ls.sizeHint = ls.minimumSize;
                    ls.stretch = info.cur_size == -1
                                 ? pick(orientation, info.item->sizeHint())
                                 : info.cur_size;
                    ls.expansive = true;
                } else {
                    ls.sizeHint = info.cur_size == -1
                                 ? pick(orientation, info.item->sizeHint())
                                 : info.cur_size;
                }
            }

            // sanity checks
            ls.sizeHint = qMax(ls.sizeHint, ls.minimumSize);
            ls.minimumSize = qMin(ls.minimumSize, ls.maximumSize);

            VDEBUG("    dockwidget cur %4d min %4d max %4d, hint %4d stretch %4d "
                   "expansive %d empty %d",
                   info.cur_size, ls.minimumSize, ls.maximumSize, ls.sizeHint, ls.stretch, ls.expansive, ls.empty);
        }
    }

    qGeomCalc(a, 0, a.count(), 0, pick(orientation, rect.size()), 0);

    // DO IT
    VDEBUG("  final placement:");
    for (int i = 0; i < a.count(); ++i) {
	const QLayoutStruct &ls = a.at(i);
        if (ls.empty)
            continue;

	QDockWidgetLayoutInfo &info = layout_info[i];

	if (info.is_sep) {
	    VDEBUG("    separator  cur %4d", ls.size);
	} else {
	    VDEBUG("    dockwidget cur %4d min %4d max %4d pos %4d",
		   ls.size, ls.minimumSize, ls.maximumSize, ls.pos);
	}


	if (relayout_type == QInternal::RelayoutDragging) {
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
QSize QDockWidgetLayout::minimumSize() const
{
    if (!minSize.isValid()) {
        VDEBUG("QDockWidget::minimumSize");

        int size = 0, perp = 0;
        const int sep_extent =
            QApplication::style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);

        for (int it = 0; it < layout_info.count(); ++it) {
            const QDockWidgetLayoutInfo &info = layout_info.at(it);
            int s, p;
            if (info.is_sep) {
                s = p = (info.item->widget()->isHidden()) ? 0 : sep_extent;
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

        minSize = (orientation == Qt::Horizontal) ? QSize(size, perp) : QSize(perp, size);
    }
    return minSize;
}

/*! \reimp */
QSize QDockWidgetLayout::sizeHint() const
{
    if (!szHint.isValid()) {
        VDEBUG("QDockWidget::sizeHint");

        int size = 0, perp = 0;
        const int sep_extent =
            QApplication::style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);

        for (int it = 0; it < layout_info.count(); ++it) {
            const QDockWidgetLayoutInfo &info = layout_info.at(it);
            int s, p;
            if (info.is_sep) {
                s = p = (info.item->widget()->isHidden()) ? 0 : sep_extent;
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

        szHint = (orientation == Qt::Horizontal) ? QSize(size, perp) : QSize(perp, size);
    }
    return szHint;
}

void QDockWidgetLayout::invalidate()
{
    if (relayout_type != QInternal::RelayoutDragging) {
        QLayout::invalidate();
        minSize = szHint = QSize();
    }
}

bool QDockWidgetLayout::isEmpty() const
{
    for (int i = 0; i < layout_info.count(); ++i) {
        const QDockWidgetLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (!info.item->isEmpty())
            return false;
    }
    return true;
}

void QDockWidgetLayout::setOrientation(Qt::Orientation o)
{
    orientation = o;
    invalidate();
}

/*!
 */
QDockWidgetLayoutInfo &QDockWidgetLayout::insert(int index, QLayoutItem *layoutitem)
{
    DEBUG("QDockWidgetLayout::insert: index %d, layoutitem '%s'",
          index < 0 ? layout_info.count() : index,
          ((layoutitem->widget() || layoutitem->layout())
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
		++it; // ran of the end, force append
		break;
	    }

	    ++idx;
	    it += 2;
	}

	if (it == layout_info.count()) {
	    append = true;
	} else {
	    VDEBUG("inserting at %d (%d of %d)", it, idx, index);

	    // insert the dockwidget
	    VDEBUG("    inserting dockwidget at %d", it);
	    QDockWidgetLayoutInfo dockwidget_info(layoutitem);
	    int save_it = it;
	    layout_info.insert(it++, dockwidget_info);

	    // insert a separator
	    QLayoutItem *sep_item = 0;
            Q_ASSERT(!layout_info[it].is_sep);
            QDockWidgetSeparator *sep = new QDockWidgetSeparator(this, parentWidget());
            sep_item = new QWidgetItem(sep);

	    QDockWidgetLayoutInfo sep_info(sep_item);
	    sep_info.is_sep = 1;
	    VDEBUG("    inserting separator at %d", it);
	    layout_info.insert(it++, sep_info);

	    return layout_info[save_it];
	}
    }

    Q_ASSERT(append == true);

    // append the dockwidget
    if (!layout_info.isEmpty()) {
	Q_ASSERT(!layout_info.last().is_sep);

	// insert a separator before the dockwidget
	QLayoutItem *sep_item = 0;
        QDockWidgetSeparator *sep = new QDockWidgetSeparator(this, parentWidget());
        sep_item = new QWidgetItem(sep);

	QDockWidgetLayoutInfo sep_info(sep_item);
	sep_info.is_sep = 1;
	layout_info.append(sep_info);
    }

    QDockWidgetLayoutInfo dockwidget_info(layoutitem);
    layout_info.append(dockwidget_info);

    return layout_info.last();
}

void QDockWidgetLayout::dump()
{
    DEBUG("QDockWidgetLayout::dump");
    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWidgetLayoutInfo &info = layout_info.at(i);

	if (info.is_sep) {
	    DEBUG("  index %3d pos %4d size %4d SEPARATOR", i, info.cur_pos, info.cur_size);
	} else if (info.item->layout()) {
            DEBUG("  index %3d pos %4d size %4d %s", i, info.cur_pos, info.cur_size,
		  (info.item->layout()
                   ? info.item->layout()->objectName().toLatin1().constData()
                   : "dummy"));
            QDockWidgetLayout *l = qobject_cast<QDockWidgetLayout *>(info.item->layout());
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

void QDockWidgetLayout::saveLayoutInfo()
{
    Q_ASSERT(save_layout_info == 0);
    save_layout_info = new QList<QDockWidgetLayoutInfo>(layout_info);

    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWidgetLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (!info.item->layout())
            continue;

        QDockWidgetLayout *l = qobject_cast<QDockWidgetLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        l->saveLayoutInfo();
    }
}

void QDockWidgetLayout::resetLayoutInfo()
{
    Q_ASSERT(save_layout_info != 0);
    layout_info = *save_layout_info;

    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWidgetLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (!info.item->layout())
            continue;

        QDockWidgetLayout *l = qobject_cast<QDockWidgetLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        l->resetLayoutInfo();
    }
}

void QDockWidgetLayout::discardLayoutInfo()
{
    Q_ASSERT(save_layout_info != 0);
    delete save_layout_info;
    save_layout_info = 0;

    for (int i = 0; i < layout_info.count(); ++i) {
	const QDockWidgetLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (!info.item->layout())
            continue;

        QDockWidgetLayout *l = qobject_cast<QDockWidgetLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        l->discardLayoutInfo();
    }
}

QPoint QDockWidgetLayout::constrain(QDockWidgetSeparator *sep, int delta)
{
    VDEBUG("QDockWidgetLayout::constrain: delta %4d", delta);
    QList<QDockWidgetLayoutInfo> local_list;

    for (int pass = 0; pass < 2; ++pass) {
	VDEBUG("  PASS %d", pass);
	/*
	  During pass 1, we compute the feedback constraint.  During
	  pass 2, we update layout_info using the calculated
	  constraint.
	*/

	// find 'sep'
	local_list = save_layout_info ? *save_layout_info : layout_info;
	QMutableListIterator<QDockWidgetLayoutInfo> f_it(local_list), b_it(local_list);
	while (f_it.hasNext()) {
	    const QDockWidgetLayoutInfo &info = f_it.peekNext();
	    if (info.is_sep && qobject_cast<QDockWidgetSeparator*>(info.item->widget()) == sep) break;
	    (void)f_it.next();
	    (void)b_it.next();
	}
	// at this point, the iterator is just before 'sep'

	// get info for 'sep->prev' and move to just after sep->prev
	QDockWidgetLayoutInfo &info1 = b_it.previous(); // move to just after previous separator

	(void)f_it.next(); // move to before sep->next

	 // get info for 'sep->next' and move to just before next separator
	QDockWidgetLayoutInfo &info2 = f_it.next();

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

		    QDockWidgetLayoutInfo &f_info = f_it.next();

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

		    QDockWidgetLayoutInfo &b_info = b_it.previous();

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

void QDockWidgetLayout::relayout(QInternal::RelayoutType type)
{
    QInternal::RelayoutType save_type = relayout_type;
    relayout_type = type;
    setGeometry(geometry());
    relayout_type = save_type;
}

/*!
 */
QDockWidgetLayout::Location QDockWidgetLayout::locate(const QPoint &p) const
{
    // figure out where the dockwidget goes in the layout
    const int pos = pick(orientation, p);
    const bool horizontal = orientation == Qt::Horizontal;

    DEBUG() << "  locate: mouse at" << p;

    Location location;
    location.index = -1;
    for (int i = 0; i < layout_info.count(); ++i) {
        const QDockWidgetLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (info.item->isEmpty())
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
            location.index = i;
            location.area = ((dx > dy)
                             ? ((p.x() < p2.x())
                                ? Qt::LeftDockWidgetArea
                                : Qt::RightDockWidgetArea)
                             : ((p.y() < p2.y())
                                ? Qt::TopDockWidgetArea
                                : Qt::BottomDockWidgetArea));
            DEBUG() << "  result: index" << location.index << "area" << location.area;
            return location;
        }
    }

    location.index = layout_info.count() - 1;
    location.area = (horizontal ? Qt::RightDockWidgetArea : Qt::BottomDockWidgetArea);
    DEBUG() << "  result: index" << location.index << "area" << location.area << "(off-end)";
    return location;
}

static Qt::DockWidgetAreas getAllowedAreas(const QRect &r,
                                           const QSize &sz1,
                                           const QSize &sz2,
                                           const int separatorExtent)
{
    Qt::DockWidgetAreas allowedAreas = Qt::AllDockWidgetAreas;
    if (!r.contains(QRect(r.x(),
                          r.y(),
                          sz1.width() + sz2.width() + separatorExtent,
                          qMax(sz1.height(), sz2.height())))) {
        DEBUG() << "    cannot split horizontally";
        allowedAreas &= ~(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    }
    if (!r.contains(QRect(r.x(),
                          r.y(),
                          qMax(sz1.width(), sz2.width()),
                          sz1.height() + sz2.height() +separatorExtent))) {
        DEBUG() << "    cannot split vertically";
        allowedAreas &= ~(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    }
    return allowedAreas;
}

static QRect trySplit(Qt::Orientation orientation,
                      Qt::DockWidgetArea &area,
                      Qt::DockWidgetAreas allowedAreas,
                      const QRect &r,
                      const QPoint &p,
                      int separatorExtent)
{
    if (allowedAreas == 0) {
        // cannot split anywhere
        return QRect();
    }

    if ((allowedAreas & area) != area) {
        // cannot split in the desired location, pick another one
        switch (orientation) {
        case Qt::Horizontal:
            switch (area) {
            case Qt::TopDockWidgetArea:
            case Qt::BottomDockWidgetArea:
                DEBUG() << "    cannot split vertically, trying horizontally";
                area = ((p.x() < r.center().x())
                        ? Qt::LeftDockWidgetArea
                        : Qt::RightDockWidgetArea);
                break;
            default:
                break;
            }
            if ((allowedAreas & area) != area) {
                switch (area) {
                case Qt::LeftDockWidgetArea:
                    area = Qt::RightDockWidgetArea;
                    DEBUG() << "    cannot split left, trying right";
                    break;
                case Qt::RightDockWidgetArea:
                    area = Qt::LeftDockWidgetArea;
                    DEBUG() << "    cannot split right, trying left";
                    break;
                default:
                    break;
                }
            }
            if ((allowedAreas & area) != area) {
                DEBUG() << "      cannot split, trying vertically";
                area = ((p.y() < r.center().y())
                        ? Qt::TopDockWidgetArea
                        : Qt::BottomDockWidgetArea);
            }
            break;
        case Qt::Vertical:
            switch (area) {
            case Qt::LeftDockWidgetArea:
            case Qt::RightDockWidgetArea:
                DEBUG() << "    cannot split horizontally, trying vertically";
                area = ((p.y() < r.center().y())
                        ? Qt::TopDockWidgetArea
                        : Qt::BottomDockWidgetArea);
                break;
            default:
                break;
            }
            if ((allowedAreas & area) != area) {
                switch (area) {
                case Qt::TopDockWidgetArea:
                    DEBUG() << "    cannot split top, trying bottom";
                    area = Qt::BottomDockWidgetArea;
                    break;
                case Qt::BottomDockWidgetArea:
                    DEBUG() << "    cannot split bottom, trying top";
                    area = Qt::TopDockWidgetArea;
                    break;
                default:
                    break;
                }
            }
            if ((allowedAreas & area) != area) {
                DEBUG() << "      cannot split, trying horizontally";
                area = ((p.x() < r.center().x())
                        ? Qt::LeftDockWidgetArea
                        : Qt::RightDockWidgetArea);
            }
            break;
        default:
            Q_ASSERT_X(false, "QDockWidgetLayout", "internal error");
        }
    }

    if ((allowedAreas & area) != area) {
        // still cannot split, give up
        DEBUG() << "  cannot split at all, giving up";
        return QRect();
    }

    QRect rect;
    switch (area) {
    case Qt::LeftDockWidgetArea:
        rect.setRect(r.x(),
                     r.y(),
                     (r.width() - separatorExtent) / 2,
                     r.height());
        break;
    case Qt::RightDockWidgetArea:
        rect.setRect(r.right() - (r.width() - separatorExtent - 1) / 2,
                     r.y(),
                     (r.width() - separatorExtent + 1) / 2,
                     r.height());
        break;
    case Qt::TopDockWidgetArea:
        rect.setRect(r.x(),
                     r.y(),
                     r.width(),
                     (r.height() - separatorExtent) / 2);
        break;
    case Qt::BottomDockWidgetArea:
        rect.setRect(r.x(),
                     r.bottom() - (r.height() - separatorExtent - 1) / 2,
                     r.width(),
                     (r.height() - separatorExtent + 1) / 2);
        break;
    default:
        Q_ASSERT_X(false, "QDockWidgetLayout", "internal error");
    }

    return rect;
}

/*!
 */
QRect QDockWidgetLayout::place(QDockWidget *dockwidget, const QRect &r, const QPoint &mouse)
{
    DEBUG("QDockWidgetLayout::place");

    const QPoint p = parentWidget()->mapFromGlobal(mouse);
    Location location = locate(p - geometry().topLeft());
    const QDockWidgetLayoutInfo &info = layout_info.at(location.index);
    const bool horizontal = orientation == Qt::Horizontal;

    QRect target;

    if (info.item->layout()) {
        // forward the place to the nested layout
        QDockWidgetLayout *l = qobject_cast<QDockWidgetLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        DEBUG("  forwarding...");
        target = l->place(dockwidget, r, mouse);
        DEBUG("END of QDockWidgetLayout::place (forwarded)");
        return target;
    }

    const QSize sz1 = dockwidget->minimumSizeHint(),
                sz2 = info.item->minimumSize();
    const int separatorExtent =
        parentWidget()->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);
    Qt::DockWidgetAreas allowedAreas =
        getAllowedAreas(info.item->geometry(), sz1, sz2, separatorExtent);

    /*
      we do in-place reordering if the dock widget is in this layout.

      we allow splitting into adjacent items by delaying the
      reordering until the mouse is closer to the center of the
      adjacent item then the current one
    */
    int which = -1;
    for (int i = 0; which == -1 && i < layout_info.count(); ++i) {
        const QDockWidgetLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (info.item->isEmpty())
            continue;
        if (dockwidget == info.item->widget())
            which = i;
    }
    if (which != -1) {
        if (which == location.index) {
            target = info.item->geometry();
            target.moveTopLeft(parentWidget()->mapToGlobal(target.topLeft()));
            DEBUG() << "END of place (placed back at original position" << target << ")";
            return target;
        }

        const int center =
            pick(orientation, layout_info.at(location.index).item->geometry().center());
        const int pos = pick(orientation, p);

        if ((which > location.index && pos < center)
            || (which < location.index && pos > center)) {
            DEBUG() << "  swapping" << which << "with" << location.index;
            layout_info.swap(which, location.index);
            relayout();
            target = layout_info.at(location.index).item->geometry();
            target.moveTopLeft(parentWidget()->mapToGlobal(target.topLeft()));
            // make sure we don't discard the new layout information!
            *save_layout_info = layout_info;
            DEBUG() << "END of place, in-place reorder, target is" << target;
            return target;
        } else {
            DEBUG() << "  cannot swap" << endl;
            if (horizontal)
                allowedAreas &= ~(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            else
                allowedAreas &= ~(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
        }
    }

    DEBUG() << "  trySplit:" << orientation << location.area
            << info.item->geometry() << p << sz1 << sz2 << separatorExtent;
    target = ::trySplit(orientation, location.area, allowedAreas,
                        info.item->geometry(), p, separatorExtent);
    if (!target.isEmpty()) {
        target.setSize(target.size().expandedTo(sz1));
        target.moveTopLeft(parentWidget()->mapToGlobal(target.topLeft()));
    }
    DEBUG() << "END of place, target is" << target;
    return target;
}

/*!
 */
void QDockWidgetLayout::drop(QDockWidget *dockwidget, const QRect &r, const QPoint &mouse)
{
    DEBUG("QDockWidgetLayout::drop");

    if (dockwidget->isFloating()) {
        QMainWindowLayout *layout = qobject_cast<QMainWindowLayout *>(parentWidget()->layout());
        Q_ASSERT(layout != 0);
        layout->removeRecursive(dockwidget);
    }

    const QPoint p = parentWidget()->mapFromGlobal(mouse);
    Location location = locate(p - geometry().topLeft());
    const QDockWidgetLayoutInfo &info = layout_info.at(location.index);
    const bool horizontal = orientation == Qt::Horizontal;

    if (info.item->layout()) {
        // forward the drop to the nested layout
        QDockWidgetLayout *l = qobject_cast<QDockWidgetLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
        DEBUG("  forwarding...");
        l->drop(dockwidget, r, mouse);
        DEBUG("END of QDockWidgetLayout::drop (forwarded)");
        return;
    }

    if (dockwidget == info.item->widget()) {
        // placed back at original position
        if (dockwidget->isFloating()) {
            dockwidget->setFloating(false);
            dockwidget->show();
        }
        DEBUG("END of drop (shortcut - dropped at original position)");
        return;
    }

    const QSize sz1 = dockwidget->minimumSizeHint(),
                sz2 = info.item->minimumSize();
    const int separatorExtent =
        parentWidget()->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);
    Qt::DockWidgetAreas allowedAreas =
        getAllowedAreas(info.item->geometry(), sz1, sz2, separatorExtent);

    /*
      we do in-place reordering if the dock widget is in this layout.

      we allow splitting into adjacent items by delaying the
      reordering until the mouse is closer to the center of the
      adjacent item then the current one
    */
    int which = -1;
    for (int i = 0; which == -1 && i < layout_info.count(); ++i) {
        const QDockWidgetLayoutInfo &info = layout_info.at(i);
        if (info.is_sep)
            continue;
        if (info.item->isEmpty())
            continue;
        if (dockwidget == info.item->widget())
            which = i;
    }
    if (which != -1) {
        DEBUG() << "  drop after in-place reorder, only allowing perpendicular splits";
        if (horizontal)
            allowedAreas &= ~(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        else
            allowedAreas &= ~(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    }

    DEBUG() << "  trySplit:" << orientation << location.area
            << info.item->geometry() << p << sz1 << sz2 << separatorExtent;
    QRect target = ::trySplit(orientation, location.area, allowedAreas,
                              info.item->geometry(), p, separatorExtent);
    if (!target.isEmpty()) {
        if (!dockwidget->isFloating()) {
            QMainWindowLayout *layout =
                qobject_cast<QMainWindowLayout *>(parentWidget()->layout());
            Q_ASSERT(layout != 0);
            layout->removeRecursive(dockwidget);
        }

        bool nested = false;
        switch (orientation) {
        case Qt::Horizontal:
            switch (location.area) {
            case Qt::TopDockWidgetArea:
            case Qt::BottomDockWidgetArea:
                nested = true;
            default:
                break;
            }
            break;
        case Qt::Vertical:
            switch (location.area) {
            case Qt::LeftDockWidgetArea:
            case Qt::RightDockWidgetArea:
                nested = true;
            default:
                break;
            }
            break;
        default:
            Q_ASSERT_X(false, "QDockWidgetLayout", "internal error");
        }

        if (nested) {
            split(qobject_cast<QDockWidget *>(info.item->widget()), dockwidget, location.area);
        } else {
            int at = location.index / 2;
            if (location.area == Qt::RightDockWidgetArea
                || location.area == Qt::BottomDockWidgetArea)
                ++at;
            const int sz = pick(orientation, target.size());
            const_cast<QDockWidgetLayoutInfo &>(info).cur_size -= sz + separatorExtent;
            QDockWidgetLayoutInfo &newInfo = insert(at, new QWidgetItem(dockwidget));
            newInfo.cur_size = sz;
            newInfo.is_dropped = true;
            relayout(QInternal::RelayoutDropped);
            newInfo.is_dropped = false;
        }

        if (dockwidget->isFloating()) {
            // reparent the dock window into the main window
            dockwidget->setFloating(false);
            dockwidget->show();
        }
    }

    DEBUG("END of drop");
}

void QDockWidgetLayout::extend(QDockWidget *dockwidget, Qt::Orientation direction)
{
    if (direction == orientation) {
        addWidget(dockwidget);
    } else {
        Q_ASSERT(relayout_type == QInternal::RelayoutNormal);
        relayout_type = QInternal::RelayoutDropped;

        QDockWidgetLayout *nestedLayout = new QDockWidgetLayout(area, orientation);
        nestedLayout->setParent(this);
        nestedLayout->setObjectName(objectName() + QLatin1String("_nestedCopy"));

        for (int i = 0; i < layout_info.count(); ++i) {
            const QDockWidgetLayoutInfo &info = layout_info.at(i);
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
        addWidget(dockwidget);
    }
}

void QDockWidgetLayout::split(QDockWidget *existing, QDockWidget *with, Qt::DockWidgetArea area)
{
    int which = -1;
    for (int i = 0; which == -1 && i < layout_info.count(); ++i) {
	const QDockWidgetLayoutInfo &info = layout_info.at(i);
	if (info.is_sep)
            continue;
	if (existing == info.item->widget())
            which = i;
    }
    Q_ASSERT(which != -1);
    const QDockWidgetLayoutInfo &info = layout_info.at(which);

    Q_ASSERT(relayout_type == QInternal::RelayoutNormal);
    relayout_type = QInternal::RelayoutDropped;

    int save_size = info.cur_size;
    removeWidget(existing);
    // note: info is invalid from now on

    // create a nested window dock in place of the current widget
    QDockWidgetLayout *nestedLayout =
        new QDockWidgetLayout(area, orientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal);
    nestedLayout->setParent(this);
    nestedLayout->setObjectName(objectName() + "_nestedLayout");
    insert(which / 2, nestedLayout).cur_size = save_size;
    invalidate();

    switch (area) {
    case Qt::LeftDockWidgetArea:
    case Qt::TopDockWidgetArea:
        nestedLayout->addWidget(with);
        nestedLayout->addWidget(existing);
        break;

    case Qt::RightDockWidgetArea:
    case Qt::BottomDockWidgetArea:
        nestedLayout->addWidget(existing);
        nestedLayout->addWidget(with);
        break;

    default:
        Q_ASSERT_X(false, "QDockWidgetLayout", "internal error");
        break;
    }

    relayout_type = QInternal::RelayoutNormal;
}

void QDockWidgetLayout::maybeDelete()
{
    if (layout_info.isEmpty())
        delete this;
}
