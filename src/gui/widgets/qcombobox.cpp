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

#include "qcombobox.h"
#include <qpainter.h>
#include <qlineedit.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qlistview.h>
#include <qitemdelegate.h>
#include <qmap.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <private/qcombobox_p.h>

#define d d_func()
#define q q_func()

/*!
    \internal
*/

ListViewContainer::ListViewContainer(QListView *listView, QWidget *parent)
    : QFrame(parent), list(listView), top(0), bottom(0)
{
    // setup container
    setFrameStyle(QFrame::Box|QFrame::Plain);
    setLineWidth(1);

    // setup the listview
    Q_ASSERT(list);
    list->setParent(this);
    list->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    list->viewport()->installEventFilter(this);
    setFocusProxy(list);
    list->setAttribute(Qt::WA_CompositeChild);
    setAttribute(Qt::WA_CompositeParent);
    QStyleOptionComboBox opt;
    opt.init(parent);
    if (QComboBox *cmb = qt_cast<QComboBox *>(parent))
        opt.editable = cmb->isEditable();
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (style().styleHint(QStyle::SH_ComboBox_Popup, &opt, parent))
        list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (style().styleHint(QStyle::SH_ComboBox_ListMouseTracking, &opt, parent) ||
        style().styleHint(QStyle::SH_ComboBox_Popup, &opt, parent)) {
        list->setMouseTracking(true);
    }
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setFrameStyle(QFrame::NoFrame);
    list->setLineWidth(0);
    list->setSpacing(0);
    list->setBeginEditActions(QAbstractItemView::NeverEdit);
    list->setFocusPolicy(Qt::StrongFocus);
    connect(list->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(updateScrollers()));
    connect(list->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
            this, SLOT(updateScrollers()));
    connect(list, SIGNAL(itemEntered(QModelIndex,const QMouseEvent*)),
            this, SLOT(setCurrentIndex(QModelIndex,const QMouseEvent*)));

    // add widgets to layout and create scrollers if needed
    QBoxLayout *layout =  new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setSpacing(0);
    layout->setMargin(0);
    if (style().styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
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

/*!
    \internal
*/

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
        if(needTop)
            top->show();
        else
            top->hide();
        if(needBottom)
            bottom->show();
        else
            bottom->hide();
    } else {
        top->hide();
        bottom->hide();
    }
}

/*
  \internal

  Sets currentIndex on itemEntered if the LeftButton is not pressed. This
  means that if mouseTracking(...) is on, we setCurrentIndex and select
  even when LeftButton is not pressed.
*/
void ListViewContainer::setCurrentIndex(const QModelIndex &index, const QMouseEvent *event)
{
    if (event->modifiers() & Qt::LeftButton)
        return;

    list->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

/*
  \internal

  Returns the listview used for the combobox popup.
*/
QListView *ListViewContainer::listView() const
{
    return list;
}


/*!
    \internal
*/

bool ListViewContainer::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonRelease: {
        QMouseEvent *m = static_cast<QMouseEvent *>(e);
        if (list->rect().contains(m->pos())) {
            if (qt_cast<QComboBox*>(parentWidget())->autoHide())
                hide();
            emit itemSelected(list->currentIndex());
        }
        break;
    }
    default:
        break;
    }
    return QFrame::eventFilter(o, e);
}

/*!
    \internal
*/

void ListViewContainer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (qt_cast<QComboBox*>(parentWidget())->autoHide())
            hide();
        emit itemSelected(list->currentIndex());
        return;
    case Qt::Key_Down:
        if (!(e->modifiers() & Qt::AltModifier))
            break;
    case Qt::Key_F4:
    case Qt::Key_Escape:
        hide();
        return;
    default:
        break;
    }
    e->ignore();
}


/*!
    \internal
*/
void ListViewContainer::hideEvent(QHideEvent *)
{
    emit containerDisappearing();
}

/*!
    \internal
*/

void ListViewContainer::mousePressEvent(QMouseEvent *e)
{
    QComboBox *comboBox = qt_cast<QComboBox *>(parentWidget());
    if (comboBox) {
        QRect ignoreRect = comboBox->rect();
        if (comboBox->isEditable()) {
            QStyleOptionComboBox opt;
            opt.init(comboBox);
            opt.subControls = QStyle::SC_All;
            opt.activeSubControls = QStyle::SC_ComboBoxArrow;
            ignoreRect = QStyle::visualRect(style().querySubControlMetrics(
                                                QStyle::CC_ComboBox, &opt,
                                                QStyle::SC_ComboBoxArrow, comboBox), comboBox);
        }
        ignoreRect = QRect(comboBox->mapToGlobal(ignoreRect.topLeft()),
                           comboBox->mapToGlobal(ignoreRect.bottomRight()));
        if (ignoreRect.contains(e->globalPos()))
            setAttribute(Qt::WA_NoMouseReplay);
    }
    QFrame::mousePressEvent(e);
}

