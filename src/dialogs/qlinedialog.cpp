/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qlinedialog.cpp#5 $
**
** Definition of QLineDialog class
**
** Created : 991212
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qlinedialog.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <qvalidator.h>

//
//  W A R N I N G
//  -------------
//
//  It is very unlikely that this code will be available in the final
//  Qt release.  It might be available soon after then, but a number
//  of important API changes still need to be made.
//
//  Thus, it is important that you do NOT use this code in an application
//  unless you are willing for your application to be dependent on a
//  specific snapshot releases of Qt.
//

struct QLineDialogPrivate
{
    QLineEdit *lineEdit;
    QSpinBox *spinBox;
    QComboBox *comboBox, *editComboBox;
    QPushButton *ok;
    QWidgetStack *stack;
    QLineDialog::Type type;
};

/*!
  \class QLineDialog qlinedialog.h
  \brief A convenience dialog to get a simple input from the user

  The QLineDialog is a simple dialog which can be used if you
  need a simple input from the user. This can be text, a number or 
  an item from a list. Also a label has to be set to tell the user
  what he/she should input.
  
  To be able to get different types of inputs, you can set the input type of
  the dialog with setType().

  To e.g. get a textual input from the user you can use one of the static
  methods like this:

  \code
  bool ok = FALSE;
  QString text = QLineDialog::getText( tr( "Please enter your name" ), QString::null, &ok, this );
  if ( ok && !text.isEmpty() )
      ;// user entered something and pressed ok
  else
      ;// user entered nothing or pressed cancel
  \endcode
  
  There are more static convenience methods!
  
  \sa getText(), getInteger(), getDouble(), getItem()
*/

/*! 
  \enum QLineDialog::Type

  This enum type specifies the type of the dialog
  (which kind of input can be done):
  
  <ul>
  <li>\c LineEdit - A QLineEdit is used for taking the input, so a textual or
  (e.g. using a QValidator) a numerical input can be done. Using lineEdit()
  the QLineEdit can be accessed.
  <li>\c SpinBox - A QSpinBox is used for taking the input, so a decimal
  input can be done. Using spinBox() the QSpinBox can be accessed.
  <li>\c ComboBox - A readonly QComboBox is used for taking the input,
  so one item of a list can be chosen. Using comboBox() the QComboBox
  can be accessed.
  <li>\c EditableComboBox - An editable QComboBox is used for taking the input,
  so either one item of a list can be chosen or a text can be entered. Using
  editableComboBox() the QComboBox can be accessed.
  </ul>
*/

/*!
  Constructs the dialog. \a label is the text which is shown to the user (it should mention
  to the user what he/she should input), \a parent the parent widget of the dialog, \a name
  the name of it and if you set \a modal to TRUE, the dialog pops up modally, else it pops
  up modeless. With \a type you specify the type of the dialog.
  
  \sa getText(), getInteger(), getDouble(), getItem()
*/

