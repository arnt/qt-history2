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

#ifndef QT_NO_SPINBOX

#include <qapplication.h>
#include <qclipboard.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qdebug.h>
#include <qpalette.h>
#include <qstyle.h>

#if defined(Q_WS_X11)
#include <limits.h>
#endif
static const int thresholdTime = 500; // ### make this a stylehint in 4.1

//#define QABSTRACTSPINBOX_QSBDEBUG
#ifdef QABSTRACTSPINBOX_QSBDEBUG
#  define QASBDEBUG qDebug
#else
#  define QASBDEBUG if (false) qDebug
#endif

/*!
    \class QAbstractSpinBox
    \brief The QAbstractSpinBox class provides a spinbox and a line edit to
    display values.

    \ingroup abstractwidgets

    The class is designed as a common super class for widgets like
    QSpinBox, QDoubleSpinBox and QDateTimeEdit

    Here are the main properties of the class:

    \list 1

    \i \l text: The text that is displayed in the QAbstractSpinBox.

    \i \l alignment: The alignment of the text in the QAbstractSpinBox.

    \i \l wrapping: Whether the QAbstractSpinBox wraps from the
    minimum value to the maximum value and vica versa.

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
  \fn void QAbstractSpinBox::editingFinished()

  This signal is emitted editing is finished. This happens when the
  spinbox looses focus and when enter is pressed.
*/

/*!
    Constructs an abstract spinbox with the given \a parent with default
    \l wrapping, and \l alignment properties.
*/

QAbstractSpinBox::QAbstractSpinBox(QWidget *parent)
    : QWidget(*new QAbstractSpinBoxPrivate, parent, 0)
{
    Q_D(QAbstractSpinBox);
    d->init();
}

/*!
    \internal
*/
QAbstractSpinBox::QAbstractSpinBox(QAbstractSpinBoxPrivate &dd, QWidget *parent)
    : QWidget(dd, parent, 0)
{
    Q_D(QAbstractSpinBox);
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

    This enum type describes the symbols that can be displayed on the buttons
    in a spin box.

    \inlineimage qspinbox-updown.png
    \inlineimage qspinbox-plusminus.png

    \value UpDownArrows Little arrows in the classic style.
    \value PlusMinus \bold{+} and \bold{-} symbols.

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
    Q_D(const QAbstractSpinBox);
    return d->buttonSymbols;
}

void QAbstractSpinBox::setButtonSymbols(ButtonSymbols bs)
{
    Q_D(QAbstractSpinBox);
    if (d->buttonSymbols != (d->buttonSymbols = bs))
        update();
}

/*!
    \property QAbstractSpinBox::text

    \brief the spin box's text, including any prefix and suffix

    There is no default text.
*/

QString QAbstractSpinBox::text() const
{
    return lineEdit()->displayText();
}


/*!
    \property QAbstractSpinBox::specialValueText
    \brief the special-value text

    If set, the spin box will display this text instead of a numeric
    value whenever the current value is equal to minimum(). Typical use
    is to indicate that this choice has a special (default) meaning.

    For example, if your spin box allows the user to choose the margin
    width in a print dialog and your application is able to
    automatically choose a good margin width, you can set up the spin
    box like this:

    \code
        QSpinBox marginBox(-1, 20, 1, parent);
        marginBox.setSuffix(" mm");
        marginBox.setSpecialValueText("Auto");
    \endcode

    The user will then be able to choose a margin width from 0-20
    millimeters or select "Auto" to leave it to the application to
    choose. Your code must then interpret the spin box value of -1 as
    the user requesting automatic margin width.

    All values are displayed with the prefix and suffix (if set), \e
    except for the special value, which only shows the special value
    text.

    To turn off the special-value text display, call this function
    with an empty string. The default is no special-value text, i.e.
    the numeric value is shown as usual.

    If no special-value text is set, specialValueText() returns an
    empty string.
*/

QString QAbstractSpinBox::specialValueText() const
{
    Q_D(const QAbstractSpinBox);

    return d->specialValueText;
}

void QAbstractSpinBox::setSpecialValueText(const QString &s)
{
    Q_D(QAbstractSpinBox);

    d->specialValueText = s;
    d->cachedText.clear();
    d->cachedValue.clear();
    d->update();
}

