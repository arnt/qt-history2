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
#include <qstandarditemmodel.h>
#include <qmap.h>
#include <qevent.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <private/qcombobox_p.h>
#define d d_func()
#define q q_func()


QStyleOptionMenuItem MenuDelegate::getStyleOption(const QStyleOptionViewItem &option,
                                                      const QModelIndex &index) const {
    QStyleOptionMenuItem menuOption;

    menuOption.palette = QApplication::palette("QMenu");
    menuOption.state = QStyle::State_None;
    if (mCombo->topLevelWidget()->isActiveWindow())
        menuOption.state = QStyle::State_Active;
    if (option.state & QStyle::State_Enabled)
        menuOption.state |= QStyle::State_Enabled;
    if (option.state & QStyle::State_Selected)
        menuOption.state |= QStyle::State_Selected;
    menuOption.checkType = QStyleOptionMenuItem::NonExclusive;
    menuOption.checked = mCombo->currentItem() == index.row();
    menuOption.menuItemType = QStyleOptionMenuItem::Normal;
    menuOption.icon = index.model()->data(index, QAbstractItemModel::DecorationRole).toIcon();
    menuOption.text = index.model()->data(index, QAbstractItemModel::DisplayRole).toString();
    menuOption.tabWidth = 0;
    menuOption.maxIconWidth = 0;
    menuOption.maxIconWidth =  option.decorationSize.width() + 4;
    menuOption.menuRect = option.rect;
    menuOption.rect = option.rect;
    extern QHash<QByteArray, QFont> *qt_app_fonts_hash();
    menuOption.font = qt_app_fonts_hash()->value("QComboMenuItem", mCombo->font());
    return menuOption;
}

/*!
    \internal
*/

ItemViewContainer::ItemViewContainer(QAbstractItemView *itemView, QComboBox *parent)
    : QFrame(parent), combo(parent), view(0), top(0), bottom(0)
{
    // we need the combobox
    Q_ASSERT(parent);

    // setup container
    setFrameStyle(QFrame::Box|QFrame::Plain);
    setLineWidth(1);

    // we need a vertical layout
    QBoxLayout *layout =  new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setSpacing(0);
    layout->setMargin(0);

    // set item view
    setItemView(itemView);

    // add scroller arrows if style needs them
    QStyleOptionComboBox opt = comboStyleOption();
    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
        top = new Scroller(QAbstractSlider::SliderSingleStepSub, this);
        bottom = new Scroller(QAbstractSlider::SliderSingleStepAdd, this);
    }
    if (top) {
        layout->insertWidget(0, top);
        connect(top, SIGNAL(doScroll(int)), this, SLOT(scrollItemView(int)));
    }
    if (bottom) {
        layout->addWidget(bottom);
        connect(bottom, SIGNAL(doScroll(int)), this, SLOT(scrollItemView(int)));
    }
}

/*!
    \internal
*/

void ItemViewContainer::scrollItemView(int action)
{
    if (view->verticalScrollBar())
        view->verticalScrollBar()->triggerAction(static_cast<QAbstractSlider::SliderAction>(action));
}