/*!
    \enum QComboBox::InsertionPolicy

    This enum specifies what the QComboBox should do when a new string is
    entered by the user.

    \value NoInsertion    The string will not be inserted into the combobox.
    \value AtTop          The string will be inserted as the first item in the
                          combobox.
    \value AtCurrent      The current item will be \e replaced by the string.
    \value AtBottom       The string will be inserted after the last item
                          in the combobox.
    \value AfterCurrent   The string is inserted after the current item in the
                          combobox.
    \value BeforeCurrent  The string is inserted before the current item in
                          the combobox.
*/

/*!
    \fn void QComboBox::textChanged(const QString &text)

    This signal is sent when the text changes in the current item. The
    item's new \a text is given.
*/

/*!
    \fn void QComboBox::activated(int row)

    This signal is sent when an item in the combobox is activated. The
    item's \a row is given.

*/

/*!
    \fn void QComboBox::activated(const QString &text)

    This signal is sent when an item in the combobox is activated. The
    item's \a text is given.

*/

/*!
    \fn void QComboBox::activated(const QModelIndex &)

    \internal

*/

/*!
    \fn void QComboBox::highlighted(int row)

    This signal is sent when an item in the combobox is highlighted. The
    item's \a row is given.
*/

/*!
    \fn void QComboBox::highlighted(const QString &text)

    This signal is sent when an item in the combobox is highlighted. The
    item's \a text is given.
*/

/*!
    \fn void QComboBox::highlighted(const QModelIndex &index)

    \internal

    This signal is sent when an item in the combobox is highlighted. The
    item's model item \a index is given.
*/

/*!
    \fn void QComboBox::rootChanged(const QModelIndex &old, const QModelIndex &root)

    \internal

    This signal is sent when the root model item for the combobox is changed.
    Both the \a old root index and the new \a root index are given.
*/

/*!
    Constructs a combobox with the given \a parent, using the default model.
*/
QComboBox::QComboBox(QWidget *parent) :
    QWidget(*new QComboBoxPrivate(), parent, 0)
{
    d->init();
    setModel(new ComboModel());
}

