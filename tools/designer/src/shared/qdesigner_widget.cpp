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

#include "qdesigner_widget.h"
#include "qdesigner_command.h"
#include "layout.h"
#include "invisible_widget.h"

#include <abstractformwindow.h>
#include <abstractformeditor.h>
#include <abstractwidgetfactory.h>
#include <propertysheet.h>
#include <qextensionmanager.h>

#include <QtGui/QBitmap>
#include <QtGui/QPixmapCache>
#include <QtGui/QToolButton>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>
#include <QtGui/qevent.h>

#include <QtCore/qdebug.h>


class FriendlyLayout: public QLayout
{
public:
    inline FriendlyLayout(): QLayout() { Q_ASSERT(0); }

    friend class QLayoutWidgetItem;
};

static void paintGrid(QWidget *widget, AbstractFormWindow *formWindow, QPaintEvent *e, bool needFrame = false)
{
    QPainter p(widget);
    p.setClipRect(e->rect());

    p.fillRect(e->rect(), widget->palette().brush(widget->backgroundRole()));

    QString grid_name;
    grid_name.sprintf("AbstractFormWindowGrid_%d_%d", formWindow->grid().x(), formWindow->grid().y());

    QPixmap grid;
    if (!QPixmapCache::find(grid_name, grid)) {
        grid = QPixmap(350 + (350 % formWindow->grid().x()), 350 + (350 % formWindow->grid().y()));
        grid.fill(widget->palette().foreground().color());
        QBitmap mask(grid.width(), grid.height());
        mask.fill(Qt::color0);
        QPainter p(&mask);
        p.setPen(Qt::color1);
        for (int y = 0; y < grid.width(); y += formWindow->grid().y()) {
            for (int x = 0; x < grid.height(); x += formWindow->grid().x()) {
                p.drawPoint(x, y);
            }
        }
        grid.setMask(mask);
        QPixmapCache::insert(grid_name, grid);
    }

    p.drawTiledPixmap(0, 0, widget->width(), widget->height(), grid);

    if (needFrame) {
        p.setPen(widget->palette().dark().color());
        p.drawRect(widget->rect());
    }
}

void QDesignerDialog::paintEvent(QPaintEvent *e)
{
    if (!m_formWindow->hasFeature(AbstractFormWindow::GridFeature)) {
        QWidget::paintEvent(e);
        return;
    }

    paintGrid(this, m_formWindow, e);
}

void QDesignerLabel::updateBuddy()
{
    if (myBuddy.isEmpty())
        return;

    QList<QWidget*> l = qFindChildren<QWidget*>(topLevelWidget(), myBuddy);
    if (l.size())
        QLabel::setBuddy(l.first());
}

QDesignerWidget::QDesignerWidget(AbstractFormWindow* formWindow, QWidget *parent)
    : QWidget(parent), m_formWindow(formWindow)
{
    need_frame = true;
}

QDesignerWidget::~QDesignerWidget()
{
}

void QDesignerWidget::paintEvent(QPaintEvent *e)
{
    if (m_formWindow->hasFeature(AbstractFormWindow::GridFeature)) {
        paintGrid(this, m_formWindow, e, need_frame);
    } else {
        QWidget::paintEvent(e);
    }

}

void QDesignerWidget::dragEnterEvent(QDragEnterEvent *)
{
//    e->setAccepted(QTextDrag::canDecode(e));
}

QLayoutWidget::QLayoutWidget(AbstractFormWindow *formWindow, QWidget *parent)
    : QWidget(parent), m_formWindow(formWindow),
      m_support(formWindow, this)
{
}

