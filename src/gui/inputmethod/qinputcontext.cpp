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

/****************************************************************************
**
** Implementation of QInputContext class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

//#define QT_NO_IM_PREEDIT_RELOCATION

#include "qinputcontext.h"

#ifndef QT_NO_IM

#include "qplatformdefs.h"

#include "qapplication.h"
#include "qwidget.h"
#include "private/qobject_p.h"
#include "qmenu.h"
#include "qtextformat.h"
#include "qpalette.h"

#include <stdlib.h>
#include <limits.h>

// TODO: separate QInputContextPrivate into qinputcontext_p.h and
//       qinputcontext_x11.cpp
class QInputContextPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QInputContext)
public:
    QInputContextPrivate()
	: focusWidget(0)
    {}

    QWidget *focusWidget;
};


/*!
    \class QInputContext qinputcontext.h
    \brief The QInputContext class abstracts the input method dependent data and composing state.

    \ingroup i18n

    An input method is responsible to input complex text that cannot
    be inputted via simple keymap. It converts a sequence of input
    events (typically key events) into a text string through the input
    method specific converting process. The class of the processes are
    widely ranging from simple finite state machine to complex text
    translator that pools a whole paragraph of a text with text
    editing capability to perform grammar and semantic analysis.

    To abstract such different input method specific intermediate
    information, Qt offers the QInputContext as base class. The
    concept is well known as 'input context' in the input method
    domain. an input context is created for a text widget in response
    to a demand. It is ensured that an input context is prepared for
    an input method before input to a text widget.

    Multiple input contexts that is belonging to a single input method
    may concurrently coexist. Suppose multi-window text editor. Each
    text widget of window A and B holds different QInputContext
    instance which contains different state information such as
    partially composed text.

    \section1 Groups of functions:

    \table
    \header \i Context \i Functions

    \row \i Receiving information \i
	x11FilterEvent(),
	filterEvent(),
	mouseHandler()

    \row \i Sending back composed text \i
	sendIMEvent(),

    \row \i State change notification \i
	setFocus(),
	unsetFocus(),
	reset()

    \row \i Context information \i
	identifierName(),
	language(),
	font(),
	isComposing(),

    \endtable


    \section1 Sharing input context between text widgets

    Any input context can be shared between several text widgets to
    reduce resource consumption. In ideal case, each text widgets
    should be allocated dedicated input context. But some complex
    input contexts require slightly heavy resource such as 100
    kilobytes of memory. It prevents quite many text widgets from
    being used concurrently.

    To resolve such problem, we can share an input context. There is
    one 'input context holder widget' per text widgets that shares
    identical input context. In this model, the holder widget owns the
    shared input context. Other text widgets access the input context
    via QApplication::locateICHolderWidget(). But the access
    convention is transparently hidden into QWidget, so developers are
    not required to aware of it.

    What developer should know is only the mapping function
    QApplication::locateICHolderWidget(). It accepts a widget as
    argument and returns its holder widget. Default implementation
    returns the top-level widget of the widget as reasonable
    assumption.  But some applications should reimplement the function
    to fit application specific usability. See
    QApplication::locateICHolderWidget() for further information.


    \section1 Preedit preservation

    As described above, input contexts have wide variety of amount of
    the state information in accordance with belonging input
    method. It is ranging from 2-3 keystrokes of sequence in
    deterministic input methods to hundreds of keystrokes with
    semantic text refinement in complex input methods such as ordinary
    Japanese input method. The difference requires the different reset
    policies in losing input focus.

    The former simple input method case, users will prefer resetting
    the context to back to the neutral state when something
    happened. Suppose a web browsing. The user scroll the page by
    scrollbar after he or she has typed a half of the valid key
    sequence into a text widget. In the case, the input context should
    be reset in losing focus when he or she has dragged the
    scrollbar. He or she will be confused if the input context is
    still preserved until focused back to the text widget because he
    or she will restart typing with first key of the sequence as a
    habitual operation.

    On the other hand, we should choose completely different policy
    for the latter complex input method case. Suppose same situation
    as above but he or she is using a complex input method. In the
    case, he or she will be angry if the input context has been lost
    when he or she has dragged the scrollbar because the input context
    contained a valuably composed text made up by considerable input
    cost. So we should not reset the input context in the case. And
    the input context should be preserved until focused back to the
    text widget. This behavior is named as 'preedit preservation'.

    The two policies can be switched by calling or not calling reset()
    in unsetFocus(). Default implementation of unsetFocus() calls
    reset() to fit the simple input methods. The implementation is
    expressed as 'preedit preservation is disabled'.


    \section1 Preedit relocation

    Although the most case of the preedit preservation problem for
    complex input methods is resolved as described above, there is a
    special case. Suppose the case that matches all of the following
    conditions.

    \list

    \i a input focus has been moved from a text widget to another text
    widget directly

    \i the input context is shared between the two text widgets

    \i preedit preservation is enabled for the input context

    \endlist

    In the case, there are the following two requirements that
    contradicts each other. The input context sharing causes it.

    \list

    \i the input context has to be reset to prepare to input to the
    newly focused text widget

    \i the input context has to be preserved until focused back to the
    previous text widget

    \endlist

    A intrinsic feature named 'preedit relocation' is available to
    compromise the requirements. If the feature is enabled for the
    input context, it is simply moved to the new text widget with the
    preedit string. The user continues the input on the new text
    widget, or relocate it to another text widget. The preedit of
    previous text widget is automatically cleared to back to the
    neutral state of the widget.

    This strange behavior is just a compromise. As described in
    previous section, complex input method user should not be exposed
    to the risk losing the input context because it contains valuable
    long text made up with considerable input cost. The user will
    immediately focus back to the previous text widget to continue the
    input in the correct text widget if the preedit relocation
    occurred. The feature is mainly existing as safety.

    The feature properly works even if the focus is moved as
    following. Input method developers are not required to be aware of
    the relocation protocol since QInputContext transparently handles
    it.

    a text widget -> a non-text widget -> another text widget

    To enable the preedit relocation feature, the input context class
    have to reimplement isPreeditRelocationEnabled() as returns true.
    The implementation requires that the preedit preservation is also
    enabled since preedit relocation is a special case of the preedit
    preservation. If the preedit relocation is disabled, the input
    context is simply reset in the relocation case.


    \section1 Input context instanciation
    \section1 Input method switching

    \section1 Text widget implementor's guide

    Add following code fragment into createPopupMenu() to add input
    method dependent submenus.

    \code
    #ifndef QT_NO_IM
        QInputContext *qic = getInputContext();
        if (qic)
            qic->addActionsTo(popup);
    #endif
    \endcode

    \sa QInputContextPlugin, QInputContextFactory, QApplication::locateICHolderWidget(), QApplication::defaultInputMethod()
*/


