/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdialog.cpp#90 $
**
** Implementation of QDialog class
**
** Created : 950502
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qdialog.h"
#include "qpushbutton.h"
#include "qapplication.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qwidgetlist.h"


// NOT REVISED
/*!
  \class QDialog qdialog.h
  \brief The QDialog class is the base class of dialog windows.

  \ingroup dialogs
  \ingroup abstractwidgets

  A dialog window is a widget used to communicate with the user. It offers
  mechanisms such as default buttons.

  The dialog window can either be modeless or modal. A modeless dialog is
  a normal window, while a modal window must be finished before the user
  can continue with other parts of the program.	 The third constructor
  argument must be set to TRUE to create a modal dialog, otherwise it will
  create a modeless dialog.

  Example (your own modal dialog):
  \code
    class Modal : public QDialog {
	Q_OBJECT
    public:
	Modal( QWidget *parent, QString name );
    };

    Modal::Modal( QWidget *parent, QString name )
	: QDialog( parent, name, TRUE )
    {
	QPushButton *ok, *cancel;
	ok = new QPushButton( "OK", this );
	ok->setGeometry( 10,10, 100,30 );
	connect( ok, SIGNAL(clicked()), SLOT(accept()) );
	cancel = new QPushButton( "Cancel", this );
	cancel->setGeometry( 10,60, 100,30 );
	connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    }
  \endcode

  Note that the parent widget has an additional meaning for modal
  dialogs.  A modal dialog is placed on top of the parent widget. The
  dialog is centered on the screen if the parent widget is zero.

  You would normally call exec() to start a modal dialog. This enters
  a local event loop, which is terminated when the modal dialog calls
  done() (or accept() or reject()).

  Example (using a modal dialog):
  \code
    Modal m;
    if ( m.exec() ) {
       // ok was pressed, then fetch the interesting dialog data
    }
  \endcode

  Modeless dialogs behave just like ordinary toplevel widgets. The only difference
  is that they have the default button mechanism.

  \sa QTabDialog QWidget QSemiModal
  <a href="guibooks.html#fowler">GUI Design Handbook: Dialogs, Standard.</a>
*/


/*!
  Constructs a dialog named \e name, which has a parent widget \e parent.

  The dialog will by default be modeless, unless you set \e modal to
  TRUE to construct a modal dialog.

  The \a f argument is the \link QWidget::QWidget() widget flags,
  \endlink which can be used to customize the window frame style.
  
  If you e.g. don't want a What"s this button in the titlebar of the dialog,
  pass WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu
  here.

  A dialog is always a top level widget. The optional parent, however,
  will know about this child and also delete it on
  destruction. Furthermore, the window system will be able to tell
  that both the dialog and the parent belong together. This works for
  Windows and also some X11 window managers, that will for instance
  provide a common taskbar entry in that case.

  It is recommended to pass a parent.
*/

QDialog::QDialog( QWidget *parent, const char *name, bool modal, WFlags f )
    : QWidget( parent, name, (modal ? (f | WType_Modal) : f) | WType_TopLevel | WStyle_Dialog )
{
    rescode = 0;
    did_move = FALSE;
    did_resize = FALSE;
    mainDef = 0;
}

/*!
  Destructs the QDialog, deleting all its children.
*/

QDialog::~QDialog()
{
    // Need to hide() here, as our (to-be) overridden hide()
    // will not be called in ~QWidget.
    hide();
}

/*!
  \internal
  This function is called by the push button \e pushButton when it
  becomes the default button. If \a pushButton is 0, the dialogs
  default default button becomes the default button. This is what a
  push button calls when it looses focus.
*/

void QDialog::setDefault( QPushButton *pushButton )
{
    QObjectList *list = queryList( "QPushButton" );
    QObjectListIt it( *list );
    QPushButton *pb;
    bool hasMain = FALSE;
    while ( (pb = (QPushButton*)it.current()) ) {
	++it;
	if ( pb == mainDef )
	    hasMain = TRUE;
	if ( pb != pushButton )
	    pb->setDefault( FALSE );
    }
    if (!pushButton && hasMain)
	mainDef->setDefault( TRUE );
    if (!hasMain)
	mainDef = pushButton;
    delete list;
}

/*!
  \internal
  Hides the default button indicator. Called when non auto-default
  push button get focus.
 */
void QDialog::hideDefault()
{
    QObjectList *list = queryList( "QPushButton" );
    QObjectListIt it( *list );
    QPushButton *pb;
    while ( (pb = (QPushButton*)it.current()) ) {
	++it;
	pb->setDefault( FALSE );
    }
}


/*!
  \fn int  QDialog::result() const

  Returns the result code of the dialog.
*/

/*!
  \fn void  QDialog::setResult( int )

  Sets the result code of the dialog.
*/


