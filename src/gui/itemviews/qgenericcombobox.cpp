#include "qgenericcombobox.h"
#include <qpainter.h>
#include <qlineedit.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qgenericlistview.h>
#include <qitemdelegate.h>
#include <qmap.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <private/qgenericcombobox_p.h>

#define d d_func()
#define q q_func()

ListViewContainer::ListViewContainer(QGenericListView *listView, QWidget *parent)
    : QFrame(parent), list(listView), top(0), bottom(0)
{
    // setup container
    setFrameStyle(QFrame::Box|QFrame::Plain);
    setLineWidth(1);

    // setup the listview
    Q_ASSERT(list);
    list->setParent(this);
    list->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    list->viewport()->installEventFilter(this);
    setFocusProxy(list);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (style().styleHint(QStyle::SH_ComboBox_Popup))
        list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (style().styleHint(QStyle::SH_ComboBox_ListMouseTracking))
        list->setMouseTracking(true);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setFrameStyle(QFrame::NoFrame);
    list->setLineWidth(0);
    list->setSpacing(0);
    list->setStartEditActions(QAbstractItemDelegate::NeverEdit);
    list->setFocusPolicy(Qt::StrongFocus);
    connect(list->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(updateScrollers()));
    connect(list->verticalScrollBar(), SIGNAL(rangeChanged(int, int)),
            this, SLOT(updateScrollers()));

    // add widgets to layout and create scrollers if needed
    QBoxLayout *layout =  new QBoxLayout(QBoxLayout::TopToBottom, this);
    if (style().styleHint(QStyle::SH_ComboBox_Popup)) {
        top = new Scroller(QAbstractSlider::SliderSingleStepSub, this);
        bottom = new Scroller(QAbstractSlider::SliderSingleStepAdd, this);
    }
    if (top) {
        layout->addWidget(top);
        connect(top, SIGNAL(doScroll(int)), this, SLOT(scrollListView(int)));
    }
    layout->addWidget(list);
    if (bottom) {
        layout->addWidget(bottom);
        connect(bottom, SIGNAL(doScroll(int)), this, SLOT(scrollListView(int)));
    }
}

void ListViewContainer::scrollListView(int action)
{
    list->verticalScrollBar()->triggerAction(static_cast<QAbstractSlider::SliderAction>(action));
}

/*
  \internal

  Hides or shows the scrollers when we emulate a popupmenu
*/
void ListViewContainer::updateScrollers()
{
    if (!top || !bottom)
        return;
    if (list->verticalScrollBar()->minimum() < list->verticalScrollBar()->maximum()) {
        bool needTop = list->verticalScrollBar()->value() > list->verticalScrollBar()->minimum()
                       + list->spacing();
        bool needBottom = list->verticalScrollBar()->value() < (list->verticalScrollBar()->maximum()
                                                                - list->spacing()*2);
        needTop ? top->show() : top->hide();
        needBottom ? bottom->show() : bottom->hide();
    } else {
            top->hide();
            bottom->hide();
    }
}

/*
  \internal

  Returns the listview used for the combobox popup.
*/
QGenericListView *ListViewContainer::listView() const
{
    return list;
}

bool ListViewContainer::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonRelease: {
        QMouseEvent *m = static_cast<QMouseEvent *>(e);
        if (list->rect().contains(m->pos())) {
            hide();
            emit itemSelected(list->currentItem());
        }
        break;
    }
    default:
        break;
    }
    return QFrame::eventFilter(o, e);
}

void ListViewContainer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        hide();
        emit itemSelected(list->currentItem());
        break;
    case Qt::Key_F4:
    case Qt::Key_Escape:
        hide();
        break;
    default:
        break;
    }
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
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    q->setCurrentItem(model->index(0, 0, root));
    if (q->style().styleHint(QStyle::SH_ComboBox_Popup))
        delegate = new MenuDelegate(model, q);
    else
        delegate = new QItemDelegate(model, q);
    QGenericListView *l = new QGenericListView(model, 0);
    l->setItemDelegate(delegate);
    container = new ListViewContainer(l, q);
    container->setParent(q, Qt::WType_Popup);
    QApplication::connect(container, SIGNAL(itemSelected(const QModelIndex &)),
                          q, SLOT(itemSelected(const QModelIndex &)));
}

