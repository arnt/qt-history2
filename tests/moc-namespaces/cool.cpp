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

void Fett::BoringGuy::playChess()
{
    warning( "Fett::CoolGuy::playChess()" );
}


void Fett::Arne::And::Anda::beenThere() const
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

void Fetere::CoolGuy::chillOut()
{
    warning( "Fetere::CoolGuy::chillOut()" );
}

void Fetere::BoringGuy::recitePi()
{
    warning( "Fetere::CoolGuy::recitePi()" );
    emit fellAsleep();
}

void Fetere::BoringGuy::playChess()
{
    warning( "Fetere::CoolGuy::playChess()" );
}


void Fetere::Arne::And::Anda::beenThere() const
{
    warning( "Fetere::CoolGuy::beenThere()" );
    emit doneThat();
}


