/****************************************************************************
** $Id: $
**
** Implementation of QDialog class
**
** Created : 950502
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qdialog.h"

#ifndef QT_NO_DIALOG

#include "qpushbutton.h"
#include "qapplication.h"
#include "qobjectlist.h"
#include "qwidgetlist.h"
#include "qlayout.h"
#include "qsizegrip.h"
#include "qwhatsthis.h"
#include "qpopupmenu.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif
#if defined( Q_OS_TEMP )
#include "qt_windows.h"
#endif

/*!
  \class QDialog qdialog.h
  \brief The QDialog class is the base class of dialog windows.

  \ingroup dialogs
  \ingroup abstractwidgets
  \mainclass

  A dialog window is a top-level window mostly used for short-term tasks
  and brief communications with the user. QDialogs may be modal or
  modeless. QDialogs can have \link #default default buttons\endlink,
  support \link #extensibility extensibility\endlink and may provide a
  \link #return return value\endlink. QDialogs can have a QSizeGrip in
  their lower-right corner, using setSizeGripEnabled().

    Note that QDialog uses the parent widget slightly differently from
    other classes in Qt.  A dialog is always a top-level widget, but if
    it has a parent, its default location is centered on top of the
    parent. It will also share the parent's taskbar entry, for example.

  There are three kinds of dialog that are useful:
    \list 1
  \i A <b>modal</b> dialog is a dialog that blocks input to other visible
  windows in the same application: users must finish interacting
  with the dialog and close it before they can access any other window
  in the application. Modal dialogs have their own local event loop.
  Dialogs which are used to request a filename from the user or which
  are used to set application preferences are usually modal. Call exec()
  to display a modal dialog. When the user closes the dialog, exec()
  will provide a useful \link #return return value\endlink, and the flow
  of control will follow on from the exec() call at this time. Typically
  we connect a default button, e.g. "OK", to the accept() slot and a
  "Cancel" button to the reject() slot, to get the dialog to close and
  return the appropriate value. Alternatively you can connect to the
  done() slot, passing it \c Accepted or \c Rejected.

  \i A <b>modeless</b> dialog is a dialog that operates independently of
  other windows in the same application.  Find and replace dialogs in
  word-processors are often modeless to allow the user to interact with
  both the application's main window and the dialog. Call show() to
  display a modeless dialog. show() returns immediately so the flow of
  control will continue in the calling code. In practice you will often
  call show() and come to the end of the function in which show() is
  called with control returning to the main event loop.

  \i <a name="semimodal">A "<b>semi-modal</b>" dialog is a modal dialog
  that returns control to the caller immediately. Semi-modal dialogs do
  not have their own event loop, so you will need to call
  QApplication::processEvents() periodically to give the semi-modal
  dialog the opportunity to process its events. A progress dialog
  (e.g. QProgressDialog) is an example, where you only want the user to
  be able to interact with the progress dialog, e.g. to cancel a long
  running operation, but need to actually carry out the operation.
  Semi-modal dialogs are displayed by setting the modal flag to TRUE and
  calling the \l show() function.
  \endlist

    <a name="default"><b>Default button</b><br>
  A dialog's "default" button is the button that's pressed when the user
  presses Enter or Return. This button is used to signify that the user
  accepts the dialog's settings and wishes to close the dialog. Use
  QPushButton::setDefault(), QPushButton::isDefault() and
  QPushButton::autoDefault() to set and control the dialog's default
  button.

    <a name="extensibility"><b>Extensibility</b><br>
  Extensibility is the ability to show the dialog in two ways: a partial
  dialog that shows the most commonly used options, and a full dialog
  that shows all the options. Typically an extensible dialog
  will initially appear as a partial dialog, but with a "More"
  button. If the user clicks the "More" button, the full dialog will
  appear. Extensibility is controlled with setExtension(),
  setOrientation() and showExtension().

    <a name="return"><b>Return value (modal dialogs)</b><br>
    Modal dialogs are often used in situations where a return value is
    required; for example to indicate whether the user pressed OK or
    Cancel. A dialog can be closed by calling the accept() or the
    reject() slots, and exec() will return \c Accepted or \c Rejected as
    appropriate. After the exec() call has returned the result is
    available from result().


    <a name="examples"><b>Examples</b><br>

    A modal dialog.

    \quotefile network/networkprotocol/view.cpp
    \skipto QFileDialog *dlg
    \printuntil return

    A modeless dialog. After the show() call, control returns to the main
    event loop.
    \quotefile life/main.cpp
    \skipto argv
    \printuntil QApplication
    \skipto scale
    \printline
    \skipto LifeDialog
    \printuntil show
    \skipto exec
    \printuntil }

    See the \l QProgressDialog documentation for an example of a
    semi-modal dialog.

    \sa QTabDialog QWidget QProgressDialog
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

  A QSizeGrip is placed in the bottom right corner of the dialog when this
  property is enabled.  By default, the size grip is disabled.
*/

