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

#include <qapplication.h>
#include <qinputdialog.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qlabel.h>

class Test : public QVBox
{
    Q_OBJECT

public:
    Test() {
	setSpacing( 5 );
	setMargin( 5 );
	QPushButton *b = 0;
	b = new QPushButton( "Get Text", this );
	connect( b, SIGNAL( clicked() ), this, SLOT( lined() ) );
	b = new QPushButton( "Get Number", this );
	connect( b, SIGNAL( clicked() ), this, SLOT( num() ) );
	b = new QPushButton( "Get Double", this );
	connect( b, SIGNAL( clicked() ), this, SLOT( dbl() ) );
	b = new QPushButton( "Get From List", this );
	connect( b, SIGNAL( clicked() ), this, SLOT( lst1() ) );
	b = new QPushButton( "Get From List (Editable)", this );
	connect( b, SIGNAL( clicked() ), this, SLOT( lst2() ) );
	res = new QLabel( "Nothing yet....", this );
    }



private slots:
    void lined() {
	bool ok = FALSE;
	QString s = QInputDialog::getText( "Enter a Text", "Text:", "Default", &ok );
	if ( !s.isEmpty() && ok )
	    res->setText( "Entered Text: " + s );
	else
	    res->setText( "Cancel Pressed!" );
    }

    void num() {
	bool ok = FALSE;
	int num = QInputDialog::getInteger( "Enter a Number", "Number:", 22, 0, 1000, 1, &ok );
	if ( ok )
	    res->setText( "Entered Number: " + QString::number( num ) );
	else
	    res->setText( "Cancel Pressed!" );
    }

    void dbl() {
	bool ok = FALSE;
	double num = QInputDialog::getDouble( "Enter a Number", "Number:", 33.6, 0, 1000, 2, &ok );
	if ( ok )
	    res->setText( "Entered Number: " + QString::number( num ) );
	else
	    res->setText( "Cancel Pressed!" );
    }

    void lst1() {
	QStringList lst;
	lst << "First" << "Second" << "Third" << "Fourth" << "Fifth";
	bool ok = FALSE;
	QString s = QInputDialog::getItem( "Select an Item", "Item:", lst, 3, FALSE, &ok );
	if ( !s.isEmpty() && ok )
	    res->setText( "Selected Item: " + s );
	else
	    res->setText( "Cancel Pressed!" );
    }

    void lst2() {
	QStringList lst;
	lst << "First" << "Second" << "Third" << "Fourth" << "Fifth";
	bool ok = FALSE;
	QString s = QInputDialog::getItem( "Select an Item", "Item:", lst, 3, TRUE, &ok );
	if ( !s.isEmpty() && ok )
	    res->setText( "Selected Item: " + s );
	else
	    res->setText( "Cancel Pressed!" );
    }

private:
    QLabel *res;

};



int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    Test t;
    a.setMainWidget( &t );
    t.show();
    return a.exec();
}

#include "main.moc"
