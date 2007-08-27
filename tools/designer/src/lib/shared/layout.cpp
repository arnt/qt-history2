/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "layout_p.h"
#include "qdesigner_utils_p.h"
#include "qlayout_widget_p.h"
#include "spacer_widget_p.h"
#include "layoutdecoration.h"
#include "widgetfactory_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerWidgetDataBaseInterface>
#include <QtDesigner/QDesignerMetaDataBaseInterface>

#include <QtCore/qdebug.h>
#include <QtCore/QVector>

#include <QtGui/qevent.h>
#include <QtGui/QGridLayout>
#include <QtGui/QPainter>
#include <QtGui/QBitmap>
#include <QtGui/QSplitter>
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#include <QtGui/QScrollArea>

namespace qdesigner_internal {
/*!
  \class Layout layout.h
  \brief Baseclass for layouting widgets in the Designer (Helper for Layout commands)
  \internal

  Classes derived from this abstract base class are used for layouting
  operations in the Designer (creating/breaking layouts).

  Instances live in the Layout/BreakLayout commands.
*/

/*!  \a p specifies the parent of the layoutBase \a lb. The parent
  might be changed in setup(). If the layoutBase is a
  container, the parent and the layoutBase are the same. Also they
  always have to be a widget known to the designer (e.g. in the case
  of the tabwidget parent and layoutBase are the tabwidget and not the
  page which actually gets laid out. For actual usage the correct
  widget is found later by Layout.)
 */

Layout::Layout(const QWidgetList &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, LayoutInfo::Type layoutType) :
    m_widgets(wl),
    m_parentWidget(p),
    m_layoutBase(lb),
    m_formWindow(fw),
    m_layoutType(layoutType),
    m_isBreak(false)
{
    if (m_layoutBase)
        m_oldGeometry = m_layoutBase->geometry();
}

Layout::~Layout()
{
}

/*!  The widget list we got in the constructor might contain too much
  widgets (like widgets with different parents, already laid out
  widgets, etc.). Here we set up the list and so the only the "best"
  widgets get laid out.
*/

void Layout::setup()
{
    m_startPoint = QPoint(32767, 32767);

    // Go through all widgets of the list we got. As we can only
    // layout widgets which have the same parent, we first do some
    // sorting which means create a list for each parent containing
    // its child here. After that we keep working on the list of
    // children which has the most entries.
    // Widgets which are already laid out are thrown away here too

    QMultiMap<QWidget*, QWidget*> lists;
    foreach (QWidget *w, m_widgets) {
        QWidget *p = w->parentWidget();

        if (p && LayoutInfo::layoutType(m_formWindow->core(), p) != LayoutInfo::NoLayout
                && m_formWindow->core()->metaDataBase()->item(p->layout()) != 0)
            continue;

        lists.insert(p, w);
    }

    QWidgetList lastList;
    QWidgetList parents = lists.keys();
    foreach (QWidget *p, parents) {
        QWidgetList children = lists.values(p);

        if (children.count() > lastList.count())
            lastList = children;
    }


    // If we found no list (because no widget did fit at all) or the
    // best list has only one entry and we do not layout a container,
    // we leave here.
    QDesignerWidgetDataBaseInterface *widgetDataBase = m_formWindow->core()->widgetDataBase();
    if (lastList.count() < 2 &&
                        (!m_layoutBase ||
                          (!widgetDataBase->isContainer(m_layoutBase, false) &&
                            m_layoutBase != m_formWindow->mainContainer()))
                       ) {
        m_widgets.clear();
        m_startPoint = QPoint(0, 0);
        return;
    }

    // Now we have a new and clean widget list, which makes sense
    // to layout
    m_widgets = lastList;
    // Also use the only correct parent later, so store it

    Q_ASSERT(m_widgets.isEmpty() == false);

    m_parentWidget = m_formWindow->core()->widgetFactory()->widgetOfContainer(m_widgets.first()->parentWidget());
    // Now calculate the position where the layout-meta-widget should
    // be placed and connect to widgetDestroyed() signals of the
    // widgets to get informed if one gets deleted to be able to
    // handle that and do not crash in this case
    foreach (QWidget *w, m_widgets) {
        connect(w, SIGNAL(destroyed()), this, SLOT(widgetDestroyed()));
        m_startPoint = QPoint(qMin(m_startPoint.x(), w->x()), qMin(m_startPoint.y(), w->y()));
        const QRect rc(w->geometry());

        m_geometries.insert(w, rc);
        // Change the Z-order, as saving/loading uses the Z-order for
        // writing/creating widgets and this has to be the same as in
        // the layout. Else saving + loading will give different results
        w->raise();
    }

    sort();
}

void Layout::widgetDestroyed()
{
    if (QWidget *w = qobject_cast<QWidget *>(sender())) {
        m_widgets.removeAt(m_widgets.indexOf(w));
        m_geometries.remove(w);
    }
}

bool Layout::prepareLayout(bool &needMove, bool &needReparent)
{
    foreach (QWidget *widget, m_widgets) {
        widget->raise();
    }

    needMove = !m_layoutBase;
    needReparent = needMove || qobject_cast<QLayoutWidget*>(m_layoutBase) || qobject_cast<QSplitter*>(m_layoutBase);

    QDesignerWidgetFactoryInterface *widgetFactory = m_formWindow->core()->widgetFactory();
    QDesignerMetaDataBaseInterface *metaDataBase = m_formWindow->core()->metaDataBase();

    if (m_layoutBase == 0) {
        const bool useSplitter = m_layoutType == LayoutInfo::HSplitter || m_layoutType == LayoutInfo::VSplitter;
        const QString baseWidgetClassName = useSplitter ? QLatin1String("QSplitter") : QLatin1String("QLayoutWidget");
        m_layoutBase = widgetFactory->createWidget(baseWidgetClassName, widgetFactory->containerOfWidget(m_parentWidget));
        if (useSplitter) {
            m_layoutBase->setObjectName(QLatin1String("splitter"));
            m_formWindow->ensureUniqueObjectName(m_layoutBase);
        }
    } else {
        LayoutInfo::deleteLayout(m_formWindow->core(), m_layoutBase);
    }

    metaDataBase->add(m_layoutBase);

    Q_ASSERT(m_layoutBase->layout() == 0 || metaDataBase->item(m_layoutBase->layout()) == 0);

    return true;
}

static bool isMainContainer(QDesignerFormWindowInterface *fw, const QWidget *w)
{
    return w && (w == fw || w == fw->mainContainer());
}

static bool isPageOfContainerWidget(QDesignerFormWindowInterface *fw, QWidget *widget)
{
    QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension*>(
            fw->core()->extensionManager(), widget->parentWidget());

    if (c != 0) {
        for (int i = 0; i<c->count(); ++i) {
            if (widget == c->widget(i))
                return true;
        }
    }

    return false;
}
void Layout::finishLayout(bool needMove, QLayout *layout)
{
    if (m_parentWidget == m_layoutBase) {
        QWidget *widget = m_layoutBase;
        m_oldGeometry = widget->geometry();

        bool done = false;
        while (!isMainContainer(m_formWindow, widget) && !done) {
            if (!m_formWindow->isManaged(widget)) {
                widget = widget->parentWidget();
                continue;
            } else if (LayoutInfo::isWidgetLaidout(m_formWindow->core(), widget)) {
                widget = widget->parentWidget();
                continue;
            } else if (isPageOfContainerWidget(m_formWindow, widget)) {
                widget = widget->parentWidget();
                continue;
            } else if (widget->parentWidget()) {
                QScrollArea *area = qobject_cast<QScrollArea*>(widget->parentWidget()->parentWidget());
                if (area && area->widget() == widget) {
                    widget = area;
                    continue;
                }
            }

            done = true;
        }

        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        // We don't want to resize the form window
        if (!Utils::isCentralWidget(m_formWindow, widget))
            widget->adjustSize();

        return;
    }

    if (needMove)
        m_layoutBase->move(m_startPoint);

    const QRect g(m_layoutBase->pos(), m_layoutBase->size());

    if (LayoutInfo::layoutType(m_formWindow->core(), m_layoutBase->parentWidget()) == LayoutInfo::NoLayout && !m_isBreak)
        m_layoutBase->adjustSize();
    else if (m_isBreak)
        m_layoutBase->setGeometry(m_oldGeometry);

    m_oldGeometry = g;
    if (layout)
        layout->invalidate();
    m_layoutBase->show();

    if (qobject_cast<QLayoutWidget*>(m_layoutBase) || qobject_cast<QSplitter*>(m_layoutBase)) {
        m_formWindow->manageWidget(m_layoutBase);
        m_formWindow->selectWidget(m_layoutBase);
    }
}

void Layout::undoLayout()
{
    if (!m_widgets.count())
        return;

    m_formWindow->selectWidget(m_layoutBase, false);

    QDesignerWidgetFactoryInterface *widgetFactory = m_formWindow->core()->widgetFactory();
    QHashIterator<QWidget *, QRect> it(m_geometries);
    while (it.hasNext()) {
        it.next();

        if (!it.key())
            continue;

        QWidget* w = it.key();
        const QRect rc = it.value();

        const bool showIt = w->isVisibleTo(m_formWindow);
        QWidget *container = widgetFactory->containerOfWidget(m_parentWidget);

        // ### remove widget here
        QWidget *parentWidget = w->parentWidget();
        QDesignerFormEditorInterface *core = m_formWindow->core();
        QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parentWidget);

        if (deco)
            deco->removeWidget(w);

        w->setParent(container);
        w->setGeometry(rc);

        if (showIt)
            w->show();
    }