class QDialogPrivate : public Qt
{
public:

    QDialogPrivate()
	: mainDef(0), orientation(Horizontal),extension(0)
#ifndef QT_NO_SIZEGRIP
	,resizer(0)
#endif
	{
    }

    QPushButton* mainDef;
    Orientation orientation;
    QWidget* extension;
    QSize size, min, max;
#ifndef QT_NO_SIZEGRIP
    QSizeGrip* resizer;
#endif
    QPoint lastRMBPress;
    QPoint relPos; // relative position to the main window
};

/*!
  Constructs a dialog called \a name, with parent \a parent.

  If \a modal is FALSE (the default), the dialog is modeless and should
  be displayed with show(). If \a modal is TRUE and the dialog is
  displayed with exec(), the dialog is modal, i.e. blocks input to other
  windows. If \a modal is TRUE and the dialog is displayed show(), the
  dialog is semi-modal.

  The widget flags \a f are passed on to the QWidget constructor.

  If, for example, you don't want a What's this button in the titlebar
  of the dialog, pass WStyle_Customize | WStyle_NormalBorder |
  WStyle_Title | WStyle_SysMenu in \a f.

  We recommend that you always pass a non-null parent.

  \sa QWidget::setWFlags() Qt::WidgetFlags
*/

QDialog::QDialog( QWidget *parent, const char *name, bool modal, WFlags f )
    : QWidget( parent, name,
	       (modal ? (f|WShowModal) : f) | WType_Dialog )
{
    rescode = 0;
    did_move = FALSE;
    did_resize = FALSE;
    has_relpos = FALSE;
    in_loop = FALSE;
    d = new QDialogPrivate;
}

/*!
  Destroys the QDialog, deleting all its children.
*/

QDialog::~QDialog()
{
    // Need to hide() here, as our (to-be) overridden hide()
    // will not be called in ~QWidget.
    hide();
    delete d;
}

/*!
  \internal
  This function is called by the push button \a pushButton when it
  becomes the default button. If \a pushButton is 0, the dialogs
  default default button becomes the default button. This is what a
  push button calls when it loses focus.
*/

void QDialog::setDefault( QPushButton *pushButton )
{
#ifndef QT_NO_PUSHBUTTON
    QObjectList *list = queryList( "QPushButton" );
    Q_ASSERT(list);
    QObjectListIt it( *list );
    QPushButton *pb;
    bool hasMain = FALSE;
    while ( (pb = (QPushButton*)it.current()) ) {
	++it;
	if ( pb->topLevelWidget() != this )
	    continue;
	if ( pb == d->mainDef )
	    hasMain = TRUE;
	if ( pb != pushButton )
	    pb->setDefault( FALSE );
    }
    if (!pushButton && hasMain)
	d->mainDef->setDefault( TRUE );
    if (!hasMain)
	d->mainDef = pushButton;
    delete list;
#endif
}

/*!
  \internal
  Hides the default button indicator. Called when non auto-default
  push button get focus.
 */
void QDialog::hideDefault()
{
#ifndef QT_NO_PUSHBUTTON
    QObjectList *list = queryList( "QPushButton" );
    QObjectListIt it( *list );
    QPushButton *pb;
    while ( (pb = (QPushButton*)it.current()) ) {
	++it;
	pb->setDefault( FALSE );
    }
    delete list;
#endif
}


/*!
  \fn int  QDialog::result() const

  Returns the modal dialog's result code, \c Accepted or \c Rejected.
*/

/*!
  \fn void  QDialog::setResult( int i )

  Sets the modal dialog's result code to \a i.
*/


/*!
    Executes a modal dialog. Control passes to the dialog until the user
    closes it, at which point the local event loop finishes and the
    function returns with the \l DialogCode result.
    Users will not be able to interact with any other window in
    the same application until they close this dialog. For a modeless or
    semi-modal dialog use show().

  \sa show(), result()
*/