#ifdef QT_COMPAT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QComboBox::QComboBox(QWidget *parent, const char *name) :
    QWidget(*new QComboBoxPrivate(), parent, 0)
{
    d->init();
    setModel(new ComboModel());
    setObjectName(name);
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QComboBox::QComboBox(bool rw, QWidget *parent, const char *name) :
    QWidget(*new QComboBoxPrivate(), parent, 0)
{
    d->init();
    setModel(new ComboModel());
    setEditable(rw);
    setObjectName(name);
}

#endif //QT_COMPAT

/*!
    \class QComboBox qcombobox.h
    \brief The QComboBox widget is a combined button and popup list.

    \ingroup basic
    \mainclass

    A QComboBox provides a means of presenting a list of options to the user
    in a way that takes up the minimum amount of screen space.

    A combobox is a selection widget that displays the current item,
    and can pop up a list of selectable items. A combobox may be editable,
    allowing the user to modify each item in the list.

    QComboBox supports three different display styles: Aqua/Motif 1.x,
    Motif 2.0 and Windows. In Motif 1.x, a combobox was called
    XmOptionMenu. In Motif 2.0, OSF introduced an improved combobox
    and named that XmComboBox. QComboBox provides both.

    QComboBox provides two different constructors. The simplest
    constructor creates an "old-style" combobox in Motif (or Aqua)
    style:
    \code
        QComboBox *c = new QComboBox(this, "read-only combobox");
    \endcode

    The other constructor creates a new-style combobox in Motif style,
    and can create both read-only and editable comboboxes:
    \code
        QComboBox *c1 = new QComboBox(false, this, "read-only combobox" );
        QComboBox *c2 = new QComboBox(true, this, "editable combobox" );
    \endcode

    New-style comboboxes use a list box in both Motif and Windows
    styles, and both the content size and the on-screen size of the
    list box can be limited with sizeLimit() and setMaxCount()
    respectively. Old-style comboboxes use a popup in Aqua and Motif
    style, and that popup will happily grow larger than the desktop if
    you put enough data into it.

    The two constructors create identical-looking comboboxes in
    Windows style.

    Comboboxes can contain pixmaps as well as strings; the
    insertItem() and changeItem() functions are suitably overloaded.
    For editable comboboxes, the function clearEdit() is provided,
    to clear the displayed string without changing the combobox's
    contents.

    A combobox emits two signals, activated() and highlighted(), when
    a new item has been activated (selected) or highlighted (made
    current). Both signals exist in two versions, one with a \c
    QString argument and one with an \c int argument. If the user
    highlights or activates a pixmap, only the \c int signals are
    emitted. Whenever the text of an editable combobox is changed the
    textChanged() signal is emitted.

    When the user enters a new string in an editable combobox, the
    widget may or may not insert it, and it can insert it in several
    locations. The default policy is is \c AtBottom but you can change
    this using setInsertionPolicy().

    It is possible to constrain the input to an editable combobox
    using QValidator; see setValidator(). By default, any input is
    accepted.

    If the combobox is not editable then it has a default
    focusPolicy() of \c TabFocus, i.e. it will not grab focus if
    clicked. This differs from both Windows and Motif. If the combobox
    is editable then it has a default focusPolicy() of \c StrongFocus,
    i.e. it will grab focus if clicked.

    A combobox can be populated using the insert functions,
    insertStringList() and insertItem() for example. Items can be
    changed with changeItem(). An item can be removed with
    removeItem() and all items can be removed with clear(). The text
    of the current item is returned by currentText(), and the text of
    a numbered item is returned with text(). The current item can be
    set with setCurrentItem() or setCurrentText(). The number of items
    in the combobox is returned by count(); the maximum number of
    items can be set with setMaxCount(). You can allow editing using
    setEditable(). For editable comboboxes you can set auto-completion
    using setAutoCompletion() and whether or not the user can add
    duplicates is set with setDuplicatesEnabled().

    <img src="qcombo1-m.png">(Motif 1, read-only)<br clear=all>
    <img src="qcombo2-m.png">(Motif 2, editable)<br clear=all>
    <img src="qcombo3-m.png">(Motif 2, read-only)<br clear=all>
    <img src="qcombo1-w.png">(Windows style)

    \sa QLineEdit QListBox QSpinBox QRadioButton QButtonGroup
    \link guibooks.html#fowler GUI Design Handbook: Combo Box,\endlink
    \link guibooks.html#fowler GUI Design Handbook: Drop-Down List Box.\endlink
*/

/*!
    \internal
*/

QComboBox::QComboBox(QComboBoxPrivate &dd, QWidget *parent) :
    QWidget(dd, parent, 0)
{
    d->init();
}

void QComboBoxPrivate::init()
{
    QListView *l = new QListView(0);
    d->model = l->model();
    container = new ListViewContainer(l, q);
    container->setParent(q, Qt::WType_Popup);
    q->setFocusPolicy(Qt::TabFocus);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    q->setCurrentItem(0);
    QStyleOptionComboBox opt = getStyleOption();
    if (q->style().styleHint(QStyle::SH_ComboBox_Popup, &opt, q))
        l->setItemDelegate(new MenuDelegate(l, q));
    QObject::connect(container, SIGNAL(itemSelected(QModelIndex)),
                     q, SLOT(itemSelected(QModelIndex)));
    QObject::connect(q->listView()->selectionModel(),
                     SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                     q, SLOT(emitHighlighted(QModelIndex)));
    QObject::connect(container, SIGNAL(containerDisappearing()), q, SLOT(resetButton()));
}

void QComboBoxPrivate::resetButton()
{
    arrowDown = false;
}

QStyleOptionComboBox QComboBoxPrivate::getStyleOption() const
{
    QStyleOptionComboBox opt;
    opt.state = QStyle::Style_None;
    opt.rect = q->rect();
    opt.palette = q->palette();
    opt.init(q);
    if (q->isEditable() && q->lineEdit()->hasFocus())
        opt.state |= QStyle::Style_HasFocus;
    opt.subControls = QStyle::SC_All;
    if (arrowDown)
        opt.activeSubControls = QStyle::SC_ComboBoxArrow;
    else
        opt.activeSubControls = QStyle::SC_None;
    opt.editable = q->isEditable();
    return opt;
}

void QComboBoxPrivate::updateLineEditGeometry()
{
    if (!lineEdit)
        return;

    QStyleOptionComboBox opt = d->getStyleOption();
    QRect editorRect = QStyle::visualRect(q->style().querySubControlMetrics(
                                              QStyle::CC_ComboBox, &opt,
                                              QStyle::SC_ComboBoxEditField, q), q);
    const QPixmap &pix = q->pixmap(q->currentItem());
    if (!pix.isNull())
        editorRect.setLeft(editorRect.left() + pix.width() + 4);
    lineEdit->setGeometry(editorRect);
}

void QComboBoxPrivate::returnPressed()
{
    if (lineEdit && !lineEdit->text().isEmpty()) {
        QString text = lineEdit->text();
        // check for duplicates (if not enabled) and quit
        if (!d->duplicatesEnabled && q->contains(text))
            return;
        int row = -1;
        switch (insertionPolicy) {
        case QComboBox::AtTop:
            row = 0;
            break;
        case QComboBox::AtBottom:
            row = d->model->rowCount(q->root());
            break;
        case QComboBox::AtCurrent:
        case QComboBox::AfterCurrent:
        case QComboBox::BeforeCurrent:
            if (!d->model->rowCount(q->root()) || !currentIndex.isValid())
                row = 0;
            else if (insertionPolicy == QComboBox::AtCurrent)
                q->setItemText(text, q->currentItem());
            else if (insertionPolicy == QComboBox::AfterCurrent)
                row = q->currentItem() + 1;
            else if (insertionPolicy == QComboBox::BeforeCurrent)
                row = q->currentItem();
            break;
        case QComboBox::NoInsertion:
        default:
            break;
        }
        if (row >= 0) {
            q->insertItem(text, row);
            q->setCurrentItem(row);
        }
    }
}

/*
  \internal

  handles auto completion
*/
void QComboBoxPrivate::complete()
{
    if (d->skipCompletion || !lineEdit || !autoCompletion) {
        skipCompletion = false;
        return;
    }
    QString text = lineEdit->text();
    if (!text.isEmpty()) {
        QModelIndexList list
            = d->model->match(currentIndex, QAbstractItemModel::EditRole, text);
        if (!list.count())
            return;
        QString completed = d->model->data(list.first(),
                                           QAbstractItemModel::EditRole).toString();
        int start = completed.length();
        int length = text.length() - start; // negative length
        lineEdit->setText(completed);
        lineEdit->setSelection(start, length);
    }
}

void QComboBoxPrivate::itemSelected(const QModelIndex &item)
{
    if (item != currentIndex) {
        q->setCurrentItem(item.row());
    } else if (q->isEditable()) {
        if (lineEdit) {
            lineEdit->selectAll();
            lineEdit->setText(model->data(currentIndex, QAbstractItemModel::EditRole).toString());
        }
        emitActivated(currentIndex);
    }
}

void QComboBoxPrivate::emitActivated(const QModelIndex &index)
{
    QString text(q->model()->data(index, QAbstractItemModel::EditRole).toString());
    emit q->activated(index.row());
    emit q->activated(text);
    emit q->activated(index);
}

void QComboBoxPrivate::emitHighlighted(const QModelIndex &index)
{
    QString text(q->model()->data(index, QAbstractItemModel::EditRole).toString());
    emit q->highlighted(index.row());
    emit q->highlighted(text);
    emit q->highlighted(index);
}

/*!
    Destroys the combobox.
*/

QComboBox::~QComboBox()
{
    // ### check delegateparent and delete delegate if us?
}

/*!
    \property QComboBox::sizeLimit
    \brief the maximum allowed size on screen of the combobox
*/

int QComboBox::sizeLimit() const
{
    return d->sizeLimit;
}

void QComboBox::setSizeLimit(int limit)
{
    if (limit > 0)
        d->sizeLimit = limit;
}

/*!
    \property QComboBox::count
    \brief the number of items in the combobox
*/

int QComboBox::count() const
{
    return d->model->rowCount(root());
}

/*!
    \property QComboBox::maxCount
    \brief the maximum number of items allowed in the combobox
*/

void QComboBox::setMaxCount(int max)
{
    if (max < count())
        model()->removeRows(max, root(), count() - max);

    d->maxCount = max;
}

int QComboBox::maxCount() const
{
    return d->maxCount;
}

/*!
    \property QComboBox::autoCompletion
    \brief whether the combobox provides auto-completion for editable items

    \sa editable
*/

bool QComboBox::autoCompletion() const
{
    return d->autoCompletion;
}

void QComboBox::setAutoCompletion(bool enable)
{
    d->autoCompletion = enable;
}

/*!
    \property QComboBox::duplicatesEnabled
    \brief whether the combobox can contain duplicate items
*/

bool QComboBox::duplicatesEnabled() const
{
    return d->duplicatesEnabled;
}

void QComboBox::setDuplicatesEnabled(bool enable)
{
    d->duplicatesEnabled = enable;
}

/*!
    \property QComboBox::autoHide
    \brief whether the combobox hides the popup listbox when an item is activated
*/

bool QComboBox::autoHide() const
{
    return d->autoHide;
}

void QComboBox::setAutoHide(bool enable)
{
    d->autoHide = enable;
}

/*!
  Returns true if any item in the combobox matches the given \a text.
*/
bool QComboBox::contains(const QString &text) const
{
    return model()->match(model()->index(0, 0, root()),
                          QAbstractItemModel::EditRole, text, 1,
                          QAbstractItemModel::MatchExactly
                          |QAbstractItemModel::MatchCase).count() > 0;
}

/*!
  Returns the index of the item containing the given \a text; otherwise
  returns -1.

  The \a flags specify how the items in the combobox are searched.
*/
int QComboBox::findItem(const QString &text, QAbstractItemModel::MatchFlags flags) const
{
    QModelIndexList result;
    QModelIndex start = model()->index(0, 0, root());
    result = model()->match(start, QAbstractItemModel::EditRole, text, 1, flags);
    if (result.isEmpty())
        return -1;
    return result.first().row();
}

/*!
    \property QComboBox::insertionPolicy
    \brief the policy used to determine where user-inserted items should
    appear in the combobox

    The default value is \c AtBottom, indicating that new items will appear
    at the bottom of the list of items.

    \sa InsertionPolicy
*/

QComboBox::InsertionPolicy QComboBox::insertionPolicy() const
{
    return d->insertionPolicy;
}

void QComboBox::setInsertionPolicy(InsertionPolicy policy)
{
    d->insertionPolicy = policy;
}

/*!
    \property QComboBox::editable
    \brief whether the combobox can be edited by the user
*/

bool QComboBox::isEditable() const
{
    return d->lineEdit != 0;
}

void QComboBox::setEditable(bool editable)
{
    if (isEditable() == editable)
        return;

    if (editable) {
        setFocusPolicy(Qt::StrongFocus);
        setLineEdit(new QLineEdit(this));
    } else {
        delete d->lineEdit;
        d->lineEdit = 0;
    }
}

/*!
    Sets the line \a edit to use instead of the current line edit widget.
*/
void QComboBox::setLineEdit(QLineEdit *edit)
{
    if ( !edit ) {
	Q_ASSERT(edit != 0);
	return;
    }
    if (!(model()->flags(model()->index(0, 0, root())) & QAbstractItemModel::ItemIsEditable)) {
        delete edit;
        return;
    }

    edit->setText(currentText());
    delete d->lineEdit;

    setFocusPolicy(Qt::StrongFocus);
    d->lineEdit = edit;
    if (d->lineEdit->parent() != this)
	d->lineEdit->setParent(this);
    connect(d->lineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
    connect(d->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(complete()));
    connect(d->lineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(textChanged(QString)));
    d->lineEdit->setFrame(false);
    d->lineEdit->setAttribute(Qt::WA_CompositeChild);
    setAttribute(Qt::WA_CompositeParent);
    setFocusProxy(d->lineEdit);
    setAttribute(Qt::WA_InputMethodEnabled);
    d->updateLineEditGeometry();

    if (isVisible())
	d->lineEdit->show();

    update();
}

/*!
    Returns the line edit used to edit items in the combobox, or 0 if there
    is no line edit.

    Only editable combo boxes have a line edit.
*/
QLineEdit *QComboBox::lineEdit() const
{
    return d->lineEdit;
}

/*!
    \fn void QComboBox::setValidator(const QValidator *validator)

    Sets the \a validator to use instead of the current validator.
*/

void QComboBox::setValidator(const QValidator *v)
{
    if (d->lineEdit)
        d->lineEdit->setValidator(v);
}

/*!
    Returns the validator that is used to constrain text input for the
    combobox.

    \sa editable
*/

const QValidator *QComboBox::validator() const
{
    return d->lineEdit ? d->lineEdit->validator() : 0;
}

/*!
    Returns the item delegate used by the popup list view.

    \sa setItemDelegate()
*/
QAbstractItemDelegate *QComboBox::itemDelegate() const
{
    return listView()->itemDelegate();
}

/*!
    Sets the item \a delegate for the popup list view.

    \sa itemDelegate()
*/
void QComboBox::setItemDelegate(QAbstractItemDelegate *delegate)
{
    Q_ASSERT(delegate);
    listView()->setItemDelegate(delegate);
}

/*!
    Returns the model used by the combobox.
*/

QAbstractItemModel *QComboBox::model() const
{
    return d->model;
}

/*!
  Sets the model to be \a model,
*/

void QComboBox::setModel(QAbstractItemModel *model)
{
    d->model = model;
    d->container->listView()->setModel(model);
}

/*!
    \internal

    Returns the root model item index for the items in the combobox.

*/

QModelIndex QComboBox::root() const
{
    return QModelIndex(d->root);
}

/*!
    \internal

    Sets the root model item \a index for the items in the combobox.
*/

void QComboBox::setRoot(const QModelIndex &index)
{
    QModelIndex old = d->root;
    d->root = QPersistentModelIndex(index, d->model);
    listView()->setRoot(index);
    emit rootChanged(old, index);
    update();
}

/*!
    \property QComboBox::currentItem
*/

int QComboBox::currentItem() const
{
    return d->currentIndex.row();
}

void QComboBox::setCurrentItem(int row)
{
    QModelIndex index = model()->index(row, 0, root());
    if (!index.isValid() || index == d->currentIndex)
        return;
    QModelIndex old = d->currentIndex;
    d->currentIndex = QPersistentModelIndex(index, d->model);
    emit currentChanged(old, index);
    update();
}

/*!
    \property QComboBox::currentText
*/

QString QComboBox::currentText() const
{
    if (d->lineEdit)
        return d->lineEdit->text();
    else if (d->currentIndex.isValid())
	return model()->data(d->currentIndex, QAbstractItemModel::EditRole).toString();
    else
	return QString::null;
}

void QComboBox::setCurrentText(const QString& text)
{
    if (d->currentIndex.isValid()) {
        model()->setData(d->currentIndex, QAbstractItemModel::EditRole, text);
        if (d->lineEdit)
            d->lineEdit->setText(text);
    }
}

/*!
    Returns the text for the given \a row in the combobox.
*/

QString QComboBox::text(int row) const
{
    QModelIndex index = model()->index(row, 0, root());
    return model()->data(index, QAbstractItemModel::EditRole).toString();
}

/*!
    Returns the pixmap for the given \a row in the combobox.
*/

QPixmap QComboBox::pixmap(int row) const
{
    QStyleOptionComboBox opt = d->getStyleOption();
    QModelIndex index = model()->index(row, 0, root());
    return model()->data(d->currentIndex, QAbstractItemModel::DecorationRole).toIconSet()
        .pixmap(QIcon::Automatic,
                opt.state & QStyle::Style_Enabled ? QIcon::Normal : QIcon::Disabled);
}

/*!
    Inserts the strings from the \a list into the combobox as separate items,
    starting at the \a row specified.
*/

void QComboBox::insertStringList(const QStringList &list, int row)
{
    if (list.isEmpty() || list.count() + count() > d->maxCount)
        return;

    if (row < 0)
        row = d->model->rowCount(root());

    if (model()->insertRows(row, root(), list.count())) {
        QModelIndex item;
        for (int i = 0; i < list.count(); ++i) {
            item = model()->index(i+row, 0, root());
            model()->setData(item, QAbstractItemModel::EditRole, list.at(i));
        }
    }
    if (!d->currentIndex.isValid())
        setCurrentItem(row);
}

/*!
    Inserts the \a text into the combobox at the given \a row.
*/

void QComboBox::insertItem(const QString &text, int row)
{
    if (!(count() < d->maxCount))
        return;
    if (row < 0)
        row = d->model->rowCount(root());
    QModelIndex item;
    if (model()->insertRows(row, root(), 1)) {
        item = model()->index(row, 0, root());
        model()->setData(item, QAbstractItemModel::EditRole, text);

        if (d->currentIndex.isValid() && item == d->currentIndex) {
            update();
            if (d->lineEdit)
                d->lineEdit->setText(text);
        }
    }
    if (!d->currentIndex.isValid())
        setCurrentItem(row);
}

/*!
    Inserts the \a icon into the combobox at the given \a row.
*/

void QComboBox::insertItem(const QIcon &icon, int row)
{
    if (!(count() < d->maxCount))
        return;
    if (row < 0)
        row = d->model->rowCount(root());
    QModelIndex item;
    if (model()->insertRows(row, root(), 1)) {
        item = model()->index(row, 0, root());
        model()->setData(item, QAbstractItemModel::DecorationRole, icon);
    }
    if (!d->currentIndex.isValid())
        setCurrentItem(row);
}

/*!
    Inserts the \a icon and \a text into the combobox at the given \a row.
*/

void QComboBox::insertItem(const QIcon &icon, const QString &text, int row)
{
    if (!(count() < d->maxCount))
        return;
    if (row < 0)
        row = d->model->rowCount(root());
    QModelIndex item;
    if (model()->insertRows(row, root(), 1)) {
        item = model()->index(row, 0, root());
        QMap<int, QVariant> values;
        values.insert(QAbstractItemModel::EditRole, text);
        values.insert(QAbstractItemModel::DecorationRole, icon);
        model()->setItemData(item, values);
    }
    if (!d->currentIndex.isValid())
        setCurrentItem(row);
}

/*!
    \internal

    Removes the item in the given \a row from the combobox.
*/

void QComboBox::removeItem(int row)
{
    model()->removeRows(row, root(), 1);
}

/*!
    Sets the \a text for the item on the given \a row in the combobox.
*/

void QComboBox::setItemText(const QString &text, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        model()->setData(item, QAbstractItemModel::EditRole, text);
    }
}

/*!
    Sets the \a icon for the item on the given \a row in the combobox.
*/

void QComboBox::setItemIcon(const QIcon &icon, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        model()->setData(item, QAbstractItemModel::DecorationRole, icon);
    }
}

