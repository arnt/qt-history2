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

#ifndef QT_NO_MAINWINDOW

#include "qdockseparator_p.h"
#include "qdockwidgetlayout_p.h"

#include "qdockwidget.h"
#include "qmainwindow.h"
#include "qtoolbar.h"

#include <qapplication.h>
#include <qdebug.h>
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

static inline POSITION positionForArea(uint area)
{
    switch (area) {
    case Qt::LeftDockWidgetArea:   return LEFT;
    case Qt::RightDockWidgetArea:  return RIGHT;
    case Qt::TopDockWidgetArea:    return TOP;
    case Qt::BottomDockWidgetArea: return BOTTOM;
    default: break;
    }
    return CENTER;
}

#ifndef QT_NO_TOOLBAR
static inline POSITION positionForArea(Qt::ToolBarArea area)
{
    switch (area) {
    case Qt::LeftToolBarArea:   return LEFT;
    case Qt::RightToolBarArea:  return RIGHT;
    case Qt::TopToolBarArea:    return TOP;
    case Qt::BottomToolBarArea: return BOTTOM;
    default: break;
    }
    return CENTER;
}
#endif

static inline int pick(POSITION p, const QSize &s)
{ return p == TOP || p == BOTTOM ? s.height() : s.width(); }

static inline int pick(POSITION p, const QPoint &pt)
{ return p == TOP || p == BOTTOM ? pt.y() : pt.x(); }

static inline void set(POSITION p, QSize &s, int x)
{ if (p == LEFT || p == RIGHT) s.setWidth(x); else s.setHeight(x); }

static inline int pick_perp(POSITION p, const QSize &s)
{ return p == TOP || p == BOTTOM ? s.width() : s.height(); }

static inline int pick_perp(POSITION p, const QPoint &pt)
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
    : QLayout(mainwindow), statusbar(0), relayout_type(QInternal::RelayoutNormal), save_layout_info(0)
#ifndef QT_NO_TOOLBAR
      , save_tb_layout_info(0)
#endif
{
    setObjectName(mainwindow->objectName() + "_layout");

    corners[Qt::TopLeftCorner]     = Qt::TopDockWidgetArea;
    corners[Qt::TopRightCorner]    = Qt::TopDockWidgetArea;
    corners[Qt::BottomLeftCorner]  = Qt::BottomDockWidgetArea;
    corners[Qt::BottomRightCorner] = Qt::BottomDockWidgetArea;

    for (int i = 0; i < Qt::NDockWidgetAreas + 1; ++i) {
        QMainWindowLayoutInfo info;
        info.item = 0;
        info.sep = 0;
        info.is_dummy = false;
        layout_info.append(info);
    }
}

QMainWindowLayout::~QMainWindowLayout()
{
#ifndef QT_NO_TOOLBAR
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
        for (int i = 0; i < lineInfo.list.size(); ++i)
            delete lineInfo.list.at(i).item;
    }
    tb_layout_info.clear();
#endif
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

#ifndef QT_NO_STATUSBAR
QStatusBar *QMainWindowLayout::statusBar() const
{ return statusbar ? qobject_cast<QStatusBar *>(statusbar->widget()) : 0; }

void QMainWindowLayout::setStatusBar(QStatusBar *sb)
{
    if (sb)
        addChildWidget(sb);
    delete statusbar;
    statusbar = sb ? new QWidgetItem(sb) : 0;
}
#endif // QT_NO_STATUSBAR

QWidget *QMainWindowLayout::centralWidget() const
{ return layout_info[CENTER].item ? layout_info[CENTER].item->widget() : 0; }

void QMainWindowLayout::setCentralWidget(QWidget *cw)
{
    delete layout_info[CENTER].item;
    if (cw) {
        addChildWidget(cw);
        layout_info[CENTER].item = new QWidgetItem(cw);
    } else {
        layout_info[CENTER].item = 0;
    }
    layout_info[CENTER].size = QSize();
    invalidate();
}

#ifndef QT_NO_TOOLBAR
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
void QMainWindowLayout::addToolBar(Qt::ToolBarArea area,
                                   QToolBar *toolbar,
                                   bool needAddChildWidget)
{
    if (needAddChildWidget)
        addChildWidget(toolbar);
    else
        removeToolBarInfo(toolbar);

    POSITION pos = positionForArea(area);
    // see if we have an existing line in the tb - append it in the last in line
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        if (tb_layout_info.at(line).pos == pos) {
            while (line < tb_layout_info.size() - 1 && tb_layout_info.at(line + 1).pos == pos)
                ++line;

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
            lineInfo.list.append(newinfo);
            return;
        }
    }

    // no line to continue, add one and recurse
    addToolBarBreak(area);
    addToolBar(area, toolbar, false);
}

