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
    setSelectionMode(SingleSelection);
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
    case Qt::Key_Enter:
    case Qt::Key_Return:
        hide();
        emit itemSelected(currentItem());
        break;
    case Qt::Key_F4:
    case Qt::Key_Escape:
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
    q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    q->setFrameStyle(QFrame::NoFrame);
    q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    q->setSelectionMode(QAbstractItemView::SingleSelection);
    q->setCurrentItem(q->model()->index(0, 0, q->root()));
}

void QGenericComboBoxPrivate::handleReturnPressed()
{
    QLineEdit *lineEdit = qt_cast<QLineEdit *>(editor);
    if (lineEdit && !lineEdit->text().isEmpty()) {
        QString text = lineEdit->text();
        // check for duplicates (if not enabled) and quit
        if (!d->duplicatesEnabled && d->contains(text, QAbstractItemModel::Display))
            return;
        QModelIndex newItem;
        switch (insertionPolicy) {
        case QGenericComboBox::AtTop:
            newItem = q->insertItem(text, 0);
            break;
        case QGenericComboBox::AtBottom:
            newItem = q->insertItem(text, q->model()->rowCount(q->root()));
            break;
        case QGenericComboBox::AtCurrent:
        case QGenericComboBox::AfterCurrent:
        case QGenericComboBox::BeforeCurrent:
            if (!q->model()->rowCount(q->root()) || !q->currentItem().isValid())
                newItem = q->insertItem(text, 0);
            else if (insertionPolicy == QGenericComboBox::AtCurrent)
                q->changeItem(text, q->currentItem().row());
            else if (insertionPolicy == QGenericComboBox::AfterCurrent)
                newItem = q->insertItem(text, q->currentItem().row() + 1);
            else if (insertionPolicy == QGenericComboBox::BeforeCurrent)
                newItem = q->insertItem(text, q->currentItem().row());
            break;
        case QGenericComboBox::NoInsertion:
        default:
            break;
        }
        if (newItem.isValid())
            q->setCurrentItem(newItem);
    }
}

void QGenericComboBoxPrivate::handleTextChanged()
{
    if (!autoCompletion)
        return;
    QLineEdit *lineEdit = qt_cast<QLineEdit *>(editor);
    QString text = lineEdit->text();
    if (lineEdit && !text.isEmpty()) {
        if (lastKey != Qt::Key_Delete && lastKey != Qt::Key_Backspace) {
            QModelIndexList list
                = d->model->match(q->currentItem(), QAbstractItemModel::Display, text);
            if (!list.count())
                return;
            QString completed = d->model->data(list.first(),
                                               QAbstractItemModel::Display).toString();
            int start = completed.length();
            int length = text.length() - start; // negative length
            lineEdit->setText(completed);
            lineEdit->setSelection(start, length);
        }
    }
}

void QGenericComboBoxPrivate::itemSelected(const QModelIndex &item)
{
    if (item != q->currentItem()) {
        q->setCurrentItem(item);
    } else if (q->isEditable()) {
        QLineEdit *lineEdit = qt_cast<QLineEdit *>(editor);
        if (lineEdit)
            lineEdit->selectAll();
        q->itemDelegate()->setEditorData(editor, q->currentItem());
        q->emit activated(q->currentItem());
    }
}