/*!
    Sets the \a icon and \a text for the given \a row in the combobox.
*/

void QComboBox::setItem(const QIcon &icon, const QString &text, int row)
{
    QModelIndex item = model()->index(row, 0, root());
    if (item.isValid()) {
        QMap<int, QVariant> map;
        map.insert(QAbstractItemModel::EditRole, text);
        map.insert(QAbstractItemModel::DecorationRole, icon);
        model()->setItemData(item, map);
    }
}

/*!
    \internal

    Returns the list view used to display the combobox.

*/

QListView *QComboBox::listView() const
{
    return d->container->listView();
}

/*!
    \reimp

    This implementation caches the size hint to avoid resizing when
    the contents change dynamically. To invalidate the cached value
    call setFont().
*/

QSize QComboBox::sizeHint() const
{
    if (isVisible() && d->sizeHint.isValid())
	return d->sizeHint;

    // default width and height
    const QFontMetrics &fm = fontMetrics();
    d->sizeHint.setWidth(fm.width("XX") + 2);
    d->sizeHint.setHeight(qMax(fm.lineSpacing(), 14) + 2);

    // get maximum needed width based on model data
    int maxWidth = 0;
    QString txt;
    QModelIndex index;
    for (int i = 0; i < model()->rowCount(root()); ++i) {
        index = model()->index(i, 0, root());
        txt = text(index.row());
        const QPixmap &pix = pixmap(index.row());
        // check listview item width
        maxWidth = listView()->itemSizeHint(index).width();
        // check combo text+pixmap width
        maxWidth = qMax(maxWidth, fm.width(txt) + 2 + (pix.isNull() ? 0 : pix.width() + 4));
        if (maxWidth > d->sizeHint.width())
            d->sizeHint.setWidth(maxWidth);
    }

    // add style and strut values
    QStyleOptionComboBox opt = d->getStyleOption();
    d->sizeHint = (style().sizeFromContents(QStyle::CT_ComboBox, &opt, d->sizeHint, fm, this)
                   .expandedTo(QApplication::globalStrut()));
    return d->sizeHint;
}