void QLayoutWidget::paintEvent(QPaintEvent*)
{
    if (!m_formWindow->hasFeature(AbstractFormWindow::GridFeature))
        return;

    if (m_formWindow->editMode() != AbstractFormWindow::WidgetEditMode)
        return;

    QPainter p(this);

#if 0 // ### enable me
    if (layout()) {
        p.setPen(QPen(palette().mid().color(), 1, Qt::DotLine));

        int index = 0;
        QMap<int, int> rows;
        QMap<int, int> columns;

        while (QLayoutItem *item = layout()->itemAt(index)) {
            ++index;

            QRect g = item->geometry();
            rows.insert(g.y(), g.y());
            columns.insert(g.x(), g.x());

            rows.insert(g.bottom(), g.bottom());
            columns.insert(g.right(), g.right());
        }

        QRect g = layout()->geometry();

        foreach (int row, rows.keys())
            p.drawLine(g.x(), row, g.right(), row);

        foreach (int column, columns.keys())
            p.drawLine(column, g.y(), column, g.bottom());
    }
#endif

    p.setPen(QPen(Qt::red, 1));

    p.drawRect(0, 0, width() - 1, height() - 1);
}

void QLayoutWidget::updateMargin()
{
    if (!layout())
        return;

#if 0 // ### fix me
    if (qt_cast<QLayoutWidget*>(parentWidget())) {
        layout()->setMargin(style()->pixelMetric(QStyle::PM_DefaultChildMargin, 0, 0));
        qDebug() << "use default-child margin";
    } else {
        layout()->setMargin(style()->pixelMetric(QStyle::PM_DefaultToplevelMargin, 0, 0));
        qDebug() << "use default-toplevel margin";
    }
#endif
}

bool QLayoutWidget::event(QEvent *e)
{
    switch (e->type()) {
        case QEvent::ParentChange:
            if (e->type() == QEvent::ParentChange)
                updateMargin();
            break;

        case QEvent::LayoutRequest: {
            (void) QWidget::event(e);

            if (layout() && LayoutInfo::layoutType(formWindow()->core(), parentWidget()) == LayoutInfo::NoLayout)
                resize(layout()->sizeHint());

            update();

            return true;
        }

        default:
            break;
    }

    return QWidget::event(e);
}

int QLayoutWidget::layoutMargin() const
{
    if (layout())
        return layout()->margin();

    qWarning("unknown margin!");
    return 0;
}

void QLayoutWidget::setLayoutMargin(int layoutMargin)
{
    if (layout())
        layout()->setMargin(layoutMargin);

    QList<QLayoutWidget*> lst = qFindChildren<QLayoutWidget*>(this);
    foreach (QLayoutWidget *lay, lst)
        lay->setLayoutMargin(layoutMargin);
}

int QLayoutWidget::layoutSpacing() const
{
    if (layout())
        return layout()->spacing();

    qWarning("unknown spacing!");
    return 0;
}

void QLayoutWidget::setLayoutSpacing(int layoutSpacing)
{
    if (layout())
        layout()->setSpacing(layoutSpacing);
}



// ---- QLayoutSupport ----
QLayoutSupport::QLayoutSupport(AbstractFormWindow *formWindow, QWidget *widget, QObject *parent)
    : QObject(parent),
      m_formWindow(formWindow),
      m_widget(widget),
      m_currentIndex(-1),
      m_currentInsertMode(ILayoutDecoration::InsertWidgetMode)
{
    QPalette p;
    p.setColor(QPalette::Background, Qt::red);

    m_indicatorLeft = new InvisibleWidget(m_widget);
    m_indicatorLeft->setPalette(p);
    m_indicatorLeft->hide();

    m_indicatorTop = new InvisibleWidget(m_widget);
    m_indicatorTop->setPalette(p);
    m_indicatorTop->hide();

    m_indicatorRight = new InvisibleWidget(m_widget);
    m_indicatorRight->setPalette(p);
    m_indicatorRight->hide();

    m_indicatorBottom = new InvisibleWidget(m_widget);
    m_indicatorBottom->setPalette(p);
    m_indicatorBottom->hide();

    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(formWindow->core()->extensionManager(), m_widget)) {
        sheet->setChanged(sheet->indexOf("margin"), true);
        sheet->setChanged(sheet->indexOf("spacing"), true);
    }
}

