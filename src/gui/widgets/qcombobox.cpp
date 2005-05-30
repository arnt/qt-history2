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
#include <qstylepainter.h>
#include <qlineedit.h>
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
#include <qdebug.h>

QComboBoxPrivate::QComboBoxPrivate()
    : QWidgetPrivate(),
      model(0),
      lineEdit(0),
      container(0),
      insertPolicy(QComboBox::InsertAtBottom),
      sizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow),
      minimumContentsLength(0),
      shownOnce(false),
      autoCompletion(true),
      duplicatesEnabled(false),
      skipCompletion(false),
      frame(true),
      maxVisibleItems(10),
      maxCount(INT_MAX),
      modelColumn(0),
      arrowState(QStyle::State_None),
      hoverControl(QStyle::SC_None)
{
}

QStyleOptionMenuItem QComboMenuDelegate::getStyleOption(const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const
{
    QStyleOptionMenuItem menuOption;
    menuOption.palette = QApplication::palette("QMenu");
    menuOption.state = QStyle::State_None;
    if (mCombo->window()->isActiveWindow())
        menuOption.state = QStyle::State_Active;
    if (option.state & QStyle::State_Enabled)
        menuOption.state |= QStyle::State_Enabled;
    if (option.state & QStyle::State_Selected)
        menuOption.state |= QStyle::State_Selected;
    menuOption.checkType = QStyleOptionMenuItem::NonExclusive;
    menuOption.checked = mCombo->currentIndex() == index.row();
    menuOption.menuItemType = QStyleOptionMenuItem::Normal;
    menuOption.icon = qvariant_cast<QIcon>(index.model()->data(index, Qt::DecorationRole));
    menuOption.text = index.model()->data(index, Qt::DisplayRole).toString();
    menuOption.tabWidth = 0;
    menuOption.maxIconWidth = 0;
    menuOption.maxIconWidth =  option.decorationSize.width() + 4;
    menuOption.menuRect = option.rect;
    menuOption.rect = option.rect;
    extern QHash<QByteArray, QFont> *qt_app_fonts_hash();
    menuOption.font = qt_app_fonts_hash()->value("QComboMenuItem", mCombo->font());
    return menuOption;
}

void QComboBoxPrivate::updateArrow(QStyle::StateFlag state)
{
    Q_Q(QComboBox);
    if (arrowState == state)
        return;
    arrowState = state;
    QStyleOptionComboBox opt = getStyleOption();
    q->update(q->style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow));
}

/*!
    \internal
*/
void QComboBoxPrivate::modelDestroyed()
{
    Q_Q(QComboBox);
    model = 0;
    q->setModel(new QStandardItemModel(0, 1, q));
}

/*!
    \internal
*/
bool QComboBoxPrivate::updateHoverControl(const QPoint &pos)
{

    Q_Q(QComboBox);
    QRect lastHoverRect = hoverRect;
    QStyle::SubControl lastHoverControl = hoverControl;
    bool doesHover = q->testAttribute(Qt::WA_Hover);
    if (lastHoverControl != newHoverControl(pos) && doesHover) {
        q->update(lastHoverRect);
        q->update(hoverRect);
        return true;
    }
    return !doesHover;
}

/*!
    \internal
*/
QStyle::SubControl QComboBoxPrivate::newHoverControl(const QPoint &pos)
{
    Q_Q(QComboBox);
    QStyleOptionComboBox opt = getStyleOption();
    opt.subControls = QStyle::SC_All;
    hoverControl = q->style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt, pos, q);
    hoverRect = (hoverControl != QStyle::SC_None)
                   ? q->style()->subControlRect(QStyle::CC_ComboBox, &opt, hoverControl, q)
                   : QRect();
    return hoverControl;
}

/*!
    \internal
*/

