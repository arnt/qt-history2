#include <qapplication.h>
#include <qdatetime.h>
#include "main.h"

/*This is a nifty comment 
 */

int main(int argc, char **argv)
{
  QApplication a(argc, argv);
  Sender sender;
  Receiver receiver;
  QObject::connect( &sender, SIGNAL( signal1() ), &receiver, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal2() ), &receiver, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal3() ), &receiver, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal4() ), &receiver, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal5() ), &receiver, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal6() ), &receiver, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal7() ), &receiver, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal8() ), &receiver, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal9() ), &receiver, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal10() ), &receiver, SLOT( slot() ) );
  /*  Receiver receiver2;
  QObject::connect( &sender, SIGNAL( signal1() ), &receiver2, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal2() ), &receiver2, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal3() ), &receiver2, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal4() ), &receiver2, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal5() ), &receiver2, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal6() ), &receiver2, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal7() ), &receiver2, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal8() ), &receiver2, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal9() ), &receiver2, SLOT( slot() ) );
  QObject::connect( &sender, SIGNAL( signal10() ), &receiver2, SLOT( slot() ) );
  */
  QTime time;
  time.start();
  for (int i = 0; i < 100000; i++ )
      sender.emitSignal();
  qDebug(" %d", time.elapsed() );
}

