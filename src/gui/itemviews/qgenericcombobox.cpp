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
    : QFrame(parent), ignoreMousePress(false), list(listView), top(0), bottom(0)
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
    list->setBeginEditActions(QAbstractItemDelegate::NeverEdit);
    list->setFocusPolicy(Qt::StrongFocus);
    connect(list->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(updateScrollers()));
    connect(list->verticalScrollBar(), SIGNAL(rangeChanged(int, int)),
            this, SLOT(updateScrollers()));
    connect(list, SIGNAL(onItem(const QModelIndex &, int)),
            this, SLOT(setCurrentItem(const QModelIndex &, int)));

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

  Sets currentItem on onItem if the LeftButton is not pressed. This
  means that if mouseTracking(...) is on, we setCurrentItem and select
  even when LeftButton is not pressed.
*/
void ListViewContainer::setCurrentItem(const QModelIndex &index, int bstate)
{
    if (bstate & Qt::LeftButton)
        return;

    list->setCurrentItem(index);
}

/*
  \internal

  Returns the listview used for the combobox popup.
*/
QGenericListView *ListViewContainer::listView() const
{
    return list;
}

/*
  \internal

  returns if the next mouspress should be ignore by the combobox
  (replay of mousepress on arrowrect when listview is visible) and
  resets ignoreMousePress to false
*/
bool ListViewContainer::ignoreNextMousePress()
{
    if (ignoreMousePress) {
        ignoreMousePress = false;
        return true;
    }
    return false;
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

void ListViewContainer::mousePressEvent(QMouseEvent *e)
{
    QGenericComboBox *comboBox = qt_cast<QGenericComboBox *>(parentWidget());
    if (comboBox) {
        QStyleOptionComboBox opt(0);
        opt.init(comboBox);
        opt.parts = QStyle::SC_All;
        opt.activeParts = QStyle::SC_ComboBoxArrow;
        QRect arrowRect = style().querySubControlMetrics(QStyle::CC_ComboBox, &opt,
                                                         QStyle::SC_ComboBoxArrow, comboBox);
        QRect globalArrowRect(comboBox->mapToGlobal(arrowRect.topLeft()),
                              comboBox->mapToGlobal(arrowRect.bottomRight()));
        if (globalArrowRect.contains(e->globalPos())) {
            ignoreMousePress = true; // ignore next mousepress (replayed) if click on arrowrect
        }
    }
    QFrame::mousePressEvent(e);
}

/*!
    Constructs a combobox widget with parent \a parent using a default model.
*/
QGenericComboBox::QGenericComboBox(QWidget *parent) :
    QWidget(*new QGenericComboBoxPrivate(), parent, 0)
{
    d->model = new ComboModel();
    d->init();
}

/*!
    Constructs a combobox widget with parent \a parent using the item model \a model
*/
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
    QApplication::connect(q->listView()->selectionModel(),
                          SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
                          q, SLOT(q->emitHighlighted(const QModelIndex &)));
}

QStyleOptionComboBox QGenericComboBoxPrivate::getStyleOption() const
{
    QStyleOptionComboBox opt(0);
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

    QStyleOptionComboBox opt = d->getStyleOption();
    QRect editorRect = q->style().querySubControlMetrics(QStyle::CC_ComboBox, &opt,
                                                         QStyle::SC_ComboBoxEditField, q);
    lineEdit->setGeometry(editorRect);
}

void QGenericComboBoxPrivate::returnPressed()
{
    if (lineEdit && !lineEdit->text().isEmpty()) {
        QString text = lineEdit->text();
        // check for duplicates (if not enabled) and quit
        if (!d->duplicatesEnabled && d->contains(text, QAbstractItemModel::Role_Edit))
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
            = d->model->match(q->currentItem(), QAbstractItemModel::Role_Edit, text);
        if (!list.count())
            return;
        QString completed = d->model->data(list.first(),
                                           QAbstractItemModel::Role_Edit).toString();
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
            lineEdit->setText(model->data(q->currentItem(), QAbstractItemModel::Role_Edit)
                              .toString());
        }
        emitActivated(q->currentItem());
    }
}

/*
  \internal
  returns true if any item under \a root containes \a text in role \a role.
*/
bool QGenericComboBoxPrivate::contains(const QString &text, int role)
{
    return model->match(model->index(0, 0, q->root()),
                             role, text, 1, QAbstractItemModel::Match_Exactly
                             |QAbstractItemModel::Match_Case).count() > 0;
}

void QGenericComboBoxPrivate::emitActivated(const QModelIndex &index)
{
    QString text(q->model()->data(index, QAbstractItemModel::Role_Edit).toString());
    emit q->activated(index.row());
    emit q->activated(text);
    emit q->activated(index);
}