QComboBoxPrivateContainer::QComboBoxPrivateContainer(QAbstractItemView *itemView, QComboBox *parent)
    : QFrame(parent, Qt::Popup), combo(parent), view(0), top(0), bottom(0)
{
    // we need the combobox and itemview
    Q_ASSERT(parent);
    Q_ASSERT(itemView);

    // setup container
    setFrameStyle(QFrame::StyledPanel|QFrame::Plain);
    setLineWidth(1);
    blockMouseReleaseTimer.setSingleShot(true);

    // we need a vertical layout
    QBoxLayout *layout =  new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setSpacing(0);
    layout->setMargin(0);

    // set item view
    setItemView(itemView);

    // add scroller arrows if style needs them
    QStyleOptionComboBox opt = comboStyleOption();
    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
        top = new QComboBoxPrivateScroller(QAbstractSlider::SliderSingleStepSub, this);
        bottom = new QComboBoxPrivateScroller(QAbstractSlider::SliderSingleStepAdd, this);
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

void QComboBoxPrivateContainer::scrollItemView(int action)
{
    if (view->verticalScrollBar())
        view->verticalScrollBar()->triggerAction(static_cast<QAbstractSlider::SliderAction>(action));
}

/*
  \internal

  Hides or shows the scrollers when we emulate a popupmenu
*/
void QComboBoxPrivateContainer::updateScrollers()
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
        if (needTop)
            top->show();
        else
            top->hide();
        if (needBottom)
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
void QComboBoxPrivateContainer::setCurrentIndex(const QModelIndex &index)
{
    view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

/*
  \internal

  Returns the item view used for the combobox popup.
*/
QAbstractItemView *QComboBoxPrivateContainer::itemView() const
{
    return view;
}

/*!
  \internal

  Sets the item view to be used for the combobox popup.
*/
void QComboBoxPrivateContainer::setItemView(QAbstractItemView *itemView)
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
    qobject_cast<QBoxLayout*>(layout())->insertWidget(top ? 1 : 0, view);
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
int QComboBoxPrivateContainer::spacing() const
{
    QListView *lview = qobject_cast<QListView*>(view);
    if (lview)
        return lview->spacing();
    return 0;
}

/*!
    \internal
*/

bool QComboBoxPrivateContainer::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::KeyPress:
        switch (static_cast<QKeyEvent*>(e)->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (view->currentIndex().isValid()) {
                combo->hidePopup();
                emit itemSelected(view->currentIndex());
            }
            return true;
        case Qt::Key_Down:
            if (!(static_cast<QKeyEvent*>(e)->modifiers() & Qt::AltModifier))
                break;
            // fall through
        case Qt::Key_F4:
        case Qt::Key_Escape:
            combo->hidePopup();
            return true;
        default:
            break;
        }
    break;
    case QEvent::MouseButtonRelease: {
        QMouseEvent *m = static_cast<QMouseEvent *>(e);
        if (isVisible() && view->rect().contains(m->pos()) && view->currentIndex().isValid()
            && !blockMouseReleaseTimer.isActive()) {
            combo->hidePopup();
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
void QComboBoxPrivateContainer::hideEvent(QHideEvent *)
{
    emit resetButton();
}

void QComboBoxPrivateContainer::mousePressEvent(QMouseEvent *e)
{

    QStyleOptionComboBox opt = comboStyleOption();
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_ComboBoxArrow;
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt,
                                                           combo->mapFromGlobal(e->globalPos()),
                                                           combo);
    if ((combo->isEditable() && sc == QStyle::SC_ComboBoxArrow)
        || (!combo->isEditable() && sc != QStyle::SC_None))
        setAttribute(Qt::WA_NoMouseReplay);
    QFrame::mousePressEvent(e);
}

void QComboBoxPrivateContainer::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    emit resetButton();
}

/*!
  \internal
*/
QStyleOptionComboBox QComboBoxPrivateContainer::comboStyleOption() const
{
    QStyleOptionComboBox opt;
    opt.state = QStyle::State_None;
    opt.rect = combo->rect();
    opt.palette = combo->palette();
    opt.init(combo);
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
    \enum QComboBox::SizeAdjustPolicy

    This enum specifies how the size of the QComboBox should adjust
    when new content is added or content changes.

    \value AdjustToContents              The combobox will always adjust to the contens
    \value AdjustToContentsOnFirstShow   The combobox will adjust to its contents the first time it is show.
    \value AdjustToMinimumContentsLength The combobox only adjusts to the \c minimumContentsLength
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
    Q_D(QComboBox);
    d->init();
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QComboBox::QComboBox(QWidget *parent, const char *name) :
    QWidget(*new QComboBoxPrivate(), parent, 0)
{
    Q_D(QComboBox);
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
    Q_D(QComboBox);
    d->init();
    setEditable(rw);
    setObjectName(name);
}

#endif //QT3_SUPPORT

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
    editTextChanged() signal is emitted.

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
    set with setCurrentIndex(). The number of items in the combobox is
    returned by count(); the maximum number of items can be set with
    setMaxCount(). You can allow editing using setEditable(). For
    editable comboboxes you can set auto-completion using
    setAutoCompletion() and whether or not the user can add duplicates
    is set with setDuplicatesEnabled().

    \inlineimage macintosh-combobox.png Screenshot in Macintosh style
    \inlineimage windows-combobox.png Screenshot in Windows style

    \sa QLineEdit, QSpinBox, QRadioButton, QButtonGroup,
        {fowler}{GUI Design Handbook: Combo Box, Drop-Down List Box}
*/

/*!
    \internal
*/

void QComboBoxPrivate::init()
{
    Q_Q(QComboBox);
    q->setModel(new QStandardItemModel(0, 1, q));
    q->setFocusPolicy(Qt::WheelFocus);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

QComboBoxPrivateContainer* QComboBoxPrivate::viewContainer()
{
    if (container)
        return container;

    Q_Q(QComboBox);
    container = new QComboBoxPrivateContainer(new QListView(), q);
    container->itemView()->setModel(model);
    QStyleOptionComboBox opt = getStyleOption();
    if (q->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, q) && !q->isEditable())
        q->setItemDelegate(new QComboMenuDelegate(container->itemView(), q));
    container->setLayoutDirection(Qt::LayoutDirection(
                                    q->style()->styleHint(QStyle::SH_ComboBox_LayoutDirection,
                                                          &opt, q)));
    QObject::connect(container, SIGNAL(itemSelected(QModelIndex)),
                     q, SLOT(itemSelected(QModelIndex)));
    QObject::connect(container->itemView()->selectionModel(),
                     SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                     q, SLOT(emitHighlighted(QModelIndex)));
    QObject::connect(container, SIGNAL(resetButton()), q, SLOT(resetButton()));
    return container;
}


void QComboBoxPrivate::resetButton()
{
    updateArrow(QStyle::State_None);
}

void QComboBoxPrivate::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_Q(QComboBox);
    if (topLeft.parent() != root)
        return;

    if (sizeAdjustPolicy == QComboBox::AdjustToContents) {
        sizeHint = QSize();
        q->updateGeometry();
    }

    if (currentIndex.row() >= topLeft.row() && currentIndex.row() <= bottomRight.row()) {
        if (lineEdit) {
            lineEdit->setText(q->itemText(currentIndex.row()));
            updateLineEditGeometry();
        }
        q->update();
    }
}

void QComboBoxPrivate::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_Q(QComboBox);
    if (parent != root)
        return;

    if (sizeAdjustPolicy == QComboBox::AdjustToContents) {
        sizeHint = QSize();
        q->updateGeometry();
    }

    // make sure if the combobox was empty and the current index is
    // not valid that the first item is set to be current
    if (start == 0 && (end - start + 1) == q->count() && !currentIndex.isValid())
        q->setCurrentIndex(0);
}

void QComboBoxPrivate::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(QComboBox);
    if (parent != root)
        return;

    if (sizeAdjustPolicy == QComboBox::AdjustToContents) {
        sizeHint = QSize();
        q->updateGeometry();
    }

    if (currentIndex.row() >= start && currentIndex.row() <= end) {
        if (end < (q->count() - 1))
            q->setCurrentIndex(end + 1);
        else if (start > 0)
            q->setCurrentIndex(start - 1);
        else {
            currentIndex = QPersistentModelIndex();
            if (lineEdit) {
                lineEdit->setText(QString());
                updateLineEditGeometry();
            }
            q->update();
        }
    }
}

