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
    inline bool isEmpty() const
    { return false; }

    QWidget *widget;
    QRect rect;
};


QMainWindowLayout::QMainWindowLayout(QMainWindow *mainwindow)
    : QLayout(mainwindow), statusbar(0), relayout_type(QInternal::RelayoutNormal), save_layout_info(0),
      save_tb_layout_info(0)
{
    setObjectName(mainwindow->objectName() + "_layout");

    corners[Qt::TopLeftCorner]     = Qt::DockWindowAreaTop;
    corners[Qt::TopRightCorner]    = Qt::DockWindowAreaTop;
    corners[Qt::BottomLeftCorner]  = Qt::DockWindowAreaBottom;
    corners[Qt::BottomRightCorner] = Qt::DockWindowAreaBottom;

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
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
        for (int i = 0; i < lineInfo.list.size(); ++i)
            delete lineInfo.list.at(i).item;
    }
    tb_layout_info.clear();
    for (int i = 0; i < NPOSITIONS; ++i) {
        delete layout_info[i].item;
        if (layout_info[i].sep)
            delete layout_info[i].sep->widget();
        delete layout_info[i].sep;
        layout_info[i].item = 0;
        layout_info[i].sep = 0;
    }
    delete statusbar;
}

QStatusBar *QMainWindowLayout::statusBar() const
{ return statusbar ? qt_cast<QStatusBar *>(statusbar->widget()) : 0; }

void QMainWindowLayout::setStatusBar(QStatusBar *sb)
{
    addChildWidget(sb);
    delete statusbar;
    statusbar = new QWidgetItem(sb);
}

QWidget *QMainWindowLayout::centralWidget() const
{ return layout_info[CENTER].item ? layout_info[CENTER].item->widget() : 0; }

void QMainWindowLayout::setCentralWidget(QWidget *cw)
{
    addChildWidget(cw);
    delete layout_info[CENTER].item;
    layout_info[CENTER].item = new QWidgetItem(cw);
    layout_info[CENTER].size = QSize();
    invalidate();
}

void QMainWindowLayout::addToolBarBreak(Qt::ToolBarArea area)
{
    ToolBarLineInfo newLine;
    newLine.pos = positionForArea(area);
    switch (newLine.pos) {
    case TOP:
    case BOTTOM:
        {
            for (int line = 0; line < tb_layout_info.size(); ++line) {
                ToolBarLineInfo &lineInfo = tb_layout_info[line];
                if (lineInfo.pos == LEFT || lineInfo.pos == RIGHT) {
                    tb_layout_info.insert(line, newLine);
                    return;
                }
            }
        }
        // fall through intended
    default:
        tb_layout_info.append(newLine);
        TBDEBUG() << "appended new line";
        break;
    }
}

void QMainWindowLayout::insertToolBarBreak(QToolBar *before)
{
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        ToolBarLineInfo &lineInfo = tb_layout_info[line];
        for (int i = 0; i < lineInfo.list.size(); ++i) {
            const ToolBarLayoutInfo &info = lineInfo.list.at(i);
            if (info.item->widget() == before) {
                ToolBarLineInfo newLine;
                newLine.pos = lineInfo.pos;
                for (; i < lineInfo.list.size(); ++i)
                    newLine.list += lineInfo.list.takeAt(i);
                tb_layout_info.insert(line + 1, newLine);
                return;
            }
        }
    }
}

/*!
    Adds \a toolbar to \a area, continuing the current line.
*/
void QMainWindowLayout::addToolBar(Qt::ToolBarArea area, QToolBar *toolbar)
{
    removeToolBarInfo(toolbar);

    POSITION pos = positionForArea(area);
    // see if we have an existing line in the tb - append it in the last in line
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        if (tb_layout_info.at(line).pos == pos) {
            while (line < tb_layout_info.size() - 1 && tb_layout_info.at(line + 1).pos == pos)
                ++line;

            addChildWidget(toolbar);

            switch (pos) {
            case TOP:
            case BOTTOM:
                toolbar->setOrientation(Qt::Horizontal);
                break;
            case LEFT:
            case RIGHT:
                toolbar->setOrientation(Qt::Vertical);
                break;
            default:
                break;
            }

            ToolBarLineInfo &lineInfo = tb_layout_info[line];
            ToolBarLayoutInfo newinfo;
            newinfo.item = new QWidgetItem(toolbar);
            newinfo.is_dummy = false;
            lineInfo.list.append(newinfo);
            return;
        }
    }

    // no line to continue, add one and recurse
    addToolBarBreak(area);
    addToolBar(area, toolbar);
}