QLayoutSupport::~QLayoutSupport()
{
    if (m_indicatorLeft)
        m_indicatorLeft->deleteLater();

    if (m_indicatorTop)
        m_indicatorTop->deleteLater();

    if (m_indicatorRight)
        m_indicatorRight->deleteLater();

    if (m_indicatorBottom)
        m_indicatorBottom->deleteLater();
}

void QLayoutSupport::tryRemoveRow(int row)
{
    Q_ASSERT(gridLayout());

    bool please_removeRow = true;
    int index = 0;
    while (QLayoutItem *item = gridLayout()->itemAt(index)) {
        QRect info = itemInfo(index);
        ++index;

        if (info.y() == row && !isEmptyItem(item)) {
            please_removeRow = false;
            break;
        }
    }

    if (please_removeRow) {
        removeRow(row);
        gridLayout()->invalidate();
    }
}

void QLayoutSupport::removeRow(int row)
{
    QHash<QLayoutItem*, QRect> infos;
    computeGridLayout(&infos);

    QMutableHashIterator<QLayoutItem*, QRect> it(infos);
    while (it.hasNext()) {
        it.next();

        QRect info = it.value();

        if (info.y() == row) {
            QLayoutItem *item = it.key();
            it.remove();

            layout()->takeAt(indexOf(item));
            delete item;
        } else if (info.y() > row) {
            info.translate(0, -1);
            it.setValue(info);
        }
    }

    rebuildGridLayout(&infos);
}

void QLayoutSupport::removeColumn(int column)
{
    QHash<QLayoutItem*, QRect> infos;
    computeGridLayout(&infos);

    QMutableHashIterator<QLayoutItem*, QRect> it(infos);
    while (it.hasNext()) {
        it.next();

        QRect info = it.value();

        if (info.x() == column) {
            QLayoutItem *item = it.key();
            it.remove();

            layout()->takeAt(indexOf(item));
            delete item;
        } else if (info.x() > column) {
            info.translate(-1, 0);
            it.setValue(info);
        }
    }

    rebuildGridLayout(&infos);
}

void QLayoutSupport::tryRemoveColumn(int column)
{
    Q_ASSERT(gridLayout());

    bool please_removeColumn = true;
    int index = 0;
    while (QLayoutItem *item = gridLayout()->itemAt(index)) {
        QRect info = itemInfo(index);
        ++index;

        if (info.x() == column && !isEmptyItem(item)) {
            please_removeColumn = false;
            break;
        }
    }

    if (please_removeColumn) {
        removeColumn(column);
        gridLayout()->invalidate();
    }
}

void QLayoutSupport::simplifyLayout()
{
    if (!gridLayout())
        return;

    for (int r = 0; r < gridLayout()->rowCount(); ++r) {
        tryRemoveRow(r);
    }

    for (int c = 0; c < gridLayout()->columnCount(); ++c) {
        tryRemoveColumn(c);
    }
#if 0
    if (QGridLayout *g = gridLayout())
        createEmptyCells(g);
#endif
}