QStyleOptionComboBox QComboBoxPrivate::getStyleOption() const
{
    Q_Q(const QComboBox);
    QStyleOptionComboBox opt;
    opt.init(q);
    if (!q->isEditable() && q->hasFocus())
        opt.state |= QStyle::State_Selected;
    opt.subControls = QStyle::SC_All;
    if (arrowState == QStyle::State_Sunken) {
        opt.activeSubControls = QStyle::SC_ComboBoxArrow;
        opt.state |= arrowState;
    } else {
        opt.activeSubControls = hoverControl;
    }
    opt.editable = q->isEditable();
    opt.frame = frame;
    if (currentIndex.isValid()) {
        opt.currentText = q->currentText();
        opt.currentIcon = q->itemIcon(currentIndex.row());
    }
    opt.iconSize = q->iconSize();

    return opt;
}

void QComboBoxPrivate::updateLineEditGeometry()
{
    if (!lineEdit)
        return;

    Q_Q(QComboBox);
    QStyleOptionComboBox opt = getStyleOption();
    QRect editRect = q->style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                                QStyle::SC_ComboBoxEditField, q);
    if (!q->itemIcon(q->currentIndex()).isNull()) {
        QRect comboRect(editRect);
        editRect.setWidth(editRect.width() - q->iconSize().width() - 4);
        editRect = QStyle::alignedRect(q->layoutDirection(), Qt::AlignRight,
                                       editRect.size(), comboRect);
    }
    lineEdit->setGeometry(editRect);
}

