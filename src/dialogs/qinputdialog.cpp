/****************************************************************************
** $Id$
**
** Implementation of QInputDialog class
**
** Created : 991212
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
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

#include "qinputdialog.h"

#ifndef QT_NO_INPUTDIALOG

#include "qlayout.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qspinbox.h"
#include "qcombobox.h"
#include "qwidgetstack.h"
#include "qvalidator.h"
#include "qapplication.h"

class QInputDialogPrivate
{
public:
    friend class QInputDialog;
    QLineEdit *lineEdit;
    QSpinBox *spinBox;
    QComboBox *comboBox, *editComboBox;
    QPushButton *ok;
    QWidgetStack *stack;
    QInputDialog::Type type;
};

/*!
  \class QInputDialog qinputdialog.h
  \brief The QInputDialog class provides a simple convenience dialog to get a single value from the user.
  \ingroup dialogs
  \mainclass

  The QInputDialog is a simple dialog which can be used if you need to
  get a single input value from the user. The input value can be a
  string, a number or an item from a list. A label has to be set to tell
  the user what they should input.

  Four static convenience functions are provided:
  getText(), getInteger(), getDouble() and getItem(). All the
  functions can be used in a similar way, for example:
  \code
  bool ok = FALSE;
  QString text = QInputDialog::getText(
		    tr( "Application name" ),
		    tr( "Please enter your name" ),
		    QLineEdit::Normal, QString::null, &ok, this );
  if ( ok && !text.isEmpty() )
      ;// user entered something and pressed OK
  else
      ;// user entered nothing or pressed Cancel
  \endcode

  \img inputdialogs.png Input Dialogs

*/

/*!
  \enum QInputDialog::Type

  This enum specifies the type of the dialog, i.e. what kind of data you
  want the user to input:

  \value LineEdit  A QLineEdit is used for obtaining the input which may be
  a string or (e.g. using a QValidator) a number. The QLineEdit can be
  accessed using lineEdit().
  \value SpinBox  A QSpinBox is used for obtaining integer input.
  Use spinBox() to access the QSpinBox.
  \value ComboBox  A read-only QComboBox is used to provide a fixed
  list of choices from which the user can choose.
  Use comboBox() to access the QComboBox.
  \value EditableComboBox  An editable QComboBox is used to provide a fixed
  list of choices from which the user can choose, but which also
  allows the user to enter their own value instead.
  Use editableComboBox() to access the QComboBox.
*/

/*!
  Constructs the dialog. The \a label is the text which is shown to the user
  (it should mention to the user what they should input). The \a parent
  is the dialog's parent widget. The widget is called \a name. If \a
  modal is TRUE the dialog will be modal. The \a type parameter is used
  to specify which type of dialog to construct.

  \sa getText(), getInteger(), getDouble(), getItem()
*/

QInputDialog::QInputDialog( const QString &label, QWidget* parent, const char* name,
			  bool modal, Type type)
    : QDialog( parent, name, modal )
{
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( parent && parent->icon() &&!parent->icon()->isNull() )
	setIcon( *parent->icon() );
    else if ( qApp->mainWidget() && qApp->mainWidget()->icon() && !qApp->mainWidget()->icon()->isNull() )
	QDialog::setIcon( *qApp->mainWidget()->icon() );
#endif

    d = new QInputDialogPrivate;
    d->lineEdit = 0;
    d->spinBox = 0;
    d->comboBox = 0;

    QVBoxLayout *vbox = new QVBoxLayout( this, 6, 6 );

    QLabel* l = new QLabel( label, this, "qt_inputdlg_lbl" );
    vbox->addWidget( l );

    d->stack = new QWidgetStack( this, "qt_inputdlg_ws" );
    vbox->addWidget( d->stack );
    d->lineEdit = new QLineEdit( d->stack, "qt_inputdlg_le" );
    d->spinBox = new QSpinBox( d->stack, "qt_inputdlg_sb" );
    d->comboBox = new QComboBox( FALSE, d->stack, "qt_inputdlg_cb" );
    d->editComboBox = new QComboBox( TRUE, d->stack, "qt_inputdlg_editcb" );

    QHBoxLayout *hbox = new QHBoxLayout( 6 );
    vbox->addLayout( hbox, AlignRight );

    d->ok = new QPushButton( tr( "OK" ), this, "qt_ok_btn" );
    d->ok->setDefault( TRUE );
    QPushButton *cancel = new QPushButton( tr( "Cancel" ), this, "qt_cancel_btn" );

    QSize bs( d->ok->sizeHint() );
    if ( cancel->sizeHint().width() > bs.width() )
	bs.setWidth( cancel->sizeHint().width() );

    d->ok->setFixedSize( bs );
    cancel->setFixedSize( bs );

    hbox->addWidget( new QWidget( this, "qt_hbox" ) );
    hbox->addWidget( d->ok );
    hbox->addWidget( cancel );

    connect( d->lineEdit, SIGNAL( returnPressed() ),
	     this, SLOT( tryAccept() ) );
    connect( d->lineEdit, SIGNAL( textChanged( const QString & ) ),
	     this, SLOT( textChanged( const QString & ) ) );

    connect( d->ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );

    resize( QMAX( sizeHint().width(), 400 ), sizeHint().height() );

    setType( type );
}

