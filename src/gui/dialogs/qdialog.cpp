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

#include "qdialog.h"

#ifndef QT_NO_DIALOG

#include "qevent.h"
#include "qdesktopwidget.h"
#include "qpushbutton.h"
#include "qapplication.h"
#include "qlayout.h"
#include "qsizegrip.h"
#include "qwhatsthis.h"
#include "qmenu.h"
#include "qcursor.h"
#include "private/qdialog_p.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#if defined(Q_OS_TEMP)
#include "qt_windows.h"
#endif

/*!
    \class QDialog
    \brief The QDialog class is the base class of dialog windows.

    \ingroup dialogs
    \ingroup abstractwidgets
    \mainclass

    A dialog window is a top-level window mostly used for short-term
    tasks and brief communications with the user. QDialogs may be
    modal or modeless. QDialogs support \link #extensibility
    extensibility\endlink and can provide a \link #return return
    value\endlink. They can have \link #default default
    buttons\endlink. QDialogs can also have a QSizeGrip in their
    lower-right corner, using setSizeGripEnabled().

    Note that QDialog uses the parent widget slightly differently from
    other classes in Qt. A dialog is always a top-level widget, but if
    it has a parent, its default location is centered on top of the
    parent's top-level widget (if it is not top-level itself). It will
    also share the parent's taskbar entry.

    \target modal
    \section1 Modal Dialogs

    A \bold{modal} dialog is a dialog that blocks input to other
    visible windows in the same application. Users must finish
    interacting with the dialog and close it before they can access
    any other window in the application. Dialogs that are used to
    request a file name from the user or that are used to set
    application preferences are usually modal.

    The most common way to display a modal dialog is to call its
    exec() function. When the user closes the dialog, exec() will
    provide a useful \link #return return value\endlink. Typically,
    to get the dialog to close and return the appropriate value, we
    connect a default button, e.g. "OK", to the accept() slot and a
    "Cancel" button to the reject() slot.
    Alternatively you can call the done() slot with \c Accepted or
    \c Rejected.

    An alternative is to call setModal(true), then show(). Unlike
    exec(), show() returns control to the caller immediately. Calling
    setModal(true) is especially useful for progress dialogs, where
    the user must have the ability to interact with the dialog, e.g.
    to cancel a long running operation. If you use show() and
    setModal(true) together you must call
    QApplication::processEvents() periodically during processing to
    enable the user to interact with the dialog. (See \l
    QProgressDialog.)

    \target modeless
    \section1 Modeless Dialogs

    A \bold{modeless} dialog is a dialog that operates
    independently of other windows in the same application. Find and
    replace dialogs in word-processors are often modeless to allow the
    user to interact with both the application's main window and with
    the dialog.

    Modeless dialogs are displayed using show(), which returns control
    to the caller immediately.

    \target default
    \section1 Default button

    A dialog's \e default button is the button that's pressed when the
    user presses Enter (Return). This button is used to signify that
    the user accepts the dialog's settings and wants to close the
    dialog. Use QPushButton::setDefault(), QPushButton::isDefault()
    and QPushButton::autoDefault() to set and control the dialog's
    default button.

    \target escapekey
    \section1 Escape Key

    If the user presses the Esc key in a dialog, QDialog::reject()
    will be called. This will cause the window to close: the \link
    QCloseEvent close event \endlink cannot be \link
    QCloseEvent::ignore() ignored \endlink.

    \target extensibility
    \section1 Extensibility

    Extensibility is the ability to show the dialog in two ways: a
    partial dialog that shows the most commonly used options, and a
    full dialog that shows all the options. Typically an extensible
    dialog will initially appear as a partial dialog, but with a
    "More" toggle button. If the user presses the "More" button down,
    the full dialog will appear. The extension widget will be resized
    to its sizeHint(). If orientation is \c Qt::Horizontal the extension
    widget's height() will be expanded to the height() of the dialog.
    If the orientation is \c Qt::Vertical the extension widget's width()
    will be expanded to the width() of the dialog. Extensibility is
    controlled with setExtension(), setOrientation() and
    showExtension().

    \target return
    \section1 Return Value (Modal Dialogs)

    Modal dialogs are often used in situations where a return value is
    required, e.g. to indicate whether the user pressed "OK" or
    "Cancel". A dialog can be closed by calling the accept() or the
    reject() slots, and exec() will return \c Accepted or \c Rejected
    as appropriate. The exec() call returns the result of the dialog.
    The result is also available from result() if the dialog has not
    been destroyed.

    \target examples
    \section1 Code Examples

    A modal dialog:

    \quotefunction snippets/dialogs/dialogs.cpp void EditorWindow::countWords()

    A modeless dialog:

    \quotefunction snippets/dialogs/dialogs.cpp void EditorWindow::find()

    \sa QTabDialog, QWidget, QProgressDialog,
        \link guibooks.html#fowler GUI Design Handbook: Dialogs, Standard\endlink
*/

