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

#include <qplatformdefs.h>
#include <private/qabstractspinbox_p.h>
#include <qabstractspinbox.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qdebug.h>
#include <qpalette.h>
#include <qspinbox.h>
#include <qstyle.h>

#if defined(Q_WS_X11)
#include <limits.h>
#endif

//#define QABSTRACTSPINBOX_QSBDEBUG
#ifdef QABSTRACTSPINBOX_QSBDEBUG
#  define QASBDEBUG qDebug
#else
#  define QASBDEBUG if (false) qDebug
#endif


#define d d_func()
#define q q_func()

/*!
    \class QAbstractSpinBox
    \brief The QAbstractSpinBox class provides a spinwidget and a line edit to
    display values.

    \ingroup abstractwidgets

    The class is designed as a common super class for widgets like
    QSpinBox, QDoubleSpinBox and QDateTimeEdit

    Here are the main properties of the class:

    \list 1

    \i \l text: The text that is displayed in the QAbstractSpinBox.

    \i \l alignment: The alignment of the text in the QAbstractSpinBox.

    \i \l cleanText: The text of the QAbstractSpinBox exluding any
    prefix/suffix.

    \i \l wrapping: Whether the QAbstractSpinBox wraps from the
    minimum value to the maximum value and vica versa.

    \i \l tracking: Whether the spinbox validates the text and emits
    signals for each change in the input.

    \endlist

    QAbstractSpinBox provides a virtual stepBy() function that is
    called whenever the user triggers a step. This function takes an
    integer value to signify how many steps were taken. E.g. Pressing
    Qt::Key_Down will trigger a call to stepBy(-1).

    QAbstractSpinBox also provide a virtual function stepEnabled() to
    determine whether stepping up/down is allowed at any point. This
    function returns a bitset of StepEnabled.
*/

/*!
    \enum QAbstractSpinBox::StepEnabledFlag

    \value StepNone
    \value StepUpEnabled
    \value StepDownEnabled
*/

/*!
    Constructs an abstract spinbox with the given \a parent with default
    \l wrapping, \l tracking, and \l alignment properties.
*/

QAbstractSpinBox::QAbstractSpinBox(QWidget *parent)
    : QWidget(*new QAbstractSpinBoxPrivate, parent, 0)
{
    d->init();
}

/*!
    \internal
*/
QAbstractSpinBox::QAbstractSpinBox(QAbstractSpinBoxPrivate &dd, QWidget *parent)
    : QWidget(dd, parent, 0)
{
    d->init();
}

/*!
    Called when the QAbstractSpinBox is destroyed.
*/

QAbstractSpinBox::~QAbstractSpinBox()
{
}

/*!
    \enum QAbstractSpinBox::ButtonSymbols

    This enum type determines what the buttons in a spin box show.

    \value UpDownArrows the buttons show little arrows in the classic
    style.

    \value PlusMinus the buttons show \bold{+} and \bold{-} symbols.

    \sa QAbstractSpinBox::buttonSymbols
*/

/*!
    \property QAbstractSpinBox::buttonSymbols

    \brief the current button symbol mode

    The possible values can be either \c UpDownArrows or \c PlusMinus.
    The default is \c UpDownArrows.

    \sa ButtonSymbols
*/

QAbstractSpinBox::ButtonSymbols QAbstractSpinBox::buttonSymbols() const
{
    return d->buttonsymbols;
}

void QAbstractSpinBox::setButtonSymbols(ButtonSymbols bs)
{
    if (d->buttonsymbols != (d->buttonsymbols = bs))
        update();
}

/*!
    \property QAbstractSpinBox::text

    \brief the spin box's text, including any prefix() and suffix()

    There is no default text.
*/

QString QAbstractSpinBox::text() const
{
    return lineEdit()->displayText();
}

/*!
    \property QAbstractSpinBox::cleanText

    \brief the text of the spin box excluding any prefix(), suffix()
    or leading or trailing whitespace.

    \sa text, prefix, suffix
*/

QString QAbstractSpinBox::cleanText() const
{
    if (d->dirty) // needed to make sure text() returns reasonable values if the spin box is not yet shown
        d->updateEdit();

    QString t = d->edit->displayText();
    d->strip(&t);
    return t;
}


/*!
    \property QAbstractSpinBox::tracking

    \brief whether valueChanged is emitted continuously while typing.

    \code
        QSpinBox *sb = new QSpinBox(this);
        sb->setTracking(true);
        // put the keyboard focus in the editor
        // type 1

        // valueChanged(1) is emitted

        // type 2

        // valueChanged(12) is emitted

        // hit return

        // valueChanged(12) is emitted again
    \endcode

    By default, tracking is turned off.

    \sa QSpinBox::minimum(), QSpinBox::maximum()
*/

bool QAbstractSpinBox::tracking() const
{
    return d->tracking;
}

void QAbstractSpinBox::setTracking(bool t)
{
    d->tracking = t;
}

/*!
    \property QAbstractSpinBox::wrapping

    \brief whether the spin box is circular.

    If wrapping is true stepping up from maximum() value will take you
    to the minimum() value and vica versa. Wrapping only make sense if
    you have minimum() and maximum() values set.

        QSpinBox *spinBox = new QSpinBox(this);
        spinBox->setRange(0, 100);
        spinBox->setWrapping(true);
        sb->setValue(100);
        sb->stepBy(1);
        // value is 0
    \endcode

    By default, wrapping is turned off.

    \sa QSpinBox::minimum(), QSpinBox::maximum()
*/

bool QAbstractSpinBox::wrapping() const
{
    return d->wrapping;
}

void QAbstractSpinBox::setWrapping(bool w)
{
    d->wrapping = w;
}