/*!
    Displays the list of items in the combobox. If the list is empty then
    no items will be shown.
*/

void QComboBox::popup()
{
    if (d->model->rowCount(root()) <= 0)
        return;

    // set current item and select it
    listView()->selectionModel()->setCurrentIndex(d->currentIndex,
                                                  QItemSelectionModel::ClearAndSelect);

    // use top item as height for complete listView
    int itemHeight = listView()->itemSizeHint(model()->index(0, 0, root())).height()
                     + listView()->spacing();
    QRect listRect(rect());
    listRect.setHeight(itemHeight * qMin(d->sizeLimit, d->model->rowCount(root()))
                       + 2*listView()->spacing() + 2);

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
    listView()->ensureItemVisible(listView()->currentIndex());
    d->container->raise();
    d->container->show();
}

/*!
    \reimp
*/

void QComboBox::hide()
{
    QWidget::hide();

    if (d->container->isVisible())
        d->container->hide();
}


/*!
    \internal

    Clears the combobox, removing all items.
*/

void QComboBox::clear()
{
    model()->removeRows(0, root(), model()->rowCount(root()));
    // ### shouldn't be necessary, model should update persistentindexes
    QModelIndex old = d->currentIndex;
    d->currentIndex = QPersistentModelIndex();
    emit currentChanged(old, d->currentIndex);
    if (d->lineEdit)
        d->lineEdit->setText(QString::null);
}