/*! \enum QDialog::DialogCode

    The value returned by a modal dialog.

    \value Accepted
    \value Rejected
*/

/*!
  \property QDialog::sizeGripEnabled
  \brief whether the size grip is enabled

  A QSizeGrip is placed in the bottom-right corner of the dialog when this
  property is enabled. By default, the size grip is disabled.
*/

#define d d_func()
#define q q_func()


/*!
  Constructs a dialog with parent \a parent.

  A dialog is always a top-level widget, but if it has a parent, its
  default location is centered on top of the parent. It will also
  share the parent's taskbar entry.

  The widget flags \a f are passed on to the QWidget constructor.
  If, for example, you don't want a What's This button in the title bar
  of the dialog, pass Qt::WStyle_Customize | Qt::WStyle_NormalBorder |
  Qt::WStyle_Title | Qt::WStyle_SysMenu in \a f.

  \sa QWidget::setWFlags()
*/

QDialog::QDialog(QWidget *parent, Qt::WFlags f)
    : QWidget(*new QDialogPrivate, parent, f | Qt::WType_Dialog)
{
}

#ifdef QT3_SUPPORT
/*!
    \overload
    \obsolete
*/
QDialog::QDialog(QWidget *parent, const char *name, bool modal, Qt::WFlags f)
    : QWidget(*new QDialogPrivate, parent,
              (modal ? (f|Qt::WShowModal) : f) | Qt::WType_Dialog)
{
    setObjectName(name);
}
#endif

/*!
  \overload
  \internal
*/
QDialog::QDialog(QDialogPrivate &dd, QWidget *parent, Qt::WFlags f)
    : QWidget(dd, parent, f | Qt::WType_Dialog)
{
}

/*!
  Destroys the QDialog, deleting all its children.
*/

QDialog::~QDialog()
{
    // Need to hide() here, as our (to-be) overridden hide()
    // will not be called in ~QWidget.
    hide();
}

/*!
  \internal
  This function is called by the push button \a pushButton when it
  becomes the default button. If \a pushButton is 0, the dialogs
  default default button becomes the default button. This is what a
  push button calls when it loses focus.
*/
void QDialogPrivate::setDefault(QPushButton *pushButton)
{
    bool hasMain = false;
    QList<QPushButton*> list = qFindChildren<QPushButton*>(q);
    for (int i=0; i<list.size(); ++i) {
        QPushButton *pb = list.at(i);
        if (pb->topLevelWidget() == q) {
            if (pb == d->mainDef)
                hasMain = true;
            if (pb != pushButton)
                pb->setDefault(false);
        }
    }
    if (!pushButton && hasMain)
        d->mainDef->setDefault(true);
    if (!hasMain)
        d->mainDef = pushButton;
}