/*!
    \property QAbstractSpinBox::wrapping

    \brief whether the spin box is circular.

    If wrapping is true stepping up from maximum() value will take you
    to the minimum() value and vica versa. Wrapping only make sense if
    you have minimum() and maximum() values set.

    \code
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
    Q_D(const QAbstractSpinBox);
    return d->wrapping;
}

void QAbstractSpinBox::setWrapping(bool w)
{
    Q_D(QAbstractSpinBox);
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

    \sa QLineEdit::readOnly
*/

bool QAbstractSpinBox::isReadOnly() const
{
    Q_D(const QAbstractSpinBox);
    return d->readOnly;
}

void QAbstractSpinBox::setReadOnly(bool enable)
{
    Q_D(QAbstractSpinBox);
    d->readOnly = enable;
    d->edit->setReadOnly(enable);
    update();
}

/*!
    \property QAbstractSpinBox::frame
    \brief whether the spin box draws itself with a frame

    If enabled (the default) the spin box draws itself inside a frame,
    otherwise the spin box draws itself without any frame.
*/

bool QAbstractSpinBox::hasFrame() const
{
    Q_D(const QAbstractSpinBox);
    return d->frame;
}


void QAbstractSpinBox::setFrame(bool enable)
{
    Q_D(QAbstractSpinBox);
    d->frame = enable;
    update();
    updateGeometry();
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
    Q_D(const QAbstractSpinBox);
    if (d->dirty)
        d->updateEdit();

    return (Qt::Alignment)d->edit->alignment();
}

void QAbstractSpinBox::setAlignment(Qt::Alignment flag)
{
    Q_D(QAbstractSpinBox);
    if (d->dirty)
        d->updateEdit();

    d->edit->setAlignment(flag);
}

/*!
    Selects all the text in the spinbox except the prefix and suffix.
*/

void QAbstractSpinBox::selectAll()
{
    Q_D(QAbstractSpinBox);

    if (d->dirty)
        d->updateEdit();

    if (!d->specialValue()) {
        const int tmp = d->edit->displayText().size() - d->suffix.size();
        d->edit->setSelection(tmp, -(tmp - d->prefix.size()));
    } else {
        d->edit->selectAll();
    }
}

/*!
    Clears the lineedit of all text but prefix and suffix.
*/