void QLayoutSupport::adjustIndicator(const QPoint &pos, int index)
{
    if (index == -1) {
        m_indicatorLeft->hide();
        m_indicatorTop->hide();
        m_indicatorRight->hide();
        m_indicatorBottom->hide();
        return;
    }

    m_currentIndex = index;
    m_currentInsertMode = ILayoutDecoration::InsertWidgetMode;

    QLayoutItem *item = layout()->itemAt(index);
    QRect g = extendedGeometry(index);

    int dx = g.right() - pos.x();
    int dy = g.bottom() - pos.y();

    int dx1 = pos.x() - g.x();
    int dy1 = pos.y() - g.y();

    int mx = qMin(dx, dx1);
    int my = qMin(dy, dy1);

    bool isVertical = mx < my;

    // ### cleanup
    if (isEmptyItem(item)) {
        QPalette p;
        p.setColor(QPalette::Background, Qt::red);
        m_indicatorRight->setPalette(p);
        m_indicatorBottom->setPalette(p);

        m_indicatorLeft->setGeometry(g.x(), g.y(), 2, g.height());
        m_indicatorTop->setGeometry(g.x(), g.y(), g.width(), 2);
        m_indicatorRight->setGeometry(g.right(), g.y(), 2, g.height());
        m_indicatorBottom->setGeometry(g.x(), g.bottom(), g.width(), 2);

        m_indicatorLeft->show();
        m_indicatorLeft->raise();

        m_indicatorTop->show();
        m_indicatorTop->raise();

        m_indicatorRight->show();
        m_indicatorRight->raise();

        m_indicatorBottom->show();
        m_indicatorBottom->raise();

        if (QGridLayout *gridLayout = qt_cast<QGridLayout*>(layout())) {
            m_currentInsertMode = ILayoutDecoration::InsertWidgetMode;
            int row, column, rowspan, colspan;
            gridLayout->getItemPosition(m_currentIndex, &row, &column, &rowspan, &colspan);
            m_currentCell = qMakePair(row, column);
        } else {
            qWarning("Warning: found a fake spacer inside a vbox layout");
            m_currentCell = qMakePair(0, 0);
        }
    } else {
        QPalette p;
        p.setColor(QPalette::Background, Qt::blue);
        m_indicatorRight->setPalette(p);
        m_indicatorBottom->setPalette(p);

        QRect r(layout()->geometry().topLeft(), layout()->parentWidget()->size());
        if (isVertical) {
            m_indicatorBottom->hide();

            if (!qt_cast<QVBoxLayout*>(layout())) {
                m_indicatorRight->setGeometry((mx == dx1) ? g.x() : g.right(), 0, 2, r.height());
                m_indicatorRight->show();
                m_indicatorRight->raise();

                int incr = (mx == dx1) ? 0 : +1;

                if (QGridLayout *gridLayout = qt_cast<QGridLayout*>(layout())) {
                    m_currentInsertMode = ILayoutDecoration::InsertColumnMode;
                    int row, column, rowspan, colspan;
                    gridLayout->getItemPosition(m_currentIndex, &row, &column, &rowspan, &colspan);
                    m_currentCell = qMakePair(row, qMax(0, column + incr));
                } else if (QBoxLayout *box = qt_cast<QBoxLayout*>(layout())) {
                    m_currentCell = qMakePair(0, box->findWidget(item->widget()) + incr);
                }
            }
        } else {
            m_indicatorRight->hide();

            if (!qt_cast<QHBoxLayout*>(layout())) {
                m_indicatorBottom->setGeometry(r.x(), (my == dy1) ? g.y() : g.bottom(), r.width(), 2);
                m_indicatorBottom->show();
                m_indicatorBottom->raise();

                int incr = (my == dy1) ? 0 : +1;

                if (QGridLayout *gridLayout = qt_cast<QGridLayout*>(layout())) {
                    m_currentInsertMode = ILayoutDecoration::InsertRowMode;
                    int row, column, rowspan, colspan;
                    gridLayout->getItemPosition(m_currentIndex, &row, &column, &rowspan, &colspan);
                    m_currentCell = qMakePair(qMax(0, row + incr), column);
                } else if (QBoxLayout *box = qt_cast<QBoxLayout*>(layout())) {
                    m_currentCell = qMakePair(box->findWidget(item->widget()) + incr, 0);
                }
            }
        }

        m_indicatorLeft->hide();
        m_indicatorTop->hide();
    }
}

int QLayoutSupport::indexOf(QLayoutItem *i) const
{
    if (!layout())
        return -1;

    int index = 0;
    while (QLayoutItem *item = layout()->itemAt(index)) {
        if (item == i)
            return index;

        ++index;
    }

    return -1;
}

int QLayoutSupport::indexOf(QWidget *widget) const
{
    if (!layout())
        return -1;

    int index = 0;
    while (QLayoutItem *item = layout()->itemAt(index)) {
        if (item->widget() == widget)
            return index;

        ++index;
    }

    return -1;
}