/*!
    \property QAbstractSpinBox::readOnly
    \brief whether the spin box is read only.

    In read-only mode, the user can still copy the text to the
    clipboard, or drag and drop the text;
    but cannot edit it.

    The QLineEdit in the QAbstractSpinBox does not show a cursor in
    read-only mode.

    \sa QLineEdit::setReadOnly, QLineEdit::readOnly
*/

bool QAbstractSpinBox::isReadOnly() const
{
    return d->readonly;
}

void QAbstractSpinBox::setReadOnly(bool enable)
{
    d->readonly = enable;
    d->edit->setReadOnly(enable);
    update();
}

/*!
    \property QAbstractSpinBox::alignment
    \brief the alignment of the spin box

    Possible Values are \c Qt::AlignLeft, \c
    Qt::AlignRight and \c Qt::AlignHCenter.

    By default, the alignment is Qt::AlignLeft

    Attempting to set the alignment to an illegal flag combination
    does nothing.

    \sa Qt::Alignment
*/

Qt::Alignment QAbstractSpinBox::alignment() const
{
    if (d->dirty)
        d->updateEdit();

    return (Qt::Alignment)d->edit->alignment();
}

void QAbstractSpinBox::setAlignment(Qt::Alignment flag)
{
    if (d->dirty)
        d->updateEdit();

    d->edit->setAlignment(flag);
}

/*!
    Selects all the text in the spinbox except the prefix and suffix
*/

void QAbstractSpinBox::selectAll()
{
    if (d->dirty)
        d->updateEdit();

    if (!d->specialValue()) {
        d->edit->setSelection(d->prefix.length(), d->edit->displayText().length()
                              - d->prefix.length() - d->suffix.length());
    } else {
        d->edit->selectAll();
    }
}
/*!
    Clears the lineedit of all text but prefix and suffix
*/

void QAbstractSpinBox::clear()
{
    if (d->dirty)
        d->updateEdit();

    d->edit->setText(d->prefix + d->suffix);
    d->edit->setCursorPosition(d->prefix.size());
}

/*!
    Virtual function that determines whether stepping up and down is
    legal at any given time.

    The up arrow will be painted as disabled unless (stepEnabled() &
    StepUpEnabled) != 0.

    The default implementation will return (StepUpEnabled|
    StepDownEnabled) if wrapping is turned on. Else it will return
    StepDownEnabled if value is > minimum() or'ed with StepUpEnabled if
    value < maximum().

    If you subclass QAbstractSpinBox you will need to reimplement this function.

    \sa QSpinBox::minimum(), QSpinBox::maximum(), wrapping()
*/


QAbstractSpinBox::StepEnabled QAbstractSpinBox::stepEnabled() const
{
    if (d->readonly)
        return StepEnabled(0);
    if (!style()->styleHint(QStyle::SH_SpinControls_DisableOnBounds)
        || d->wrapping)
        return StepEnabled(StepUpEnabled | StepDownEnabled);
    StepEnabled ret = StepNone;
    if (d->value < d->maximum) {
        ret |= StepUpEnabled;
    }
    if (d->value > d->minimum) {
        ret |= StepDownEnabled;
    }
    return ret;
}

/*!
   This virtual function is called by the QAbstractSpinBox to
   determine whether \a input is valid. Reimplemented in the various
   subclasses.
*/

QValidator::State QAbstractSpinBox::validate(QString &/*input*/, int &/*pos*/) const
{
    return QValidator::Acceptable;
}

/*!
   This virtual function is called by the QAbstractSpinBox if the
   input is not validated to QValidator::Acceptable when Return is
   pressed or interpretText() is called. It will try to change the
   text so it is valid. Reimplemented in the various subclasses.
*/

void QAbstractSpinBox::fixup(QString &) const
{
}
/*!
    Virtual function that is called whenever the user triggers a step.
    The \a steps parameter indicates how many steps were taken, e.g.
    Pressing \c Qt::Key_Down will trigger a call to stepBy(-1),
    whereas pressing \c Qt::Key_Prior will trigger a call to
    stepBy(10).

    If you subclass QAbstractSpinBox you must reimplement this
    function. Note that this function is called even if the resulting
    value will be outside the bounds of minimum and maximum. It's this
    function's job to handle these situations.

    \sa QSpinBox::setValue(), QSpinBox::minimum(), QSpinBox::maximum()
*/

void QAbstractSpinBox::stepBy(int steps)
{
    const QVariant old = d->value;
    QString tmp = d->edit->displayText();
    int cursorPos = d->edit->cursorPosition();
    bool dontstep = false;
    EmitPolicy e = EmitIfChanged;
    if (d->pendingemit) {
        dontstep = validate(tmp, cursorPos) != QValidator::Acceptable;
        d->refresh(NeverEmit);
        if (d->value != old)
            e = AlwaysEmit;
    }
    if (!dontstep) {
        d->setValue(d->bound(d->value + (d->singlestep * steps), old, steps), e);
    } else if (e == AlwaysEmit) {
        d->emitSignals(e, old);
    }
    selectAll();
}

/*!
    This function returns a pointer to the line edit of the spin box.
*/

QLineEdit *QAbstractSpinBox::lineEdit() const
{
    if (d->dirty)
        d->updateEdit();

    return d->edit;
}


/*!
    \fn void QAbstractSpinBox::setLineEdit(QLineEdit *lineEdit)

    Sets the line edit of the spinbox to be \a lineEdit instead of the
    current line edit widget.

    If validator() for the \a lineEdit returns 0, the internal validator
    of the spinbox will be set on the line edit.
*/