/*!
    Clears (removes) the validator used to check user input for the combobox.
*/

void QComboBox::clearValidator()
{
    if (d->lineEdit)
        d->lineEdit->setValidator(0);
}

/*!
    Clears the contents of the line edit used for editing in the combobox.
*/

void QComboBox::clearEdit()
{
    if (d->lineEdit)
        d->lineEdit->clear();
}

/*!
    Sets the \a text in the combobox's text edit.
*/

void QComboBox::setEditText(const QString &text)
{
    if (d->lineEdit)
        d->lineEdit->setText(text);
}

/*!
    \fn void QComboBox::currentChanged(const QModelIndex &old, const QModelIndex &current)

    \internal

    Changes the current model item from the \a old index to the newly
    specified \a current index.
*/

void QComboBox::currentChanged(const QModelIndex &, const QModelIndex &)
{
    if (d->lineEdit) {
        d->updateLineEditGeometry();
        d->lineEdit->setText(model()->data(d->currentIndex, QAbstractItemModel::EditRole)
                             .toString());
    }
    d->emitActivated(d->currentIndex);
}

/*!
    \reimp
*/

void QComboBox::focusInEvent(QFocusEvent *)
{
    update();
}

/*!
    \reimp
*/

void QComboBox::focusOutEvent(QFocusEvent *)
{
    update();
}