    LayoutInfo::deleteLayout(m_formWindow->core(), m_layoutBase);

    if (m_parentWidget != m_layoutBase && !qobject_cast<QMainWindow*>(m_layoutBase)) {
        m_formWindow->unmanageWidget(m_layoutBase);
        m_layoutBase->hide();
    } else {
        QMainWindow *mw = qobject_cast<QMainWindow*>(m_formWindow->mainContainer());
        if (m_layoutBase != m_formWindow->mainContainer() &&
                    (!mw || mw->centralWidget() != m_layoutBase))
            m_layoutBase->setGeometry(m_oldGeometry);
    }

    QWidget *ww = m_widgets.size() ? m_widgets.front() : m_formWindow;
    m_formWindow->selectWidget(ww);
}

void Layout::breakLayout()
{
    QMap<QWidget *, QRect> rects;
    foreach (QWidget *w, m_widgets) {
        rects.insert(w, w->geometry());
    }
    const QPoint m_layoutBasePos = m_layoutBase->pos();
    QDesignerWidgetDataBaseInterface *widgetDataBase = m_formWindow->core()->widgetDataBase();

    LayoutInfo::deleteLayout(m_formWindow->core(), m_layoutBase);

    const bool needReparent = qobject_cast<QLayoutWidget*>(m_layoutBase) ||
                        qobject_cast<QSplitter*>(m_layoutBase)     ||
                        (!widgetDataBase->isContainer(m_layoutBase, false) &&
                          m_layoutBase != m_formWindow->mainContainer());
    const bool needResize = qobject_cast<QSplitter*>(m_layoutBase);
    const bool add = m_geometries.isEmpty();

    QMapIterator<QWidget*, QRect> it(rects);
    while (it.hasNext()) {
        it.next();

        QWidget *w = it.key();
        if (needReparent) {
            w->setParent(m_layoutBase->parentWidget(), 0);
            w->move(m_layoutBasePos + it.value().topLeft());
            w->show();
        }

        if (needResize)
            w->resize(it.value().size());

        if (add)
            m_geometries.insert(w, QRect(w->pos(), w->size()));
    }

    if (needReparent) {
        m_layoutBase->hide();
        m_parentWidget = m_layoutBase->parentWidget();
        m_formWindow->unmanageWidget(m_layoutBase);
    } else {
        m_parentWidget = m_layoutBase;
    }

    if (!m_widgets.isEmpty() && m_widgets.first() && m_widgets.first()->isVisibleTo(m_formWindow))
        m_formWindow->selectWidget(m_widgets.first());
    else
        m_formWindow->selectWidget(m_formWindow);
}