/*!
    Adds \a toolbar before \a before
*/
void QMainWindowLayout::insertToolBar(QToolBar *before, QToolBar *toolbar)
{
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (info.item->widget() == before) {
                addChildWidget(toolbar);

                ToolBarLayoutInfo newInfo;
                newInfo.item = new QWidgetItem(toolbar);
                newInfo.is_dummy = false;
                tb_layout_info[line].list.insert(i, newInfo);
                return;
            }
        }
    }
}

Qt::ToolBarArea QMainWindowLayout::toolBarArea(QToolBar *toolbar) const
{
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (info.item->widget() == toolbar)
                return static_cast<Qt::ToolBarArea>(areaForPosition(lineInfo.pos));
        }
    }
    Q_ASSERT_X(false, "QMainWindow::toolBarArea", "'toolbar' is not managed by this main window.");
    return Qt::ToolBarAreaTop;
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

        l = new QDockWindowLayout(area, orientations[pos]);
        l->setParent(this);
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

void QMainWindowLayout::extendDockWindowArea(Qt::DockWindowArea area, QDockWindow *dockwindow,
                                             Qt::Orientation orientation)
{
    removeRecursive(dockwindow);

    QDockWindowLayout * const layout = layoutForArea(area);
    layout->extend(dockwindow, orientation);
}

void QMainWindowLayout::splitDockWindow(QDockWindow *after, QDockWindow *dockwindow,
                                        Qt::Orientation orientation)
{
    removeRecursive(dockwindow);

    const Qt::DockWindowArea area = dockWindowArea(after);
    QDockWindowLayout * const layout = layoutForArea(area);
    layout->split(after, dockwindow, (orientation == Qt::Horizontal
                                      ? Qt::DockWindowAreaRight
                                      : Qt::DockWindowAreaBottom));
}

static bool findWidgetRecursively(QLayoutItem *li, QWidget *w)
{
    QLayout *lay = li->layout();
    if (!lay)
        return false;
    int i = 0;
    QLayoutItem *child;
    while ((child = lay->itemAt(i))) {
        if (child->widget() == w) {
            return true;
        } else if (findWidgetRecursively(child, w)) {
            return true;
        } else {
            ++i;
        }
    }
    return false;
}

Qt::DockWindowArea QMainWindowLayout::dockWindowArea(QDockWindow *dockwindow) const
{
    for (int pos = 0; pos < NPOSITIONS - 1; ++pos) {
        if (!layout_info[pos].item)
            continue;
        if (findWidgetRecursively(layout_info[pos].item, dockwindow))
            return static_cast<Qt::DockWindowArea>(areaForPosition(pos));
    }
    Q_ASSERT_X(false, "QMainWindow::dockWindowArea",
               "'dockwindow' is not managed by this main window.");
    return Qt::DockWindowAreaTop;

}

void QMainWindowLayout::saveState(QDataStream &stream) const
{
    // save toolbar state
    stream << (uchar) ToolBarStateMarker;
    stream << tb_layout_info.size(); // number of toolbar lines
    if (!tb_layout_info.isEmpty()) {
        for (int line = 0; line < tb_layout_info.size(); ++line) {
            const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
            stream << lineInfo.pos;
            stream << lineInfo.list.size();
            for (int i = 0; i < lineInfo.list.size(); ++i) {
                const ToolBarLayoutInfo &info = lineInfo.list.at(i);
                stream << info.item->widget()->windowTitle();
                stream << (uchar) info.item->widget()->isShown();
                stream << info.pos;
                stream << info.size;
                stream << info.offset;
            }
        }
    }

    // save dockwindow state
    stream << (uchar) DockWindowStateMarker;
    int x = 0;
    for (int i = 0; i < NPOSITIONS - 1; ++i) {
        if (!layout_info[i].item)
            continue;
        ++x;
    }
    stream << x;
    for (int i = 0; i < NPOSITIONS - 1; ++i) {
        if (!layout_info[i].item) {
            continue;
        }
        stream << i;
        stream << layout_info[i].size;
        const QDockWindowLayout * const layout =
            qt_cast<const QDockWindowLayout *>(layout_info[i].item->layout());
        Q_ASSERT(layout != 0);
        layout->saveState(stream);
    }

    // save center widget state
    stream << layout_info[CENTER].size;
}