/*!
    Constructs an input context with the given \a parent.
*/
QInputContext::QInputContext(QObject* parent)
    : QObject(*new QInputContextPrivate, parent)
{
}


/*!
    Destroys the input context.
*/
QInputContext::~QInputContext()
{
}

/*!
    \internal
    Returns the widget that has an input focus for this input
    context. Ordinary input methods should not call this function
    directly to keep platform independence and flexible configuration
    possibility.

    The return value may differ from holderWidget() if the input
    context is shared between several text widgets.

    \sa setFocusWidget(), holderWidget()
*/
QWidget *QInputContext::focusWidget() const
{
    Q_D(const QInputContext);
    return d->focusWidget;
}


/*!
    \internal
    Sets the widget that has an input focus for this input
    context. Ordinary input methods must not call this function
    directly.

    \sa focusWidget()
*/
void QInputContext::setFocusWidget(QWidget *widget)
{
    Q_ASSERT(!widget || widget->testAttribute(Qt::WA_InputMethodEnabled));
    Q_D(QInputContext);
    d->focusWidget = widget;
}

/*!
    \fn bool QInputContext::isComposing() const

    This function indicates whether InputMethodStart event had been
    sent to the current focus widget. It is ensured that an input
    context can send InputMethodCompose or InputMethodEnd event safely
    if this function returned true.

    The state is automatically being tracked through sendIMEvent().

    \sa sendIMEvent()
*/