void QAbstractSpinBox::clear()
{
    Q_D(QAbstractSpinBox);

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
    Q_D(const QAbstractSpinBox);
    if (d->readOnly)
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
   determine whether \a input is valid. The \a pos parameter indicates
   the position in the string. Reimplemented in the various
   subclasses.
*/

QValidator::State QAbstractSpinBox::validate(QString & /* input */, int & /* pos */) const
{
    return QValidator::Acceptable;
}

/*!
   This virtual function is called by the QAbstractSpinBox if the
   \a input is not validated to QValidator::Acceptable when Return is
   pressed or interpretText() is called. It will try to change the
   text so it is valid. Reimplemented in the various subclasses.
*/

void QAbstractSpinBox::fixup(QString & /* input */) const
{
}

/*!
  Steps up by one linestep
  Calling this slot is analogous to calling stepBy(1);
  \sa stepBy(), stepDown()
*/

void QAbstractSpinBox::stepUp()
{
    stepBy(1);
}

/*!
  Steps down by one linestep
  Calling this slot is analogous to calling stepBy(-1);
  \sa stepBy(), stepUp()
*/

void QAbstractSpinBox::stepDown()
{
    stepBy(-1);
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
*/

void QAbstractSpinBox::stepBy(int steps)
{
    Q_D(QAbstractSpinBox);

    const QVariant old = d->value;
    QString tmp = d->edit->displayText();
    int cursorPos = d->edit->cursorPosition();
    bool dontstep = false;
    EmitPolicy e = EmitIfChanged;
    if (d->pendingEmit) {
        dontstep = validate(tmp, cursorPos) != QValidator::Acceptable;
        d->interpret(NeverEmit);
        if (d->value != old)
            e = AlwaysEmit;
    }
    if (!dontstep) {
        d->setValue(d->bound(d->value + (d->singleStep * steps), old, steps), e);
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
    Q_D(const QAbstractSpinBox);
    if (d->dirty)
        d->updateEdit();

    return d->edit;
}


/*!
    \fn void QAbstractSpinBox::setLineEdit(QLineEdit *lineEdit)

    Sets the line edit of the spinbox to be \a lineEdit instead of the
    current line edit widget. \a lineEdit can not be 0.

    QAbstractSpinBox takes ownership of the new lineEdit

    If QLineEdit::validator() for the \a lineEdit returns 0, the internal
    validator of the spinbox will be set on the line edit.
*/

void QAbstractSpinBox::setLineEdit(QLineEdit *lineEdit)
{
    Q_D(QAbstractSpinBox);

    if (!lineEdit) {
        Q_ASSERT(lineEdit);
        return;
    }
    delete d->edit;
    d->edit = lineEdit;
    if (!d->edit->validator())
        d->edit->setValidator(d->validator);

    if (d->edit->parent() != this)
        d->edit->setParent(this);

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
    d->edit->setGeometry(style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                 QStyle::SC_SpinBoxEditField, this));
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
    Q_D(QAbstractSpinBox);
    d->interpret(EmitIfChanged);
}

/*!
    \reimp
*/

bool QAbstractSpinBox::event(QEvent *event)
{
    Q_D(QAbstractSpinBox);
    switch (event->type()) {
    case QEvent::ApplicationLayoutDirectionChange:
        update();
        break;
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
    Q_D(QAbstractSpinBox);

    if (d->dirty) {
        d->reset();
        d->updateEdit();
    }
    d->updateSpinBox();
}

/*!
    \reimp
*/

void QAbstractSpinBox::changeEvent(QEvent *e)
{
    Q_D(QAbstractSpinBox);

    switch(e->type()) {
        case QEvent::StyleChange:
            d->spinClickTimerInterval = style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatRate, 0, this);
            d->spinClickThresholdTimerInterval = thresholdTime;
            d->reset();
            break;
        case QEvent::EnabledChange:
            if (!isEnabled()) {
                d->reset();
            }
            break;
        case QEvent::ActivationChange:
            if (!isActiveWindow()){
                d->reset();
                if (d->pendingEmit) // pendingEmit can be true even if it hasn't changed.
                    d->interpret(EmitIfChanged); // E.g. 10 to 10.0
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
    Q_D(QAbstractSpinBox);

    QStyleOptionSpinBox opt = d->getStyleOption();
    opt.subControls = QStyle::SC_SpinBoxEditField;
    d->edit->setGeometry(style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                 QStyle::SC_SpinBoxEditField, this));
    QWidget::resizeEvent(e);
}

/*!
    \reimp
*/

QSize QAbstractSpinBox::sizeHint() const
{
    Q_D(const QAbstractSpinBox);
    ensurePolished();

    const QFontMetrics fm(fontMetrics());
    int h = d->edit->sizeHint().height();
    int w = 0;
    QString s;
    s = d->prefix + d->textFromValue(d->minimum) + d->suffix + QLatin1Char(' ');
    s.truncate(18);
    w = qMax<int>(w, fm.width(s));
    s = d->prefix + d->textFromValue(d->maximum) + d->suffix + QLatin1Char(' ');
    s.truncate(18);
    w = qMax<int>(w, fm.width(s));
    if (d->specialValueText.size()) {
        s = d->specialValueText;
        w = qMax<int>(w, fm.width(s));
    }
    w += 2; // cursor blinking space

    QStyleOptionSpinBox opt = d->getStyleOption();
    QSize hint(w, h);
    QSize extra(35, 6);
    opt.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                            QStyle::SC_SpinBoxEditField, this).size();
    // get closer to final result by repeating the calculation
    opt.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                               QStyle::SC_SpinBoxEditField, this).size();
    hint += extra;

    opt.rect = rect();

    return style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
        .expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
*/

QSize QAbstractSpinBox::minimumSizeHint() const
{
    Q_D(const QAbstractSpinBox);
    ensurePolished();

    const QFontMetrics fm(fontMetrics());
    int h = d->edit->minimumSizeHint().height();
    int w = fm.width(QLatin1String("1000"));
    w += 2; // cursor blinking space

    QStyleOptionSpinBox opt = d->getStyleOption();
    QSize hint(w, h);
    QSize extra(35, 6);
    opt.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                            QStyle::SC_SpinBoxEditField, this).size();
    // get closer to final result by repeating the calculation
    opt.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                               QStyle::SC_SpinBoxEditField, this).size();
    hint += extra;

    opt.rect = rect();

    return style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
        .expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
*/

void QAbstractSpinBox::paintEvent(QPaintEvent *)
{
    Q_D(QAbstractSpinBox);

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
    Q_D(QAbstractSpinBox);

    if (!e->text().isEmpty() && d->edit->cursorPosition() < d->prefix.size())
        d->edit->setCursorPosition(d->prefix.size());

    int steps = 1;
    switch(e->key()) {
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        steps *= 10;
    case Qt::Key_Up:
    case Qt::Key_Down: {
#ifdef QT_KEYPAD_NAVIGATION
        if (QApplication::keypadNavigationEnabled()) {
            // Reserve up/down for nav - use left/right for edit.
            if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
                e->ignore();
                return;
            }
        }
#endif
        e->accept();
        const bool up = (e->key() == Qt::Key_PageUp || e->key() == Qt::Key_Up);
        if (!(stepEnabled() & (up ? StepUpEnabled : StepDownEnabled)))
            return;
        if (!up)
            steps *= -1;
        if (style()->styleHint(QStyle::SH_SpinBox_AnimateButton, 0, this)) {
            d->buttonState = (Keyboard | (up ? Up : Down));
        }
        stepBy(steps);
        return;
    }
#ifdef QT_KEYPAD_NAVIGATION
    case Qt::Key_Left:
    case Qt::Key_Right:
        if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            const bool up = (e->key() == Qt::Key_Right);
            if (!(stepEnabled() & (up ? StepUpEnabled : StepDownEnabled)))
                return;
            if (!up)
                steps *= -1;
            if (style()->styleHint(QStyle::SH_SpinBox_AnimateButton, 0, this)) {
                d->buttonState = (Keyboard | (up ? Up : Down));
            }
            stepBy(steps);
            return;
        }
        break;
    case Qt::Key_Back:
        if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            e->ignore();
            return;
        }
        break;
