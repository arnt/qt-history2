#include "qgenericcombobox.h"
#include <qpainter.h>
#include <qlineedit.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qgenericlistview.h>
#include <qitemdelegate.h>
#include <qmap.h>
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
    setFocusPolicy(Qt::StrongFocus);
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
    QWidget(*new QGenericComboBoxPrivate(), parent, 0)
{
    Q_ASSERT(model);
    d->model = model;
    d->init();
}

QGenericComboBox::QGenericComboBox(QGenericComboBoxPrivate &dd,
                                   QAbstractItemModel *model, QWidget *parent) :
    QWidget(dd, parent, 0)
{
    Q_ASSERT(model);
    d->model = model;
    d->init();
}

void QGenericComboBoxPrivate::init()
{
    q->setFocusPolicy(Qt::StrongFocus);
    q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    q->setCurrentItem(model->index(0, 0, root));
    delegate = new QItemDelegate(model, q);
    listView = new ComboListView(model, q);
    listView->setParent(q, Qt::WType_Popup);
    listView->setItemDelegate(delegate);
    QApplication::connect(listView, SIGNAL(itemSelected(const QModelIndex &)),
                          q, SLOT(itemSelected(const QModelIndex &)));
}

void QGenericComboBoxPrivate::updateLineEditGeometry()
{
    if (!lineEdit || !q->isVisible())
        return;

    QRect editorRect = q->style().querySubControlMetrics(QStyle::CC_ComboBox, q,
                                                         QStyle::SC_ComboBoxEditField);
    lineEdit->setGeometry(editorRect);
}

