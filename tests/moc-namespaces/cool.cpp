#include "cool.h"


void Fett::CoolGuy::highFive( int times )
{
    qWarning( "Fett::CoolGuy::highFive(%i)", times );
    int i;
    for( i = 0 ; i < times ; i++ )
	emit hadMartini();
}

void Fett::CoolGuy::chillOut()
{
    qWarning( "Fett::CoolGuy::chillOut()" );
}

void Fett::BoringGuy::recitePi()
{
    qWarning( "Fett::CoolGuy::recitePi()" );
    emit fellAsleep();
}

void Fett::BoringGuy::playChess() const
{
    qWarning( "Fett::CoolGuy::playChess()" );
}

void Fett::FunnyGuy::tellJoke()
{
    qWarning( "Fett::FunnyGuy::tellJoke()" );
    emit giggled();
}

void Fett::Arne::And::Anda::beenThere()
{
    qWarning( "Fett::CoolGuy::beenThere()" );
    emit doneThat();
}



void Fetere::CoolGuy::highFive( int times )
{
    qWarning( "Fetere::CoolGuy::highFive(%i)", times );
    int i;
    for( i = 0 ; i < times ; i++ )
	emit hadMartini();
}

void Fetere::CoolGuy::chillOutWith( Fett::CoolGuy &c )
{
    qWarning( "Fetere::CoolGuy::chillOutWith()" );
    c.highFive(1);
}

void Fetere::BoringGuy::recitePi()
{
    qWarning( "Fetere::CoolGuy::recitePi()" );
    emit fellAsleep();
}

void Fetere::BoringGuy::playChess() const
{
    qWarning( "Fetere::CoolGuy::playChess()" );
}

void Fetere::FunnyGuy::tellJoke()
{
    qWarning( "Fetere::FunnyGuy::tellJoke()" );
    emit giggled();
}

void Fetere::Arne::And::Anda::beenThere()
{
    qWarning( "Fetere::CoolGuy::beenThere()" );
    emit doneThat();
}