void QAbstractSpinBox::setLineEdit(QLineEdit *e)
{
    if (!e) {
        Q_ASSERT(e != 0);
        return;
    }
    const QValidator *validator = d->edit ? d->edit->validator() : 0;
    delete d->edit;
    d->edit = e;
    if (d->edit->parent() != this)
        d->edit->setParent(this);

    if (!e->validator() && validator)
        e->setValidator(validator);

    d->edit->setFrame(false);
    d->edit->setAttribute(Qt::WA_InputMethodEnabled, false);
    d->edit->setFocusProxy(this);

    if (d->type != QVariant::Invalid) {
        connect(d->edit, SIGNAL(textChanged(QString)),
                this, SLOT(editorTextChanged(QString)));
        connect(d->edit, SIGNAL(cursorPositionChanged(int,int)),
                this, SLOT(editorCursorPositionChanged(int,int)));
    }
    QStyleOptionSpinBox opt = d->getStyleOption();
    opt.subControls = QStyle::SC_SpinBoxEditField;
    d->edit->setGeometry(QStyle::visualRect(opt.direction, opt.rect,
                                            style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                                    QStyle::SC_SpinBoxEditField,
                                                                    this)));
    d->edit->setContextMenuPolicy(Qt::NoContextMenu);


    if (isVisible())
        d->edit->show();

    update();
}


/*!
    This function interprets the text of the spin box. If the value
    has changed since last interpretation it will emit signals.
*/

void QAbstractSpinBox::interpretText()
{
    d->refresh(EmitIfChanged);
}

/*!
    \reimp
*/

bool QAbstractSpinBox::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
        if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event)) 
            d->updateHoverControl(he->pos());
        break;
    default: break;
    }
    return QWidget::event(event);
}

/*!
    \reimp
*/

void QAbstractSpinBox::showEvent(QShowEvent *)
{
    if (d->dirty) {
        d->resetState();
        d->updateEdit();
    }
    d->updateSpinBox();
}

/*!
    \reimp
*/

void QAbstractSpinBox::changeEvent(QEvent *e)
{
    switch(e->type()) {
        case QEvent::StyleChange:
            d->sizehintdirty = true;
            d->spinclicktimerinterval = style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatRate, 0, this);
            d->resetState();
            break;
        case QEvent::EnabledChange:
            if (!isEnabled()) {
                d->resetState();
            }
            break;
        case QEvent::ActivationChange:
            if (!isActiveWindow()){
                d->resetState();
                if (d->pendingemit) // pendingemit can be true even if it hasn't changed.
                    d->refresh(EmitIfChanged); // E.g. 10 to 10.0
            }
            break;
        default:
            break;
    }
    QWidget::changeEvent(e);
}

/*!
    \reimp
*/

void QAbstractSpinBox::resizeEvent(QResizeEvent *e)
{
    QStyleOptionSpinBox opt = d->getStyleOption();
    opt.subControls = QStyle::SC_SpinBoxEditField;
    d->edit->setGeometry(QStyle::visualRect(opt.direction, opt.rect,
                                            style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                                    QStyle::SC_SpinBoxEditField,
                                                                    this)));
    QWidget::resizeEvent(e);
}

/*!
    \reimp
*/

QSize QAbstractSpinBox::sizeHint() const
{
    return d->sizeHint();
}

/*!
    \reimp
*/

QSize QAbstractSpinBox::minimumSizeHint() const
{
    return d->minimumSizeHint();
}

/*!
    \reimp
*/

void QAbstractSpinBox::paintEvent(QPaintEvent *)
{
    QStyleOptionSpinBox opt = d->getStyleOption();
    QPainter p(this);
    style()->drawComplexControl(QStyle::CC_SpinBox, &opt, &p, this);
}

/*!
    \reimp

    This function handles keyboard input.

    The following keys are handled specifically:
    \table
    \row \i Enter/Return
         \i This will reinterpret the text and emit a signal even if the value has not changed
         since last time a signal was emitted.
    \row \i Up
         \i This will invoke stepBy(1)
    \row \i Down
         \i This will invoke stepBy(-1)
    \row \i Page up
         \i This will invoke stepBy(10)
    \row \i Page down
         \i This will invoke stepBy(-10)
    \endtable
*/