AbstractFormEditor *QLayoutSupport::core() const
{
    return formWindow()->core();
}

void QLayoutSupport::removeWidget(QWidget *widget)
{
    LayoutInfo::Type layoutType = LayoutInfo::layoutType(core(), m_widget);

    switch (layoutType) {
        case LayoutInfo::Grid: {
            int index = indexOf(widget);
            if (index != -1) {
                QGridLayout *gridLayout = qt_cast<QGridLayout*>(layout());
                Q_ASSERT(gridLayout);
                int row, column, rowspan, colspan;
                gridLayout->getItemPosition(index, &row, &column, &rowspan, &colspan);
                gridLayout->takeAt(index);
                QSpacerItem *spacer = new QSpacerItem(20, 20);
                gridLayout->addItem(spacer, row, column, rowspan, colspan);
            }
        } break;

        case LayoutInfo::VBox:
        case LayoutInfo::HBox: {
            QBoxLayout *box = static_cast<QBoxLayout*>(layout());
            box->removeWidget(widget);
        } break;

        default:
            break;
    }
}

QList<QWidget*> QLayoutSupport::widgets(QLayout *layout)
{
    if (!layout)
        return QList<QWidget*>();

    QList<QWidget*> lst;
    int index = 0;
    while (QLayoutItem *item = layout->itemAt(index)) {
        ++index;

        QWidget *widget = item->widget();
        if (widget && formWindow()->isManaged(widget))
            lst.append(widget);
    }

    return lst;
}

void QLayoutSupport::insertWidget(QWidget *widget, const QPair<int, int> &cell)
{
    AbstractFormEditor *core = formWindow()->core();
    LayoutInfo::Type lt = LayoutInfo::layoutType(core, layout());
    switch (lt) {
        case LayoutInfo::VBox: {
            QVBoxLayout *vbox = static_cast<QVBoxLayout*>(layout());
            insert_into_box_layout(vbox, cell.first, widget);
        } break;

        case LayoutInfo::HBox: {
            QHBoxLayout *hbox = static_cast<QHBoxLayout*>(layout());
            insert_into_box_layout(hbox, cell.second, widget);
        } break;

        case LayoutInfo::Grid: {
            int index = findItemAt(cell.first, cell.second);
            qDebug() << "======> insert widget:" << widget << "index:" << index << "cell:" << cell.first << cell.second;
            Q_ASSERT(index != -1);

            insertWidget(index, widget);
        } break;

        default: {
            qWarning("expected a layout here!");
            //Q_ASSERT(0);
        }
    } // end switch
}

int QLayoutSupport::findItemAt(int at_row, int at_column) const
{
    if (QGridLayout *gridLayout = qt_cast<QGridLayout*>(layout()))
        return findItemAt(gridLayout, at_row, at_column);

    return -1;
}

int QLayoutSupport::findItemAt(QGridLayout *gridLayout, int at_row, int at_column)
{
    Q_ASSERT(gridLayout);

    int index = 0;
    while (gridLayout->itemAt(index)) {
        int row, column, rowspan, colspan;
        gridLayout->getItemPosition(index, &row, &column, &rowspan, &colspan);

        if (at_row >= row && at_row < (row + rowspan)
            && at_column >= column && at_column < (column + colspan))
            return index;

        ++index;
    }

    return -1;
}

void QLayoutWidgetItem::addTo(QLayout *layout)
{
    static_cast<FriendlyLayout*>(layout)->addChildWidget(widget());
}

void QLayoutWidgetItem::removeFrom(QLayout *layout)
{
    Q_UNUSED(layout);
}

void QLayoutSupport::insertWidget(int index, QWidget *widget)
{
    QGridLayout *gridLayout = qt_cast<QGridLayout*>(layout());
    QLayoutItem *item = gridLayout->itemAt(index);
    if (item && isEmptyItem(item)) {
        int row, column, rowspan, colspan;
        gridLayout->getItemPosition(index, &row, &column, &rowspan, &colspan);
        gridLayout->takeAt(index);
        add_to_grid_layout(gridLayout, widget, row, column, rowspan, colspan);
        delete item;
    }
}