/*!
  Returns the line edit, which is used in LineEdit mode
*/

QLineEdit *QInputDialog::lineEdit() const
{
    return d->lineEdit;
}

/*!
  Returns the spinbox, which is used in SpinBox mode
*/

QSpinBox *QInputDialog::spinBox() const
{
    return d->spinBox;
}

/*!
  Returns the combobox, which is used in ComboBox mode
*/

QComboBox *QInputDialog::comboBox() const
{
    return d->comboBox;
}

/*!
  Returns the combobox, which is used in EditableComboBox mode
*/

QComboBox *QInputDialog::editableComboBox() const
{
    return d->editComboBox;
}

/*!
  Sets the input type of the dialog to \a t.
*/

void QInputDialog::setType( Type t )
{
    switch ( t ) {
    case LineEdit:
	d->stack->raiseWidget( d->lineEdit );
	d->lineEdit->setFocus();
	break;
    case SpinBox:
	d->stack->raiseWidget( d->spinBox );
	d->spinBox->setFocus();
	break;
    case ComboBox:
	d->stack->raiseWidget( d->comboBox );
	d->comboBox->setFocus();
	break;
    case EditableComboBox:
	d->stack->raiseWidget( d->editComboBox );
	d->editComboBox->setFocus();
	break;
    }

    d->type = t;
}

/*!
  Returns the input type of the dialog.

  \sa setType()
*/

QInputDialog::Type QInputDialog::type() const
{
    return d->type;
}

/*!
  Destructor.
*/

QInputDialog::~QInputDialog()
{
    delete d;
}

/*!
  Static convenience function to get a string from the user. \a
  caption is the text which is displayed in the title bar of the dialog.
  \a label is the text which is shown to the user (it should mention
  what they should input), \a text the default text which is placed in
  the line edit. The \a mode is the echo mode the
  line edit will use.
  If \a ok is not-null it will be set to TRUE if the user pressed OK and
  FALSE if the user pressed Cancel.
  The dialog's parent is \a parent; the dialog is called \a name. The
  dialog will be modal.

  This method returns the text which has been entered in the line edit.

  Use this static method like this:

  \code
  bool ok = FALSE;
  QString text = QInputDialog::getText(
		    tr( "Application name" ),
		    tr( "Please enter your name" ),
		    QLineEdit::Normal, QString::null, &ok, this );
  if ( ok && !text.isEmpty() )
      ;// user entered something and pressed OK
  else
      ;// user entered nothing or pressed Cancel
  \endcode
*/

QString QInputDialog::getText( const QString &caption, const QString &label, QLineEdit::EchoMode mode,
			      const QString &text, bool *ok, QWidget *parent, const char *name )
{
    QInputDialog *dlg = new QInputDialog( label, parent, name ? name : "qt_inputdlg_gettext", TRUE, LineEdit );

#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg->setCaption( caption );
#endif
    dlg->lineEdit()->setText( text );
    dlg->lineEdit()->setEchoMode( mode );

    bool ok_ = FALSE;
    QString result;
    ok_ = dlg->exec() == QDialog::Accepted;
    if ( ok )
	*ok = ok_;
    if ( ok_ )
	result = dlg->lineEdit()->text();

    delete dlg;
    return result;
}

/*!
  Static convenience function to get an integer input from the user. \a
  caption is the text which is displayed in the title bar of the dialog.
  \a label is the text which is shown to the user (it should mention
  what they should input), \a num is the default number which the
  spinbox will be set to.
  \a from and \a to are the minimum and maximum values the user may
  choose, and \a step is the amount by which the values change as the
  user presses the arrow buttons to increment or decrement the value.

  If \a ok is not-null it will be set to TRUE if the user pressed OK and
  FALSE if the user pressed Cancel.
  The dialog's parent is \a parent; the dialog is called \a name. The
  dialog will be modal.

  This method returns the number which has been entered by the user.

  Use this static method like this:

  \code
  bool ok = FALSE;
  int res = QInputDialog::getInteger(
		tr( "Application name" ),
		tr( "Please enter a number" ), 22, 0, 1000, 2, &ok, this );
  if ( ok )
      ;// user entered something and pressed OK
  else
      ;// user pressed Cancel
  \endcode
*/

int QInputDialog::getInteger( const QString &caption, const QString &label, int num, int from, int to, int step,
			    bool *ok, QWidget *parent, const char *name )
{
    QInputDialog *dlg = new QInputDialog( label, parent, name ? name : "qt_inputdlg_getint", TRUE, SpinBox );
#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg->setCaption( caption );
#endif
    dlg->spinBox()->setRange( from, to );
    dlg->spinBox()->setSteps( step, 0 );
    dlg->spinBox()->setValue( num );

    bool ok_ = FALSE;
    int result;
    ok_ = dlg->exec() == QDialog::Accepted;
    if ( ok )
	*ok = ok_;
    result = dlg->spinBox()->value();

    delete dlg;
    return result;
}