/*!
  \internal
  This function sets the default default pushbutton to \a pushButton.
  This function is called by QPushButton::setDefault().
*/
void QDialogPrivate::setMainDefault(QPushButton *pushButton)
{
    d->mainDef = 0;
    setDefault(pushButton);
}

/*!
  \internal
  Hides the default button indicator. Called when non auto-default
  push button get focus.
 */
void QDialogPrivate::hideDefault()
{
    QList<QPushButton*> list = qFindChildren<QPushButton*>(q);
    for (int i=0; i<list.size(); ++i) {
        list.at(i)->setDefault(false);
    }
}

#ifdef Q_OS_TEMP
# include "qmessagebox.h"
extern const char * mb_texts[]; // Defined in qmessagebox.cpp
/*!
  \internal
  Hides special buttons which are rather shown in the title bar
  on WinCE, to conserve screen space.
*/
void QDialog::hideSpecial()
{
    // "OK"     buttons are hidden, and (Ok) shown on title bar
    // "Cancel" buttons are hidden, and (X)  shown on title bar
    // "Help"   buttons are hidden, and (?)  shown on title bar
    bool showOK = false,
         showX  = false,
         showQ  = false;
    QList<QPushButton*> list = qFindChildren<QPushButton*>(this);
    for (int i=0; i<list.size(); ++i) {
        QPushButton *pb = list.at(i);
        if (!showOK && pb->text() == qApp->translate("QMessageBox", mb_texts[QMessageBox::Ok])) {
            pb->hide();
            showOK = true;
        } else if (!showX && pb->text() == qApp->translate("QMessageBox", mb_texts[QMessageBox::Cancel])) {
            pb->hide();
            showX = true;
        } else if (!showQ && pb->text() == qApp->tr("Help")) {
            pb->hide();
            showQ = true;
        }
    }
    if (showOK || showQ) {
        DWORD ext = GetWindowLong(winId(), GWL_EXSTYLE);
        ext |= showOK ? WS_EX_CAPTIONOKBTN : 0;
        ext |= showQ  ? WS_EX_CONTEXTHELP: 0;
        SetWindowLong(winId(), GWL_EXSTYLE, ext);
    }
    if (!showX) {
        DWORD ext = GetWindowLong(winId(), GWL_STYLE);
        ext &= ~WS_SYSMENU;
        SetWindowLong(winId(), GWL_STYLE, ext);
    }
}
#endif

/*!
  Returns the modal dialog's result code, \c Accepted or \c Rejected.

  Do not call this function if the dialog was constructed with the \c
  Qt::WA_DeleteOnClose flag.
*/
int QDialog::result() const
{
    return d->rescode;
}

/*!
  \fn void QDialog::setResult(int i)

  Sets the modal dialog's result code to \a i.
*/
void QDialog::setResult(int r)
{
    d->rescode = r;
}


/*!
    Shows the dialog as a \link #modal modal \endlink dialog,
    blocking until the user closes it. The function returns a \l
    DialogCode result.

    Users cannot interact with any other window in the same
    application until they close the dialog.

  \sa show(), result()
*/

int QDialog::exec()
{
    if (d->eventLoop) {
        qWarning("QDialog::exec: Recursive call detected");
        return -1;
    }

    bool deleteOnClose = testAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_DeleteOnClose, false);

    bool wasShowModal = testWFlags(Qt::WShowModal);
    setWFlags(Qt::WShowModal);
    setResult(0);

    show();

    QEventLoop eventLoop;
    d->eventLoop = &eventLoop;
    (void) eventLoop.exec();
    d->eventLoop = 0;

    if (!wasShowModal)
        clearWFlags(Qt::WShowModal);

    int res = result();
    if (deleteOnClose)
        delete this;
    return res;
}


/*! Closes the dialog and sets its result code to \a r. If this dialog
  is shown with exec(), done() causes the local event loop to finish,
  and exec() to return \a r.

  As with QWidget::close(), done() deletes the dialog if the \c
  Qt::WA_DeleteOnClose flag is set. If the dialog is the application's
  main widget, the application terminates. If the dialog is the
  last window closed, the QApplication::lastWindowClosed() signal is
  emitted.

  \sa accept(), reject(), QApplication::mainWidget(), QApplication::quit()
*/