static QString suggestLayoutName(const char *className)
{
    // Legacy
    if (!qstrcmp(className, "QHBoxLayout"))
        return QLatin1String("horizontalLayout");
    if (!qstrcmp(className, "QVBoxLayout"))
        return QLatin1String("verticalLayout");
    if (!qstrcmp(className, "QGridLayout"))
        return QLatin1String("gridLayout");

    QString name = QString::fromUtf8(className);
    qtify(name);
    return name;
}
QLayout *Layout::createLayout(int type)
{
    Q_ASSERT(m_layoutType != LayoutInfo::HSplitter && m_layoutType != LayoutInfo::VSplitter);
    QLayout *layout = m_formWindow->core()->widgetFactory()->createLayout(m_layoutBase, 0, type);
    // set a name
    layout->setObjectName(suggestLayoutName(layout->metaObject()->className()));
    m_formWindow->ensureUniqueObjectName(layout);
    // QLayoutWidget
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_formWindow->core()->extensionManager(), layout);
    if (sheet && qobject_cast<QLayoutWidget*>(m_layoutBase)) {
        sheet->setProperty(sheet->indexOf(QLatin1String("leftMargin")), 0);
        sheet->setProperty(sheet->indexOf(QLatin1String("topMargin")), 0);
        sheet->setProperty(sheet->indexOf(QLatin1String("rightMargin")), 0);
        sheet->setProperty(sheet->indexOf(QLatin1String("bottomMargin")), 0);
    }
    return layout;
}