/*
  \internal

  Hides or shows the scrollers when we emulate a popupmenu
*/
void ItemViewContainer::updateScrollers()
{
    if (!top || !bottom)
        return;

    QStyleOptionComboBox opt = comboStyleOption();
    if (!combo->isEditable() &&
        combo->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo) &&
        view->verticalScrollBar()->minimum() < view->verticalScrollBar()->maximum()) {
        bool needTop = view->verticalScrollBar()->value()
                       > (view->verticalScrollBar()->minimum() + spacing());
        bool needBottom = view->verticalScrollBar()->value()
                          < (view->verticalScrollBar()->maximum() - spacing()*2);
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

  Sets currentIndex on entered if the LeftButton is not pressed. This
  means that if mouseTracking(...) is on, we setCurrentIndex and select
  even when LeftButton is not pressed.
*/
void ItemViewContainer::setCurrentIndex(const QModelIndex &index)
{
    view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

/*
  \internal

  Returns the item view used for the combobox popup.
*/
QAbstractItemView *ItemViewContainer::itemView() const
{
    return view;
}

/*!
  \internal

  Sets the item view to be used for the combobox popup
*/
void ItemViewContainer::setItemView(QAbstractItemView *itemView)
{
    Q_ASSERT(itemView);

    // clean up old one
    if (view) {
        view->removeEventFilter(this);
        view->viewport()->removeEventFilter(this);
        disconnect(view->verticalScrollBar(), SIGNAL(valueChanged(int)),
                   this, SLOT(updateScrollers()));
        disconnect(view->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
                   this, SLOT(updateScrollers()));
        disconnect(view, SIGNAL(entered(QModelIndex)),
                   this, SLOT(setCurrentIndex(QModelIndex)));
        delete view;
        view = 0;
    }

    // setup the item view
    view = itemView;
    view->setParent(this);
    qt_cast<QBoxLayout*>(layout())->insertWidget(top ? 1 : 0, view);
    view->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    view->installEventFilter(this);
    view->viewport()->installEventFilter(this);
    QStyleOptionComboBox opt = comboStyleOption();
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo)
        && !combo->isEditable())
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (style()->styleHint(QStyle::SH_ComboBox_ListMouseTracking, &opt, combo) ||
        style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo)) {
        view->setMouseTracking(true);
    }
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setFrameStyle(QFrame::NoFrame);
    view->setLineWidth(0);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(view->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(updateScrollers()));
    connect(view->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
            this, SLOT(updateScrollers()));
    connect(view, SIGNAL(entered(QModelIndex)),
            this, SLOT(setCurrentIndex(QModelIndex)));
}

/*!
  \internal

  returns the spacing between the items in the view
*/
int ItemViewContainer::spacing() const
{
    QListView *lview = qt_cast<QListView*>(view);
    if (lview)
        return lview->spacing();
    return 0;
}

/*!
    \internal
*/