/*!
    Adds \a toolbar before \a before
*/
void QMainWindowLayout::insertToolBar(QToolBar *before, QToolBar *toolbar)
{
    addChildWidget(toolbar);

    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (info.item->widget() == before) {

                ToolBarLayoutInfo newInfo;
                newInfo.item = new QWidgetItem(toolbar);
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
    return Qt::TopToolBarArea;
}
#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET
QDockWidgetLayout *QMainWindowLayout::layoutForArea(Qt::DockWidgetArea area)
{
    POSITION pos = positionForArea(area);
    QMainWindowLayoutInfo &info = layout_info[pos];
    QDockWidgetLayout *l = 0;

    if (!info.item) {
        // create new dock window layout
        static const Qt::Orientation orientations[] = {
            Qt::Vertical,   // LEFT
            Qt::Vertical,   // RIGHT
            Qt::Horizontal, // TOP
            Qt::Horizontal, // BOTTOM
        };

        l = new QDockWidgetLayout(area, orientations[pos]);
        l->setParent(this);
        l->setObjectName(objectName() + "_dockwidgetLayout" + QString::number(area, 16));

        info.item = l;

        // create separator
        Q_ASSERT(!info.sep);
        info.sep = new QWidgetItem(new QDockSeparator(l, parentWidget()));
    } else {
        l = qobject_cast<QDockWidgetLayout *>(info.item->layout());
        Q_ASSERT(l != 0);
    }
    return l;
}

void QMainWindowLayout::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget,
                                      Qt::Orientation orientation)
{
    removeRecursive(dockwidget);

    QDockWidgetLayout * const layout = layoutForArea(area);
    layout->extend(dockwidget, orientation);
}

void QMainWindowLayout::splitDockWidget(QDockWidget *after, QDockWidget *dockwidget,
                                        Qt::Orientation orientation)
{
    removeRecursive(dockwidget);

    const Qt::DockWidgetArea area = dockWidgetArea(after);
    QDockWidgetLayout * const layout = layoutForArea(area);
    layout->split(after, dockwidget, (orientation == Qt::Horizontal
                                      ? Qt::RightDockWidgetArea
                                      : Qt::BottomDockWidgetArea));
}
#endif // QT_NO_DOCKWIDGET

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

#ifndef QT_NO_DOCKWIDGET
Qt::DockWidgetArea QMainWindowLayout::dockWidgetArea(QDockWidget *dockwidget) const
{
    for (int pos = 0; pos < NPOSITIONS - 1; ++pos) {
        if (!layout_info[pos].item)
            continue;
        if (findWidgetRecursively(layout_info[pos].item, dockwidget))
            return static_cast<Qt::DockWidgetArea>(areaForPosition(pos));
    }
    Q_ASSERT_X(false, "QMainWindow::dockWidgetArea",
               "'dockwidget' is not managed by this main window.");
    return Qt::TopDockWidgetArea;

}
#endif // QT_NO_DOCKWIDGET

void QMainWindowLayout::saveState(QDataStream &stream) const
{
#ifndef QT_NO_TOOLBAR
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
                QWidget *widget = info.item->widget();
                if (widget->objectName().isEmpty()) {
                    qWarning("QMainWindow::saveState(): 'objectName' not set for QToolBar "
                             "%p '%s', using 'windowTitle' instead",
                             widget, widget->windowTitle().toLocal8Bit().constData());
                    stream << widget->windowTitle();
                } else {
                    stream << widget->objectName();
                }
                stream << (uchar) !widget->isHidden();
                stream << info.pos;
                stream << info.size;
                stream << info.offset;
            }
        }
    }
#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET
    // save dockwidget state
    stream << (uchar) DockWidgetStateMarker;
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
        const QDockWidgetLayout * const layout =
            qobject_cast<const QDockWidgetLayout *>(layout_info[i].item->layout());
        Q_ASSERT(layout != 0);
        layout->saveState(stream);
    }
#endif // QT_NO_DOCKWIDGET

    // save center widget state
    stream << layout_info[CENTER].size;
}

