/****************************************************************************
** $Id: //depot/qt/main/examples/table/main.cpp#2 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "../qtable.h"
#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qvalidator.h>

class ReadOnlyNumber : public QTableItem
{
public:
    ReadOnlyNumber( QTable *t, int num ) :
	QTableItem( t, QString::number( num ) ) { setTypeChangeAllowed( FALSE ); }
    QWidget *editor() const { return 0; }
};

class ReadWriteNumber : public QTableItem
{
public:
    ReadWriteNumber( QTable *t, int num ) :
	QTableItem( t, QString::number( num ) ) { setTypeChangeAllowed( FALSE ); }
    QWidget *editor() const {
	QLineEdit *le = (QLineEdit*)QTableItem::editor();
	le->setValidator( new QIntValidator( le ) );
	le->setAlignment( AlignRight );
	return le;
    }
};

class SexChooser : public QTableItem
{
public:
    SexChooser( QTable *t )
	: QTableItem( t, "female" ), cb( 0 ) { setTypeChangeAllowed( FALSE ); setEditType( Always ); }
    QWidget *editor() const {
	( (SexChooser*)this )->cb = new QComboBox( table()->viewport() );
	cb->insertItem( "female" );
	cb->insertItem( "male" );
	return cb;
    }
    void setContentFromEditor( QWidget *w ) {
	if ( w->inherits( "QComboBox" ) )
	    setText( ( (QComboBox*)w )->currentText() );
	else
	    QTableItem::setContentFromEditor( w );
    }
    void setText( const QString &s ) {
	if ( cb ) {
	    if ( s == "male" )
		cb->setCurrentItem( 1 );
	    else
		cb->setCurrentItem( 0 );
	}
	QTableItem::setText( s );
    }

private:
    QComboBox *cb;

};

class CheckItem : public QTableItem
{
public:
    CheckItem( QTable *t ) : QTableItem( t, "No" ) { setTypeChangeAllowed( FALSE ); setEditType( OnCurrent ); }
    QWidget *editor() const {
	QCheckBox *cb = new QCheckBox( "Yes", table()->viewport() );
	cb->setChecked( text() == "Yes" );
	return cb;
    }
    void setContentFromEditor( QWidget *w ) {
	if ( w->inherits( "QCheckBox" ) )
	    setText( ( (QCheckBox*)w )->isChecked() ? "Yes" : "No" );
	else
	    QTableItem::setContentFromEditor( w );
    }
};

class RecordManager : public QObject
{
    Q_OBJECT

public:
    RecordManager( QTable *t ) : table( t ) {}

public slots:
    void addRecord() {
    	table->setRows( table->rows() + 1 );
	int row = table->rows() - 1;
	table->setCellContent( row, 0, new ReadOnlyNumber( table, row + 1 ) );
 	table->setCellContent( row, 3, new SexChooser( table ) );
	table->setCellContent( row, 4, new ReadWriteNumber( table, row + 500 ) );
 	table->setCellContent( row, 5, new CheckItem( table ) );
    }

private:
    QTable *table;
};

int main( int argc, char **argv )
{
    QApplication a(argc,argv);			

    QVBox vbox( 0 );
    QTable *t = new QTable( 0, 6, &vbox );
    t->setSorting( FALSE );
    QPushButton *b = new QPushButton( "Add &Record", &vbox );
    b->setFixedWidth( b->sizeHint().width() );
    RecordManager *r = new RecordManager( t );
    QObject::connect( b, SIGNAL( clicked() ),
		      r, SLOT( addRecord() ) );
    r->addRecord();
    t->setCellText( 0, 1, "Mustermann" );
    t->setCellText( 0, 2, "Max" );
    ( (SexChooser*)t->cellContent( 0, 3 ) )->setText( "male" );

    t->horizontalHeader()->setLabel( 0, "Number" );
    t->horizontalHeader()->setLabel( 1, "Last Name" );
    t->horizontalHeader()->setLabel( 2, "First Name" );
    t->horizontalHeader()->setLabel( 3, "Sex" );
    t->horizontalHeader()->setLabel( 4, "ID-Number" );
    t->horizontalHeader()->setLabel( 5, "Receive Ads" );

    a.setMainWidget( &vbox );
    vbox.show();
    return a.exec();
}

#include "main.moc"