void Layout::reparentToLayoutBase(QWidget *w)
{
    if (w->parent() != m_layoutBase) {
        w->setParent(m_layoutBase, 0);
        w->move(QPoint(0,0));
    }
}

namespace { // within qdesigner_internal

// ----- PositionSortPredicate: Predicate to be usable as LessThan function to sort widgets by position
class PositionSortPredicate {
public:
    PositionSortPredicate(Qt::Orientation orientation) : m_orientation(orientation) {}
    bool operator()(const QWidget* w1, const QWidget* w2) {
        return m_orientation == Qt::Horizontal ? w1->x() < w2->x() : w1->y() < w2->y();
    }
    private:
    const Qt::Orientation m_orientation;
};

// -------- BoxLayout
class BoxLayout : public Layout
{
public:
    BoxLayout(const QWidgetList &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb,
              Qt::Orientation orientation);

    virtual void doLayout();
    virtual void sort();

private:
    const Qt::Orientation m_orientation;
};

BoxLayout::BoxLayout(const QWidgetList &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb,
                     Qt::Orientation orientation)  :
    Layout(wl, p, fw, lb, orientation == Qt::Horizontal ? LayoutInfo::HBox : LayoutInfo::VBox),
    m_orientation(orientation)
{
}

void BoxLayout::sort()
{
    QWidgetList wl = widgets();
    qStableSort(wl.begin(), wl.end(), PositionSortPredicate(m_orientation));
    setWidgets(wl);
}

void BoxLayout::doLayout()
{
    bool needMove, needReparent;
    if (!prepareLayout(needMove, needReparent))
        return;

    QBoxLayout *layout = static_cast<QBoxLayout *>(createLayout(m_orientation == Qt::Horizontal ? LayoutInfo::HBox : LayoutInfo::VBox));

    const QWidgetList &wl = widgets();
    foreach (QWidget *w, wl) {
        if (needReparent)
            reparentToLayoutBase(w);

        if (const Spacer *spacer = qobject_cast<const Spacer*>(w))
            layout->addWidget(w, 0, spacer->alignment());
        else
            layout->addWidget(w);
        w->show();
    }
    finishLayout(needMove, layout);
}

// --------  SplitterLayout
class SplitterLayout : public Layout
{
public:
    SplitterLayout(const QWidgetList &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb,
                   Qt::Orientation orientation);

