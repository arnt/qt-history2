/****************************************************************************
** $Id: //depot/qt/main/tests/newlayout/lw.cpp#2 $
**
** QGridLayout example
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#include <qapp.h>
#include <qlabel.h>
#include <qmlined.h>
#include <qcolor.h>
#include <qgrpbox.h> 
#include <qpushbt.h>
#include <qlayout.h>

#include <qgrid.h>
#include <qhbox.h>
#include <qvbox.h>

RCSTAG("$Id: //depot/qt/main/tests/newlayout/lw.cpp#2 $");


#include "lw.moc"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    QWidget *f;

     for ( int i = 0; i < 3; i++ ) {
	  switch ( i ) {
	  case 0: 
	       f= new QHBox;
	      break;
	  case 1: 
	      f = new QVBox;
	      break;
	  default:
	      f = new QGrid( 2, 2 );
	      break;
	  }
	  QLabel* l1 = new QLabel(f);
	l1->setText("This is label 1.");
	l1->setBackgroundColor( yellow );

	QLabel* l2 = new QLabel(f);
	l2->setText("This\nis\nlabel\ntoo.");
	l2->setBackgroundColor( red );

	QLabel* l3 = new QLabel(f);
	l3->setText("This is label III.");
	l3->setBackgroundColor( red );

	QLabel* l4 = new QLabel(f);
	l4->setText("More label.");
	l4->setBackgroundColor( cyan );

	f->show();
    }
    //    a.setMainWidget(f);
    a.exec();
}


#if 0


    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );

    QGridLayout *gm = new QGridLayout( f, 4, 4, 5 );


    gm->addColSpacing(3,25);
    gm->setColStretch(2,10);
    gm->setRowStretch(2,5);
    /////////////////////////////////////////////////////////

    QPushButton* qb = new QPushButton( "Quit", f );
    a.connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    qb->setFixedSize( qb->size() );
    gm->addWidget( qb, 0, 2, AlignCenter );
    

    QMultiLineEdit *ed = new QMultiLineEdit(f);
    ed->setText("This is supposed to be a large window\n you know.");
    ed->setMinimumSize( 150, 150 );
    //gm->addWidget( ed, 1, 1 );
    gm->addMultiCellWidget( ed, 1, 1, 1, 2 );


    ////////////////////////////////////////////////////////
    gm->activate();
    //gm->freeze();
#endif
