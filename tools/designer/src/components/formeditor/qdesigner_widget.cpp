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
#include "qdesigner_stackedbox.h"
#include "formwindow.h"
#include "command.h"
#include "qtundo.h"
#include "layout.h"

#include <abstractformeditor.h>
#include <propertysheet.h>
#include <qextensionmanager.h>

#include <qpair.h>
#include <qbitmap.h>
#include <qpixmapcache.h>
#include <qtabbar.h>
#include <qtoolbutton.h>
#include <qpainter.h>
#include <qevent.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qaction.h>
#include <qmessagebox.h>
#include <qdebug.h>

static void paintGrid(QWidget *widget, AbstractFormWindow *formWindow, QPaintEvent *e, bool needFrame = false)
{
    QPainter p(widget);
    p.setClipRect(e->rect());

    p.fillRect(e->rect(), widget->palette().brush(widget->backgroundRole()));

    QString grid_name;
    grid_name.sprintf("FormWindowGrid_%d_%d", formWindow->grid().x(), formWindow->grid().y());

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
    if (!m_formWindow->hasFeature(FormWindow::GridFeature)) {
        QWidget::paintEvent(e);
        return;
    }

    paintGrid(this, m_formWindow, e);
}

void QDesignerLabel::updateBuddy()
{
    if (myBuddy.isEmpty())
        return;

    QList<QWidget*> l = qFindChildren<QWidget*>(topLevelWidget(), myBuddy.latin1());
    if (l.size())
        QLabel::setBuddy(l.first());
}

QDesignerWidget::QDesignerWidget(FormWindow* formWindow, QWidget *parent)
    : QWidget(parent), m_formWindow(formWindow)
{
    need_frame = true; // ### qt_cast<QDesignerStackedWidget*>(parent) != 0;
}

QDesignerWidget::~QDesignerWidget()
{
}

void QDesignerWidget::paintEvent(QPaintEvent *e)
{
    if (m_formWindow->hasFeature(FormWindow::GridFeature)) {
        paintGrid(this, m_formWindow, e, need_frame);
    } else {
        QWidget::paintEvent(e);
    }

}

void QDesignerWidget::dragEnterEvent(QDragEnterEvent *)
{
//    e->setAccepted(QTextDrag::canDecode(e));
}

QLayoutWidget::QLayoutWidget(FormWindow *formWindow, QWidget *parent)
    : QWidget(parent), m_formWindow(formWindow),
      sp(QWidget::sizePolicy()),
      m_support(formWindow, this)
{
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

    QLayoutItem *item = layout()->itemAt(index);
    QRect g = item->geometry();

    int dx = g.right() - pos.x();
    int dy = g.bottom() - pos.y();

    int dx1 = pos.x() - g.x();
    int dy1 = pos.y() - g.y();

    int mx = qMin(dx, dx1);
    int my = qMin(dy, dy1);

    bool isVertical = mx < my;

    // ### cleanup
    if (item->spacerItem()) { // false /*isEmptyItem(item)*/) {
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
            }
        } else {
            m_indicatorRight->hide();

            if (!qt_cast<QHBoxLayout*>(layout())) {
                m_indicatorBottom->setGeometry(r.x(), (my == dy1) ? g.y() : g.bottom(), r.width(), 2);
                m_indicatorBottom->show();
                m_indicatorBottom->raise();
            }
        }

        m_indicatorLeft->hide();
        m_indicatorTop->hide();
    }
}

void QLayoutWidget::paintEvent(QPaintEvent*)
{
    if (!m_formWindow->hasFeature(FormWindow::GridFeature))
        return;

    if (m_formWindow->editMode() != AbstractFormWindow::WidgetEditMode)
        return;

    QPainter p(this);

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

    p.setPen(QPen(Qt::red, 1));

    p.drawRect(0, 0, width() - 1, height() - 1);
}

QSizePolicy QLayoutWidget::sizePolicy() const
{
    return sp;
}