void QGenericComboBoxPrivate::returnPressed()
{
    if (lineEdit && !lineEdit->text().isEmpty()) {
        QString text = lineEdit->text();
        // check for duplicates (if not enabled) and quit
        if (!d->duplicatesEnabled && d->contains(text, QAbstractItemModel::Edit))
            return;
        QModelIndex newItem;
        switch (insertionPolicy) {
        case QGenericComboBox::AtTop:
            newItem = q->insertItem(text, 0);
            break;
        case QGenericComboBox::AtBottom:
            newItem = q->insertItem(text, model->rowCount(q->root()));
            break;
        case QGenericComboBox::AtCurrent:
        case QGenericComboBox::AfterCurrent:
        case QGenericComboBox::BeforeCurrent:
            if (!model->rowCount(q->root()) || !q->currentItem().isValid())
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

/*
  \internal

  handles auto completion
*/
void QGenericComboBoxPrivate::complete()
{
    if (d->skipCompletion || !lineEdit || !autoCompletion) {
        skipCompletion = false;
        return;
    }
    QString text = lineEdit->text();
    if (!text.isEmpty()) {
        QModelIndexList list
            = d->model->match(q->currentItem(), QAbstractItemModel::Edit, text);
        if (!list.count())
            return;
        QString completed = d->model->data(list.first(),
                                           QAbstractItemModel::Edit).toString();
        int start = completed.length();
        int length = text.length() - start; // negative length
        lineEdit->setText(completed);
        lineEdit->setSelection(start, length);
    }
}

void QGenericComboBoxPrivate::itemSelected(const QModelIndex &item)
{
    if (item != q->currentItem()) {
        q->setCurrentItem(item);
    } else if (q->isEditable()) {
        if (lineEdit) {
            lineEdit->selectAll();
            lineEdit->setText(model->data(q->currentItem(), QAbstractItemModel::Edit)
                              .toString());
        }
        q->emit activated(q->currentItem());
    }
}

/*
  \internal
  returns true if any item under \a root containes \a text in role \a role.
*/
bool QGenericComboBoxPrivate::contains(const QString &text, int role)
{
    return model->match(model->index(0, 0, q->root()),
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

int QGenericComboBox::count() const
{
    return model()->rowCount(root());
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
    return d->lineEdit != 0;
}

void QGenericComboBox::setEditable(bool editable)
{
    if (isEditable() == editable)
        return;

    if (editable) {
        setLineEdit(new QLineEdit(this));
    } else {
        delete d->lineEdit;
        d->lineEdit = 0;
    }
}

void QGenericComboBox::setLineEdit(QLineEdit *edit)
{
    if ( !edit ) {
	Q_ASSERT(edit != 0);
	return;
    }

    edit->setText(currentText());
    delete d->lineEdit;

    d->lineEdit = edit;
    if (d->lineEdit->parent() != this)
	d->lineEdit->setParent(this);
    connect(d->lineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
    connect(d->lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(complete()));
    d->lineEdit->setFrame(false);
    d->lineEdit->setAttribute(Qt::WA_CompositeChild);
    setAttribute(Qt::WA_CompositeParent);
    setFocusProxy(d->lineEdit);
    setInputMethodEnabled(true);
    d->updateLineEditGeometry();

    if (isVisible())
	d->lineEdit->show();

    update();
}

QLineEdit *QGenericComboBox::lineEdit() const
{
    return d->lineEdit;
}

/*!
    Returns the item delegate used by the combobox and the popup
    listview.

    \sa setItemDelegate()
*/
QAbstractItemDelegate *QGenericComboBox::itemDelegate() const
{
    return d->delegate;
}

/*!
    Sets the item delegate for the combobox and the popup listview to \a delegate.

    \sa itemDelegate()
*/
void QGenericComboBox::setItemDelegate(QAbstractItemDelegate *delegate)
{
    Q_ASSERT(delegate);
    if (delegate->model() != model()) {
         qWarning("QGenericComboBox::setItemDelegate() failed: Trying to set a delegate, "
                  "which works on a different model than the view.");
         return;
    }

    if (d->delegate && d->delegate->parent() == this)
        delete d->delegate;

    d->delegate = delegate;
    d->listView->setItemDelegate(d->delegate);
}

QAbstractItemModel *QGenericComboBox::model() const
{
    return d->model;
}

QModelIndex QGenericComboBox::root() const
{
    return QModelIndex(d->root);
}

void QGenericComboBox::setRoot(const QModelIndex &index)
{
    QModelIndex old = d->root;
    d->root = QPersistentModelIndex(index, d->model);
    emit rootChanged(old, index);
    update();
}

QModelIndex QGenericComboBox::currentItem() const
{
    return (QModelIndex)d->currentItem;
}

void QGenericComboBox::setCurrentItem(const QModelIndex &index)
{
    if (!index.isValid() || index == d->currentItem)
        return;
    QModelIndex old = d->currentItem;
    d->currentItem = QPersistentModelIndex(index, d->model);
    emit currentChanged(old, index);
    update();
}

QString QGenericComboBox::currentText() const
{
    if (d->lineEdit)
        return d->lineEdit->text();
    else if (currentItem().isValid())
	return model()->data(currentItem(), QAbstractItemModel::Edit).toString();
    else
	return QString::null;
}

void QGenericComboBox::setCurrentText(const QString& text)
{
    if (currentItem().isValid()) {
        model()->setData(currentItem(), QAbstractItemModel::Edit, text);
        if (d->lineEdit)
            d->lineEdit->setText(text);
    }
}

QModelIndex QGenericComboBox::insertItem(const QString &text, int row)
{
    QModelIndex item;
    if (model()->insertRows(row, root())) {
        item = model()->index(row, 0, root());
        model()->setData(item, QAbstractItemModel::Edit, text);
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
        values.insert(QAbstractItemModel::Edit, text);
        values.insert(QAbstractItemModel::Decoration, icon);
        model()->setItemData(item, values);
    }
    return item;
}

QModelIndex QGenericComboBox::changeItem(const QString &text, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        model()->setData(item, QAbstractItemModel::Edit, text);
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
        map.insert(QAbstractItemModel::Edit, text);
        map.insert(QAbstractItemModel::Decoration, icon);
        model()->setItemData(item, map);
    }
    return item;
}

QGenericListView *QGenericComboBox::listView() const
{
    return d->listView;
}

QSize QGenericComboBox::sizeHint() const
{
    if (isVisible() && d->sizeHint.isValid())
	return d->sizeHint;

    const QFontMetrics &fm = fontMetrics();
    d->sizeHint.setWidth(fm.width("XXX"));
    d->sizeHint.setHeight(fontMetrics().lineSpacing());

    QItemOptions options;
    options.editing = isEditable();
    options.selected = true;
    options.focus = q->hasFocus();

    QSize itemSize;
    for (int i = 0; i < model()->rowCount(root()); i++) {
        itemSize = d->delegate->sizeHint(fontMetrics(), options, model()->index(i, 0, root()));
        if (itemSize.width() > d->sizeHint.width())
            d->sizeHint.setWidth(itemSize.width());
        if (itemSize.height() > d->sizeHint.height())
            d->sizeHint.setHeight(itemSize.height());
    }
    d->sizeHint = (style().sizeFromContents(QStyle::CT_ComboBox, this, d->sizeHint)
                   .expandedTo(QApplication::globalStrut()));
    return d->sizeHint;
}

void QGenericComboBox::currentChanged(const QModelIndex &old, const QModelIndex &current)
{
    qDebug(QString("currentChanged: old: %1 %2, current: %3,%4, has %5")
           .arg(old.row()).arg(old.column())
           .arg(current.row()).arg(current.column())
           .arg(model()->parent(old).isValid() ? " parent" : " no parent").latin1());
    if (d->lineEdit)
        d->lineEdit->setText(model()->data(q->currentItem(), QAbstractItemModel::Edit)
                             .toString());
    emit activated(currentItem());
}

void QGenericComboBox::focusInEvent(QFocusEvent *)
{
    update();
}

void QGenericComboBox::focusOutEvent(QFocusEvent *)
{
    update();
}

void QGenericComboBox::resizeEvent(QResizeEvent *)
{
    d->updateLineEditGeometry();
}

void QGenericComboBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect comboRect = rect();
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
    QItemOptions options;

    QModelIndex current = currentItem();
    if (current.isValid()) {
        options.palette = palette();
        options.editing = isEditable();
        options.itemRect = delegateRect;
        options.focus = q->hasFocus();
        options.selected = options.focus;
        d->delegate->paint(&painter, options, current);
    }
}

void QGenericComboBox::mousePressEvent(QMouseEvent *e)
{
    // prevent popup for replayed events
    if (d->listView->ignoreNextMousePress())
        return;

    QRect arrowRect = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
                                                     QStyle::SC_ComboBoxArrow);
    if (arrowRect.contains(e->pos()) && (!d->listView || !d->listView->isVisible()))
        popupListView();
}

void QGenericComboBox::keyPressEvent(QKeyEvent *e)
{
    QModelIndex newIndex;
    switch (e->key()) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        // skip autoCompletion if Delete or Backspace has been pressed
        d->skipCompletion = true;
        e->ignore();
        break;
    case Qt::Key_PageUp:
    case Qt::Key_Up:
        newIndex = model()->index(currentItem().row() - 1, currentItem().column(), root());
        break;
    case Qt::Key_PageDown:
    case Qt::Key_Down:
        newIndex = model()->index(currentItem().row() + 1, currentItem().column(), root());
        break;
    case Qt::Key_Home:
        if (isEditable())
            e->ignore();
        else
            newIndex = model()->index(0, 0, root());
        break;
    case Qt::Key_End:
        if (isEditable())
            e->ignore();
        else
            newIndex = model()->index(model()->rowCount(root()) - 1, 0, root());
        break;
    default:
        if (!e->text().isEmpty() && !isEditable()) {
            d->listView->setCurrentItem(currentItem());
            d->listView->keyboardSearch(e->text());
            if (d->listView->currentItem().isValid()
                && d->listView->currentItem() != currentItem())
                setCurrentItem(d->listView->currentItem());
        }
        break;
    }

    if (newIndex.isValid())
        setCurrentItem(newIndex);
}

void QGenericComboBox::popupListView()
{
    if (model()->rowCount(root()) <= 0)
        return;

    // set current item and select it
    if (d->listView->currentItem() != currentItem())
        d->listView->setCurrentItem(currentItem());
    if (!d->listView->selectionModel()->isSelected(d->listView->currentItem()))
        d->listView->selectionModel()->select(d->listView->currentItem(),
                                              QItemSelectionModel::ClearAndSelect);

    // use top item as height for complete listView
    int itemHeight = d->listView->itemSizeHint(model()->index(0, 0, root())).height() + d->listView->spacing();
    QRect listRect = rect();
    listRect.setHeight(itemHeight * qMin(d->sizeLimit, model()->rowCount(root())) +
                       d->listView->spacing());

    // make sure the widget fits on screen
    //### do horizontally as well
    QRect screen = QApplication::desktop()->availableGeometry(this);
    QPoint below = mapToGlobal(rect().bottomLeft());
    int belowHeight = screen.bottom() - below.y();
    QPoint above = mapToGlobal(rect().topLeft());
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