int QDialog::exec()
{
    if ( in_loop ) {
	qWarning( "QDialog::exec: Recursive call detected." );
	return -1;
    }

    bool destructiveClose = testWFlags( WDestructiveClose );
    clearWFlags( WDestructiveClose );

    bool wasShowModal = testWFlags( WShowModal );
    setWFlags( WShowModal );
    setResult( 0 );

    show();

    in_loop = TRUE;
    qApp->enter_loop();

    if ( !wasShowModal )
	clearWFlags( WShowModal );

    int res = result();

    if ( destructiveClose )
      delete this;

    return res;
}


/*! Hides the modal dialog and sets its result code to \a r. This
  uses the local event loop to finish, and exec() to return \a r.

  If the dialog has the \c WDestructiveClose flag set, done() also
  deletes the dialog. If the dialog is the applications's main widget,
  the application quits.

  \sa accept(), reject(), QApplication::mainWidget(), QApplication::quit()
*/

void QDialog::done( int r )
{
    hide();
    setResult( r );

    // We cannot use close() here, as close() calls closeEvent() calls
    // reject() calls close().  But we can at least keep the
    // mainWidget() and WDestructiveClose semantics. There should not
    // be much of a difference whether the users types Alt-F4 or
    // Escape. Without that, destructive-close dialogs were more or
    // less useless without subclassing.
    if ( qApp->mainWidget() == this )
	qApp->quit();

    if ( testWFlags(WDestructiveClose) )
	delete this;
}

/*!
  Hides the modal dialog and sets the result code to \c Accepted.
  \sa reject() done()
*/

void QDialog::accept()
{
    done( Accepted );
}

/*!
  Hides the modal dialog and sets the result code to \c Rejected.
  \sa accept() done()
*/

void QDialog::reject()
{
    done( Rejected );
}

/*! \reimp */
bool QDialog::eventFilter( QObject *o, QEvent *e )
{
    return QWidget::eventFilter( o, e );
}

/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*! \reimp */
void QDialog::contextMenuEvent( QContextMenuEvent *e )
{
#if !defined(QT_NO_WHATSTHIS) && !defined(QT_NO_POPUPMENU)
    QWidget* w = childAt( e->pos(), TRUE );
    if ( !w )
	return;
    QString s = QWhatsThis::textFor( w, e->pos(), TRUE );
    if ( !s.isEmpty() ) {
	QPopupMenu p(0,"qt_whats_this_menu");
	p.insertItem( tr("What's This?"), 42 );
	if ( p.exec( e->globalPos() ) >= 42 )
	    QWhatsThis::display( s, w->mapToGlobal( w->rect().center() ) );
    }
#endif
}