bool QMainWindowLayout::restoreState(QDataStream &stream)
{
    // restore toolbar layout
    uchar tmarker;
    stream >> tmarker;
    if (tmarker != ToolBarStateMarker)
        return false;

    int lines;
    stream >> lines;
    QList<ToolBarLineInfo> toolBarState;
    const QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(parentWidget());
    for (int line = 0; line < lines; ++line) {
        ToolBarLineInfo lineInfo;
        stream >> lineInfo.pos;
        int size;
        stream >> size;
        for (int i = 0; i < size; ++i) {
            ToolBarLayoutInfo info;
            QString windowTitle;
            stream >> windowTitle;
            uchar shown;
            stream >> shown;
            stream >> info.pos;
            stream >> info.size;
            stream >> info.offset;

            // find toolbar
            QToolBar *toolbar = 0;
            for (int t = 0; t < toolbars.size(); ++t) {
                if (toolbars.at(t)->windowTitle() == windowTitle) {
                    toolbar = toolbars.at(t);
                    break;
                }
            }
            if (!toolbar)
                continue;

            info.item = new QWidgetItem(toolbar);
            toolbar->setShown(shown);
            lineInfo.list << info;
        }
        toolBarState << lineInfo;
    }

    if (stream.status() != QDataStream::Ok)
        return false;

    // replace existing toolbar layout
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
        for (int i = 0; i < lineInfo.list.size(); ++i)
            delete lineInfo.list.at(i).item;
    }
    tb_layout_info = toolBarState;

    // restore dockwindow layout
    uchar dmarker;
    stream >> dmarker;
    if (dmarker != DockWindowStateMarker)
        return false;

    save_layout_info = new QVector<QMainWindowLayoutInfo>(layout_info);

    // clear out our working copy
    for (int i = 0; i < NPOSITIONS - 1; ++i) {
        layout_info[i].item = 0;
        layout_info[i].sep = 0;
        layout_info[i].size = QSize();
        layout_info[i].is_dummy = false;
    }

    int areas;
    stream >> areas;
    for (int area = 0; area < areas; ++area) {
        int pos;
        stream >> pos;
        stream >> layout_info[pos].size;
        QDockWindowLayout * const layout =
            layoutForArea(static_cast<Qt::DockWindowArea>(areaForPosition(pos)));
        if (!layout->restoreState(stream)) {
            stream.setStatus(QDataStream::ReadCorruptData);
            break;
        }
    }

    // restore center widget size
    stream >> layout_info[CENTER].size;

    if (stream.status() != QDataStream::Ok) {
        // restore failed, get rid of the evidence
        for (int i = 0; i < NPOSITIONS - 1; ++i) {
            if (layout_info[i].sep)
                delete layout_info[i].sep->widget();
            delete layout_info[i].sep;
            delete layout_info[i].item;
        }
        layout_info = *save_layout_info;

        delete save_layout_info;
        save_layout_info = 0;
        relayout_type = QInternal::RelayoutNormal;

        return false;
    }

    // replace existing dockwindow layout
    for (int i = 0; i < NPOSITIONS - 1; ++i) {
        if ((*save_layout_info)[i].sep)
            delete (*save_layout_info)[i].sep->widget();
        delete (*save_layout_info)[i].sep;
        delete (*save_layout_info)[i].item;
    }

    delete save_layout_info;
    save_layout_info = 0;
    relayout_type = QInternal::RelayoutNormal;

    return true;
}

QLayoutItem *QMainWindowLayout::itemAt(int index) const
{
    int x = 0;
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	for (int i = 0; i < lineInfo.list.size(); ++i) {
            if (x++ == index) {
                const ToolBarLayoutInfo &info = lineInfo.list.at(i);
                return info.item;
            }
        }
    }
    for (int i = 0; i < NPOSITIONS; ++i) {
        if (!layout_info[i].item)
            continue;
        if (x++ == index)
            return layout_info[i].item;
    }
    return 0;
}