void QAbstractSpinBox::keyPressEvent(QKeyEvent *e)
{
    if (!e->text().isEmpty() && d->edit->cursorPosition() < d->prefix.size())
        d->edit->setCursorPosition(d->prefix.size());

    int steps = 1;
    switch(e->key()) {
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        steps *= 10;
    case Qt::Key_Up:
    case Qt::Key_Down: {
        const bool up = (e->key() == Qt::Key_PageUp || e->key() == Qt::Key_Up);
        if (!(stepEnabled() & (up ? StepUpEnabled : StepDownEnabled)))
            return;
        if (!up)
            steps *= -1;
        if (style()->styleHint(QStyle::SH_SpinBox_AnimateButton, 0, this)) {
            d->buttonstate = (Keyboard | (up ? Up : Down));
        }
        stepBy(steps);
        return;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return:
        d->refresh(AlwaysEmit);
        selectAll();
        e->accept();
        return;

    case Qt::Key_U:
        if (e->modifiers() & Qt::ControlModifier) {
            clear();
            e->accept();
            return;
        }

    case Qt::Key_End:
    case Qt::Key_Home:
        if (e->modifiers() & Qt::ShiftModifier) {
            int currentPos = d->edit->cursorPosition();
            const QString text = d->edit->displayText();
            if (e->key() == Qt::Key_End) {
                if ((currentPos == 0 && !d->prefix.isEmpty()) || text.size() - d->suffix.size() <= currentPos) {
                    break; // let lineedit handle this
                } else {
                    d->edit->setSelection(currentPos, text.size() - d->suffix.size() - currentPos);
                }
            } else {
                if ((currentPos == text.size() && !d->suffix.isEmpty()) || currentPos <= d->prefix.size()) {
                    break; // let lineedit handle this
                } else {
                    d->edit->setSelection(currentPos, d->prefix.size() - currentPos);
                }
            }
            e->accept();
            return;
        }

    case Qt::Key_Z:
    case Qt::Key_Y:
        if (e->modifiers() & Qt::ControlModifier) {
            e->ignore();
            return;
        }
        break;
    default:
        break;
    }

    d->edit->event(e);
}

/*!
    \reimp
*/

void QAbstractSpinBox::keyReleaseEvent(QKeyEvent *e)
{
    if (style()->styleHint(QStyle::SH_SpinBox_AnimateButton, 0, this)
        && d->buttonstate & Keyboard && !e->isAutoRepeat()) {
        d->resetState();
    } else {
        d->edit->event(e);
    }
}

/*!
    \reimp
*/

void QAbstractSpinBox::wheelEvent(QWheelEvent *e)
{
    const int steps = (e->delta() > 0 ? 1 : -1);
    if (stepEnabled() & (steps > 0 ? StepUpEnabled : StepDownEnabled))
        stepBy(e->modifiers() & Qt::ControlModifier ? steps * 10 : steps);
    e->accept();
}


/*!
    \reimp
*/
void QAbstractSpinBox::focusInEvent(QFocusEvent *e)
{
    d->edit->event(e);
    if (e->reason() == Qt::TabFocusReason || e->reason() == Qt::BacktabFocusReason) {
        selectAll();
    }
    QWidget::focusInEvent(e);
}

/*!
    \reimp
*/

void QAbstractSpinBox::focusOutEvent(QFocusEvent *e)
{
    if (d->pendingemit)
        d->refresh(EmitIfChanged);
    d->resetState();
    d->edit->event(e);
    QWidget::focusOutEvent(e);
}

/*!
    \reimp
*/

void QAbstractSpinBox::closeEvent(QCloseEvent *e)
{
    d->resetState();
    if (d->pendingemit)
        d->refresh(EmitIfChanged);
    QWidget::closeEvent(e);
}

/*!
    \reimp
*/

void QAbstractSpinBox::hideEvent(QHideEvent *e)
{
    d->resetState();
    QWidget::hideEvent(e);
}

/*!
    \reimp
*/

void QAbstractSpinBox::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->spinclicktimerid) {
        if (d->buttonstate & Up) {
            stepBy(1);
            if (!(stepEnabled() & StepUpEnabled))
                d->resetState();
        } else if (d->buttonstate & Down) {
            stepBy(-1);
            if (!(stepEnabled() & StepDownEnabled))
                d->resetState();
        }
    }
}

/*!
    \reimp
*/

void QAbstractSpinBox::contextMenuEvent(QContextMenuEvent *e)
{
#ifndef QT_NO_POPUPMENU
    d->resetState();
    QMenu *menu = d->edit->createStandardContextMenu();
    menu->addSeparator();
    const uint se = stepEnabled();
    QAction *up = menu->addAction(tr("&Step up"));
    up->setEnabled(se & StepUpEnabled);
    QAction *down = menu->addAction(tr("Step &down"));
    down->setEnabled(se & StepDownEnabled);

    const QPointer<QAbstractSpinBox> that = this;
    const QPoint pos = (e->reason() == QContextMenuEvent::Mouse)
        ? e->globalPos() : mapToGlobal(QPoint(e->pos().x(), 0)) + QPoint(width() / 2, height() / 2);
    const QAction *action = menu->exec(pos);
    delete menu;
    if (that) {
        if (action == up) {
            stepBy(1);
        } else if (action == down) {
            stepBy(-1);
        }
    }
#endif
    e->accept();
}

/*!
    \reimp
*/

void QAbstractSpinBox::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton) {
        d->dragging = true;
    }

    d->updateHoverControl(e->pos());

    // If we have a timer ID, update the state
    const StepEnabled se = stepEnabled();
    if (d->spinclicktimerid != -1) {
        if ((se & StepUpEnabled) && d->hoverControl == QStyle::SC_SpinBoxUp)
            d->updateState(true);
        else if ((se & StepDownEnabled) && d->hoverControl == QStyle::SC_SpinBoxDown)
            d->updateState(false);
        else
            d->resetState();
        e->accept();
    }
}

/*!
    \reimp
*/

void QAbstractSpinBox::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || d->buttonstate != None)
        return;

    d->updateHoverControl(e->pos());

    const StepEnabled se = stepEnabled();
    if ((se & StepUpEnabled) && d->hoverControl == QStyle::SC_SpinBoxUp) {
        d->updateState(true);
        e->accept();
    } else if ((se & StepDownEnabled) && d->hoverControl == QStyle::SC_SpinBoxDown) {
        d->updateState(false);
        e->accept();
    }
}

/*!
    \reimp
*/
void QAbstractSpinBox::mouseReleaseEvent(QMouseEvent *)
{
    d->dragging = false;
    if ((d->buttonstate & Mouse) != 0)
        d->resetState();
}

// --- QAbstractSpinBoxPrivate ---

/*!
    \internal
    Constructs a QAbstractSpinBoxPrivate object
*/

QAbstractSpinBoxPrivate::QAbstractSpinBoxPrivate()
    : edit(0), type(QVariant::Invalid), spinclicktimerid(-1),
      spinclicktimerinterval(100), buttonstate(None), sizehintdirty(true),
      dirty(true), cachedtext("\x01"), cachedstate(QValidator::Invalid),
      pendingemit(false), readonly(false), tracking(false), wrapping(false),
      dragging(false), ignorecursorpositionchanged(false),
      buttonsymbols(QAbstractSpinBox::UpDownArrows)
{
}