void QComboBoxPrivate::returnPressed()
{
    Q_Q(QComboBox);
    if (lineEdit && !lineEdit->text().isEmpty()) {
        QString text = lineEdit->text();
        // check for duplicates (if not enabled) and quit
        int index = -1;
        if (!duplicatesEnabled) {
            index = q->findText(text);
            if (index != -1) {
                q->setCurrentIndex(index);
                emitActivated(currentIndex);
                return;
            }
        }
        switch (insertPolicy) {
        case QComboBox::InsertAtTop:
            index = 0;
            break;
        case QComboBox::InsertAtBottom:
            index = q->count();
            break;
        case QComboBox::InsertAtCurrent:
        case QComboBox::InsertAfterCurrent:
        case QComboBox::InsertBeforeCurrent:
            if (!q->count() || !currentIndex.isValid())
                index = 0;
            else if (insertPolicy == QComboBox::InsertAtCurrent)
                q->setItemText(q->currentIndex(), text);
            else if (insertPolicy == QComboBox::InsertAfterCurrent)
                index = q->currentIndex() + 1;
            else if (insertPolicy == QComboBox::InsertBeforeCurrent)
                index = q->currentIndex();
            break;
        case QComboBox::NoInsert:
        default:
            break;
        }
        if (index >= 0) {
            q->insertItem(index, text);
            q->setCurrentIndex(index);
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
    if (skipCompletion || !lineEdit || !autoCompletion) {
        skipCompletion = false;
        return;
    }
    QString text = lineEdit->text();
    if (!text.isEmpty()) {
        QModelIndexList list = model->match(currentIndex, Qt::EditRole, text);
        if (!list.count())
            return;
        QString completed = model->data(list.first(), Qt::EditRole).toString();
        int start = completed.length();
        int length = text.length() - start; // negative length
        lineEdit->setText(completed);
        lineEdit->setSelection(start, length);
    }
}

void QComboBoxPrivate::itemSelected(const QModelIndex &item)
{
    Q_Q(QComboBox);
    if (item != currentIndex) {
        q->setCurrentIndex(item.row());
    } else if (lineEdit) {
        lineEdit->selectAll();
        lineEdit->setText(q->itemText(currentIndex.row()));
    }
    emitActivated(currentIndex);
}

void QComboBoxPrivate::emitActivated(const QModelIndex &index)
{
    Q_Q(QComboBox);
    QString text(q->model()->data(index, Qt::EditRole).toString());
    emit q->activated(index.row());
    emit q->activated(text);
}

void QComboBoxPrivate::emitHighlighted(const QModelIndex &index)
{
    Q_Q(QComboBox);
    if (!index.isValid())
        return;
    QString text(q->model()->data(index, Qt::EditRole).toString());
    emit q->highlighted(index.row());
    emit q->highlighted(text);
}

/*!
    Destroys the combobox.
*/
QComboBox::~QComboBox()
{
    // ### check delegateparent and delete delegate if us?
    Q_D(QComboBox);

    disconnect(d->model, SIGNAL(destroyed()),
               this, SLOT(modelDestroyed()));
}

/*!
    \property QComboBox::maxVisibleItems
    \brief the maximum allowed size on screen of the combobox
*/

int QComboBox::maxVisibleItems() const
{
    Q_D(const QComboBox);
    return d->maxVisibleItems;
}

void QComboBox::setMaxVisibleItems(int maxItems)
{
    Q_D(QComboBox);
    if (maxItems > 0)
        d->maxVisibleItems = maxItems;
}

/*!
    \property QComboBox::count
    \brief the number of items in the combobox
*/

int QComboBox::count() const
{
    Q_D(const QComboBox);
    return d->model->rowCount(rootModelIndex());
}

/*!
    \property QComboBox::maxCount
    \brief the maximum number of items allowed in the combobox

    Note: If you set the maximum number to be less then the current
    amount of items in the combobox, the extra items will be
    truncated. This also applies if you have set an external model on
    the combobox.
*/

void QComboBox::setMaxCount(int max)
{
    Q_D(QComboBox);
    if (max < count())
        model()->removeRows(max, count() - max, rootModelIndex());

    d->maxCount = max;
}

int QComboBox::maxCount() const
{
    Q_D(const QComboBox);
    return d->maxCount;
}

/*!
    \property QComboBox::autoCompletion
    \brief whether the combobox provides auto-completion for editable items

    \sa editable
*/

bool QComboBox::autoCompletion() const
{
    Q_D(const QComboBox);
    return d->autoCompletion;
}

void QComboBox::setAutoCompletion(bool enable)
{
    Q_D(QComboBox);
    d->autoCompletion = enable;
}

/*!
    \property QComboBox::duplicatesEnabled
    \brief whether the combobox can contain duplicate items
*/

bool QComboBox::duplicatesEnabled() const
{
    Q_D(const QComboBox);
    return d->duplicatesEnabled;
}

void QComboBox::setDuplicatesEnabled(bool enable)
{
    Q_D(QComboBox);
    d->duplicatesEnabled = enable;
}

/*!  \fn int QComboBox::findText(const QString &text, Qt::MatchFlags flags = Qt::MatchExactly|Qt::MatchCaseSensitive) const

  Returns the index of the item containing the given \a text; otherwise
  returns -1.

  The \a flags specify how the items in the combobox are searched.
*/

/*!
  Returns the index of the item containing the given \a data for the
  given \a role; otherwise returns -1.

  The \a flags specify how the items in the combobox are searched.
*/
int QComboBox::findData(const QVariant &data, int role, Qt::MatchFlags flags) const
{
    Q_D(const QComboBox);
    QModelIndexList result;
    QModelIndex start = model()->index(0, d->modelColumn, rootModelIndex());
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
    Q_D(const QComboBox);
    return d->insertPolicy;
}

void QComboBox::setInsertPolicy(InsertPolicy policy)
{
    Q_D(QComboBox);
    d->insertPolicy = policy;
}

/*!
    \property QComboBox::sizeAdjustPolicy
    \brief the policy describing how the size of the combobox changes
    when the content changes


    The default value is \c AdjustToContentsOnFirstShow

    \sa SizeAdjustPolicy
*/

QComboBox::SizeAdjustPolicy QComboBox::sizeAdjustPolicy() const
{
    Q_D(const QComboBox);
    return d->sizeAdjustPolicy;
}

void QComboBox::setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy policy)
{
    Q_D(QComboBox);
    if (policy == d->sizeAdjustPolicy)
        return;

    d->sizeAdjustPolicy = policy;
    d->sizeHint = QSize();
    updateGeometry();
}

/*!
    \property QComboBox::minimumContentsLength
    \brief the value describing the minimum number of characters that
    will fit into the combobox.

    This value is ignored if \c AdjustToMinimumContentsLength is not
    set. The default value is 0.

    \sa sizeAdjustPolicy
*/

int QComboBox::minimumContentsLength() const
{
    Q_D(const QComboBox);
    return d->minimumContentsLength;
}

void QComboBox::setMinimumContentsLength(int characters)
{
    Q_D(QComboBox);
    if (characters == d->minimumContentsLength || characters < 0)
        return;

    d->minimumContentsLength = characters;

    if (d->sizeAdjustPolicy == AdjustToMinimumContentsLength) {
        d->sizeHint = QSize();
        updateGeometry();
    }
}

/*!
    \property QComboBox::iconSize
    \brief the size of the icons shown in the combobox.

    Unless explicitly set this returns the default value of the current style.
*/

QSize QComboBox::iconSize() const
{
    Q_D(const QComboBox);
    if (d->iconSize.isValid())
        return d->iconSize;

    int iconWidth = style()->pixelMetric(QStyle::PM_SmallIconSize);
    return QSize(iconWidth, iconWidth);
}

void QComboBox::setIconSize(const QSize &size)
{
    Q_D(QComboBox);
    if (size == d->iconSize)
        return;

    view()->setIconSize(size);
    d->iconSize = size;
    d->sizeHint = QSize();
    updateGeometry();
}

/*!
    \property QComboBox::editable
    \brief whether the combobox can be edited by the user
*/

bool QComboBox::isEditable() const
{
    Q_D(const QComboBox);
    return d->lineEdit != 0;
}

void QComboBox::setEditable(bool editable)
{
    Q_D(QComboBox);
    if (isEditable() == editable)
        return;
    setAttribute(Qt::WA_InputMethodEnabled, editable);

    QStyleOptionComboBox opt = d->getStyleOption();
    if (editable) {
        if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
            setItemDelegate(new QItemDelegate(view()));
            d->viewContainer()->updateScrollers();
            view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        QLineEdit *le = new QLineEdit(this);
        le->setLayoutDirection(Qt::LayoutDirection(
                               style()->styleHint(QStyle::SH_ComboBox_LayoutDirection,
                                                  &opt, this)));
        setLineEdit(le);
    } else {
        if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
            setItemDelegate(new QComboMenuDelegate(view(), this));
            d->viewContainer()->updateScrollers();
            view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
    Q_D(QComboBox);
    if ( !edit ) {
	Q_ASSERT(edit != 0);
	return;
    }
    if (!(model()->flags(model()->index(0, d->modelColumn, rootModelIndex()))
                & Qt::ItemIsEditable)) {
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
    connect(d->lineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(editTextChanged(QString)));
#ifdef QT3_SUPPORT
    connect(d->lineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
#endif
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
    Q_D(const QComboBox);
    return d->lineEdit;
}

/*!
    \fn void QComboBox::setValidator(const QValidator *validator)

    Sets the \a validator to use instead of the current validator.
*/

void QComboBox::setValidator(const QValidator *v)
{
    Q_D(QComboBox);
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
    Q_D(const QComboBox);
    return d->lineEdit ? d->lineEdit->validator() : 0;
}

/*!
    Returns the item delegate used by the popup list view.

    \sa setItemDelegate()
*/
QAbstractItemDelegate *QComboBox::itemDelegate() const
{
    return view()->itemDelegate();
}

/*!
    Sets the item \a delegate for the popup list view.
    The combobox takes ownership of the delegate.

    \sa itemDelegate()
*/
void QComboBox::setItemDelegate(QAbstractItemDelegate *delegate)
{
    Q_ASSERT(delegate);
    delete view()->itemDelegate();
    view()->setItemDelegate(delegate);
}

/*!
    Returns the model used by the combobox.
*/

QAbstractItemModel *QComboBox::model() const
{
    Q_D(const QComboBox);
    return d->model;
}

/*!
    Sets the model to be \a model. \a model must not be 0.
    If you want to clear the contents of a model, call clear().

    \sa clear()
*/

void QComboBox::setModel(QAbstractItemModel *model)
{
    Q_D(QComboBox);
    Q_ASSERT(model);

    if (d->model) {
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(rowsInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(destroyed()),
                   this, SLOT(modelDestroyed()));
        if (d->model->QObject::parent() == this)
            delete d->model;
    }

    d->model = model;

    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(dataChanged(QModelIndex,QModelIndex)));
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(rowsInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(destroyed()),
            this, SLOT(modelDestroyed()));

    if (d->container)
        d->container->itemView()->setModel(model);
}

/*!
   \internal

    Returns the root model item index for the items in the combobox.

*/

QModelIndex QComboBox::rootModelIndex() const
{
    Q_D(const QComboBox);
    return QModelIndex(d->root);
}

/*!
    \internal

    Sets the root model item \a index for the items in the combobox.
*/

void QComboBox::setRootModelIndex(const QModelIndex &index)
{
    Q_D(QComboBox);
    d->root = QPersistentModelIndex(index);
    view()->setRootIndex(index);
    update();
}

/*!
    \property QComboBox::currentIndex
    \brief the index of the current item in the combobox. The index
    can change when inserting or removing items.
*/

int QComboBox::currentIndex() const
{
    Q_D(const QComboBox);
    return d->currentIndex.row();
}

void QComboBox::setCurrentIndex(int index)
{
    Q_D(QComboBox);
    if (!model())
        return;
    QModelIndex mi = model()->index(index, d->modelColumn, rootModelIndex());
    if (!mi.isValid() || mi == d->currentIndex)
        return;
    d->currentIndex = QPersistentModelIndex(mi);
    if (d->lineEdit) {
        d->lineEdit->setText(itemText(d->currentIndex.row()));
        d->updateLineEditGeometry();
    }
    update();
}

/*!
    \property QComboBox::currentText
    \brief the text of the current item
*/

QString QComboBox::currentText() const
{
    Q_D(const QComboBox);
    if (d->lineEdit)
        return d->lineEdit->text();
    else if (d->currentIndex.isValid())
	return model()->data(d->currentIndex, Qt::EditRole).toString();
    else
	return QString();
}

/*!
    Returns the text for the given \a index in the combobox.
*/

QString QComboBox::itemText(int index) const
{
    Q_D(const QComboBox);
    QModelIndex mi = model()->index(index, d->modelColumn, rootModelIndex());
    return model()->data(mi, Qt::EditRole).toString();
}

/*!
    Returns the icon for the given \a index in the combobox.
*/
QIcon QComboBox::itemIcon(int index) const
{
    Q_D(const QComboBox);
    QModelIndex item = model()->index(index, d->modelColumn, rootModelIndex());
    return qvariant_cast<QIcon>(model()->data(item, Qt::DecorationRole))
        .pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize),
                isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

/*!
   Returns the data for the given \a role in the given \a index in the
   combobox, or QVariant::Invalid if there is no data for this role.
*/
QVariant QComboBox::itemData(int index, int role) const
{
    Q_D(const QComboBox);
    QModelIndex mi = model()->index(index, d->modelColumn, rootModelIndex());
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
    Q_D(QComboBox);
    if (!(count() < d->maxCount))
        return;
    if (index < 0)
        index = 0;
    else if (index > count())
        index = count();
    QModelIndex item;
    if (model()->insertRows(index, 1, rootModelIndex())) {
        item = model()->index(index, d->modelColumn, rootModelIndex());
        QMap<int, QVariant> values;
        values.insert(Qt::EditRole, text);
        values.insert(Qt::DecorationRole, icon);
        values.insert(Qt::UserRole, userData);
        model()->setItemData(item, values);
    }
}

/*!
    Inserts the strings from the \a list into the combobox as separate items,
    starting at the \a index specified.
*/
void QComboBox::insertItems(int index, const QStringList &list)
{
    Q_D(QComboBox);
    if (list.isEmpty() || list.count() + count() > d->maxCount)
        return;

    if (index < 0)
        index = count();

    if (model()->insertRows(index, list.count(), rootModelIndex())) {
        QModelIndex item;
        for (int i = 0; i < list.count(); ++i) {
            item = model()->index(i+index, d->modelColumn, rootModelIndex());
            model()->setData(item, list.at(i), Qt::EditRole);
        }
    }
}

/*!
    \internal

    Removes the item at the given \a index from the combobox.
    This will update the current index if the index is removed.
*/
void QComboBox::removeItem(int index)
{
    Q_ASSERT(index >= 0 && index < count());
    model()->removeRows(index, 1, rootModelIndex());
}

/*!
    Sets the \a text for the item on the given \a index in the combobox.
*/

void QComboBox::setItemText(int index, const QString &text)
{
    Q_D(const QComboBox);
    QModelIndex item = model()->index(index, d->modelColumn, rootModelIndex());
    if (item.isValid()) {
        model()->setData(item, text, Qt::EditRole);
    }
}

/*!
    Sets the \a icon for the item on the given \a index in the combobox.
*/

void QComboBox::setItemIcon(int index, const QIcon &icon)
{
    Q_D(const QComboBox);
    QModelIndex item = model()->index(index, d->modelColumn, rootModelIndex());
    if (item.isValid()) {
        model()->setData(item, icon, Qt::DecorationRole);
    }
}

/*!
    Sets the data \a role for the item on the given \a index in the combobox
    to the specified \a value.
*/
void QComboBox::setItemData(int index, const QVariant &value, int role)
{
    Q_D(const QComboBox);
    QModelIndex item = model()->index(index, d->modelColumn, rootModelIndex());
    if (item.isValid()) {
        model()->setData(item, value, role);
    }
}

/*!
    Returns the list view used for the combobox popup.
*/
QAbstractItemView *QComboBox::view() const
{
    Q_D(const QComboBox);
    return const_cast<QComboBoxPrivate*>(d)->viewContainer()->itemView();
}

/*!
  Sets the view to be used in the combobox popup to the given \a
  itemView. The combobox takes ownership of the view.
*/
void QComboBox::setView(QAbstractItemView *itemView)
{
    Q_D(QComboBox);
    Q_ASSERT(itemView);
    itemView->setModel(d->model);
    d->viewContainer()->setItemView(itemView);
}

/*!
    \reimp

    This implementation caches the size hint to avoid resizing when
    the contents change dynamically. To invalidate the cached value
    change the \c sizeAdjustPolicy.
*/
QSize QComboBox::sizeHint() const
{
    Q_D(const QComboBox);
    if (d->sizeHint.isValid())
        return d->sizeHint.expandedTo(QApplication::globalStrut());

    // find out if we have any icons
    bool hasIcon = false;
    for (int i = 0; i < count() && !hasIcon; ++i) {
        if (!itemIcon(i).isNull())
            hasIcon = true;
    }

    // height
    const QFontMetrics &fm = fontMetrics();
    d->sizeHint.setHeight(qMax(fm.lineSpacing(), 14) + 2);
    if (hasIcon)
        d->sizeHint.setHeight(qMax(d->sizeHint.height(), iconSize().height() + 2));

    // text width
    switch (d->sizeAdjustPolicy) {
    case AdjustToContents:
    case AdjustToContentsOnFirstShow:
        if (count() == 0) {
            d->sizeHint.rwidth() = 7 * fm.width(QChar('x'));
        } else {
            for (int i = 0; i < count(); ++i) {
                d->sizeHint.setWidth(qMax(d->sizeHint.width(), fm.width(itemText(i))));
            }
        }
        break;
    case AdjustToMinimumContentsLength:
        d->sizeHint.setWidth(d->minimumContentsLength*fm.width('X'));
        break;
    default:
        break;
    }

    // add icon width
    d->sizeHint.setWidth(d->sizeHint.width() + (hasIcon ? iconSize().width() + 4 : 0));

    // add style and strut values
    QStyleOptionComboBox opt = d->getStyleOption();
    d->sizeHint = style()->sizeFromContents(QStyle::CT_ComboBox, &opt, d->sizeHint, this);
    return d->sizeHint.expandedTo(QApplication::globalStrut());
}

/*!
    Displays the list of items in the combobox. If the list is empty then
    the no items will be shown.
*/
void QComboBox::showPopup()
{
    Q_D(QComboBox);
    if (count() <= 0)
        return;

    // set current item and select it
    view()->selectionModel()->setCurrentIndex(d->currentIndex,
                                              QItemSelectionModel::ClearAndSelect);
    QComboBoxPrivateContainer* container = d->viewContainer();
    // use top item as height for complete listView
    int itemHeight = view()->sizeHintForIndex(model()->index(0, d->modelColumn, rootModelIndex())).height()
                     + container->spacing();
    QRect listRect(rect());

    QStyleOptionComboBox opt = d->getStyleOption();
    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this))
        listRect.setHeight(itemHeight * count());
    else
        listRect.setHeight(itemHeight * qMin(d->maxVisibleItems, count()));
    listRect.setHeight(listRect.height() + 2*container->spacing()
                       + style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2);

    // make sure the widget fits on screen
    //### do horizontally as well
    QRect screen = QApplication::desktop()->availableGeometry(this);
    QPoint below = mapToGlobal(rect().bottomLeft());
    int belowHeight = screen.bottom() - below.y();
    QPoint above = mapToGlobal(rect().topLeft());
    int aboveHeight = above.y() - screen.y();
    if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
        listRect.moveTopLeft(above);
        listRect.moveTop(listRect.top() - view()->visualRect(view()->currentIndex()).top());
        if (listRect.top() < screen.top())
            listRect.moveTopLeft(below);
        listRect.setBottom(qMin(screen.bottom(), listRect.bottom()));
    } else if (listRect.height() <= belowHeight) {
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

    container->setGeometry(listRect);
    view()->scrollTo(view()->currentIndex());
    container->raise();
    container->show();
    view()->setFocus();
}

/*!
    Hides the list of items in the combobox if it is currently visible;
    otherwise this function does nothing.
*/
void QComboBox::hidePopup()
{
    Q_D(QComboBox);
    if (d->container && d->container->isVisible())
        d->container->hide();
}

/*!
    \internal

    Clears the combobox, removing all items.

    Note: If you have set an external model on the combobox this model
    will still be cleared when calling this function.
*/

void QComboBox::clear()
{
    model()->removeRows(0, model()->rowCount(rootModelIndex()), rootModelIndex());
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
    Q_D(QComboBox);
    if (d->lineEdit)
        d->lineEdit->clear();
}

/*!
    Sets the \a text in the combobox's text edit.
*/

void QComboBox::setEditText(const QString &text)
{
    Q_D(QComboBox);
    if (d->lineEdit)
        d->lineEdit->setText(text);
}

/*!
    \reimp
*/

void QComboBox::focusInEvent(QFocusEvent *e)
{
    Q_D(QComboBox);
    update();
    if (d->lineEdit)
        d->lineEdit->event(e);
}

/*!
    \reimp
*/

void QComboBox::focusOutEvent(QFocusEvent *e)
{
    Q_D(QComboBox);
    update();
    if (d->lineEdit)
        d->lineEdit->event(e);
}

/*! \reimp */
void QComboBox::changeEvent(QEvent *e)
{
    Q_D(QComboBox);
    switch (e->type()) {
    case QEvent::StyleChange:
        d->sizeHint = QSize(); // invalidate size hint
        if (d->lineEdit)
            d->updateLineEditGeometry();
        //### need to update scrollers etc. as well here
        break;
    case QEvent::EnabledChange:
        if (!isEnabled())
            hidePopup();
        break;
    case QEvent::PaletteChange:
        d->viewContainer()->setPalette(palette());
        break;
    case QEvent::FontChange:
        d->sizeHint = QSize(); // invalidate size hint
        d->viewContainer()->setFont(font());
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
    Q_D(QComboBox);
    d->updateLineEditGeometry();
}

/*!
    \reimp
*/

void QComboBox::paintEvent(QPaintEvent *)
{
    Q_D(QComboBox);
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt = d->getStyleOption();
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // draw the icon and text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

/*!
    \reimp
*/
void QComboBox::showEvent(QShowEvent *e)
{
    Q_D(QComboBox);
    if (!d->shownOnce && d->sizeAdjustPolicy == QComboBox::AdjustToContentsOnFirstShow) {
        d->sizeHint = QSize();
        updateGeometry();
    }
    d->shownOnce = true;
    QWidget::showEvent(e);
}

/*!
    \reimp
*/
void QComboBox::hideEvent(QHideEvent *)
{
    hidePopup();
}

/*!
    \reimp
*/
bool QComboBox::event(QEvent *event)
{
    Q_D(QComboBox);
    switch(event->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
    if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event))
        d->updateHoverControl(he->pos());
        break;
    case QEvent::ShortcutOverride:
        if (d->lineEdit)
            return d->lineEdit->event(event);
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

/*!
    \reimp
*/
void QComboBox::mousePressEvent(QMouseEvent *e)
{
    Q_D(QComboBox);
    QStyleOptionComboBox opt = d->getStyleOption();
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt, e->pos(),
                                                           this);
    if ((sc == QStyle::SC_ComboBoxArrow || !isEditable())
        && !d->viewContainer()->isVisible()) {
        if (sc == QStyle::SC_ComboBoxArrow)
            d->updateArrow(QStyle::State_Sunken);
        d->viewContainer()->blockMouseReleaseTimer.start(QApplication::doubleClickInterval());
        showPopup();
    }
}

/*!
    \reimp
*/
void QComboBox::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QComboBox);
    Q_UNUSED(e);
    d->updateArrow(QStyle::State_None);
}

