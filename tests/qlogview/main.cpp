#include <qapplication.h>
#include <qfont.h>
#include <qtextview.h>
#include "qlogview.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );
//    QTextView log;
    QLogView log;
    app.setMainWidget( &log );
    
    log.setText( "Per Kåre \n\n\nbottolfson jr.\n", Qt::red );
  //  log.setPaletteBackgroundColor( Qt::black );
    log.append( "per kåre æøå knutsen\noddvar brå\n\n\nTrond Kjernåsen\n\n", Qt::green);
//    log.setFont( QFont("Times",18) );
    QString str;
    timeval st, et;
    gettimeofday( &st, 0 );
    for ( int i = 0; i < 200000; i++ ) {
	str.sprintf("%06d: 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n", i);
	log.append( str );
// 	if ( (i % 2000) == 0 )
// 	    fprintf(stderr,"\rLines done: %08d", i );
    }
    gettimeofday( &et, 0 );

    log.setLineColor( 5000, Qt::blue );
    fprintf(stderr,"\n");
    fprintf(stderr,"Loop time: %d.%03d s\n", (int)(et.tv_sec - st.tv_sec),(int)(abs(et.tv_usec - st.tv_usec)%1000000) / 1000);
//     for ( int i = 0; i < 3000000; i++ ) {
// 	str.sprintf("%06d:\n", i);
// 	log.append( str );
// 	if ( (i % 10000) == 0 )
// 	    fprintf(stderr,"\rLines done: %08d", i );
//     }
    fprintf(stderr,"\n");
    str.sprintf("Trond Kjgyfzp\n");
    log.append( str );

// //    puts( log.text().latin1() );
//     log.setText("Trond\n");
//     for(int i = 0; i < 15000; i++)
// 	log.append("%06d: 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n");
//     gettimeofday( &st, 0 );
//     for ( int i = 0; i < 60000; i++ ) {
// 	str.sprintf("%06d: 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n", i);
// 	log.append( str );
// 	if ( (i % 2000) == 0 )
// 	    fprintf(stderr,"\rLines done: %08d", i );
//     }
//     gettimeofday( &et, 0 );

//     fprintf(stderr,"\n");
//     fprintf(stderr,"Loop time: %d.%03d s\n", (et.tv_sec - st.tv_sec),(abs(et.tv_usec - st.tv_usec)%1000000) / 1000);

    fprintf( stderr,"%d.%03d Kb\n", log.length()/1024, log.length() % 1024 );
    
    log.show();
//    log.setFont( QFont("courier",15) );
    return app.exec();
}
