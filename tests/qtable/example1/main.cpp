/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qtable.h>
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
    ReadOnlyNumber( QTable *t, EditType et, int num ) :
	QTableItem( t, et, QString::number( num ) ) { setReplaceable( FALSE ); }
    QWidget *createEditor() const { return 0; }
};

class ReadWriteNumber : public QTableItem
{
public:
    ReadWriteNumber( QTable *t, EditType et, int num ) :
	QTableItem( t, et, QString::number( num ) ) { setReplaceable( FALSE ); }
    QWidget *createEditor() const {
	QLineEdit *le = (QLineEdit*)QTableItem::createEditor();
	le->setValidator( new QIntValidator( le ) );
	le->setAlignment( AlignRight );
	return le;
    }
};

class SexChooser : public QTableItem
{
public:
    SexChooser( QTable *t, EditType et )
	: QTableItem( t, et, "female" ), cb( 0 ) { setReplaceable( FALSE ); }
    QWidget *createEditor() const {
	( (SexChooser*)this )->cb = new QComboBox( table()->viewport() );
	cb->insertItem( "female" );
	cb->insertItem( "male" );
	return cb;
    }
    void setContentFromCreateEditor( QWidget *w ) {
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
    CheckItem( QTable *t, EditType et ) : QTableItem( t, et, "No" ) { setReplaceable( FALSE ); }
    QWidget *createEditor() const {
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
    	table->setNumRows( table->numRows() + 1 );
	int row = table->numRows() - 1;
	table->setItem( row, 0, new ReadOnlyNumber( table, QTableItem::Never, row + 1 ) );
 	table->setItem( row, 3, new SexChooser( table, QTableItem::Always ) );
	table->setItem( row, 4, new ReadWriteNumber( table, QTableItem::OnTyping, row + 500 ) );
 	table->setItem( row, 5, new CheckItem( table, QTableItem::WhenCurrent ) );
    }

private:
    QTable *table;
};

int main( int argc, char **argv )
{
    QApplication a(argc,argv);			

    QVBox vbox( 0 );
    QTable *t = new QTable( 0, 6, &vbox );
    t->setSorting( TRUE );
    QPushButton *b = new QPushButton( "Add &Record", &vbox );
    b->setFixedWidth( b->sizeHint().width() );
    RecordManager *r = new RecordManager( t );
    QObject::connect( b, SIGNAL( clicked() ),
		      r, SLOT( addRecord() ) );
    r->addRecord();
    t->setText( 0, 1, "Mustermann" );
    t->setText( 0, 2, "Max" );
    ( (SexChooser*)t->item( 0, 3 ) )->setText( "male" );

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
