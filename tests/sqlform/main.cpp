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

class MyForm : public QSqlForm
{
public:
    MyForm( QWidget * parent = 0, const char * name = 0 );
    void init();
    
public slots:
    bool insert();
    bool update();
    bool del();
};

MyForm::MyForm( QWidget * parent, const char * name )
    : QSqlForm( parent, name )
{
}

void MyForm::init()
{
    if( !view() ) return;
    
    QSqlRecordNavigator * n = new QSqlRecordNavigator( this );
    connect( n, SIGNAL(firstClicked()), SLOT(first()) );
    connect( n, SIGNAL(previousClicked()), SLOT(previous()) );
    connect( n, SIGNAL(nextClicked()), SLOT(next()) );
    connect( n, SIGNAL(lastClicked()), SLOT(last()) );
    connect( n, SIGNAL(updateClicked()), SLOT(update()) );
    connect( n, SIGNAL(seekRecord(int)), SLOT(seek(int)) );
    connect( n, SIGNAL(insertClicked()), SLOT(insert()) );
    connect( n, SIGNAL(delClicked()), SLOT(del()) );

    connect( this, SIGNAL(recordChanged(int)), n, SLOT(updateRecordNum(int)) );       

    QSqlEditorFactory f( qApp );
    QWidget * le;
    QLabel * lb; 
    QVBoxLayout * vb = new QVBoxLayout( this );
    QGridLayout * g = new QGridLayout( vb );
    
    g->setMargin( 5 );
    g->setSpacing( 3 );
    
    for(int i = 0; i < view()->count(); i++){
	QString name = view()->field( i )->name();
	name[0] = name[0].upper(); // capitalism the first letter

	lb = new QLabel( name, this );
	g->addWidget( lb, i, 0 );
	
	le = f.createEditor( this, view()->value( i ) );
	g->addWidget( le, i, 1 );
	associate( le, view()->field( i ) );
    }
    
    vb->addWidget( n );
    syncWidgets();
}

bool MyForm::insert()
{
    if( view() ){
	syncFields();
	view()->insert();
	view()->select( view()->filter(), view()->sort() );
	view()->last();
	syncWidgets();
	return TRUE;
    }
    return FALSE;
}

bool MyForm::update()
{
    if( view() ){
	syncFields();
	int at = view()->at();
	if( view()->update( view()->primaryIndex() ) && 
	    view()->select( view()->filter(), view()->sort() ) &&
	    view()->seek( at ) )
	    return TRUE;
    }
    
    return FALSE;
}

bool MyForm::del()
{
    if( view() && view()->del( view()->primaryIndex() ) ){
	view()->select( view()->filter(), view()->sort() );
	view()->first();
	syncWidgets();
	return TRUE;
    }
    return FALSE;
}



int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    MyForm form( 0 );

    QSqlConnection::addDatabase( qApp->argv()[1],
				 qApp->argv()[2],
				 qApp->argv()[3],
				 qApp->argv()[4],
				 qApp->argv()[5]);
    
    QSqlDatabase * db = QSqlConnection::database();
    if( !db->isOpen() ) return 0;
    
    QSqlView view( qApp->argv()[6] );
    QSqlIndex order( qApp->argv()[6] );
    
    order.append( view.field( 0 ) );
    view.select( order );
    if(!view.first()) return 0;

    a.setMainWidget( &form );
    form.setView( &view );
    form.init();
    form.show();
    return a.exec();
};
