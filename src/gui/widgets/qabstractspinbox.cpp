#include <qdatetime.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qevent.h>
#include <qstyle.h>
#if defined(Q_WS_X11)
#include <limits.h>
#endif

#include "qabstractspinbox.h"
#include "qspinbox.h"
#include "qdatetimeedit.h"
#include "qabstractspinbox_p.h"
#include <qpalette.h>

#define d d_func()
#define q q_func()

bool operator<(const QCoreVariant &arg1, const QCoreVariant &arg2);
bool operator>(const QCoreVariant &arg1, const QCoreVariant &arg2);
QCoreVariant operator+(const QCoreVariant &arg1, const QCoreVariant &arg2);
QCoreVariant operator-(const QCoreVariant &arg1, const QCoreVariant &arg2);
QCoreVariant operator*(const QCoreVariant &arg1, double multiplier);
double operator/(const QCoreVariant &arg1, const QCoreVariant &arg2);

/*!
    \class QAbstractSpinBox q4abstractspinbox.h \brief
    The QAbstractSpinBox class provides a spinwidget and a lineedit to
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
    Constructs an abstract spinbox.

    The \a parent arguments is sent to the QWidget constructor.

    \l wrapping defaults to false.
    \l tracking defaults to false.
    \l alignment defaults to Qt::AlignLeft. // ### qlocale
*/

QAbstractSpinBox::QAbstractSpinBox(QWidget *parent, Qt::WFlags f)
    : QWidget(*new QAbstractSpinBoxPrivate, parent, f)
{
    d->init();
}

QAbstractSpinBox::QAbstractSpinBox(QAbstractSpinBoxPrivate &dd, QWidget *parent, Qt::WFlags f)
    : QWidget(dd, parent, f)
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

    \value PlusMinus the buttons show <b>+</b> and <b>-</b> symbols.

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
    if (d->dirty) // needed to make sure text() returns reasonable values if the spin box is not yet shown
	d->updateEdit();

    return d->edit->displayText();
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

    \code
        QSpinBox *sb = new QSpinBox(0, 100, 1, 0, this);
        sb->setValue(100);
        sb->stepUp();
        // value is 100

        sb->setWrapping(true);
        sb->stepUp();
        // value is 0
    \endcode

q    By default, wrapping is turned off.

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

bool QAbstractSpinBox::slider() const
{
    return d->slider;
}

void QAbstractSpinBox::setSlider(bool s)
{
    d->slider = s;
    d->sliderpressed = d->slider && d->sliderpressed;
    update();
}


/*!
    \property QAbstractSpinBox::alignment
    \brief the alignment of the spin box

    Possible Values are \c Qt::AlignAuto, \c Qt::AlignLeft, \c
    Qt::AlignRight and \c Qt::AlignHCenter.

    Attempting to set the alignment to an illegal flag combination
    does nothing.

    \sa Qt::AlignmentFlags
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
    Virtual function that determines whether stepping up and down is
    legal at any given time.

    The up arrow will be painted as disabled unless stepEnabled() &
    StepUpEnabled != 0.

    The default implementation will return (StepUpEnabled|
    StepDownEnabled) if wrapping is turned on. Else it will return
    StepDownEnabled if value is > minimum() or'ed with StepUpEnabled if
    value < maximum().

    If you subclass QAbstractSpinBox you will need to reimplement this function.

    \sa QSpinBox::minimum(), QSpinBox::maximum(), wrapping()
*/


QAbstractSpinBox::StepEnabled QAbstractSpinBox::stepEnabled() const
{
    StepEnabled ret = StepNone;
    if (d->wrapping || d->value < d->maximum) {
	ret |= StepUpEnabled;
    }
    if (d->wrapping || d->value > d->minimum) {
	ret |= StepDownEnabled;
    }
    return ret;
}

/*!
    Virtual function that is called when a step is whenever the user
    triggers a step. This function takes an integer value to signify
    how many steps were taken. E.g. Pressing Qt::Key_Down will trigger a
    call to stepBy(-1), whereas pressing Qt::Key_Prior will trigger a call
    to stepBy(10).

    If you subclass QAbstractSpinBox you will need to reimplement this
    function. Note that this function is called even if the resulting
    value will be outside the bounds of minimum and maximum. It's this
    function's job to handle these situations.

    \sa QSpinBox::setValue(), QSpinBox::minimum(), QSpinBox::maximum()
*/