#endif
    case Qt::Key_Enter:
    case Qt::Key_Return:
        d->interpret(AlwaysEmit);
        selectAll();
        e->ignore();
        emit editingFinished();
        return;

#ifdef QT_KEYPAD_NAVIGATION
    case Qt::Key_Select:
        if (QApplication::keypadNavigationEnabled()) {
            // Toggles between left/right moving cursor and inc/dec.
            setEditFocus(!hasEditFocus());
            if (!hasEditFocus())
                selectAll();
        }
        return;
#endif

#ifdef Q_WS_X11 // only X11
    case Qt::Key_U:
        if (e->modifiers() & Qt::ControlModifier) {
            e->accept();
            if (!isReadOnly())
                clear();
            return;
        }
        break;
#else // Mac and Windows
    case Qt::Key_A:
        if (e->modifiers() & Qt::ControlModifier) {
            selectAll();
            e->accept();
            return;
        }
        break;
#endif

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
        break;

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
    Q_D(QAbstractSpinBox);

    if (d->buttonState & Keyboard && !e->isAutoRepeat()
        && style()->styleHint(QStyle::SH_SpinBox_AnimateButton, 0, this)) {
        d->reset();
    } else {
        d->edit->event(e);
    }
}

/*!
    \reimp
*/

#ifndef QT_NO_WHEELEVENT
void QAbstractSpinBox::wheelEvent(QWheelEvent *e)
{
    const int steps = (e->delta() > 0 ? 1 : -1);
    if (stepEnabled() & (steps > 0 ? StepUpEnabled : StepDownEnabled))
        stepBy(e->modifiers() & Qt::ControlModifier ? steps * 10 : steps);
    e->accept();
}
#endif


/*!
    \reimp
*/
void QAbstractSpinBox::focusInEvent(QFocusEvent *e)
{
    Q_D(QAbstractSpinBox);

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
    Q_D(QAbstractSpinBox);

    if (d->pendingEmit)
        d->interpret(EmitIfChanged);
    d->reset();
    d->edit->event(e);
    QWidget::focusOutEvent(e);
    emit editingFinished();
}

/*!
    \reimp
*/

void QAbstractSpinBox::closeEvent(QCloseEvent *e)
{
    Q_D(QAbstractSpinBox);

    d->reset();
    if (d->pendingEmit)
        d->interpret(EmitIfChanged);
    QWidget::closeEvent(e);
}

