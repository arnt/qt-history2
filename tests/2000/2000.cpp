#include <qapplication.h>
#include <qdatetime.h>

int main( int, char ** )
{
    QDate d;

    debug( "new year" );
    d.setYMD( 1999, 12, 31 );
    debug( "d %d %d %d/%d-%d",
	   QDate::currentDate().daysTo( d ),
	   d.dayOfWeek(), d.day(), d.month(), d.year() );
    d = d.addDays( 1 );
    debug( "d %d %d %d/%d-%d",
	   QDate::currentDate().daysTo( d ),
	   d.dayOfWeek(), d.day(), d.month(), d.year() );

    int i;

    debug( "feb" );
    for( i=1; i < 30; i++ ) {
	d.setYMD( 2000, 2, i );
	debug( "d %d %d %d/%d-%d",
	       QDate::currentDate().daysTo( d ),
	       d.dayOfWeek(), d.day(), d.month(), d.year() );
    }

    debug( "feb 26 + 5" );
    d.setYMD( 2000, 2, 26 );
    for( i=0; i < 6; i++ ) {
	debug( "d %d %d %d/%d-%d",
	       QDate::currentDate().daysTo( d ),
	       d.dayOfWeek(), d.day(), d.month(), d.year() );
	d = d.addDays( 1 );
    }

    debug( "2001" );
    d.setYMD( 2001, 1, 1 );
    debug( "d %d %d %d/%d-%d",
	   QDate::currentDate().daysTo( d ),
	   d.dayOfWeek(), d.day(), d.month(), d.year() );

    debug( "feb 2001 26 + 5" );
    d.setYMD( 2001, 2, 26 );
    for( i=0; i < 6; i++ ) {
	debug( "d %d %d %d/%d-%d",
	       QDate::currentDate().daysTo( d ),
	       d.dayOfWeek(), d.day(), d.month(), d.year() );
	d = d.addDays( 1 );
    }

    d.setYMD( 2000, 3, 1 );
    debug( "d %d %d %d/%d-%d",
	   QDate::currentDate().daysTo( d ),
	   d.dayOfWeek(), d.day(), d.month(), d.year() );
    d.setYMD( 2001, 3, 1);
    debug( "d %d %d %d/%d-%d",
	   QDate::currentDate().daysTo( d ),
	   d.dayOfWeek(), d.day(), d.month(), d.year() );


}