void QDialog::done(int r)
{
    hide();
    setResult(r);
    d->close_helper(QWidgetPrivate::CloseNoEvent);
}

/*!
  Hides the modal dialog and sets the result code to \c Accepted.

  \sa reject() done()
*/

void QDialog::accept()
{
    done(Accepted);
}

/*!
  Hides the modal dialog and sets the result code to \c Rejected.

  \sa accept() done()
*/

void QDialog::reject()
{
    done(Rejected);
}

/*! \reimp */
bool QDialog::eventFilter(QObject *o, QEvent *e)
{
    return QWidget::eventFilter(o, e);
}

/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*! \reimp */
void QDialog::contextMenuEvent(QContextMenuEvent *e)
{
#if !defined(QT_NO_WHATSTHIS) && !defined(QT_NO_MENU)
    QWidget *w = childAt(e->pos());
    if (!w) {
        w = rect().contains(e->pos()) ? this : 0;
        if (!w)
            return;
    }
    while (w && w->whatsThis().size() == 0 && !w->testAttribute(Qt::WA_CustomWhatsThis))
        w = w->isTopLevel() ? 0 : w->parentWidget();
    if (w) {
        QMenu p;
        QAction *wt = p.addAction(tr("What's This?"));
        if (p.exec(e->globalPos()) == wt) {
            QHelpEvent e(QEvent::WhatsThis, w->rect().center(),
                         w->mapToGlobal(w->rect().center()));
            QApplication::sendEvent(w, &e);
        }
    }
#endif
}

/*! \reimp */
void QDialog::keyPressEvent(QKeyEvent *e)
{
    //   Calls reject() if Escape is pressed. Simulates a button
    //   click for the default button if Enter is pressed. Move focus
    //   for the arrow keys. Ignore the rest.
#ifdef Q_WS_MAC
    if(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period) {
        reject();
    } else
#endif
    if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return: {
            QList<QPushButton*> list = qFindChildren<QPushButton*>(this);
            for (int i=0; i<list.size(); ++i) {
                QPushButton *pb = list.at(i);
                if (pb->isDefault() && pb->isVisible()) {
                    if (pb->isEnabled())
                        pb->click();
                    return;
                }
            }
        }
        break;
        case Qt::Key_Escape:
            reject();
            break;
        case Qt::Key_Up:
        case Qt::Key_Left:
            if (focusWidget() &&
                 (focusWidget()->focusPolicy() == Qt::StrongFocus ||
                   focusWidget()->focusPolicy() == Qt::WheelFocus)) {
                e->ignore();
                break;
            }
            // call ours, since c++ blocks us from calling the one
            // belonging to focusWidget().
            focusNextPrevChild(false);
            break;
        case Qt::Key_Down:
        case Qt::Key_Right:
            if (focusWidget() &&
                 (focusWidget()->focusPolicy() == Qt::StrongFocus ||
                   focusWidget()->focusPolicy() == Qt::WheelFocus)) {
                e->ignore();
                break;
            }
            focusNextPrevChild(true);
            break;
        default:
            e->ignore();
            return;
        }
    } else {
        e->ignore();
    }
}

/*! \reimp */
void QDialog::closeEvent(QCloseEvent *e)
{
#ifndef QT_NO_WHATSTHIS
    if (isModal() && QWhatsThis::inWhatsThisMode())
        QWhatsThis::leaveWhatsThisMode();
#endif
    if (isShown())
        reject();
    if (isHidden())
        e->accept();
}