/*!
    \reimp
*/

void QAbstractSpinBox::hideEvent(QHideEvent *e)
{
    Q_D(QAbstractSpinBox);
    d->reset();
    QWidget::hideEvent(e);
}

/*!
    \reimp
*/

void QAbstractSpinBox::timerEvent(QTimerEvent *e)
{
    Q_D(QAbstractSpinBox);

    bool doStep = false;
    if (e->timerId() == d->spinClickThresholdTimerId) {
        killTimer(d->spinClickThresholdTimerId);
        d->spinClickThresholdTimerId = -1;
        d->spinClickTimerId = startTimer(d->spinClickTimerInterval);
        doStep = true;
    } else if (e->timerId() == d->spinClickTimerId) {
        doStep = true;
    }

    if (doStep) {
        const StepEnabled st = stepEnabled();
        if (d->buttonState & Up) {
            if (!(st & StepUpEnabled)) {
                d->reset();
            } else {
                stepBy(1);
            }
        } else if (d->buttonState & Down) {
            if (!(st & StepDownEnabled)) {
                d->reset();
            } else {
                stepBy(-1);
            }
        }
    }
    QWidget::timerEvent(e);
    return;
}

/*!
    \reimp
*/

void QAbstractSpinBox::contextMenuEvent(QContextMenuEvent *e)
{
#ifndef QT_NO_MENU
    Q_D(QAbstractSpinBox);

    d->reset();
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
    e->accept();
#endif
}

/*!
    \reimp
*/

void QAbstractSpinBox::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QAbstractSpinBox);

    d->updateHoverControl(e->pos());

    // If we have a timer ID, update the state
    const StepEnabled se = stepEnabled();
    if (d->spinClickTimerId != -1) {
        if ((se & StepUpEnabled) && d->hoverControl == QStyle::SC_SpinBoxUp)
            d->updateState(true);
        else if ((se & StepDownEnabled) && d->hoverControl == QStyle::SC_SpinBoxDown)
            d->updateState(false);
        else
            d->reset();
        e->accept();
    }
}

/*!
    \reimp
*/

void QAbstractSpinBox::mousePressEvent(QMouseEvent *e)
{
    Q_D(QAbstractSpinBox);

    if (e->button() != Qt::LeftButton || d->buttonState != None) {
        return;
    }

    d->updateHoverControl(e->pos());
    e->accept();

    const StepEnabled se = stepEnabled();
    if ((se & StepUpEnabled) && d->hoverControl == QStyle::SC_SpinBoxUp) {
        d->updateState(true);
    } else if ((se & StepDownEnabled) && d->hoverControl == QStyle::SC_SpinBoxDown) {
        d->updateState(false);
    } else {
        e->ignore();
    }
}

/*!
    \reimp
*/
void QAbstractSpinBox::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QAbstractSpinBox);

    if ((d->buttonState & Mouse) != 0)
        d->reset();
    e->accept();
}

// --- QAbstractSpinBoxPrivate ---

/*!
    \internal
    Constructs a QAbstractSpinBoxPrivate object
*/

QAbstractSpinBoxPrivate::QAbstractSpinBoxPrivate()
    : edit(0), type(QVariant::Invalid), spinClickTimerId(-1),
      spinClickTimerInterval(100), spinClickThresholdTimerId(-1), spinClickThresholdTimerInterval(thresholdTime),
      buttonState(None), dirty(true), cachedText("\x01"), cachedState(QValidator::Invalid),
      pendingEmit(false), readOnly(false), wrapping(false),
      ignoreCursorPositionChanged(false), frame(true),
      hoverControl(QStyle::SC_None), buttonSymbols(QAbstractSpinBox::UpDownArrows), validator(0)
{
}

/*
   \internal
   Called when the QAbstractSpinBoxPrivate is destroyed
*/
QAbstractSpinBoxPrivate::~QAbstractSpinBoxPrivate()
{
}
/*!
    \internal
    Updates the old and new hover control. Does nothing if the hover
    control has not changed.
*/
bool QAbstractSpinBoxPrivate::updateHoverControl(const QPoint &pos)
{
    Q_Q(QAbstractSpinBox);
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
    Q_Q(QAbstractSpinBox);

    QStyleOptionSpinBox opt = getStyleOption();
    opt.subControls = QStyle::SC_All;
    hoverControl = q->style()->hitTestComplexControl(QStyle::CC_SpinBox, &opt, pos, q);
    hoverRect = q->style()->subControlRect(QStyle::CC_SpinBox, &opt, hoverControl, q);
    return hoverControl;
}