    virtual void doLayout();
    virtual void sort();

private:
    const Qt::Orientation m_orientation;
};

SplitterLayout::SplitterLayout(const QWidgetList &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb,
                               Qt::Orientation orientation) :
    Layout(wl, p, fw, lb, orientation == Qt::Horizontal ? LayoutInfo::HSplitter : LayoutInfo::VSplitter),
    m_orientation(orientation)
{
}

void SplitterLayout::sort()
{
    QWidgetList wl = widgets();
    qStableSort(wl.begin(), wl.end(), PositionSortPredicate(m_orientation));
    setWidgets(wl);
}

void SplitterLayout::doLayout()
{
    bool needMove, needReparent;
    if (!prepareLayout(needMove, needReparent))
        return;

    QSplitter *splitter = qobject_cast<QSplitter*>(layoutBaseWidget());
    Q_ASSERT(splitter != 0);

    const QWidgetList &wl = widgets();
    foreach (QWidget *w, wl) {
        if (needReparent)
            reparentToLayoutBase(w);
        splitter->addWidget(w);
        w->show();
    }

    splitter->setOrientation(m_orientation);
    finishLayout(needMove);
}

//  ---------- Grid: Helper for laying out grids
class Grid
{
public:
    Grid(int rows, int cols);
    ~Grid();

    QWidget* cell(int row, int col) const { return m_cells[ row * m_ncols + col]; }
    void setCell(int row, int col, QWidget* w) { m_cells[ row * m_ncols + col] = w; }
    void setCells(QRect c, QWidget* w) {
        for (int rows = c.bottom()-c.top(); rows >= 0; rows--)
            for (int cols = c.right()-c.left(); cols >= 0; cols--) {
                setCell(c.top()+rows, c.left()+cols, w);
            }
    }
    int numRows() const { return m_nrows; }
    int numCols() const { return m_ncols; }

    void simplify();
    bool locateWidget(QWidget* w, int& row, int& col, int& rowspan, int& colspan);

private:
    void merge();
    int countRow(int r, int c) const;
    int countCol(int r, int c) const;
    void setRow(int r, int c, QWidget* w, int count);
    void setCol(int r, int c, QWidget* w, int count);
    bool isWidgetStartCol(int c) const;
    bool isWidgetEndCol(int c) const;
    bool isWidgetStartRow(int r) const;
    bool isWidgetEndRow(int r) const;
    bool isWidgetTopLeft(int r, int c) const;
    void extendLeft();
    void extendRight();
    void extendUp();
    void extendDown();

    const int m_nrows;
    const int m_ncols;