/*!
    \internal
    Updates the old and new hover control. Does nothing if the hover
    control has not changed.
*/
bool QAbstractSpinBoxPrivate::updateHoverControl(const QPoint &pos)
{
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
    Returns the hover control at \a pos.
    This will update the hoverRect and hoverControl.
*/
QStyle::SubControl QAbstractSpinBoxPrivate::newHoverControl(const QPoint &pos)
{
    if (hoverRect.contains(pos))
        return hoverControl;

    QStyleOptionSpinBox opt = d->getStyleOption();
    opt.subControls = QStyle::SC_All;
    QRect upButtonRect = q->style()->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxUp, q);
    QRect downButtonRect = q->style()->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxDown, q);
    QRect lineEditRect = q->style()->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxEditField, q);

    if (upButtonRect.contains(pos)) {
        hoverRect = upButtonRect;
        hoverControl = QStyle::SC_SpinBoxUp;
    } else if (downButtonRect.contains(pos)) {
        hoverRect = downButtonRect;
        hoverControl = QStyle::SC_SpinBoxDown;
    } else if (lineEditRect.contains(pos)) {
        hoverRect = lineEditRect;
        hoverControl = QStyle::SC_SpinBoxEditField;
    } else {
        hoverRect = QRect();
        hoverControl = QStyle::SC_None;
    }

    return hoverControl;
}

/*!
    \internal
    Strips any prefix/suffix from \a text.
*/

void QAbstractSpinBoxPrivate::strip(QString *text) const
{
    Q_ASSERT(text);
    if (specialvaluetext.size() && *text == specialvaluetext)
        return;
    int from = 0;
    int length = text->length();
    bool changed = false;
    if (prefix.size() && text->startsWith(prefix)) {
        from += prefix.length();
        length -= from;
	changed = true;
    }
    if (suffix.size() && text->endsWith(suffix)) {
        length -= suffix.length();
	changed = true;
    }
    if (changed)
    *text = text->mid(from, length);
}

/*!
    \internal
    Returns true if a specialValueText has been set and the current value is minimum.
*/

bool QAbstractSpinBoxPrivate::specialValue() const
{
    return (value == minimum && specialvaluetext.size() > 0);
}

/*!
    \internal
    Virtual function that emits signals. Reimplemented in the different subclasses.
*/

void QAbstractSpinBoxPrivate::emitSignals(EmitPolicy, const QVariant &)
{
}

/*!
    \internal

    Slot connected to the line edit's textChanged(const QString &)
    signal. Will interpret the text if tracking is true. Otherwise it
    will set the pendingemit flag to true.
*/

void QAbstractSpinBoxPrivate::editorTextChanged(const QString &t)
{
    if (tracking) {
        QString tmp = t;
        int pos = edit->cursorPosition();
        QValidator::State state = q->validate(tmp, pos);
        if (state == QValidator::Acceptable) {
            const QVariant v = d->valueFromText(tmp);
            if (tmp != t) {
                const bool wasBlocked = edit->blockSignals(true);
                edit->setText(prefix + tmp + suffix);
                edit->blockSignals(wasBlocked);
            }
	    setValue(v, EmitIfChanged, false);
            pendingemit = false;
        } else {
            pendingemit = true;
        }
    } else {
        pendingemit = true;
    }
}

/*!
    \internal

    Virtual slot connected to the line edit's
    cursorPositionChanged(int, int) signal. Will move the cursor to a
    valid position if the new one is invalid. E.g. inside the prefix.
    Reimplemented in Q[Date|Time|DateTime]EditPrivate to account for
    the different sections etc.
*/

void QAbstractSpinBoxPrivate::editorCursorPositionChanged(int oldpos, int newpos)
{
    if (!ignorecursorpositionchanged && !specialValue() && !dragging) {
        ignorecursorpositionchanged = true;

        bool allowSelection = true;
        int pos = -1;
        if (newpos < prefix.size() && newpos != 0) {
            if (oldpos == 0) {
                allowSelection = false;
                pos = prefix.size();
            } else {
                pos = oldpos;
            }
        } else if (newpos > edit->text().size() - suffix.size()
                   && newpos != edit->text().size()) {
            if (oldpos == edit->text().size()) {
                pos = edit->text().size() - suffix.size();
                allowSelection = false;
            } else {
                pos = edit->text().size();
            }
        }
        if (pos != -1) {
            const int selLength = edit->selectionStart() >= 0 && allowSelection
                                  ? (edit->selectedText().length()
                                     * (newpos < pos ? -1 : 1)) - newpos + pos
                                  : 0;

            const bool wasBlocked = edit->blockSignals(true);
            if (selLength != 0) {
                edit->setSelection(pos - selLength, selLength);
            } else {
                edit->setCursorPosition(pos);
            }
            edit->blockSignals(wasBlocked);
        }
        ignorecursorpositionchanged = false;
    }
}

/*!
    \internal

    Initialises the QAbstractSpinBoxPrivate object.
*/

void QAbstractSpinBoxPrivate::init()
{
    spinclicktimerinterval = q->style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatRate, 0, q);
    q->setFocusPolicy(Qt::WheelFocus);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    q->setAttribute(Qt::WA_InputMethodEnabled);

    q->setLineEdit(new QLineEdit(q));
    if (d->type != QVariant::Invalid)
        edit->setValidator(new QSpinBoxValidator(q, this));

    edit->setObjectName("qt_spinbox_lineedit");

}

/*!
    \internal

    Calls QWidget::update() on the area where the arrows are painted.
*/

void QAbstractSpinBoxPrivate::updateSpinBox()
{
    if (q) {
        QStyleOptionSpinBox opt = getStyleOption();
        q->update(QStyle::visualRect(opt.direction, opt.rect,
                                     q->style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                                QStyle::SC_SpinBoxUp, q)));
        q->update(QStyle::visualRect(opt.direction, opt.rect,
                                     q->style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                                QStyle::SC_SpinBoxDown, q)));
    }
}

/*!
    \internal

    Resets the state of the spinbox. E.g. the state is set to
    (Keyboard|Up) if Key up is currently pressed.
*/

