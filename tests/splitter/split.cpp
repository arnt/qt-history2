#include <qapplication.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qfile.h>
#include <qdatastream.h>

#include <qlineedit.h>

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QSplitter top;
    QLabel l( "Label", &top );
    l.setBackgroundColor( Qt::blue.light() );

    (new QLabel( "Hei", &top))->setBackgroundColor( Qt::red );
    (new QLabel( "Hallo", &top))->setBackgroundColor( Qt::yellow );
    (new QLabel( "Hei", &top))->setBackgroundColor( Qt::magenta );
    (new QLabel( "Hei Hei", &top))->setBackgroundColor( Qt::white );

    a.setMainWidget( &top );
    {
	QFile f( "split.dat" );
	if ( f.open( IO_ReadOnly ) ) {
	    QDataStream s( &f );
	    QSize size;
	    s >> size;
	    QValueList<int> sizes;
	    s >> sizes;
	    top.resize(size);
	    top.setSizes( sizes );
	}
    }
    
    top.show();
    int r = a.exec();
    QValueList<int> sizes = top.sizes();
    QSize size = top.size();

    QValueListIterator<int> it;
      int i=0;
      for ( it = sizes.begin(); it != sizes.end(); ++it )
      debug( "size[%d]= %d", i++, *it );    

    {
	QFile f( "split.dat" );
	f.open( IO_WriteOnly );
	QDataStream s( &f );
	s << size;
	s << sizes;

    }
    return r;
}