/*
  \internal
  returns true if any item under \a root containes \a text in role \a role.
*/
bool QGenericComboBoxPrivate::contains(const QString &text, int role)
{
    return q->model()->match(q->model()->index(0, 0, q->root()),
                             role, text, 1, QAbstractItemModel::MatchExactly
                             |QAbstractItemModel::MatchCase).count() > 0;
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

bool QGenericComboBox::autoCompletion() const
{
    return d->autoCompletion;
}

void QGenericComboBox::setAutoCompletion(bool enable)
{
    d->autoCompletion = enable;
}

bool QGenericComboBox::duplicatesEnabled() const
{
    return d->duplicatesEnabled;
}

void QGenericComboBox::setDuplicatesEnabled(bool enable)
{
    d->duplicatesEnabled = enable;
}

QGenericComboBox::InsertionPolicy QGenericComboBox::insertionPolicy() const
{
    return d->insertionPolicy;
}

void QGenericComboBox::setInsertionPolicy(InsertionPolicy policy)
{
    d->insertionPolicy = policy;
}

bool QGenericComboBox::isEditable() const
{
    return d->editor != 0;
}

void QGenericComboBox::setEditable(bool editable)
{
    if (isEditable() == editable)
        return;

    if (editable) {
        QItemOptions options = viewOptions();
        options.itemRect = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
                                                          QStyle::SC_ComboBoxEditField);
        options.focus = hasFocus();
        d->editor = itemDelegate()->editor(QAbstractItemDelegate::AlwaysEdit,
                                           this, options, currentItem());
        QLineEdit *lineEdit = qt_cast<QLineEdit *>(d->editor);
        if (lineEdit) {
            connect(lineEdit, SIGNAL(textChanged(const QString&)),
                    this, SLOT(handleTextChanged()));
            connect(lineEdit, SIGNAL(textChanged(const QString&)),
                    this, SIGNAL(textChanged(const QString&)));
            connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(handleReturnPressed()));
            lineEdit->installEventFilter(this);
        }
        itemDelegate()->setEditorData(d->editor, currentItem());
        if (lineEdit)
            lineEdit->selectAll();
        setFocusProxy(d->editor);
        updateCurrentEditor();
    } else {
        itemDelegate()->releaseEditor(QAbstractItemDelegate::Cancelled, d->editor, currentItem());
        d->editor = 0;
        setFocusProxy(0);
    }
}

QModelIndex QGenericComboBox::insertItem(const QString &text, int row)
{
    QModelIndex item;
    if (model()->insertRows(row, root())) {
        item = model()->index(row, 0, root());
        model()->setData(item, QAbstractItemModel::Display, text);
    }
    return item;
}

QModelIndex QGenericComboBox::insertItem(const QIconSet &icon, int row)
{
    QModelIndex item;
    if (model()->insertRows(row, root())) {
        item = model()->index(row, 0, root());
        model()->setData(item, QAbstractItemModel::Decoration, icon);
    }
    return item;
}

QModelIndex QGenericComboBox::insertItem(const QString &text, const QIconSet &icon, int row)
{
    QModelIndex item;
    if (model()->insertRows(row, root())) {
        item = model()->index(row, 0, root());
        QMap<int, QVariant> values;
        values.insert(QAbstractItemModel::Display, text);
        values.insert(QAbstractItemModel::Decoration, icon);
        model()->setItemData(item, values);
    }
    return item;
}

QModelIndex QGenericComboBox::changeItem(const QString &text, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        model()->setData(item, QAbstractItemModel::Display, text);
    }
    return item;
}

QModelIndex QGenericComboBox::changeItem(const QIconSet &icon, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        model()->setData(item, QAbstractItemModel::Decoration, icon);
    }
    return item;
}

QModelIndex QGenericComboBox::changeItem(const QString &text, const QIconSet &icon, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        QMap<int, QVariant> map;
        map.insert(QAbstractItemModel::Display, text);
        map.insert(QAbstractItemModel::Decoration, icon);
        model()->setItemData(item, map);
    }
    return item;
}

QGenericListView *QGenericComboBox::listView() const
{
    return d->listView;
}

QModelIndex QGenericComboBox::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                         Qt::ButtonState)
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

QSize QGenericComboBox::sizeHint() const
{
    if (isVisible() && d->sizeHint.isValid())
	return d->sizeHint;

    QFontMetrics fm = fontMetrics();
    d->sizeHint.setWidth(fm.width("XXX"));
    d->sizeHint.setHeight(fontMetrics().lineSpacing());

    QSize itemSize;
    for (int i = 0; i < model()->rowCount(root()); i++) {
        itemSize = itemSizeHint(model()->index(i, 0, root()));
        if (itemSize.width() > d->sizeHint.width())
            d->sizeHint.setWidth(itemSize.width());
        if (itemSize.height() > d->sizeHint.height())
            d->sizeHint.setHeight(itemSize.height());
    }
    // padding for selectionrect to cover all
    d->sizeHint.setHeight(d->sizeHint.height() + 2);

    d->sizeHint = (style().sizeFromContents(QStyle::CT_ComboBox, this,
                                            d->sizeHint).
                   expandedTo(QApplication::globalStrut()));

    return d->sizeHint;
}

