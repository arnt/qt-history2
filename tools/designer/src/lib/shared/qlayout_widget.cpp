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

#include "qlayout_widget_p.h"
#include "qdesigner_command_p.h"
#include "layout_p.h"
#include "invisible_widget_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerLayoutDecorationExtension>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerWidgetFactoryInterface>

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

static qdesigner_internal::InvisibleWidget *createIndicator(QWidget *parent, const QPalette &p)
{
    qdesigner_internal::InvisibleWidget * rc = new qdesigner_internal::InvisibleWidget(parent);
    rc->setAutoFillBackground(true);
    rc->setPalette(p);
    rc->hide();
    return rc;
}

static const char *leftMarginC = "leftMargin";
static const char *rightMarginC = "rightMargin";
static const char *bottomMarginC = "bottomMargin";
static const char *topMarginC = "topMargin";
static const char *spacingC = "spacing";
static const char *horizontalSpacingC = "horizontalSpacing";
static const char *verticalSpacingC =  "verticalSpacing";

namespace {
    enum { ShiftValue = 1 };
}

namespace qdesigner_internal {

class FriendlyLayout: public QLayout
{
public:
    inline FriendlyLayout(): QLayout() { Q_ASSERT(0); }

    friend class ::QLayoutWidgetItem;
};
}

using namespace qdesigner_internal;

