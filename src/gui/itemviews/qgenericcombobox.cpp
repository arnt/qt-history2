#include "qgenericcombobox.h"
#include <qpainter.h>
#include <qlineedit.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qgenericlistview.h>
#include <private/qgenericcombobox_p.h>

#define d d_func()
#define q q_func()

ComboListView::ComboListView(QAbstractItemModel *model, QWidget *parent) :
    QGenericListView(model, parent), ignoreMousePress(false)
{
    setMouseTracking(true);
    setSelectionMode(Single);
    setFrameStyle(QFrame::Box|QFrame::Plain);
    setLineWidth(1);
    setStartEditActions(QAbstractItemDelegate::NeverEdit);
}

/*! internal
  needed to skip mousePressEvents for replayed events generated when the ComboListView is closed/hidden.
*/
bool ComboListView::ignoreNextMousePress()
{
    if (ignoreMousePress) {
        ignoreMousePress = false;
        return true;
    }
    return false;
}

void ComboListView::mouseReleaseEvent(QMouseEvent *)
{
    hide();
    emit itemSelected(currentItem());
}

void ComboListView::mouseMoveEvent(QMouseEvent *e)
{
    QMouseEvent tmp(e->type(), e->pos(), e->button(), e->state() | Qt::LeftButton);
    QGenericListView::mouseMoveEvent(&tmp);
}

void ComboListView::keyPressEvent(QKeyEvent *e)
{
    QGenericListView::keyPressEvent(e);
    switch (e->key()) {
    case Key_Enter:
    case Key_Return:
        hide();
        emit itemSelected(currentItem());
        break;
    case Key_F4:
    case Key_Escape:
        hide();
        break;
    default:
        break;
    }
}

bool ComboListView::event(QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
        // hide if mousePress outside listview
        if (!rect().contains(mouseEvent->pos())) {
            QGenericComboBox *comboBox = qt_cast<QGenericComboBox *>(parentWidget());
            if (comboBox) {
                QRect arrowRect = comboBox->style().querySubControlMetrics(
                    QStyle::CC_ComboBox, comboBox, QStyle::SC_ComboBoxArrow);
                QRect globalArrowRect(comboBox->mapToGlobal(arrowRect.topLeft()),
                                      comboBox->mapToGlobal(arrowRect.bottomRight()));
                if (globalArrowRect.contains(mouseEvent->globalPos()))
                    ignoreMousePress = true; // ignore next mousepress (replayed) if click on arrowrect
            }
            hide();
            return true;
        }
    }
    return QGenericListView::event(e);
}

QGenericComboBox::QGenericComboBox(QAbstractItemModel *model, QWidget *parent) :
    QAbstractItemView(*new QGenericComboBoxPrivate(), model, parent)
{
    d->init();

}

QGenericComboBox::QGenericComboBox(QGenericComboBoxPrivate &dd,
                                   QAbstractItemModel *model, QWidget *parent) :
    QAbstractItemView(dd, model, parent)
{
    d->init();
}

void QGenericComboBoxPrivate::init()
{
    q->setFrameStyle(QFrame::NoFrame);
    q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    q->setSelectionMode(QAbstractItemView::Single);
    q->setCurrentItem(q->model()->index(0, 0, q->root()));
}

QGenericComboBox::~QGenericComboBox()
{
    delete d->listView;
    d->listView = 0;
}


int QGenericComboBox::sizeLimit() const
{
    return d->sizeLimit;
}

void QGenericComboBox::setSizeLimit(int limit)
{
    if (limit > 0)
        d->sizeLimit = limit;
}

QModelIndex QGenericComboBox::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                         ButtonState)
{
    QModelIndex current = selectionModel()->currentItem();
    if (!current.isValid())
        return QModelIndex();

    switch (cursorAction) {
    case MovePageUp:
    case MoveUp:
        return model()->index(current.row() - 1, current.column(), root());
    case MovePageDown:
    case MoveDown:
        return model()->index(current.row() + 1, current.column(), root());
    case MoveHome:
        return model()->index(0, 0, root());
    case MoveEnd:
        return model()->index(model()->rowCount(model()->parent(current)) - 1,
                              current.column(),
                              root());
    case MoveLeft:
    case MoveRight:
    default:
        return current;
    }
}

QModelIndex QGenericComboBox::itemAt(int x, int y) const
{
    if (d->viewport->rect().contains(x, y))
        return selectionModel()->currentItem();
    else
        return QModelIndex();
}

int QGenericComboBox::horizontalOffset() const
{
    return 0;
}

int QGenericComboBox::verticalOffset() const
{
    return 0;
}

