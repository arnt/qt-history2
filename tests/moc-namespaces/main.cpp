#include <qapplication.h>
#include "cool.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );

    Fett::CoolGuy cool1;
    Fett::BoringGuy boring1;
    Fett::Arne::And::Anda duck1;

    Fetere::CoolGuy cool2;
    Fetere::BoringGuy boring2;
    Fetere::Arne::And::Anda duck2;

    QObject::connect( &cool1, SIGNAL(hadMartini()),
                      &cool2, SLOT(chillOut()) );
    QObject::connect( &cool1, SIGNAL(hadMartini()),
                      &duck2, SLOT(beenThere()) );

    QObject::connect( &cool2, SIGNAL(hadMartini()),
                      &boring1, SLOT(playChess()) );
    QObject::connect( &cool2, SIGNAL(hadMartini()),
                      &duck1, SLOT(beenThere()) );



    cool1.highFive( 5 );
    cool2.highFive( 2 );
}