/*!
    This function can be reimplemented in a subclass to filter input
    events.

    Return true if the \a event has been consumed. Otherwise, the
    unfiltered \a event will be forwarded to widgets as ordinary
    way. Although the input events have accept() and ignore()
    methods, leave it untouched.

    \a event is currently restricted to QKeyEvent. But some input
    method related events such as QWheelEvent or QTabletEvent may be
    added in future.

    The filtering opportunity is always given to the input context as
    soon as possible. It has to be taken place before any other key
    event consumers such as eventfilters and accelerators because some
    input methods require quite various key combination and
    sequences. It often conflicts with accelerators and so on, so we
    must give the input context the filtering opportunity first to
    ensure all input methods work properly regardless of application
    design.

    Ordinary input methods require discrete key events to work
    properly, so Qt's key compression is always disabled for any input
    contexts.

    \sa QKeyEvent, x11FilterEvent()
*/
bool QInputContext::filterEvent(const QEvent * /*event*/)
{
    return false;
}

/*!
  Sends an input method event specified by \a event to the current focus
  widget. Implementations of QInputContext should call this method to
  send the generated input method events and not
  QApplication::sendEvent(), as the events might have to get dispatched
  to a different application on some platforms.

  Some complex input methods route the handling to several child
  contexts (e.g. to enable language switching). To account for this,
  QInputContext will check if the parent object is a QInputContext. If
  yes, it will call the parents sendEvent() implementation instead of
  sending the event directly.

  \sa QInputMethodEvent
*/
void QInputContext::sendEvent(const QInputMethodEvent &event)
{
    // route events over input context parents to make chaining possible.
    QInputContext *p = qobject_cast<QInputContext *>(parent());
    if (p) {
        p->sendEvent(event);
        return;
    }

    QWidget *focus = focusWidget();
    if (!focus)
	return;

    QInputMethodEvent e(event);
    qApp->sendEvent(focus, &e);
}


/*!
    This function can be reimplemented in a subclass to handle mouse
    presses/releases/doubleclicks/moves within the preedit text. You
    can use the function to implement mouse-oriented user interface
    such as text selection or popup menu for candidate selection.

    The parameter \a x is the offset within the string that was sent
    with the InputMethodCompose event. The alteration boundary of \a x is
    ensured as character boundary of preedit string accurately.

    The event type is \c QEvent::MouseButtonPress,
    \c QEvent::MouseButtonRelease, \c QEvent::MouseButtonDblClick or \c
    QEvent::MouseButtonMove. The event's button and state indicate
    the kind of operation that was performed.

    The method interface is imported from
    QWSInputMethod::mouseHandler() of Qt/Embedded 2.3.7 and extended
    for desktop system.
 */
void QInputContext::mouseHandler(int /*x*/, QMouseEvent *event)
{
    // Default behavior for simple ephemeral input contexts. Some
    // complex input contexts should not be reset here.
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
	reset();
}


/*!
    Returns the font of the current input widget
*/
QFont QInputContext::font() const
{
    Q_D(const QInputContext);
    if (!d->focusWidget)
        return QApplication::font();

    return qvariant_cast<QFont>(d->focusWidget->inputMethodQuery(Qt::ImFont));
}

/*!
    This virtual function is called when a state in the focus widget
    has changed. QInputContext can then use
    QWidget::inputMethodQuery() to query the new state of the widget.
*/
void QInputContext::update()
{
}

/*!
    This virtual function is called when the specified \a widget is
    destroyed. The \a widget is a widget on which this input context
    is installed.
*/
void QInputContext::widgetDestroyed(QWidget *widget)
{
    Q_D(QInputContext);
    if (widget == d->focusWidget)
        setFocusWidget(0);
}

