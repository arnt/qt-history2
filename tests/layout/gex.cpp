/****************************************************************************
** $Id: //depot/qt/main/tests/layout/gex.cpp#2 $
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

RCSTAG("$Id: //depot/qt/main/tests/layout/gex.cpp#2 $");

#include <qwidget.h>

class Kill : public QWidget 
{
public:
    Kill( QWidget *parent, const char *name=0 )
	:QWidget(parent,name) {}
protected:
    void mouseReleaseEvent( QMouseEvent * );
};

#include <qevent.h>
#include <qapp.h>

void Kill::mouseReleaseEvent( QMouseEvent *m )
{
    QWidget *w = QApplication::widgetAt( mapToGlobal( m->pos() ),TRUE);
    if ( w && w != this && w != parentWidget() )
	delete w;
}




int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QGroupBox *f = new QGroupBox;
    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );

    QGridLayout *gm = new QGridLayout( f, 3, 3, 5 );


    /////////////////////////////////////////////////////////

    QPushButton* qb = new QPushButton( "Quit", f );
    a.connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    qb->setFixedSize( qb->size() );
    gm->addWidget( qb, 0, 2, AlignTop | AlignRight );

    QMultiLineEdit *ed = new QMultiLineEdit(f);
    ed->setText("This is supposed to be a large window\n you know.");
    ed->setMinimumSize( 150, 150 );
    //gm->addWidget( ed, 1, 1 );
    gm->addMultiCellWidget( ed, 1, 1, 1, 2 );

    QLabel* l1 = new QLabel(f);
    l1->setText("This is label 1.");
    l1->setBackgroundColor( yellow );
    l1->setMinimumSize( l1->sizeHint() );
    gm->addWidget( l1, 0, 1 );

    QLabel* l2 = new QLabel(f);
    l2->setText("This\nis\nlabel\ntoo.");
    l2->setBackgroundColor( red );
    l2->setMinimumSize( l2->sizeHint() );
    gm->addWidget( l2, 1, 0 );

    QLabel* l3 = new QLabel(f);
    l3->setText("This is label III.");
    l3->setBackgroundColor( red );
    l3->setMinimumSize( l3->sizeHint() );
    gm->addWidget( l3, 2, 2 );

    QLabel* l4 = new QLabel(f);
    l4->setText("More label.");
    l4->setBackgroundColor( cyan );
    l4->setMinimumSize( l4->sizeHint() );
    gm->addMultiCellWidget( l4, 2, 2, 0, 1 );


    ////////////////////////////////////////////////////////
    gm->activate();
    //gm->freeze();
    f->show();

    a.setMainWidget(f);
    return a.exec();
}