    QWidget** m_cells;
    bool* m_cols;
    bool* m_rows;
};

Grid::Grid(int r, int c) :
    m_nrows(r),
    m_ncols(c),
    m_cells(new QWidget*[ r * c ]),
    m_cols(new bool[ c ]),
    m_rows(new bool[ r ])
{
    qFill(m_cells, m_cells + r * c,  static_cast<QWidget *>(0));
}

Grid::~Grid()
{
    delete [] m_cells;
    delete [] m_cols;
    delete [] m_rows;
}

int Grid::countRow(int r, int c) const
{
    QWidget* w = cell(r, c);
    int i = c + 1;
    while (i < m_ncols && cell(r, i) == w)
        i++;
    return i - c;
}

int Grid::countCol(int r, int c) const
{
    QWidget* w = cell(r, c);
    int i = r + 1;
    while (i < m_nrows && cell(i, c) == w)
        i++;
    return i - r;
}

void Grid::setCol(int r, int c, QWidget* w, int count)
{
    for (int i = 0; i < count; i++)
        setCell(r + i, c, w);
}

void Grid::setRow(int r, int c, QWidget* w, int count)
{
    for (int i = 0; i < count; i++)
        setCell(r, c + i, w);
}

bool Grid::isWidgetStartCol(int c) const
{
    int r;
    for (r = 0; r < m_nrows; r++) {
        if (cell(r, c) && ((c==0) || (cell(r, c)  != cell(r, c-1)))) {
            return true;
        }
    }
    return false;
}

bool Grid::isWidgetEndCol(int c) const
{
    int r;
    for (r = 0; r < m_nrows; r++) {
        if (cell(r, c) && ((c == m_ncols-1) || (cell(r, c) != cell(r, c+1))))
            return true;
    }
    return false;
}

bool Grid::isWidgetStartRow(int r) const
{
    int c;
    for (c = 0; c < m_ncols; c++) {
        if (cell(r, c) && ((r==0) || (cell(r, c) != cell(r-1, c))))
            return true;
    }
    return false;
}

bool Grid::isWidgetEndRow(int r) const
{
    int c;
    for (c = 0; c < m_ncols; c++) {
        if (cell(r, c) && ((r == m_nrows-1) || (cell(r, c) != cell(r+1, c))))
            return true;
    }
    return false;
}


bool Grid::isWidgetTopLeft(int r, int c) const
{
    QWidget* w = cell(r, c);
    if (!w)
        return false;
    return (!r || cell(r-1, c) != w) && (!c || cell(r, c-1) != w);
}

void Grid::extendLeft()
{
    int r,c,i;
    for (c = 1; c < m_ncols; c++) {
        for (r = 0; r < m_nrows; r++) {
            QWidget* w = cell(r, c);
            if (!w)
                continue;

            int cc = countCol(r, c);
            int stretch = 0;
            for (i = c-1; i >= 0; i--) {
                if (cell(r, i))
                    break;
                if (countCol(r, i) < cc)
                    break;
                if (isWidgetEndCol(i))
                    break;
                if (isWidgetStartCol(i)) {
                    stretch = c - i;
                    break;
                }
            }
            if (stretch) {
                for (i = 0; i < stretch; i++)
                    setCol(r, c-i-1, w, cc);
            }
        }
    }
}


void Grid::extendRight()
{
    int r,c,i;
    for (c = m_ncols - 2; c >= 0; c--) {
        for (r = 0; r < m_nrows; r++) {
            QWidget* w = cell(r, c);
            if (!w)
                continue;
            int cc = countCol(r, c);
            int stretch = 0;
            for (i = c+1; i < m_ncols; i++) {
                if (cell(r, i))
                    break;
                if (countCol(r, i) < cc)
                    break;
                if (isWidgetStartCol(i))
                    break;
                if (isWidgetEndCol(i)) {
                    stretch = i - c;
                    break;
                }
            }
            if (stretch) {
                for (i = 0; i < stretch; i++)
                    setCol(r, c+i+1, w, cc);
            }
        }
    }

}

void Grid::extendUp()
{
    int r,c,i;
    for (r = 1; r < m_nrows; r++) {
        for (c = 0; c < m_ncols; c++) {
            QWidget* w = cell(r, c);
            if (!w)
                continue;
            int cr = countRow(r, c);
            int stretch = 0;
            for (i = r-1; i >= 0; i--) {
                if (cell(i, c))
                    break;
                if (countRow(i, c) < cr)
                    break;
                if (isWidgetEndRow(i))
                    break;
                if (isWidgetStartRow(i)) {
                    stretch = r - i;
                    break;
                }
            }
            if (stretch) {
                for (i = 0; i < stretch; i++)
                    setRow(r-i-1, c, w, cr);
            }
        }
    }
}

void Grid::extendDown()
{
    int r,c,i;
    for (r = m_nrows - 2; r >= 0; r--) {
        for (c = 0; c < m_ncols; c++) {
            QWidget* w = cell(r, c);
            if (!w)
                continue;
            int cr = countRow(r, c);
            int stretch = 0;
            for (i = r+1; i < m_nrows; i++) {
                if (cell(i, c))
                    break;
                if (countRow(i, c) < cr)
                    break;
                if (isWidgetStartRow(i))
                    break;
                if (isWidgetEndRow(i)) {
                    stretch = i - r;
                    break;
                }
            }
            if (stretch) {
                for (i = 0; i < stretch; i++)
                    setRow(r+i+1, c, w, cr);
            }
        }
    }

}

void Grid::simplify()
{
    extendLeft();
    extendRight();
    extendUp();
    extendDown();
    merge();
}


void Grid::merge()
{
    int r,c;
    for (c = 0; c < m_ncols; c++)
        m_cols[c] = false;

    for (r = 0; r < m_nrows; r++)
        m_rows[r] = false;

    for (c = 0; c < m_ncols; c++) {
        for (r = 0; r < m_nrows; r++) {
            if (isWidgetTopLeft(r, c)) {
                m_rows[r] = true;
                m_cols[c] = true;
            }
        }
    }
}

bool Grid::locateWidget(QWidget *w, int &row, int &col, int &rowspan, int &colspan)
{
    int r, c, r2, c2;

    for (c = 0; c < m_ncols; c++) {
        for (r = 0; r < m_nrows; r++) {
            if (cell(r, c) == w) {
                row = 0;
                for (r2 = 1; r2 <= r; r2++) {
                    if (m_rows[r2-1])
                        row++;
                }
                col = 0;
                for (c2 = 1; c2 <= c; c2++) {
                    if (m_cols[c2-1])
                        col++;
                }
                rowspan = 0;
                for (r2 = r ; r2 < m_nrows && cell(r2, c) == w; r2++) {
                    if (m_rows[r2])
                        rowspan++;
                }
                colspan = 0;
                for (c2 = c; c2 < m_ncols && cell(r, c2) == w; c2++) {
                    if (m_cols[c2])
                        colspan++;
                }
                return true;
            }
        }
    }
    return false;
}

// ----------- GridLayout
class GridLayout : public Layout
{
public:
    GridLayout(const QWidgetList &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, const QSize &res);
    ~GridLayout();