void QAbstractSpinBoxPrivate::resetState()
{
    buttonstate = None;
    if (q) {
        if (spinclicktimerid != -1)
            q->killTimer(spinclicktimerid);
        spinclicktimerid = -1;
        updateSpinBox();
    }
}

/*!
    \internal

    Updates the state of the spinbox.
*/

void QAbstractSpinBoxPrivate::updateState(bool up)
{
    if ((up && (buttonstate & Up)) || (!up && (buttonstate & Down)))
        return;
    resetState();
    if (q && (q->stepEnabled() & (up ? QAbstractSpinBox::StepUpEnabled
                                  : QAbstractSpinBox::StepDownEnabled))) {
        spinclicktimerid = q->startTimer(spinclicktimerinterval);
        buttonstate = (up ? (Mouse | Up) : (Mouse | Down));
        q->stepBy(up ? 1 : -1);
    }
}

/*!
    \internal

    If sizehintdirty is true and edit != 0
    recalculates the sizehint. Called from QAbstractSpinBox::sizeHint().
*/

void QAbstractSpinBoxPrivate::calculateSizeHints() const
{
    if (sizehintdirty && edit) {
        const QFontMetrics fm(q->fontMetrics());
        int h = edit->sizeHint().height();
        int w = 0;
        QString s;
        s = prefix + textFromValue(minimum) + suffix + QLatin1Char(' ');
        s.truncate(18);
        w = qMax<int>(w, fm.width(s));
        s = prefix + textFromValue(maximum) + suffix + QLatin1Char(' ');
        s.truncate(18);
        w = qMax<int>(w, fm.width(s));
        if (specialvaluetext.size()) {
            s = specialvaluetext;
            w = qMax<int>(w, fm.width(s));
        }
        w += 2; // cursor blinking space

        QStyleOptionSpinBox opt = getStyleOption();
        QSize hint(w, h);
        QSize extra(35,6);
        opt.rect.setSize(hint + extra);
        extra += hint - q->style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                   QStyle::SC_SpinBoxEditField, q).size();
        // get closer to final result by repeating the calculation
        opt.rect.setSize(hint + extra);
        extra += hint - q->style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                   QStyle::SC_SpinBoxEditField, q).size();
        hint += extra;

        cachedsizehint = hint.expandedTo(QApplication::globalStrut());
        cachedminimumsizehint = hint.expandedTo(QApplication::globalStrut());
        const_cast<QAbstractSpinBoxPrivate *>(this)->sizehintdirty = false;
    }
}

/*!
    \internal

    Creates a QStyleOptionSpinBox with the right flags set.
*/

QStyleOptionSpinBox QAbstractSpinBoxPrivate::getStyleOption() const
{
    QStyleOptionSpinBox opt;
    opt.init(q);
    opt.activeSubControls = 0;
    opt.buttonSymbols = buttonsymbols;
    opt.subControls = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown | QStyle::SC_SpinBoxFrame;

    if (buttonstate & Up)
        opt.activeSubControls = QStyle::SC_SpinBoxUp;
    else if (buttonstate & Down)
        opt.activeSubControls = QStyle::SC_SpinBoxDown;
    else
        opt.activeSubControls = hoverControl;
    if (buttonstate)
        opt.state |= QStyle::State_Down;

    if (type != QVariant::Invalid) {
        opt.percentage = (value - minimum) / (maximum - minimum);
        opt.stepEnabled = q->stepEnabled();
    } else {
        opt.stepEnabled = QAbstractSpinBox::StepNone;
        opt.percentage = 0.0;
    }
    return opt;
}

/*!
    \internal

    Returns the cached sizehint. If sizehintdirty is true it
    recalculates the sizehint. Called from QAbstractSpinBox::sizeHint().
*/

QSize QAbstractSpinBoxPrivate::sizeHint() const
{
    q->ensurePolished();
    calculateSizeHints();
    return cachedsizehint;
}

/*!
    \internal

    Returns the minimumSizeHint. If sizehintdirty is true it
    recalculates the sizehint. Called from QAbstractSpinBox::minimumSizeHint().
*/

QSize QAbstractSpinBoxPrivate::minimumSizeHint() const
{
    q->ensurePolished();
    calculateSizeHints();
    return cachedminimumsizehint;
}

/*!
    \internal

    Bounds \a val to be within minimum and maximum. Also tries to be
    clever about setting it at min and max depending on what it was
    and what direction it was changed etc.
*/

QVariant QAbstractSpinBoxPrivate::bound(const QVariant &val, const QVariant &old, int steps) const
{
    QVariant v = val;
    if (!wrapping || steps == 0 || old.isNull()) {
        if (v < minimum) {
            v = wrapping ? maximum : minimum;
        }
        if (v > maximum) {
            v = wrapping ? minimum : maximum;
        }
    } else {
        const bool wasMin = old == minimum;
        const bool wasMax = old == maximum;
        const bool wrapped = (v > old && steps < 0) || (v < old && steps > 0);
        if (v > maximum) {
            v = ((wasMax && !wrapped && steps > 0) || (steps < 0 && !wasMin && wrapped))
                ? minimum : maximum;
        } else if (wrapped && (v > maximum || v < minimum)) {
            v = (wasMax && steps > 0 || (!wasMin && steps < 0)) ? minimum : maximum;
        } else if (v < minimum) {
            v = (!wasMax && !wasMin ? minimum : maximum);
        }
    }

    return v;
}

/*!
    \internal

    Sets the value of the spin box to \a val. Depending on the value
    of \a ep it will also emit signals.
*/

void QAbstractSpinBoxPrivate::setValue(const QVariant &val, EmitPolicy ep,
                                       bool doUpdate)
{
    const QVariant old = value;
    value = bound(val);
    pendingemit = false;
    const bool changed = old != value;
    if (doUpdate)
        update();
    if (ep == AlwaysEmit || (ep == EmitIfChanged && changed)) {
        emitSignals(ep, old);
    }
}