/*!
    \fn void QInputContext::reset()

    This function can be reimplemented in a subclass to reset the
    state of the input method.

    This function is called by several widgets to reset input
    state. For example, a text widget call this function before
    inserting a text to make widget ready to accept a text.

    Default implementation is sufficient for simple input method. You
    can override this function to reset external input method engines
    in complex input method. In the case, call QInputContext::reset()
    to ensure proper termination of inputting.

    You must not send any QInputMethodEvent except empty InputMethodEnd event using
    QInputContext::reset() at reimplemented reset(). It will break
    input state consistency.
*/


/*!
  \fn QString QInputContext::identifierName()

    This function must be implemented in any subclasses to return the
    identifier name of the input method.

    Return value is the name to identify and specify input methods for
    the input method switching mechanism and so on. The name has to be
    consistent with QInputContextPlugin::keys(). The name has to
    consist of ASCII characters only.

    There are two different names with different responsibility in the
    input method domain. This function returns one of them. Another
    name is called 'display name' that stands for the name for
    endusers appeared in a menu and so on.

    \sa QInputContextPlugin::keys(), QInputContextPlugin::displayName()
*/


/*!
  \fn QString QInputContext::language()

    This function must be implemented in any subclasses to return a
    language code (e.g. "zh_CN", "zh_TW", "zh_HK", "ja", "ko", ...)
    of the input context. If the input context can handle multiple
    languages, return the currently used one. The name has to be
    consistent with QInputContextPlugin::language().

    This information will be used by language tagging feature in
    QInputMethodEvent. It is required to distinguish unified han characters
    correctly. It enables proper font and character code
    handling. Suppose CJK-awared multilingual web browser
    (that automatically modifies fonts in CJK-mixed text) and XML editor
    (that automatically inserts lang attr).

    \sa QInputContextPlugin::language()
*/


/*!
    This is a preliminary interface for Qt 4.
*/
QList<QAction *> QInputContext::actions()
{
    return QList<QAction *>();
}

/*!
    \enum QInputContext::StandardFormat

    \value PreeditFormat  The preedit text.
    \value SelectionFormat  The selection text.

    \sa standardFormat()
*/

/*!
    Returns a QTextFormat object that specifies the format for
    component \a s.
*/
QTextFormat QInputContext::standardFormat(QInputContext::StandardFormat s) const
{
    QWidget *focus = focusWidget();
    const QPalette &pal = focus ? focus->palette() : qApp->palette();

    QTextCharFormat fmt;
    QColor bg;
    switch (s) {
    case QInputContext::PreeditFormat: {
        fmt.setFontUnderline(true);
        int h1, s1, v1, h2, s2, v2;
        pal.color(QPalette::Base).getHsv(&h1, &s1, &v1);
        pal.color(QPalette::Background).getHsv(&h2, &s2, &v2);
        bg.setHsv(h1, s1, (v1 + v2) / 2);
        break;
    }
    case QInputContext::SelectionFormat: {
        bg = pal.text().color();
        fmt.setForeground(pal.background());
        break;
    }
    }
    fmt.setBackground(QBrush(bg));
    return fmt;
}

#ifdef Q_WS_X11
/*!
    This function may be overridden only if input method is depending
    on X11 and you need raw XEvent. Otherwise, this function must not.

    This function is designed to filter raw key events for XIM, but
    other input methods may use this to implement some special
    features such as distinguishing Shift_L and Shift_R.

    Return true if the \a event has been consumed. Otherwise, the
    unfiltered \a event will be translated into QEvent and forwarded
    to filterEvent(). Filtering at both x11FilterEvent() and
    filterEvent() in single input method is allowed.

    \a keywidget is a client widget into which a text is inputted. \a
    event is inputted XEvent.

    \sa filterEvent()
*/
bool QInputContext::x11FilterEvent(QWidget * /*keywidget*/, XEvent * /*event*/)
{
    return false;
}
#endif

#endif //Q_NO_IM