#ifdef Q_OS_TEMP
/*! \internal
    \reimp
*/
bool QDialog::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::OkRequest:
    case QEvent::HelpRequest: {
        QString bName =
            (e->type() == QEvent::OkRequest)
            ? qApp->translate("QMessageBox", mb_texts[QMessageBox::Ok])
            : qApp->tr("Help");
        QList<QPushButton*> list = qFindChildren<QPushButton*>(this);
        for (int i=0; i<list.size(); ++i) {
            QPushButton *pb = list.at(i);
            if (pb->text() == bName) {
                if (pb->isEnabled())
                    pb->click();
                return pb->isEnabled();
            }
        } }
    default: break;
    }
    return QWidget::event(e);
}
#endif


/*****************************************************************************
  Geometry management.
 *****************************************************************************/

/*!
    Shows the dialog as a \link #modeless modeless \endlink dialog.
    Control returns immediately to the calling code.

    The dialog will be modal or modeless according to the value
    of the \l modal property.

    \sa exec(), modal
*/

void QDialog::show()
{
    if (testWState(Qt::WState_Visible))
        return;

#ifdef Q_OS_TEMP
    hideSpecial();
#endif

    if (!testAttribute(Qt::WA_Moved)) {
        Qt::WindowStates state = windowState();
        adjustPosition(parentWidget());
        setAttribute(Qt::WA_Moved, false); // not really an explicit position
        if (state != windowState())
            setWindowState(state);
    }
    QWidget::show();
    showExtension(d->doShowExtension);
    QWidget *fw = topLevelWidget()->focusWidget();
    if (!fw)
        fw = this;

    /*
      The following block is to handle a special case, and does not
      really follow propper logic in concern of autoDefault and TAB
      order. However, it's here to ease usage for the users. If a
      dialog has a default QPushButton, and first widget in the TAB
      order also is a QPushButton, then we give focus to the main
      default QPushButton. This simplifies code for the developers,
      and actually catches most cases... If not, then they simply
      have to use [widget*]->setFocus() themselves...
    */
    if (d->mainDef && fw->focusPolicy() == Qt::NoFocus) {
        QWidget *first = fw;
        while ((first = first->nextInFocusChain()) != fw && first->focusPolicy() == Qt::NoFocus)
            ;
        if (first != d->mainDef && qt_cast<QPushButton*>(first))
            d->mainDef->setFocus();
    }
    if (!d->mainDef && isTopLevel()) {
        QWidget *w = fw;
        while ((w = w->nextInFocusChain()) != fw) {
            QPushButton *pb = qt_cast<QPushButton *>(w);
            if (pb && pb->autoDefault() && pb->focusPolicy() != Qt::NoFocus) {
                pb->setDefault(true);
                break;
            }
        }
    }
    if (fw) {
        QFocusEvent e(QEvent::FocusIn, Qt::TabFocusReason);
        QApplication::sendEvent(fw, &e);
    }

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::DialogStart);
#endif
}

/*!\reimp */
void QDialog::showEvent(QShowEvent *event)
{
    if (!event->spontaneous() && !testAttribute(Qt::WA_Moved)) {
	Qt::WindowStates  state = windowState();
        adjustPosition(parentWidget());
        setAttribute(Qt::WA_Moved, false); // not really an explicit position
	if (state != windowState())
	    setWindowState(state);
    }
}

