/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qlinedialog.cpp#5 $
**
** Definition of QLineDialog class
**
** Created : 950428
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
    QLineEdit* lineEdit;
    QPushButton *ok;
};

/*!
  \class QLineDialog qlinedialog.h
  \brief A convenience dialog to get a simple input from the user
  
  The QLineDialog is a simple dialog which can be used if you
  need a simple input from the user. You can set a label, and the
  user can input a text in a line edit and confirm or cancel the input.

  In most cases you will use the static method getText() like this
  
  \code
  bool ok = FALSE;
  QString text = QLineDialog::getText( tr( "Please enter your name" ), QString::null, &ok, this );
  if ( ok && !text.isEmpty() )
      ;// user entered something and pressed ok
  else
      ;// user entered nothing or pressed cancel
  \endcode
*/

/*!
  Constructs the dialog. \a label is the text which is shown to the user (it should mention
  to the user what he/she should input), \a parent the parent widget of the dialog, \a name
  the name of it and if you set \a modal to TRUE, the dialog pops up modally, else it pops
  up modeless.
  
  \sa getText()
*/

QLineDialog::QLineDialog( const QString &label, QWidget* parent, const char* name, bool modal )
    : QDialog( parent, name, modal )
{
    d = new QLineDialogPrivate;
    d->lineEdit = 0;

    QVBoxLayout *vbox = new QVBoxLayout( this, 6, 6 );

    QLabel* l = new QLabel( label, this );
    vbox->addWidget( l );

    d->lineEdit = new QLineEdit( this );
    vbox->addWidget( d->lineEdit );

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

    connect( d->ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( d->lineEdit, SIGNAL( returnPressed() ), this, SLOT( tryAccept() ) );
    connect( d->lineEdit, SIGNAL( textChanged( const QString & ) ), this, SLOT( textChanged( const QString & ) ) );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );

    resize( QMAX( sizeHint().width(), 400 ), sizeHint().height() );

    d->lineEdit->setFocus();
}

/*!
  Destructor.
*/

QLineDialog::~QLineDialog()
{
    delete d;
}

/*!
  Returns the text which has been entered in the line edit or an empty
  string if nothing has been entered.
*/

QString QLineDialog::text() const
{
    return d->lineEdit->text();
}

/*!
  Sets the text in the line edit to \a text.
*/

void QLineDialog::setText( const QString& text )
{
    d->lineEdit->setText( text );
}

/*!
  Static convenience function to get an input from the user. \a label is the text which 
  is shown to the user (it should mention to the user what he/she should input), \a text
  the default text which will be initially set to the line edit, \a ok a pointer to
  a bool which will be (if not 0!) set to TRUE if the user pressed ok or to FALSE if the
  user pressed cancel, \a parent the parent widget of the dialog, \a name
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
    QLineDialog *dlg = new QLineDialog( label, parent, name, TRUE );
    dlg->setCaption( tr( "Enter a text" ) );
    dlg->setText( text );
    bool ok_ = FALSE;
    QString result;
    ok_ = dlg->exec() == QDialog::Accepted;
    if ( ok )
	*ok = ok_;
    if ( ok_ )
	result = dlg->text();
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