void QGenericComboBox::updateCurrentEditor()
{
    if (!isEditable())
        return;
    QItemOptions options = viewOptions();
    options.itemRect = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
                                                      QStyle::SC_ComboBoxEditField);
    itemDelegate()->updateEditorGeometry(d->editor, options, currentItem());
}

void QGenericComboBox::currentChanged(const QModelIndex &old, const QModelIndex &current)
{
    QAbstractItemView::currentChanged(old, current);
    qDebug(QString("currentChanged: old: %1 %2, current: %3,%4, has %5")
           .arg(old.row()).arg(old.column())
           .arg(current.row()).arg(current.column())
           .arg(model()->parent(old).isValid() ? " parent" : " no parent").latin1());
    if (isEditable()) {
        itemDelegate()->setEditorData(d->editor, currentItem());
        QLineEdit *lineEdit = qt_cast<QLineEdit *>(d->editor);
        if (lineEdit)
            lineEdit->selectAll();
    }
    emit activated(currentItem());
}

void QGenericComboBox::contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QAbstractItemView::contentsChanged(topLeft, bottomRight);
    qDebug(QString("contentsChanged tl: %1 %2, br: %3,%4, has %5")
           .arg(topLeft.row()).arg(topLeft.column())
           .arg(bottomRight.row()).arg(bottomRight.column())
           .arg(model()->parent(topLeft).isValid() ? " parent" : " no parent").latin1());
}

void QGenericComboBox::contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight){
    QAbstractItemView::contentsInserted(topLeft, bottomRight);
    qDebug(QString("contentsInserted tl: %1 %2, br: %3,%4, has %5")
           .arg(topLeft.row()).arg(topLeft.column())
           .arg(bottomRight.row()).arg(bottomRight.column())
           .arg(model()->parent(topLeft).isValid() ? " parent" : " no parent").latin1());
}

void QGenericComboBox::contentsRemoved(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QAbstractItemView::contentsRemoved(topLeft, bottomRight);
    qDebug(QString("contentsRemoved tl: %1 %2, br: %3,%4, has %5")
           .arg(topLeft.row()).arg(topLeft.column())
           .arg(bottomRight.row()).arg(bottomRight.column())
           .arg(model()->parent(topLeft).isValid() ? " parent" : " no parent").latin1());
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

void QGenericComboBox::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
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

    QRect delegateRect = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
                                                      QStyle::SC_ComboBoxEditField);
    // delegate paints content
    QItemOptions options = viewOptions();

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

bool QGenericComboBox::eventFilter (QObject *watched, QEvent *e)
{
    if (watched == d->editor && e->type() == QEvent::KeyPress)
        d->lastKey = static_cast<QKeyEvent*>(e)->key();
    return false;
}

bool QGenericComboBox::startEdit(const QModelIndex &,
                                 QAbstractItemDelegate::StartEditAction,
                                 QEvent *)
{
    return false;
}

void QGenericComboBox::updateGeometries()
{
    updateCurrentEditor();
}

void QGenericComboBox::popupListView()
{
    if (model()->rowCount(root()) <= 0)
        return;
    if (!d->listView) {
        d->listView = new ComboListView(model(), this);
        d->listView->setParent(this, Qt::WType_Popup);
        connect(d->listView, SIGNAL(itemSelected(const QModelIndex &)),
                this, SLOT(itemSelected(const QModelIndex &)));
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
    listRect.setHeight(itemHeight * qMin(d->sizeLimit, model()->rowCount(root())) +
                       d->listView->spacing());

    // make sure the widget fits on screen
    //### do horizontally as well
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

#include "moc_qgenericcombobox.cpp"
