#include <qapplication.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qslider.h>
#include <qdatetime.h>
#include <qmessagebox.h>

#include <stdlib.h>

#include "sliderrace.h"

QMutex raceMutex;
QMutex exitMtx;
static const char horseNames[4][13] =
{
    "Daisy Dee", "Jolly Jumper", "Dicke Oma", "Shadowfax"
};

/*
  Commentator implementation
*/

Commentator::Commentator( Race* r )
: race( r ), exit( FALSE )
{
    horses.setAutoDelete( TRUE );
    action = new QPushButton( "Place your horses!", race );
    connect( action, SIGNAL(clicked()), this, SLOT(placeHorses()));
}

Commentator::~Commentator()
{
}

void Commentator::run()
{
    while ( horses.count() < 4 )
	msleep(1);

    action->setText( "Start the race!" );
    disconnect( action, SIGNAL(clicked()), this, SLOT(placeHorses()) );
    connect( action, SIGNAL(clicked()), this, SLOT(startRace()));

    race->waitForFinish();

    Horse* winner = 0;
    uint h;
    for ( h = 0; h < horses.count(); h++ ) {
	if ( !horses.at( h )->toGo() )
	    winner = horses.at( h );
    }
    action->setText( QString( "And the winner is... %1 " ).arg( winner->name() ) );
    disconnect( action, SIGNAL(clicked()), this, SLOT(startRace()) );
    connect( action, SIGNAL(clicked()), this, SLOT(quit()));

    msleep( 200 );

    race->celebrate();

    QPalette pal = action->palette();
    while ( !exit ) {
	for ( uint i = 0; i < 256; i++ ) {
	    qApp->lock();
	    pal.setColor( QColorGroup::Button, QColor( 0, 255 - i, i ) );
	    action->setPalette( pal );
	    qApp->unlock();
	}
    }
    for ( h = 0; h < horses.count(); h++ ) {
	horses.at( h )->exit = TRUE;
	horses.at( h )->wait();
    }
}

void Commentator::placeHorses()
{
    qApp->lock();
    Horse* h = new Horse( race, QString( horseNames[ horses.count() ]) );
    horses.append( h );
    h->start();
    qApp->unlock(FALSE);
}

void Commentator::startRace()
{
    qApp->lock();
    
    race->start();
    
    QMessageBox::information( race, "Information!",
			      "The race has been started!\n"
			      "But this dialog is modal, so all threads \n"
			      "trying to block the application have to wait!" );

    qApp->unlock();
}

void Commentator::quit()
{
    // This function call is actually caused by a mouse event
    // that was processed in the eventloop, so the application
    // mutex is locked.
    qApp->unlock(FALSE);

    exit = TRUE;
    wait(); // and wait for exit
    qApp->quit();

    qApp->lock();
    // return to eventloop like we left it
}

/*
  Horse implementation
*/
Horse::Horse( Race* r, const QString& message )
: race( r ), exit( FALSE )
{
    speed = 10 + 100*( double(rand()) / double(RAND_MAX) ) ;

    QHBox* box = new QHBox( race );
    box->setSpacing( 20 );
    QLabel* label = new QLabel( message, box );
    horse = new QSlider( Horizontal, box );
    horse->setMaxValue( 100 );
    box->show();
    msg = message;
}

Horse::~Horse()
{
}

void Horse::run()
{
    race->waitForStart();

    bool winner = FALSE;

    for ( int i = horse->minValue(); i <= horse->maxValue(); i++ ) {
	if ( *(race->isFinished()) )
	    break;
	qApp->lock();
	horse->setValue( i );
	qApp->unlock();
	msleep( speed );
    }

    raceMutex.lock();
    if ( ! *(race->isFinished()) ) {
	race->finished();
	winner = TRUE;
    }
    raceMutex.unlock();

    race->waitForCeremony();

    if ( winner ) {
	qApp->lock();
	QPalette pal = horse->palette();
	qApp->unlock(FALSE);

	int min = double(horse->maxValue()) / double(2);
	int max = horse->maxValue();

	msleep( 500 );
	while ( !exit ) {
	    int j;
	    for ( j = horse->maxValue(); j > min; j-- ) {
		if ( exit )
		    break;
		double p = double(j) / double(max);
		qApp->lock();
		pal.setColor( QColorGroup::Button, QColor( 255*p, 255*(1-p), 0 ) );
		horse->setValue( j );
		horse->setPalette( pal );
		qApp->unlock();
		msleep( 1 );
	    }
	    if ( exit )
		break;
	    for ( j = min; j <= horse->maxValue(); j++ ) {
		if ( exit )
		    break;
		double p = double(j) / double(max);
		qApp->lock();
		pal.setColor( QColorGroup::Button, QColor( 255*p, 255*(1-p), 0 ) );
		horse->setValue( j );
		horse->setPalette( pal );
		qApp->unlock();
		msleep( 1 );
	    }
	}
    }
}

int Horse::toGo()
{
    int dist = horse->maxValue() - horse->value();
    return dist;
}

/*
  Race implementation
*/

Race::Race()
{
    setCaption( "Qt Slider Race - place your bets!" );
    setMargin( 15 );
    setSpacing( 35 );

    over = FALSE;
    comment = new Commentator( this );
    comment->start();
    startEvent = new QThreadEvent();
    finishEvent = new QThreadEvent();
    stopEvent = new QThreadEvent();
}

Race::~Race()
{
    delete comment;
    delete startEvent;
    delete finishEvent;
    delete stopEvent;
}

// guess what

int main( int argc, char **argv )
{
    srand( QTime::currentTime().msec() + 1000 * QTime::currentTime().second() );

    QApplication app( argc, argv );

    Race w;

    app.setMainWidget( &w );
    w.show();

    return app.exec();
}