bool ItemViewContainer::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::KeyPress:
        switch (static_cast<QKeyEvent*>(e)->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (view->currentIndex().isValid()) {
                if (combo->autoHide())
                    hide();
                emit itemSelected(view->currentIndex());
            }
            return true;
        case Qt::Key_Down:
            if (!(static_cast<QKeyEvent*>(e)->modifiers() & Qt::AltModifier))
                break;
            // fall through
        case Qt::Key_F4:
        case Qt::Key_Escape:
            hide();
            return true;
        default:
            break;
        }
    break;
    case QEvent::MouseButtonRelease: {
        QMouseEvent *m = static_cast<QMouseEvent *>(e);
        if (isVisible() && view->rect().contains(m->pos()) && view->currentIndex().isValid()) {
            if (combo->autoHide())
                hide();
            emit itemSelected(view->currentIndex());
            return true;
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
void ItemViewContainer::hideEvent(QHideEvent *)
{
    emit containerDisappearing();
}

void ItemViewContainer::mousePressEvent(QMouseEvent *e)
{
    QRect ignoreRect = combo->rect();
    if (combo->isEditable()) {
        QStyleOptionComboBox opt = comboStyleOption();
        opt.subControls = QStyle::SC_All;
        opt.activeSubControls = QStyle::SC_ComboBoxArrow;
        ignoreRect = QStyle::visualRect(opt.direction, opt.rect,
                                        style()->subControlRect(
                                            QStyle::CC_ComboBox, &opt,
                                            QStyle::SC_ComboBoxArrow, combo));
    }
    ignoreRect = QRect(combo->mapToGlobal(ignoreRect.topLeft()),
                       combo->mapToGlobal(ignoreRect.bottomRight()));
    if (ignoreRect.contains(e->globalPos()))
        setAttribute(Qt::WA_NoMouseReplay);
    QFrame::mousePressEvent(e);
}

/*!
  \internal
*/
QStyleOptionComboBox ItemViewContainer::comboStyleOption() const
{
    QStyleOptionComboBox opt;
    opt.state = QStyle::State_None;
    opt.rect = combo->rect();
    opt.palette = combo->palette();
    opt.init(combo);
    if (combo->isEditable() && combo->lineEdit()->hasFocus())
        opt.state |= QStyle::State_HasFocus;
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_None;
    opt.editable = combo->isEditable();
    return opt;
}



/*!
    \enum QComboBox::InsertPolicy

    This enum specifies what the QComboBox should do when a new string is
    entered by the user.

    \value NoInsert            The string will not be inserted into the combobox.
    \value InsertAtTop         The string will be inserted as the first item in the combobox.
    \value InsertAtCurrent     The current item will be \e replaced by the string.
    \value InsertAtBottom      The string will be inserted after the last item in the combobox.
    \value InsertAfterCurrent  The string is inserted after the current item in the combobox.
    \value InsertBeforeCurrent The string is inserted before the current item in the combobox.
    \omitvalue NoInsertion
    \omitvalue AtTop
    \omitvalue AtCurrent
    \omitvalue AtBottom
    \omitvalue AfterCurrent
    \omitvalue BeforeCurrent
*/

/*!
    \fn void QComboBox::activated(int index)

    This signal is sent when an item in the combobox is activated. The
    item's \a index is given.

*/

/*!
    \fn void QComboBox::activated(const QString &text)

    This signal is sent when an item in the combobox is activated. The
    item's \a text is given.

*/

/*!
    \fn void QComboBox::highlighted(int index)

    This signal is sent when an item in the combobox is highlighted. The
    item's \a index is given.
*/

/*!
    \fn void QComboBox::highlighted(const QString &text)

    This signal is sent when an item in the combobox is highlighted. The
    item's \a text is given.
*/

/*!
    Constructs a combobox with the given \a parent, using the default
    model QStandardItemModel.
*/
QComboBox::QComboBox(QWidget *parent) :
    QWidget(*new QComboBoxPrivate(), parent, 0)
{
    d->init();
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
    list box can be limited with maxVisibleItems() and setMaxCount()
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
    this using setInsertPolicy().

    It is possible to constrain the input to an editable combobox
    using QValidator; see setValidator(). By default, any input is
    accepted.

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

    \inlineimage macintosh-combobox.png Screenshot in Macintosh style
    \inlineimage windows-combobox.png Screenshot in Windows style

    \sa QLineEdit QSpinBox QRadioButton QButtonGroup
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
    container = new ItemViewContainer(l, q);
    container->setParent(q, Qt::WType_Popup);
    q->setModel(new QStandardItemModel(0, 1, q));
    q->setFocusPolicy(Qt::TabFocus);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    q->setCurrentItem(0);
    QStyleOptionComboBox opt = d->getStyleOption();
    if (q->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, q) && !q->isEditable())
        q->setItemDelegate(new MenuDelegate(l, q));
    QObject::connect(container, SIGNAL(itemSelected(QModelIndex)),
                     q, SLOT(itemSelected(QModelIndex)));
    QObject::connect(q->itemView()->selectionModel(),
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
    opt.state = QStyle::State_None;
    opt.rect = q->rect();
    opt.palette = q->palette();
    opt.init(q);
    if (q->isEditable() && q->lineEdit()->hasFocus())
        opt.state |= QStyle::State_HasFocus;
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

    QStyleOptionComboBox opt = getStyleOption();
    QRect editorRect = QStyle::visualRect(opt.direction, opt.rect, q->style()->subControlRect(
                                              QStyle::CC_ComboBox, &opt,
                                              QStyle::SC_ComboBoxEditField, q));
    const QPixmap &pix = q->itemIcon(q->currentItem()).pixmap(q->style()->pixelMetric(QStyle::PM_SmallIconSize));
    if (!pix.isNull())
        editorRect.setLeft(editorRect.left() + pix.width() + 4);
    lineEdit->setGeometry(editorRect);
}

void QComboBoxPrivate::returnPressed()
{
    if (lineEdit && !lineEdit->text().isEmpty()) {
        QString text = lineEdit->text();
        // check for duplicates (if not enabled) and quit
        int index = -1;
        if (!d->duplicatesEnabled) {
            index = q->findText(text);
            if (index != -1) {
                q->setCurrentItem(index);
                emitActivated(currentIndex);
                return;
            }
        }
        switch (insertPolicy) {
        case QComboBox::AtTop:
            index = 0;
            break;
        case QComboBox::AtBottom:
            index = q->count();
            break;
        case QComboBox::AtCurrent:
        case QComboBox::AfterCurrent:
        case QComboBox::BeforeCurrent:
            if (!q->count() || !currentIndex.isValid())
                index = 0;
            else if (insertPolicy == QComboBox::AtCurrent)
                q->setItemText(q->currentItem(), text);
            else if (insertPolicy == QComboBox::AfterCurrent)
                index = q->currentItem() + 1;
            else if (insertPolicy == QComboBox::BeforeCurrent)
                index = q->currentItem();
            break;
        case QComboBox::NoInsert:
        default:
            break;
        }
        if (index >= 0) {
            q->insertItem(index, text);
            q->setCurrentItem(index);
            emitActivated(currentIndex);
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
        QModelIndexList list = model->match(currentIndex, QAbstractItemModel::EditRole, text);
        if (!list.count())
            return;
        QString completed = model->data(list.first(), QAbstractItemModel::EditRole).toString();
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
    } else if (lineEdit) {
        lineEdit->selectAll();
        lineEdit->setText(model->data(currentIndex, QAbstractItemModel::EditRole).toString());
    }
    emitActivated(currentIndex);
}

void QComboBoxPrivate::emitActivated(const QModelIndex &index)
{
    QString text(q->model()->data(index, QAbstractItemModel::EditRole).toString());
    emit q->activated(index.row());
    emit q->activated(text);
}

void QComboBoxPrivate::emitHighlighted(const QModelIndex &index)
{
    QString text(q->model()->data(index, QAbstractItemModel::EditRole).toString());
    emit q->highlighted(index.row());
    emit q->highlighted(text);
}

/*!
    Destroys the combobox.
*/

QComboBox::~QComboBox()
{
    // ### check delegateparent and delete delegate if us?
}

/*!
    \property QComboBox::maxVisibleItems
    \brief the maximum allowed size on screen of the combobox
*/

int QComboBox::maxVisibleItems() const
{
    return d->maxVisibleItems;
}

void QComboBox::setMaxVisibleItems(int maxItems)
{
    if (maxItems > 0)
        d->maxVisibleItems = maxItems;
}

/*!
    \property QComboBox::count
    \brief the number of items in the combobox
*/

int QComboBox::count() const
{
    return d->model->rowCount(rootModelIndex());
}

/*!
    \property QComboBox::maxCount
    \brief the maximum number of items allowed in the combobox
*/

void QComboBox::setMaxCount(int max)
{
    if (max < count())
        model()->removeRows(max, count() - max, rootModelIndex());

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
  \internal

  Returns true if the combobox hides the popup listbox when an item is
  activated
*/
bool QComboBox::autoHide() const
{
    return d->autoHide;
}

/*!
  \internal

  When \a enable is set to true the combobox hides the popup listbox
  when an item is activated
*/
void QComboBox::setAutoHide(bool enable)
{
    d->autoHide = enable;
}

/*!
  \fn int QComboBox::findText(const QString &text, QAbstractItemModel::MatchFlags flags) const

  Returns the index of the item containing the given \a text; otherwise
  returns -1.

  The \a flags specify how the items in the combobox are searched.
*/

/*!
  Returns the index of the item containing the given \a data for the
  given \a role; otherwise returns -1.

  The \a flags specify how the items in the combobox are searched.
*/
int QComboBox::findData(const QVariant &data, int role, QAbstractItemModel::MatchFlags flags) const
{
    QModelIndexList result;
    QModelIndex start = model()->index(0, 0, rootModelIndex());
    result = model()->match(start, role, data, 1, flags);
    if (result.isEmpty())
        return -1;
    return result.first().row();
}

/*!
    \property QComboBox::insertPolicy
    \brief the policy used to determine where user-inserted items should
    appear in the combobox

    The default value is \c AtBottom, indicating that new items will appear
    at the bottom of the list of items.

    \sa InsertPolicy
*/

QComboBox::InsertPolicy QComboBox::insertPolicy() const
{
    return d->insertPolicy;
}

void QComboBox::setInsertPolicy(InsertPolicy policy)
{
    d->insertPolicy = policy;
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

    QStyleOptionComboBox opt = d->getStyleOption();
    if (editable) {
        if (q->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
            setItemDelegate(new QItemDelegate(itemView()));
            d->container->updateScrollers();
            itemView()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        setLineEdit(new QLineEdit(this));
    } else {
        if (q->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
            setItemDelegate(new MenuDelegate(itemView(), this));
            d->container->updateScrollers();
            itemView()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
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
    if (!(model()->flags(model()->index(0, 0, rootModelIndex())) & QAbstractItemModel::ItemIsEditable)) {
        delete edit;
        return;
    }

    edit->setText(currentText());
    delete d->lineEdit;

    d->lineEdit = edit;
    if (d->lineEdit->parent() != this)
	d->lineEdit->setParent(this);
    connect(d->lineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
    connect(d->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(complete()));
    d->lineEdit->setFrame(false);
    d->lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    d->lineEdit->setFocusProxy(this);
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
    return itemView()->itemDelegate();
}

/*!
    Sets the item \a delegate for the popup list view.
    The combobox takes ownership of the delegate.

    \sa itemDelegate()
*/
void QComboBox::setItemDelegate(QAbstractItemDelegate *delegate)
{
    Q_ASSERT(delegate);
    delete itemView()->itemDelegate();
    itemView()->setItemDelegate(delegate);
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
    d->container->itemView()->setModel(model);
}

/*!
    \internal

    Returns the root model item index for the items in the combobox.

*/

QModelIndex QComboBox::rootModelIndex() const
{
    return QModelIndex(d->root);
}

/*!
    \internal

    Sets the root model item \a index for the items in the combobox.
*/

void QComboBox::setRootModelIndex(const QModelIndex &index)
{
    QModelIndex old = d->root;
    d->root = QPersistentModelIndex(index);
    itemView()->setRootIndex(index);
    update();
}

/*!
    \property QComboBox::currentItem
*/

int QComboBox::currentItem() const
{
    return d->currentIndex.row();
}

void QComboBox::setCurrentItem(int index)
{
    if (!model())
        return;
    QModelIndex mi = model()->index(index, 0, rootModelIndex());
    if (!mi.isValid() || mi == d->currentIndex)
        return;
    QModelIndex old = d->currentIndex;
    d->currentIndex = QPersistentModelIndex(mi);
    if (d->lineEdit)
        d->lineEdit->setText(
            model()->data(d->currentIndex, QAbstractItemModel::EditRole).toString());
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

/*!
    Returns the text for the given \a index in the combobox.
*/

QString QComboBox::itemText(int index) const
{
    QModelIndex mi = model()->index(index, 0, rootModelIndex());
    return model()->data(mi, QAbstractItemModel::EditRole).toString();
}

/*!
    Returns the icon for the given \a index in the combobox.
*/
QIcon QComboBox::itemIcon(int index) const
{
    QStyleOptionComboBox opt = d->getStyleOption();
    QModelIndex item = model()->index(index, 0, rootModelIndex());
    return model()->data(item, QAbstractItemModel::DecorationRole).toIcon()
        .pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize),
                opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled);
}

/*!
   Returns the data for the given \a role in the given \a index in the
   combobox, or QVariant::Invalid if there is no data for this role.
*/
QVariant QComboBox::itemData(int index, int role) const
{
    QModelIndex mi = model()->index(index, 0, rootModelIndex());
    return model()->data(mi, role);
}

/*!
  \fn void QComboBox::insertItem(int index, const QString &text, const QVariant &userData)

  Inserts the \a text and \a userData into the combobox at the given \a index.
*/

/*!
    Inserts the \a icon, \a text and \a userData into the combobox at the given \a index.
*/
void QComboBox::insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userData)
{
    if (!(count() < d->maxCount))
        return;
    if (index < 0)
        index = count();
    QModelIndex item;
    if (model()->insertRows(index, 1, rootModelIndex())) {
        item = model()->index(index, 0, rootModelIndex());
        QMap<int, QVariant> values;
        values.insert(QAbstractItemModel::EditRole, text);
        values.insert(QAbstractItemModel::DecorationRole, icon);
        values.insert(QAbstractItemModel::UserRole, userData);
        model()->setItemData(item, values);
        if (!d->currentIndex.isValid()) {
            setCurrentItem(index);
        } else if (item == d->currentIndex) {
            update();
            if (d->lineEdit)
                d->lineEdit->setText(text);
        }
    }
}

/*!
    Inserts the strings from the \a list into the combobox as separate items,
    starting at the \a index specified.
*/
void QComboBox::insertItems(int index, const QStringList &list)
{
    if (list.isEmpty() || list.count() + count() > d->maxCount)
        return;

    if (index < 0)
        index = count();

    if (model()->insertRows(index, list.count(), rootModelIndex())) {
        QModelIndex item;
        for (int i = 0; i < list.count(); ++i) {
            item = model()->index(i+index, 0, rootModelIndex());
            model()->setData(item, list.at(i), QAbstractItemModel::EditRole);
            if (!d->currentIndex.isValid()) {
                setCurrentItem(i+index);
            } else if (item == d->currentIndex) {
                update();
                if (d->lineEdit)
                    d->lineEdit->setText(list.at(i));
            }
        }
    }
}

/*!
    \internal

    Removes the item at the given \a index from the combobox.
*/
void QComboBox::removeItem(int index)
{
    model()->removeRows(index, 1, rootModelIndex());
}

/*!
    Sets the \a text for the item on the given \a index in the combobox.
*/

void QComboBox::setItemText(int index, const QString &text)
{
    QModelIndex item = model()->index(index, 0, rootModelIndex());
    if (item.isValid()) {
        model()->setData(item, text, QAbstractItemModel::EditRole);
    }
}

/*!
    Sets the \a icon for the item on the given \a index in the combobox.
*/

void QComboBox::setItemIcon(int index, const QIcon &icon)
{
    QModelIndex item = model()->index(index, 0, rootModelIndex());
    if (item.isValid()) {
        model()->setData(item, icon, QAbstractItemModel::DecorationRole);
    }
}

/*!
    Sets the data \a role for the item on the given \a index in the combobox
    to the specified \a value.
*/
void QComboBox::setItemData(int index, const QVariant &value, int role)
{
    QModelIndex item = model()->index(index, 0, rootModelIndex());
    if (item.isValid()) {
        model()->setData(item, value, role);
    }
}

/*!
    Returns the list view used for the combobox popup.
*/
QAbstractItemView *QComboBox::itemView() const
{
    return d->container->itemView();
}

/*!
  Sets the view to be used in the combobox popup to the given \a itemView.
*/
void QComboBox::setItemView(QAbstractItemView *itemView)
{
    Q_ASSERT(itemView);
    itemView->setModel(d->model);
    d->container->setItemView(itemView);
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
    for (int i = 0; i < model()->rowCount(rootModelIndex()); ++i) {
        index = model()->index(i, 0, rootModelIndex());
        txt = itemText(index.row());
        const QPixmap &pix = itemIcon(index.row()).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
        // check listview item width
        maxWidth = itemView()->sizeHintForIndex(index).width();
        // check combo text+pixmap width
        maxWidth = qMax(maxWidth, fm.width(txt) + 2 + (pix.isNull() ? 0 : pix.width() + 4));
        if (maxWidth > d->sizeHint.width())
            d->sizeHint.setWidth(maxWidth);
    }

    // add style and strut values
    QStyleOptionComboBox opt = d->getStyleOption();
    d->sizeHint = (style()->sizeFromContents(QStyle::CT_ComboBox, &opt, d->sizeHint, this) .expandedTo(QApplication::globalStrut()));
    return d->sizeHint;
}

/*!
    Displays the list of items in the combobox. If the list is empty then
    no items will be shown.
*/

void QComboBox::popup()
{
    if (count() <= 0)
        return;

    // set current item and select it
    itemView()->selectionModel()->setCurrentIndex(d->currentIndex,
                                                  QItemSelectionModel::ClearAndSelect);

    // use top item as height for complete listView
    int itemHeight = itemView()->sizeHintForIndex(model()->index(0, 0, rootModelIndex())).height()
                     + d->container->spacing();
    QRect listRect(rect());
    listRect.setHeight(itemHeight * qMin(d->maxVisibleItems, count())
                       + 2*d->container->spacing() + 2);

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
    itemView()->ensureVisible(itemView()->currentIndex());
    d->container->raise();
    d->container->show();
    itemView()->setFocus();
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
    model()->removeRows(0, model()->rowCount(rootModelIndex()), rootModelIndex());
    // ### shouldn't be necessary, model should update persistentindexes
    QModelIndex old = d->currentIndex;
    d->currentIndex = QPersistentModelIndex();
    if (d->lineEdit)
        d->lineEdit->setText(QString::null);
}

/*!
    \fn void QComboBox::clearValidator()

    Use setValidator(0) instead.
*/

/*!
    Clears the contents of the line edit used for editing in the combobox.
*/

void QComboBox::clearEditText()
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
    \reimp
*/

void QComboBox::focusInEvent(QFocusEvent *e)
{
    update();
    if (d->lineEdit)
        d->lineEdit->event(e);
}

/*!
    \reimp
*/

void QComboBox::focusOutEvent(QFocusEvent *e)
{
    update();
    if (d->lineEdit)
        d->lineEdit->event(e);
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
    style()->drawComplexControl(QStyle::CC_ComboBox, &opt, &painter, this);

    // draw the icon and text
    if (d->currentIndex.isValid()) {
        QString txt = model()->data(d->currentIndex, QAbstractItemModel::DisplayRole).toString();
        const QPixmap &pix = itemIcon(currentItem()).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
        QRect editField = QStyle::visualRect(opt.direction, opt.rect, q->style()->subControlRect(
                                                 QStyle::CC_ComboBox, &opt,
                                                 QStyle::SC_ComboBoxEditField, this));
        if (!pix.isNull()) {
            QRect pixRect(editField);
            pixRect.setWidth(pix.width() +  4);
            painter.setClipRect(pixRect);
            painter.drawPixmap(pixRect.left() + 2, (height() - pix.height()) / 2, pix);
            editField.setLeft(pixRect.right());
        }

        if (!txt.isNull() && !isEditable()) {
            painter.setClipRect(editField);
            painter.drawText(editField.adjusted(1, 0, -1, 0), Qt::AlignLeft|Qt::AlignVCenter, txt);
        }
    }
}

/*!
    \reimp
*/

void QComboBox::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionComboBox opt = d->getStyleOption();
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt, e->pos(),
                                                           this);
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
    update(QStyle::visualRect(opt.direction, opt.rect, style()->subControlRect(
                                  QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow)));
}

/*!
    \reimp
*/

void QComboBox::keyPressEvent(QKeyEvent *e)
{
    int newIndex = currentItem();
    switch (e->key()) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        // skip autoCompletion if Delete or Backspace has been pressed
        d->skipCompletion = true;
        break;
    case Qt::Key_PageUp:
    case Qt::Key_Up:
        --newIndex;
        break;
    case Qt::Key_Down:
        if (e->modifiers() & Qt::AltModifier) {
            popup();
            return;
        }
        // fall through
    case Qt::Key_PageDown:
        ++newIndex;
        break;
    case Qt::Key_Home:
        if (!d->lineEdit)
            newIndex = 0;
        break;
    case Qt::Key_End:
        if (!d->lineEdit)
            newIndex = count() - 1;
        break;
    case Qt::Key_F4:
        if (!e->modifiers()) {
            popup();
            return;
        }
        break;
    case Qt::Key_Space:
        if (!d->lineEdit) {
            popup();
            return;
        }
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
        if (!d->lineEdit)
            e->ignore();
        break;
    default:
        if (!d->lineEdit && !e->text().isEmpty()) {
            // use keyboardSearch from the listView so we do not duplicate code
            itemView()->setCurrentIndex(d->currentIndex);
            itemView()->keyboardSearch(e->text());
            if (itemView()->currentIndex().isValid()
                && itemView()->currentIndex() != d->currentIndex)
                newIndex = itemView()->currentIndex().row();
        }
    }

    if (newIndex >= 0 && newIndex < count() && newIndex != currentItem()) {
        setCurrentItem(newIndex);
        d->emitActivated(d->currentIndex);
    } else if (d->lineEdit) {
        d->lineEdit->event(e);
    }
}


/*!
    \reimp
*/

void QComboBox::keyReleaseEvent(QKeyEvent *e)
{
    if (d->lineEdit)
        d->lineEdit->event(e);
}

/*!
    \reimp
*/
void QComboBox::wheelEvent(QWheelEvent *e)
{
    if (!d->container->isVisible()) {
        int newIndex = currentItem();

        if (e->delta() > 0)
            --newIndex;
        else
            ++newIndex;

        if (newIndex >= 0 && newIndex < count()) {
            setCurrentItem(newIndex);
            d->emitActivated(d->currentIndex);
        }
        e->accept();
    }
}

/*!
    \reimp
*/
void QComboBox::inputMethodEvent(QInputMethodEvent *e)
{
    if (d->lineEdit)
        d->lineEdit->event(e);
}

/*!
    \reimp
*/
QVariant QComboBox::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (d->lineEdit)
        return d->lineEdit->inputMethodQuery(query);
    return QWidget::inputMethodQuery(query);
}

/*!
    \fn bool QComboBox::editable() const

    Use isEditable() instead.
*/

/*!
    \fn void QComboBox::insertItem(const QPixmap &pixmap, int index)

    Use an insertItem() function that takes a QIcon instead, for
    example, insertItem(index, QIcon(pixmap)).
*/

/*!
    \fn void QComboBox::insertItem(const QPixmap &pixmap, const QString &text, int index)


    Use an insertItem() function that takes a QIcon instead, for
    example, insertItem(index, QIcon(pixmap), text).
*/

/*!
    \fn void QComboBox::changeItem(const QString &text, int index)

    Use setItemText() instead.
*/

/*!
    \fn void QComboBox::changeItem(const QPixmap &pixmap, int index)

    Use setItemIcon() instead, for example,
    setItemIcon(index, QIcon(pixmap)).
*/

/*!
    \fn void QComboBox::changeItem(const QPixmap &pixmap, const QString &text, int index)

    Use setItem() instead, for example, setItem(index, QIcon(pixmap),text).
*/

/*!
    \fn void QComboBox::addItem(const QString &text, const QVariant &userData)

    Adds an item to the combobox with the given \a text, and containing the
    specified \a userData. The item is appended to the list of existing items.
*/

/*!
    \fn void QComboBox::addItem(const QIcon &icon, const QString &text,
                                const QVariant &userData)

    Adds an item to the combobox with the given \a icon and \a text, and
    containing the specified \a userData. The item is appended to the list of
    existing items.
*/

/*!
    \fn void QComboBox::addItems(const QStringList &texts)

    Adds each of the strings in the given \a texts to the combobox. Each item
    is appended to the list of existing items in turn.
*/

/*!
    \fn void QComboBox::editTextChanged(const QString &text)

    This signal is emitted when the text in the combobox's line edit
    widget is changed. The new text is specified by \a text.
*/

/*!
    \fn QComboBox::InsertPolicy QComboBox::insertionPolicy() const
    \compat

    Use QComboBox::insertPolicy instead.
*/

/*!
    \fn void QComboBox::setInsertionPolicy(InsertPolicy policy)
    \compat

    Use QComboBox::insertPolicy instead.
*/

/*!
    \fn void QComboBox::setCurrentText(const QString &text)
    \compat

    Use setItemText() instead.

    \sa currentItem()
*/

/*!
    \fn QString QComboBox::text(int index) const
    \compat

    Use itemText() instead.
*/

/*!
    \fn QPixmap QComboBox::pixmap(int index) const
    \compat

    Use itemIcon() instead.
*/

/*!
    \fn void QComboBox::insertStringList(const QStringList &list, int index)
    \compat

    Use insertItems() instead.
*/

/*!
    \fn void QComboBox::insertItem(const QString &text, int index)
    \compat
*/

/*!
    \fn void QComboBox::clearEdit()
    \compat

    Use clearEditText() instead.
*/

#include "moc_qcombobox.cpp"