void QLayoutWidget::updateMargin()
{
    if (!layout())
        return;

#if 0
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
        case QEvent::ChildAdded:
        case QEvent::ChildRemoved: {
            QChildEvent *ev = static_cast<QChildEvent*>(e);
            if (QWidget *widget = qt_cast<QWidget*>(ev->child())) {
                if (formWindow()->isManaged(widget)) {
                    updateSizePolicy();
                }
            }
        } break;

        case QEvent::ParentChange:
            if (e->type() == QEvent::ParentChange)
                updateMargin();
            updateSizePolicy();
            break;

        case QEvent::LayoutRequest: {
            bool rtn = QWidget::event(e);
            if (LayoutInfo::layoutType(formWindow()->core(), parentWidget()) == LayoutInfo::NoLayout)
                resize(layout()->sizeHint());
            update();
            return rtn;
        }

        default:
            break;
    }

    return QWidget::event(e);
}


/*
  This function must be called on QLayoutWidget creation and whenever
  the QLayoutWidget's parent layout changes (e.g., from a QHBoxLayout
  to a QVBoxLayout), because of the (illogical) way layouting works.
*/
void QLayoutWidget::updateSizePolicy()
{
    QList<QWidget*> l = widgets(layout());
    if (l.isEmpty()) {
        sp = QWidget::sizePolicy();
        return;
    }

    /*
      QSizePolicy::MayShrink & friends are private. Here we assume the
      following:

          Fixed = 0
          Maximum = MayShrink
          Minimum = MayGrow
          Preferred = MayShrink | MayGrow
    */

    int ht = (int) QSizePolicy::Preferred;
    int vt = (int) QSizePolicy::Preferred;

    if (layout()) {
        /*
          parentLayout is set to the parent layout if there is one and if it is
          top level, in which case layouting is illogical.
        */
        QLayout *parentLayout = 0;
        if (parent() && parent()->isWidgetType()) {
            parentLayout = ((QWidget *)parent())->layout();
            if (parentLayout &&
                 ::qt_cast<QLayoutWidget*>(parentLayout->parentWidget()))
                parentLayout = 0;
        }

        if (::qt_cast<QVBoxLayout*>(layout())) {
            if (::qt_cast<QHBoxLayout*>(parentLayout))
                vt = QSizePolicy::Minimum;
            else
                vt = QSizePolicy::Fixed;

            foreach (QWidget *w, l) {
                if (w->testWState(Qt::WState_ForceHide))
                    continue;

                if (!w->sizePolicy().mayGrowHorizontally())
                    ht &= ~QSizePolicy::Minimum;
                if (!w->sizePolicy().mayShrinkHorizontally())
                    ht &= ~QSizePolicy::Maximum;
                if (w->sizePolicy().mayGrowVertically())
                    vt |= QSizePolicy::Minimum;
                if (w->sizePolicy().mayShrinkVertically())
                    vt |= QSizePolicy::Maximum;
            }
        } else if (::qt_cast<QHBoxLayout*>(layout())) {
            if (::qt_cast<QVBoxLayout*>(parentLayout))
                ht = QSizePolicy::Minimum;
            else
                ht = QSizePolicy::Fixed;

            foreach (QWidget *w, l) {
                if (w->testWState(Qt::WState_ForceHide))
                    continue;

                if (w->sizePolicy().mayGrowHorizontally())
                    ht |= QSizePolicy::Minimum;
                if (w->sizePolicy().mayShrinkHorizontally())
                    ht |= QSizePolicy::Maximum;
                if (!w->sizePolicy().mayGrowVertically())
                    vt &= ~QSizePolicy::Minimum;
                if (!w->sizePolicy().mayShrinkVertically())
                    vt &= ~QSizePolicy::Maximum;
            }
        } else if (::qt_cast<QGridLayout*>(layout())) {
            ht = QSizePolicy::Fixed;
            vt = QSizePolicy::Fixed;
            if (parentLayout) {
                if (::qt_cast<QVBoxLayout*>(parentLayout))
                    ht = QSizePolicy::Minimum;
                else if (::qt_cast<QHBoxLayout*>(parentLayout))
                    vt = QSizePolicy::Minimum;
            }

            foreach (QWidget *w, l) {
                if (w->testWState(Qt::WState_ForceHide))
                    continue;

                if (w->sizePolicy().mayGrowHorizontally())
                    ht |= QSizePolicy::Minimum;
                if (w->sizePolicy().mayShrinkHorizontally())
                    ht |= QSizePolicy::Maximum;
                if (w->sizePolicy().mayGrowVertically())
                    vt |= QSizePolicy::Minimum;
                if (w->sizePolicy().mayShrinkVertically())
                    vt |= QSizePolicy::Maximum;
            }
        }
        if (layout()->expanding() & QSizePolicy::Horizontally)
            ht = QSizePolicy::Expanding;
        if (layout()->expanding() & QSizePolicy::Vertically)
            vt = QSizePolicy::Expanding;

        layout()->invalidate();
    }

    sp = QSizePolicy((QSizePolicy::SizeType) ht, (QSizePolicy::SizeType) vt);
}