bool QMainWindowLayout::restoreState(QDataStream &stream)
{
#ifndef QT_NO_TOOLBAR
    // restore toolbar layout
    uchar tmarker;
    stream >> tmarker;
    if (tmarker != ToolBarStateMarker)
        return false;

    int lines;
    stream >> lines;
    QList<ToolBarLineInfo> toolBarState;
    QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(parentWidget());
    for (int line = 0; line < lines; ++line) {
        ToolBarLineInfo lineInfo;
        stream >> lineInfo.pos;
        int size;
        stream >> size;
        for (int i = 0; i < size; ++i) {
            ToolBarLayoutInfo info;
            QString objectName;
            stream >> objectName;
            uchar shown;
            stream >> shown;
            stream >> info.pos;
            stream >> info.size;
            stream >> info.offset;

            // find toolbar
            QToolBar *toolbar = 0;
            for (int t = 0; t < toolbars.size(); ++t) {
                QToolBar *tb = toolbars.at(t);
                if (tb && tb->objectName() == objectName) {
                    toolbar = tb;
                    toolbars[t] = 0;
                    break;
                }
            }
            if (!toolbar) {
                qWarning("QMainWindow::restoreState(): cannot find a QToolBar named "
                         "'%s', trying to match using 'windowTitle' instead.",
                         objectName.toLocal8Bit().constData());
                // try matching the window title
                for (int t = 0; t < toolbars.size(); ++t) {
                    QToolBar *tb = toolbars.at(t);
                    if (tb && tb->windowTitle() == objectName) {
                        toolbar = tb;
                        break;
                    }
                }
                if (!toolbar) {
                    qWarning("QMainWindow::restoreState(): cannot find a QToolBar with "
                             "matching 'windowTitle' (looking for '%s').",
                             objectName.toLocal8Bit().constData());
                    continue;
                }
            }

            info.item = new QWidgetItem(toolbar);
            toolbar->setVisible(shown);
            toolbar->setOrientation((lineInfo.pos == LEFT  || lineInfo.pos == RIGHT)
                                    ? Qt::Vertical
                                    : Qt::Horizontal);
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
#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET
    // restore dockwidget layout
    uchar dmarker;
    stream >> dmarker;
    if (dmarker != DockWidgetStateMarker)
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
        QDockWidgetLayout * const layout =
            layoutForArea(static_cast<Qt::DockWidgetArea>(areaForPosition(pos)));
        if (!layout->restoreState(stream)) {
            stream.setStatus(QDataStream::ReadCorruptData);
            break;
        }
    }
#endif // QT_NO_DOCKWIDGET

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

#ifndef QT_NO_DOCKWIDGET
    // replace existing dockwidget layout
    for (int i = 0; i < NPOSITIONS - 1; ++i) {
        if ((*save_layout_info)[i].sep)
            delete (*save_layout_info)[i].sep->widget();
        delete (*save_layout_info)[i].sep;
        delete (*save_layout_info)[i].item;
    }
#endif // QT_NO_DOCKWIDGET

    delete save_layout_info;
    save_layout_info = 0;
    relayout_type = QInternal::RelayoutNormal;

    return true;
}


int QMainWindowLayout::count() const
{
    qWarning("QMainWindowLayout::count()");
    return 10; //#################################################
}

QLayoutItem *QMainWindowLayout::itemAt(int index) const
{
    int x = 0;
#ifndef QT_NO_TOOLBAR
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	for (int i = 0; i < lineInfo.list.size(); ++i) {
            if (x++ == index) {
                const ToolBarLayoutInfo &info = lineInfo.list.at(i);
                return info.item;
            }
        }
    }
#endif
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
#ifndef QT_NO_TOOLBAR
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
#endif
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
#ifndef QT_NO_DOCKWIDGET
    const Qt::DockWidgetArea area = static_cast<Qt::DockWidgetArea>(areaForPosition(pos));

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
#endif // QT_NO_DOCKWIDGET
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
  Returns the size hint for the first user item in a tb layout. This
  is used to contrain the minimum size a tb can have.
*/
static QSize get_item_sh(QLayout *layout)
{
    QLayoutItem *item = layout->itemAt(1);
    if (item && item->widget())
	return item->widget()->sizeHint();
    else
	return QSize(0, 0);
}

/*
  Returns the real size hint for items in a layout - including sizes
  for hidden items.
*/
static QSize get_real_sh(QLayout *layout)
{
    QSize real_sh(0, 0);
    int i = 0;
    while (layout && layout->itemAt(i))
        real_sh += layout->itemAt(i++)->widget()->sizeHint();
    --i;
    int spacing = layout->spacing();
    int margin = layout->margin();
    real_sh += QSize(spacing*i + margin*2, spacing*i + margin*2);
    return real_sh;
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

#ifndef QT_NO_TOOLBAR
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
            if (info.item->isEmpty())
                continue;
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
	tb_fill = tb_layout_info.at(0).list.at(0).item->widget()->layout()->margin() * 2
                  + QApplication::style()->pixelMetric(QStyle::PM_ToolBarHandleExtent)
		  + QApplication::style()->pixelMetric(QStyle::PM_ToolBarItemSpacing) * 2
                  + QApplication::style()->pixelMetric(QStyle::PM_ToolBarExtensionExtent);
    }
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        ToolBarLineInfo &lineInfo = tb_layout_info[line];
        int num_tbs = lineInfo.list.size();
	POSITION where = static_cast<POSITION>(lineInfo.pos);

        bool first = true;
        for (int i = 0; i < num_tbs; ++i) {
            ToolBarLayoutInfo &info = lineInfo.list[i];
            if (info.item->isEmpty())
                continue;

 	    set(where, info.size, pick(where, tb_rect[line].size()));

	    // position
 	    if (first) { // first tool bar can't have an offset
                first = false;
                int nextIndex = nextVisible(i, lineInfo);
		if (nextIndex != -1 && !info.size.isEmpty()
		    && pick_perp(where, info.offset) > pick_perp(where, info.size)) {
		    // swap if dragging it past the next one
                    ToolBarLayoutInfo &next = lineInfo.list[nextIndex];
                    next.pos = tb_rect[line].topLeft();
                    next.size.setHeight(pick_perp(where, get_item_sh(next.item->widget()->layout())) + tb_fill);
                    next.offset = QPoint();
                    if (where == LEFT || where == RIGHT)
			info.pos = QPoint(tb_rect[line].left(), next.pos.y() + next.size.height());
                    else
			info.pos = QPoint(next.pos.x() + next.size.width(), tb_rect[line].top());
                    info.offset = QPoint(); // has to be done before swapping
                    lineInfo.list.swap(i, nextIndex);
                } else {
		    info.pos = tb_rect[line].topLeft();
		    info.offset = QPoint();
		}
	    } else {
                int prevIndex = prevVisible(i, lineInfo);
                Q_ASSERT_X(prevIndex != -1, "QMainWindowLayout", "internal error");

                ToolBarLayoutInfo &prev = lineInfo.list[prevIndex];
		QSize min_size = get_item_sh(info.item->widget()->layout());
		set_perp(where, min_size, pick_perp(where, min_size) + tb_fill);
                const int cur_pt = info.size.isEmpty()
                                   ? (pick_perp(where, prev.pos) + pick_perp(where, get_real_sh(prev.item->widget()->layout())))
                                   : pick_perp(where, prev.pos) + pick_perp(where, prev.size);
 		const int prev_min = pick_perp(where, get_item_sh(prev.item->widget()->layout())) + tb_fill;
                const int snap_dist = 12;

                info.pos = tb_rect[line].topLeft();
                set_perp(where, info.pos, cur_pt + pick_perp(where, info.offset));

		if (pick_perp(where, info.offset) < 0) { // left/up motion
		    if (pick_perp(where, prev.size) + pick_perp(where, info.offset) >= prev_min) {
                        // shrink the previous one and increase size of current with same
                        QSize real_sh = get_real_sh(prev.item->widget()->layout());
                        QSize sz(0, 0);
			set_perp(where, sz, pick_perp(where, info.offset));
                        if ((pick_perp(where, prev.size) + pick_perp(where, sz)
                             < pick_perp(where, real_sh) - snap_dist)
                            || (pick_perp(where, prev.size) + pick_perp(where, sz)
                                > pick_perp(where, real_sh) + snap_dist))
                        {
                            prev.size += sz;
                            info.size -= sz;
                        } else {
                            info.pos = tb_rect[line].topLeft();
                            set_perp(where, prev.size, pick_perp(where, real_sh));
                            int pt = pick_perp(where, prev.pos) + pick_perp(where, prev.size);
                            set_perp(where, info.pos, pt);
                        }
                    } else {
			// can't shrink - push the previous one if possible
			bool can_push = false;
			for (int l = i-2; l >= 0; --l) {
			    ToolBarLayoutInfo &t = lineInfo.list[l];
			    if (pick_perp(where, t.size) + pick_perp(where, info.offset) >
				pick_perp(where, get_item_sh(t.item->widget()->layout())) + tb_fill) {
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
                            set_perp(where, prev.size, prev_min);
			    if (pick_perp(where, info.pos) < pick_perp(where, prev.pos))
				lineInfo.list.swap(i, i-1);
                            else
                                set_perp(where, info.pos, pick_perp(where, prev.pos) + prev_min);
			}
		    }

		} else if (pick_perp(where, info.offset) > 0) { // right/down motion
		    if (pick_perp(where, info.size) - pick_perp(where, info.offset) > pick_perp(where, min_size)) {
                        QSize real_sh = get_real_sh(prev.item->widget()->layout());
			QSize sz(0, 0);
			set_perp(where, sz, pick_perp(where, info.offset));
                        if ((pick_perp(where, prev.size) + pick_perp(where, sz)
                             > pick_perp(where, real_sh) + snap_dist)
                            || (pick_perp(where, prev.size) < pick_perp(where, real_sh) - snap_dist))
                        {
                            info.size -= sz;
                        } else {
                            info.pos = tb_rect[line].topLeft();
                            set_perp(where, prev.size, pick_perp(where, real_sh));
                            int pt = pick_perp(where, prev.pos) + pick_perp(where, prev.size);
                            set_perp(where, info.pos, pt);
                        }
		    } else {
			bool can_push = false;
			for (int l = i+1; l < num_tbs; ++l) {
			    ToolBarLayoutInfo &t = lineInfo.list[l];
			    if (pick_perp(where, t.size) - pick_perp(where, info.offset)
				> pick_perp(where, get_item_sh(t.item->widget()->layout())) + tb_fill) {
				QPoint pt;
				set_perp(where, pt, pick_perp(where, info.offset));
				t.pos += pt;
				t.size -= QSize(pt.x(), pt.y());
				can_push = true;
				break;
			    }
			}
			if (!can_push) {
			    int can_remove = pick_perp(where, info.size) - pick_perp(where, min_size);
			    set_perp(where, info.pos, cur_pt + can_remove);
			    QSize sz(0, 0);
			    set_perp(where, sz, can_remove);
			    info.size -= sz;

                            int nextIndex = nextVisible(i, lineInfo);
			    if (nextIndex != -1) {
                                ToolBarLayoutInfo &t = lineInfo.list[nextIndex];
                                if (pick_perp(where, info.pos) + pick_perp(where, info.offset) > pick_perp(where, t.pos))
                                    lineInfo.list.swap(i, nextIndex);
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
	    if (num_tbs == 1) {
		set_perp(where, info.size, pick_perp(where, tb_rect[line].size()));
            } else {
                int nextIndex = nextVisible(i, lineInfo);
                if (nextIndex == -1) {
                    // do a sanity check on the pos
                    if (pick_perp(where, info.pos) >= pick_perp(where, tb_rect[line].size())) {
                        int min = pick_perp(where, get_item_sh(info.item->widget()->layout())) + tb_fill;
                        set_perp(where, info.pos, pick_perp(where, tb_rect[line].size()) - min);
                    }
                    set_perp(where, info.size, pick_perp(where, tb_rect[line].size()));
                    if (where == LEFT || where == RIGHT)
                        info.size.setHeight(tb_rect[line].bottom() - info.pos.y() + 1);
                    else
                        info.size.setWidth(tb_rect[line].right() - info.pos.x() + 1);
                    if (pick_perp(where, info.size) < 1)
                        set_perp(where, info.size, pick_perp(where, get_item_sh(info.item->widget()->layout())) + tb_fill);
                }
            }

	    if (i > 0) {
		// assumes that all tbs are positioned at the correct
		// pos - fill in the gaps btw them
                int prevIndex = prevVisible(i, lineInfo);
                if (prevIndex != -1) {
                    ToolBarLayoutInfo &prev = lineInfo.list[prevIndex];
                    set_perp(where, prev.size, pick_perp(where, info.pos) - pick_perp(where, prev.pos));
                }
            }
	}

	for (int i = 0; i < num_tbs; ++i) {
	    ToolBarLayoutInfo &info = lineInfo.list[i];
            if (info.item->isEmpty()) {
                info.size = QSize();
                continue;
            }

	    QRect tb(info.pos, info.size);
	    tb = QStyle::visualRect(QApplication::layoutDirection(), tb_rect[line], tb);
	    if (!tb.isEmpty() && relayout_type == QInternal::RelayoutNormal)
		info.item->setGeometry(tb);
	}
    }
#endif // QT_NO_TOOLBAR

    // layout dockwidgets and center widget
    const int ext = QApplication::style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);

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
    case Qt::TopDockWidgetArea:
        top.setLeft(r.left());
        left.setTop(top.bottom() + 1);
        break;
    case Qt::LeftDockWidgetArea:
        left.setTop(r.top());
        top.setLeft(left.right() + 1);
        break;
    default:
        Q_ASSERT(false);
    }

    switch (corners[Qt::BottomLeftCorner]) {
    case Qt::BottomDockWidgetArea:
        bottom.setLeft(r.left());
        left.setBottom(bottom.top() - 1);
        break;
    case Qt::LeftDockWidgetArea:
        left.setBottom(r.bottom());
        bottom.setLeft(left.right() + 1);
        break;
    default:
        Q_ASSERT(false);
    }

    switch (corners[Qt::TopRightCorner]) {
    case Qt::TopDockWidgetArea:
        top.setRight(r.right());
        right.setTop(top.bottom() + 1);
        break;
    case Qt::RightDockWidgetArea:
        right.setTop(r.top());
        top.setRight(right.left() - 1);
        break;
    default:
        Q_ASSERT(false);
    }

    switch (corners[Qt::BottomRightCorner]) {
    case Qt::BottomDockWidgetArea:
        bottom.setRight(r.right());
        right.setBottom(bottom.top() - 1);
        break;
    case Qt::RightDockWidgetArea:
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
    if (!szHint.isValid()) {
        int left = 0, right = 0, top = 0, bottom = 0;

#ifndef QT_NO_TOOLBAR
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
#endif // QT_NO_TOOLBAR

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

        w1 = (corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea ? szL.width() : 0)
             + szT.width()
             + (corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea ? szR.width() : 0);
        w2 = szL.width() + szR.width() + szC.width();
        w3 = (corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea ? szL.width() : 0)
             + szB.width()
             + (corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea ? szR.width(): 0);

        h1 = (corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea ? szT.height() : 0)
             + szL.height()
             + (corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea ? szB.height(): 0);
        h2 = szT.height() + szB.height() + szC.height();
        h3 = (corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea ? szT.height() : 0)
             + szR.height()
             + (corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea ? szB.height() : 0);

        const int ext = QApplication::style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);
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
        szHint = QSize(qMax(szSB.width(), qMax(qMax(w1,w2),w3) + left + right),
                       szSB.height() + qMax(qMax(h1,h2),h3) + top + bottom);
    }
    return szHint;
}

QSize QMainWindowLayout::minimumSize() const
{
    if (!minSize.isValid()) {
        int left = 0, right = 0, top = 0, bottom = 0;

#ifndef QT_NO_TOOLBAR
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
#endif // QT_NO_TOOLBAR

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

        w1 = (corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea ? szL.width() : 0)
             + szT.width()
             + (corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea ? szR.width() : 0);
        w2 = szL.width() + szR.width() + szC.width();
        w3 = (corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea ? szL.width() : 0)
             + szB.width()
             + (corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea ? szR.width() : 0);

        h1 = (corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea ? szT.height() : 0)
             + szL.height()
             + (corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea ? szB.height() : 0);
        h2 = szT.height() + szB.height() + szC.height();
        h3 = (corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea ? szT.height() : 0)
             + szR.height()
             + (corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea ? szB.height() : 0);

        const int ext = QApplication::style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);
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
        minSize =  QSize(qMax(szSB.width(), qMax(qMax(w1,w2),w3) + left + right),
                         szSB.height() + qMax(qMax(h1,h2),h3) + top + bottom);
    }
    return minSize;
}

void QMainWindowLayout::relayout(QInternal::RelayoutType type)
{
    QRect g = geometry();
    if (g.isValid()) {
        QInternal::RelayoutType save_type = relayout_type;
        relayout_type = type;
        setGeometry(g);
        relayout_type = save_type;
    } else {
        update();
    }
}

void QMainWindowLayout::invalidate()
{
    if (relayout_type != QInternal::RelayoutDragging) {
        QLayout::invalidate();
        minSize = szHint = QSize();
    }
}

void QMainWindowLayout::saveLayoutInfo()
{
    Q_ASSERT(save_layout_info == 0);
    save_layout_info = new QVector<QMainWindowLayoutInfo>(layout_info);
    relayout_type = QInternal::RelayoutDragging;

    for (int i = 0; i < 4; ++i) {
        if (!layout_info[i].item) continue;

        QDockWidgetLayout *layout =
            qobject_cast<QDockWidgetLayout *>(layout_info[i].item->layout());
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

        QDockWidgetLayout *layout =
            qobject_cast<QDockWidgetLayout *>(layout_info[i].item->layout());
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

        QDockWidgetLayout *layout =
            qobject_cast<QDockWidgetLayout *>(layout_info[i].item->layout());
        Q_ASSERT(layout != 0);
        layout->discardLayoutInfo();
    }
}

void QMainWindowLayout::beginConstrain()
{
#ifndef QT_NO_TOOLBAR
    save_tb_layout_info = new QList<ToolBarLineInfo>(tb_layout_info);
#endif
}

void QMainWindowLayout:: endConstrain()
{
#ifndef QT_NO_TOOLBAR
    delete save_tb_layout_info;
    save_tb_layout_info = 0;
#endif
}

int QMainWindowLayout::constrain(QDockWidgetLayout *dock, int delta)
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

    const int _ext = QApplication::style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);
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
        QSize cmin, cmax;
        if (info[CENTER].item) {
            cmin = info[CENTER].item->minimumSize();
            cmax = info[CENTER].item->maximumSize();
        } else {
            cmin = QSize(0, 0);
            cmax = QSize(INT_MAX, INT_MAX);
        }
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

static
Qt::DockWidgetAreas areasForMousePosition(const QRect &r, const QPoint &p, bool floatable = false)
{
    const int dl = qAbs(r.left()   - p.x()),
              dr = qAbs(r.right()  - p.x()),
              dt = qAbs(r.top()    - p.y()),
              db = qAbs(r.bottom() - p.y()),
           delta = qMin(dl, qMin(dr, qMin(dt, db))),
       threshold = 50,
              dx = qAbs(r.center().x() - p.x()),
              dy = qAbs(r.center().y() - p.y());

    Qt::DockWidgetAreas areas = 0;
    if (delta < threshold) {
        VDEBUG() << "    below threshold";
        if (delta == dl || delta == dr) {
            if (delta == dl)
                areas = Qt::LeftDockWidgetArea;
            else
                areas = Qt::RightDockWidgetArea;

            if (dt < threshold)
                areas |= Qt::TopDockWidgetArea;
            else if (db < threshold)
                areas |= Qt::BottomDockWidgetArea;
        } else if (delta == dt || delta == db) {
            if (delta == dt)
                areas = Qt::TopDockWidgetArea;
            else
                areas = Qt::BottomDockWidgetArea;

            if (dl < threshold)
                areas |= Qt::LeftDockWidgetArea;
            else if (dr < threshold)
                areas |= Qt::RightDockWidgetArea;
        }
    } else if (!floatable) {
        VDEBUG() << "    not floatable";
        areas = ((dx > dy)
                 ? ((p.x() < r.center().x())
                    ? Qt::LeftDockWidgetArea
                    : Qt::RightDockWidgetArea)
                 : ((p.y() < r.center().y())
                    ? Qt::TopDockWidgetArea
                    : Qt::BottomDockWidgetArea));
    } else {
        VDEBUG() << "    floatable";
    }

    return areas;
}

#ifndef QT_NO_DOCKWIDGET
Qt::DockWidgetArea QMainWindowLayout::locateDockWidget(QDockWidget *dockwidget,
                                                       const QPoint &mouse) const
{
    VDEBUG() << "  locate: mouse" << mouse;

    const QPoint p = parentWidget()->mapFromGlobal(mouse);

    /*
      if there is a window dock layout under the mouse, forward the
      place request
    */
    for (int i = 0; i < 4; ++i) {
        if (!layout_info[i].item)
            continue;
        const Qt::DockWidgetArea area = static_cast<Qt::DockWidgetArea>(areaForPosition(i));
        if (!dockwidget->isAreaAllowed(area))
            continue;
        if (layout_info[i].item->isEmpty() ||
            (!layout_info[i].item->geometry().contains(p)
             && !layout_info[i].sep->geometry().contains(p)))
            continue;
        VDEBUG() << "  result: mouse over item" << i;
        return area;
    }

    Qt::DockWidgetAreas areas =
        areasForMousePosition((layout_info[CENTER].item
                               ? layout_info[CENTER].item->geometry()
                               : QRect(QPoint(0, 0), parentWidget()->size())),
                              p,
                              (dockwidget->features() & QDockWidget::DockWidgetFloatable));
    Qt::DockWidgetArea area;

    if (areas == (Qt::LeftDockWidgetArea | Qt::TopDockWidgetArea))
        area = corners[Qt::TopLeftCorner];
    else if (areas == (Qt::LeftDockWidgetArea | Qt::BottomDockWidgetArea))
        area = corners[Qt::BottomLeftCorner];
    else if (areas == (Qt::RightDockWidgetArea | Qt::TopDockWidgetArea))
        area = corners[Qt::TopRightCorner];
    else if (areas == (Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea))
        area = corners[Qt::BottomRightCorner];
    else
        area = (Qt::DockWidgetArea)(uint)areas;

    VDEBUG() << "  result:" << area;
    return area;
}

QRect QMainWindowLayout::placeDockWidget(QDockWidget *dockwidget,
                                         const QRect &r,
                                         const QPoint &mouse)
{
    DEBUG("QMainWindowLayout::placeDockWidget");

    Qt::DockWidgetArea area = locateDockWidget(dockwidget, mouse);
    QRect target;

    if (!area || !dockwidget->isAreaAllowed(area)) {
        DEBUG() << "END of QMainWindowLayout::placeDockWidget (failed to place)";
        return target;
    }

    // if there is a window dock layout already here, forward the place
    const int pos = positionForArea(area);
    if (layout_info[pos].item && !layout_info[pos].item->isEmpty()) {
        DEBUG("  forwarding...");
        QDockWidgetLayout *l = qobject_cast<QDockWidgetLayout *>(layout_info[pos].item->layout());
        Q_ASSERT(l != 0);
        target = l->place(dockwidget, r, mouse);
        DEBUG("END of QMainWindowLayout::placeDockWidget (forwarded)");
        return target;
    }

    // remove dockwidget from current position in the layout
    removeRecursive(dockwidget);

    // see if the tool window will fix in the main window
    const QSize cur = parentWidget()->size();

    QMainWindowLayoutItem layoutitem(dockwidget, r);
    layout_info[pos].item = &layoutitem;
    layout_info[pos].size = r.size();
    DEBUG() << "  pos" << pos << " size" << layout_info[pos].size;
    layout_info[pos].is_dummy = true;

    relayout(QInternal::RelayoutDragging);

    QSize currentMinSize = minSize;
    // save minSize, force minimumSize() to recalculate, and restore
    // the saved value
    minSize = QSize();
    const QSize new_min = minimumSize();
    const bool forbid = cur.width() < new_min.width() || cur.height() < new_min.height();
    minSize = currentMinSize;

    if (!forbid) {
        DEBUG() << "  placed at " << layoutitem.geometry();
        target = layoutitem.geometry();
        target.moveTopLeft(parentWidget()->mapToGlobal(target.topLeft()));
    } else {
        DEBUG() << "  forbidden, minimum size" << new_min << " larger than current size" << cur;
    }

    DEBUG() << "END of QMainWindowLayout::placeDockWidget, target" << target;

    return target;
}

void QMainWindowLayout::dropDockWidget(QDockWidget *dockwidget,
                                        const QRect &r,
                                        const QPoint &mouse)
{
    DEBUG("QMainWindowLayout::dropDockWidget");

    Qt::DockWidgetArea area = locateDockWidget(dockwidget, mouse);

    if (!area || !dockwidget->isAreaAllowed(area)) {
        DEBUG() << "END of QMainWindowLayout::dropDockWidget (failed to place)";
        return;
    }

    // if there is a window dock layout already here, forward the drop
    const int pos = positionForArea(area);
    if (layout_info[pos].item && !layout_info[pos].item->isEmpty()) {
        DEBUG() << "  forwarding...";
        QDockWidgetLayout *l = qobject_cast<QDockWidgetLayout *>(layout_info[pos].item->layout());
        Q_ASSERT(l);
        l->drop(dockwidget, r, mouse);
        relayout();
        DEBUG() << "END of QMainWindowLayout::dropDockWidget (forwarded)";
        return;
    }

    // remove dockwidget from current position in the layout
    removeRecursive(dockwidget);

    QDockWidgetLayout *dwl = layoutForArea(static_cast<Qt::DockWidgetArea>(areaForPosition(pos)));
    dwl->addWidget(dockwidget);
    layout_info[pos].size = r.size();
    relayout();

    if (dockwidget->isFloating()) {
        // reparent the dock window into the main window
        dockwidget->setFloating(false);
        dockwidget->show();
    }
    layout_info[pos].sep->widget()->show();

    DEBUG() << "END of QMainWindowLayout::dropDockWidget";
}
#endif // QT_NO_DOCKWIDGET

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

#ifndef QT_NO_DOCKWIDGET
void QMainWindowLayout::removeRecursive(QDockWidget *dockwidget)
{
    removeWidgetRecursively(this, dockwidget, save_layout_info != 0);
}
#endif // QT_NO_DOCKWIDGET

#ifndef QT_NO_TOOLBAR
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
        if (p.x() < p2.x() && toolbar->isAreaAllowed(Qt::LeftToolBarArea)) {
            pos = LEFT;
        } else if (p.x() >= p2.x() && toolbar->isAreaAllowed(Qt::RightToolBarArea)) {
            pos = RIGHT;
        } else {
            if (p.y() < p2.y() && toolbar->isAreaAllowed(Qt::TopToolBarArea))
                pos = TOP;
            else if (p.y() >= p2.y() && toolbar->isAreaAllowed(Qt::BottomToolBarArea))
                pos = BOTTOM;
        }
    } else {
        if (p.y() < p2.y() && toolbar->isAreaAllowed(Qt::TopToolBarArea)) {
                pos = TOP;
        } else if (p.y() >= p2.y() && toolbar->isAreaAllowed(Qt::BottomToolBarArea)) {
                pos = BOTTOM;
        } else {
            if (p.x() < p2.x() && toolbar->isAreaAllowed(Qt::LeftToolBarArea))
                pos = LEFT;
            else if (p.x() >= p2.x() && toolbar->isAreaAllowed(Qt::RightToolBarArea))
                pos = RIGHT;
        }
    }
    Q_ASSERT(pos != CENTER);

    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
        if (!toolbar->isAreaAllowed(static_cast<Qt::ToolBarArea>(areaForPosition(lineInfo.pos))))
            continue;
	bool break_it = false;
        for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (!info.item)
                continue;
 	    if (info.item->isEmpty())
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
	    TBDEBUG() << "left/up" << offset << "line: " << l << where;
	    if (l > 0 && tb_layout_info.at(l-1).pos == where) { // is this the first line in this tb area?
                tb_layout_info[l].list.removeAt(i);
                if (tb_layout_info[l].list.size() == 0)
                    tb_layout_info.removeAt(l);
                if (tb_layout_info.at(l-1).pos == where) {
                    TBDEBUG() << "1. appending to existing" << info.item->widget() << info.item->widget()->geometry();
                    if (tb_layout_info[l-1].list.size() > 0) {
                        const ToolBarLayoutInfo &tmp = tb_layout_info.at(l-1).list.last();
                        if (pick_perp(where, tmp.pos) == pick_perp(where, info.pos))
                            info.offset = info.pos - (tmp.pos + QPoint(tmp.size.width(), tmp.size.height()) - offset);
                        else if (pick_perp(where, tmp.pos) < pick_perp(where, info.pos))
                            info.offset = -(tmp.pos + QPoint(tmp.size.width(), tmp.size.height()) - info.pos + offset);
                    }
                    tb_layout_info[l-1].list.append(info);

                } else {
                    ToolBarLineInfo line;
                    line.pos = where;
                    line.list.append(info);
                    tb_layout_info.insert(l, line);
                    TBDEBUG() << "2. inserting new";
                }
            } else if ((tb_layout_info.at(l).list.size() > 1)
                       && ((l > 0 && tb_layout_info.at(l-1).pos == where)
                           || where == BOTTOM || where == RIGHT))
            {
                tb_layout_info[l].list.removeAt(i);
                ToolBarLineInfo line;
                line.pos = where;
                line.list.append(info);
                tb_layout_info.insert(l, line);
                TBDEBUG() << "3. inserting new" << l << toolbar;
            }
        } else if (pick(where, offset) > pick(where, info.size) + magic_offset) { // move right/down
            TBDEBUG() << "right/down" << offset << "line: " << l;
            if (l < tb_layout_info.size()-1 && tb_layout_info.at(l+1).pos == where) {
                tb_layout_info[l].list.removeAt(i);
                if (tb_layout_info.at(l).list.size() == 0)
                    tb_layout_info.removeAt(l--);
                if (tb_layout_info.at(l+1).pos == where) {
                    if (tb_layout_info[l+1].list.size() > 0) {
                        const ToolBarLayoutInfo &tmp = tb_layout_info.at(l+1).list.last();
                        if (pick_perp(where, tmp.pos) == pick_perp(where, info.pos))
                            info.offset = info.pos -(tmp.pos + QPoint(tmp.size.width(), tmp.size.height()) - offset);
                        else if (pick_perp(where, tmp.pos) < pick_perp(where, info.pos))
                            info.offset = -(tmp.pos + QPoint(tmp.size.width(), tmp.size.height()) - info.pos + offset);
                    }
                    tb_layout_info[l+1].list.append(info);
                    TBDEBUG() << "1. appending to exisitng";
                } else {
                    ToolBarLineInfo line;
                    line.pos = where;
                    line.list.append(info);
                    tb_layout_info.insert(l, line);
                    TBDEBUG() << "2. inserting new line";
                }
            } else if ((tb_layout_info.at(l).list.size() > 1)
                       && ((l < tb_layout_info.size()-1 && tb_layout_info.at(l+1).pos == where)
                           || where == TOP || where == LEFT))
            {
                tb_layout_info[l].list.removeAt(i);
                ToolBarLineInfo line;
                line.pos = where;
                line.list.append(info);
                tb_layout_info.insert(l+1, line);
                TBDEBUG() << "3. inserting new line";
            }
        }
    } else {
        TBDEBUG() << "changed area";
        addToolBar(static_cast<Qt::ToolBarArea>(areaForPosition(where)), toolbar, false);
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

int QMainWindowLayout::nextVisible(int index, const ToolBarLineInfo &lineInfo)
{
    for (++index; index < lineInfo.list.size(); ++index) {
        if (!lineInfo.list.at(index).item->isEmpty())
            break;
    }
    return (index >= 0 && index < lineInfo.list.size()) ? index : -1;
}

int QMainWindowLayout::prevVisible(int index, const ToolBarLineInfo &lineInfo)
{
    for (--index; index >= 0; --index) {
        if (!lineInfo.list.at(index).item->isEmpty())
            break;
    }
    return (index >= 0 && index < lineInfo.list.size()) ? index : -1;
}
#endif // QT_NO_TOOLBAR

#endif // QT_NO_MAINWINDOW