/*! \reimp */
void QComboBox::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::StyleChange:
        d->sizeHint = QSize(); // invalidate size hint
        if (d->lineEdit)
            d->updateLineEditGeometry();
        //### need to update scrollers etc. as well here
        break;
    case QEvent::EnabledChange:
        if (!isEnabled() && d->container)
            d->container->hide();
        break;
    case QEvent::ApplicationPaletteChange:
    case QEvent::PaletteChange:
        if (d->container)
            d->container->setPalette(palette());
        break;
    case QEvent::ApplicationFontChange:
    case QEvent::FontChange:
        d->sizeHint = QSize(); // invalidate size hint
        if (d->container)
            d->container->setFont(font());
        if (d->lineEdit)
            d->updateLineEditGeometry();
        break;
    default:
        break;
    }
    QWidget::changeEvent(e);
}

/*!
    \reimp
*/

void QComboBox::resizeEvent(QResizeEvent *)
{
    d->updateLineEditGeometry();
}

/*!
    \reimp
*/

void QComboBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt = d->getStyleOption();
    style().drawComplexControl(QStyle::CC_ComboBox, &opt, &painter, this);

    // draw the icon and text
    if (d->currentIndex.isValid()) {
        QString txt = model()->data(d->currentIndex, QAbstractItemModel::DisplayRole).toString();
        const QPixmap &pix = pixmap(currentItem());
        QRect editField = QStyle::visualRect(q->style().querySubControlMetrics(
                                                 QStyle::CC_ComboBox, &opt,
                                                 QStyle::SC_ComboBoxEditField, this), q);
        QRect textRect(editField);

        if (!pix.isNull()) {
            QRect pixRect(editField);
            pixRect.setWidth(pix.width() +  4);
            painter.setClipRect(pixRect);
	    painter.drawPixmap(pixRect.left() + 2, (height() - pix.height()) / 2, pix);
            textRect.setLeft(pixRect.right());
        }

        if (!txt.isNull() && !isEditable()) {
            textRect.setLeft(textRect.left() + 1);
            textRect.setRight(textRect.right() -1 );
            painter.setClipRect(textRect);
            painter.drawText(textRect, Qt::AlignAuto|Qt::AlignVCenter, txt);
        }
    }
}