int QLayoutWidget::layoutMargin() const
{
    if (layout())
        return layout()->margin();
    return FormWindow::DefaultMargin;
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
    return FormWindow::DefaultSpacing;
}

void QLayoutWidget::setLayoutSpacing(int layoutSpacing)
{
    if (layout())
        layout()->setSpacing(layoutSpacing);
}



// ---- QLayoutSupport ----
QLayoutSupport::QLayoutSupport(FormWindow *formWindow, QWidget *widget, QObject *parent)
    : QObject(parent), m_formWindow(formWindow), m_widget(widget)
{
    QPalette p;
    p.setColor(QPalette::Background, Qt::red);

    m_indicatorLeft = new QWidget(m_widget);
    m_indicatorLeft->setPalette(p);
    m_indicatorLeft->hide();

    m_indicatorTop = new QWidget(m_widget);
    m_indicatorTop->setPalette(p);
    m_indicatorTop->hide();

    m_indicatorRight = new QWidget(m_widget);
    m_indicatorRight->setPalette(p);
    m_indicatorRight->hide();

    m_indicatorBottom = new QWidget(m_widget);
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

void QLayoutSupport::removeWidget(QWidget *widget)
{
    int index = indexOf(widget);
    if (index != -1) {
        // ### take the item
        if (QGridLayout *grid = qt_cast<QGridLayout*>(layout())) {
            int row, column, rowspan, colspan;
            grid->getItemPosition(index, &row, &column, &rowspan, &colspan);
            QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum);
            grid->addItem(spacer, row, column, rowspan, colspan);
        }
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

void QLayoutSupport::insertWidget(QWidget *widget)
{
    AbstractFormEditor *core = formWindow()->core();
    LayoutInfo::Type lt = LayoutInfo::layoutType(core, layout());
    switch (lt) {
        case LayoutInfo::VBox: {
            QList<QWidget*> items = widgets(layout());
            items.append(widget);
            VerticalLayoutList lst(items);
            lst.sort();
            static_cast<QVBoxLayout*>(layout())->insertWidget(lst.indexOf(widget), widget);
        } break;

        case LayoutInfo::HBox: {
            QList<QWidget*> items = widgets(layout());
            items.append(widget);
            HorizontalLayoutList lst(items);
            lst.sort();
            static_cast<QVBoxLayout*>(layout())->insertWidget(lst.indexOf(widget), widget);
        } break;

        case LayoutInfo::Grid: {
            QGridLayout *g = qt_cast<QGridLayout*>(layout());
            QPoint pos = widget->pos();
            int index = findItemAt(pos);
            if (index != -1) {
                QLayoutItem *item = layout()->itemAt(index);
                if (item && item->spacerItem()) {
                    int row, column, rowspan, colspan;
                    g->getItemPosition(index, &row, &column, &rowspan, &colspan);
                    g->takeAt(index);
                    g->addWidget(widget, row, column, rowspan, colspan);
                    delete item;
                    return;
                }
            }

            QMessageBox::information(m_widget, tr("Information"), tr("not implemented yet!"));
            widget->setParent(m_widget->parentWidget());
        } break;

        default:
            Q_ASSERT(0);
    } // end switch

#if 0
    qDebug() << "QLayoutSupport::insertWidget:" << widget;

    QPoint pos = widget->pos();
    int index = findItemAt(pos);

    qDebug() << "position:" << pos << "index:" << index
        << "isEmpty:" << (index != -1 ? layout()->itemAt(index)->spacerItem() : 0);

    if (!qt_cast<QGridLayout*>(layout())) {
        qWarning("not implemented yet!");
        Q_ASSERT(0);
    }

    if (index != -1) {
        QLayoutItem *item = layout()->itemAt(index);
        if (QSpacerItem *spacer = item->spacerItem()) {

            QGridLayout *g = qt_cast<QGridLayout*>(layout());
            int row, column, rowspan, colspan;
            g->getItemPosition(index, &row, &column, &rowspan, &colspan);

            layout()->takeAt(index);
            g->addWidget(widget, row, column, rowspan, colspan);
            delete item;

            update();
        } else {
            Q_ASSERT(item->layout() == 0);

            QGridLayout *g = qt_cast<QGridLayout*>(layout());
            int row, column, rowspan, colspan;
            g->getItemPosition(index, &row, &column, &rowspan, &colspan);
            insertRow(g, row);
            qDebug() << "===============> insert row at:" << row;
            insertWidget(widget);
        }
    }
#endif
}

void QLayoutSupport::insertRow(QGridLayout *gridLayout, int row)
{
    qDebug() << "insertRow:" << row;

    QHash<QLayoutItem*, QRect> infos;
    computeGridLayout(gridLayout, &infos);

    QHashMutableIterator<QLayoutItem*, QRect> it(infos);
    while (it.hasNext()) {
        it.next();

        QRect info = it.value();
        qDebug() << "widget:" << it.key()->widget() << "info:" << info;
        if (info.y() > row) {
            info.translate(0, 1);
            it.setValue(info);
            qDebug() << "move item:" << it.key()->widget();
        }
    }

    for (int c = 0; c < gridLayout->columnCount(); ++c) {
        infos.insert(new QSpacerItem(0,0), QRect(c, row + 1, 1, 1));
        qDebug() << "inserted fake spacer at:" << QRect(c, row + 1, 1, 1);
    }

    rebuildGridLayout(gridLayout, infos);
}

void QLayoutSupport::createEmptyCells(QGridLayout *gridLayout)
{
    Q_ASSERT(gridLayout);

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

        if (!item || !item->widget())
            gridLayout->addItem(new QSpacerItem(0,0), cell.first, cell.second);
    }
}

void QLayoutSupport::insertColumn(QGridLayout *gridLayout, int column)
{
    qDebug() << "insertColumn:" << column;

    QHash<QLayoutItem*, QRect> infos;
    computeGridLayout(gridLayout, &infos);

    QHashMutableIterator<QLayoutItem*, QRect> it(infos);
    while (it.hasNext()) {
        it.next();

        QRect info = it.value();

        if (info.x() > column) {
            info.translate(1, 0);
            it.setValue(info);
        }
    }

    for (int r = 0; r < gridLayout->rowCount(); ++r)
        infos.insert(new QSpacerItem(0, 0), QRect(column + 1, r, 1, 1));

    rebuildGridLayout(gridLayout, infos);
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

int QLayoutSupport::findItemAt(const QPoint &pos) const
{
    if (!layout())
        return -1;

    int index = 0;
    while (QLayoutItem *item = layout()->itemAt(index)) {
        if (item->geometry().contains(pos))
            return index;

        ++index;
    }

    return -1;
}

void QLayoutSupport::computeGridLayout(QGridLayout *gridLayout, QHash<QLayoutItem*, QRect> *l)
{
    int index = 0;
    while (QLayoutItem *item = gridLayout->itemAt(index)) {
        QRect info = itemInfo(index);
        l->insert(item, info);

        ++index;
    }
}

void QLayoutSupport::rebuildGridLayout(QGridLayout *gridLayout, const QHash<QLayoutItem*, QRect> &infos)
{
    { // take the items
        int index = 0;
        while (gridLayout->itemAt(index))
            gridLayout->takeAt(index);
    }

    delete gridLayout;
    QGridLayout *g = new QGridLayout(widget());

    QHashIterator<QLayoutItem*, QRect> it(infos);
    while (it.hasNext()) {
        it.next();

        QRect info = it.value();

        g->addItem(it.key(), info.y(), info.x(),
                info.height(), info.width());
    }

    g->invalidate();
    qApp->processEvents();
}
