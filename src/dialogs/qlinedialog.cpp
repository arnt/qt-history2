/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qlinedialog.cpp#3 $
**
** Definition of QFileDialog class
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

struct QLineDialogPrivate
{
    QLineEdit* lineEdit;
};

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

    QPushButton *ok = new QPushButton( tr( "&OK" ), this );
    ok->setDefault( TRUE );
    QPushButton *cancel = new QPushButton( tr( "&Cancel" ), this );

    QSize bs( ok->sizeHint() );
    if ( cancel->sizeHint().width() > bs.width() )
	bs.setWidth( cancel->sizeHint().width() );

    ok->setFixedSize( bs );
    cancel->setFixedSize( bs );
    
    hbox->addWidget( new QWidget( this ) );
    hbox->addWidget( ok );
    hbox->addWidget( cancel );
    
    connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( d->lineEdit, SIGNAL( returnPressed() ), this, SLOT( accept() ) );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    
    resize( sizeHint().width() * 2, sizeHint().height() );
    
    d->lineEdit->setFocus();
}

QLineDialog::~QLineDialog()
{
    delete d;
}

QString QLineDialog::text() const
{
    return d->lineEdit->text();
}

void QLineDialog::setText( const QString& text )
{
    d->lineEdit->setText( text );
}

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
