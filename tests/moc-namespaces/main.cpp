#include <qapplication.h>
#include "cool.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );

    Fett::CoolGuy cool1;
    Fett::BoringGuy boring1;
    Fett::FunnyGuy funny1;
    Fett::Arne::And::Anda duck1;

    Fetere::CoolGuy cool2;
    Fetere::BoringGuy boring2;
    Fetere::FunnyGuy funny2;
    Fetere::Arne::And::Anda duck2;

    QObject::connect( &cool1, SIGNAL(hadMartini()),
                      &cool2, SLOT(chillOut()) );
    QObject::connect( &cool1, SIGNAL(hadMartini()),
                      &duck2, SLOT(beenThere()) );

    QObject::connect( &cool2, SIGNAL(hadMartini()),
                      &boring1, SLOT(playChess()) );
    QObject::connect( &cool2, SIGNAL(hadMartini()),
                      &duck1, SLOT(beenThere()) );

    QObject::connect( &funny1, SIGNAL(giggled()),
                      &funny2, SLOT(tellJoke()) );

    QObject::connect( &funny2, SIGNAL(giggled()),
                      &boring2, SLOT(recitePi()) );



    cool1.highFive( 5 );
    cool2.highFive( 2 );
}