/*!
    \internal
    Strips any prefix/suffix from \a text.
*/

QString QAbstractSpinBoxPrivate::stripped(const QString &t) const
{
    QString text = t;
    if (specialValueText.size() == 0 || text != specialValueText) {
        int from = 0;
        int size = text.size();
        bool changed = false;
        if (prefix.size() && text.startsWith(prefix)) {
            from += prefix.size();
            size -= from;
            changed = true;
        }
        if (suffix.size() && text.endsWith(suffix)) {
            size -= suffix.size();
            changed = true;
        }
        if (changed)
            text = text.mid(from, size);
    }
    return text.trimmed();
}

/*!
    \internal
    Returns true if a specialValueText has been set and the current value is minimum.
*/

bool QAbstractSpinBoxPrivate::specialValue() const
{
    return (value == minimum && specialValueText.size() > 0);
}

/*!
    \internal Virtual function that emits signals when the value
    changes. Reimplemented in the different subclasses.
*/

void QAbstractSpinBoxPrivate::emitSignals(EmitPolicy, const QVariant &)
{
}

/*!
    \internal

    Slot connected to the line edit's textChanged(const QString &)
    signal.
*/

void QAbstractSpinBoxPrivate::editorTextChanged(const QString &t)
{
    Q_Q(QAbstractSpinBox);

    QString tmp = t;
    int pos = edit->cursorPosition();
    QValidator::State state = q->validate(tmp, pos);
    if (state == QValidator::Acceptable) {
        const QVariant v = valueFromText(tmp);
        if (tmp != t) {
            const bool wasBlocked = edit->blockSignals(true);
            edit->setText(prefix + tmp + suffix);
            edit->blockSignals(wasBlocked);
        }
        setValue(v, EmitIfChanged, false);
        pendingEmit = false;
    } else {
        pendingEmit = true;
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
    if (!edit->hasSelectedText() && !ignoreCursorPositionChanged && !specialValue()) {
        ignoreCursorPositionChanged = true;

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
            const int selSize = edit->selectionStart() >= 0 && allowSelection
                                  ? (edit->selectedText().size()
                                     * (newpos < pos ? -1 : 1)) - newpos + pos
                                  : 0;

            const bool wasBlocked = edit->blockSignals(true);
            if (selSize != 0) {
                edit->setSelection(pos - selSize, selSize);
            } else {
                edit->setCursorPosition(pos);
            }
            edit->blockSignals(wasBlocked);
        }
        ignoreCursorPositionChanged = false;
    }
}

/*!
    \internal

    Initialises the QAbstractSpinBoxPrivate object.
*/

void QAbstractSpinBoxPrivate::init()
{
    Q_Q(QAbstractSpinBox);

    spinClickTimerInterval = q->style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatRate, 0, q);
    spinClickThresholdTimerInterval = thresholdTime;
    q->setFocusPolicy(Qt::WheelFocus);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    q->setAttribute(Qt::WA_InputMethodEnabled);

    q->setLineEdit(new QLineEdit(q));
    edit->setObjectName("qt_spinbox_lineedit");
    if (type != QVariant::Invalid) {
        validator = new QSpinBoxValidator(q, this);
        edit->setValidator(validator);
    }
}

/*!
    \internal

    Calls QWidget::update() on the area where the arrows are painted.
*/

void QAbstractSpinBoxPrivate::updateSpinBox()
{
    Q_Q(QAbstractSpinBox);

    if (q) {
        QStyleOptionSpinBox opt = getStyleOption();
        q->update(q->style()->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxUp, q));
        q->update(q->style()->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxDown, q));
    }
}

/*!
    \internal

    Resets the state of the spinbox. E.g. the state is set to
    (Keyboard|Up) if Key up is currently pressed.
*/