/*!
    \reimp
*/

void QComboBox::keyPressEvent(QKeyEvent *e)
{
    Q_D(QComboBox);
    int newIndex = currentIndex();
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
            showPopup();
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
            showPopup();
            return;
        }
        break;
    case Qt::Key_Space:
        if (!d->lineEdit) {
            showPopup();
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
            view()->setCurrentIndex(d->currentIndex);
            view()->keyboardSearch(e->text());
            if (view()->currentIndex().isValid()
                && view()->currentIndex() != d->currentIndex)
                newIndex = view()->currentIndex().row();
        }
    }

    if (newIndex >= 0 && newIndex < count() && newIndex != currentIndex()) {
        setCurrentIndex(newIndex);
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
    Q_D(QComboBox);
    if (d->lineEdit)
        d->lineEdit->event(e);
}

/*!
    \reimp
*/
void QComboBox::wheelEvent(QWheelEvent *e)
{
    Q_D(QComboBox);
    if (!d->viewContainer()->isVisible()) {
        int newIndex = currentIndex();

        if (e->delta() > 0)
            --newIndex;
        else
            ++newIndex;

        if (newIndex >= 0 && newIndex < count()) {
            setCurrentIndex(newIndex);
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
    Q_D(QComboBox);
    if (d->lineEdit)
        d->lineEdit->event(e);
}

/*!
    \reimp
*/
QVariant QComboBox::inputMethodQuery(Qt::InputMethodQuery query) const
{
    Q_D(const QComboBox);
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

    \sa currentIndex()
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


/*!
    \property QComboBox::frame
    \brief whether the combo box draws itself with a frame

    If enabled (the default) the combo box draws itself inside a
    frame, otherwise the combo box draws itself without any frame.
*/
bool QComboBox::hasFrame() const
{
    Q_D(const QComboBox);
    return d->frame;
}


void QComboBox::setFrame(bool enable)
{
    Q_D(QComboBox);
    d->frame = enable;
    update();
    updateGeometry();
}

/*!
    \property QComboBox::modelColumn
    \brief the column in the model that is visible
*/

int QComboBox::modelColumn() const
{
    Q_D(const QComboBox);
    return d->modelColumn;
}

void QComboBox::setModelColumn(int visibleColumn)
{
    Q_D(QComboBox);
    d->modelColumn = visibleColumn;
    QListView *lv = qobject_cast<QListView *>(d->viewContainer()->itemView());
    if (lv)
        lv->setModelColumn(visibleColumn);
}

/*!
    \fn int QComboBox::currentItem() const

    Use currentIndex() instead.
*/

/*!
    \fn void QComboBox::setCurrentItem(int)

    Use setCurrentIndex(int) instead.
*/

/*!
    \fn void QComboBox::popup()

    Use showPopup() instead.
*/

/*!
    \fn void QComboBox::textChanged(const QString &text)

    Use the editTextChanged(const QString &text) signal instead.
*/

/*!
    \typedef QComboBox::Policy
    \compat

    Use QComboBox::InsertPolicy instead.
*/


#include "moc_qcombobox.cpp"