QLineDialog::QLineDialog( const QString &label, QWidget* parent, const char* name, 
			  bool modal, Type type)
    : QDialog( parent, name, modal )
{
    d = new QLineDialogPrivate;
    d->lineEdit = 0;
    d->spinBox = 0;
    d->comboBox = 0;

    QVBoxLayout *vbox = new QVBoxLayout( this, 6, 6 );

    QLabel* l = new QLabel( label, this );
    vbox->addWidget( l );

    d->stack = new QWidgetStack( this );
    vbox->addWidget( d->stack );
    d->lineEdit = new QLineEdit( d->stack );
    d->spinBox = new QSpinBox( d->stack );
    d->comboBox = new QComboBox( FALSE, d->stack );
    d->editComboBox = new QComboBox( TRUE, d->stack );
    
    QHBoxLayout *hbox = new QHBoxLayout( 6 );
    vbox->addLayout( hbox, AlignRight );

    d->ok = new QPushButton( tr( "&OK" ), this );
    d->ok->setDefault( TRUE );
    QPushButton *cancel = new QPushButton( tr( "&Cancel" ), this );

    QSize bs( d->ok->sizeHint() );
    if ( cancel->sizeHint().width() > bs.width() )
	bs.setWidth( cancel->sizeHint().width() );

    d->ok->setFixedSize( bs );
    cancel->setFixedSize( bs );

    hbox->addWidget( new QWidget( this ) );
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
  Returns the line edit, which is used in the LineEdit mode
*/

QLineEdit *QLineDialog::lineEdit() const
{
    return d->lineEdit;
}

/*!
  Returns the spinbox, which is used in the SpinBox mode
*/

QSpinBox *QLineDialog::spinBox() const
{
    return d->spinBox;
}

/*!
  Returns the combobox, which is used in the ComboBox mode
*/

QComboBox *QLineDialog::comboBox() const
{
    return d->comboBox;
}

/*!
  Returns the combobox, which is used in the EditableComboBox mode
*/

QComboBox *QLineDialog::editableComboBox() const
{
    return d->editComboBox;
}

/*!
  Sets the input type of the dialog to \a t.
*/

void QLineDialog::setType( Type t )
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

QLineDialog::Type QLineDialog::type() const
{
    return d->type;
}

/*!
  Destructor.
*/

QLineDialog::~QLineDialog()
{
    delete d;
}

/*!
  Static convenience function to get a textual input from the user. \a label is the text which
  is shown to the user (it should mention to the user what he/she should input), \a text
  the default text which will be initially set to the line edit, \a ok a pointer to
  a bool which will be (if not 0!) set to TRUE if the user pressed ok or to FALSE if the
  user pressed cancel, \a parent the parent widget of the dialog and \a name
  the name of it. The dialogs pops up modally!

  This method returns the text which has been entered in the line edit.

  You will use this static method like this:

  \code
  bool ok = FALSE;
  QString text = QLineDialog::getText( tr( "Please enter your name" ), QString::null, &ok, this );
  if ( ok && !text.isEmpty() )
      ;// user entered something and pressed ok
  else
      ;// user entered nothing or pressed cancel
  \endcode
*/

QString QLineDialog::getText( const QString &label, const QString &text,
			      bool *ok, QWidget *parent, const char *name )
{
    QLineDialog *dlg = new QLineDialog( label, parent, name, TRUE, LineEdit );
    dlg->setCaption( tr( "Please enter a Text" ) );
    dlg->lineEdit()->setText( text );
    if ( !text.isEmpty() )
	dlg->lineEdit()->selectAll();
    
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
  Static convenience function to get an integral input from the user. \a label is the text which
  is shown to the user (it should mention to the user what he/she should input), \a num
  the default number which will be initially set to the spinbox, \a from and \a to the
  range in which the entered number has to be, \a step the step in which the number can
  be increased/decreased by the spinbox, \a ok a pointer to
  a bool which will be (if not 0!) set to TRUE if the user pressed ok or to FALSE if the
  user pressed cancel, \a parent the parent widget of the dialog and \a name
  the name of it. The dialogs pops up modally!

  This method returns the number which has been entered by the user.

  You will use this static method like this:

  \code
  bool ok = FALSE;
  int res = QLineDialog::getInteger( tr( "Please enter a number" ), 22, 0, 1000, 2, &ok, this );
  if ( ok )
      ;// user entered something and pressed ok
  else
      ;// user pressed cancel
  \endcode
*/

int QLineDialog::getInteger( const QString &label, int num, int from, int to, int step,
			    bool *ok, QWidget *parent, const char *name )
{
    QLineDialog *dlg = new QLineDialog( label, parent, name, TRUE, SpinBox );
    dlg->setCaption( tr( "Please enter an integral Number" ) );
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
  Static convenience function to get a decimal input from the user. \a label is the text which
  is shown to the user (it should mention to the user what he/she should input), \a num
  the default decimal number which will be initially set to the line edit, \a from and \a to the
  range in which the entered number has to be, \a decimals the number of decimal which
  the number may have, \a ok a pointer to
  a bool which will be (if not 0!) set to TRUE if the user pressed ok or to FALSE if the
  user pressed cancel, \a parent the parent widget of the dialog and \a name
  the name of it. The dialogs pops up modally!

  This method returns the number which has been entered by the user.

  You will use this static method like this:

  \code
  bool ok = FALSE;
  double res = QLineDialog::getDouble( tr( "Please enter a decimal number" ), 33.7, 0, 1000, 2, &ok, this );
  if ( ok )
      ;// user entered something and pressed ok
  else
      ;// user pressed cancel
  \endcode
*/

double QLineDialog::getDouble( const QString &label, double num, double from, double to, int decimals,
			       bool *ok, QWidget *parent, const char *name )
{
    QLineDialog *dlg = new QLineDialog( label, parent, name, TRUE, LineEdit );
    dlg->setCaption( tr( "Please enter a decimal Number" ) );
    dlg->lineEdit()->setValidator( new QDoubleValidator( from, to, decimals, dlg->lineEdit() ) );
    dlg->lineEdit()->setText( QString::number( num ) );
	dlg->lineEdit()->selectAll();
    
    bool ok_ = FALSE;
    double result;
    ok_ = dlg->exec() == QDialog::Accepted;
    if ( ok )
	*ok = ok_;
    result = dlg->lineEdit()->text().toDouble();
    
    delete dlg;
    return result;
}

/*!
  Static convenience function to let the user select an item from a string list. \a label is the text which
  is shown to the user (it should mention to the user what he/she should input), \a list the
  string list which is inserted into the combobox, \a current the nimber of the item which should
  be initially the current item, \a editable specifies if the combobox should be editable (if it is TRUE)
  or readonly (if \a editable is FALSE), \a ok a pointer to
  a bool which will be (if not 0!) set to TRUE if the user pressed ok or to FALSE if the
  user pressed cancel, \a parent the parent widget of the dialog and \a name
  the name of it. The dialogs pops up modally!

  This method returns the text of the current item, or if \a editable was TRUE, the current
  text of the combobox.

  You will use this static method like this:

  \code
  QStringList lst;
  lst << "First" << "Second" << "Third" << "Fourth" << "Fifth";
  bool ok = FALSE;
  QString res = QLineDialog::getItem( tr( "Please select an item" ), lst, 1, TRUE, &ok, this );
  if ( ok )
      ;// user selected an item and pressed ok
  else
      ;// user pressed cancel
  \endcode
*/

QString QLineDialog::getItem( const QString &label, const QStringList &list, int current, bool editable,
			      bool *ok, QWidget *parent, const char *name )
{
    QLineDialog *dlg = new QLineDialog( label, parent, name, TRUE, editable ? EditableComboBox : ComboBox );
    dlg->setCaption( tr( "Please select an Item" ) );
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
*/

void QLineDialog::textChanged( const QString &s )
{
    d->ok->setEnabled( !s.isEmpty() );
}

/*!
  \internal
*/

void QLineDialog::tryAccept()
{
    if ( !d->lineEdit->text().isEmpty() )
	accept();
}