QLayoutItem *QMainWindowLayout::takeAt(int index)
{
    DEBUG("QMainWindowLayout::takeAt: index %d", index);

    int x = 0;
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        ToolBarLineInfo &lineInfo = tb_layout_info[line];
	for (int i = 0; i < lineInfo.list.size(); ++i) {
            if (x++ == index) {
                QLayoutItem *ret = lineInfo.list.at(i).item;
                lineInfo.list.removeAt(i);
                if (lineInfo.list.size() == 0)
                    tb_layout_info.removeAt(line);
                return ret;
            }
	}
    }

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
    const Qt::DockWindowArea area = static_cast<Qt::DockWindowArea>(areaForPosition(pos));

    const struct
    {
        Qt::Corner corner1, corner2;
        POSITION perp1, perp2;
    } order[] = {
        // LEFT
        {
            Qt::TopLeftCorner,
            Qt::BottomLeftCorner,
            TOP,
            BOTTOM
        },
        // RIGHT
        {
            Qt::TopRightCorner,
            Qt::BottomRightCorner,
            TOP,
            BOTTOM
        },
        // TOP
        {
            Qt::TopLeftCorner,
            Qt::TopRightCorner,
            LEFT,
            RIGHT
        },
        // BOTTOM
        {
            Qt::BottomLeftCorner,
            Qt::BottomRightCorner,
            LEFT,
            RIGHT
        }
    };

    if (!layout->layout_info[pos].item)
        return;

    if ((layout->corners[order[pos].corner1] != area
         || !layout->layout_info[order[pos].perp1].item)
        && (layout->corners[order[pos].corner2] != area
            || !layout->layout_info[order[pos].perp2].item)) {
        // if the area does not occupy any corner, we constrain the
        // minimum size of the center to the minimum size of the area
        ls[1].minimumSize = qMax(pick_perp(pos, layout->layout_info[pos].item->minimumSize()),
                                 ls[1].minimumSize);
        ls[1].maximumSize = qMin(pick_perp(pos, layout->layout_info[pos].item->maximumSize()),
                                 ls[1].maximumSize);
    } else {
        // if the area occupies only a single corner, then we
        // distribute the minimum size of the area across the center
        // and opposite side equally
        const int min = pick_perp(pos, layout->layout_info[pos].item->minimumSize());
        if (layout->layout_info[order[pos].perp1].item
            && layout->corners[order[pos].corner1] == area
            && layout->corners[order[pos].corner2] != area) {
            ls[1].minimumSize = qMax(ls[1].minimumSize, min - ls[0].minimumSize);
        } else if (layout->layout_info[order[pos].perp2].item
                   && layout->corners[order[pos].corner2] == area
                   && layout->corners[order[pos].corner1] != area) {
            ls[1].minimumSize = qMax(ls[1].minimumSize, min - ls[2].minimumSize);
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
        ls.sizeHint  = !info.size.isEmpty()
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
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	QSize tb_sz;
        for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    QSize sh = info.item->sizeHint();
	    if (tb_sz.width() < sh.width())
		tb_sz.setWidth(sh.width());
	    if (tb_sz.height() < sh.height())
		tb_sz.setHeight(sh.height());

	}
	switch(lineInfo.pos) {
	case TOP:
	case LEFT:
	    rest_sz[line] = tb_sz;
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
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);

	QSize tb_sz;
	tb_rect[line] = r;

	switch (lineInfo.pos) {
	case TOP:
	    tb_sz = rest_sz[line];
	    tb_rect[line].setBottom(tb_rect[line].top() + tb_sz.height() - 1);
	    r.setTop(tb_rect[line].bottom() + 1);
	    break;
	case LEFT:
	    tb_sz = rest_sz[line];
	    tb_rect[line].setRight(tb_rect[line].x() + tb_sz.width() - 1);
	    r.setLeft(tb_rect[line].right() + 1);
	    break;
	case BOTTOM:
 	    tb_sz = bottom_sz.pop();
	    tb_rect[line].setTop(tb_rect[line].bottom() - tb_sz.height() + 1);
	    bottom_rect.push(tb_rect[line]);
	    r.setBottom(tb_rect[line].top() - 1);
	    break;
	case RIGHT:
 	    tb_sz = right_sz.pop();
	    tb_rect[line].setLeft(tb_rect[line].right() - tb_sz.width() + 1);
	    right_rect.push(tb_rect[line]);
	    r.setRight(tb_rect[line].left() - 1);
	    break;
	default:
	    Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
	}
    }

    // put the right and bottom rects back in correct order
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	if (lineInfo.pos == BOTTOM)
	    tb_rect[line] = bottom_rect.pop();
	else if (lineInfo.pos == RIGHT)
	    tb_rect[line] = right_rect.pop();
    }


    // at this point the space for the tool bars have been shaved off
    // the main rect, continue laying out each tool bar line

    int tb_fill = 0;
    if (tb_layout_info.size() != 0) {
	tb_fill = QApplication::style()->pixelMetric(QStyle::PM_ToolBarHandleExtent)
                  + QApplication::style()->pixelMetric(QStyle::PM_ToolBarFrameWidth) * 2
		  + QApplication::style()->pixelMetric(QStyle::PM_ToolBarItemSpacing) * 3
                  + QApplication::style()->pixelMetric(QStyle::PM_ToolBarExtensionExtent);
    }

    for (int line = 0; line < tb_layout_info.size(); ++line) {
        ToolBarLineInfo &lineInfo = tb_layout_info[line];
        int num_tbs = lineInfo.list.size();
	POSITION where = static_cast<POSITION>(lineInfo.pos);

        for (int i = 0; i < num_tbs; ++i) {
            ToolBarLayoutInfo &info = lineInfo.list[i];
 	    set(where, info.size, pick(where, tb_rect[line].size()));

	    // position
 	    if (i == 0) { // first tool bar can't have an offset
		if (num_tbs > 1 && !info.size.isEmpty()
		    && pick_perp(where, info.offset) > pick_perp(where, info.size)) {
		    // swap if dragging it past the next one
		    ToolBarLayoutInfo &next = lineInfo.list[i+1];
		    next.pos = tb_rect[line].topLeft();
		    next.size.setHeight(pick_perp(where, get_min_item_sz(next.item->widget()->layout())) + tb_fill);
		    next.offset = QPoint();
		    if (where == LEFT || where == RIGHT)
			info.pos = QPoint(tb_rect[line].left(), next.pos.y() + next.size.height());
		    else
			info.pos = QPoint(next.pos.x() + next.size.width(), tb_rect[line].top());
		    info.offset = QPoint(); // has to be done before swapping
		    lineInfo.list.swap(i, i+1);
		} else {
		    info.pos = tb_rect[line].topLeft();
		    info.offset = QPoint();
		}
	    } else {
		ToolBarLayoutInfo &prev = lineInfo.list[i-1];
		QSize minSize(0, 0);
                if(info.item->widget()->layout()->itemAt(1))
                    minSize = info.item->widget()->layout()->itemAt(1)->widget()->minimumSizeHint();
		set_perp(where, minSize, pick_perp(where, minSize) + tb_fill);
 		const int cur_pt = pick_perp(where, prev.pos) + pick_perp(where, prev.size);
		const int prev_min = pick_perp(where, get_min_item_sz(prev.item->widget()->layout())) + tb_fill;

		if (where == LEFT || where == RIGHT)
		    info.pos = QPoint(tb_rect[line].left(), cur_pt + info.offset.y());
		else
		    info.pos = QPoint(cur_pt + info.offset.x(), tb_rect[line].top());

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
			    ToolBarLayoutInfo &t = lineInfo.list[l];
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
				lineInfo.list.swap(i, i-1);
			}
		    }

		} else if (pick_perp(where, info.offset) > 0) { // right/down motion
		    if (pick_perp(where, info.size) - pick_perp(where, info.offset) > pick_perp(where, minSize)) {
			QSize sz(0, 0);
			set_perp(where, sz, pick_perp(where, info.offset));
			info.size -= sz;
		    } else {
			bool can_push = false;
			for (int l = i+1; l < num_tbs; ++l) {
			    ToolBarLayoutInfo &t = lineInfo.list[l];
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
			    int can_remove = pick_perp(where, info.size) - pick_perp(where, minSize);
			    set_perp(where, info.pos, cur_pt + can_remove);
			    QSize sz(0, 0);
			    set_perp(where, sz, can_remove);
			    info.size -= sz;
			    if (i+1 < num_tbs) {
				ToolBarLayoutInfo &t = lineInfo.list[i+1];

				if (pick_perp(where, info.pos) + pick_perp(where, info.offset) > pick_perp(where, t.pos))
				    lineInfo.list.swap(i, i+1);
			    }
			}
		    }
		}

		// Figure out a suitable default pos/size
		if (pick_perp(where, info.pos) < pick_perp(where, prev.pos) + prev_min) {
		    int sz = pick_perp(where, prev.item->widget()->sizeHint());
		    // use min size hint if size hint is smaller
		    if (sz < prev_min)
			sz = prev_min;
		    set_perp(where, info.pos, pick_perp(where, prev.pos) + sz);
		}
		info.offset = QPoint();
	    }

	    // size
	    if (num_tbs == 1)
		set_perp(where, info.size, pick_perp(where, tb_rect[line].size()));
	    else if (i == num_tbs-1) {
		set_perp(where, info.size, pick_perp(where, tb_rect[line].size()));
		if (where == LEFT || where == RIGHT) {
		    info.size.setHeight(tb_rect[line].bottom() - info.pos.y()
                                        + 1);
                } else {
		    info.size.setWidth(tb_rect[line].right() - info.pos.x()
                                       + 1);
                }
		if (pick_perp(where, info.size) < 1)
		    set_perp(where, info.size, pick_perp(where, get_min_item_sz(info.item->widget()->layout())) + tb_fill);
	    }
	    if (i > 0) {
		// assumes that all tbs are positioned at the correct
		// pos - fill in the gaps btw them
		ToolBarLayoutInfo &prev = lineInfo.list[i-1];
		set_perp(where, prev.size, pick_perp(where, info.pos) - pick_perp(where, prev.pos));
	    }
	}

	for (int i = 0; i < num_tbs; ++i) {
	    ToolBarLayoutInfo &info = lineInfo.list[i];
	    QRect tb(info.pos, info.size);
	    if (!tb.isEmpty() && relayout_type == QInternal::RelayoutNormal)
		info.item->setGeometry(tb);
	}
    }

    // layout dockwindows and center widget
    const int ext = QApplication::style()->pixelMetric(QStyle::PM_DockWindowSeparatorExtent);

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
            !layout_info[CENTER].size.isEmpty()
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
            !layout_info[CENTER].size.isEmpty()
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

    switch (corners[Qt::TopLeftCorner]) {
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

    switch (corners[Qt::BottomLeftCorner]) {
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

    switch (corners[Qt::TopRightCorner]) {
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

    switch (corners[Qt::BottomRightCorner]) {
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
        c.setCoords(left.right() + 1,
                    top.bottom() + 1,
                    right.left() - 1,
                    bottom.top() - 1);
        layout_info[CENTER].size = c.size();
        if (relayout_type == QInternal::RelayoutNormal)
            layout_info[CENTER].item->setGeometry(c);
    }
}

void QMainWindowLayout::addItem(QLayoutItem *)
{ qWarning("QMainWindowLayout: please use the public QMainWindow API instead."); }

QSize QMainWindowLayout::sizeHint() const
{
    int left = 0, right = 0, top = 0, bottom = 0;

    // layout toolbars
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
        QSize sz;
        // need to get the biggest size hint for all tool bars on each line
        for (int i = 0; i < lineInfo.list.size(); ++i) {
            const ToolBarLayoutInfo &info = lineInfo.list.at(i);
            QSize ms = info.item->sizeHint();

            if (((lineInfo.pos == LEFT || lineInfo.pos == RIGHT) && (ms.width() > sz.width()))
                || (ms.height() > sz.height()))
                sz = ms;
        }
        switch (lineInfo.pos) {
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

    w1 = (corners[Qt::TopLeftCorner] == Qt::DockWindowAreaLeft ? szL.width() : 0)
         + szT.width()
         + (corners[Qt::TopRightCorner] == Qt::DockWindowAreaRight ? szR.width() : 0);
    w2 = szL.width() + szR.width() + szC.width();
    w3 = (corners[Qt::BottomLeftCorner] == Qt::DockWindowAreaLeft ? szL.width() : 0)
         + szB.width()
         + (corners[Qt::BottomRightCorner] == Qt::DockWindowAreaRight ? szR.width(): 0);

    h1 = (corners[Qt::TopLeftCorner] == Qt::DockWindowAreaTop ? szT.height() : 0)
         + szL.height()
         + (corners[Qt::BottomLeftCorner] == Qt::DockWindowAreaBottom ? szB.height(): 0);
    h2 = szT.height() + szB.height() + szC.height();
    h3 = (corners[Qt::TopRightCorner] == Qt::DockWindowAreaTop ? szT.height() : 0)
         + szR.height()
         + (corners[Qt::BottomRightCorner] == Qt::DockWindowAreaBottom ? szB.height() : 0);

    const int ext = QApplication::style()->pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
    if (layout_info[LEFT].item && !szL.isEmpty())
        left += ext;
    if (layout_info[RIGHT].item && !szR.isEmpty())
        right += ext;
    if (layout_info[TOP].item && !szT.isEmpty())
        top += ext;
    if (layout_info[BOTTOM].item && !szB.isEmpty())
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
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
        QSize sz;
        // need to get the biggest min size for all tool bars on each line
        for (int i = 0; i < lineInfo.list.size(); ++i) {
            const ToolBarLayoutInfo &info = lineInfo.list.at(i);
            QSize ms = info.item->minimumSize();

            if (((lineInfo.pos == LEFT || lineInfo.pos == RIGHT) && (ms.width() > sz.width()))
                || (ms.height() > sz.height()))
                sz = ms;
        }
        switch (lineInfo.pos) {
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

    w1 = (corners[Qt::TopLeftCorner] == Qt::DockWindowAreaLeft ? szL.width() : 0)
         + szT.width()
         + (corners[Qt::TopRightCorner] == Qt::DockWindowAreaRight ? szR.width() : 0);
    w2 = szL.width() + szR.width() + szC.width();
    w3 = (corners[Qt::BottomLeftCorner] == Qt::DockWindowAreaLeft ? szL.width() : 0)
         + szB.width()
         + (corners[Qt::BottomRightCorner] == Qt::DockWindowAreaRight ? szR.width() : 0);

    h1 = (corners[Qt::TopLeftCorner] == Qt::DockWindowAreaTop ? szT.height() : 0)
         + szL.height()
         + (corners[Qt::BottomLeftCorner] == Qt::DockWindowAreaBottom ? szB.height() : 0);
    h2 = szT.height() + szB.height() + szC.height();
    h3 = (corners[Qt::TopRightCorner] == Qt::DockWindowAreaTop ? szT.height() : 0)
         + szR.height()
         + (corners[Qt::BottomRightCorner] == Qt::DockWindowAreaBottom ? szB.height() : 0);

    const int ext = QApplication::style()->pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
    if (layout_info[LEFT].item && !szL.isEmpty())
        left += ext;
    if (layout_info[RIGHT].item && !szR.isEmpty())
        right += ext;
    if (layout_info[TOP].item && !szT.isEmpty())
        top += ext;
    if (layout_info[BOTTOM].item && !szB.isEmpty())
        bottom += ext;

    VDEBUG("QMainWindowLayout::minimumSize:\n"
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
    } else {
        Q_ASSERT(info[BOTTOM].item && info[BOTTOM].item->layout() == dock);
        pos = BOTTOM;
        delta = -delta;
    }

    // remove delta from 'dock'
    int current = pick(pos, info[pos].size)
                  + pick(pos, info[CENTER].size)
                  + pick(pos, info[order[pos]].size);

    const int _ext = QApplication::style()->pixelMetric(QStyle::PM_DockWindowSeparatorExtent);
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
    Q_UNUSED(current);
    Q_UNUSED(new_current);
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
                 dx = qAbs(p.x() - (width / 2)),
                 dy = qAbs(p.y() - (height / 2));

    if (dx > dy) {
        if (p.x() < width / 2 && dockwindow->isDockable(Qt::DockWindowAreaLeft)) {
            // left side
            if (p.y() < height / 3 && dockwindow->isDockable(Qt::DockWindowAreaTop))
                pos = positionForArea(corners[Qt::TopLeftCorner]);
            else if (p.y() > height * 2 / 3 && dockwindow->isDockable(Qt::DockWindowAreaBottom))
                pos = positionForArea(corners[Qt::BottomLeftCorner]);
            else
                pos = LEFT;
        } else if (p.x() >= width / 2 && dockwindow->isDockable(Qt::DockWindowAreaRight)) {
            // right side
            if (p.y() < height / 3 && dockwindow->isDockable(Qt::DockWindowAreaTop))
                pos = positionForArea(corners[Qt::TopRightCorner]);
            else if (p.y() >= height * 2 / 3 && dockwindow->isDockable(Qt::DockWindowAreaBottom))
                pos = positionForArea(corners[Qt::BottomRightCorner]);
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
                pos = positionForArea(corners[Qt::TopLeftCorner]);
            else if (p.x() >= width * 2 / 3 && dockwindow->isDockable(Qt::DockWindowAreaRight))
                pos = positionForArea(corners[Qt::TopRightCorner]);
            else
                pos = TOP;
        } else if (p.y() >= height / 2 && dockwindow->isDockable(Qt::DockWindowAreaBottom)) {
            // bottom side
            if (p.x() < width / 3 && dockwindow->isDockable(Qt::DockWindowAreaLeft))
                pos = positionForArea(corners[Qt::BottomLeftCorner]);
            else if (p.x() >= width * 2 / 3 && dockwindow->isDockable(Qt::DockWindowAreaRight))
                pos = positionForArea(corners[Qt::BottomRightCorner]);
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

int QMainWindowLayout::locateToolBar(QToolBar *toolbar, const QPoint &mouse) const
{
    const int width = parentWidget()->width(),
             height = parentWidget()->height();
    const QPoint p = parentWidget()->mapFromGlobal(mouse),
                p2 = QPoint(width / 2, height / 2);
    const int dx = qAbs(p.x() - p2.x()),
              dy = qAbs(p.y() - p2.y());

    POSITION pos = CENTER;
    if (dx > dy) {
        if (p.x() < p2.x() && toolbar->isDockable(Qt::ToolBarAreaLeft)) {
            pos = LEFT;
        } else if (p.x() >= p2.x() && toolbar->isDockable(Qt::ToolBarAreaRight)) {
            pos = RIGHT;
        } else {
            if (p.y() < p2.y() && toolbar->isDockable(Qt::ToolBarAreaTop))
                pos = TOP;
            else if (p.y() >= p2.y() && toolbar->isDockable(Qt::ToolBarAreaBottom))
                pos = BOTTOM;
        }
    } else {
        if (p.y() < p2.y() && toolbar->isDockable(Qt::ToolBarAreaTop)) {
                pos = TOP;
        } else if (p.y() >= p2.y() && toolbar->isDockable(Qt::ToolBarAreaBottom)) {
                pos = BOTTOM;
        } else {
            if (p.x() < p2.x() && toolbar->isDockable(Qt::ToolBarAreaLeft))
                pos = LEFT;
            else if (p.x() >= p2.x() && toolbar->isDockable(Qt::ToolBarAreaRight))
                pos = RIGHT;
        }
    }
    Q_ASSERT(pos != CENTER);

    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	bool break_it = false;
        for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (!info.item)
                continue;
 	    if (!info.item->geometry().contains(p))
                continue;
	    pos = static_cast<POSITION>(lineInfo.pos);
	    break_it = true;
	    break;
	}
	if (break_it)
	    break;
    }
    return pos;
}

void QMainWindowLayout::dropToolBar(QToolBar *toolbar, const QPoint &mouse, const QPoint &offset)
{
    POSITION where = static_cast<POSITION>(locateToolBar(toolbar, mouse));

    if (positionForArea(toolBarArea(toolbar)) == where) {

#ifdef TOOLBAR_DEBUG
	TBDEBUG() << "###";
	for (int l = 0; l < tb_layout_info.size(); ++l) {
            const ToolBarLineInfo &lineInfo = tb_layout_info.at(l);
	    for (int i = 0; i < lineInfo.list.size(); ++i) {
		const ToolBarLayoutInfo &tmp = lineInfo.list.at(i);
 		qDebug() << "bar: " << lineInfo.pos << l << i << tmp.item->widget() << tmp.item->widget()->geometry();
	    }
	}
#endif
	// move the tool bars more than magic_offset pixels past a boundary
	// in either dir in order to move it to a different tb line
	const int magic_offset = 5;
	int l = 0, i = 0;
	ToolBarLayoutInfo info;

	for (l = 0; l < tb_layout_info.size(); ++l) {
            ToolBarLineInfo &lineInfo = tb_layout_info[l];
	    bool break_it = false;
	    for (i = 0; i < lineInfo.list.size(); ++i) {
		ToolBarLayoutInfo &tmp = lineInfo.list[i];
		if (tmp.item->widget() == toolbar) {
		    info = tmp;
		    tmp.offset += offset;
		    break_it = true;
		    break;
		}
	    }
	    if (break_it)
                break;
	}

	if (pick(where, offset) < -magic_offset) { // move left/up
	    TBDEBUG() << "left/up" << offset << l << where;
	    if (l > 0 && tb_layout_info.at(l-1).pos == where) { // is this the first line in this tb area?
		tb_layout_info[l].list.removeAt(i);
		if (tb_layout_info[l].list.size() == 0)
		    tb_layout_info.removeAt(l);
		if (tb_layout_info.at(l-1).pos == where) {
		    TBDEBUG() << "br 1 appending to existing" << info.item->widget() << info.item->widget()->geometry();
		    tb_layout_info[l-1].list.append(info);
		} else {
		    ToolBarLineInfo line;
                    line.pos = where;
		    line.list.append(info);
		    TBDEBUG() << "br 2 new tb_line";
		    tb_layout_info.insert(l, line);
		}
	    } else if (tb_layout_info.at(l).list.size() > 1) {
		tb_layout_info[l].list.removeAt(i);
		ToolBarLineInfo line;
                line.pos = where;
		line.list.append(info);
		tb_layout_info.insert(l, line);
		TBDEBUG() << "br3 new tb line" << l << toolbar;
	    }
	} else if (pick(where, offset) > pick(where, info.size) + magic_offset) { // move right/down
	    TBDEBUG() << "right/down" << offset;
	    if (l < tb_layout_info.size()-1 && tb_layout_info.at(l+1).pos == where) {
		tb_layout_info[l].list.removeAt(i);

		if (tb_layout_info.at(l).list.size() == 0)
		    tb_layout_info.removeAt(l--);

		if (tb_layout_info.at(l+1).pos == where) {
		    tb_layout_info[l+1].list.append(info);
		    TBDEBUG() << "1. appending to exisitng";
		} else {
		    ToolBarLineInfo line;
                    line.pos = where;
		    line.list.append(info);
		    tb_layout_info.insert(l, line);
		    TBDEBUG() << "2. inserting new";
		}
	    } else if (tb_layout_info.at(l).list.size() > 1) { // can remove
		tb_layout_info[l].list.removeAt(i);
		ToolBarLineInfo line;
                line.pos = where;
		line.list.append(info);
		tb_layout_info.insert(l+1, line);
		TBDEBUG() << "3. new line";

	    }
	}
    } else {
        TBDEBUG() << "changed area";
        addToolBar(static_cast<Qt::ToolBarArea>(areaForPosition(where)), toolbar);
        dropToolBar(toolbar, mouse, offset);
        return;
    }
    relayout();
}

void QMainWindowLayout::removeToolBarInfo(QToolBar *toolbar)
{
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        ToolBarLineInfo &lineInfo = tb_layout_info[line];
	for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (info.item->widget() == toolbar) {
		delete info.item;
		lineInfo.list.removeAt(i);
		if (lineInfo.list.size() == 0)
		    tb_layout_info.removeAt(line);
		break;
	    }
	}
    }
}