void QLayoutSupport::createEmptyCells(QGridLayout *&gridLayout)
{
    Q_ASSERT(gridLayout);

#if 0
    { // take the spacers items
        int index = 0;
        while (QLayoutItem *item = gridLayout->itemAt(index)) {
            if (QSpacerItem *spacer = item->spacerItem()) {
                gridLayout->takeAt(index);
                delete spacer;
                // we don't have to increment the `index' here!
            } else
                ++index;
        }
    }
#endif

    QMap<QPair<int,int>, QLayoutItem*> cells;

    for (int r = 0; r < gridLayout->rowCount(); ++r) {
        for (int c = 0; c < gridLayout->columnCount(); ++c) {
            QPair<int,int> cell = qMakePair(r, c);
            cells.insert(cell, 0);
        }
    }

    int index = 0;
    while (QLayoutItem *item = gridLayout->itemAt(index)) {
        int row, column, rowspan, colspan;
        gridLayout->getItemPosition(index, &row, &column, &rowspan, &colspan);
        ++index;

        for (int r = row; r < row + rowspan; ++r) {
            for (int c = column; c < column + colspan; ++c) {
                QPair<int,int> cell = qMakePair(r, c);
                cells[cell] = item;

                if (!item) {
                    /* skip */
                } else if (item->layout()) {
                    qWarning("unexpected layout");
                } else if (item->spacerItem()) {
                    qWarning("unexpected spacer");
                }
            }
        }
    }

    QMapIterator<QPair<int,int>, QLayoutItem*> it(cells);
    while (it.hasNext()) {
        it.next();

        const QPair<int, int> &cell = it.key();
        QLayoutItem *item = it.value();

        if (!item || !item->widget() && findItemAt(gridLayout, cell.first, cell.second) == -1) {
            gridLayout->addItem(new QSpacerItem(20, 20), cell.first, cell.second);
        }
    }
}

void QLayoutSupport::insertRow(int row)
{
    QHash<QLayoutItem*, QRect> infos;
    computeGridLayout(&infos);

    QMutableHashIterator<QLayoutItem*, QRect> it(infos);
    while (it.hasNext()) {
        it.next();

        QRect info = it.value();
        if (info.y() >= row) {
            info.translate(0, 1);
            it.setValue(info);
        }
    }

    rebuildGridLayout(&infos);

    QGridLayout *gridLayout = qt_cast<QGridLayout*>(layout());
    Q_ASSERT(gridLayout);

    if (gridLayout->rowCount() == row) {
        gridLayout->addItem(new QSpacerItem(20, 20), gridLayout->rowCount(), 0);
    }

    gridLayout = qt_cast<QGridLayout*>(layout());
    Q_ASSERT(gridLayout);

    createEmptyCells(gridLayout);

    layout()->activate();
}

void QLayoutSupport::insertColumn(int column)
{
    QHash<QLayoutItem*, QRect> infos;
    computeGridLayout(&infos);

    QMutableHashIterator<QLayoutItem*, QRect> it(infos);
    while (it.hasNext()) {
        it.next();

        QRect info = it.value();

        if (info.x() >= column) {
            info.translate(1, 0);
            it.setValue(info);
        }
    }

    rebuildGridLayout(&infos);

    QGridLayout *gridLayout = qt_cast<QGridLayout*>(layout());
    Q_ASSERT(gridLayout);

    if (gridLayout->columnCount() == column) {
        gridLayout->addItem(new QSpacerItem(20, 20), 0, gridLayout->columnCount());
    }

    gridLayout = qt_cast<QGridLayout*>(layout());
    Q_ASSERT(gridLayout);

    createEmptyCells(gridLayout);

    layout()->activate();
}

