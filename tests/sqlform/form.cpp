#include "form.h"
#include <qlineedit.h>
#include <qdatetimeedit.h>
#include <qsqlform.h>
#include <qsqlcursor.h>
#include <qsqlrecord.h>
#include <qapplication.h>

Form::Form( QWidget * parent, const char * name )
    : FormBase( parent, name )
{
    cursor = new QSqlCursor( "simpletable" );
    cursor->select( cursor->primaryIndex() );
    cursor->next();
    QSqlRecord * r = cursor->editBuffer();
    
    form = new QSqlForm( this );
    form->insert( dateEdit, r->field("date") );
    form->insert( nameEdit, r->field("name") );
    form->insert( addressEdit, r->field("address") );
    form->readFields();
}


void Form::close()
{
    qApp->quit();
}

void Form::clear()
{
    form->clearValues();
    form->readFields();
}

void Form::update()
{
    form->writeFields();
    int n = cursor->at();
    cursor->update();
    cursor->select( cursor->primaryIndex() );
    cursor->seek( n );
}

void Form::insert()
{
    QSqlQuery q("select max(id)+1 from simpletable;");
    if( q.next() ){
	qDebug("id:" + cursor->value("id").toString() );
	QSqlRecord * r = cursor->editBuffer( TRUE );
	r->setValue("id", q.value(0) );
	form->writeFields();
	cursor->insert();
	cursor->select( cursor->primaryIndex() );
    }
}

void Form::del()
{
    int n = cursor->at();
    
    if( cursor->del() ){
	cursor->select( cursor->primaryIndex() );
	if( !cursor->seek( n ) )
	    cursor->last();
	cursor->primeEditBuffer();
	form->readFields();
    }    
}

void Form::first()
{
    cursor->first();
    cursor->primeEditBuffer();
    form->readFields();
}

void Form::prev()
{
    if( !cursor->prev() ){
	cursor->first();
    }
    cursor->primeEditBuffer();
    form->readFields();
}

void Form::next()
{
    if( !cursor->next() ){
	cursor->last();
    }
    cursor->primeEditBuffer();
    form->readFields();
}

void Form::last()
{
    cursor->last();
    cursor->primeEditBuffer();
    form->readFields();
}

