#include <qtextstream.h>
#include <qdom.h>
#include <qxml.h>
#include <qfile.h>
#include <qapplication.h>
#include <qwidget.h>

int main( int argc, char** argv )
{
  Q_ASSERT( argc == 2 );

  QFile file( argv[1] );
  if ( !file.open( IO_ReadOnly ) )
  {
    printf("Could not open file\n");
    return -1;
  }

  uint size = file.size();
  char* buffer = new char[ size + 1 ];
  file.readBlock( buffer, size );
  file.close();
  buffer[ size ] = 0;

  QString text( buffer );
  delete[] buffer;

  printf("Scanning now .... %s\n", text.ascii());

  QDomDocument doc;
  if ( !doc.setContent( text ) )
  {
    printf("Can not parse\n");
    return -1;
  }

  printf("Parsed\n");

  QTextStream str( stdout, IO_WriteOnly );
  str << doc;

  if ( doc.documentElement().nodeName() == "QDL" )
  {
      QApplication app( argc, argv );
      
      QWidget* w = doc.documentElement().toWidget();
      Q_ASSERT( w );
      w->show();
      
      app.exec();
  }
  return 0;
}