/*! \internal */
void QDialog::adjustPosition(QWidget* w)
{
    QPoint p(0, 0);
    int extraw = 0, extrah = 0, scrn = 0;
    if (w)
        w = w->topLevelWidget();
    QRect desk;
    if (w) {
        scrn = QApplication::desktop()->screenNumber(w);
    } else if (QApplication::desktop()->isVirtualDesktop()) {
        scrn = QApplication::desktop()->screenNumber(QCursor::pos());
    } else {
        scrn = QApplication::desktop()->screenNumber(this);
    }
    desk = QApplication::desktop()->availableGeometry(scrn);

    QWidgetList list = QApplication::topLevelWidgets();
    for (int i = 0; (extraw == 0 || extrah == 0) && i < list.size(); ++i) {
        QWidget * current = list.at(i);
        if (current->isVisible()) {
            int framew = current->geometry().x() - current->x();
            int frameh = current->geometry().y() - current->y();

            extraw = qMax(extraw, framew);
            extrah = qMax(extrah, frameh);
        }
    }

    // sanity check for decoration frames. With embedding, we
    // might get extraordinary values
    if (extraw == 0 || extrah == 0 || extraw >= 10 || extrah >= 40) {
        extrah = 40;
        extraw = 10;
    }

#ifndef Q_OS_TEMP
    if (w) {
        // Use mapToGlobal rather than geometry() in case w might
        // be embedded in another application
        QPoint pp = w->mapToGlobal(QPoint(0,0));
        p = QPoint(pp.x() + w->width()/2,
                    pp.y() + w->height()/ 2);
    } else {
        // p = middle of the desktop
        p = QPoint(desk.x() + desk.width()/2, desk.y() + desk.height()/2);
    }
#else
    p = QPoint(desk.x() + desk.width()/2, desk.y() + desk.height()/2);
#endif

    // p = origin of this
    p = QPoint(p.x()-width()/2 - extraw,
                p.y()-height()/2 - extrah);


    if (p.x() + extraw + width() > desk.x() + desk.width())
        p.setX(desk.x() + desk.width() - width() - extraw);
    if (p.x() < desk.x())
        p.setX(desk.x());

    if (p.y() + extrah + height() > desk.y() + desk.height())
        p.setY(desk.y() + desk.height() - height() - extrah);
    if (p.y() < desk.y())
        p.setY(desk.y());

    move(p);
}


/*! \reimp */
void QDialog::hide()
{
    if (isHidden())
        return;

#ifndef QT_NO_ACCESSIBILITY
    if (isVisible())
        QAccessible::updateAccessibility(this, 0, QAccessible::DialogEnd);
#endif

    // Reimplemented to exit a modal event loop when the dialog is hidden.
    QWidget::hide();
    if (d->eventLoop)
        d->eventLoop->exit();
}


/*!
    If \a orientation is \c Qt::Horizontal, the extension will be displayed
    to the right of the dialog's main area. If \a orientation is \c
    Qt::Vertical, the extension will be displayed below the dialog's main
    area.

  \sa orientation(), setExtension()
*/
void QDialog::setOrientation(Qt::Orientation orientation)
{
    d->orientation = orientation;
}

/*!
  Returns the dialog's extension orientation.

  \sa setOrientation()
*/
Qt::Orientation QDialog::orientation() const
{
    return d->orientation;
}

/*!
    Sets the widget, \a extension, to be the dialog's extension,
    deleting any previous extension. The dialog takes ownership of the
    extension. Note that if 0 is passed any existing extension will be
    deleted.

  This function must only be called while the dialog is hidden.

  \sa showExtension(), setOrientation(), extension()
 */
void QDialog::setExtension(QWidget* extension)
{
    delete d->extension;
    d->extension = extension;

    if (!extension)
        return;

    if (extension->parentWidget() != this)
        extension->setParent(this);
    extension->hide();
}

/*!
  Returns the dialog's extension or 0 if no extension has been
  defined.

  \sa setExtension()
 */
QWidget* QDialog::extension() const
{
    return d->extension;
}


/*!
  If \a showIt is true, the dialog's extension is shown; otherwise the
  extension is hidden.

  This slot is usually connected to the \l QPushButton::toggled() signal
  of a QPushButton.

  A dialog with a visible extension is not resizeable.

  \sa show(), setExtension(), setOrientation()
 */