void QAbstractSpinBoxPrivate::reset()
{
    Q_Q(QAbstractSpinBox);

    buttonState = None;
    if (q) {
        if (spinClickTimerId != -1)
            q->killTimer(spinClickTimerId);
        if (spinClickThresholdTimerId != -1)
            q->killTimer(spinClickThresholdTimerId);
        spinClickTimerId = spinClickThresholdTimerId = -1;
        updateSpinBox();
    }
}

/*!
    \internal

    Updates the state of the spinbox.
*/

void QAbstractSpinBoxPrivate::updateState(bool up)
{
    Q_Q(QAbstractSpinBox);
    if ((up && (buttonState & Up)) || (!up && (buttonState & Down)))
        return;
    reset();
    if (q && (q->stepEnabled() & (up ? QAbstractSpinBox::StepUpEnabled
                                  : QAbstractSpinBox::StepDownEnabled))) {
        spinClickThresholdTimerId = q->startTimer(spinClickThresholdTimerInterval);
        buttonState = (up ? (Mouse | Up) : (Mouse | Down));
        q->stepBy(up ? 1 : -1);
    }
}


/*!
    \internal

    Creates a QStyleOptionSpinBox with the right flags set.
*/

QStyleOptionSpinBox QAbstractSpinBoxPrivate::getStyleOption() const
{
    Q_Q(const QAbstractSpinBox);
    QStyleOptionSpinBox opt;
    opt.init(q);
    opt.activeSubControls = 0;
    opt.buttonSymbols = buttonSymbols;
    opt.subControls = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown | QStyle::SC_SpinBoxFrame;

    if (buttonState & Up)
        opt.activeSubControls = QStyle::SC_SpinBoxUp;
    else if (buttonState & Down)
        opt.activeSubControls = QStyle::SC_SpinBoxDown;
    else
        opt.activeSubControls = hoverControl;
    if (buttonState)
        opt.state |= QStyle::State_Sunken;

    if (type != QVariant::Invalid) {
        opt.stepEnabled = q->stepEnabled();
    } else {
        opt.stepEnabled = QAbstractSpinBox::StepNone;
    }

    opt.frame = frame;
    return opt;
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
    pendingEmit = false;
    if (doUpdate)
        update();
    if (ep == AlwaysEmit || (ep == EmitIfChanged && old != value)) {
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
    int selsize = e->selectedText().size();
    const QString newText = specialValue() ? specialValueText : prefix + textFromValue(value) + suffix;
    const bool sb = e->blockSignals(true);
    e->setText(newText);

    if (!specialValue()) {
        cursor = qMin<int>(qMax<int>(cursor, prefix.size()), edit->displayText().size() - suffix.size());
        if (selsize > 0) {
            e->setSelection(cursor, selsize);
        } else {
            e->setCursorPosition(empty ? prefix.size() : cursor);
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
    Q_Q(QAbstractSpinBox);

    if (type != QVariant::Invalid) {
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

void QAbstractSpinBoxPrivate::setRange(const QVariant &min, const QVariant &max)
{
    cachedText.clear();
    cachedValue.clear();
    minimum = min;
    maximum = qMax(min, max);

    reset();
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

    Interprets text and emits signals. Called when the spinbox needs
    to interpret the text on the lineedit.
*/

void QAbstractSpinBoxPrivate::interpret(EmitPolicy ep)
{
    Q_Q(QAbstractSpinBox);
    if (type == QVariant::Invalid)
        return;

    QVariant v = getZeroVariant();
    bool doInterpret = true;
    QString tmp = edit->displayText();
    int pos = edit->cursorPosition();
    const int oldpos = pos;

    if (q->validate(tmp, pos) != QValidator::Acceptable) {
        const QString copy = tmp;
	q->fixup(tmp);
        QASBDEBUG() << "QAbstractSpinBoxPrivate::interpret() text '"
                    << edit->displayText()
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
    cachedValue.clear();
    cachedText.clear();
    setValue(v, ep, true);
    if (oldpos != pos)
        edit->setCursorPosition(pos);
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
    if (dptr->specialValueText.size() > 0 && input == dptr->specialValueText)
        return QValidator::Acceptable;

    if (!dptr->prefix.isEmpty() && !input.startsWith(dptr->prefix))
        input.prepend(dptr->prefix);

    if (!dptr->suffix.isEmpty() && !input.endsWith(dptr->suffix))
        input.append(dptr->suffix);

    return qptr->validate(input, pos);
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
#endif // QT_NO_SPINBOX

