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
#include "qlayout.h"
#include "qsizegrip.h"


// NOT REVISED
/*!
  \class QDialog qdialog.h
  \brief The QDialog class is the base class of dialog windows.

  \ingroup dialogs
  \ingroup abstractwidgets

  A dialog window is a widget used to communicate with the user. It
  offers mechanisms such as modality, default buttons and
  extensibility.

  The dialog window can either be modeless or modal. A modeless dialog
  is a normal secondary window, while a modal window must be finished
  before the user can continue with other parts of the program. The
  third constructor argument must be set to TRUE to create a modal
  dialog, otherwise it will create a modeless dialog.

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

  Note that the parent widget has an additional meaning for dialogs.
  A dialog is placed on top of the parent widget and associated with
  it (i.e. it stays on top of its parent and shares certain resources,
  for example a common taskbar entry). If the parent widget is zero,
  the dialog is centered on the screen.

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

  A dialog can also provide a QSizeGrip in the lower-right corner.  By
  default, this is disabled. You can enable it with
  setSizeGripEnabled(TRUE);
  
  \sa QTabDialog QWidget QSemiModal
  <a href="guibooks.html#fowler">GUI Design Handbook: Dialogs, Standard.</a>
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
};


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
    in_loop = FALSE;
    d = new QDialogPrivate;
}

/*!
  Destructs the QDialog, deleting all its children.
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
  This function is called by the push button \e pushButton when it
  becomes the default button. If \a pushButton is 0, the dialogs
  default default button becomes the default button. This is what a
  push button calls when it looses focus.
*/

void QDialog::setDefault( QPushButton *pushButton )
{
#ifndef QT_NO_WIDGETS
    QObjectList *list = queryList( "QPushButton" );
    QObjectListIt it( *list );
    QPushButton *pb;
    bool hasMain = FALSE;
    while ( (pb = (QPushButton*)it.current()) ) {
	++it;
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
#ifndef QT_NO_WIDGETS
    QObjectList *list = queryList( "QPushButton" );
    QObjectListIt it( *list );
    QPushButton *pb;
    while ( (pb = (QPushButton*)it.current()) ) {
	++it;
	pb->setDefault( FALSE );
    }
#endif
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
  Hides the dialog and sets the result code to \e r.

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
  Hides the dialog and sets the result code to \c Accepted.

  Equivalent to done(Accepted);
*/

void QDialog::accept()
{
    done( Accepted );
}

/*!
  Hides the dialog and sets the result code to \c Rejected.

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
#ifndef QT_NO_WIDGETS
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


    /*########### 3.0:

      This 'feature' is nonsense and will be removed in 3.0.
      show()
      should do show() and nothing more.  If these lines are removed,
      we can finally kill QSemiModal and let QProgressBar inherit
      QDialog.
     */
    if ( testWFlags(WType_Modal) && !in_loop ) {
	in_loop = TRUE;
	qApp->enter_loop();
    }
}

/*!\reimp
*/
void QDialog::hide()
{
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

/*!\reimp
*/

void QDialog::move( int x, int y )
{
    did_move = TRUE;
    QWidget::move( x, y );
}

/*!\reimp
*/

void QDialog::move( const QPoint &p )
{
    did_move = TRUE;
    QWidget::move( p );
}

/*!\reimp
*/

void QDialog::resize( int w, int h )
{
    did_resize = TRUE;
    QWidget::resize( w, h );
}

/*!\reimp
*/

void QDialog::resize( const QSize &s )
{
    did_resize = TRUE;
    QWidget::resize( s );
}

/*!\reimp
*/

void QDialog::setGeometry( int x, int y, int w, int h )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( x, y, w, h );
}

/*!\reimp
*/

void QDialog::setGeometry( const QRect &r )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( r );
}


/*!
  Sets whether the dialog should extend horizontally or vertically,
  depending on \a orientation.

  \sa orientation(), setExtension()
*/
void QDialog::setOrientation( Orientation orientation )
{
    d->orientation = orientation;
}

/*!
  Returns the extension direction of the dialog.

  \sa setOrientation()
*/
Qt::Orientation QDialog::orientation() const
{
    return d->orientation;
}

/*!
  Sets \a extension to be the dialog's extension.

  The dialogs takes over ownership of the extension. Any previously
  defined extension is deleted.

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
  defined yet.

  \sa setExtension()
 */
QWidget* QDialog::extension() const
{
    return d->extension;
}


/*!
  Extends the dialog to show its extension if \a showIt is TRUE,
  otherwise hides the extension.

  This slot is usually connected to the toggled-signal of a toggle
  button (see QPushButton::toggled() ). Per default, the dialog
  extends horizontally. Adjust this behaviour with setOrientation().

  Nothing happens if the dialog is not visible yet.

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


/*!\reimp
 */
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


/*!\reimp
 */
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


/*!
  \fn bool QStatusBar::isSizeGripEnabled() const

  Returns whether the QSizeGrip in the bottom right of the dialog
  is enabled.

  \sa setSizeGripEnabled()
*/

bool QDialog::isSizeGripEnabled() const
{
#ifndef QT_NO_SIZEGRIP
    return (bool)d->resizer;
#else
    return FALSE;
#endif    
}

/*!
  Enables or disables the QSizeGrip in the bottom right of the dialog.
  By default, the size grip is disabled.

  \sa isSizeGripEnabled()
*/
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



/*!\reimp
 */
void QDialog::resizeEvent( QResizeEvent * )
{
#ifndef QT_NO_SIZEGRIP
    if ( d->resizer )
	d->resizer->move( rect().bottomRight() -d->resizer->rect().bottomRight() ); 
#endif
}
