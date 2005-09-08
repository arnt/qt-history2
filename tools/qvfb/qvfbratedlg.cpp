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

#include <qlayout.h>
#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include "qvfbratedlg.h"

QVFbRateDialog::QVFbRateDialog( int rate, QWidget *parent, const char *name,
    bool modal )
    : QDialog( parent, name, modal )
{
    oldRate = rate;

    QVBoxLayout *tl = new QVBoxLayout( this, 5 );

    QLabel *label = new QLabel( "Target frame rate:", this );
    tl->addWidget( label );

    QHBoxLayout *hl = new QHBoxLayout( tl );
    rateSlider = new QSlider( 1, 100, 10, rate, Qt::Horizontal, this );
    hl->addWidget( rateSlider );
    connect( rateSlider, SIGNAL(valueChanged(int)), this, SLOT(rateChanged(int)) );
    rateLabel = new QLabel( QString( "%1fps" ).arg(rate), this );
    hl->addWidget( rateLabel );

    hl = new QHBoxLayout( tl );
    QPushButton *pb = new QPushButton( "OK", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(ok()) );
    hl->addWidget( pb );
    pb = new QPushButton( "Cancel", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(cancel()) );
    hl->addWidget( pb );
}

void QVFbRateDialog::rateChanged( int r )
{
    if ( rateSlider->value() != r )
	rateSlider->setValue( r );
    rateLabel->setText( QString( "%1fps" ).arg(r) );
    emit updateRate(r);
}

void QVFbRateDialog::cancel()
{
    rateChanged( oldRate );
    reject();
}

void QVFbRateDialog::ok()
{
    oldRate = rateSlider->value();
    accept();
}
