#include <qapplication.h>
#include <qfont.h>
#include "qlogview.h"
#include <stdio.h>

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );
    QLogView log;
    app.setMainWidget( &log );
    
      log.setText( "Per Kåre bottolfson jr.\n" );
// //      log.append( "per kåre knutsen\noddvar brå\n\n\nTrond Kjernåsen\n\n" );
     log.setFont( QFont("Times",18) );
     QString str;
     for ( int i = 0; i < 30000; i++ ) {
	 str.sprintf("%06d: 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n", i);
	 log.append( str );
	 if ( (i % 10000) == 0 )
	     fprintf(stderr,"\rLines done: %08d", i );
     }
//      fprintf(stderr,"\n");
//      for ( int i = 0; i < 3000000; i++ ) {
// 	 str.sprintf("%06d:\n", i);
// 	 log.append( str );
// 	 if ( (i % 10000) == 0 )
// 	     fprintf(stderr,"\rLines done: %08d", i );
//      }
     fprintf(stderr,"\n");
     str.sprintf("Trond Kjgyfzp\n");
     log.append( str );

// //    puts( log.text().latin1() );
//    log.setText("Trond");
    fprintf( stderr,"%d.%03d Kb\n", log.length()/1024, log.length() % 1024 );
    
    log.show();
//    log.setFont( QFont("courier",15) );
    return app.exec();
}
