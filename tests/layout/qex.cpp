/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <qapplication.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

#include <qlayout.h>

class ExampleWidget : public QWidget
{
    Q_OBJECT
public:
    ExampleWidget( QWidget *parent=0, const char* name = 0 );
public slots:
    void insertWidget(); 
private:
    QBoxLayout *testbox;
};


#include "qex.moc"


void ExampleWidget::insertWidget()
{
    QLabel *nl = new QLabel( "Inserting", this );
    nl->setBackgroundColor( QColor( 222, 150, 111 ) );
    //    testbox->insertItem( 1, new QWidgetItem( nl ) );
    testbox->insertWidget( 1, nl );
    nl->show();
}


ExampleWidget::ExampleWidget( QWidget *parent, const char* name)
    :QWidget(parent,name), testbox(0)
{
    QBoxLayout *gm = new QBoxLayout( this, QBoxLayout::TopToBottom, 5 );
#if 1
    QBoxLayout *b1 = new QBoxLayout( QBoxLayout::LeftToRight);
    testbox = b1;
    gm->addLayout( b1, 10 );

    for ( int i=0; i<4; i++ ) {
	QLabel* lab = new QLabel(this);
	lab->setText("Testing");
	lab->setFrameStyle( QFrame::Panel   | QFrame::Plain );
	lab->setAlignment( AlignTop | AlignHCenter );
	lab->setBackgroundColor(yellow);

	b1->addWidget( lab, 20 );
    }

    QPushButton* tb = new QPushButton( "Insert", this );
    connect( tb, SIGNAL(clicked()), this, SLOT(insertWidget()) );
    b1->addWidget( tb, 0, AlignTop );

    QPushButton* qb = new QPushButton( "Quit", this );
    qApp->connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    b1->addWidget( qb, 0, AlignTop );


#endif
    QLabel* large = new QLabel(this);
    large->setText("This is supposed to be a large window\n you know.");
    large->adjustSize();
    large->setMinimumSize( large->size());
    large->setFrameStyle( QFrame::Panel | QFrame::Plain );
    large->setAlignment( AlignVCenter | AlignHCenter );
    large->setBackgroundColor(white);

    QBoxLayout *b2 = new QBoxLayout( QBoxLayout::LeftToRight);
    gm->addLayout( b2, 50 );

    b2->addWidget( large, 100 ); // hstretch

    {
	QLabel* s = new QLabel(this);
	s->setText("This\n is\n supposed\n to be\n centered\n relatively.");
	s->adjustSize();
	s->setMinimumSize( s->size());
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( AlignVCenter | AlignHCenter );
	s->setBackgroundColor(cyan);
	b2->addWidget( s, 10, AlignCenter );
    }

    {
	QLabel* s = new QLabel(this);
	s->setText("This is a widget inside the outermost box");
	s->adjustSize();
	s->setMinimumSize( s->width(), s->height() );
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( AlignVCenter | AlignHCenter );
	s->setBackgroundColor(red);
	gm->addWidget( s, 1 );
    }

    gm->activate();
    //gm->freeze();

}


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    ExampleWidget f;

    f.show();

    a.setMainWidget(&f);
    return a.exec();
}