/*!
    \reimp
*/

void QComboBox::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionComboBox opt = d->getStyleOption();
    QStyle::SubControl sc = style().querySubControl(QStyle::CC_ComboBox, &opt, e->pos(), this);
    if ((sc == QStyle::SC_ComboBoxArrow || (sc == QStyle::SC_ComboBoxEditField && !isEditable()))
        && !d->container->isVisible()) {
        if (sc == QStyle::SC_ComboBoxArrow)
            d->arrowDown = true;
        popup();
    }
}

/*!
    \reimp
*/
void QComboBox::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    d->arrowDown = false;
    QStyleOptionComboBox opt = d->getStyleOption();
    update(QStyle::visualRect(style().querySubControlMetrics(
                                  QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow), this));
}

/*!
    \reimp
*/

void QComboBox::keyPressEvent(QKeyEvent *e)
{
    int newRow = -1;
    QModelIndex newIndex;
    switch (e->key()) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        // skip autoCompletion if Delete or Backspace has been pressed
        d->skipCompletion = true;
        e->ignore();
        return;
    case Qt::Key_PageUp:
    case Qt::Key_Up:
        newRow = currentItem() - 1;
        break;
    case Qt::Key_Down:
        if (e->modifiers() & Qt::AltModifier) {
            popup();
            return;
        }
        // fall through
    case Qt::Key_PageDown:
        newRow = currentItem() + 1;
        break;
    case Qt::Key_Home:
        if (isEditable()) {
            e->ignore();
            return;
        }
        newRow = 0;
        break;
    case Qt::Key_End:
        if (isEditable()) {
            e->ignore();
            return;
        }
        newRow = d->model->rowCount(root()) - 1;
        break;
    case Qt::Key_F4:
        if (!e->modifiers())
            popup();
        else
            e->ignore();
        return;
    case Qt::Key_Space:
        if (isEditable())
            e->ignore();
        else
            popup();
        return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
        e->ignore();
        return;
    default:
        if (e->text().isEmpty() || isEditable()) {
            e->ignore();
            return;
        }
        // use keyboardSearch from the listView so we do not duplicate code
        listView()->setCurrentIndex(d->currentIndex);
        listView()->keyboardSearch(e->text());
        if (listView()->currentIndex().isValid()
            && listView()->currentIndex() != d->currentIndex)
            newRow = listView()->currentIndex().row();
        break;
    }
    e->accept();
    setCurrentItem(newRow);
}

/*!
    \fn bool QComboBox::editable() const

    Use isEditable() instead.
*/

/*!
    \fn void QComboBox::insertItem(const QPixmap &pixmap, int row)

    Use an insertItem() function that takes a QIcon instead, for
    example, insertItem(QIcon(pixmap), row).
*/

/*!
    \fn void QComboBox::insertItem(const QPixmap &pixmap, const QString &text, int row)


    Use an insertItem() function that takes a QIcon instead, for
    example, insertItem(QIcon(pixmap), text, row).
*/

/*!
    \fn void QComboBox::changeItem(const QString &text, int row)

    Use setItemText() instead.
*/

/*!
    \fn void QComboBox::changeItem(const QPixmap &pixmap, int row)

    Use setItemIcon() instead, for example,
    setItemIcon(QIcon(pixmap), row).
*/

/*!
    \fn void QComboBox::changeItem(const QPixmap &pixmap, const QString &text, int row)

    Use setItem() instead, for example, setItem(QIcon(pixmap), text row).
*/


#include "moc_qcombobox.cpp"