void QAbstractSpinBox::stepBy(int steps)
{
    QCoreVariant v = d->value;
    // to make sure it behaves reasonably when typing something and then stepping in non-tracking mode
    QString tmp = d->edit->displayText();
    if (d->pendingemit && d->validate(&tmp, 0, &v) != QValidator::Acceptable) {
        return;  // ### should I do this?
    }
    v = d->bound(v + (d->singlestep * steps), d->value, steps);
    d->setValue(v, EmitIfChanged);
    d->edit->setSelection(d->prefix.length(), d->edit->displayText().length()
                          - d->prefix.length() - d->suffix.length());
}

/*!
    This function returns a pointer to the line edit of the spin box.
*/

QLineEdit *QAbstractSpinBox::lineEdit() const
{
    return d->edit;
}

/*!
    This function interprets the text of the spin box. If the value
    has changed since last interpretation it will emit signals.
*/

void QAbstractSpinBox::interpretText()
{
    d->refresh(EmitIfChanged);
}

/*
    !\reimp
*/

void QAbstractSpinBox::showEvent(QShowEvent *)
{
    if (d->dirty) {
	d->resetState();
	d->updateEdit();
    }
    d->updateSpinBox();
}

/*
    !\reimp
*/

void QAbstractSpinBox::changeEvent(QEvent *e)
{
    switch(e->type()) {
	case QEvent::StyleChange:
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

/*
    !\reimp
*/

void QAbstractSpinBox::resizeEvent(QResizeEvent *e)
{
    QStyleOptionSpinBox sb = d->styleOption();
    sb.parts = QStyle::SC_SpinBoxEditField;
    d->edit->setGeometry(style().querySubControlMetrics(QStyle::CC_SpinBox, &sb,
                                                        QStyle::SC_SpinBoxEditField, this));
    QWidget::resizeEvent(e);
}

/*
    !\reimp
*/

QSize QAbstractSpinBox::sizeHint() const
{
    return d->sizeHint();
}

/*
    !\reimp
*/

QSize QAbstractSpinBox::minimumSizeHint() const
{
    return d->minimumSizeHint();
}

/*
    !\reimp
*/

void QAbstractSpinBox::paintEvent(QPaintEvent *)
{
    QStyleOptionSpinBox opt = d->styleOption();
    QPainter p(this);
    style().drawComplexControl(QStyle::CC_SpinBox, &opt, &p, this);
}

/*
    !\reimp

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
    case Qt::Key_Prior:
    case Qt::Key_Next:
	steps *= 10;
    case Qt::Key_Up:
    case Qt::Key_Down: {
	const bool up = (e->key() == Qt::Key_Prior || e->key() == Qt::Key_Up);
	if (!(stepEnabled() & (up ? StepUpEnabled : StepDownEnabled)))
	    return;
	if (!up)
	    steps *= -1;
	if (style().styleHint(QStyle::SH_SpinBox_AnimateButton, this)) {
	    d->buttonstate = (Keyboard | (up ? Up : Down));
	}
	stepBy(steps);
	return; }

    case Qt::Key_Enter:
    case Qt::Key_Return:
	d->refresh(AlwaysEmit);
	d->edit->setSelection(d->prefix.length(), d->edit->displayText().length() - d->prefix.length() - d->suffix.length());
	return;

    case Qt::Key_Z:
    case Qt::Key_Y:
	if (e->state() & Qt::ControlButton) // don't allow undo/redo I guess I maybe should do somwthing in acceloverride
	    break;
    default: break;
    }
    QWidget::keyPressEvent(e);
}

/*
    !\reimp
*/

void QAbstractSpinBox::keyReleaseEvent(QKeyEvent *e)
{
    if (style().styleHint(QStyle::SH_SpinBox_AnimateButton, this)
	&& d->buttonstate & Keyboard && !e->isAutoRepeat()) {
	d->resetState();
    } else {
	e->ignore();
    }
}

/*
    !\reimp
*/

void QAbstractSpinBox::wheelEvent(QWheelEvent *e)
{
    setFocus();
    const int steps = (e->delta() > 0 ? 1 : -1);
    if (stepEnabled() & (steps > 0 ? StepUpEnabled : StepDownEnabled))
	stepBy(e->state() & Qt::ControlButton ? steps * 10 : steps);
    e->accept();
}

/*
    !\reimp
*/

void QAbstractSpinBox::focusOutEvent(QFocusEvent *e)
{
    if (d->pendingemit)
	d->refresh(EmitIfChanged);
    QWidget::focusOutEvent(e);
}

/*
    !\reimp
*/

void QAbstractSpinBox::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->spinkeytimerid || e->timerId() == d->spinclicktimerid) {
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

/*
    !\reimp
*/

void QAbstractSpinBox::contextMenuEvent(QContextMenuEvent *e)
{
#ifndef QT_NO_POPUPMENU
    d->resetState();
    const QPointer<QMenu> menu = new QMenu(this);
#ifndef QT_NO_CLIPBOARD
    const bool selected = d->edit->hasSelectedText();
    menu->addAction(tr("Cu&t"), d->edit, SLOT(cut()))->setEnabled(selected);
    menu->addAction(tr("&Copy"), d->edit, SLOT(copy()))->setEnabled(selected);
    menu->addAction(tr("&Paste"), d->edit,
                    SLOT(paste()))->setEnabled(QApplication::clipboard()->text().size());
#endif
    menu->addAction(tr("Select &All"), d->edit, SLOT(selectAll()));
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
}

/*
    !\reimp
*/

void QAbstractSpinBox::mouseMoveEvent(QMouseEvent *e)
{
    d->dragging = true;
    if (d->sliderpressed)
        d->setValue(d->valueForPosition(e->pos().x()), EmitIfChanged);
    QWidget::mouseMoveEvent(e);
}

/*
    !\reimp
*/

void QAbstractSpinBox::mousePressEvent(QMouseEvent *e)
{
    const QPoint p(e->pos());
    const StepEnabled se = stepEnabled();
    QStyleOptionSpinBox sb = d->styleOption();
    sb.parts = QStyle::SC_All;
    if (style().querySubControlMetrics(QStyle::CC_SpinBox, &sb,
                                       QStyle::SC_SpinBoxUp, this).contains(p)) {
	if (e->button() != Qt::LeftButton || !(se & StepUpEnabled) || d->buttonstate != None) {
	    e->accept();
	    return;
	}
	d->spinclicktimerid = startTimer(d->spinclicktimerinterval);
	d->buttonstate = (Mouse | Up);
	stepBy(1);
        return;
    }
    if (style().querySubControlMetrics(QStyle::CC_SpinBox, &sb,
                                       QStyle::SC_SpinBoxDown, this).contains(p)) {
	if (e->button() != Qt::LeftButton || !(se & StepDownEnabled) || d->buttonstate != None) {
	    e->accept();
	    return;
	}
	d->spinclicktimerid = startTimer(d->spinclicktimerinterval);
	d->buttonstate = (Mouse | Down);
	stepBy(-1);
        return;
    }

    if (d->slider && style().querySubControlMetrics(QStyle::CC_SpinBox, &sb,
                                                    QStyle::SC_SpinBoxSlider, this).contains(p)) {
        if (e->button() != Qt::LeftButton || d->buttonstate != None) {
	    e->accept();
	    return;
	}
        d->sliderpressed = true;
        d->setValue(d->valueForPosition(e->pos().x()), EmitIfChanged);
        return;
    }

    QWidget::mousePressEvent(e);
}

/*
    !\reimp
*/

void QAbstractSpinBox::mouseReleaseEvent(QMouseEvent *e)
{
    d->dragging = d->sliderpressed = false;
    if (d->buttonstate & Mouse) {
	d->resetState();
    } else {
	e->ignore();
    }
}

// --- QAbstractSpinBoxPrivate ---

/*!
    \internal
    Constructs a QAbstractSpinBoxPrivate object
*/

QAbstractSpinBoxPrivate::QAbstractSpinBoxPrivate()
    : edit(0), spinclicktimerid(-1), spinkeytimerid(-1), spinclicktimerinterval(100), spinkeytimerinterval(200),
      buttonstate(None), sizehintdirty(true), dirty(true), useprivate(false), pendingemit(false),
      tracking(false), wrapping(false), dragging(false), ignorecursorpositionchanged(false), slider(false),
      sliderpressed(false), buttonsymbols(QAbstractSpinBox::UpDownArrows)
{
    resetState();
}

/*!
    \internal
    Strips any prefix/suffix from \a text.
*/

void QAbstractSpinBoxPrivate::strip(QString *text) const
{
    if (specialvaluetext.size() && *text == specialvaluetext)
	return;
    int from = 0;
    int length = text->length();
    if (prefix.size() && text->startsWith(prefix)) {
	from += prefix.length();
	length -= from;
    }
    if (suffix.size() && text->endsWith(suffix)) {
	length -= suffix.length();
    }
    *text = text->mid(from, length).trimmed();
}

/*!
    \internal
    Returns true if a specialValueText has been set and the current value is minimum.
*/

bool QAbstractSpinBoxPrivate::specialValue() const
{
    return (value == minimum && specialvaluetext.size());
}

/*!
    \internal
    Virtual function that emits signals. Reimplemented in the different subclasses.
*/

void QAbstractSpinBoxPrivate::emitSignals()
{
    if (slider) {
        updateSlider();
    }
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
        QValidator::State state;
	const QCoreVariant v = d->mapTextToValue(&tmp, &state); // Already validated
	if (v != value && state == QValidator::Acceptable) {
            if (tmp != t) {
                const bool wasBlocked = edit->blockSignals(true);
                edit->setText(tmp);
                edit->blockSignals(wasBlocked);
            }
            value = v; // don't want to call setValue because that would change the text.
            emitSignals();
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
    if (!ignorecursorpositionchanged) {
        ignorecursorpositionchanged = true;

        bool allowSelection = true;
        if (!dragging) {
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
                    pos = oldpos;
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
    spinclicktimerinterval = q->style().styleHint(QStyle::SH_SpinBox_ClickAutoRepeatRate, q);
    spinkeytimerinterval = q->style().styleHint(QStyle::SH_SpinBox_KeyPressAutoRepeatRate, q);
    lineEdit()->setObjectName(QString("lineedit for %1").arg(QString(q->objectName())).latin1());
    edit->setAttribute(Qt::WA_CompositeChild);
    q->setAttribute(Qt::WA_CompositeParent);
    q->setFocusProxy(edit);
    if (useprivate) {
        edit->setValidator(new QSpinBoxValidator(this, q));
	QObject::connect(edit, SIGNAL(textChanged(QString)), q, SLOT(editorTextChanged(QString)));
        QObject::connect(edit, SIGNAL(cursorPositionChanged(int, int)), q,
                         SLOT(editorCursorPositionChanged(int, int)));
    }
}

/*!
    \internal

    Returns the line edit and creates it if edit == 0.
*/


QLineEdit *QAbstractSpinBoxPrivate::lineEdit()
{
    if (!edit)
	edit = new QLineEdit(q);
    return edit;
}

/*!
    \internal

    Calls QWidget::update() on the area where the arrows are painted.
*/

void QAbstractSpinBoxPrivate::updateSpinBox()
{
    if (q) {
        QStyleOptionSpinBox sb = styleOption();
	q->update(q->style().querySubControlMetrics(QStyle::CC_SpinBox, &sb,
                                                    QStyle::SC_SpinBoxButtonField, q));
    }
}

/*!
    \internal

    Calls QWidget::update() on the area where the slider is painted.
*/

void QAbstractSpinBoxPrivate::updateSlider()
{
    if (q) {
        QStyleOptionSpinBox sb = styleOption();
	q->update(q->style().querySubControlMetrics(QStyle::CC_SpinBox, &sb,
                                                    QStyle::SC_SpinBoxSlider, q));
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
	if (spinkeytimerid != -1)
	    q->killTimer(spinkeytimerid);
	spinkeytimerid = -1;
	updateSpinBox();
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
        int w = 35;
        int wx = fm.width(' ');
        QString s;
        s = prefix + mapValueToText(minimum) + suffix;
        s.truncate(18);
        w = qMax(w, fm.width(s) + wx);
        s = prefix + mapValueToText(maximum) + suffix;
        s.truncate(18);
        w = qMax(w, fm.width(s) + wx);
        if (specialvaluetext.size()) {
            s = specialvaluetext;
            w = qMax(w, fm.width(s) + wx);
        }
        w += 30;

        QStyleOptionSpinBox sb = styleOption();
        cachedsizehint = QSize(w + q->style().querySubControlMetrics(QStyle::CC_SpinBox, &sb,
                                                                QStyle::SC_SpinBoxButtonField, q).
                               width(), h + q->style().pixelMetric(QStyle::PM_DefaultFrameWidth) * 2).
                         expandedTo(QApplication::globalStrut());
        h = edit->minimumSizeHint().height() + (q->style().pixelMetric(QStyle::PM_DefaultFrameWidth) * 2);
        if (slider)
            h += q->style().pixelMetric(QStyle::PM_SpinBoxSliderHeight, q);
        cachedminimumsizehint = QSize(w, h).expandedTo(QApplication::globalStrut());
        const_cast<QAbstractSpinBoxPrivate *>(this)->sizehintdirty = false;
    }
}

/*!
    \internal

    Creates a QStyleOptionSpinBox with the right flags set.
*/

QStyleOptionSpinBox QAbstractSpinBoxPrivate::styleOption() const
{
    QStyleOptionSpinBox opt(0);
    opt.init(q);
    opt.stepEnabled = q->stepEnabled();
    opt.activeParts = 0;
    opt.buttonSymbols = buttonsymbols;
    opt.parts = QStyle::SC_SpinBoxFrame | QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown;
    if (slider)
        opt.parts |= QStyle::SC_SpinBoxSlider;

    if (buttonstate & Up)
        opt.activeParts = QStyle::SC_SpinBoxUp;
    else if (buttonstate & Down)
        opt.activeParts = QStyle::SC_SpinBoxDown;

    opt.percentage = (value - minimum) / (maximum - minimum);
    opt.slider = slider;
    return opt;
}

QCoreVariant QAbstractSpinBoxPrivate::valueForPosition(int pos) const
{
    QStyleOptionSpinBox sb = styleOption();
    QRect r = q->style().querySubControlMetrics(QStyle::CC_SpinBox, &sb,
                                                QStyle::SC_SpinBoxSlider, q);

    double percentage = (double)pos / r.width();

    QCoreVariant ret = minimum + (maximum - minimum) * percentage;
    return ret;
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

QCoreVariant QAbstractSpinBoxPrivate::bound(const QCoreVariant &val, const QCoreVariant &old, int steps) const
{
    QCoreVariant v = val;
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
	} else if (wasMin && steps < 0 || wasMax && steps > 0) {
	    v = (wasMax ? minimum : maximum);
	}
    }

    return v;
}

/*!
    \internal

    Sets the value of the spin box to \a val. Depending on the value
    of \ep it will also emit signals.
*/

void QAbstractSpinBoxPrivate::setValue(const QCoreVariant &val, EmitPolicy ep)
{
    const QCoreVariant old = value;
    value = bound(val);
    pendingemit = false;
    update();
    if (ep == AlwaysEmit || (ep == EmitIfChanged && old != value)) {
	emitSignals();
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
    const QString newText = specialValue() ? specialvaluetext : prefix + mapValueToText(value) + suffix;
    const bool sb = e->blockSignals(true);
    e->setText(newText);

    cursor = qMin(qMax(cursor, prefix.length()), edit->displayText().length() - suffix.length());
    if (sellength > 0) {
        e->setSelection(cursor, sellength);
    } else {
        e->setCursorPosition(empty ? prefix.length() : cursor);
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
    if (useprivate) {
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

    Virtual method called from QSpinBoxValidator::validate. This
    method is reimeplemented in the various subclasses to map the text
    of the lineedit to a value of the right type.
*/

QCoreVariant QAbstractSpinBoxPrivate::mapTextToValue(QString *, QValidator::State *) const
{
    return QCoreVariant();
}

/*!
    \internal

    Virtual method called from updateEdit. This method method is
    reimeplemented in the various subclasses to map a value to the
    string it should be displayed as.
*/

QString QAbstractSpinBoxPrivate::mapValueToText(const QCoreVariant &) const
{
    return QString();
}

/*!
    \internal

    Convenience function to set min/max values.
*/

void QAbstractSpinBoxPrivate::setBoundary(Boundary b, const QCoreVariant &val)
{
    if (b == Minimum) {
	if (maximum < val) {
	    minimum = maximum;
	    maximum = val;
	} else {
	    minimum = val;
	}
    } else {
	if (minimum > val) {
	    maximum = minimum;
	    minimum = val;
	} else {
	    maximum = val;
	}
    }
    setValue(bound(value), EmitIfChanged);
    resetState();
    update();
}

/*!
    \internal

    Convenience function to get a variant of the right type.
*/

QCoreVariant QAbstractSpinBoxPrivate::getZeroVariant() const
{
    QCoreVariant ret;
    switch (type) {
    case QCoreVariant::Int: ret = QCoreVariant((int)0); break;
    case QCoreVariant::Double: ret = QCoreVariant((double)0); break;
    case QCoreVariant::DateTime: ret = QCoreVariant(DATETIME_MIN); break;
    default: break;
    }
    return ret;
}

/*!
    \internal

    Virtual method called from QSpinBoxValidator::validate. This
    method is reimeplemented in the various subclasses.
*/

QValidator::State QAbstractSpinBoxPrivate::validate(QString *input, int *, QCoreVariant *val) const
{
    if (!useprivate) {
	return QValidator::Acceptable;
    } else if (!input) {
        return QValidator::Invalid;
    } else if (specialvaluetext.size() && *input == specialvaluetext) {
	if (val)
	    *val = minimum;
	return QValidator::Acceptable;
    } else if ((prefix.size() && !input->startsWith(prefix))
	       || (suffix.size() && !input->endsWith(suffix))) {
 	return QValidator::Invalid;
    }

    strip(input);
    QValidator::State state;
    if (val) {
        *val = mapTextToValue(input, &state);
    } else {
        mapTextToValue(input, &state);
    }
    if (prefix.size() || suffix.size())
        *input = prefix + *input + suffix;

    return state;
}

/*!
    \internal

    Interprets text and emits signals. Called when the user presses Enter.
*/

void QAbstractSpinBoxPrivate::refresh(EmitPolicy ep)
{
    if (!useprivate)
	return;

    QCoreVariant v = getZeroVariant();
    QString tmp = d->edit->displayText();
    int pos = d->edit->cursorPosition();
    if (validate(&tmp, &pos, &v) != QValidator::Acceptable) {
	v = value;
    }

    setValue(v, ep);
}

// --- QSpinBoxValidator ---

/*!
    \internal
    Constructs a QSpinBoxValidator object
*/

QSpinBoxValidator::QSpinBoxValidator(QAbstractSpinBoxPrivate *p, QObject *parent)
    : QValidator(parent)
{
    dptr = p;
}

/*!
    \internal
    Calls the virtual QAbstractSpinBoxPrivate::validate function.
*/

QValidator::State QSpinBoxValidator::validate(QString &input, int &pos) const
{
    State s = dptr->validate(&input, &pos, 0);
    return s;
}

// --- global ---


/*!
    \internal
    Compares two variants and returns true if \a arg1 < \a arg2
*/

bool operator<(const QCoreVariant &arg1, const QCoreVariant &arg2)
{
    if (arg1.type() != arg2.type())
	qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
		 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QCoreVariant::Int: return arg1.toInt() < arg2.toInt();
    case QCoreVariant::Double: return arg1.toDouble() < arg2.toDouble();
    case QCoreVariant::DateTime: return arg1.toDateTime() < arg2.toDateTime();
    default: break;
    }
    return false;
}

/*!
    \internal
    Compares two variants and returns true if \a arg1 > \a arg2
*/

bool operator>(const QCoreVariant &arg1, const QCoreVariant &arg2)
{
    if (arg1.type() != arg2.type())
	qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
		 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QCoreVariant::Int: return arg1.toInt() > arg2.toInt();
    case QCoreVariant::Double: return arg1.toDouble() > arg2.toDouble();
    case QCoreVariant::DateTime: return arg1.toDateTime() > arg2.toDateTime();
    default: break;
    }
    return false;
}

/*!
    \internal
    Adds two variants together and returns the result.
*/

QCoreVariant operator+(const QCoreVariant &arg1, const QCoreVariant &arg2)
{
    QCoreVariant ret;
    if (arg1.type() != arg2.type())
	qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
		 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QCoreVariant::Int: ret = QCoreVariant(arg1.toInt() + arg2.toInt()); break;
    case QCoreVariant::Double: ret = QCoreVariant(arg1.toDouble() + arg2.toDouble()); break;
    case QCoreVariant::DateTime: {
        QDateTime a2 = arg2.toDateTime();
        QDateTime a1 = arg1.toDateTime().addDays(DATETIME_MIN.daysTo(a2));
        a1.setTime(a1.time().addMSecs(QTime().msecsTo(a2.time())));
        ret = QCoreVariant(a1);
    }
    default: break;
    }
    return ret;
}


/*!
    \internal
    Subtracts two variants and returns the result.
*/

QCoreVariant operator-(const QCoreVariant &arg1, const QCoreVariant &arg2)
{
    QCoreVariant ret;
    if (arg1.type() != arg2.type())
	qWarning("%s %d: Different types. This should never happen (%s vs %s)", __FILE__, __LINE__,
		 arg1.typeName(), arg2.typeName());
    switch (arg1.type()) {
    case QCoreVariant::Int: ret = QCoreVariant(arg1.toInt() - arg2.toInt()); break;
    case QCoreVariant::Double: ret = QCoreVariant(arg1.toDouble() - arg2.toDouble()); break;
    case QCoreVariant::DateTime: {
        QDateTime a1 = arg1.toDateTime();
        QDateTime a2 = arg2.toDateTime();
        int days = a2.daysTo(a1);
        int secs = a2.secsTo(a2);
        int msecs = qMax(0, a1.time().msec() - a2.time().msec());
        if (days < 0 || secs < 0 || msecs < 0) {
            qDebug("%s %d: if (days < 0 || secs < 0 || msecs < 0) {", __FILE__, __LINE__);
            ret = arg1;
        } else {
            QDateTime dt = a2.addDays(days).addSecs(secs);
            if (msecs > 0)
                dt.setTime(dt.time().addMSecs(msecs));
            ret = QCoreVariant(dt);
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

QCoreVariant operator*(const QCoreVariant &arg1, double multiplier) // should probably do each field more separately
{
    QCoreVariant ret;

    switch (arg1.type()) {
    case QCoreVariant::Int: ret = QCoreVariant((int)(arg1.toInt() * multiplier)); break;
    case QCoreVariant::Double: ret = QCoreVariant(arg1.toDouble() * multiplier); break;
    case QCoreVariant::DateTime: {
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



double operator/(const QCoreVariant &arg1, const QCoreVariant &arg2)
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
    case QVariant::DateTime: {
        a1 = DATE_MIN.daysTo(arg1.toDate());
        a2 = DATE_MIN.daysTo(arg2.toDate());
        a1 += (double)TIME_MIN.msecsTo(arg1.toDateTime().time()) / (long)(3600 * 24 * 1000);
        a2 += (double)TIME_MIN.msecsTo(arg2.toDateTime().time()) / (long)(3600 * 24 * 1000);
    }
    default: break;
    }

    return (a1 != 0 && a2 != 0) ? (a1 / a2) : 0.0;
}

#include "moc_qabstractspinbox.cpp"