/*! \reimp */
void QDialog::keyPressEvent( QKeyEvent *e )
{
    //   Calls reject() if Escape is pressed.  Simulates a button
    //   click for the default button if Enter is pressed.  Move focus
    //   for the arrow keys.  Ignore the rest.
    if ( e->state() == 0 || ( e->state() & Keypad && e->key() == Key_Enter ) ) {
	switch ( e->key() ) {
	case Key_Enter:
	case Key_Return: {
#ifndef QT_NO_PUSHBUTTON
	    QObjectList *list = queryList( "QPushButton" );
	    QObjectListIt it( *list );
	    QPushButton *pb;
	    while ( (pb = (QPushButton*)it.current()) ) {
		if ( pb->isDefault() && pb->isVisible() ) {
		    delete list;
		    if ( pb->isEnabled() ) {
			emit pb->clicked();
		    }
		    return;
		}
		++it;
	    }
	    delete list;
#endif
	}
	break;
	case Key_Escape:
	    reject();
	    break;
	case Key_Up:
	case Key_Left:
	    if ( focusWidget() &&
		 ( focusWidget()->focusPolicy() == QWidget::StrongFocus ||
		   focusWidget()->focusPolicy() == QWidget::WheelFocus ) ) {
		e->ignore();
		break;
	    }
	    // call ours, since c++ blocks us from calling the one
	    // belonging to focusWidget().
	    focusNextPrevChild( FALSE );
	    break;
	case Key_Down:
	case Key_Right:
	    if ( focusWidget() &&
		 ( focusWidget()->focusPolicy() == QWidget::StrongFocus ||
		   focusWidget()->focusPolicy() == QWidget::WheelFocus ) ) {
		e->ignore();
		break;
	    }
	    focusNextPrevChild( TRUE );
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
void QDialog::closeEvent( QCloseEvent *e )
{
#ifndef QT_NO_WHATSTHIS
    if ( isModal() && QWhatsThis::inWhatsThisMode() )
	QWhatsThis::leaveWhatsThisMode();
#endif
    e->accept();
    reject();
}


/*****************************************************************************
  Geometry management.
 *****************************************************************************/

/*!
    Shows a modeless or semi-modal dialog. Control returns immediately
    to the calling code.

    The dialog does not have a local event loop so you will need to call
    QApplication::processEvents() periodically to give the dialog the
    opportunity to process its events.

    The dialog will be \link #semimodal semi-modal\endlink if the modal
    flag was set to TRUE in the constructor.

  \warning

  In Qt 2.x, calling show() on a modal dialog enters a local event
  loop, and works like exec(), but doesn't return the result code
  exec() returns. Trolltech has always warned that doing this is unwise.

  \sa exec()
*/

void QDialog::show()
{
    if ( testWState(WState_Visible) )
	return;
#ifndef QT_NO_PUSHBUTTON
    if ( !d->mainDef ) {
	QObjectList *pbs = queryList( "QPushButton" );
	if ( pbs && !pbs->isEmpty() ) {
	    QObjectListIt it( *pbs );
	    QPushButton *button;
	    while ( ( button = (QPushButton*)it.current() ) ) {
		++it;
		if ( button->topLevelWidget() == this && button->autoDefault() ) {
		    button->setDefault( TRUE );
		    break;
		}
	    }
	}
	delete pbs;
    }
#endif
    if ( !did_resize )
	adjustSize();
    if ( has_relpos ) {
	if ( parentWidget() )
	    adjustPositionInternal( parentWidget(), TRUE );
    } else if ( !did_move ) {
	adjustPositionInternal( parentWidget() );
    }
    QWidget::show();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::DialogStart );
#endif
}


/*!\internal
 */
void QDialog::adjustPosition( QWidget* w) 
{
    adjustPositionInternal( w );
}

void QDialog::adjustPositionInternal( QWidget*w, bool useRelPos)
{
    QPoint p( 0, 0 );
    int extraw = 0, extrah = 0;
    if ( w )
	w = w->topLevelWidget();
    QRect desk = QApplication::desktop()->screenGeometry( QApplication::desktop()->screenNumber( w ? w : qApp->mainWidget() ) );

    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );
    while ( (extraw == 0 || extrah == 0) &&
	    it.current() != 0 ) {
	int w, h;
	QWidget * current = it.current();
	++it;
	w = current->geometry().x() - current->x();
	h = current->geometry().y() - current->y();

	extraw = QMAX( extraw, w );
	extrah = QMAX( extrah, h );
    }
    delete list;

    // sanity check for decoration frames. With embedding, we
    // might get extraordinary values
    if ( extraw >= 10 || extrah >= 40 )
	extraw = extrah = 0;

    if ( useRelPos ) {
	p = w->pos() + d->relPos;
	
    } else {
	if ( w ) {
	    // Use mapToGlobal rather than geometry() in case w might
	    // be embedded in another application
	    QPoint pp = w->mapToGlobal( QPoint(0,0) );
	    p = QPoint( pp.x() + w->width()/2,
			pp.y() + w->height()/ 2 );
	} else {
	    // p = middle of the desktop
	    p = QPoint( desk.x() + desk.width()/2, desk.y() + desk.height()/2 );
	}
#if defined( Q_OS_TEMP )
	p = QPoint( 0, GetSystemMetric( SM_CYCAPTION ) );
#endif

	// p = origin of this
	p = QPoint( p.x()-width()/2 - extraw,
		    p.y()-height()/2 - extrah );
    }
    

    if ( p.x() + extraw + width() > desk.x() + desk.width() )
	p.setX( desk.x() + desk.width() - width() - extraw );
    if ( p.x() < desk.x() )
	p.setX( desk.x() );

    if ( p.y() + extrah + height() > desk.y() + desk.height() )
	p.setY( desk.y() + desk.height() - height() - extrah );
    if ( p.y() < desk.y() )
	p.setY( desk.y() );

    move( p );
}


/*! \reimp */
void QDialog::hide()
{
    if ( isHidden() )
	return;

#if defined(QT_ACCESSIBILITY_SUPPORT)
    if ( isVisible() )
	QAccessible::updateAccessibility( this, 0, QAccessible::DialogEnd );
#endif

    if ( parentWidget() ) {
	d->relPos = pos() - parentWidget()->topLevelWidget()->pos();
	has_relpos = 1;
	did_move = 0;
    }

    // Reimplemented to exit a modal when the dialog is hidden.
    QWidget::hide();
    if ( in_loop ) {
	in_loop = FALSE;
	qApp->exit_loop();
    }
}


/*****************************************************************************
  Detects any widget geometry changes done by the user.
 *****************************************************************************/

/*! \reimp */

void QDialog::move( int x, int y )
{
    did_move = TRUE;
    QWidget::move( x, y );
}

/*! \reimp */

void QDialog::move( const QPoint &p )
{
    did_move = TRUE;
    QWidget::move( p );
}

/*! \reimp */

void QDialog::resize( int w, int h )
{
    did_resize = TRUE;
    QWidget::resize( w, h );
}

/*! \reimp */

void QDialog::resize( const QSize &s )
{
    did_resize = TRUE;
    QWidget::resize( s );
}

/*! \reimp */

void QDialog::setGeometry( int x, int y, int w, int h )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( x, y, w, h );
}