void QDialog::showExtension(bool showIt)
{
    d->doShowExtension = showIt;
    if (!d->extension)
        return;
    if (!testWState(Qt::WState_Visible))
        return;
    if (d->extension->isVisible() == showIt)
        return;

    if (showIt) {
        d->size = size();
        d->min = minimumSize();
        d->max = maximumSize();
#ifndef QT_NO_LAYOUT
        if (layout())
            layout()->setEnabled(false);
#endif
        QSize s(d->extension->sizeHint()
                 .expandedTo(d->extension->minimumSize())
                 .boundedTo(d->extension->maximumSize()));
        if (d->orientation == Qt::Horizontal) {
            int h = qMax(height(), s.height());
            d->extension->setGeometry(width(), 0, s.width(), h);
            setFixedSize(width() + s.width(), h);
        } else {
            int w = qMax(width(), s.width());
            d->extension->setGeometry(0, height(), w, s.height());
            setFixedSize(w, height() + s.height());
        }
        d->extension->show();
    } else {
        d->extension->hide();
        // workaround for CDE window manager that won't shrink with (-1,-1)
        setMinimumSize(d->min.expandedTo(QSize(1, 1)));
        setMaximumSize(d->max);
        resize(d->size);
#ifndef QT_NO_LAYOUT
        if (layout())
            layout()->setEnabled(true);
#endif
    }
}


/*! \reimp */
QSize QDialog::sizeHint() const
{
    if (d->extension)
        if (d->orientation == Qt::Horizontal)
            return QSize(QWidget::sizeHint().width(),
                        qMax(QWidget::sizeHint().height(),d->extension->sizeHint().height()));
        else
            return QSize(qMax(QWidget::sizeHint().width(), d->extension->sizeHint().width()),
                        QWidget::sizeHint().height());

    return QWidget::sizeHint();
}


/*! \reimp */
QSize QDialog::minimumSizeHint() const
{
    if (d->extension)
        if (d->orientation == Qt::Horizontal)
            return QSize(QWidget::minimumSizeHint().width(),
                        qMax(QWidget::minimumSizeHint().height(), d->extension->minimumSizeHint().height()));
        else
            return QSize(qMax(QWidget::minimumSizeHint().width(), d->extension->minimumSizeHint().width()),
                        QWidget::minimumSizeHint().height());

    return QWidget::minimumSizeHint();
}

/*! \property QDialog::modal
    \brief whether show() should pop up the dialog as modal or modeless

    By default, this property is false and show() pops up the dialog as
    modeless.

    exec() ignores the value of this property and always pops up the
    dialog as modal.

    \sa show(), exec()
*/

void QDialog::setModal(bool modal)
{
    if (modal)
        setWFlags(Qt::WShowModal);
    else
        clearWFlags(Qt::WShowModal);
}


bool QDialog::isSizeGripEnabled() const
{
#ifndef QT_NO_SIZEGRIP
    return !!d->resizer;
#else
    return false;
#endif
}


void QDialog::setSizeGripEnabled(bool enabled)
{
#ifndef QT_NO_SIZEGRIP
    if (!enabled != !d->resizer) {
        if (enabled) {
            d->resizer = new QSizeGrip(this);
            // adjustSize() processes all events, which is suboptimal
            d->resizer->resize(d->resizer->sizeHint());
            if (isRightToLeft())
                d->resizer->move(rect().bottomLeft() -d->resizer->rect().bottomLeft());
            else
                d->resizer->move(rect().bottomRight() -d->resizer->rect().bottomRight());
            d->resizer->raise();
            d->resizer->show();
        } else {
            delete d->resizer;
            d->resizer = 0;
        }
    }
#endif //QT_NO_SIZEGRIP
}



/*! \reimp */
void QDialog::resizeEvent(QResizeEvent *)
{
#ifndef QT_NO_SIZEGRIP
    if (d->resizer) {
        if (isRightToLeft())
            d->resizer->move(rect().bottomLeft() -d->resizer->rect().bottomLeft());
        else
            d->resizer->move(rect().bottomRight() -d->resizer->rect().bottomRight());
        d->resizer->raise();
    }
#endif
}

#endif // QT_NO_DIALOG
