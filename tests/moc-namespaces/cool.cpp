#include "cool.h"


void Fett::CoolGuy::highFive( int times )
{
    warning( "Fett::CoolGuy::highFive(%i)", times );
    int i;
    for( i = 0 ; i < times ; i++ )
	emit hadMartini();
}

void Fett::CoolGuy::chillOut()
{
    warning( "Fett::CoolGuy::chillOut()" );
}

void Fett::BoringGuy::recitePi()
{
    warning( "Fett::CoolGuy::recitePi()" );
    emit fellAsleep();
}

void Fett::BoringGuy::playChess() const
{
    warning( "Fett::CoolGuy::playChess()" );
}

void Fett::FunnyGuy::tellJoke()
{
    warning( "Fett::FunnyGuy::tellJoke()" );
    emit giggled();
}

void Fett::Arne::And::Anda::beenThere()
{
    warning( "Fett::CoolGuy::beenThere()" );
    emit doneThat();
}



void Fetere::CoolGuy::highFive( int times )
{
    warning( "Fetere::CoolGuy::highFive(%i)", times );
    int i;
    for( i = 0 ; i < times ; i++ )
	emit hadMartini();
}

void Fetere::CoolGuy::chillOutWith( Fett::CoolGuy &c )
{
    warning( "Fetere::CoolGuy::chillOutWith()" );
    c.highFive(1);
}

void Fetere::BoringGuy::recitePi()
{
    warning( "Fetere::CoolGuy::recitePi()" );
    emit fellAsleep();
}

void Fetere::BoringGuy::playChess() const
{
    warning( "Fetere::CoolGuy::playChess()" );
}

void Fetere::FunnyGuy::tellJoke()
{
    warning( "Fetere::FunnyGuy::tellJoke()" );
    emit giggled();
}

void Fetere::Arne::And::Anda::beenThere()
{
    warning( "Fetere::CoolGuy::beenThere()" );
    emit doneThat();
}