/*!
  For modal dialogs: Starts the dialog and returns the result code.

  Equivalent to calling show(), then result().

  This function is very useful for modal dialogs, but makes no sense for
  modeless dialog. It enters a new local event loop. The event loop is
  terminated when the dialog is hidden, usually by calling done().

  A warning message is printed if you call this function for a modeless
  dialog.

  \sa show(), result()
*/

int QDialog::exec()
{
#if defined(CHECK_STATE)
    if ( !testWFlags(WType_Modal) )
	qWarning( "QDialog::exec: Calling this function for a modeless dialog "
		 "makes no sense" );
#endif
    setResult( 0 );
    show();
    return result();
}


/*!
  Closes the dialog and sets the result code to \e r.

  Equivalent to calling hide(), then setResult(\e r ).

  This function is very useful for modal dialogs. It leaves the local
  event loop and returns from the exec() or show() function.

  \warning Although done() will return to the caller also if this
  dialog is modal, the local event loop is then marked for
  termination. Hence, a program should not try to do anything that
  depends on event handling before the corresponding exec() or show()
  has returned.

  \sa accept(), reject()
*/

void QDialog::done( int r )
{
    hide();
    setResult( r );
}

/*!
  Closes the dialog and sets the result code to \c Accepted.

  Equivalent to done(Accepted);
*/

void QDialog::accept()
{
    done( Accepted );
}

/*!
  Closes the dialog and sets the result code to \c Rejected.

  Equivalent to done(Rejected);
*/

void QDialog::reject()
{
    done( Rejected );
}


/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*!\reimp
*/
void QDialog::keyPressEvent( QKeyEvent *e )
{
    //   Calls reject() if Escape is pressed.  Simulates a button click
    //   for the default button if Enter is pressed.  All other keys are
    //   ignored.
    if ( e->state() == 0 ) {
	switch ( e->key() ) {
	case Key_Enter:
	case Key_Return: {
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

/*!
  Calls reject() if it is a modal dialog, or accepts the close event
  if it is a modeless dialog.
*/

void QDialog::closeEvent( QCloseEvent *e )
{
    e->accept();
    reject();					// same as Cancel
}


/*****************************************************************************
  Geometry management.
 *****************************************************************************/

/*!
  Shows the dialog box on the screen, as QWidget::show() and enters a
  local event loop if this dialog is modal (see constructor).

  This implementation also does automatic resizing and automatic
  positioning. If you have not already resized or moved the dialog, it
  will find a size that fits the contents and a position near the middle
  of the screen (or centered relative to the parent widget if any).

  \warning Calling show() for a modal dialog enters a local event loop.
  The event loop is terminated when the dialog is hidden, usually by
  calling done().

  \sa exec()
*/

void QDialog::show()
{
    if ( testWState(WState_Visible) )
	return;
    if ( !did_resize )
	adjustSize();
    if ( !did_move ) {
	QWidget *w = parentWidget();
	QPoint p( 0, 0 );
	int extraw = 0, extrah = 0;
	QWidget * desk = QApplication::desktop();
	if ( w )
	    w = w->topLevelWidget();

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

	if ( w )
	    p = QPoint( w->geometry().x() + w->width()/2,
			w->geometry().y() + w->height()/ 2 );
	else
	    p = QPoint( desk->width()/2, desk->height()/2 );

	p = QPoint( p.x()-width()/2 - extraw,
		    p.y()-height()/2 - extrah );

	if ( p.x() + extraw + width() > desk->width() )
	    p.setX( desk->width() - width() - extraw );
	if ( p.x() < 0 )
	    p.setX( 0 );

	if ( p.y() + extrah + height() > desk->height() )
	    p.setY( desk->height() - height() - extrah );
	if ( p.y() < 0 )
	    p.setY( 0 );

	move( p );
    }
    QWidget::show();
    if ( testWFlags(WType_Modal) )
	qApp->enter_loop();
}

/*!\reimp
*/
void QDialog::hide()
{
    // Reimplemented to exit a modal when the dialog is hidden.
    bool ex = testWState(WState_Visible) && testWFlags(WType_Modal);
    QWidget::hide();
    if ( ex )
	qApp->exit_loop();
}


/*****************************************************************************
  Detects any widget geometry changes done by the user.
 *****************************************************************************/

/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QDialog::move( int x, int y )
{
    did_move = TRUE;
    QWidget::move( x, y );
}

/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QDialog::move( const QPoint &p )
{
    did_move = TRUE;
    QWidget::move( p );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QDialog::resize( int w, int h )
{
    did_resize = TRUE;
    QWidget::resize( w, h );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QDialog::resize( const QSize &s )
{
    did_resize = TRUE;
    QWidget::resize( s );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QDialog::setGeometry( int x, int y, int w, int h )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( x, y, w, h );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QDialog::setGeometry( const QRect &r )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( r );
}
