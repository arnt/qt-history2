

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
    rateSlider = new QSlider( 10, 100, 10, rate, QSlider::Horizontal, this );
    hl->addWidget( rateSlider );
    connect( rateSlider, SIGNAL(valueChanged(int)), this, SLOT(rateChanged(int)) );
    rateLabel = new QLabel( QString( "%1fps" ).arg(rate), this );
    hl->addWidget( rateLabel );

    hl = new QHBoxLayout( tl );
    QPushButton *pb = new QPushButton( "OK", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(accept()) );
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