Q4StyleOptionComboBox QGenericComboBoxPrivate::getStyleOption() const
{
    Q4StyleOptionComboBox opt(0);
    opt.init(q);
    opt.parts = QStyle::SC_All;
//     if (arrowDown)
    opt.activeParts = QStyle::SC_ComboBoxArrow;
//     else
//         opt.activeParts = QStyle::SC_None;
    opt.editable = q->isEditable();
    return opt;
}

void QGenericComboBoxPrivate::updateLineEditGeometry()
{
    if (!lineEdit || !q->isVisible())
        return;

    Q4StyleOptionComboBox opt = d->getStyleOption();
    QRect editorRect = q->style().querySubControlMetrics(QStyle::CC_ComboBox, &opt,
                                                         QStyle::SC_ComboBoxEditField, q);
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
    // ### check delegateparent and delete delegate if us?
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

/*!
    Sets the line edit to use \a edit instead of the current line edit.
*/
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

/*!
    Returns the line edit, or 0 if there is no line edit.

    Only editable listboxes have a line editor.
*/
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
    listView()->setItemDelegate(d->delegate);
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
    listView()->setRoot(index);
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
    return d->container->listView();
}

QSize QGenericComboBox::sizeHint() const
{
    if (isVisible() && d->sizeHint.isValid())
	return d->sizeHint;

    const QFontMetrics &fm = fontMetrics();
    d->sizeHint.setWidth(fm.width("XXX"));
    d->sizeHint.setHeight(fontMetrics().lineSpacing());
    if (d->lineEdit)
        d->sizeHint.setHeight(d->lineEdit->sizeHint().height());

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
    Q4StyleOptionComboBox opt = d->getStyleOption();
    d->sizeHint = (style().sizeFromContents(QStyle::CT_ComboBox, &opt,
                                            d->sizeHint, fm, this)
                   .expandedTo(QApplication::globalStrut()));
    return d->sizeHint;
}

void QGenericComboBox::popup()
{
    if (model()->rowCount(root()) <= 0)
        return;

    // set current item
    listView()->setCurrentItem(currentItem());

    // use top item as height for complete listView
    int itemHeight = listView()->itemSizeHint(model()->index(0, 0, root())).height()
                     + listView()->spacing();
    QRect listRect(rect());
    listRect.setHeight(itemHeight * qMin(d->sizeLimit, model()->rowCount(root()))
                       + listView()->spacing()*2);

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

    d->container->setGeometry(listRect);
    listView()->ensureItemVisible(listView()->currentItem());
    d->container->raise();
    d->container->show();
}

void QGenericComboBox::currentChanged(const QModelIndex &, const QModelIndex &)
{
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
    Q4StyleOptionComboBox opt = d->getStyleOption();
    style().drawComplexControl(QStyle::CC_ComboBox, &opt, &painter, this);
    QRect delegateRect = style().querySubControlMetrics(QStyle::CC_ComboBox, &opt,
                                                        QStyle::SC_ComboBoxEditField, this);
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
    Q4StyleOptionComboBox opt = d->getStyleOption();
    QRect arrowRect = style().querySubControlMetrics(QStyle::CC_ComboBox, &opt,
                                                     QStyle::SC_ComboBoxArrow, this);

    if (arrowRect.contains(e->pos()) && (!listView() || !listView()->isVisible()))
        popup();
    else
        e->ignore();
}

void QGenericComboBox::keyPressEvent(QKeyEvent *e)
{
    QModelIndex newIndex;
    switch (e->key()) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        // skip autoCompletion if Delete or Backspace has been pressed
        d->skipCompletion = true;
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
        if (!isEditable())
            newIndex = model()->index(0, 0, root());
        break;
    case Qt::Key_End:
        if (!isEditable())
            newIndex = model()->index(model()->rowCount(root()) - 1, 0, root());
        break;
    default:
        if (!e->text().isEmpty() && !isEditable()) {
            // use keyboardSearch from the listView so we do not duplicate code
            listView()->setCurrentItem(currentItem());
            listView()->keyboardSearch(e->text());
            if (listView()->currentItem().isValid()
                && listView()->currentItem() != currentItem())
                setCurrentItem(listView()->currentItem());
        }
        break;
    }

    if (newIndex.isValid())
        setCurrentItem(newIndex);
}

#include "moc_qgenericcombobox.cpp"