/*!
    \internal

    Updates the line edit to reflect the current value of the spin box.
*/

void QAbstractSpinBoxPrivate::updateEdit() const
{
    QLineEdit *e = const_cast<QLineEdit*>(edit);
    const bool empty = e->text().isEmpty();
    int cursor = e->cursorPosition();
    int sellength = e->selectedText().length();
    const QString newText = specialValue() ? specialvaluetext : prefix + textFromValue(value) + suffix;
    const bool sb = e->blockSignals(true);
    e->setText(newText);

    if (!specialValue()) {
        cursor = qMin<int>(qMax<int>(cursor, prefix.length()), edit->displayText().length() - suffix.length());
        if (sellength > 0) {
            e->setSelection(cursor, sellength);
        } else {
            e->setCursorPosition(empty ? prefix.length() : cursor);
        }
    }
    e->blockSignals(sb);

    const_cast<QAbstractSpinBoxPrivate *>(this)->dirty = false;
}

/*!
    \internal

    Calls updateEdit() and updateSpinBox() if the widget is visible. Else sets the dirty flag.
*/

void QAbstractSpinBoxPrivate::update()
{
    if (d->type != QVariant::Invalid) {
        if (!q->isVisible()) {
            dirty = true;
        } else {
            updateEdit();
            updateSpinBox();
        }
    }
}

/*!
    \internal

    Convenience function to set min/max values.
*/

void QAbstractSpinBoxPrivate::setBoundary(Boundary b, const QVariant &val)
{
    if (b == Minimum) {
        minimum = val;
        if (maximum < minimum)
            maximum = minimum;
    } else {
        maximum = val;
        if (minimum > maximum)
            minimum = maximum;
    }
    resetState();
    setValue(bound(value), EmitIfChanged);
}

/*!
    \internal

    Convenience function to get a variant of the right type.
*/

QVariant QAbstractSpinBoxPrivate::getZeroVariant() const
{
    QVariant ret;
    switch (type) {
    case QVariant::Int: ret = QVariant((int)0); break;
    case QVariant::Double: ret = QVariant((double)0); break;
    case QVariant::Time: ret = QVariant(QTime()); break;
    case QVariant::Date: ret = QVariant(DATE_INITIAL); break;
    case QVariant::DateTime: ret = QVariant(QDateTime(DATE_INITIAL, QTime())); break;
    default: break;
    }
    return ret;
}

/*!
    \internal

    Virtual method called that calls the public textFromValue()
    functions in the subclasses. Needed to change signature from
    QVariant to int/double/QDateTime etc. Used when needing to display
    a value textually.

    This method is reimeplemented in the various subclasses.
*/

QString QAbstractSpinBoxPrivate::textFromValue(const QVariant &) const
{
    return QString();
}

/*!
    \internal

    Virtual method called that calls the public valueFromText()
    functions in the subclasses. Needed to change signature from
    QVariant to int/double/QDateTime etc. Used when needing to
    interpret a string as another type.

    This method is reimeplemented in the various subclasses.
*/

QVariant QAbstractSpinBoxPrivate::valueFromText(const QString &) const
{
    return QVariant();
}
/*!
    \internal

    Interprets text and emits signals. Called from interpretText() and
    when the user presses Enter.
*/

void QAbstractSpinBoxPrivate::refresh(EmitPolicy ep)
{
    if (d->type == QVariant::Invalid)
        return;

    QVariant v = getZeroVariant();
    bool doInterpret = true;
    QString tmp = d->edit->displayText();
    int pos = d->edit->cursorPosition();
    const int oldpos = pos;

    if (q->validate(tmp, pos) != QValidator::Acceptable) {
        const QString copy = tmp;
	q->fixup(tmp);
        QASBDEBUG() << "QAbstractSpinBoxPrivate::refresh() text '"
                    << d->edit->displayText()
                    << "' >> '" << copy << "'"
                    << "' >> '" << tmp << "'";

        doInterpret = tmp != copy && (q->validate(tmp, pos) == QValidator::Acceptable);
        if (!doInterpret) {
            v = value;
        }
    }
    if (doInterpret) {
        v = valueFromText(tmp);
    }

    setValue(v, ep);
    if (oldpos != pos)
        d->edit->setCursorPosition(pos);
}

// --- QSpinBoxValidator ---

/*!
    \internal
    Constructs a QSpinBoxValidator object
*/

QSpinBoxValidator::QSpinBoxValidator(QAbstractSpinBox *qp, QAbstractSpinBoxPrivate *dp)
    : QValidator(qp), qptr(qp), dptr(dp)
{
    setObjectName("qt_spinboxvalidator");
}

/*!
    \internal

    Checks for specialValueText, prefix, suffix and calls
    the virtual QAbstractSpinBox::validate function.
*/

QValidator::State QSpinBoxValidator::validate(QString &input, int &pos) const
{
    if (dptr->specialvaluetext.size() > 0 && input == dptr->specialvaluetext) {
        return QValidator::Acceptable;
    } else if ((!dptr->prefix.isEmpty() && !input.startsWith(dptr->prefix))
	       || (!dptr->suffix.isEmpty() && !input.endsWith(dptr->suffix))) {
        return QValidator::Invalid;
    } else {
	return qptr->validate(input, pos);
    }
}
/*!
    \internal
    Calls the virtual QAbstractSpinBox::fixup function.
*/

void QSpinBoxValidator::fixup(QString &input) const
{
    qptr->fixup(input);
}
// --- global ---


/*!
    \internal
    Compares two variants and returns true if \a arg1 < \a arg2
*/