/*!
  Static convenience function to get a floating point number from the user. \a
  caption is the text which is displayed in the title bar of the dialog.
  \a label is the text which is shown to the user (it should mention
  what they should input), \a num is the default floating point number
  that the line edit will be set to.
  \a from and \a to are the minimum and maximum values the user may
  choose, and \a decimals is the maximum number of decimal places the
  number may have.

  If \a ok is not-null it will be set to TRUE if the user pressed OK and
  FALSE if the user pressed Cancel.
  The dialog's parent is \a parent; the dialog is called \a name. The
  dialog will be modal.

  This method returns the floating point number which has been entered
  by the user.

  Use this static method like this:

  \code
  bool ok = FALSE;
  double res = QInputDialog::getDouble(
		tr( "Application name" ),
		tr( "Please enter a decimal number" ),
		33.7, 0, 1000, 2, &ok, this );
  if ( ok )
      ;// user entered something and pressed OK
  else
      ;// user pressed Cancel
  \endcode
*/

double QInputDialog::getDouble( const QString &caption, const QString &label, double num,
				double from, double to, int decimals,
				bool *ok, QWidget *parent, const char *name )
{
    QInputDialog *dlg = new QInputDialog( label, parent, name ? name : "qt_inputdlg_getdbl", TRUE, LineEdit );
#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg->setCaption( caption );
#endif
    dlg->lineEdit()->setValidator( new QDoubleValidator( from, to, decimals, dlg->lineEdit() ) );
    dlg->lineEdit()->setText( QString::number( num ) );
	dlg->lineEdit()->selectAll();

    bool ok_ = FALSE;
    double result;
    ok_ = dlg->exec() == QDialog::Accepted;
    if ( ok )
	*ok = ok_;

    QString editText = dlg->lineEdit()->text().lower();
    bool isExp = editText.find('e') != -1;
    int i = editText.find( '.' );

    if ( i >= 0 ) {
	// has decimal point, now count digits after that
	i++;
	int j = i;
	while( editText[j].isDigit() || editText[j] == 'e' || editText[j] == '-' || editText[j] == '+' )
	    j++;
        if ( !isExp && j - i > decimals ) // if number contains an 'e' e.g. is exponential we don't truncate at all
            editText.truncate( i + decimals );
    }
    result = editText.toDouble();


    delete dlg;
    return result;
}

/*!
  Static convenience function to let the user select an item from a
  string list. \a caption is the text which is displayed in the title
  bar of the dialog. \a label is the text which is shown to the user (it
  should mention what they should input). \a list is the
  string list which is inserted into the combobox, and \a current is the number
  of the item which should be the current item. If \a editable is TRUE
  the user can enter their own text; if \a editable is FALSE the user
  may only select one of the existing items.

  If \a ok is not-null it will be set to TRUE if the user pressed OK and
  FALSE if the user pressed Cancel.
  The dialog's parent is \a parent; the dialog is called \a name. The
  dialog will be modal.

  This method returns the text of the current item, or if \a editable
  is TRUE, the current text of the combobox.

  Use this static method like this:

  \code
  QStringList lst;
  lst << "First" << "Second" << "Third" << "Fourth" << "Fifth";
  bool ok = FALSE;
  QString res = QInputDialog::getItem(
		    tr( "Application name" ),
		    tr( "Please select an item" ), lst, 1, TRUE, &ok, this );
  if ( ok )
      ;// user selected an item and pressed OK
  else
      ;// user pressed Cancel
  \endcode
*/

QString QInputDialog::getItem( const QString &caption, const QString &label, const QStringList &list,
			       int current, bool editable,
			       bool *ok, QWidget *parent, const char *name )
{
    QInputDialog *dlg = new QInputDialog( label, parent, name ? name : "qt_inputdlg_getitem", TRUE, editable ? EditableComboBox : ComboBox );
#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg->setCaption( caption );
#endif
    if ( editable ) {
	dlg->editableComboBox()->insertStringList( list );
	dlg->editableComboBox()->setCurrentItem( current );
    } else {
	dlg->comboBox()->insertStringList( list );
	dlg->comboBox()->setCurrentItem( current );
    }

    bool ok_ = FALSE;
    QString result;
    ok_ = dlg->exec() == QDialog::Accepted;
    if ( ok )
	*ok = ok_;
    if ( editable )
	result = dlg->editableComboBox()->currentText();
    else
	result = dlg->comboBox()->currentText();

    delete dlg;
    return result;
}

/*!
  \internal

  This slot is invoked when the text is changed; the new text is passed
  in \a s.
*/

void QInputDialog::textChanged( const QString &s )
{
    d->ok->setEnabled( !s.isEmpty() );
}

/*!
  \internal
*/

void QInputDialog::tryAccept()
{
    if ( !d->lineEdit->text().isEmpty() )
	accept();
}

#endif
