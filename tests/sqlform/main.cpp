#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqleditorfactory.h>
#include <qsqlform.h>
#include <qsql.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qsqlindex.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qsqlview.h>

#include "qsqlrecordnavigator.h"

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QSqlConnection::addDatabase( qApp->argv()[1],
				 qApp->argv()[2],
				 qApp->argv()[3],
				 qApp->argv()[4],
				 qApp->argv()[5]);
    
    QSqlDatabase * db = QSqlConnection::database();
    if( !db->isOpen() ) return 0;
    
    QSqlEditorFactory * f = new QSqlEditorFactory( qApp );
    QSqlView r( qApp->argv()[6] );
    QSqlForm form( 0 );
    QSqlIndex id( qApp->argv()[6] );
    
    id.append( r.field(1) );
    
    r.select( id );
    if(!r.first()) return 0;

    QWidget * le;
    QLabel * lb; 
    QVBoxLayout * v = new QVBoxLayout( &form );
    QGridLayout * g = new QGridLayout( v );
    
    g->setMargin( 5 );
    g->setSpacing( 3 );
    
    QSqlRecordNavigator * n = new QSqlRecordNavigator( &form );
    QObject::connect( n, SIGNAL(firstClicked()), &form, SLOT(first()) );
    QObject::connect( n, SIGNAL(previousClicked()), &form, SLOT(previous()) );
    QObject::connect( n, SIGNAL(nextClicked()), &form, SLOT(next()) );
    QObject::connect( n, SIGNAL(lastClicked()), &form, SLOT(last()) );
    QObject::connect( n, SIGNAL(updateClicked()), &form, SLOT(update()) );
    QObject::connect( n, SIGNAL(seekRecord(int)), &form, SLOT(seek(int)) );
    QObject::connect( n, SIGNAL(insertClicked()), &form, SLOT(insert()) );
    QObject::connect( n, SIGNAL(delClicked()), &form, SLOT(del()) );

    QObject::connect( &form, SIGNAL(recordChanged(int)), n, 
		      SLOT(updateRecordNum(int)) );
    
    for(int i = 0; i < r.count(); i++){
	lb = new QLabel( r.field(i)->name(), &form, 
			 QString().sprintf( "Label_%d", i ) );
	g->addWidget( lb, i, 0 );
	
	le = f->createEditor( &form, r[i] );
	le->setName( QString().sprintf( "Editor_%d", i ) );
	g->addWidget( le, i, 1 );
	form.associate( le, r.field(i) );
    }
    
    v->addWidget( n );
  
    a.setMainWidget( &form );
    form.setView( &r );
    form.syncWidgets();
    form.show();
    
    return a.exec();
};