bool operator<(const QVariant &arg1, const QVariant &arg2)
{
    if (arg1.type() != arg2.type())
        qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
                 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QVariant::Int: return arg1.toInt() < arg2.toInt();
    case QVariant::Double: return arg1.toDouble() < arg2.toDouble();
    case QVariant::Date: return arg1.toDate() < arg2.toDate();
    case QVariant::Time: return arg1.toTime() < arg2.toTime();
    case QVariant::DateTime: return arg1.toDateTime() < arg2.toDateTime();
    default: break;
    }
    return false;
}

/*!
    \internal
    Compares two variants and returns true if \a arg1 > \a arg2
*/

bool operator>(const QVariant &arg1, const QVariant &arg2)
{
    if (arg1.type() != arg2.type())
        qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
                 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QVariant::Int: return arg1.toInt() > arg2.toInt();
    case QVariant::Double: return arg1.toDouble() > arg2.toDouble();
    case QVariant::Time: return arg1.toTime() > arg2.toTime();
    case QVariant::Date: return arg1.toDate() > arg2.toDate();
    case QVariant::DateTime: return arg1.toDateTime() > arg2.toDateTime();
    default: break;
    }
    return false;
}

/*!
    \internal
    Compares two variants and returns true if \a arg1 >= \a arg2
*/

bool operator<=(const QVariant &arg1, const QVariant &arg2)
{
    if (arg1.type() != arg2.type())
        qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
                 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QVariant::Int:
    case QVariant::Double:
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime: return (arg1 < arg2 || arg1 == arg2);
    default: break;
    }
    return false;
}

/*!
    \internal
    Compares two variants and returns true if \a arg1 >= \a arg2
*/

bool operator>=(const QVariant &arg1, const QVariant &arg2)
{
    if (arg1.type() != arg2.type())
        qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
                 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QVariant::Int:
    case QVariant::Double:
    case QVariant::Time:
    case QVariant::Date:
    case QVariant::DateTime: return (arg1 > arg2 || arg1 == arg2);
    default: break;
    }
    return false;
}

/*!
    \internal
    Adds two variants together and returns the result.
*/

QVariant operator+(const QVariant &arg1, const QVariant &arg2)
{
    QVariant ret;
    if (arg1.type() != arg2.type())
        qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
                 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QVariant::Int: ret = QVariant(arg1.toInt() + arg2.toInt()); break;
    case QVariant::Double: ret = QVariant(arg1.toDouble() + arg2.toDouble()); break;
    case QVariant::DateTime: {
        QDateTime a2 = arg2.toDateTime();
        QDateTime a1 = arg1.toDateTime().addDays(DATETIME_MIN.daysTo(a2));
        a1.setTime(a1.time().addMSecs(QTime().msecsTo(a2.time())));
        ret = QVariant(a1);
    }
    default: break;
    }
    return ret;
}


/*!
    \internal
    Subtracts two variants and returns the result.
*/

QVariant operator-(const QVariant &arg1, const QVariant &arg2)
{
    QVariant ret;
    if (arg1.type() != arg2.type())
        qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
                 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QVariant::Int: ret = QVariant(arg1.toInt() - arg2.toInt()); break;
    case QVariant::Double: ret = QVariant(arg1.toDouble() - arg2.toDouble()); break;
    case QVariant::DateTime: {
        QDateTime a1 = arg1.toDateTime();
        QDateTime a2 = arg2.toDateTime();
        int days = a2.daysTo(a1);
        int secs = a2.secsTo(a1);
        int msecs = qMax<int>(0, a1.time().msec() - a2.time().msec());
        if (days < 0 || secs < 0 || msecs < 0) {
            ret = arg1;
        } else {
            QDateTime dt = a2.addDays(days).addSecs(secs);
            if (msecs > 0)
                dt.setTime(dt.time().addMSecs(msecs));
            ret = QVariant(dt);
        }
    }
    default: break;
    }
    return ret;
}

/*!
    \internal
    Multiplies \a arg1 by \a multiplier and returns the result.
*/

QVariant operator*(const QVariant &arg1, double multiplier)
{
    QVariant ret;

    switch (arg1.type()) {
    case QVariant::Int: ret = QVariant((int)(arg1.toInt() * multiplier)); break;
    case QVariant::Double: ret = QVariant(arg1.toDouble() * multiplier); break;
    case QVariant::DateTime: {
        double days = DATE_MIN.daysTo(arg1.toDateTime().date()) * multiplier;
        int daysInt = (int)days;
        days -= daysInt;
        long msecs = (long)((TIME_MIN.msecsTo(arg1.toDateTime().time()) * multiplier) + (days * (24 * 3600 * 1000)));
        ret = QDateTime(QDate().addDays(int(days)), QTime().addMSecs(msecs));
        break;
    }
    default: ret = arg1; break;
    }

    return ret;
}



double operator/(const QVariant &arg1, const QVariant &arg2)
{
    double a1 = 0;
    double a2 = 0;

    switch (arg1.type()) {
    case QVariant::Int:
        a1 = (double)arg1.toInt();
        a2 = (double)arg2.toInt();
        break;
    case QVariant::Double:
        a1 = arg1.toDouble();
        a2 = arg2.toDouble();
        break;
    case QVariant::DateTime:
        a1 = DATE_MIN.daysTo(arg1.toDate());
        a2 = DATE_MIN.daysTo(arg2.toDate());
        a1 += (double)TIME_MIN.msecsTo(arg1.toDateTime().time()) / (long)(3600 * 24 * 1000);
        a2 += (double)TIME_MIN.msecsTo(arg2.toDateTime().time()) / (long)(3600 * 24 * 1000);
    default: break;
    }

    return (a1 != 0 && a2 != 0) ? (a1 / a2) : 0.0;
}

#include "moc_qabstractspinbox.cpp"