QRect QLayoutSupport::itemInfo(int index) const
{
    Q_ASSERT(layout());

    if (QGridLayout *l = qt_cast<QGridLayout*>(layout())) {
        int row, column, rowSpan, columnSpan;
        l->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
        return QRect(column, row, columnSpan, rowSpan);
    } else if (qt_cast<QHBoxLayout*>(layout())) {
        return QRect(0, index, 1, 1);
    } else if (qt_cast<QVBoxLayout*>(layout())) {
        return QRect(index, 0, 1, 1);
    } else {
        Q_ASSERT(0); // ### not supported yet!
        return QRect();
    }
}

QRect QLayoutSupport::extendedGeometry(int index) const
{
    QLayoutItem *item = layout()->itemAt(index);
    QRect g = item->geometry();

    QRect info = itemInfo(index);

    if (info.x() == 0) {
        QPoint topLeft = g.topLeft();
        topLeft.rx() = layout()->geometry().left();
        g.setTopLeft(topLeft);
    }

    if (info.y() == 0) {
        QPoint topLeft = g.topLeft();
        topLeft.ry() = layout()->geometry().top();
        g.setTopLeft(topLeft);
    }

    if (QVBoxLayout *vbox = qt_cast<QVBoxLayout*>(layout())) {
        if (vbox->itemAt(index+1) == 0) {
            QPoint bottomRight = g.bottomRight();
            bottomRight.ry() = layout()->geometry().bottom();
            g.setBottomRight(bottomRight);
        }
    } else if (QHBoxLayout *hbox = qt_cast<QHBoxLayout*>(layout())) {
        if (hbox->itemAt(index+1) == 0) {
            QPoint bottomRight = g.bottomRight();
            bottomRight.rx() = layout()->geometry().right();
            g.setBottomRight(bottomRight);
        }
    } else if (QGridLayout *grid = qt_cast<QGridLayout*>(layout())) {
        if (grid->rowCount() == info.y()) {
            QPoint bottomRight = g.bottomRight();
            bottomRight.ry() = layout()->geometry().bottom();
            g.setBottomRight(bottomRight);
        }

        if (grid->columnCount() == info.x()) {
            QPoint bottomRight = g.bottomRight();
            bottomRight.rx() = layout()->geometry().right();
            g.setBottomRight(bottomRight);
        }
    }

    return g;
}

int QLayoutSupport::findItemAt(const QPoint &pos) const
{
    if (!layout())
        return -1;

    int best = -1;
    int bestIndex = -1;

    int index = 0;
    while (QLayoutItem *item = layout()->itemAt(index)) {

        QRect g = item->geometry();

        int dist = (g.center() - pos).manhattanLength();
        if (best == -1 || dist < best) {
            best = dist;
            bestIndex = index;
        }

        ++index;
    }

    return bestIndex;
}

void QLayoutSupport::computeGridLayout(QHash<QLayoutItem*, QRect> *l)
{
    int index = 0;
    while (QLayoutItem *item = layout()->itemAt(index)) {
        QRect info = itemInfo(index);
        l->insert(item, info);

        ++index;
    }
}

void QLayoutSupport::rebuildGridLayout(QHash<QLayoutItem*, QRect> *infos)
{
    QGridLayout *gridLayout = qt_cast<QGridLayout*>(layout());

    { // take the items
        int index = 0;
        while (gridLayout->itemAt(index))
            gridLayout->takeAt(index);
    }

    Q_ASSERT(gridLayout == m_widget->layout());

    AbstractFormEditor *core = formWindow()->core();
    LayoutInfo::deleteLayout(core, m_widget);

    gridLayout = (QGridLayout*) core->widgetFactory()->createLayout(m_widget, 0, LayoutInfo::Grid);

    QHashIterator<QLayoutItem*, QRect> it(*infos);
    while (it.hasNext()) {
        it.next();

        QRect info = it.value();

        gridLayout->addItem(it.key(), info.y(), info.x(),
                info.height(), info.width());
    }
}