QRect QGenericComboBox::itemViewportRect(const QModelIndex &item) const
{
    if (item == currentItem())
        return d->viewport->rect();
    else
        return QRect();
}

void QGenericComboBox::ensureItemVisible(const QModelIndex &item)
{
    if (item.isValid() && item != currentItem()) {
        setCurrentItem(item);
        d->viewport->update();
    }
}

void QGenericComboBox::setSelection(const QRect &rect, int command)
{
    if (rect.intersects(d->viewport->rect()))
        selectionModel()->select(QItemSelection(currentItem(), currentItem(), model()), command);
}

QRect QGenericComboBox::selectionViewportRect(const QItemSelection &selection) const
{
    if (selection.contains(currentItem(), model()))
        return itemViewportRect(currentItem());
    return QRect();
}

void QGenericComboBox::paintEvent(QPaintEvent *)
{
    QPainter painter(d->viewport);
    QRect comboRect = d->viewport->rect();
    painter.setPen(palette().color(QPalette::Text));

    // paint the combobox except content
    QStyle::SFlags flags = QStyle::Style_Default;
    flags |= (isEnabled() ?  QStyle::Style_Enabled : 0);
    flags |= (hasFocus() ? QStyle::Style_HasFocus : 0);
    style().drawComplexControl(QStyle::CC_ComboBox, &painter, this, comboRect, palette(),
                               flags, (uint)QStyle::SC_All, QStyle::SC_ComboBoxArrow);

    // delegate paints content
    QRect delegateRect = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
                                                      QStyle::SC_ComboBoxEditField);
    QItemOptions options;
    getViewOptions(&options);

    QModelIndex current = currentItem();
    if (current.isValid()) {
        options.itemRect = delegateRect;
        options.selected = selectionModel()->isSelected(current);
        options.focus = q->hasFocus();
        itemDelegate()->paint(&painter, options, current);
    }
}

void QGenericComboBox::mousePressEvent(QMouseEvent *e)
{
    // prevent popup for replayed events
    if (d->listView && d->listView->ignoreNextMousePress())
        return;

    QRect arrowRect = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
                                                     QStyle::SC_ComboBoxArrow);
    if (arrowRect.contains(e->pos()) && (!d->listView || !d->listView->isVisible()))
        popupListView();
}

bool QGenericComboBox::startEdit(const QModelIndex &item,
                                 QAbstractItemDelegate::StartEditAction action,
                                 QEvent *event)
{
    if (d->editable)
        return QAbstractItemView::startEdit(item, action, event);
    else
        return false;
}

void QGenericComboBox::popupListView()
{
    if (model()->rowCount(root()) <= 0)
        return;
    if (!d->listView) {
        d->listView = new ComboListView(model(), this);
        d->listView->setParent(this, WType_Popup);
        connect(d->listView, SIGNAL(itemSelected(const QModelIndex &)),
                this, SLOT(setCurrentItem(const QModelIndex &)));
    }

    // set current item and select it
    if (d->listView->currentItem() != currentItem())
        d->listView->setCurrentItem(currentItem());
    if (!d->listView->selectionModel()->isSelected(d->listView->currentItem()))
        d->listView->selectionModel()->select(d->listView->currentItem(),
                                              QItemSelectionModel::ClearAndSelect);

    // use top item as height for complete listView
    int itemHeight = itemSizeHint(model()->index(0, 0, root())).height() + d->listView->spacing();
    QRect listRect = d->viewport->rect();
    listRect.setHeight(itemHeight * d->sizeLimit + d->listView->spacing());

    // make sure the widget fits on screen
    //### do horizontally as well?
    QRect screen = QApplication::desktop()->availableGeometry(this);
    QPoint below = d->viewport->mapToGlobal(d->viewport->rect().bottomLeft());
    int belowHeight = screen.bottom() - below.y();
    QPoint above = d->viewport->mapToGlobal(d->viewport->rect().topLeft());
    int aboveHeight = above.y() - screen.y();
    if (listRect.height() <= belowHeight) {
        listRect.moveTopLeft(below);
    } else if (listRect.height() <= aboveHeight) {
        listRect.moveBottomLeft(above);
    } else if (belowHeight >= aboveHeight) {
        listRect.setHeight(belowHeight);
        listRect.moveTopLeft(below);
    } else {
        listRect.setHeight(aboveHeight);
        listRect.moveBottomLeft(above);
    }

    d->listView->setGeometry(listRect);
    d->listView->ensureItemVisible(d->listView->currentItem());
    d->listView->raise();
    d->listView->show();
}