void QGenericComboBoxPrivate::emitHighlighted(const QModelIndex &index)
{
    QString text(q->model()->data(index, QAbstractItemModel::Role_Edit).toString());
    emit q->highlighted(index.row());
    emit q->highlighted(text);
    emit q->highlighted(index);
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

void QGenericComboBox::setMaxCount(int max)
{
    if (max < count())
        model()->removeRows(max, root(), count() - max);

    d->maxCount = max;
}

int QGenericComboBox::maxCount() const
{
    return d->maxCount;
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
    connect(d->lineEdit, SIGNAL(textChanged(const QString&)),
            this, SIGNAL(textChanged(const QString &)));
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

void QGenericComboBox::setValidator(const QValidator *v)
{
    if (d->lineEdit)
        d->lineEdit->setValidator(v);
}

const QValidator *QGenericComboBox::validator() const
{
    return d->lineEdit ? d->lineEdit->validator() : 0;
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
	return model()->data(currentItem(), QAbstractItemModel::Role_Edit).toString();
    else
	return QString::null;
}

void QGenericComboBox::setCurrentText(const QString& text)
{
    if (currentItem().isValid()) {
        model()->setData(currentItem(), QAbstractItemModel::Role_Edit, text);
        if (d->lineEdit)
            d->lineEdit->setText(text);
    }
}

QString QGenericComboBox::text(int row) const
{
    QModelIndex index = model()->index(row, 0, root());
    return model()->data(index, QAbstractItemModel::Role_Edit).toString();
}

QPixmap QGenericComboBox::pixmap(int row) const
{
    QModelIndex index = model()->index(row, 0, root());
    return model()->data(index, QAbstractItemModel::Role_Edit).toIconSet().pixmap();
}

void QGenericComboBox::insertStringList(const QStringList &list, int row)
{
    if (list.isEmpty())
        return;

    if (row < 0)
        row = model()->rowCount(root());

    if (model()->insertRows(row, root(), list.count())) {
        QModelIndex item;
        for (int i = 0; i < list.count(); ++i) {
            item = model()->index(i+row, 0, root());
            model()->setData(item, QAbstractItemModel::Role_Edit, list.at(i));
        }
    }
}

QModelIndex QGenericComboBox::insertItem(const QString &text, int row)
{
    if (row < 0)
        row = model()->rowCount(root());
    QModelIndex item;
    if (model()->insertRows(row, root())) {
        item = model()->index(row, 0, root());
        model()->setData(item, QAbstractItemModel::Role_Edit, text);
    }
    return item;
}

QModelIndex QGenericComboBox::insertItem(const QIconSet &icon, int row)
{
    if (row < 0)
        row = model()->rowCount(root());
    QModelIndex item;
    if (model()->insertRows(row, root())) {
        item = model()->index(row, 0, root());
        model()->setData(item, QAbstractItemModel::Role_Decoration, icon);
    }
    return item;
}

QModelIndex QGenericComboBox::insertItem(const QString &text, const QIconSet &icon, int row)
{
    if (row < 0)
        row = model()->rowCount(root());
    QModelIndex item;
    if (model()->insertRows(row, root())) {
        item = model()->index(row, 0, root());
        QMap<int, QVariant> values;
        values.insert(QAbstractItemModel::Role_Edit, text);
        values.insert(QAbstractItemModel::Role_Decoration, icon);
        model()->setItemData(item, values);
    }
    return item;
}

void QGenericComboBox::removeItem(int row)
{
    model()->removeRows(row, root(), 1);
}

QModelIndex QGenericComboBox::changeItem(const QString &text, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        model()->setData(item, QAbstractItemModel::Role_Edit, text);
    }
    return item;
}

QModelIndex QGenericComboBox::changeItem(const QIconSet &icon, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        model()->setData(item, QAbstractItemModel::Role_Decoration, icon);
    }
    return item;
}

QModelIndex QGenericComboBox::changeItem(const QString &text, const QIconSet &icon, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        QMap<int, QVariant> map;
        map.insert(QAbstractItemModel::Role_Edit, text);
        map.insert(QAbstractItemModel::Role_Decoration, icon);
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

    QStyleOptionViewItem option(0);
    option.state |= (isEditable() ? QStyle::Style_Editing : QStyle::Style_Default);
    option.state |= QStyle::Style_Selected;
    option.state |= (q->hasFocus() ? QStyle::Style_HasFocus : QStyle::Style_Default);

    QSize itemSize;
    for (int i = 0; i < model()->rowCount(root()); i++) {
        itemSize = d->delegate->sizeHint(fontMetrics(), option, model()->index(i, 0, root()));
        if (itemSize.width() > d->sizeHint.width())
            d->sizeHint.setWidth(itemSize.width());
        if (itemSize.height() > d->sizeHint.height())
            d->sizeHint.setHeight(itemSize.height());
    }
    QStyleOptionComboBox opt = d->getStyleOption();
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
        d->lineEdit->setText(model()->data(q->currentItem(), QAbstractItemModel::Role_Edit)
                             .toString());
    d->emitActivated(currentItem());
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
    QStyleOptionComboBox opt = d->getStyleOption();
    style().drawComplexControl(QStyle::CC_ComboBox, &opt, &painter, this);
    QRect delegateRect = style().querySubControlMetrics(QStyle::CC_ComboBox, &opt,
                                                        QStyle::SC_ComboBoxEditField, this);
    // delegate paints content
    QStyleOptionViewItem itemOpt(0);

    QModelIndex current = currentItem();
    if (current.isValid()) {
        itemOpt.init(this);
        itemOpt.decorationPosition = QStyleOptionViewItem::Left;
        itemOpt.state |= (isEditable() ? QStyle::Style_Editing : QStyle::Style_Default);
        itemOpt.state |= (q->hasFocus()
                        ? QStyle::Style_HasFocus|QStyle::Style_Selected : QStyle::Style_Default);
        itemOpt.rect = delegateRect;
        d->delegate->paint(&painter, itemOpt, current);
    }
}

void QGenericComboBox::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionComboBox opt = d->getStyleOption();
    QRect arrowRect = style().querySubControlMetrics(QStyle::CC_ComboBox, &opt,
                                                     QStyle::SC_ComboBoxArrow, this);

    if (arrowRect.contains(e->pos()) && !d->container->isVisible()
        && !d->container->ignoreNextMousePress())
        popup();
    QWidget::mousePressEvent(e);
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