/*! \reimp */

void QDialog::setGeometry( const QRect &r )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( r );
}


/*!
    If \a orientation is \c Horizontal, the extension will be displayed
    to the right of the dialog's main area. If \a orientation is \c
    Vertical, the extension will be displayed below the dialog's main
    are.

  \sa orientation(), setExtension()
*/
void QDialog::setOrientation( Orientation orientation )
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
void QDialog::setExtension( QWidget* extension )
{
    delete d->extension;
    d->extension = extension;

    if ( !extension )
	return;

    if ( extension->parentWidget() != this )
	extension->reparent( this, QPoint(0,0) );
    else
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
  If \a showIt is TRUE, the dialog's extension is shown; otherwise the
  extension is hidden.

  This slot is usually connected to the \l QButton::toggled() signal
  of a QPushButton.

  If the dialog is not visible, or has no extension, nothing happens.

  \sa show(), setExtension(), setOrientation()
 */
void QDialog::showExtension( bool showIt )
{
    if ( !d->extension )
	return;
    if ( !testWState(WState_Visible) )
	return;

    if ( showIt ) {
	d->size = size();
	d->min = minimumSize();
	d->max = maximumSize();
#ifndef QT_NO_LAYOUT
	if ( layout() )
	    layout()->setEnabled( FALSE );
#endif
	QSize s( d->extension->sizeHint() );
	if ( d->orientation == Horizontal ) {
	    d->extension->setGeometry( width(), 0, s.width(), height() );
	    setFixedSize( width() + s.width(), height() );
	} else {
	    d->extension->setGeometry( 0, height(), width(), s.height() );
	    setFixedSize( width(), height() + s.height() );
	}
	d->extension->show();
    } else {
	d->extension->hide();
	setMinimumSize( d->min );
	setMaximumSize( d->max );
	resize( d->size );
#ifndef QT_NO_LAYOUT
	if ( layout() )
	    layout()->setEnabled( TRUE );
#endif
    }
}


/*! \reimp */
QSize QDialog::sizeHint() const
{
    if ( d->extension )
	if ( d->orientation == Horizontal )
	    return QSize( QWidget::sizeHint().width(),
			QMAX( QWidget::sizeHint().height(),d->extension->sizeHint().height() ) );
	else
	    return QSize( QMAX( QWidget::sizeHint().width(), d->extension->sizeHint().width() ),
			QWidget::sizeHint().height() );

    return QWidget::sizeHint();
}


/*! \reimp */
QSize QDialog::minimumSizeHint() const
{
    if ( d->extension )
	if (d->orientation == Horizontal )
	    return QSize( QWidget::minimumSizeHint().width(),
			QMAX( QWidget::minimumSizeHint().height(), d->extension->minimumSizeHint().height() ) );
	else
	    return QSize( QMAX( QWidget::minimumSizeHint().width(), d->extension->minimumSizeHint().width() ),
			QWidget::minimumSizeHint().height() );

    return QWidget::minimumSizeHint();
}



bool QDialog::isSizeGripEnabled() const
{
#ifndef QT_NO_SIZEGRIP
    return !!d->resizer;
#else
    return FALSE;
#endif
}


void QDialog::setSizeGripEnabled(bool enabled)
{
#ifndef QT_NO_SIZEGRIP
    if ( !enabled != !d->resizer ) {
	if ( enabled ) {
	    d->resizer = new QSizeGrip( this, "QDialog::resizer" );
	    d->resizer->adjustSize();
	    d->resizer->move( rect().bottomRight() -d->resizer->rect().bottomRight() );
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
void QDialog::resizeEvent( QResizeEvent * )
{
#ifndef QT_NO_SIZEGRIP
    if ( d->resizer )
	d->resizer->move( rect().bottomRight() -d->resizer->rect().bottomRight() );
#endif
}

#endif // QT_NO_DIALOG