    virtual void doLayout();
    virtual void sort();

protected:
    QWidget *widgetAt(QGridLayout *layout, int row, int column) const;

protected:
    QWidgetList buildGrid(const QWidgetList &);
    QSize m_resolution;
    Grid* m_grid;

};
GridLayout::GridLayout(const QWidgetList &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, const QSize &res) :
    Layout(wl, p, fw, lb, LayoutInfo::Grid),
    m_resolution(res),
    m_grid(0)
{
}

GridLayout::~GridLayout()
{
    delete m_grid;
}

QWidget *GridLayout::widgetAt(QGridLayout *layout, int row, int column) const
{
    int index = 0;
    while (QLayoutItem *item = layout->itemAt(index)) {
        if (item->widget()) {
            int r, c, rowspan, colspan;
            layout->getItemPosition(index, &r, &c, &rowspan, &colspan);
            if (row == r && column == c)
                return item->widget();
        }
        ++index;
    }
    return 0;
}

void GridLayout::doLayout()
{
    bool needMove, needReparent;
    if (!prepareLayout(needMove, needReparent))
        return;

    QGridLayout *layout =  static_cast<QGridLayout *>(createLayout(LayoutInfo::Grid));

    if (!m_grid)
        sort();

    const QWidgetList &widgetList = widgets();
    foreach (QWidget *w, widgetList) {
        int r = 0, c = 0, rs = 0, cs = 0;

        if (m_grid->locateWidget(w, r, c, rs, cs)) {
            if (needReparent)
                reparentToLayoutBase(w);

            Qt::Alignment alignment = Qt::Alignment(0);
            if (const Spacer *spacer = qobject_cast<const Spacer*>(w))
                alignment = spacer->alignment();

            if (rs * cs == 1) {
                layout->addWidget(w, r, c, 1, 1, alignment);
            } else {
                layout->addWidget(w, r, c, rs, cs, alignment);
            }

            w->show();
        } else {
            qDebug("ooops, widget '%s' does not fit in layout", w->objectName().toUtf8().constData());
        }
    }

    QLayoutSupport::createEmptyCells(layout);

    finishLayout(needMove, layout);
}

void GridLayout::sort()
{
    setWidgets(buildGrid(widgets()));
}

// Remove duplicate entries (Remove next, if equal to current)
static void removeDuplicates(QVector<int> &v)
{
    if (v.size() < 2)
        return;

    for (QVector<int>::iterator current = v.begin() ; (current != v.end()) && ((current+1) != v.end()) ; )
        if ( (*current == *(current+1)) )
            v.erase(current+1);
        else
            ++current;
}

QWidgetList GridLayout::buildGrid(const QWidgetList &widgetList)
{
    if (widgetList.empty())
        return QWidgetList();

    // Pixel to cell conversion:
    // By keeping a list of start'n'stop values (x & y) for each widget,
    // it is possible to create a very small grid of cells to represent
    // the widget layout.
    // -----------------------------------------------------------------

    // We need a list of both start and stop values for x- & y-axis
    const int widgetCount = widgetList.size();
    QVector<int> x( widgetCount * 2 );
    QVector<int> y( widgetCount * 2 );

    // Using push_back would look nicer, but operator[] is much faster
    int index  = 0;
    for (int i = 0; i < widgetCount; ++i) {
        const QRect widgetPos = widgetList.at(i)->geometry();
        x[index]   = widgetPos.left();
        x[index+1] = widgetPos.right();
        y[index]   = widgetPos.top();
        y[index+1] = widgetPos.bottom();
        index += 2;
    }

    qSort(x);
    qSort(y);

    // Remove duplicate x enteries (Remove next, if equal to current)
    removeDuplicates(x);
    removeDuplicates(y);

    delete m_grid;
    m_grid = new Grid(y.size() - 1, x.size() - 1);

    // Mark the cells in the grid that contains a widget
    foreach (QWidget *w, widgetList) {
        const QRect widgetPos = w->geometry();
        QRect c(0, 0, 0, 0); // rect of columns/rows

        // From left til right (not including)
        const int leftIdx = x.indexOf(widgetPos.left());
        Q_ASSERT(leftIdx != -1);
        c.setLeft(leftIdx);
        for (int cw=leftIdx; cw<x.size(); cw++)
            if (x[cw] <  widgetPos.right())
                c.setRight(cw);
            else
                break;
        // From top til bottom (not including)
        const int topIdx = y.indexOf(widgetPos.top());
        Q_ASSERT(topIdx != -1);
        c.setTop(topIdx);
        for (int ch=topIdx; ch<y.size(); ch++)
            if (y[ch] <  widgetPos.bottom())
                c.setBottom(ch);
            else
                break;
        m_grid->setCells(c, w); // Mark cellblock
    }

    m_grid->simplify();

    QWidgetList ordered;
    for (int i = 0; i < m_grid->numRows(); i++)
        for (int j = 0; j < m_grid->numCols(); j++) {
            QWidget *w = m_grid->cell(i, j);
            if (w && !ordered.contains(w))
                ordered.append(w);
        }
    return ordered;
}
} // anonymous

Layout* Layout::createLayout(const QWidgetList &widgets,  QWidget *parentWidget,
                             QDesignerFormWindowInterface *fw,
                             QWidget *layoutBase, LayoutInfo::Type layoutType)
{
    switch (layoutType) {
    case LayoutInfo::Grid: {
        const QPoint grid = fw->grid();
        const QSize sz(qMax(5, grid.x()), qMax(5, grid.y()));
        return new GridLayout(widgets, parentWidget, fw, layoutBase, sz);
    }
    case LayoutInfo::HBox:
    case LayoutInfo::VBox: {
        const Qt::Orientation orientation = layoutType == LayoutInfo::HBox ? Qt::Horizontal : Qt::Vertical;
        return new BoxLayout(widgets, parentWidget, fw, layoutBase, orientation);
    }
    case LayoutInfo::HSplitter:
    case LayoutInfo::VSplitter: {
        const Qt::Orientation orientation = layoutType == LayoutInfo::HSplitter ? Qt::Horizontal : Qt::Vertical;
        return new SplitterLayout(widgets, parentWidget, fw, layoutBase, orientation);
    }
    default:
        break;
    }
    Q_ASSERT(0);
    return 0;
}
} // namespace qdesigner_internal