// ---- QLayoutSupport ----
QLayoutSupport::QLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent)  :
      QObject(parent),
      m_formWindow(formWindow),
      m_widget(widget),
      m_currentIndex(-1),
      m_currentInsertMode(QDesignerLayoutDecorationExtension::InsertWidgetMode)
{
    QPalette p;
    p.setColor(QPalette::Base, Qt::red);

    m_indicatorLeft = createIndicator(m_widget, p);
    m_indicatorTop = createIndicator(m_widget, p);
    m_indicatorRight = createIndicator(m_widget, p);
    m_indicatorBottom = createIndicator(m_widget, p);

    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(formWindow->core()->extensionManager(), m_widget)) {
        sheet->setChanged(sheet->indexOf(QLatin1String(leftMarginC)), true);
        sheet->setChanged(sheet->indexOf(QLatin1String(topMarginC)), true);
        sheet->setChanged(sheet->indexOf(QLatin1String(rightMarginC)), true);
        sheet->setChanged(sheet->indexOf(QLatin1String(bottomMarginC)), true);
        sheet->setChanged(sheet->indexOf(QLatin1String(spacingC)), true);
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
        const QRect info = itemInfo(index);
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
        const QRect info = itemInfo(index);
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

    if (QGridLayout *g = gridLayout())
        createEmptyCells(g);
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
    m_currentInsertMode = QDesignerLayoutDecorationExtension::InsertWidgetMode;

    QLayoutItem *item = layout()->itemAt(index);
    QRect g = extendedGeometry(index);

    const int dx = g.right() - pos.x();
    const int dy = g.bottom() - pos.y();

    const int dx1 = pos.x() - g.x();
    const int dy1 = pos.y() - g.y();

    const int mx = qMin(dx, dx1);
    const int my = qMin(dy, dy1);

    const bool isVertical = mx < my;

    // ### cleanup
    if (isEmptyItem(item)) {
        QPalette p;
        p.setColor(QPalette::Window, Qt::red);
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

        if (QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout())) {
            m_currentInsertMode = QDesignerLayoutDecorationExtension::InsertWidgetMode;
            int row, column, rowspan, colspan;
            gridLayout->getItemPosition(m_currentIndex, &row, &column, &rowspan, &colspan);
            m_currentCell = qMakePair(row, column);
        } else {
            qDebug("Warning: found a fake spacer inside a vbox layout");
            m_currentCell = qMakePair(0, 0);
        }
    } else {
        QPalette p;
        p.setColor(QPalette::Window, Qt::blue);
        m_indicatorRight->setPalette(p);
        m_indicatorBottom->setPalette(p);

        const QRect r(layout()->geometry().topLeft(), layout()->parentWidget()->size());
        if (isVertical) {
            m_indicatorBottom->hide();

            if (!qobject_cast<QVBoxLayout*>(layout())) {
                m_indicatorRight->setGeometry((mx == dx1) ? g.x() : g.right(), 0, 2, r.height());
                m_indicatorRight->show();
                m_indicatorRight->raise();

                const int incr = (mx == dx1) ? 0 : +1;

                if (qobject_cast<QGridLayout*>(layout())) {
                    m_currentInsertMode = QDesignerLayoutDecorationExtension::InsertColumnMode;

                    QRect info = itemInfo(m_currentIndex);
                    m_currentCell = qMakePair(info.top(), incr ? info.right() + 1 : info.left());
                } else if (QBoxLayout *box = qobject_cast<QBoxLayout*>(layout())) {
                    m_currentCell = qMakePair(0, box->indexOf(item->widget()) + incr);
                }
            }
        } else {
            m_indicatorRight->hide();

            if (!qobject_cast<QHBoxLayout*>(layout())) {
                m_indicatorBottom->setGeometry(r.x(), (my == dy1) ? g.y() : g.bottom(), r.width(), 2);
                m_indicatorBottom->show();
                m_indicatorBottom->raise();

                const int incr = (my == dy1) ? 0 : +1;

                if (qobject_cast<QGridLayout*>(layout())) {
                    m_currentInsertMode = QDesignerLayoutDecorationExtension::InsertRowMode;

                    QRect info = itemInfo(m_currentIndex);
                    m_currentCell = qMakePair(incr ? info.bottom() + 1 : info.top(), info.left());
                } else if (QBoxLayout *box = qobject_cast<QBoxLayout*>(layout())) {
                    m_currentCell = qMakePair(box->indexOf(item->widget()) + incr, 0);
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

QDesignerFormEditorInterface *QLayoutSupport::core() const
{
    return formWindow()->core();
}

void QLayoutSupport::removeWidget(QWidget *widget)
{
    LayoutInfo::Type layoutType = LayoutInfo::layoutType(core(), m_widget);

    switch (layoutType) {
        case LayoutInfo::Grid: {
            const int index = indexOf(widget);
            if (index != -1) {
                QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
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

QList<QWidget*> QLayoutSupport::widgets(QLayout *layout) const
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
    QDesignerFormEditorInterface *core = formWindow()->core();
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
            Q_ASSERT(index != -1);

            insertWidget(index, widget);
        } break;

        default: {
#ifdef QD_DEBUG
            qDebug() << "expected a layout here!";
            Q_ASSERT(0);
#endif
        }
    } // end switch
}

int QLayoutSupport::findItemAt(int at_row, int at_column) const
{
    if (QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout()))
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

QLayoutWidgetItem::QLayoutWidgetItem(QWidget *widget)
    : QWidgetItem(widget)
{
}

void QLayoutWidgetItem::setGeometry(const QRect &r)
{
    widget()->setGeometry(r);
}

QSize QLayoutWidgetItem::sizeHint() const
{
    if (QLayout *l = theLayout())
        return l->sizeHint();

    return QWidgetItem::sizeHint();
}

QSize QLayoutWidgetItem::minimumSize() const
{
    if (QLayout *l = theLayout())
        return l->minimumSize();

    return QWidgetItem::minimumSize();
}

QSize QLayoutWidgetItem::maximumSize() const
{
    if (QLayout *l = theLayout())
        return l->maximumSize();

    return QWidgetItem::maximumSize();
}

Qt::Orientations QLayoutWidgetItem::expandingDirections() const
{
    if (QLayout *l = theLayout())
        return l->expandingDirections();

    return QWidgetItem::expandingDirections();
}

void QLayoutWidgetItem::addTo(QLayout *layout)
{
    static_cast<qdesigner_internal::FriendlyLayout*>(layout)->addChildWidget(widget());
}

bool QLayoutWidgetItem::hasHeightForWidth() const
{
    if (QLayout *l = theLayout())
        return l->hasHeightForWidth();

    return QWidgetItem::hasHeightForWidth();
}

int QLayoutWidgetItem::heightForWidth(int w) const
{
    if (QLayout *l = theLayout())
        return l->heightForWidth(w);

    return QWidgetItem::heightForWidth(w);
}

int QLayoutWidgetItem::minimumHeightForWidth(int w) const
{
    if (QLayout *l = theLayout())
        return l->minimumHeightForWidth(w);

    return QWidgetItem::minimumHeightForWidth(w);
}

void QLayoutWidgetItem::removeFrom(QLayout *layout)
{
    Q_UNUSED(layout);
}

void QLayoutSupport::insertWidget(int index, QWidget *widget)
{
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
    QLayoutItem *item = gridLayout->itemAt(index);

    if (!item || !isEmptyItem(item)) {
        qDebug() << "the cell is not empty";
        return;
    }

    int row, column, rowspan, colspan;
    gridLayout->getItemPosition(index, &row, &column, &rowspan, &colspan);
    gridLayout->takeAt(index);
    add_to_grid_layout(gridLayout, widget, row, column, rowspan, colspan);
    delete item;
}

void QLayoutSupport::createEmptyCells(QGridLayout *&gridLayout)
{
    Q_ASSERT(gridLayout);

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

    QMap<QPair<int,int>, QLayoutItem*> cells;

    for (int r = 0; r < gridLayout->rowCount(); ++r) {
        for (int c = 0; c < gridLayout->columnCount(); ++c) {
            const QPair<int,int> cell = qMakePair(r, c);
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
                const QPair<int,int> cell = qMakePair(r, c);
                cells[cell] = item;

                if (!item) {
                    /* skip */
                } else if (item->layout()) {
                    qDebug("unexpected layout");
                } else if (item->spacerItem()) {
                    qDebug("unexpected spacer");
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

    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
    Q_ASSERT(gridLayout);

    if (gridLayout->rowCount() == row) {
        gridLayout->addItem(new QSpacerItem(20, 20), gridLayout->rowCount(), 0);
    }

    gridLayout = qobject_cast<QGridLayout*>(layout());
    Q_ASSERT(gridLayout);

    createEmptyCells(gridLayout);

    layout()->activate();
}

void QLayoutSupport::insertColumn(int column)
{
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
    if (!gridLayout)
        return;

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

    gridLayout = qobject_cast<QGridLayout*>(layout());
    Q_ASSERT(gridLayout);

    if (gridLayout->columnCount() == column) {
        gridLayout->addItem(new QSpacerItem(20, 20), 0, gridLayout->columnCount());
    }

    gridLayout = qobject_cast<QGridLayout*>(layout());
    Q_ASSERT(gridLayout);

    createEmptyCells(gridLayout);

    layout()->activate();
}

QRect QLayoutSupport::itemInfo(int index) const
{
    Q_ASSERT(layout());

    if (QGridLayout *l = qobject_cast<QGridLayout*>(layout())) {
        int row, column, rowSpan, columnSpan;
        l->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
        return QRect(column, row, columnSpan, rowSpan);
    } else if (qobject_cast<QHBoxLayout*>(layout())) {
        return QRect(index, 0, 1, 1);
    } else if (qobject_cast<QVBoxLayout*>(layout())) {
        return QRect(0, index, 1, 1);
    } else {
        Q_ASSERT(0); // ### not supported yet!
        return QRect();
    }
}

QRect QLayoutSupport::extendedGeometry(int index) const
{
    QLayoutItem *item = layout()->itemAt(index);
    QRect g = item->geometry();

    const QRect info = itemInfo(index);

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

    if (QVBoxLayout *vbox = qobject_cast<QVBoxLayout*>(layout())) {
        if (vbox->itemAt(index+1) == 0) {
            QPoint bottomRight = g.bottomRight();
            bottomRight.ry() = layout()->geometry().bottom();
            g.setBottomRight(bottomRight);
        }
    } else if (QHBoxLayout *hbox = qobject_cast<QHBoxLayout*>(layout())) {
        if (hbox->itemAt(index+1) == 0) {
            QPoint bottomRight = g.bottomRight();
            bottomRight.rx() = layout()->geometry().right();
            g.setBottomRight(bottomRight);
        }
    } else if (QGridLayout *grid = qobject_cast<QGridLayout*>(layout())) {
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

        const QRect g = item->geometry();

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
        const QRect info = itemInfo(index);
        l->insert(item, info);

        ++index;
    }
}

void QLayoutSupport::rebuildGridLayout(QHash<QLayoutItem*, QRect> *infos)
{
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
    int leftMargin, topMargin, rightMargin, bottomMargin;
    leftMargin = topMargin = rightMargin = bottomMargin = 0;
    int horizSpacing = gridLayout->horizontalSpacing();
    int vertSpacing = gridLayout->verticalSpacing();
    bool leftMarginChanged, topMarginChanged, rightMarginChanged, bottomMarginChanged, horizSpacingChanged, vertSpacingChanged;
    leftMarginChanged = topMarginChanged = rightMarginChanged = bottomMarginChanged = horizSpacingChanged = vertSpacingChanged = false;

    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), gridLayout);
    if (sheet) {
        const int leftMarginIndex = sheet->indexOf(QLatin1String(leftMarginC));
        const int topMarginIndex  = sheet->indexOf(QLatin1String(topMarginC));
        const int rightMarginIndex = sheet->indexOf(QLatin1String(rightMarginC));
        const int bottomMarginIndex = sheet->indexOf(QLatin1String((bottomMarginC)));
        const int horizSpacingIndex = sheet->indexOf(QLatin1String(horizontalSpacingC));
        const int vertSpacingIndex = sheet->indexOf(QLatin1String(verticalSpacingC));
        Q_ASSERT(leftMarginIndex != -1 && topMarginIndex  != -1 && rightMarginIndex   != -1 && bottomMargin  != -1 && horizSpacingIndex != -1 && vertSpacingIndex  != -1);
        leftMargin = sheet->property(leftMarginIndex).toInt();
        topMargin = sheet->property(topMarginIndex).toInt();
        rightMargin = sheet->property(rightMarginIndex).toInt();
        bottomMargin = sheet->property(bottomMarginIndex).toInt();
        horizSpacing = sheet->property(horizSpacingIndex).toInt();
        vertSpacing = sheet->property(vertSpacingIndex).toInt();

        leftMarginChanged = sheet->isChanged(leftMarginIndex);
        topMarginChanged = sheet->isChanged(topMarginIndex);
        rightMarginChanged = sheet->isChanged(rightMarginIndex);
        bottomMarginChanged = sheet->isChanged(bottomMarginIndex);
        horizSpacingChanged = sheet->isChanged(horizSpacingIndex);
        vertSpacingChanged = sheet->isChanged(vertSpacingIndex);
    }

    { // take the items
        int index = 0;
        while (gridLayout->itemAt(index))
            gridLayout->takeAt(index);
    }

    Q_ASSERT(gridLayout == m_widget->layout());

    LayoutInfo::deleteLayout(core, m_widget);

    gridLayout = (QGridLayout*) core->widgetFactory()->createLayout(m_widget, 0, LayoutInfo::Grid);

    QHashIterator<QLayoutItem*, QRect> it(*infos);
    while (it.hasNext()) {
        it.next();

        const QRect info = it.value();

        gridLayout->addItem(it.key(), info.y(), info.x(),
                info.height(), info.width());
    }

    QDesignerPropertySheetExtension *newSheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), gridLayout);
    if (sheet && newSheet) {
        const int leftMarginIndex = newSheet->indexOf(QLatin1String(leftMarginC));
        const int topMarginIndex  = newSheet->indexOf(QLatin1String(topMarginC));
        const int rightMarginIndex = newSheet->indexOf(QLatin1String(rightMarginC));
        const int bottomMarginIndex = newSheet->indexOf(QLatin1String((bottomMarginC)));
        const int horizSpacingIndex = newSheet->indexOf(QLatin1String(horizontalSpacingC));
        const int vertSpacingIndex = newSheet->indexOf(QLatin1String(verticalSpacingC));
        Q_ASSERT(leftMarginIndex != -1 && topMarginIndex  != -1 && rightMarginIndex   != -1 && bottomMargin  != -1 && horizSpacingIndex != -1 && vertSpacingIndex  != -1);

        newSheet->setProperty(leftMarginIndex, leftMargin);
        newSheet->setChanged(leftMarginIndex, leftMarginChanged);
        newSheet->setProperty(topMarginIndex, topMargin);
        newSheet->setChanged(topMarginIndex, topMarginChanged);
        newSheet->setProperty(rightMarginIndex,  rightMargin);

        newSheet->setChanged(topMarginIndex, topMarginChanged);
        newSheet->setProperty(rightMarginIndex, rightMargin);
        newSheet->setChanged(rightMarginIndex, rightMarginChanged);
        newSheet->setProperty(bottomMarginIndex, bottomMargin);
        newSheet->setChanged(bottomMarginIndex, bottomMarginChanged);

        newSheet->setProperty(horizSpacingIndex, horizSpacing);
        newSheet->setChanged(horizSpacingIndex, horizSpacingChanged);
        newSheet->setProperty(vertSpacingIndex, vertSpacing);
        newSheet->setChanged(vertSpacingIndex, vertSpacingChanged);
    }
}

QLayoutWidget::QLayoutWidget(QDesignerFormWindowInterface *formWindow, QWidget *parent)
    : QWidget(parent), m_formWindow(formWindow),
      m_support(formWindow, this), m_leftMargin(0), m_topMargin(0), m_rightMargin(0), m_bottomMargin(0)
{
}

void QLayoutWidget::paintEvent(QPaintEvent*)
{
    if (!m_formWindow->hasFeature(QDesignerFormWindowInterface::GridFeature))
        return;

    if (m_formWindow->currentTool() != 0)
        return;

    // only draw red borders if we're editting widgets

    QPainter p(this);

    if (layout() != 0) {
        p.setPen(QPen(QColor(255, 0, 0, 35), 1));

        int index = 0;

        while (QLayoutItem *item = layout()->itemAt(index)) {
            ++index;

            if (item->spacerItem())
                p.drawRect(item->geometry());
        }

    }

    p.setPen(QPen(Qt::red, 1));
    p.drawRect(0, 0, width() - 1, height() - 1);
}

void QLayoutWidget::updateMargin()
{
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

            if (layout() && LayoutInfo::layoutType(formWindow()->core(), parentWidget()) == LayoutInfo::NoLayout) {
                resize(layout()->totalMinimumSize().expandedTo(size()));
            }

            update();

            return true;
        }

        default:
            break;
    }

    return QWidget::event(e);
}

int QLayoutWidget::layoutLeftMargin() const
{
    if (m_leftMargin < 0 && layout()) {
        int margin;
        layout()->getContentsMargins(&margin, 0, 0, 0);
        return margin;
    }
    return m_leftMargin;
}

void QLayoutWidget::setLayoutLeftMargin(int layoutMargin)
{
    m_leftMargin = layoutMargin;
    if (layout()) {
        int newMargin = m_leftMargin;
        if (newMargin >= 0 && newMargin < ShiftValue)
            newMargin = ShiftValue;
        int left, top, right, bottom;
        layout()->getContentsMargins(&left, &top, &right, &bottom);
        layout()->setContentsMargins(newMargin, top, right, bottom);
    }
}

int QLayoutWidget::layoutTopMargin() const
{
    if (m_topMargin < 0 && layout()) {
        int margin;
        layout()->getContentsMargins(0, &margin, 0, 0);
        return margin;
    }
    return m_topMargin;
}

void QLayoutWidget::setLayoutTopMargin(int layoutMargin)
{
    m_topMargin = layoutMargin;
    if (layout()) {
        int newMargin = m_topMargin;
        if (newMargin >= 0 && newMargin < ShiftValue)
            newMargin = ShiftValue;
        int left, top, right, bottom;
        layout()->getContentsMargins(&left, &top, &right, &bottom);
        layout()->setContentsMargins(left, newMargin, right, bottom);
    }
}

int QLayoutWidget::layoutRightMargin() const
{
    if (m_rightMargin < 0 && layout()) {
        int margin;
        layout()->getContentsMargins(0, 0, &margin, 0);
        return margin;
    }
    return m_rightMargin;
}

void QLayoutWidget::setLayoutRightMargin(int layoutMargin)
{
    m_rightMargin = layoutMargin;
    if (layout()) {
        int newMargin = m_rightMargin;
        if (newMargin >= 0 && newMargin < ShiftValue)
            newMargin = ShiftValue;
        int left, top, right, bottom;
        layout()->getContentsMargins(&left, &top, &right, &bottom);
        layout()->setContentsMargins(left, top, newMargin, bottom);
    }
}

int QLayoutWidget::layoutBottomMargin() const
{
    if (m_bottomMargin < 0 && layout()) {
        int margin;
        layout()->getContentsMargins(0, 0, 0, &margin);
        return margin;
    }
    return m_bottomMargin;
}

void QLayoutWidget::setLayoutBottomMargin(int layoutMargin)
{
    m_bottomMargin = layoutMargin;
    if (layout()) {
        int newMargin = m_bottomMargin;
        if (newMargin >= 0 && newMargin < ShiftValue)
            newMargin = ShiftValue;
        int left, top, right, bottom;
        layout()->getContentsMargins(&left, &top, &right, &bottom);
        layout()->setContentsMargins(left, top, right, newMargin);
    }
}
